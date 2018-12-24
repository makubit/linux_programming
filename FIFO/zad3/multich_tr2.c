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

//----------------------------------------

int main(int argc, char* argv[])
{
    //printf("%d, %d, %d, %d\n\n", POLLERR, POLLHUP, POLLNVAL);
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

                //printf("%s, %c\n", characters, optarg[9]);
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
    //int fdData;

    char buff2[100];
    int fd_data = open(fileWithData, O_RDONLY);
    read(fd_data, buff2, sizeof(buff2));

    for(int i = 0; i<strlen(characters); i++)
    {
//        int fd_data = open(fileWithData, O_RDONLY);

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

        //TODO: while for children
        printf("NOW PROCESSING CHILD: %c\n", characters[i]);

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
            int fd_tr1[2];
            pipe(fd_tr1);
            int fd_tr2[2];
            pipe(fd_tr2);

            pid_t chld_pid = fork();
            if(chld_pid == 0) //child execute tr -d
            {
                /* Close all unused descriptors, than replace fd_tr1[1] with stdin and fd_tr2[0] with stdout */
                close(fd_tr1[1]);
                close(fd_tr2[0]);

                dup2(fd_tr1[0], 0); //replace stdin with input end of pipe
                dup2(fd_tr2[1], 1); //replace stdout with output end of pipe

                close(fd_tr1[0]);
                close(fd_tr2[1]);

                char c[2] = { characters[i], '\0'};
                execl("/usr/bin/tr", "/usr/bin/tr", "-d", c, NULL);

                exit(0);
            }

            else //parent writes and reads from pipes
            {
                //read(fd_data, buff, 100);
                printf("PARENT CHILD %s", buff2);
                close(fd_tr1[0]);
                close(fd_tr2[1]);

                write(fd_tr1[1], buff, sizeof(buff));
                close(fd_tr1[1]);

                read(fd_tr2[0], buff2, sizeof(buff2));
                close(fd_tr2[0]);

            }

            close(fd1[0]);
            close(fd2[0]);

            /* Write result to pipe */
            write(fd2[1], buff2, 100);
            close(fd2[1]);

            exit(0);
        }

        else //parent
        {
            close(fd1[0]);

            //char buff[100];

            /* Write string to process in child */
            write(fd1[1], buff2, 100);
            close(fd1[1]);

            printf("Waiting for child to process...\n");

            struct pollfd pollfds;
            pollfds.fd = fd2[1];
            pollfds.events = POLLOUT;

            poll(&pollfds, 1, 500);	

            printf("events: %d, revents: %d\n\n",pollfds.events, pollfds.revents);

            close(fd2[1]);

            /* Read processed string from second pipe */
            //char buff2[100];
            read(fd2[0], buff2, 100);
            printf("PROCESSED STRING RESULT: %s\n\n", buff2);

            close(fd2[0]);
        }
    }
    return 0;
}
