#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

pid_t pgid; //group pid
static volatile pid_t latest_sending_chld_pid = 0; //who sent sigusr1

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
        printf("\n  [*]  Child killed: %d\n", first->status.si_pid);
        if(first->status.si_pid == latest_sending_chld_pid)
            printf("       Darvin Award Granted\n");

        printf("\n");

        //free(first);
        first = newChld;

        if(first == NULL)
            queue = NULL;
    }
    else
        printf("Queue is empty, cannot pop\n");

    chldNumber--;
}

void free_queue()
{
    while(first)
    {
        newChld = first;
        free(first);
    }
}

//---------------------------------------------------------------

void display_help()
{
    printf("\nHow to use this program:\n  a.out N:F:T\n  N: int, children number\n  F: float, time stamps\n  T: text, should be in quotes!\n\n");
}

/* Split string with ':' delimiter */
int split_str(char** result, char* str)
{
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

void nsleep()
{
    struct timespec time;
    time.tv_sec = 1;
    time.tv_nsec = 0;

    nanosleep(&time, NULL);
}

static void sigusr1_handler(int signo, siginfo_t* status, void* context)
{
    printf("\n * * * * * * * * * * * * * * * * * * * * * * * *\n\tThis program sent SIGUSR1: %d\n * * * * * * * * * * * * * * * * * * * * * * * *\n\n", status->si_pid);

    latest_sending_chld_pid = status->si_pid; //save who sent signal
}

static void sigchld_handler(int signo, siginfo_t* status, void* context)
{
   siginfo_t status2;
   while(!waitid(P_PGID, pgid, &status2, WNOHANG | WEXITED))
   {
       if((status2.si_code == CLD_KILLED) || (status2.si_code == CLD_EXITED))
           push(status2);
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

    /* Split str */
    char* result[3];
    
    if(split_str(result, parameters) == -1)
    {
        perror("Split str error, exiting...\n");
        display_help();
        return 1;
    }

    /* Set variables */
    int n_chld = strtol(result[0], NULL, 0);
    char* t_text = malloc(sizeof(char) * strlen(result[2]));
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
    signal(SIGUSR2, SIG_IGN);

    /* Create process group */
    if(setpgid(getpid(), 0) == -1)
        perror("Setpgid error\n");

    pgid = getpgid(getpid());
    
    /* Create children */
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

    /* Synchronize */
    nsleep();

    /* Send signal to all processes in this group */
    if(killpg(pgid, SIGUSR2) == -1)
        perror("Sending SIGUSR2 error\n");

    /* Do until all chilren are dead */
    while(chldNumber > 0)
    {
        while(queue)
            pop();

        nsleep();
    }

    /* Free memory */
    free_queue();

    return 0;
}
