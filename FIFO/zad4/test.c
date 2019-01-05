#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    char str[100];
    strcpy(str,  "Moj string do odwrocenia");

    int i = 0;
    int j = strlen(str) - 1;

    char temp;
    while(i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;

        i++;
        j--;
    }

    printf("%s\n", str);

    return 0;
}
