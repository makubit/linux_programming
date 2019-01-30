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
#include <arpa/inet.h>
#include <time.h>

#define NANOSEC 1000000
#define TIMER_LOCATION 1 //-r: 1, -s: 0
#define SEN_BLOCK_SIZE 112*1024
static char send_s[4] = { 's', 'e', 'n', 'd' };

struct dataraport {
    struct timespec delay_a;
    struct timespec delay_b;
    char* md5_final;
};

void display_help()
{
  printf("Instructions:\n -> -#<int> optional(:<int>) mandatory parameter, number of blocks we want to get from producer\n");
  printf(" -> -s <float> | -r <float> mandatory, time\n");
  printf(" -> [<addr>:]port mandatory,  port address\n");
}

int convert_to_int(char* first_p, char* second_p)
{
  //convert to int
  int cnt = strtol(first_p, NULL, 0);
  if(cnt <= 0)
  {
    printf(" <cnt> has to be >= 0\n");
    display_help();
    exit(EXIT_FAILURE);
  }
  if(second_p)
  {
      int cnt_tmp = strtol(second_p, NULL, 0);
      if(cnt > cnt_tmp)
      {
          perror("-# error: second parameter cannot be lesser than first parameter\n");
          exit(EXIT_FAILURE);
      }

      srand(time(NULL));
      return (rand() % (cnt_tmp + 1 - cnt)) + cnt;
    }
    return cnt;
}

