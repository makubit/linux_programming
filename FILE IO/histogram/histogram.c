//program wyznacza histogram z danych zawartych w pliku wskazanym parametrem pozycyjnym argv[1]
//parametrem -b możemy określić liczbę przedziałów na którą chcemy podzielić liczbę double -> domyślna wartość 64; warunek: (b>1)
//drukowane są do pliku "result" dane o rozłożeniu liczb w przedziałach
//

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <float.h> //zeby miec mozliwosc dodac stala DBL_MAX
#include <sys/stat.h>
#include <string.h>

int rangeNumber = 2;

int main(int argc, char* argv[])
{
  int c;

  while((c=getopt(argc, argv, "b:")) != -1)
	 switch(c)
	 {
		case 'b':
		  rangeNumber = strtol(optarg, NULL, 0);
		  if(rangeNumber < 0)
			  perror("Wrong range\n");
		  break;

		case '?':
		  printf("Usage Error\n");
		  return -1;
	 }

  int randomDoubles = open(argv[optind], O_RDONLY); //otwieramy plik z paramteru pozycyjnego z danymi
  
  if(randomDoubles <0)
  {
	 perror("randomDoubles error");
	 return 1;
  }

  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  int writeFile = open("result", O_CREAT | O_WRONLY, mode);
  if(writeFile < 0)
  {
  	perror("writeFile error\n");
	return 1;
  }

  double rangeValue = (DBL_MAX/rangeNumber) * 2;

  //liczymy wielkosc pliku
  struct stat st;
  stat(argv[optind], &st);
  ssize_t size = st.st_size;
  int numbersAmount = size/sizeof(double); //ilosc liczb w pliku
  double buffSize[numbersAmount];
  memset(buffSize, 0, numbersAmount);

  ssize_t randomDoublesSize = read(randomDoubles, &buffSize, size);
  if(randomDoublesSize < 0)
  {
  	perror("randomDoublesSize error\n");
	return 1;
  }

  int rangeQuantity = 0;

  double firstRange = -DBL_MAX;
  double secondRange = firstRange + rangeValue;

  //najpierw zapisujemy do pliku ile mamy liczb i na ile przedzialow dzielimy libczę double
  char temp[10];
  sprintf(temp, "%d", numbersAmount);
  char buff[30] = "# cnt ";

  strcat(buff, temp);
  strcat(buff, "\n");
  write(writeFile, buff, sizeof(buff));

  sprintf(temp, "%d", rangeNumber);
  memset(buff, 0, sizeof(buff)/sizeof(char));
  strcpy(buff, "# bins ");
  strcat(buff, temp);
  strcat(buff, "\n");
  write(writeFile, buff, sizeof(buff));

  //zabieramy się za rozkładanie danych w przedziałach, gdzie lb_przedziałów to zadana w parametrze -b liczba przedziałów na które mamy podzielić double'a
  for(int i = 0; i < rangeNumber; i++)
  {
	 memset(buff, 0, sizeof(buff)/sizeof(char));
	 memset(temp, 0, sizeof(temp)/sizeof(char));
	 sprintf(temp, "%d", i);
	 strcat(buff, temp);
	 strcat(buff, ": ");
	 write(writeFile, buff, sizeof(buff));

	 int j = 0;
	 while(buffSize[j])
	 {
		 if( buffSize[j] >= firstRange && buffSize[j] < secondRange)
			rangeQuantity++;
		 j++;
	 }
	 
	 memset(buff, 0, sizeof(buff)/sizeof(char));
	 memset(temp, 0, sizeof(temp)/sizeof(char));
	 sprintf(temp, "%d", rangeQuantity);
	 strcat(buff, temp);
	 strcat(buff, "\n");
	 write(writeFile, buff, sizeof(buff));

	 firstRange += rangeValue; 
	 secondRange += rangeValue;
	 rangeQuantity = 0;
  }

	close(writeFile);
      	close(randomDoubles);	
	return 0;
}
