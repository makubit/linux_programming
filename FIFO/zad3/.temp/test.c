//vim: sw=3 sts=3 et:
//MULTICHANNEL FILTER
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <poll.h>
#include <stropts.h>
#include <sys/select.h>

void displayHelp()
{
    printf("How to use this program:\n  -X <letters to process tr -d>\n  -f <file with data>\n  note: data file should contain string data\n\n");
}

void nosleep()
{
    struct timespec time;
    time.tv_sec = 0;
    time.tv_nsec = 750000000;

    if(nanosleep(&time, NULL) == -1)
        perror("Nanosleep error\n");
}

char* reverseString(char* str)
{
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

    return str;
}

//----------------------------------------

int main(int argc, char* argv[])
{
    int c;
    char characters[11];
    char* fileWithData = NULL;
    while((c = getopt(argc, argv, "X:f:")) != -1)
        switch(c)
        {
            case 'X':
                /* Take only 10 characters from parameter */
                for(int i = 0; i< 10; i++)
                    characters[i] = optarg[i];
                characters[10] = '\0'; //mark end of string
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
        displayHelp();
        return 1;
    }

    if(fileWithData == NULL)
    {
        perror("No file with data to process, exiting...\n");
        displayHelp();
        return 1;
    }

    /* Prepare pipes */
    int fd1[2]; //sends input data from parent to child
    int fd2[2]; //sends edited string from child to parent

    //clear the screen
    printf("\ec");

    /* Main loop for creating children */
    for(int i=0;i<strlen(characters);i++)
    {
        if (pipe(pfd1) == -1)
        {
            printf("pipe create ERROR\n");
            exit(EXIT_FAILURE);
        }
        if (pipe(pfd2) == -1)
        {
            printf("pipe create ERROR\n");
            exit(EXIT_FAILURE);
        }
        if((pids[i]=fork())<0)
        {
            printf("Error in fork\n");
            exit(EXIT_FAILURE);
        }
        else if (pids[i] == 0)
        {
            if(close(pfd1[1])==-1)
            {
                perror("CloseC1\n");
                exit(EXIT_FAILURE);
            }  
            if(close(pfd2[0])==-1)
            {
                perror("CloseC2\n");
                exit(EXIT_FAILURE);
            }  
            if(dup2(pfd1[0],STDIN_FILENO)==-1)
            {
                perror("dup2C1\n");
                exit(EXIT_FAILURE);
            }
            if(dup2(pfd2[1],STDOUT_FILENO)==-1)
            {
                perror("dup2C2\n");
                exit(EXIT_FAILURE);
            }
            //char * arg[]={"TR",&string[i],(char*)NULL};
              //      execv(arg[0],arg);

		
           
        }
        if((f=fork())==-1)
        {
            printf("fork error\n");
            exit(EXIT_FAILURE);
        }
        else if (f==0)
        {
            fd=open(path,O_RDONLY);
           
            if (fd == -1)
            {
                        printf("Can't open a file!\n");
                        exit(EXIT_FAILURE);
                }
            if(close(pfd1[0])==-1)
            {
                perror("CloseP1\n");
                exit(EXIT_FAILURE);
            }  
           
            printf("Potomek %d \n",i+1);
            while(read(fd,&c,sizeof(c))>0)
            {
       
                fds.fd=pfd2[1];
                fds.events=0;
                fds.events |= POLLIN;
                if((pret=poll(&fds,1,timeout))==0)
                {
                    printf("Czekam\n");
       
                }
                write(pfd1[1],&c,sizeof(c));   
                read(pfd2[0],&c,sizeof(c));
            }
            if(close(pfd2[1])==-1)
            {
                perror("CloseP2\n");
                exit(EXIT_FAILURE);
            }
           
            close(fd);
        }
       
 
    }

    wait(NULL); //wait until all children die

    return 0;

}
