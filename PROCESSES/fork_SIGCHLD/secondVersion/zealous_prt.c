// vim: et sts=2 sw=2
//second program
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <math.h>

/* Define flags */
#define NOMORECHLD FALSE

/* Define child Queue*/
struct chldQueue
{
   siginfo_t status;
   struct chldQueue* next; /* pointer to next item */
};

/*struct chldQueue
{
   struct chldQueueItem pHead;
   struct chldQueueItem pTail;
};*/

//static struct chldQueue queue;

/* Create our queue */
struct chldQueue* queue = NULL;
struct chldQueue* newItem = NULL;
struct chldQueue* first  = NULL;

static volatile int childrenNumber = 0;
static volatile int childrenNumberDeadOrAlive = 0;
int pgid;
/* Flag to set when there are no more alive children */

/* Functions to manage queue */
void push(siginfo_t status)
{
   newItem = (struct chldQueue*)malloc(sizeof(struct chldQueue));
   (*newItem).status = status;
   if(queue == NULL)
           first = newItem;
   else
           (*queue).next = newItem;
   queue = newItem;
}

void pop()
{
   if(first != NULL)
   {
      newItem = (*first).next;

      /* Print obituary */
      printf("\n----------\nCHILD HAS JUST DIEDED\nit's PID: %d\nstatus: %d\ncause of death: %d\n[*] REQUIESCAT IN PACE [*]\n\n", first->status.si_pid, first->status.si_status, first->status.si_code);
      
      free(first);
      first = newItem;
      if(first == NULL)
              queue = NULL;
   }
   else
           printf("Queue is empty, cannot pop...\n");
}


/* Definfe function to handle signal SIGCHLD */
static void sigchld_handler(int signo, siginfo_t* status, void* context)
{
   siginfo_t status2;
   pid_t childPid;
   printf("--->%d<----%d::%d\n\n%d\n\n",status->si_pid, getpgid(status->si_pid), pgid, status->si_code);
     while(!waitid(P_PGID, pgid, &status2, WNOHANG)) /* waitid() i używamy statusu z waitID to to się zmienia, a to w parametrze jest stałe */
   {
      printf("Obsługa sygnału góra\t");
      if(status2.si_code == CLD_KILLED)
      {
         push(status2); /* Push new obituary when is dead */ //???????????????????
         printf("--->------>Jestem w obsłusze syngału!\nLiczba dzieci: %d\n pid: %d\n", childrenNumber, status2.si_pid);

         childrenNumber--;
         childrenNumberDeadOrAlive--;
      }

      else if(status2.si_code == CLD_STOPPED)
      {
        childrenNumberDeadOrAlive--;
        printf("Obsługa sygnału stopped\t");

      }

      else if(status2.si_code == CLD_CONTINUED)
      {
         childrenNumberDeadOrAlive++;
         printf("Obsługa sygnału continued\t%d", childrenNumberDeadOrAlive);

      }

      else if(childrenNumber == 0)
              printf("Nie ma więcej żywych dzieci\n");

   }     
}

//----------------------------------------------------
int main(int argc, char* argv[])
{
   int c;
   float floatVar = M_E; /* Default value for float parameter is e */
   char* pEnd = NULL; /* For checking validity of parameters */

   while((c=getopt(argc, argv, "t:")) != -1)
      switch(c)
      {
         case 't':
            floatVar = strtod(optarg, &pEnd);
            if(*pEnd)
               printf("Could not parse -t argument\n");
            break;
         case '?':
            printf("Wrong usage of parametes, try -t <float>\n");
            break;
      }

   //int pgid = 0;
   //int childrenNumber = 0;
   
   /* Check if there are two positional parameters */
   if(!(argv[optind] || argv[optind+1]))
   {
      printf("There has to be two positional parameters with type <int>\n");
      return -1;
   }

   /* Check if parameters were passed correctly */    
   pgid = strtol(argv[optind], &pEnd, 0);
   if(*pEnd)
   {
      printf("Parameter has to be a int number, try again...\n");
      return -1;
   }

   childrenNumber = strtol(argv[optind+1], &pEnd, 0);
   childrenNumberDeadOrAlive = childrenNumber;
   if(*pEnd)
   {
      printf("Parameter has to be a int number, try again...\n");
      return -1;
   }

   printf("->%d, %d, %f\n", pgid, childrenNumber, floatVar);

   /* First part: Wait for children being synchronized */
   int i = 0;
   pid_t childPid = -1;
   siginfo_t status;
   while(i < childrenNumber)
   {
      childPid = waitid(P_PGID, pgid, &status, WNOHANG | WSTOPPED | WEXITED); /* Waiting for stopped or killed children */
      if(childPid != -1)
      {
         if(status.si_pid > 0)
         {
            printf("Synchronizuję potomka: %d\n", status.si_pid);
            i++;
         } //TODO: if cannot read status -> error
      }
   }

   /* Create sigaction structure to monitor incoming signals */
   struct sigaction act;
   //act.sa_handler = sigchld_handler;
   //!!
   sigemptyset(&act.sa_mask);
   //act.sa_handler = sigchld_handler;
   act.sa_sigaction = &sigchld_handler;
   act.sa_flags = SA_SIGINFO;
   //sigset_t emptyMask, blockMask;

   /* Handle SIGCHLD signal */
   if(sigaction(SIGCHLD, &act, NULL) < 0)
   {
     perror("reg.signal");
     //fprintf(stderr, "Cannot handle signal SIGCHLD\n");
     return -1;
   }
   //sigemptyset(&blockMask);
   //sigaddset(&blockMask, SIGCHLD);
   //sigprocmask(SIG_SETMASK, &blockMask, NULL);

   /* Wake up whole group of children */
   int sendContinue = killpg(pgid, SIGCONT);
   sleep(5);
   if(sendContinue)
      printf("Something went wrong / Sending SIGCONT\n");

   /* Until all children die */
   while(childrenNumber > 0)
   { 
      /* Passive waiting */
      pause();

      /* Print which Child has died -> next loop */
      while(queue)
         pop();

      /* if no more alive and -> managed by while loop */
      /* if no more active chld -> childrenNumber == 0 -> werid nanosleep, then wake up chld */
      if((childrenNumberDeadOrAlive == 0) && (childrenNumber > 0))
      {
         printf("!!!!!!!!!!!Sleeping...\n");
         sleep(2);

         /* Wake up all children */
         int sendContinue = killpg(pgid, SIGCONT);
         if(sendContinue)
            printf("Something went wrong / Sending SIGCONT\n");
      }

      /* if chl > 0 continue while loop */ 
      printf("Waiting....\n");
      //sleep(5);

      printf("%d<<>>%d\n\n", childrenNumber, childrenNumberDeadOrAlive);
   }






   return 0;
   
   
   
}

   
