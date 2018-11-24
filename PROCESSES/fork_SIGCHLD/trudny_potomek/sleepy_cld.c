//vim: sw=2 sts=2 et :
//program na zaliczenie 2. TRUDNY POMOMEK
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

int randSignal()
{
	srand(time(0));
	int random = rand() % 3;

	if(random == 0)
	{
		printf("Wysyłam sigstop\nAby kontynuować wpisz w terminalu: kill -SIGCONT %d\n",getpid() );
		return SIGSTOP;
	}
	else if(random == 1)
	{
		printf("wysyłam sigtstp\nAby kontynuować wpisz w terminalu: kill -SIGCONT %d\n",getpid() );
	   return SIGTSTP;
	}
	else
	{	
		printf("wysyłam sigterm\n");
		return SIGTERM;
	}
}

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Niezbędny parametr typu int do uśpienia programu!\n");
		return 1;
	}

	printf("Wysyłam sigstop\nAby kontynuować wpisz w terminalu: kill -SIGCONT %d\n",getpid() );
	raise(SIGSTOP);

	char* pEnd = NULL;
	long csec = strtod(argv[1], &pEnd);
	if(*pEnd)
	{
		printf("Parametr pozycyjny musi być liczbą, nie mogę dalej kontynuować pracy...\n");
	return 1;	
	}

	struct timespec time1 =
	{
		.tv_sec = csec / NANOSEC,
		.tv_nsec = (csec * NANOSEC) % NANOSEC
	};

	while(1)
	{
		nanosleep(&time1, NULL); //sprawdzić poprawność działania

		raise(randSignal());

		printf("Continue...\n");

	}
	return 0;	
}
