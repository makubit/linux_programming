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

int main(int argc, char* argv[])
{
	int c;
	//char pathToProgram1[20];
	char* pathToProgram1 = NULL;
	//memset(pathToProgram1, 0, sizeof(pathToProgram1)/sizeof(char));
	char* pathToProgram2 = NULL;

	while((c=getopt(argc, argv, "p:c:")) != -1)
		switch(c)
		{
			case 'p':
				pathToProgram2 = optarg;
				 break;
			case 'c':
				 pathToProgram1 = optarg;
				 //strcpy(pathToProgram1, optarg);
				 printf("%s", pathToProgram1);
				break;
			case '?':
				printf("Wrong usage of parametes, try -c <path_to_program.out>\n");
			break;
		}
	
	//nasz indeks do tworzenia potomków
	int idx = optind;
	//pid kolejnego potomka, pgid całej grupy potomków
	pid_t pid, pgid = 0;
	int childrenNumber = 0;
	float childrenMin = 0;

	//TODO: Rozbić na osobną funckję
	while(argv[idx] != NULL)
	{
		//odczytuje argv[optdind] i go przetwarzam -> tworzę potomka etc
		char* pEnd;
		float floatVar = strtod(argv[idx], &pEnd);
		if(*pEnd != '\0')
		{
			printf("Parametr argv[%d] = %s nie jest floatem i nie będzie procesowany!\n", idx, argv[idx]);
			break;
		}
		//liczymy który to potomek i sprawdzamy, czy jest min
		childrenNumber++;
		if(floatVar < childrenMin || childrenNumber == 1)
			childrenMin = floatVar;

		//jeżeli mamy parametr c to robimy execa
		//jeżeli nie to idziemy dalej, na razie idziemy dalej
		//to jest nasze esle
		pid = fork();
		//pgid można ustawić na 0
		//pgid ustawić w rodzicu i rodzic powinien zmieniać grupę
		if(!pgid)
		{
			setpgid(pid, 0); //zwraca 0, jeżeli się powiodło -> TODO: Sprawdzić, czy się powiodło
			pgid = getpgid(pid);
		}
		else
			setpgid(pid, pgid); //jeśli nie jest pierwszm procesem, to ustawiamy jego grupe na pgid
		printf("%d - pid grupy, %d - pid potomka\n", getpgid(pid), pid);

		if( pid == 0) //dziecko
		{
			//procesujemy jeżeli było obecne -c CZYLI mamy scieżkę do pliku i wywołujemy
			//go z parametrem <float> który coś tam robi sobie
			if(pathToProgram1)
			{
				char* fakeArgv[] = { pathToProgram1, argv[idx], NULL }; //tworzymy argumenty dla naszego
				//execa włącznie z nazwą, która zawsze znajduje się na [0] miejscu
				int exe = execv(pathToProgram1, fakeArgv); //wywołanie execa
				if(exe == -1)
				{
						printf("Nie powiodło się");
						printf("ERRNO: %d\n", errno);
				}

				return 0;
			}

		  //PRZYPADEK KIEDY NIE BYŁO PARAMETRU
			else 
			{
				struct timespec time1, time2;
				time1.tv_sec = floatVar / 100;
				time1.tv_nsec = (long)(floatVar*NANOSEC/100) % NANOSEC;

				nanosleep(&time1, &time2);
				printf("Wyspany!\n");
				
				//drukuje nam jakas randomowa liczbe z zakresu 0-127
				srand(time(0));
				int random = rand() % (127 + 1);
				//printf("->%d\n", random);

				return random;
			}
			return 0; //to chyba nie jest tutaj potrzebne
		
		}
		
		//rodzic nie robi nic, poza inkrementowaniem pętli towrzenia się potomków
		idx++;
	} //KONIEC WHILE'A

	//DOGLĄDANIE POTOMKÓW

	//1. jeśli był podany parametr -p
	if(pathToProgram2 != NULL)
		{
		//potrzebujemy:
		//->PID grupy procesów na str
		char strPgid[10];
		memset(strPgid, 0, sizeof(strPgid)/sizeof(char));
		sprintf(strPgid, "%d", pgid);

		//->ilość utworzonych potomków na str
		char childrenNumberStr[10];
		memset(childrenNumberStr, 0, sizeof(childrenNumberStr)/sizeof(char));	
		sprintf(childrenNumberStr, "%d", childrenNumber);

		//->-t wartość najmniejszego z nich
		char childrenMinStr[10];
		memset(childrenMinStr, 0, sizeof(childrenMinStr)/sizeof(char));	
		sprintf(childrenMinStr, "%f", childrenMin);

		char* fakeArgv[] = { pathToProgram2,"-t", childrenMinStr, childrenNumberStr, strPgid, NULL }; //tworzymy argumenty dla naszego

		int exe = execv(pathToProgram2, fakeArgv); //wywołanie execa
		if(exe == -1)
			printf("Odpalenie programu2 nie powiodło się, errno = %d\n", errno);

		return 0;
		}

	else	//kiedy nie mamy ścieżki do pliku
	{
		pid_t childPid = -1;
		siginfo_t status;

		//definiujemy sobie naszą kolejną strukturę czasu
		struct timespec time1, time2;
		printf("Children min: %f",childrenMin);
		time1.tv_sec = (long)(childrenMin / 2) / 100;
		time1.tv_nsec = (long)((childrenMin/2) * NANOSEC/100) % NANOSEC;
		printf("--%ld, %ld--", time1.tv_sec, time1.tv_nsec);

		//nanosleep(&time1, NULL);
		
		int i = 0;
		while(i < childrenNumber)
		{
			nanosleep(&time1, &time2);
			//sleep(1);

			childPid = waitid(P_ALL, 0, &status, WNOHANG | WEXITED);
			if(childPid != -1)
			{
				if(status.si_pid != 0)
				{
					printf("----------\nWŁAŚNIE UMARŁ POTOMEK\npid zamordowanego potomka: %d\nstatus zamordowanego: %d\nprzyczyna śmierci: %d\n[*] REQUIESCAT IN PACE [*]\n----------\n\n", status.si_pid, status.si_status, status.si_code);
					i++;
				}
				//else
					//fprintf(stderr, "Could not read child's status");
				//i++;
			}
		}
		//stderr
	}

	return 0;
}
