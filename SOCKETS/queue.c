#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <signal.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <fcntl.h>
#include <arpa/inet.h>

/***********************************************************
 * CUSTOMERS QUEUE
 **********************************************************/

struct customers_q
{
   int fd; //przechowujemy deskryptor
   struct customers_q* next; /* pointer to next item */
};

/* Create our queue */
struct customers_q* queue = NULL;
struct customers_q* tempItem = NULL;
struct customers_q* first  = NULL;

/* Variables and counters to control signals */
static volatile int childrenNumber = 0;
static volatile int childrenNumberDeadOrAlive = 0;
int pgid;

/* Functions to manage queue */
void push(int fd)
{
   tempItem = (struct customers_q*)malloc(sizeof(struct customers_q));
   (*tempItem).fd = status;
   if(queue == NULL)
           first = tempItem;
   else
           (*queue).next = tempItem;
   queue = tempItem;
}

void pop()
{
   if(first != NULL)
   {
      tempItem = (*first).next;

      //free(first);
      first = tempItem;
      if(first == NULL)
              queue = NULL;
   }
   else
           printf("Customers queue is empty, cannot pop...\n");
}

int main(int argc, char* argv[])
{



  return 0;
}
