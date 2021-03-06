//MAIN PROGRAM
//vim: sw=2 sts=2 et :

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

#define NANOSEC 1000000000L

//----------------------------------------------------

int main(int argc, char* argv[])
{
	int c;
	char* pathToProgram1 = NULL;
	char* pathToProgram2 = NULL;

	while((c=getopt(argc, argv, "p:c:")) != -1)
		switch(c)
		{
			case 'p':
				pathToProgram2 = optarg;
				break;
			case 'c':
				pathToProgram1 = optarg;
				break;
			case '?':
				printf("Wrong usage of parametes, try -c/-p <path_to_program.out>\n");
			break;
		}
	
	int idx = optind; /* Index for generating children */
	pid_t pid, pgid = 0; /* pid -> for child, pgid -> for group */
	int childrenNumber = 0;
	float childMin = 0;

	/* Main loop for generating children */
	while(argv[idx] != NULL)
	{
		/* Check if float value in parameter was valid */
		char* pEnd;
		float floatVar = strtod(argv[idx], &pEnd);
		if(*pEnd != '\0')
		{
			printf("Parameter argv[%d] = %s is not float, try again\n", idx, argv[idx]);
			break;
		}

		/* Count number of children */
		childrenNumber++;

		/* Set min value */
		if(floatVar < childMin || childrenNumber == 1)
			childMin = floatVar;

		/* Make a child */
		pid = fork();

		/* Set group id if it was not set previously, otherwise get existing group id */
		if(!pgid)
		{
			int setP = setpgid(pid, 0);
			if(setP == -1)
				printf("Setpgid error\n");

			pgid = getpgid(pid);
		}
		else
		{
			int setP = setpgid(pid, pgid);
			if(setP == -1)
				printf("Setpgid error\n");
		}
		
		/* Process if it is child */
		if(pid == 0) 
		{
			/* Process if -c parameter was passed */
			if(pathToProgram1)
			{
				char* fakeArgv[] = { pathToProgram1, argv[idx], NULL }; /* Args for first program */
				int exe = execv(pathToProgram1, fakeArgv); /* Execute program */
				if(exe == -1)
					printf("Error in executing file\nERRNO: %d\n", errno);

				return 0;
			}

		  	/* Process if there was no -c parameter */
			else 
			{
				struct timespec time1;
				time1.tv_sec = floatVar / 100;
				time1.tv_nsec = (long)(floatVar * NANOSEC / 100) % NANOSEC;

				int nano = nanosleep(&time1, NULL);
				if(nano)
					printf("Can't sleep properly...\n");
				
				/* Generate random number in range <0, 127> */
				srand(time(0));
				int random = rand() % (127 + 1);

				return random;
			}
		}
		
		idx++;
	} /* End of while loop */
	
	/*
	*   Watch after children 
	*/
 
	/* Process if -p parameter was passed */
	if(pathToProgram2 != NULL)
		{
		/* Convert numbers to string */
		char strPgid[10]; /* Group id */
		memset(strPgid, 0, sizeof(strPgid)/sizeof(char));
		sprintf(strPgid, "%d", pgid);

		char childrenNumberStr[10]; /* Number of children */
		memset(childrenNumberStr, 0, sizeof(childrenNumberStr)/sizeof(char));	
		sprintf(childrenNumberStr, "%d", childrenNumber);

		char childMinStr[10]; /* Min float passed as a parameter */
		memset(childMinStr, 0, sizeof(childMinStr)/sizeof(char));	
		sprintf(childMinStr, "%f", childMin);

		char* fakeArgv[] = { pathToProgram2,"-t", childMinStr, strPgid, childrenNumberStr, NULL }; /* Make args for second program */

		int exe = execv(pathToProgram2, fakeArgv); /* Execure second program */
		if(exe == -1)
			printf("Error in executing file\nERRNO = %d\n", errno);

		return 0;
		}

	/* Process if there was no -c parameter */
	else
	{
		pid_t childPid = -1;
		siginfo_t status;

		struct timespec time1;
		time1.tv_sec = (long)(childMin / 2) / 100;
		time1.tv_nsec = (long)((childMin/2) * NANOSEC/100) % NANOSEC;
		
		/* Loop for waiting for children's death */
		int i = 0;
		while(i < childrenNumber)
		{
			int nano = nanosleep(&time1, NULL);
			if(nano)
				printf("Can't sleep properly...\n");

			childPid = waitid(P_PGID, pgid, &status, WNOHANG | WEXITED);
			if(childPid != -1)
			{
				if(status.si_pid > 0)
				{
					printf("\n----------\nCHILD HAS JUST DIEDED\nit's PID: %d\nstatus: %d\ncause of death: %d\n[*] REQUIESCAT IN PACE [*]\n----------\n\n", status.si_pid, status.si_status, status.si_code);
					i++;
				}
				else if(errno == ECHILD) //(status.si_pid < 0) /* Exclude processes that are not children of calling process */
					fprintf(stderr, "Could not read child's status");
			}
		}
	}

	return 0;
}
