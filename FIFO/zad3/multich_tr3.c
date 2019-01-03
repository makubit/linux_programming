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

static volatile int counters[10]; //counters for all children
static char result1[50];
static char result2[50];

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

    /* Make fifo file -> on write end I will send data from all children, on read I use cat to see whats happening */
    /*mkfifo("fifo", 0666);
    int fifo_fd = open("fifo", O_WRONLY | O_RDONLY | O_NONBLOCK);

    pid_t mkfifo_pid = fork();
    if(mkfifo_pid < 0)
    {
        perror("fork for mkfifo failed, exiting...\n");
        return 1;
    }*/

    //if(mkfifo_pid == 0)
    //{
        /* Call system function cat to see what children have processed */
        //system("cat < fifo");
        //printf("\e[1;1H\e[2J");
        //exit(0);
    //}
    //for(int i = 0; i< 2*strlen(characters); i++)
       // write(fifo_fd, "\n", 1);

    /* Rest of the program, open fifo file */
    //int fifo_fd = open("fifo", O_WRONLY | O_NONBLOCK);

    /* Prepare pipes */
    int fd1[2]; //sends input data from parent to child
    int fd2[2]; //sends edited string from child to parent

    char buff2[1]; //buffor for reading from child
    int count1= 0;
    int count2 = 0;
//    char result1[79];
    //memset(result, 0, 79);
    //result[0][0] = '\n';
    //result[79] = '\n';
    //result[79*2] = '\n';
 //   char result2[50];
//    memset(result2, 0, 50);

    //wyczysc caly ekran
    printf("\ec");

    
    
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

        //printf("NOW PROCESSING CHILD: %c\n", characters[i]);

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

                    char buff_temp[2] = {""};
                    if(read(fd_tr2[0], buff_temp, 2) > 0) //check if there is newletter
                        strcpy(buff2, buff_temp);

                    close(fd_tr2[0]);
                    //exit(0);
                }

                //int line = 2*i + 2;
                //printf("\e[%d;0H%c", 2*i + 2, buff[0]);
            //}
                /* Write result to pipe */
                write(fd2[1], buff2, 1);
                printf("%c\n", buff2[0]);

                /*if(i == 0)
                {   
                    result1[count1]=buff2[0];
                    //result2[0] = '\n';
                    count1++;}
                else
                {
                    //result1[0] = '\n';
                    result2[count2] = buff2[0];
                    count2++;
                }*/
                    //count2++;

                //printf("\ec");
                //printf("%s\n%s\n", result1, result2);
                //printf("\n%s\n", result2);
                //}
                //
                /*******************************************************
                 *  TRYING CONSOLE_CODES
                 */
                
                //printf("\e[2J");
                //printf("tekst\ntest\ntekst\n");
                ////int line = 2*i + 2;
                ////printf("\e[%d;0H%c", 2*i + 2, buff[0]);//, buff2[0]);
                //printf("%c", buff[0]);
                //printf("helo");
                
                


                //char ch = "";
                //while((ch = getc(fifo_fd)) != "\n"
//write(fifo_fd, "\n", 1);

                //close(fifo_fd);
                //int nfifo_fd = open("fifo", O_RDONLY | O_WRONLY | O_NONBLOCK);
                //lseek(fifo_fd, 20, SEEK_SET);
                //write(fifo_fd, buff2, 1);

                //printf("check");
                //char ch[1];
                //read(fifo_fd, ch, 1);
                //while(ch != EOF)
                //{
                    //read(fifo_fd, ch, 1);
                    //if(ch == "\n")
                        //write(fifo_fd, buff2, 1);
            
            //}
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
        //pid_t for_data = fork();
        //if(for_data == 0)
        //{
            int fd_data = open(fileWithData, O_RDONLY | O_NONBLOCK);

            /*pid_t try = fork();
            if(try == 0)
            {
                int o = 0;
                while(o != 10)
                {
                    nosleep();
printf("\ec");
                printf("%s\n%s\nend", result1, result2);
                o++;

                }
                exit(0);

            }*/

            while( read(fd_data, buff2, 1) > 0 ) //while we can read data from file
            {
                /* Write string to process in child */
                write(fd1[1], buff2, 1);

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

                //printf("events: %d, revents: %d\n\n",pollfds.events, pollfds.revents);

                /* Read processed string from second pipe */
                nosleep(); //sleep for 3/4sec

                read(fd2[0], buff2, 1);
                //write(fifo_fd, buff2, 1);
                //if(i==0) result1[0] = buff2[0];
                //else result2[0] = buff2[0];
                //printf("\ec");
                //printf("%s\n%s\nend", result1, result2);

                /*result[0] = buff2[0];
 printf("\ec");
                printf("\n%s\n", result);
                printf("%s", result2);*/

            }
            exit(0);
        //}
        close(fd1[1]);
        close(fd2[1]);
        close(fd2[0]);
    }
}
wait(NULL); //wait until all children die

return 0;

}