float convert_to_float(char* first_p, char* second_p)
{
  //convert to float
  float dly = strtod(first_p, NULL);
  if(dly <= 0)
  {
    printf(" <dly> has to be >= 0\n");
    display_help();
    exit(EXIT_FAILURE);
  }
  if(second_p)
  {
    float dly_tmp = strtod(second_p, NULL);
    if(dly > dly_tmp)
    {
        perror("-# error: second parameter cannot be lesser than first parameter\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    return ((float)rand()/RAND_MAX + (rand() % ((int)dly_tmp + 1 - (int)dly)) + (int)dly);
  }
  return dly;
}

int convert_address(char* addr)
{
  char* first_p = strtok(addr, "port");
  first_p = strtok(first_p, "[");
  first_p = strtok(first_p, ":]");

  int temp = 0;
  if(first_p == NULL)
  {
    char* loc = "localhost";
    for(int i=0; i< strlen(loc); i++)
        temp += loc[i];

    return temp;
  }
  temp = strtol(first_p, NULL, 0);

  if(temp <= 0)
  {
    printf(" port error: wrong port number\n");
    display_help();
    exit(EXIT_FAILURE);
  }

  return temp;
}

void add_to_dataraport(struct dataraport* data_r, unsigned char* md5_final, int ticks_counter, struct timespec* times)
{
    data_r[ticks_counter].md5_final = (char*)malloc(sizeof(md5_final)); //!
    strcpy(data_r[ticks_counter].md5_final, (char*)md5_final);

    data_r[ticks_counter].delay_a.tv_sec = times[1].tv_sec - times[0].tv_sec;
    data_r[ticks_counter].delay_a.tv_nsec = times[1].tv_nsec - times[0].tv_nsec;

    data_r[ticks_counter].delay_b.tv_sec = times[3].tv_sec - times[2].tv_sec;
    data_r[ticks_counter].delay_b.tv_nsec = times[3].tv_nsec - times[2].tv_nsec;
}

void gen_raport(struct dataraport* data_r, int cnt)
{
    struct timespec t_mono;
    struct timespec t_real;

    clock_gettime(CLOCK_MONOTONIC, &t_mono);
    clock_gettime(CLOCK_REALTIME, &t_real);

    fprintf(stderr, "\t********** RAPORT **********\n");
    fprintf(stderr, " -> Monotonic: %ldsec, %ldnsec\n", t_mono.tv_sec, t_mono.tv_nsec);
    fprintf(stderr, " -> RealTime: %ldsec, %ldnsec\n", t_real.tv_sec, t_real.tv_nsec);

    fprintf(stderr, " -> PID: %d, IPAddress: 127.0.0.1\n\n", getpid());

    //time differences
    //  a) between sending message and reading first bytes of reply
    //  b) betewwn reading first bytes and last bytes

    for(int i = 0; i<cnt; i++)
    {
        fprintf(stderr, " \t********** BLOCK NO %d **********\n", i+1);
        fprintf(stderr, " -> Diff between sending mes and reading: %ldsec, %ldnsec\n", data_r[i].delay_a.tv_sec, data_r[i].delay_a.tv_nsec);
        fprintf(stderr, " -> Diff between start & end of reading: %ldsec, %ldnsec\n", data_r[i].delay_b.tv_sec, data_r[i].delay_b.tv_nsec);
        fprintf(stderr, " -> MD5SUM: %s\n\n", data_r[i].md5_final);
    }
}

/**********************************************************************************
 *********************************************************************************/

int main(int argc, char* argv[])
{
  int c;
  char* tempbuff = NULL;
  int cnt = 0;
  float dly = 0;
  char* first_p = NULL;
  char* second_p = NULL;
  int port_addr = 0;

  while((c = getopt(argc, argv, "#:r:s:")) != -1)
    {
      switch(c)
      {
          case '#':
              tempbuff = (char*)malloc(sizeof(optarg));
              strcpy(tempbuff, optarg);
              first_p = strtok(tempbuff, ":");
              second_p = strtok(NULL, ":");

              //convert to int
              cnt = convert_to_int(first_p, second_p);

              break;
          case 'r':
              if(dly > 0)
              {
                  printf(" -r error, cannot use both -r and -s\n");
                  display_help();
                  exit(EXIT_FAILURE);
              }

              tempbuff = (char*)malloc(sizeof(optarg));
              strcpy(tempbuff, optarg);
              first_p = strtok(tempbuff, ":");
              second_p = strtok(NULL, ":");

              //convert to float
              dly = convert_to_float(first_p, second_p);

              break;
          case 's':
              if(dly > 0)
              {
                  printf(" -r error, cannot use both -r and -s\n");
                  display_help();
                  exit(EXIT_FAILURE);
              }

              tempbuff = (char*)malloc(sizeof(optarg));
              strcpy(tempbuff, optarg);
              first_p = strtok(tempbuff, ":");
              second_p = strtok(NULL, ":");

              //convert to float
              dly = convert_to_float(first_p, second_p);

              break;
          case '?':
              display_help();
              break;
      }
    }

    if((cnt == 0) || (dly == 0) || (argv[optind] == NULL))
    {
      printf(" Mandatory parameters: <cnt>, <dly>, [<addr>]:port\n");
      display_help();
      exit(EXIT_FAILURE);
    }

    port_addr = convert_address(argv[optind]);

    /******** CREATE FDS ********/
    int consumer_fd = socket(AF_INET, SOCK_STREAM, 0);
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if((consumer_fd == -1) || (timer_fd == -1))
    {
        perror("Creating consumer or timer error\n");
        exit(EXIT_FAILURE);
    }

    /******** CREATE TIMER ********/
    struct itimerspec ts;
    ts.it_interval.tv_sec = (long)(dly);
    ts.it_interval.tv_nsec = (long)(dly * NANOSEC) % NANOSEC;
    ts.it_value.tv_sec = (long)(dly);
    ts.it_value.tv_nsec = (long)(dly * NANOSEC) % NANOSEC;

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
    A.sin_port = htons(port_addr);

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

    /************** CREATE DATA RAPORT STRUCTURE ***************/
    struct dataraport data_r[cnt];
    struct timespec clock_times[4];

    /************** POLL **************/
    uint64_t timer_ticks;
    char read_data[SEN_BLOCK_SIZE];
    int returned_fds = 0;

    struct pollfd pfds[2];
    pfds[0].fd = timer_fd;
    pfds[0].events = POLLIN;
    pfds[1].fd = consumer_fd;
    pfds[1].events = POLLIN;

    int ticks_counter = 0;
    int recived = 0;
    int check_r = 0; //to check if we recived all data we wanted

    while(recived < cnt)
    {
        returned_fds = poll(pfds, 2, 5000);

        if(returned_fds > 0)
        {
          if((ticks_counter) < cnt)
          {
              if(pfds[0].revents == POLLIN)
              {
                read(timer_fd, &timer_ticks, sizeof(timer_ticks));

                for(int i = 0; i < timer_ticks; i++)
                    send(consumer_fd, send_s, sizeof(send_s), 0);

                clock_gettime(CLOCK_REALTIME, &clock_times[0]);
                ticks_counter++;
              }
            }
          else
            {
              close(timer_fd);
            }

            if(pfds[1].revents == POLLIN)
            {
                clock_gettime(CLOCK_REALTIME, &clock_times[1]);
                clock_gettime(CLOCK_REALTIME, &clock_times[2]);

                int r = read(consumer_fd, read_data, SEN_BLOCK_SIZE);

                //check if we recived all bytes we wanted, than md5 and clock_gettime
                if(r == SEN_BLOCK_SIZE)
                {
                  recived++;
                }
                else if(r > 0)
                {
                  check_r +=r;

                  if(check_r == SEN_BLOCK_SIZE)
                  {
                    recived++;
                    check_r = 0;
                  }
                }

                clock_gettime(CLOCK_REALTIME, &clock_times[3]);

                //create md5sum
                unsigned char md5_final[16];
                MD5_CTX contx;
                MD5_Init(&contx);
                MD5_Update(&contx, read_data, sizeof(read_data));
                MD5_Final(md5_final, &contx);

                /********* ADD TO DATA RAPORT STRUCTURE  *********/
                add_to_dataraport(data_r, md5_final, ticks_counter-1, clock_times);
            }
        }
    }

    gen_raport(data_r, cnt);

    //free all data

    close(timer_fd);
    shutdown(consumer_fd, 2);
    close(consumer_fd);

    return 0;
}
