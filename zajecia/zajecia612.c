#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

static void handler(int sig, siginfo_t *si, void* uc) {
	printf("Caught signal REALTIMEMAX-5\n\n");

	

}

int main(int argc, char* argv[]) 
{
	if(argc>5) {
		printf("Za dużo parametrów...\n\n");
		return 1;
	}

	float tab[argc-1];
	int i = 0;
	while(i != argc-1) {
		tab[i] = strtod(argv[i+1], NULL);
	
		i++;
	}

	struct sigaction sa;
	sa.sa_flags=SA_SIGINFO;
	sa.sa_sigaction = handler;
	sigemptyset(&sa.sa_mask);
	if(sigaction(SIGRTMAX-5 ,&sa , NULL) == -1) {
		printf("Error, exiting...\n");
		return 1;
	}

	struct sigevent sev;
	timer_t timerid;
	struct itimerspec its;
	

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGRTMAX-5;
	sev.sigev_value.sival_ptr = &timerid;

	if(timer_create(CLOCK_MONOTONIC, &sev, &timerid) == -1) {
		printf("Error, exiting...\n");
		return 1;
	}

	/* Start the timer */
	its.it_value.tv_sec = tab[0];
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0; //tutaj wpisujemy interwały, czyli potwarzanie się, w zadaniu ma być tylko ONE SHOT!!!
	its.it_interval.tv_nsec = 0;

	timer_gettime

	if(timer_settime(timerid, 0, &its, NULL) == -1) {
		printf("Error, exiting...\n");
		return 1;
	}

	pause();



	return 0;
}
