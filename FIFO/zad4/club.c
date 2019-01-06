#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

void display_help()
{
    printf("\nHow to use this program:\n  a.out N:F:T\n  N: int, children number\n  F: float, time stamps\n  T: text, should be in quotes!\n\n");
}

int split_str(char** result, char* str)
{
    //3 result string from parsing
    int i = 0;
    char* temp = strtok(str, ":");

    while(temp)
    {
        *(result + i++) = strdup(temp);
        temp = strtok(0, ":");
    }

    if(i != 3)
        return -1;

    return 0;
}

//-----------------------------------------------------

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        display_help();
        return 1;
    }

    char parameters[100];
    strcpy(parameters, argv[1]);

    /* Buffers for parsing result */
    char n_chld[100]; //integer - > chld number
    char f_time[100]; //time stamps
    char t_text[100]; //text for chanting

    //function for splitting
    char* result[3];
    
    if(split_str(result, parameters) == -1)
    {
        perror("Split str error, exiting...\n");
        display_help();
        return 1;
    }

    printf("%s, %s, %s\n\n", result[0], result[1], result[2]);


    return 0;
}
