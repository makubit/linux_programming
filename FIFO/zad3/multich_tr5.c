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
            char buff_chld[1]; //to read & write to parent
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

                    /* If there is nothing to read, send empty string */
                    if(read(fd_tr2[0], buff_chld, 2) == 0)
                        strcpy(buff_chld, "");

                    close(fd_tr2[0]);
                }

                /* Write result to pipe */
                write(fd2[1], buff_chld, 1);
            
            } //while

            close(fd1[0]);
            close(fd2[0]);
            close(fd2[1]);

            exit(0);
        }
        else //MAIN parent
        {
            close(fd1[0]);
            
            /* Prepare buffor to store letters */
            char buff_print[100];
            int n_letters = 0;
            memset(buff_print, 0, 100);

            pid_t for_data = fork();
            if(for_data ==0)
            {

                int fd_data = open(fileWithData, O_RDONLY | O_NONBLOCK);
                //int n_letters = 0;
                char buff_write[1];

                while( read(fd_data, buff_write, 1) > 0 ) //while we can read data from file
                {
                    /* Write string to process in child */
                    write(fd1[1], buff_write, 1);

                    /* Wait for child to be processed */
                    struct pollfd pollfds;
                    pollfds.fd = fd2[1];
                    pollfds.events = POLLOUT;

                    int poll_check = poll(&pollfds, 1, 500);
                    if(poll_check <= 0)
                    {
                        perror("Poll failed, exiting...\n");
                        return 1;
                    }

                    /* Read processed string from second pipe */
                    nosleep(); //sleep for 3/4sec

                    char char_print[1];
                    read(fd2[0], char_print, 1);

                    if(strcmp(char_print, "") == 0)
                        n_letters--;
                    else              
                        buff_print[n_letters-1] = char_print[0];

                    /* PRINTING MAGIC */
                    printf("\e[%d;%dH%s\n", 2*i + 2, 1, reverseString(buff_print));

                    n_letters++;

                    if(n_letters == 78)
                        n_letters = 0;
                }
                exit(0);
            } //child

            close(fd1[1]);
            close(fd2[1]);
            close(fd2[0]);
        }
    }

    wait(NULL); //wait until all children die

    return 0;

}
