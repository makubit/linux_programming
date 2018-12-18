//vim: sw=3 sts=3 et:
//second program to communicate via fifo file
//SENDING PROGRAM
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>

//------------------------------------------------

int main(int argc, char* argv[])
{
   if(argc != 2)
   {
      perror("argument with path to fifo file is needed\n");
      return 1;
   }

   char* myFifoFile = argv[1];
   mkfifo(myFifoFile, 0666);

   /* Open random data file */
   mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
   int randomData = open("/dev/urandom", O_RDONLY, mode);
   if(randomData < 0)
   {
   	perror("urandom error\n");
	return 1;
   }

   /* Open fifo file */
   int fd = open(myFifoFile, O_WRONLY);

   /* Ignore SIGPIPE signal */
   signal(SIGPIPE, SIG_IGN);

   while(1)
   {
      /* Generate random number */
      srand(time(NULL));
      int randomBytes = rand() % 4 + 1;

      char tempTab[randomBytes];
      read(randomData, tempTab, randomBytes); //TODO: Check if success
      printf("%s, %d\n", tempTab, randomBytes);

      write(fd, tempTab, randomBytes);

      struct timespec time;
      time.tv_sec = 0;
      time.tv_nsec = 421052631; //8/19sec

      nanosleep(&time, NULL); //TODO: Check if success

      /* Handle EPIPE error in errno variable */
      if(errno == EPIPE)
      {
         perror("EPIPE ERROR\n");
         return 1;
      }
   }

   close(fd);

   return 0;
}
