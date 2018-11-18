//program wspomagający program rnd_doubles_idx
//samodzielne wywołanie:
//w terminalu wpisujemy kolejno:
//gcc -Wall dbl_index.c
//./a.out -i <int> -b <int> -o <string> <string>
//program działa, dopóki ręcznie nie usuniemy pliku .gen-live
//w tym czasie możemy przeglądać plik wynikowy o nazwie podanej w parametrze -o
//jeżeli usuniemy flagę .gen-live, plik result również zostanie usunięty
//
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <float.h>
#include <time.h>

void parameterUsageError(char p)
{
	printf("Wrong usage: parameter -%c is mandatory!\n", p);
}

int main(int argc, char* argv[])
{
	int c;
	int varRange=0;
	int noRange=0;
	char resultFileName[30];
	char dataSource[30];
	while((c=getopt(argc, argv, "i:b:o:")) != -1)
		switch(c)
		{
			case 'i':
				varRange = strtol(optarg, NULL, 0);
				if(varRange <= 0)
				{
					perror("Parameter -i needs <int>\n");
					return 1;
				}
				break;
			case 'b':
				noRange = strtol(optarg, NULL, 0);
				if(noRange <= 0)
				{
					perror("Parameter -b needs <int>\n");
					return 1;
				}
				break;
			case 'o':
				//resultFileName = optarg;
				strcpy(resultFileName, optarg);
				if(resultFileName == NULL)
				{
					perror("Parameter -o needs <string>\n");
					return 1;
				}
				break;
			case '?':
				perror("Wrong usage of parameters, guess what you did wrong\n");
				return 1;
		}

	//sprawdzamy, czy wszystkie obowiązkowe parametry są załączone
	if(varRange == 0)
	{
		parameterUsageError('i');
		return 1;
	}

	if(noRange == 0)
	{
		parameterUsageError('b');
		return 1;
	}

	if(resultFileName == NULL)
	{
		parameterUsageError('o');
		return 1;
	}

	if(!strcpy(dataSource, argv[optind]))
	{
		printf("Needed source file path\n");
		return 1;
	}

	//otwieramy wszystkie niezbędne pliki we/wy
	int randomDoublesFile = open(dataSource, O_RDONLY);
	if(randomDoublesFile < 0)
	{
		printf("randomDoublesFile error\n");
		return 1;
	}

	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int resultFile = open(resultFileName, O_CREAT | O_WRONLY, mode);

	//obliczamy zakresy, wielkosc pliku i ilosc ziennych znajdujacych się w źródłowym pliku binarnym
	struct stat st;
	stat(dataSource, &st);
	ssize_t dataSize = st.st_size;
	int quantityOfFigures = dataSize / sizeof(double); //ilosc lb w pliku
	double randomDoublesBuff[quantityOfFigures];
	memset(randomDoublesBuff, 0, quantityOfFigures);

	double rangeValue = (DBL_MAX/varRange) * 2;

	//czytamy plik z danymi
	read(randomDoublesFile, &randomDoublesBuff, dataSize);
	
	//tworzymy zmienne potrzebne do wykonywania przeszukania pliku pod względem zmiennych z zadanego przedziału histogramu
	double firstRange = -DBL_MAX;
   double secondRange = firstRange + rangeValue;

	for(int i=0; i<noRange; i++)
	{
		firstRange += rangeValue;
		secondRange += rangeValue;
	}

	//glowna pętla, która nam sprawdza liczby z pliku
	int i = 0;
	while(randomDoublesBuff[i])
	{
		if(randomDoublesBuff[i] >=	firstRange && randomDoublesBuff[i] < secondRange)
		{
			char tempBuff[50];
			char tempVar[20];
			char tempIdx[20];

			memset(tempBuff, 0, sizeof(tempBuff)/sizeof(char));
			memset(tempVar, 0, sizeof(tempVar)/sizeof(char));
			memset(tempIdx, 0, sizeof(tempIdx)/sizeof(char));

			sprintf(tempIdx, "%d", i);
			sprintf(tempVar, "%lg", randomDoublesBuff[i]);
			strcat(tempBuff, tempIdx);
			strcat(tempBuff, " ");
			strcat(tempBuff, tempVar);
			strcat(tempBuff, "\n");

			//zapisanie do pliku
			write(resultFile, tempBuff, sizeof(tempBuff)/sizeof(char));
		}
		i++;
	}

	//pętla aktywna sprawdzająca, czy .gen-live dalej istnieje
	int genLive = open(".gen-live", O_WRONLY | O_CREAT, mode);

	struct timespec tim, tim2;
	tim.tv_sec = 1;
	tim.tv_nsec = 500000000L;

	while(stat(".gen-live", &st) == 0) //plik istnieje
	{
		nanosleep(&tim, &tim2);
		printf("still working...\n");
	}

	close(resultFile);
	close(randomDoublesFile);

	//usuniecie pliku indeksowego
	if(unlink(resultFileName))
		printf("Niepoprawnie usunięto plik!\n");


	return 0;
}
