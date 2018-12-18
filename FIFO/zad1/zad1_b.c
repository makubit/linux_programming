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
   for(int i = 0; i < ARRAY_SIZE; i++)
   {
      if(tab[i] != tab[0])
      {
         printf("letter: %c and %c on position %d\n", tab[i], tab[0], i);
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

   /* Prepare timespec structures */
   struct timespec time;
   time.tv_sec = 2;
   time.tv_nsec = 600000000;

   struct timespec before_read;
   struct timespec after_read;

   //ssize_t readFifo = 1;

   while((myCharArray1[0] != 'Z'))
   {
      clock_gettime(CLOCK_REALTIME, &before_read);

      /* Read from fifo file */
      ssize_t readFifo = read(fd, myCharArray1, sizeof(myCharArray1));

      clock_gettime(CLOCK_REALTIME, &after_read);

      nanosleep(&time, NULL);

      printf("---> Reading from fifo file letter: %c <---\n", myCharArray1[0]);

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

      /* Print time stamps */
      printf("Time before read: %ldsec %ldnsec\n", before_read.tv_sec, before_read.tv_nsec);
      printf("Time after read: %ldsec %ldnsec\n", after_read.tv_sec, after_read.tv_nsec);

      struct timespec temp;
      temp.tv_sec = after_read.tv_sec - before_read.tv_sec;
      temp.tv_nsec = after_read.tv_nsec - before_read.tv_nsec;

      /* Check if nsec is not negative */
      if((temp.tv_nsec < 0) && (temp.tv_sec > 0))
      {
         temp.tv_sec -= 1;
         temp.tv_nsec += 1000000000;
      }

      /* Summarise */
      printf("Time difference: %ldsec %ldnsec, bytes read: %ld\n\n", temp.tv_sec, temp.tv_nsec, readFifo);

      counter++;
   }

   close(fd);

   return 0;
}
