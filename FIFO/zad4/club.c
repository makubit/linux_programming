#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

pid_t pgid;
static volatile latest_sending_chld_pid;

/* Queue for dead children */
struct chldQueue
{
    siginfo_t status;
    pid_t chld_pid;
    struct chldQueue* next;
};

struct chldQueue* queue = NULL;
struct chldQueue* newChld = NULL;
struct chldQueue* first = NULL;

static volatile int chldNumber = 0;

void push(siginfo_t status)
{
    newChld = (struct chldQueue*)malloc(sizeof(struct chldQueue));
    (*newChld).status = status;
    if(queue == NULL)
        first = newChld;
    else
        (*queue).next = newChld;

    queue = newChld;
}

void pop()
{
    if(first != NULL)
    {
        newChld = (*first).next;

        /* Print obituary */
        printf("\nChild killed: %d\n", first->status.si_pid);
        //if()

        free(first);
        first = newChld;

        if(first == NULL)
            queue = NULL;

    }
    else
        printf("Queue is empty, cannot pop\n");

    chldNumber--;
}

//---------------------------------------------------------------

void display_help()
{
    printf("\nHow to use this program:\n  a.out N:F:T\n  N: int, children number\n  F: float, time stamps\n  T: text, should be in quotes!\n\n");
}

int split_str(char** result, char* str)
{
    //3 result string from parsing
    int i = 0;
    char* temp = strtok(str, ":");

    while(temp)
    {
        *(result + i++) = strdup(temp);
        temp = strtok(0, ":");
    }

    if(i != 3)
        return -1;

    return 0;
}

static void sigusr1_handler(int signo, siginfo_t* status, void* context)
{
    printf("\n * * * * * * * * * * * * * * * * * * * * * * * *\n   This program sent SIGUSR1: %d, looooooser\n * * * * * * * * * * * * * * * * * * * * * * * *\n\n", status->si_pid);

    latest_sending_chld_pid = status->si_pid;
}

static void sigchld_handler(int signo, siginfo_t* status, void* context)
{
   siginfo_t status2;
   while(!waitid(P_PGID, pgid, &status2, WNOHANG | WEXITED))
   {
       if((status2.si_code == CLD_KILLED) || (status2.si_code == CLD_EXITED))
       {
           push(status2);
           //chldNumber--;
       }
       else
           break;
   }
}

//-----------------------------------------------------

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        display_help();
        return 1;
    }

    char parameters[100];
    strcpy(parameters, argv[1]);

    /* Buffers for parsing result */
    //char n_chld[100]; //integer - > chld number
    //char f_time[100]; //time stamps
    //char t_text[100]; //text for chanting

    //function for splitting
    char* result[3];
    
    if(split_str(result, parameters) == -1)
    {
        perror("Split str error, exiting...\n");
        display_help();
        return 1;
    }

    //set variables
    int n_chld = strtol(result[0], NULL, 0);
    float f_time = strtod(result[1], NULL);
    char* t_text = malloc(sizeof(char) * strlen(result[2]));
    //strcpy(t_text, result[2]);
    t_text = strdup(result[2]);

    /* Set global chldNumber */
    chldNumber = n_chld;

    /* Signals handling */
    struct sigaction usr1_act, chld_act;

    sigemptyset(&usr1_act.sa_mask);
    usr1_act.sa_sigaction = &sigusr1_handler;
    usr1_act.sa_flags = SA_SIGINFO;

    sigemptyset(&chld_act.sa_mask);
    chld_act.sa_sigaction = &sigchld_handler;
    chld_act.sa_flags = SA_SIGINFO;

    if(sigaction(SIGUSR1, &usr1_act, NULL) == -1)
        perror("SIGUSR1 sigaction error\n");

    if(sigaction(SIGCHLD, &chld_act, NULL) == -1)
        perror("SIGCHLD sigaction error\n");

    /* Ignore SIGUSR2 signal */
    signal(SIGUSR2, SIG_IGN); // TODO: check in errno

    /* Create process group */
    if(setpgid(getpid(), 0) == -1)
        perror("Setpgid error\n");

    pgid = getpgid(getpid()); //save pgid
    
    //tworzenie potomków zgodnie z parametrem n
    for(int i = 0; i<n_chld; i++)
    {
        pid_t chld_pid = fork();

        if(setpgid(chld_pid, pgid) == -1)
            perror("Setpgid for chld error\n");

        if(chld_pid == 0)
        {
            
            if(execl("./hools.out", "./hools.out", "-d", result[1], t_text, NULL) == -1)
                perror("Execl error\n");

            exit(0);
        }
    }

    /* Synchronize */ //TODO: change it
    sleep(1);

    /* Send signal to all processes in this group */
    if(killpg(pgid, SIGUSR2) == -1)
        perror("Sending SIGUSR2 error\n");

    while(chldNumber > 0)
    {
        while(queue)
            pop();
        sleep(1);
    }

    wait(NULL);

    //potomek -> program hools, zobaczyć czy dostaje pid grupy

    //gdy wszystkie potomki stworzone, wysłanie sygnału SIGUSR2 do grupy procesów

    //*signal handling

    return 0;
}
