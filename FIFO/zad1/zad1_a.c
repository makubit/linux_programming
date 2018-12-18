//vim: sw=3 sts=3 et :
//first program to communitace via FIFO file
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

#define ARRAY_SIZE 32000

//---------------------------------------------------

int main(int argc, char* argv[])
{
   if(argc != 2)
   {
      perror("Argument with path to file is needed\n");
      return 1;
   }

   int fd;

   /* Prepare fifo file */
   char* myFifoFile = argv[1];
   mkfifo(myFifoFile, 0666);

   char myChar = 'A';
   char myCharArray[ARRAY_SIZE];
   memset(myCharArray, 0, sizeof(myCharArray));

   fd = open(myFifoFile, O_WRONLY);

   /* Prepare timespec structures */
   struct timespec time; // for nanosleep
   time.tv_sec = 1;
   time.tv_nsec = 200000000;

   struct timespec before_write;
   struct timespec after_write;

   while(myChar <= 'Z')
   {
      /* Make a table with character */
      for(int i=0; i<ARRAY_SIZE; i++)
      {
         myCharArray[i] = myChar;
      }

      printf("---> Writing to file letter: %c <---\n", myCharArray[0]);
   
      clock_gettime(CLOCK_REALTIME, &before_write);

      /* Write to file */
      ssize_t written = write(fd, myCharArray, sizeof(myCharArray));

      clock_gettime(CLOCK_REALTIME, &after_write);

      nanosleep(&time, NULL);

      /* Print time stamps */
      printf("Time before write: %ldsec %ldnsec\n", before_write.tv_sec, before_write.tv_nsec);
      printf("Time after write: %ldsec %ldnsec\n", after_write.tv_sec, after_write.tv_nsec);

      struct timespec temp;
      temp.tv_sec = after_write.tv_sec - before_write.tv_sec;
      temp.tv_nsec = after_write.tv_nsec - before_write.tv_nsec;

      /* Check if nsec is not negative */
      if((temp.tv_nsec < 0) && (temp.tv_sec > 0))
      {
         temp.tv_sec -= 1;
         temp.tv_nsec += 1000000000;
      }

      /* Summarise */
      printf("Time difference: %ldsec %ldnsec, bytes written: %ld\n\n", temp.tv_sec, temp.tv_nsec, written);

      myChar++;
   }

   close(fd);

   return 0;
}
