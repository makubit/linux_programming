#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define NANOSEC 1000000000L

static void sig_handler(int signo, siginfo_t * sinfo, void* context)
{
	

}

int main(int argc, char* argv[])
{
	int c;
	int xINT = 5;
	float dFLOAT = 1.7724; //pierwiastek z pi

	while((c=getopt(argc, argv, "x:d:")) != -1)
		switch(c)
		{
			case 'x':
				xINT = strtol(optarg, NULL, 0);
				break;
			case 'd':
				dFLOAT = strtod(optarg, NULL);
				break;
			case '?':
				printf("Nieprawidłowo przekazany paramter\n");
				break;
		}

	struct timespec time1;
	time1.tv_sec = (int)dFLOAT/10;
	time1.tv_nsec = (long)(dFLOAT * NANOSEC / 10) % NANOSEC;
	//printf("%d, %d\n", time1.tv_sec, time1.tv_nsec);
	//
	
	struct timespec pomiar1;
	struct timespec pomiar2;

	struct timespec tab[xINT];

	struct sigaction act;
	sigemptyset(&act.sa_mask);
	//sigaddset(&act.sa_mask, SIGUSR1);
	act.sa_sigaction = &sig_handler;
	act.sa_flags = SA_SIGINFO;
	
	sigaction(SIGUSR2, &act, NULL); //funkcja umiżliwiająca 

	sigset_t blockMask;
	sigemptyset(&blockMask);
	sigaddset(&blockMask, SIGUSR1);
	
	sigpockmask(SIG_SETMASK, &blockMask, NULL); //blokowanie sygnału USR1





	int i=0;
	while(i < xINT)
	{

		clock_gettime(CLOCK_REALTIME, &pomiar1);
		//printf("%d, %d\n", pomiar1.tv_sec, pomiar1.tv_nsec);

		nanosleep(&time1, NULL);

		clock_gettime(CLOCK_REALTIME, &pomiar2);

		tab[i].tv_sec = pomiar2.tv_sec - pomiar1.tv_sec;
		tab[i].tv_nsec = pomiar2.tv_nsec - pomiar1.tv_nsec;

	if(tab[i].tv_nsec < 0 && tab[i].tv_sec > 0)
		{
			tab[i].tv_sec -= 1;
			tab[i].tv_nsec += NANOSEC;
		}


		
		i++;
	}

	struct timespec srednia;
	srednia.tv_sec = 0;
	srednia.tv_nsec = 0;
	
	i=0;
	while(i<xINT)
	{
		srednia.tv_sec += tab[i].tv_sec;
 		srednia.tv_nsec += tab[i].tv_nsec;
		i++;		
	}

	srednia.tv_sec /= xINT;
	srednia.tv_nsec /= xINT;

	printf("--> %d sek, %d nsec\n", srednia.tv_sec, srednia.tv_nsec);
	printf("-->powinno byc: %d sek, %d nsec\n", time1.tv_sec, time1.tv_nsec);

	printf("-->%d\n", srednia.tv_nsec - time1.tv_nsec);



	return 0;
}
