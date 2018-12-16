//vim: sw=3 sts=3 et :
//second program to communicate via FIFO file

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

#define ARRAY_SIZE 16000

//--------------------------------------------

int sameLetters(char* tab)
{
   for(int i = 0; i< strlen(tab); i++)
   {
      if(tab[i] != tab[0])
      {
         printf("letter: %c and %c\n", tab[i], tab[0]);
         return 1;
      }
   }

   return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2)
  {
      perror("Argument with path to file is needed\n");
      return 1;
  }

   int fd; //file descriptior
   int counter = 1; //for the second block

   char* myFifoFile = argv[1];
   mkfifo(myFifoFile, 0666);

   char myCharArray1[ARRAY_SIZE];
   char myCharArray2[ARRAY_SIZE];
   memset(myCharArray1, 0, sizeof(myCharArray1));
   memset(myCharArray2, 0, sizeof(myCharArray2));

   fd = open(myFifoFile, O_RDONLY);

   while(fd != EOF)
   {
      read(fd, myCharArray1, sizeof(myCharArray1));

      printf("Reading from fifo: \n%c\n\n", myCharArray1[0]);

      /* Check of there are the same letters in block */
      if(sameLetters(myCharArray1))
         perror("There are not the same letters...\n");

      /* Chceck if previus block is the same */
      if(counter % 2)
         strcpy(myCharArray2, myCharArray1);
      else
      {
         int same = strcmp(myCharArray1, myCharArray2);
         if(!same)
            perror("Previous block is not the same\n");
      }

      struct timespec time;
      time.tv_sec = 2;
      time.tv_nsec = 600000000;
      nanosleep(&time, NULL);

      counter++;
   }

   close(fd);

   return 0;
}
