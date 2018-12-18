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

#define NANOSEC 10000000000L

/* Define child Queue*/
struct chldQueue
{
   siginfo_t status;
   struct chldQueue* next; /* pointer to next item */
};

/* Create our queue */
struct chldQueue* queue = NULL;
struct chldQueue* newItem = NULL;
struct chldQueue* first  = NULL;

/* Variables and counters to control signals */
static volatile int childrenNumber = 0;
static volatile int childrenNumberDeadOrAlive = 0;
int pgid;

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
      printf("\n\n--------------------\nCHILD HAS JUST DIEDED\nit's PID: %d\nstatus: %d\ncause of death: %d\n[*] REQUIESCAT IN PACE [*]\n--------------------\n\n", first->status.si_pid, first->status.si_status, first->status.si_code);
      
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
   while(!waitid(P_PGID, pgid, &status2, WNOHANG | WCONTINUED | WEXITED | WSTOPPED)) /* Wait works until there is no more signals incoming */
   {
      if((status2.si_code == CLD_KILLED) || (status2.si_code == CLD_EXITED))
      {
         push(status2); /* Push new obituary when is dead */
         printf("\nEND IS SOON FOR CHILD NUMBER . . . %d\n", status2.si_pid);

         childrenNumber--;
         childrenNumberDeadOrAlive--;
      }

      else if(status2.si_code == CLD_STOPPED)
      {
        childrenNumberDeadOrAlive--;
        printf("\nJust stopping signal no %d \n", status2.si_pid);

      }

      else if(status2.si_code == CLD_CONTINUED)
      {
         childrenNumberDeadOrAlive++;
         printf("\nWaking up child no %d\n", status2.si_pid);

      }
      
      else 
        break;

     }
 }

//----------------------------------------------------
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
   childrenNumberDeadOrAlive = 0; /* Incoming children are sleeping */
   if(*pEnd)
   {
      printf("Parameter has to be a int number, try again...\n");
      return -1;
   }

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
            i++;
         else if(status.si_pid < 0)
           perror("Cannot synchronize child\n");
      }
   }

   /* Create sigaction structure to monitor incoming signals */
   struct sigaction act;
   sigemptyset(&act.sa_mask);
   act.sa_sigaction = &sigchld_handler;
   act.sa_flags = SA_SIGINFO;

   /* Handle SIGCHLD signal */
   if(sigaction(SIGCHLD, &act, NULL) < 0)
   {
     perror("Cannot handle SIGCHLD...\n");
     return -1;
   }

   /* Wake up whole group of children */
   int sendContinue = killpg(pgid, SIGCONT);
   if(sendContinue)
   {
      printf("Cannot wake up children\n");
      return 1;
   }

   /*
    *    Until all children die
    */


   while(childrenNumber > 0)
   { 
      /* Passive waiting */
      pause();

      /* Print which Child has died */
      while(queue)
         pop();

      /* If no more awaken BUT still alive */
      if((childrenNumberDeadOrAlive == 0) && (childrenNumber > 0))
      {
         printf("\nAll children are sleeping right now...\n");
        
         struct timespec time1;
         double temp = floatVar * (1/(M_PI*M_PI));
         time1.tv_sec = (long)temp;
         time1.tv_nsec = (long)(temp * NANOSEC) % NANOSEC;

         nanosleep(&time1, NULL);

         /* Wake up all children */
         int sendContinue = killpg(pgid, SIGCONT);
         if(sendContinue)
            printf("Something went wrong (sending SIGCONT)\n");
      }

      /* Print counters */
      printf("\n>>COUNTERS:\n>>children alive: [%d]\tchildren sleeping: [%d]\n", childrenNumber, childrenNumberDeadOrAlive);

   }

   return 0; 
   
}

   
