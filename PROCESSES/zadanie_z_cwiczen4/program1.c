//cwiczenie z forkowaniem innego programu/procesu
//oraz ...
//vim: sts=2 sw=2 et:

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#define N 1

int main(int argc, char* argv[])
{
//forkujemy
//potem wykonuje sie inna czesc programu
//

	pid_t pid;
	pid = fork();

//tworzymy dwa procesy -> child bedzie mial pid 0, parent będzie miał >0

	if(pid == 0)
		ChildProcess();
	else
		ParentProcess();


	

	printf("Dalsza czesc programu");	

	return 0;
}
