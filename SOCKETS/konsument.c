/* Konsument odbiera 112KB od producenta
 * generuje md5
 * generuje raport */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <openssl/md5.h>
#include <time.h>

#define PORT 12345
#define BUFFER_SIZE112 112*1000
#define BLOCKS 2
static char send_s[4] = { 's', 'e', 'n', 'd' };

struct dataraport {
    struct timespec delay_a;
    struct timespec delay_b;
    char* md5_final;
};

void add_to_dataraport(struct dataraport* data_r, unsigned char* md5_final, int ticks_counter, struct timespec* times)
{
    data_r[ticks_counter].md5_final = malloc(sizeof(md5_final));
    strcpy(data_r[ticks_counter].md5_final, (char*)md5_final);

    data_r[ticks_counter].delay_a.tv_sec = times[1].tv_sec - times[0].tv_sec;
    data_r[ticks_counter].delay_a.tv_nsec = times[1].tv_nsec - times[0].tv_nsec;

    data_r[ticks_counter].delay_b.tv_sec = times[3].tv_sec - times[2].tv_sec;
    data_r[ticks_counter].delay_b.tv_nsec = times[3].tv_nsec - times[2].tv_nsec;
}

void gen_raport(struct dataraport* data_r, int blocks)
{
    struct timespec t_mono;
    struct timespec t_real;

    clock_gettime(CLOCK_MONOTONIC, &t_mono);
    clock_gettime(CLOCK_REALTIME, &t_real);

    fprintf(stderr, "\t********** RAPORT **********\n");
    fprintf(stderr, " -> Monotonic: %ldsec, %ldnsec\n", t_mono.tv_sec, t_mono.tv_nsec);
    fprintf(stderr, " -> RealTime: %ldsec, %ldnsec\n", t_real.tv_sec, t_real.tv_nsec);

    fprintf(stderr, " -> PID: %d, IPAddress: 127.0.0.1\n\n", getpid());

    //roznice czasu
    //  a) miedzy wyslanie zgloszenia a odczytaniem pierwszych bajtow
    //  b) odczytaniem pierwszych bajtow, a całością bloku

    for(int i = 0; i<blocks; i++)
    {
        fprintf(stderr, " \t********** BLOCK NO %d **********\n", i+1);
        fprintf(stderr, " -> Diff between sending mes and reading: %ldsec, %ldnsec\n", data_r[i].delay_a.tv_sec, data_r[i].delay_a.tv_nsec);
        fprintf(stderr, " -> Diff between start & end of reading: %ldsec, %ldnsec\n\n", data_r[i].delay_b.tv_sec, data_r[i].delay_b.tv_nsec);
    }
}

/**********************************************************************************
 *********************************************************************************/

int main(int argc, char* argv[])
{
    int c;

    //while((c = getopt(argc, argv, "#:r:s:")));


    /******** CREATE FDS ********/
    int consumer_fd = socket(AF_INET, SOCK_STREAM, 0);
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if((consumer_fd == -1) || (timer_fd == -1))
    {
        perror("Creating consumer error\n");
        exit(EXIT_FAILURE);
    }

    /******** CREATE TIMER ********/
    struct itimerspec ts;
    ts.it_interval.tv_sec = 1;
    ts.it_interval.tv_nsec = 500000;
    ts.it_value.tv_sec = 1;
    ts.it_value.tv_nsec = 500000;

    if(timerfd_settime(timer_fd, 0, &ts, NULL) < 0)
    {
        perror("Set time in timer error\n");
        close(timer_fd);
        exit(EXIT_FAILURE);
    }

    /************* CREATE SOCKET **********/
    struct sockaddr_in A;
    socklen_t addr_lenA = sizeof(A);

    A.sin_family = AF_INET;
    A.sin_port = htons(PORT);

    if(inet_aton("127.0.0.2", &A.sin_addr) == -1)
    {
        perror("Inet aton error: Invalid address or address not supported\n");
        exit(EXIT_FAILURE);
    }

    if(connect(consumer_fd, (const struct sockaddr*)&A, addr_lenA) == -1)
    {
        perror("Connection failed\n");
        exit(EXIT_FAILURE);
    }

    /************** CREATE DATA RAPORT STRUCTURE ***************/
    struct dataraport data_r[BLOCKS];
    //struct timespec before_read_be, before_read_af, read_be, read_af;
    struct timespec clock_times[4];

    /************** POLL **************/
    uint64_t timer_ticks;
    char read_data[8];
    int returned_fds = 0;
    struct pollfd pfds[2];
    pfds[0].fd = timer_fd;
    pfds[0].events = POLLIN;
    pfds[1].fd = consumer_fd;
    pfds[1].events = POLLIN;


    int ticks_counter = 0;
    //while(ticks_counter <= BLOCKS)
    for(int i = 0; i<BLOCKS*2; i++)
    {
        returned_fds = poll(pfds, 2, 5000);
        printf("\n%d -> %d\n", pfds[0].events, pfds[0].revents);
        printf("\n%d -> %d\n", pfds[1].events, pfds[1].revents);

        if(returned_fds > 0)
        {
            if(pfds[0].revents == POLLIN)
            {
                read(timer_fd, &timer_ticks, sizeof(timer_ticks));

                for(int i = 0; i < timer_ticks; i++)
                {
                    send(consumer_fd, send_s, sizeof(send_s), 0);
                    clock_gettime(CLOCK_REALTIME, &clock_times[0]);
                }

                ticks_counter++;;
            }

            if(pfds[1].revents == POLLIN)
            {
                clock_gettime(CLOCK_REALTIME, &clock_times[1]);
                clock_gettime(CLOCK_REALTIME, &clock_times[2]);
                read(consumer_fd, read_data, sizeof(read_data));
                clock_gettime(CLOCK_REALTIME, &clock_times[3]);
                printf("%s\n", read_data);

                //create md5sum
                unsigned char md5_final[16];
                MD5_CTX contx;
                MD5_Init(&contx);
                MD5_Update(&contx, read_data, sizeof(read_data));
                MD5_Final(md5_final, &contx);

                /********* ADD TO DATA RAPORT STRUCTURE  *********/
                printf("Ticks counter: %d\n", ticks_counter);
                add_to_dataraport(data_r, md5_final, ticks_counter-1, clock_times);

            }
        }
    }

    gen_raport(data_r, BLOCKS);

    close(timer_fd);
    close(consumer_fd);

    return 0;
}
