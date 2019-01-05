#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
//#include <singal.h>

int main(int argc, char* argv[])
{
    int c;
    char* f_parameter;
    float time = 1.0; //kwarta dekasekundy
    char* text;

    while((c = getopt(argc, argv, "d:")) != -1)
        switch(c)
        {
            case 'd':
                //f_parameter = optarg;
                time = strtod(optarg, NULL);
                break;
            case '?':
                perror("Wrong usage of parameters\n");
                break;
        }

    strcpy(text, argv[optind]);
    printf("%s\n", text);

    return 0;
}
