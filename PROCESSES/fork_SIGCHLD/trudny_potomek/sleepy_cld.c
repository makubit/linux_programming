//vim: sw=2 sts=2 et :
//program obowiązkowy 2. TRUDNY POMOMEK
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#define NANOSEC 1000000000L

/* Function to generate random signal: SIGSTOP, SIGTSTP or SIGTERM */
int randSignal()
{
	srand(time(0));
	int random = rand() % 3;

	if(random == 0)
	{
		printf("Sending SIGSTOP\nTo continue type in terminal: kill -SIGCONT %d\n",getpid() );
		return SIGSTOP;
	}
	else if(random == 1)
	{
		printf("Sending SIGTSTP\nTo continue type in terminal: kill -SIGCONT %d\n",getpid() );
	   return SIGTSTP;
	}
	else
	{	
		printf("Sending SIGTERM\nBye world!\n");
		return SIGTERM;
	}
}

//------------------------------------------------------

int main(int argc, char* argv[])
{
	/* Chech if there is parameter */
	if(argc != 2)
	{
		printf("Parameter <int> is needed, try again\n");
		return 1;
	}

	/* Create child */
	pid_t pid = fork();
	
	/* Process child */
	if(pid == 0)
	{
		printf("Sending SIGSTOP\nTo continue type in terminal: kill -SIGCONT %d\n", getpid());
		raise(SIGSTOP); /* At first stop, than after proper command in terminal process remaining code */

		char* pEnd = NULL;
		long csec = strtod(argv[1], &pEnd);
		if(*pEnd)
		{
			printf("Parameter must be a number, cannot process...\n");
			return 1;	
		}

		struct timespec time1 =
		{
			.tv_sec = csec / 100,
			.tv_nsec = (csec * NANOSEC/100) % NANOSEC
		};
		
		/* After waking up generate random signal with randSignal() until child is killed permamently */
		while(1)
		{
			int nano = nanosleep(&time1, NULL);
			if(nano)
				printf("Nanosleep returned value != 0\n");

			raise(randSignal());

			printf("Continue...\n");

		}
	}

	return 0;	
}
