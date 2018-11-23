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
	while((c=getopt(argc, argv, "p:c:")) != -1)
		switch(c)
		{
			case 'p':
				break;
			case 'c':
				break;
			break;
		}
	
	

  //
	int idx = optind;
	//printf("%d", optind);
	pid_t pid, pgid = 0;


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
		if( pid == 0) //dziecko
		{
			if(!pgid)
				setpgid(pid, pgid); //pgid można ustawić na 0
			pgid = getpgid(pid); //pgid ustawić w rodzicu i rodzic powinien zmieniać grupę
			printf("%d - pid grupy, %d - pid potomka\n", pgid, pid);
			break;
		}
			
	


		idx++;
	}
}
