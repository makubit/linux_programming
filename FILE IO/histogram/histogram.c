//program wyznacza histogram z danych zawartych w pliku wskazanym parametrem -b (nieobowiązkowy)
//

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <float.h> //zeby miec mozliwosc dodac stala DBL_MAX
#include <sys/stat.h>

long int lb_przedzialow = 64;

int main(int argc, char* argv[])
{
  if(argv[1] == NULL)
			 perror("Niewlasciwa sciezka");
  //printf("%s\n", argv[1]);

  int c;

  while((c=getopt(argc, argv, "b:")) != -1)
	 switch(c)
	 {
		case 'b':
		  if(strtol(optarg, NULL, 0) <= 0)
					 perror("Niepoprawny przedzial\n");
		  lb_przedzialow = strtol(optarg, NULL, 0);
		  break;

		default:
		  break;
	 }

  int randomDoubles = open(argv[1], O_RDONLY);
  if(randomDoubles <0)
			 perror("randomDoubles error");

  double stale_przedzialy = DBL_MAX/lb_przedzialow;
  printf("%lg\n\n", 2*stale_przedzialy);
  printf("%lg\n", DBL_MAX);
  printf("%lg\n", DBL_MIN);
  printf("%lg\n", DBL_MAX - DBL_MIN);

  //liczymy wielkosc pliku
  struct stat st;
  stat(argv[1], &st);
  ssize_t size = st.st_size;
  //printf("%d\n", size); -> trzeba pamiętać, że tą wielkość trzeba podzielić jeszcze przez sizeof(double)
  double buf[size/sizeof(double)];
  memset(buf, 0, size/sizeof(double));


  ssize_t randomDoublesSize = read(randomDoubles, &buf, size);

  int ilosc_lb_w_przedziale = 0;

  double pierwszy_zakres = DBL_MIN;
  double min = DBL_MIN;
  double drugi_zakres = min + 2*stale_przedzialy;
  
  for(int i = 0; i < 2; i++)
  {
	printf("Podejscie nr: %d\n", i+1);
	 int j = 0;
	 while( buf[j] != 0)
	 {
		 if( buf[j] >= pierwszy_zakres && buf[j] < drugi_zakres)
		 {
			//printf("%lg\n", buf[j]);
			ilosc_lb_w_przedziale++;
		 }
		 j++;
	 }
	 
	 printf("ilosc liczb w przedziale %lg - %lg: %d\n", pierwszy_zakres, drugi_zakres, ilosc_lb_w_przedziale);
	 pierwszy_zakres += 2*stale_przedzialy; 
	 drugi_zakres += 2*stale_przedzialy;
	 ilosc_lb_w_przedziale = 0;
  }	 


	return 0;
}
