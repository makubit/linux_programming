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

#define PORT 12345
static char send_s[4] = { 's', 'e', 'n', 'd' };


int main(int argc, char* argv[])
{
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
   ts.it_interval.tv_sec = 0;
   ts.it_interval.tv_nsec = 0;
   ts.it_value.tv_sec = 2;
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

   if(inet_aton("127.0.0.1", &A.sin_addr) == -1)
   {
       perror("Inet aton error: Invalid address or address not supported\n");
       exit(EXIT_FAILURE);
   }

   if(connect(consumer_fd, (const struct sockaddr*)&A, addr_lenA) == -1)
   {
       perror("Connection failed\n");
       exit(EXIT_FAILURE);
   }

    /************** POLL **************/
    uint64_t timer_ticks;
    char read_data[8];
    struct pollfd pfds[2];
    pfds[0].fd = timer_fd;
    pfds[0].events = POLLIN;
    pfds[1].fd = consumer_fd;
    pfds[1].events = POLLIN;


    //for(int i = 0; i<5; i++)
    int returned_fds = poll(pfds, 2, 5000);
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
            }
        }

        else if(pfds[1].revents == POLLIN)
        {
            read(consumer_fd, read_data, sizeof(read_data));
            printf("%s\n", read_data);

            //create md5sum
            unsigned char md5_final[16];
            MD5_CTX contx;
            MD5_Init(&contx);
            MD5_Update(&contx, read_data, sizeof(read_data));
            MD5_Final(md5_final, &contx);
            printf("%s\n\n", md5_final);
        }
    }

    return 0;
}
