/* program wczytuje dwa zestawy danych -> intigery(-d) oraz float (-f)
 * każdy inny argument ma być automatycznie ignorowany
 * program działa do momentu, aż występują parametry
 * to, co przechwytuje z lini poleceń to string -> trzeba zamienić na int/float
 * na końcu ma wypisać średnią policznoną po podanych parametrach
 * */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>

int main(int argc, char* argv[])
{
	//przechowywanie sumy liczb
	float suma = 0.0;
	//przechowywanie liczby dodanych
	int lb = 0;
	int tempInt = 0; //bedziemy tymczasowo przechowywać zmienna int
	float tempFloat = 0.0; // bedzimey tymczasowo przechowywać zmienna float
	char* wart = NULL;
	char** endPtr = NULL;
	int c;

	while(( c= getopt(argc, argv, "d:f:")) != -1)
		switch(c)
		{
			case 'd':
				wart = optarg; //zapisujemy wartosc arg -> będziemy musieli zmienić na int!!	
				//printf("value d = %s\n", optarg);
				tempInt = strtod(wart, endPtr);
				printf("Int value = %d\n", tempInt);
				lb++;
				suma += tempInt;
				break;

			case 'f':
				wart = optarg;
				//printf("value f = %s\n", wart);
				tempFloat = strtod(wart, endPtr);
				printf("Float value = %lf\n", tempFloat);
				lb++;
				suma += tempFloat;
				break;

			case '?':
				if(optopt == 'd' || optopt == 'f')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if(isprint(optopt))
					fprintf(stderr, "Unknown option -%c \n", optopt);
				else
					fprintf(stderr, "Unknown option character %x .\n", optopt);
				return 1;

			default:
				abort();
		}

	float sr = suma/lb;
	printf("\n----------------------\nSuma:\t%lf,\tLiczba zmiennych:\t%d,\nSrednia:\t%lf\n----------------------\n", suma, lb, sr);		
		


	return 0;
}
