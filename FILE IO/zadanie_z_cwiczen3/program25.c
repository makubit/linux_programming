// vim
//program losuje 1024^2 liczby
//
//open, read, write, close
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>

#define lb (1024*1024/sizeof(double))

long int d_lb;
char* sciezka;


int main(int argc, char* argv[])

{
	/*if(argc != 3)
	{
		perror("Niewlasciwa liczba parametrów.\n-d <<int> rozmiar pliku z wynikami>\n-o <sciezka do pliku z wynikami>\n");
		return 1;
	}*/

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
				perror("parametr-o nie moze byc nullem");
			sciezka = optarg;
			break;
		case '?':
			perror("Niewlasciwa skladnia\n");
			break;
	}

	//printf("%s\n", sciezka);
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int file = open(sciezka, O_WRONLY | O_CREAT, mode);

	int randomData = open("/dev/urandom", O_RDONLY); //1.czy tutaj mam zdefiniowac wielkosc jakos??
	if (randomData < 0)
	{
		printf("Cos poszlo nie tak :(\n");
		return 1;
	}


	double myRandomData[lb*d_lb];
	ssize_t result = read(randomData, myRandomData, sizeof(myRandomData));
	ssize_t nbytes = sizeof(myRandomData);

	for(int i =0; i<lb*d_lb; i++) //lb * -d, które jest intem przekazywanym przez parametr
	{
		while( fpclassify(myRandomData[i]) != FP_NORMAL )
		{
			read(randomData, (myRandomData+i), sizeof(double));
			//podmienia jedna w kolko, dopoki jest NAN	
		}	
	}
	
	//ssize_t bytes_written = write(file, myRandomData, nbytes);
	write(file, myRandomData, nbytes);


	return 0;
}
