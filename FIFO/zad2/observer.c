//vim: sw=3 sts=3 et:
//fist program to communicate via fifo file
//OBSERVING PROGRAM
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>

//----------------------------------------------

int main(int argc, char* argv[])
{
   if(argc != 2)
   {
      perror("Argument with path to fifo file is needed\n");
      return 1;
   }
   
   char* myFifoFile = argv[1];
   mkfifo(myFifoFile, 0666);
   
   int fd = open(myFifoFile, O_RDONLY);
   int cnt = 0; //for ioctl result

   while(cnt < 600)
   {
     int ioctlRes = ioctl(fd, FIONREAD, &cnt);
     if(ioctlRes)
     {
     	perror("IOCTL error\n");
	return 1;
     }

     printf("%d\n", cnt);
     
     struct timespec time;
     time.tv_sec = 1;
     time.tv_nsec = 571428571; //4/7 sec

     nanosleep(&time, NULL);
   }

   close(fd);

   return 0;
}
