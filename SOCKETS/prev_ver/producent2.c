/* Producent generuje 640 bajtów do bufora cyklicznego
 * wysyła dane w paczkach po 112KB do konsumentów
 * generuje raport do pliku */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <signal.h>
#include <poll.h>
#include <sys/timerfd.h>

#define PORT 12345
#define NANOSEC 1000000
#define BUFF_SIZE 1024*1024*1.25
#define DATA_SIZE 640

/*****************************************************
 * CIRCULAR BUFFER
 ****************************************************/

typedef struct c_buff
{
    char* buffer;
    int max;
    int capacity;
    int el_size;
    size_t head;
    int tail;

} c_buff;

void cb_init(c_buff* cbuf, int max)
{
    cbuf->buffer = malloc(max);
    cbuf->capacity = 0;
    cbuf->el_size = 1;
    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->max = max;
}

void cb_push(c_buff* cbuf, int data)
{
    int idx = (cbuf->head) % cbuf->max;

    if(cbuf->capacity == cbuf->max)
        cbuf->tail++;
    cbuf->head++;
    cbuf->buffer[idx] = data;
}

int cb_pop(c_buff* cbuf)
{
    int idx = (cbuf->tail) % cbuf->max;
    cbuf->tail++;

    return cbuf->buffer[idx];
}

//------------------------------------------------------

void display_help()
{
  printf("Instructions:\n -> -r <char*> mandatory, path to ganerate raports\n");
  printf(" -> -t <int> mandatory, <int> has to be in range(1, 8)\n");
  printf(" -> [<addr>:]port mandatory,  port address\n");
}

