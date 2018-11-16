// Program generuje plik binarny zawierający liczby zmiennoprzecinkowe double.
// Pobierane są z urządzenia /dev/urandom
// Weryfikacja, czy dane nie reprezentują wartości specjanej (NaN) za pomocą fpclassify
// Niepoprawne liczby są losowane ponownie
// 
// Program na konsoli pokazuje jakie liczby zostały zamienione
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>

#define LB (64*1024/sizeof(double))

long int d_lb;
char* sciezka;


int main(int argc, char* argv[])
{

	//sprawdzamy poprawność parametrów
	//
	int c;
	while((c=getopt(argc, argv, "d:o:")) != -1)
	switch(c)
	{
		case 'd':
			if(strtol(optarg, NULL, 0) <=0)
				perror("paramert -d > 0\n");
			
			d_lb = strtol(optarg, NULL, 0);
			break;
		case 'o':
			if(optarg == NULL)
				perror("parametr -o nie moze byc nullem");
			sciezka = optarg;
			break;
		case '?':
			break;
	}

	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int file = open(sciezka, O_WRONLY | O_CREAT, mode);

	int randomData = open("/dev/urandom", O_RDONLY, mode);
	if (randomData < 0)
	{
		printf("/dev/urandom error\n");
		return 1;
	}

	double myRandomData[LB*d_lb];
	memset(myRandomData, 0, LB*d_lb);

	ssize_t nbytes = sizeof(myRandomData);
	ssize_t result = read(randomData, myRandomData, nbytes);
	if(result < 0)
	{
		perror("reading randomData error\n");
		return 1;
	}

	for(int i =0; i<LB*d_lb; i++) //lb * -d, które jest intem przekazywanym przez parametr
	{
		while( fpclassify(myRandomData[i]) != FP_NORMAL )
		{
			//dla pewnosci drukujemy nany, sprawdzamy, czy udało się je zamienić
			printf("Podmieniam nan %d: -> %lg", i, myRandomData[i]);
			read(randomData, (myRandomData+i), sizeof(double));
			printf("-> %lg\n", myRandomData[i]);
			//podmienia jedna w kolko, dopoki jest NAN	
		}	
	}
	
	ssize_t bytes_written = write(file, myRandomData, nbytes);
	if(bytes_written < 0)
			perror("bytes_written error");

	return 0;
}
