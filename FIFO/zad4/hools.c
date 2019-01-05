#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

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
   int random = rand() % 2;

   if(random == 0)
       return SIG_IGN;
       
   return SIG_DFL; 
}

int get_random_number()
{
    srand(time(0));

    return rand() % 10;
}

void nsleep(float t)
{
    struct timespec time;
    time.tv_sec = t;
    time.tv_nsec = 0; //TODO: przelicznik na karte dekasekundy

    nanosleep(&time, NULL);
}

/* Singal SIGUSR2 handling */
static void sigusr2_handler(int signo, siginfo_t* status, void* context)
{
    printf("Handler\n");
}

void displayHelp()
{
    printf("How to use this program:\n  -d <float number for sleeping>\n  positional parameter \"text\"\n\n");
}

//-------------------------------------------------------------

int main(int argc, char* argv[])
{
    int c;
    char d_parameter[10]; //ilosc czasu do sparsowania
    float time = 1.0; //kwarta dekasekundy
    char text[100]; //tekst do skandowania

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

    //TODO: Jesli nie ma tekstu -> blad

    /* Prepare text */
    strcpy(text, argv[optind]);

    //czeka na sygnal SIGUSR2 od rodzica -> sigsuspend
    sigset_t sigusr2_mask;
    sigfillset(&sigusr2_mask);
    sigdelset(&sigusr2_mask, SIGUSR2);

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = &sigusr2_handler;
    act.sa_flags = SA_SIGINFO;

    //change text
    strcpy(text, add_pid_to_text(text));
    
    sigaction(SIGUSR2, &act, NULL); //TODO: check

    sigsuspend(&sigusr2_mask);

    while(1)
    {

        //wypisuje skandowane haslo
        printf("%s\n", text);

        //zmienia dyspozycje dla sygnalu SIGUSR1 na losowo wybrane SIG_IGN lub SIG_DFL
        //get_random_signal();
        signal(SIGUSR1, get_random_disposition());

        //z prawdopodobienstwem 1/10 wysyla sygnal SIGUSR1 do grupy procesow, do ktorej sam nalezy
        if(!get_random_number())
        {
            pid_t pgrp = getpgid(getpid());
            killpg(pgrp, SIGUSR1);
        }

        printf("%d\n", get_random_number());

        //spi przez zadana ilosc w parametrze -d
        nsleep(time);
    }

    return 0;
}
