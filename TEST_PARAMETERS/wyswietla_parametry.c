#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	int i=0;
	for(i;i<argc;i++)
	{
		printf("Parametr numer: %s \n ", argv[i]);
		sleep(1);
	}	
}
