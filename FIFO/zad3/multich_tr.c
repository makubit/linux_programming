//vim: sw=3 sts=3 et:
//MULTICHANNEL FILTER
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>

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

   /* Prepare pipes */
   int fd1[2]; //sends input data from parent to child
   int fd2[2]; //sends edited string from child to parent

   if(pipe(fd1) == -1)
   {
      perror("Pipe failed, exiting...\n");
      return 1;
   }

   if(pipe(fd2) == -1)
   {
      perror("Pipe failed, exiting...\n");
      return 1;
   }

   //make a child which will execute tr -d 'letter'
   pid_t pid = fork();
   if(pid < 0)
   {
      perror("Child failed\n");
      return 1;
   }

   if(pid == 0) //child
   {
      close(fd1[1]); //writing end

      /* Read string using first pipe */
      char buff[100];
      read(fd1[0], buff, 100);
      printf("%s--> buff inside child\n", buff);

      //NEXT STEP: execute tr -d !!!!!!!!!!
      //char* arguments[] = {"./tr.out", "a", buff, NULL};
      //execv("./tr.out", arguments);

      close(fd1[0]);
      close(fd2[0]);

      //char* buff2 = "Nowy buff";
      /* Write result to pipe */
      write(fd2[1], "nowy buff", 100);
      close(fd2[1]);

      return 0;
   }
   else //parent
   {
      close(fd1[0]);

      char buff[100] = "Random Dataaaaaeeee eee";

      /* Write string to process in child */
      write(fd1[1], buff, 100);
      close(fd1[1]);

      printf("Waiting for child to process...\n");
      int status;
      wait(&status); // wait for child to process this string

      close(fd2[1]);

      /* Read processed string from second pipe */
      char buff2[100];
      read(fd2[0], buff2, 100);
      printf("PROCESSED STRING RESULT: %s\n\n", buff2);
      
      close(fd2[0]);
   }

   return 0;
}
