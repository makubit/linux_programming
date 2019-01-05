#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>

void time_to_sleep()
{
    struct timespec time;
    time.tv_sec = 1;
    time.tv_nsec = 0;

    if(nanosleep(&time, NULL) == -1 )
        perror("Nanosleep error\n");
}

char rndByte()
{
    int randomByte = open("/dev/urandom", O_RDONLY);
    char byte[1];
    read(randomByte, byte, 1);

    return byte;
}

void sighandler(int signo)
{


}

//---------------------------------------------------------

int main(int argc, char* argv[])
{
   if(argc != 2)
   {
       perror("You have to pass one parameter\n");
       return 1;
   }

   int  number = strtol(argv[1], NULL, 0);

   //robimy pipe
   int fdp[2];
   if(pipe(fdp) == -1)
   {
       perror("Pipe error\n");
       return 0;
   }
   
   /* Tworzym potomka, który tworzy więcej potomków */
   pid_t ffpid = fork();
   pid_t ccpid;

   if(ffpid == 0) //jeśli dziecko, to tworzymy więcej dzieci
   {
      for(int i = 0; i<number; i++)
      {
          ccpid = fork();

          srand(i+1);
          
          if(ccpid == 0)
          {
              //zawiesza sie na seleccie na fpd[0]; -> read koniec
              fd_set set;
              FD_ZERO(&set);
              FD_SET(fdp[0], &set);

              if(select(FD_SETSIZE, &set, NULL, NULL, NULL) <= 0)//zawiesza sie na czekaniu na akcje na końcu fdp[0]
                  perror("Select error\n");

              //losujemy czas
              time_to_sleep();

              printf("dziecko umiera\n");
            
              exit(0);
          }
      }
          exit(0); //kończy swoje działanie
   }

  //czeka na pośrednie dziecko
   wait(&ffpid);

   //char* byte = rndByte();
   int randomByte = open("/dev/urandom", O_RDONLY);
   char byte[1];
   read(randomByte, byte, 1);

   //wysyamy na pipe
   write(fdp[1], byte, 1);

   //fd_set set;
   //FD_ZERO(&set);
   //FD_SET(fdp[1], &set);

   //if(select(FD_SETSIZE, NULL, &set, NULL, NULL) <= 0)
       //perror("Select error\n");

   //time_to_sleep();




    return 0;
}
