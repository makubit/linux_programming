//vim: sw=3 sts=3 et:
//MULTICHANNEL FILTER
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

//----------------------------------------

int main(int argc, char* argv[])
{
   int c;
   char characters[9];
   char* fileWithData = NULL;
   while((c = getopt(argc, argv, "X:f:")) != -1)
      switch(c)
      {
         case 'X':
            //strcpy(characters, optarg);
            /*if(!optarg[10])
            {
               printf("String is too long -> max 10 characters\n");
               return 1;
            }*/
            //strcpy(characters, optarg);
            for(int i = 0; i< 10; i++)
               characters[i] = optarg[i];
            printf("%s, %c\n", characters, optarg[10]);
            break;
         case 'f':
            fileWithData = optarg;
            break;
         case '?':
            perror("Wrong usage of parameters\n");
         break;
      }

   if(characters == NULL)
   {
      perror("Parameter -X is mandatory, exiting...\n");
      return 1;
   }

   return 0;
}
