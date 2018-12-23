//program do pisania do fifo
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

//------------------------------------

int main(int argc, char* argv[])
{
  if(argc != 2)
  {
     perror("One parameter is needed\n");
     return 1;
  } 

  char* myFifo = argv[1];
  mkfifo(myFifo, 0666);

  int fd = open(myFifo, O_WRONLY);

  while(1)
  {
     write(
  }

   return 0;
}
