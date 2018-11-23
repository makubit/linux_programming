//vim: sw=2 sts=2 et :

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char* argv[])
{
	int c;
	char* pathToProgram;

	while((c=getopt(argc, argv, "p:c:")) != -1)
		switch(c)
		{
			case 'p':
				 break;
			case 'c':
				 pathToProgram = optarg;
				break;
			case '?':
				printf("Wrong usage of parametes, try -c <path_to_program.out>\n");
			break;
		}
	
	//nasz indeks do tworzenia potomków
	int idx = optind;
	//pid kolejnego potomka, pgid całej grupy potomków
	pid_t pid, pgid = 0;

	//TODO: Rozbić na osobną funckję
	while(argv[idx] != NULL)
	{
		//odczytuje argv[optdind] i go przetwarzam -> tworzę potomka etc
		char* pEnd;
		float var = strtod(argv[idx], &pEnd);
		if(*pEnd != '\0')
		{
			printf("Parametr argv[%d] = %s nie jest floatem i nie będzie procesowany!\n", idx, argv[idx]);
			break;
		}

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
			if(pathToProgram)
			{
				char* fakeArgv[] = { pathToProgram, argv[idx], NULL }; //tworzymy argumenty dla naszego
				//execa włącznie z nazwą, która zawsze znajduje się na [0] miejscu
				int exe = execvp(pathToProgram, fakeArgv); //wywołanie execa

				return 0;
			}

		  //PRZYPADEK KIEDY NIE BYŁO PARAMETRU
			
			return 0;
		}
			
	


		idx++;
	}
}
