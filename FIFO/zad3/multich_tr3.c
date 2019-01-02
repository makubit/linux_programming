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

void nosleep()
{
    struct timespec time;
    time.tv_sec = 0;
    time.tv_nsec = 750000000;

    if(nanosleep(&time, NULL) == -1)
        perror("Nanosleep error\n");
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
        return 1;
    }

    /* Prepare pipes */
    int fd1[2]; //sends input data from parent to child
    int fd2[2]; //sends edited string from child to parent

    char buff2[1];
    //int fd_data = open(fileWithData, O_RDONLY);

    //SYSTEM CALL TEST
    //system("cat < fifo");
    //
    //
    //
    /* Flags for non-blocking reading and writing */
    //typedef struct fd_set set_flags1, set_flags2;

    //int flags1, flags2;

    /* Read data to process in blocks of 100 bytes*/
    //read(fd_data, buff2, sizeof(buff2));
    //

    //TODO: in final stage read will be inside loop -> we have to read all data, not only 100bytes
    //probably loop with reading should be inside parent, before he sends data to children to parse

    for(int i = 0; i<strlen(characters); i++)
    {
        if(pipe(fd1) == -1)
        {
            perror("Pipe fd1 failed, exiting...\n");
            return 1;
        }

        if(pipe(fd2) == -1)
        {
            perror("Pipe fd2 failed, exiting...\n");
            return 1;
        }

        /* Set nonblocking */
        //FD_SET(fd1[0], set_flags1);
        /*flags1 = fcntl(fd1[0], F_GETFL);
          if(flags1 == -1)
          perror("fcntl flags error\n");

        //FD_SET(fd2[0], &set_flags2);
        flags2 = fcntl(fd2[0], F_GETFL);
        if(flags2 == -1)
        perror("fcntl flags error\n");

        flags1 |= O_NONBLOCK;
        flags2 |= O_NONBLOCK;

        int check_fcntl = fcntl(fd1[0], F_SETFL, flags1);
        //check
        check_fcntl = fcntl(fd2[0], F_SETFL, flags2);
        //check

        flags1 = fcntl(fd1[1], F_GETFL);
        flags2 = fcntl(fd2[1], F_GETFL);
        //check
        //

        flags1 |= O_NONBLOCK;
        flags2 |= O_NONBLOCK;

        check_fcntl = fcntl(fd1[1], F_SETFL, flags1);
        check_fcntl = fcntl(fd2[1], F_SETFL, flags2);*/


        printf("NOW PROCESSING CHILD: %c\n", characters[i]);

        //make a child which will execute tr -d 'letter'
        pid_t pid = fork();
        if(pid < 0)
        {
            perror("Child failed\n");
            return 1;
        }

        if(pid == 0) //MAIN child
        {
            close(fd1[1]); //writing end

            /* Read character using first pipe */
            char buff[1];
            while ( read(fd1[0], buff, 1) > 0 )
            {

                /* Exec tr -d */
                int fd_tr1[2];
                if(pipe(fd_tr1) == -1)
                {
                    perror("Pipe fd_tr1 failed, exiting...\n");
                    return 1;
                }

                int fd_tr2[2];
                if(pipe(fd_tr2) == -1)
                {  
                    perror("Pipe fd_tr2 failed, exiting...\n");
                    return 1;
                }

                pid_t chld_pid = fork();

                if(chld_pid == 0) //child executes tr -d
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
                    close(fd_tr1[0]);
                    close(fd_tr2[1]);

                    write(fd_tr1[1], buff, sizeof(buff));
                    close(fd_tr1[1]);

                    read(fd_tr2[0], buff2, sizeof(buff2));
                    close(fd_tr2[0]);
                }

                /* Write result to pipe */
                write(fd2[1], buff2, 1);
            
            } //while

            close(fd1[0]);
            close(fd2[0]);
            close(fd2[1]);

            exit(0);
        }
    else //MAIN parent
    {
        close(fd1[0]);

        /* Fork for sending data to all children at the same time */
        pid_t for_data = fork();
        if(for_data == 0)
        {
            int fd_data = open(fileWithData, O_RDONLY | O_NONBLOCK);

            while( read(fd_data, buff2, 1) > 0 ) //while we can read data from file
            {
                /* Write string to process in child */
                write(fd1[1], buff2, 1);

                /* Wait for child to be processed */
                struct pollfd pollfds;
                pollfds.fd = fd2[1];
                pollfds.events = POLLOUT | O_NONBLOCK;

                int poll_check = poll(&pollfds, 1, 500);
                if(poll_check <= 0)
                {
                    perror("Poll failed, exiting...\n");
                    return 1;
                }

                //printf("events: %d, revents: %d\n\n",pollfds.events, pollfds.revents);

                /* Read processed string from second pipe */
                nosleep(); //sleep for 3/4sec

                read(fd2[0], buff2, 1);
                printf("%s\n", buff2);

            }
            exit(0);
        }
        close(fd1[1]);
        close(fd2[1]);
        close(fd2[0]);
    }
}

wait(NULL); //wait until all children die

return 0;

}
