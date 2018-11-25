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

static siginfo_t status;

/* Define main functions to handle queue */
/*void push(siginfo_t status)
{
	struct chldQueueItem new_item = (struct chldQueueItem*)malloc(sizeof(struct chldQueueItem));
	memset(new_item, 0, sizeof(struct chldQueueItem));
	(*new_item).status = status;

	if(queue == NULL)
	{
		queue = (struct chldQueue*)malloc(sizeof(struct chldQueue));
		memset(queue, 0, sizeof(struct chldQueue));

		(*queue).pHead = new_item;
	}
	else
		(*queue).(*pTail).pNext = new_item;

	(*queue).pTail = new_item;
}*/

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


/* Definfe function to handle signal SIGCHLD */
static void sigchld_handler(int signo)
{
		
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

	int pgid = 0;
	int childrenNumber = 0;
	
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
	if(*pEnd)
	{
		printf("Parameter has to be a int number, try again...\n");
		return -1;
	}

	printf("->%d, %d, %f\n", pgid, childrenNumber, floatVar);

	/* First part: Wait for children being synchronized */
	int i = 0;
	pid_t childPid = -1;
	//siginfo_t status;
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
	
	/* Until all children die */
	// --- //
	
	/* Handle SIGCHLD signal */
	if(signal(SIGCHLD, sigchld_handler) == SIG_ERR)
	{
		fprintf(stderr, "Cannot handle signal SIGCHLD\n");
		return -1;
	}
	






	return 0;
	
	
	
}

	
