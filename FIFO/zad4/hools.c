#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#define NSEC 1000000

/* Function to add process pid before text */
char* add_pid_to_text(char* text)
{
    char result[100];
    char pid_str[10];
    pid_t pid = getpid();
    sprintf(pid_str, "%d", (int)pid);

    strcat(result, pid_str);
    strcat(result, text);
    strcpy(text, result);

    return text;
}

void* get_random_disposition()
{
   srand(time(0));
   int random = (rand() + getpid()) % 2;

   if(random != 0)
       return SIG_DFL;

   return SIG_IGN; 
}

int get_random_number()
{
    srand(time(0));

    return rand() % 10;
}

void nsleep(float t)
{
    float temp = t * 2.5;
    struct timespec time;
    time.tv_sec = (int)temp;
    time.tv_nsec = (long)(temp * NSEC) % NSEC;

    nanosleep(&time, NULL);
}

/* Singal SIGUSR2 handling */
static void sigusr2_handler(int signo, siginfo_t* status, void* context)
{

}

void display_help()
{
    printf("How to use this program:\n  -d <float number for sleeping>\n  positional parameter \"text\"\n\n");
}

//-------------------------------------------------------------

int main(int argc, char* argv[])
{
    int c;
    char d_parameter[10];
    float time = 1.0;
    char text[100];

    while((c = getopt(argc, argv, "d:")) != -1)
        switch(c)
        {
            case 'd':
                strcpy(d_parameter, optarg);
                time = strtod(d_parameter, NULL);
                break;
            case '?':
                perror("Wrong usage of parameters\n");
                break;
        }

    /* Prepare text */
    strcpy(text, argv[optind]);

    /* Wait for SIGUSR2 */
    sigset_t sigusr2_mask;
    sigfillset(&sigusr2_mask);
    sigdelset(&sigusr2_mask, SIGUSR2);

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = &sigusr2_handler;
    act.sa_flags = SA_SIGINFO;

    /* Change text */
    strcpy(text, add_pid_to_text(text));
    
    if(sigaction(SIGUSR2, &act, NULL) == -1)
        perror("chld: SIGUSR2 sigaction error\n");

    sigsuspend(&sigusr2_mask);

    pid_t pgrp = getpgid(getpid());
    
    int count = 0;

    while(1)
    {
        /* Chant */
        printf("%s\n", text);

        /* Change disposition */
        signal(SIGUSR1, get_random_disposition());

        /* Get random probability & send signal */
        if(!get_random_number())
            killpg(pgrp, SIGUSR1);

        nsleep(time);

        /* In case we waited too long for child to be killed */
        if(count > 50)
            raise(SIGUSR1);

        count++;
    }

    return 0;
}
