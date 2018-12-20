//treść zadania 1
//napisać program, który dostaje 1 parametr -> sciezka do pliku fifo, powinien sprawdzić, czy plik jest fifo. (nie musi)
//Program otwiera fifodo odczytu i czeka, aż go ktoś otworzy, gdy tak sie stanie program przechodzi w tryb nieblokujący
//wykonuje następujące czynności: z pliku odczytuje po 1 bajcie w pętelce i wpisuje do bufora(15 znaków)
//po wypełnieniu wyświetla i zapełnia bufor od nowa
//Jeżeli oczekiwanie na kolejn znak przekroczy 2.75sek to wypisuje na ekran komunikat oraz niepełny bufor jest wypisywany na ekran
//Jeżeli bufor był pusty, to wypisywany jest drugi komunikat
//Jeżeli druga strona zamknie fifo, to nasz program ma znowu wrócić do trybu blokujacego i czekać aż go ktoś znowu otworzy
//programy do pisania będą pisane w bashu (???)
//sleep z floatem działa
//jak napotkamy na end of file w pliku to przechodzimy w tryb blokujący
//MULTIPLEKSACJA ma zapewnic, żeby były odczyty czasu
//poll() funckja
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <time.h>

//------------------------------------------------

int main(int argc, char* argv[])
{
   if(argc != 2)
   {
      perror("One positional parameter is needed\n");
      return 1;
   }

   char* myFifo = argv[1];
   mkfifo(myFifo, 0666);

   /* Open fifo */
   int fd = open(myFifo, O_RDONLY);

   /* Prepare char array */
   char charArray[15];
   int counter = 0;

   while(1)
   {
      char* temp = NULL;
      ssize_t b = read(fd, charArray, 1);
      if(b == 0)
         sleep(1);
      //char temp2 = *temp;
      //charArray[counter] = temp2;
      else{
      counter++;
      if(counter >= 15)
      {
         printf("Zapełnione znaki: %s\n\n", charArray);
         //memcpy(charArray, NULL, 15);
         counter = 0;
      }
      }
      
   }

   return 0;
}
