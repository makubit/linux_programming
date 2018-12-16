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

   printf("%s -> fifo file name\n", myFifoFile);

   char myChar = 'A';
   char myCharArray[ARRAY_SIZE];
   memset(myCharArray, 0, sizeof(myCharArray));

   //mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
   fd = open(myFifoFile, O_WRONLY);

   while(myChar <= 'Z')
   {
      /* Make a table with character */
      for(int i=0; i<ARRAY_SIZE; i++)
      {
         myCharArray[i] = myChar;
      }

      printf("%c...\n\n", myCharArray[ARRAY_SIZE - 1]);
   
      printf("writing to file...\n");
      write(fd, myCharArray, sizeof(myCharArray));

      struct timespec time;
      time.tv_sec = 1;
      time.tv_nsec = 200000000;
      nanosleep(&time, NULL);

      myChar++;
   }

   close(fd);


   return 0;

}
