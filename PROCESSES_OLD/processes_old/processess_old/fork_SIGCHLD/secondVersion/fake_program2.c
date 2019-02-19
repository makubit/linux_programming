//fake program2 for testing 

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[])
{
	int c;
	char* parameterT;
	while((c=getopt(argc, argv, "t:")) != -1)
		switch(c)
		{
			case 't':
				parameterT = optarg;
	 			break;
			case '?':
				printf("Wrong usage of parameter -t\n");		
		}
	printf("Działa fake2.out\n");
	printf("Podano parametry:\n-t %s\nPozycyjne: %s, %s\n", parameterT, argv[3], argv[4]);

	return 0;
}
