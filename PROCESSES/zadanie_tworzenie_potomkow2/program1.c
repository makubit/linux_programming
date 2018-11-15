// vim: sw=2 sts=2 et :
//program zajecia 15.11.2018
//
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>


int main(int argc, char* argv[])
{

  if(argc !=2)
  {
    perror("Niewlasciwa lb parametrow\n");
    return 1;
  }

  char* pEnd;

  double lb = strtod(argv[1], &pEnd);
  //printf("%lf\n", lb);
  if(*pEnd != '\0')
    return 1;

  pid_t pid_potomka, pid;
  pid_potomka = fork();
  int status;

  //tutaj wykonuje sie wait + nanosleep

  if(pid_potomka == 0)
  {
    //pierwszy potomek, tutaj tworzymy grupę
    setsid(); //towrzymy grupę
    //pid_t pid;

    for(int i =0; i<20; i++)
    {
      pid = fork();
      //printf("%d. pid=%d\n",i, pid);

      if(pid == 0)
      {
        //printf("Nowy potomek\n");
       //petla nieskonczona
      }

      //return -1;

    }

    return -1; //musi byc tutaj return, bo trafia do rodzica
  }

  pid_potomka = wait(&status);
  printf("Pid potomka: %d\n", pid_potomka);


//killpg setpgid
  if(pid == 0)
  {
    //printf("Child process\n");
    sleep(2);
    pid_t waiting = wait(&status);
    //printf("%d\n", waiting);
    return 1; 
  }

  for(int i = 0; i<20; i++)
  {
    //printf("Parent process\n", pid);    
    //sleep(lb);
    pid_t waiting = wait(&status);
    printf("%d\n", waiting);
  }






  return 0;
}
