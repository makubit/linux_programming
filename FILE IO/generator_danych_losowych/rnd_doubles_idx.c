//program generuje dane za pomocą innych programów, następnie uruchamia potrzebną liczbę współbieżnych z nim procesów, z których każdy wykonuje program dbl_index
//vim: sw=2 sts=2 et :
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


int main(int argc, char* argv[])
{
	int c;
	int nNum = 0;
	int varRange = 1;
	
	while((c=getopt(argc, argv, "b:i:")) != -1)
	switch(c)
	{
		case 'b':
			nNum = strtol(optarg, NULL, 0);
			if(nNum <= 0)
			{
				printf("Error: nNum <= 0\n");
				return 1;
			}
			break;
		case 'i':
			varRange = strtol(optarg, NULL, 0);
		  if(varRange <=0)
		  {
		  	printf("Error: varRange <= 0\n");
			return 1;
		  }	 
		  break;
		case '?':
		  printf("Wrong usage of parameters\n");
		  return 1;
	}

	//sprawdzamy, czy obowiazkowy parametr jest załączony poprawnie
	if(nNum == 0)
	{
		printf("Parameter -b is mandatory!\n");
		return 1;
	}

	//najpierw tworzymy proces potomny, który umozliwi nam wygenerowanie danych typu double
	pid_t childPid = fork();
	int status;

	if(childPid == 0)
	{
			//tutaj się dzieje dziecko
			char p[10];
		 	sprintf(p, "%d", nNum);
			char *argvs[] = {"a.out", "-d", p, "-o", "data", NULL}; //dlaczego przy większych wartościach się zacina? >2, program sam w sobie działa bez tego
			int exe = execvp("../pobieranie_danych_losowych/a.out", argvs);
			if(exe == -1)
							printf("execvp didn't work\n");
			return 0;
	}
	else
	{
		//tutaj się dzieje rodzic
		printf("RODZIC");
	}


	return 0;
}