float convert_to_float18(char* int_buff)
{
  char* pEnd = NULL;
  float temp = strtod(int_buff, &pEnd);

  if((temp <=0) || (temp >8) || (*pEnd != '\0'))
  {
    printf(" converting to int error: -t parameter has to be an integer in range(1, 8)\n");
    display_help();
    exit(EXIT_FAILURE);
  }

  //convert min/96 to secs
  //1temp --- 60/96sek
  return (temp*60/96);
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

//-------------- RAPORT ----------------
void generate_raport()
{
  printf("generate_raport\n");
}

//----------------------------------------------------------
//----------------------------------------------------------

int main(int argc, char* argv[])
{
  int c;
  char* tempbuff = NULL;
  char* raport_path = NULL;
  float pace_val = 0;
  int port_addr = 0;

  while((c = getopt(argc, argv, "r:t:")) != -1)
    {
      switch(c)
      {
          case 'r':
              raport_path = malloc(sizeof(optarg));
              strcpy(raport_path, optarg);
              break;
          case 't':
              tempbuff = malloc(sizeof(optarg));
              strcpy(tempbuff, optarg);
              pace_val = convert_to_float18(tempbuff);
              break;
          case '?':
              display_help();
              break;
      }
    }

    if((raport_path == NULL) || (pace_val == 0) || (argv[optind] == NULL))
    {
      printf(" Mandatory parameters: -r <path>, -t <val>, [<addr>]:port\n");
      display_help();
      exit(EXIT_FAILURE);
    }

    port_addr = convert_address(argv[optind]);

   /******************************************
   * BUFFER TODO:
   ******************************************/
    c_buff* cb;
    cb = malloc(sizeof(c_buff));

    cb_init(cb, 10);

    /************* BUFFER TESTS ***************/
    /*cb_push(cb, 'A');
    char* c = "hro";
    memcpy(&cb->buffer[1], c, strlen(c));
    cb->head +=3;
    memcpy(&cb->buffer[cb->head], c, strlen(c));
    cb->head += strlen(c);
    printf("%s %d\n", cb->buffer, cb->head);*/
    /*****************************************/



    /********************************************************
     * SOCKET *
     *******************************************************/

    /********* CREATE FDS **********/
    int producer_fd = socket(AF_INET, SOCK_STREAM, 0);
    int dtimer_fd = timerfd_create(CLOCK_MONOTONIC, 0); //to produce data
    int rtimer_fd = timerfd_create(CLOCK_MONOTONIC, 0); //to generate raport
    if((producer_fd == -1) || (dtimer_fd == -1) || (rtimer_fd == -1))
    {
        perror("Creating consumer or timer error\n");
        exit(EXIT_FAILURE);
    }

    /******** CREATE TIMERS ********/
    struct itimerspec dts, rts; //dts -> struct for dtimer_fd, rts -> struct for rtimer_fd

    dts.it_interval.tv_sec = (long)(pace_val);
    dts.it_interval.tv_nsec = (long)(pace_val * NANOSEC) % NANOSEC;
    dts.it_value.tv_sec = (long)(pace_val);
    dts.it_value.tv_nsec = (long)(pace_val * NANOSEC) % NANOSEC;

    rts.it_interval.tv_sec = 5;
    rts.it_interval.tv_nsec = 0;
    rts.it_value.tv_sec = 5;
    rts.it_value.tv_nsec = 0;

    if(timerfd_settime(dtimer_fd, 0, &dts, NULL) < 0)
    {
        perror("Set time in dtimer error\n");
        close(dtimer_fd);
        close(rtimer_fd);
        exit(EXIT_FAILURE);
    }

    if(timerfd_settime(rtimer_fd, 0, &rts, NULL) < 0)
    {
        perror("Set time in rtimer error\n");
        close(dtimer_fd);
        close(rtimer_fd);
        exit(EXIT_FAILURE);
    }

    /************* CREATE SOCKET **********/
    struct sockaddr_in A;
    socklen_t addr_lenA = sizeof(A);

    A.sin_family = AF_INET;
    A.sin_port = htons(port_addr);

    if(inet_aton("127.0.0.2", &A.sin_addr) == -1)
    {
        perror("Aton inet error: Invalid address or address not supported\n");
        exit(EXIT_FAILURE);
    }

    if(bind(producer_fd, (const struct sockaddr*)&A, addr_lenA) == -1)
    {
        perror("bind error\n");
        exit(EXIT_FAILURE);
    }

    /************** CREATE DATA RAPORT STRUCTURE ***************/
    /************** POLL **************/
    uint64_t dtimer_ticks, rtimer_ticks;
    char read_data[8];
    int returned_fds = 0;
    int consumer_counter = 0;
    int consumer_max = 10;
    int consumer_fds[10];

    struct pollfd pfds[13]; //resize
    pfds[0].fd = dtimer_fd;
    pfds[0].events = POLLIN;
    pfds[1].fd = rtimer_fd;
    pfds[1].events = POLLIN;
    pfds[2].fd = producer_fd;
    pfds[2].events = POLLIN;

    int dticks_counter = 0; //??
    int rticks_counter = 0; //??

    while(1)
    {
      returned_fds = poll(pfds, 10, 5000);
      printf("\n%d -> %d\n", pfds[0].events, pfds[0].revents);
      printf("%d -> %d\n", pfds[1].events, pfds[1].revents);
      //printf("%d -> %d\n", pfds[2].events, pfds[2].revents);

      if(returned_fds > 0)
      {
          if(pfds[0].revents == POLLIN)
          {
            read(dtimer_fd, &dtimer_ticks, sizeof(dtimer_ticks));

            cb_push(cb, 'A');
            printf("buffer: %s\n", cb->buffer);

            dticks_counter++;
          }

          if(pfds[1].revents == POLLIN)
          {
            read(rtimer_fd, &rtimer_ticks, sizeof(rtimer_ticks));
            printf("generuj\n");
            generate_raport();

            rticks_counter++;
          }

          if(pfds[2].revents) //if some actions on this socket -> listen and connect
          {
            if(listen(producer_fd, 50) == -1)
            {
                    perror("Cannot listen error\n");
                    exit(EXIT_FAILURE);
            }

            if(pfds[2].revents == POLLHUP)
              continue;

            //if(consumer_counter == consumer_max)
            //{
              //realloc
            //}

            struct sockaddr_in B;
            int addr_lenB = sizeof(B);

            consumer_fds[consumer_counter] = accept(producer_fd, (struct sockaddr*)&B, (socklen_t*)&addr_lenB);
            if(consumer_fds[consumer_counter] == -1)
            {
                perror("Accept error\n");
                exit(EXIT_FAILURE);
            }
            printf("accepted\n");



          }

          //każda dalsza ilość fds, trzeba przeleciec wszystkie
      }

      sleep(1);
    }


    /*if(listen(producer_fd, 50) == -1)
    {
        perror("Listen error\n");
        exit(EXIT_FAILURE);
    }*/

    /*struct sockaddr_in B;
    int addr_lenB = sizeof(B);

    int consumer_fd = accept(producer_fd, (struct sockaddr*)&B, (socklen_t*)&addr_lenB);
    if(consumer_fd == -1)
    {
        perror("Accept error\n");
        exit(EXIT_FAILURE);
    }*/

    /*char buff[100];
    char* s = "Wysylam";
    memset(buff, 0, sizeof(buff));
    printf("accepted\n");

    //pthread_t tid[10];
    //if(pthread_create(&tid[0], NULL, ))
    read(consumer_fd, buff, sizeof(buff));
    send(consumer_fd,s, sizeof(s), 0 );
    read(consumer_fd, buff, sizeof(buff));

    send(consumer_fd, s, sizeof(s), 0);

    printf("%s\n\n", buff);*/

    close(dtimer_fd);
    close(rtimer_fd);
    //close(consumer_fd);
    close(producer_fd);


    return 0;
}