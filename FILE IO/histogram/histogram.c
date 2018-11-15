//program wyznacza histogram z danych zawartych w pliku wskazanym parametrem -b

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <float.h> //zeby miec mozliwosc dodac stala DBL_MAX

long int lb_przedzialow = 64;

int main(int argc, char* argv[])
{
  if(argv[1] == NULL)
			 perror("Niewlasciwa sciezka");
  printf("%s\n", argv[1]);

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

  int randomDoubles = open("../zadanie_z_cwiczen3/data", O_RDONLY);
  if(randomDoubles <0)
			 perror("randomDoubles < 0");

  double stale_przedzialy = DBL_MAX/lb_przedzialow;
  double buf[sizeof(randomDoubles)];


  ssize_t randomDoublesSize = read(randomDoubles, &buf, sizeof(double));

  int ilosc_lb_w_przedziale = 0;
  
  for(int i = 0; i< lb_przedzialow; i++)
  {
	 int j = 0;
	 while( read(randomDoubles, (buf+j), sizeof(double)) != -1)
	 {
		 printf("%f\n", buf[j]);
		 j++;
	 }
	 printf("%d\n", j);
	 break;
  }	 


	return 0;
}
