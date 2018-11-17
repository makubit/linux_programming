#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char* argv[])
{
	int c;
	while((c = getopt(argc, argv, "b:")) != -1)
		switch(c)
		{
			case 'b':
				printf("Zlapalo b!");
				int lb = strtol(optarg, NULL, 0);
				printf("%d\n", lb);
				break;
			case '?':
				break;
		}

	int newFile = open(argv[optind], O_RDONLY);
	//write(newFile, "eloelo", sizeof("eloelo"));
	char buf[20];
	memset(buf, 0, sizeof(buf)/sizeof(char));
	read(newFile, buf, sizeof(buf)/sizeof(char));
	printf("%s", buf);

	return 0;

}
