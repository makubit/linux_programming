//simple tr -d program to delete specified letter from string
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

//--------------------------------------------

int main(int argc, char* argv[])
{
  if(argc != 3)
  {
     perror("Positional parameter with string and with letter to cut are needed\n");
     return 1;
  }

  char* stringToProcess = argv[2];
  char* letter = argv[1];
  char l = *letter;

  int i, j;
  for(i = 0; i<strlen(stringToProcess)+1; i++)
  {
      if(stringToProcess[i] == l)
      {
         for(j = i; j<strlen(stringToProcess)+1; j++)
         {
            stringToProcess[j] = stringToProcess[j+1];
         }
         stringToProcess[j] = '\0';
      }

  }

  printf("Result: %s\n", stringToProcess);

   return 0;
}
