//vim: sw=3 sts=3 et :
//second program to communicate via FIFO file

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

//--------------------------------------------

int main(int argc, char* argv[])
{
  if(argc != 2)
  {
      perror("Argument with path to file is needed\n");
      return 1;
  }

   int fd;

   char* myFifoFile = argv[1];
   mkfifo(myFifoFile, 0666);

   char myCharArray[16000];
   memset(myCharArray, 0, sizeof(myCharArray));

   while(1)
   {
      fd = open(myFifoFile, O_RDONLY);
      read(fd, myCharArray, sizeof(myCharArray));
      close(fd);

      printf("Reading from fifo: \n%s\n\n", myCharArray);

      struct timespec time;
      time.tv_sec = 1;
      time.tv_nsec = 600000000;
      nanosleep(&time, NULL);
   }



}
