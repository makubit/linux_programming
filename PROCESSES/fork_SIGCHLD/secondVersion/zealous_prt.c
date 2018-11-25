//second program
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

//----------------------------------------------------

int main(int argc, char* argv[])
{
	int c;
	float floatVarStr = 0;

	while((c=getopt(argc, argv, "p:c:")) != -1)
		switch(c)
		{
			case 't':
				char* pEnd;
				floatVarStr = strtod(optarg, &pEnd);
				if(*pEnd)
					printf("Could not parse -t argument\n);
				break;
			case '?':
				printf("Wrong usage of parametes, try -c/-p <path_to_program.out>\n");
			break;
		}
}
