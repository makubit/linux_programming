/* Producent generuje 640 bajtów do bufora cyklicznego
 * wysyła dane w paczkach po 112KB do konsumentów
 * generuje raport do pliku */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <signal.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/syscall.h>

#define NANOSEC 1000000
#define BUFF_SIZE 1024*1024*1.25
#define GEN_BLOCK_SIZE 640
#define SEN_BLOCK_SIZE 112*1024
// :640 = 192

char* str_loop = "abcdefghijklmnopqrstuwxyzABCDEFGHIJKLMNOPQRSTUWXYZ";
static int PRODUCTION = 1;
static int START_PRODUCTION = 1;
static int connected = 0;

struct dataraport {
  int consumer_id;
  char* ip_addr;
  int data_blocks;
};

/*****************************************************
 * CIRCULAR BUFFER
 ****************************************************/

 typedef struct tbuffer {
   char* in_buffer;
 } buffer;

 typedef struct tc_buff
 {
     buffer* buff;
     int max;
     int capacity;
     size_t head;
     int tail;
     int generated;
     int sold;
 } c_buff;

 void cb_init(c_buff* cbuf, int max)
 {
     cbuf->buff = (buffer*)malloc(max); //1,25MB
     cbuf->capacity = 0; //ile mamy na stanie
     cbuf->head = 0;
     cbuf->tail = 0;
     cbuf->max = max/GEN_BLOCK_SIZE;
     cbuf->generated = 0; //co 5 sekund zmiana
     cbuf->sold = 0; //co 5 sekund zmiana
 }

 void cb_push(c_buff* cbuf, int data)
 {
     int idx = (cbuf->head) % cbuf->max;

     if(cbuf->capacity == cbuf->max)
         cbuf->tail++;
     else //add only if capacity < max
     {
       cbuf->capacity++;
       cbuf->head++;
       cbuf->generated++;

       //add 640 bytes
       cbuf->buff[idx].in_buffer = (char*)malloc(GEN_BLOCK_SIZE);
       memset(cbuf->buff[idx].in_buffer, data, GEN_BLOCK_SIZE);
     }
 }

 char* cb_pop(c_buff* cbuf, char* data)
 {
     int idx = (cbuf->tail) % cbuf->max;
     cbuf->capacity-=(SEN_BLOCK_SIZE/GEN_BLOCK_SIZE);
     cbuf->sold++;

     for(int i = 0; i < (SEN_BLOCK_SIZE/GEN_BLOCK_SIZE); i++) //192 blokow 640 == 112KB
       snprintf(data, sizeof(cbuf->buff[idx++].in_buffer), "%s", cbuf->buff[idx++].in_buffer);

     cbuf->tail += (SEN_BLOCK_SIZE/GEN_BLOCK_SIZE);

     if(START_PRODUCTION == 0)
     START_PRODUCTION = 1;

     return data;
 }

 /*****************************************************
  * CUSTOMERS QUEUE
  ****************************************************/

struct customers_q
{
   int fd; //for fd
   struct customers_q* next; /* pointer to next item */
};

/* Create our queue */
struct customers_q* queue = NULL;
struct customers_q* tempItem = NULL;
struct customers_q* first  = NULL;

static int q_size = 0;

/* Functions to manage queue */
void q_push(int fd)
{
   q_size++;
   tempItem = (struct customers_q*)malloc(sizeof(struct customers_q));
   (*tempItem).fd = fd;
   if(queue == NULL)
      first = tempItem;
   else
      (*queue).next = tempItem;

   queue = tempItem;
}

int q_pop()
{
   if(first != NULL)
   {
      q_size--;
      int temp_fd = first->fd;
      tempItem = (*first).next;

      //free(first);
      first = tempItem;
      if(first == NULL)
              queue = NULL;

      return temp_fd;
   }
   else
   {
     printf("Customers queue is empty, cannot pop...\n");
     return -1;
   }
}

//------------------------------------------------------
//------------------------------------------------------
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
    char* loc = "localhost\0";
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

//-------------- RAPORTS ----------------
void generate_raport(char* raport, c_buff* cb)
{
  char temp[2048];
  memset(raport, 0, 2048);
  struct timespec t_mono;
  struct timespec t_real;

  clock_gettime(CLOCK_MONOTONIC, &t_mono);
  clock_gettime(CLOCK_REALTIME, &t_real);

  strcat(raport, "Regular raport:\n");
  sprintf(temp, " -> Monotonic: %ldsec, %ldnsec\n", t_mono.tv_sec, t_mono.tv_nsec);
  strcat(raport, temp);
  sprintf(temp, " -> RealTime: %ldsec, %ldnsec\n", t_real.tv_sec, t_real.tv_nsec);
  strcat(raport, temp);
  sprintf(temp, " -> Connected: %d\n", connected);
  strcat(raport, temp);
  sprintf(temp, " -> In stock: %d :: Percentage-wise: %.2lf%% \n -> In last 5 secs generated: %d, sold: %d\n\n", cb->capacity*GEN_BLOCK_SIZE, (float)(cb->capacity)/(float)(cb->max)*100, cb->generated*GEN_BLOCK_SIZE, cb->sold*SEN_BLOCK_SIZE);
  strcat(raport, temp);
}

void gen_raport_1(char* raport, struct sockaddr_in B) //new connection
{
  char temp[2048];
  memset(raport, 0, 2048);
  struct timespec t_mono;
  struct timespec t_real;

  clock_gettime(CLOCK_MONOTONIC, &t_mono);
  clock_gettime(CLOCK_REALTIME, &t_real);

  strcat(raport, "New connection:\n");
  sprintf(temp, " -> Monotonic: %ldsec, %ldnsec\n", t_mono.tv_sec, t_mono.tv_nsec);
  strcat(raport, temp);
  sprintf(temp, " -> RealTime: %ldsec, %ldnsec\n", t_real.tv_sec, t_real.tv_nsec);
  strcat(raport, temp);

  sprintf(temp, " -> IP Address: %s \n\n", inet_ntoa(B.sin_addr));
  strcat(raport, temp);
}

void gen_raport_2(char* raport, struct dataraport data_raport) //lost connection
{
  char temp[2048];
  memset(raport, 0, 2048);
  struct timespec t_mono;
  struct timespec t_real;

  clock_gettime(CLOCK_MONOTONIC, &t_mono);
  clock_gettime(CLOCK_REALTIME, &t_real);

  strcat(raport, "Lost connection:\n");
  sprintf(temp, " -> Monotonic: %ldsec, %ldnsec\n", t_mono.tv_sec, t_mono.tv_nsec);
  strcat(raport, temp);
  sprintf(temp, " -> RealTime: %ldsec, %ldnsec\n", t_real.tv_sec, t_real.tv_nsec);
  strcat(raport, temp);
  sprintf(temp, " -> IP Address: %s \n", data_raport.ip_addr);
  strcat(raport, temp);
  sprintf(temp, " -> Ilosc przeslanych bloków: %d \n\n", data_raport.data_blocks-1);
  strcat(raport, temp);
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
              raport_path = (char*)malloc(sizeof(optarg));
              strcpy(raport_path, optarg);
              break;
          case 't':
              tempbuff = (char*)malloc(sizeof(optarg));
              strcpy(tempbuff, optarg);
              pace_val = convert_to_float18(tempbuff);
              break;
          case '?':
              display_help();
              break;
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
   * INIT BUFFER
   ******************************************/
    c_buff* cb;
    cb = (c_buff*)malloc(sizeof(c_buff)); //1 general buffer
    cb_init(cb, BUFF_SIZE);

    /************** CREATE RAPORT FILE ********************/
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int raport_fd = open(raport_path, O_WRONLY | O_CREAT, mode);

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

    if(inet_aton("127.0.0.1", &A.sin_addr) == -1)
    {
        perror("Aton inet error: Invalid address or address not supported\n");
        exit(EXIT_FAILURE);
    }

    if(bind(producer_fd, (const struct sockaddr*)&A, addr_lenA) == -1)
    {
        perror("bind error\n");
        exit(EXIT_FAILURE);
    }

    /************** POLL **************/
    uint64_t dtimer_ticks, rtimer_ticks;
    int returned_fds = 0;
    int consumer_counter = 0;
    int reallocs = 0;
    int consumer_max = 50;

    struct pollfd* pfds;
    pfds = (struct pollfd*)malloc(sizeof(struct pollfd) * 53); ////

    pfds[0].fd = dtimer_fd;
    pfds[0].events = POLLIN;
    pfds[1].fd = rtimer_fd;
    pfds[1].events = POLLIN;
    pfds[2].fd = producer_fd;
    pfds[2].events = POLLIN;

    int dticks_counter = 0;
    int rticks_counter = 0;

    /************** CREATE DATA RAPORT STRUCTURE ***************/
    struct dataraport* data_raport;
    data_raport = (struct dataraport*)malloc(sizeof(struct dataraport) * consumer_max); //!
    char* str_prod_point = str_loop;

    /***********************************************************
    * MAIN LOOP
    ***********************************************************/

    while(PRODUCTION)
    {
      returned_fds = poll(pfds, 53, 5000);

      if(returned_fds > 0)
      {
          if(pfds[0].revents == POLLIN)
          {
            if(START_PRODUCTION)
            {
              read(dtimer_fd, &dtimer_ticks, sizeof(dtimer_ticks));

            //generate data to buffer
            for(int k = 0; k < dtimer_ticks; k++)
            {
              if(*str_prod_point == '\0')
                str_prod_point = str_loop;

              cb_push(cb, *str_prod_point++);
            }

            dticks_counter++;
            returned_fds--;

            if(cb->capacity == cb->max)
              START_PRODUCTION = 0;
            }
          }

          if(pfds[1].revents == POLLIN)
          {
            read(rtimer_fd, &rtimer_ticks, sizeof(rtimer_ticks));

            char rapo[1024];
            memset(rapo, 0, sizeof(rapo));
            generate_raport(rapo, cb);

            write(raport_fd, rapo, strlen(rapo));

            rticks_counter++;
            returned_fds--;

            //set circle buffer vars = 0
            cb->generated = 0;
            cb->sold = 0;
          }

          if(pfds[2].revents) //if some actions on this socket -> listen and connect
          {
            if(listen(producer_fd, 50) == -1)
            {
                    perror("Cannot listen error\n");
                    exit(EXIT_FAILURE);
            }

            if((pfds[2].revents == POLLHUP) || (pfds[2].revents == (POLLHUP | POLLERR | POLLIN)))
              continue;

            if(consumer_counter == consumer_max) //realloc pfds structure
            {
              reallocs++;
              pfds = (struct pollfd*)realloc(pfds, (reallocs * consumer_max) * sizeof(struct pollfd));

              //realloc data_raport
              data_raport = (struct dataraport*)realloc(data_raport, (reallocs * consumer_max) * sizeof(struct dataraport));
            }

            //struct for sockaddress
            struct sockaddr_in B;
            int addr_lenB = sizeof(B);

            //add fd to poll struct
            pfds[consumer_counter+3].events = POLLIN;
            pfds[consumer_counter+3].fd  = accept(producer_fd, (struct sockaddr*)&B, (socklen_t*)&addr_lenB);
            if(pfds[consumer_counter+3].fd  == -1)
            {
                perror("Accept error\n");
                exit(EXIT_FAILURE);
            }

            //add to dataraport structure
            data_raport[consumer_counter].consumer_id = consumer_counter;
            data_raport[consumer_counter].ip_addr = inet_ntoa(B.sin_addr);
            data_raport[consumer_counter].data_blocks = 0;

            consumer_counter++;
            returned_fds--;
            connected++;

            //after new connection -> generate raport
            char rapo[1024];
            memset(rapo, 0, sizeof(rapo));
            gen_raport_1(rapo, B);
            write(raport_fd, rapo, strlen(rapo));
          }

          //other fds
          if(returned_fds > 0)
          {
            for(int i = 0; i < consumer_counter; i++)
            {
              if(pfds[i+3].revents == POLLIN)
              {
                //consumer sends 4 bytes
                char recvmes[4];
                read(pfds[i+3].fd, recvmes, sizeof(recvmes));

                q_push(pfds[i+3].fd);

                data_raport[i].data_blocks++; //requested

                returned_fds--;
                if(returned_fds == 0) //when we read all fds no need to continue
                  break;
              }
              else if(pfds[i+3].revents == (POLLHUP | POLLERR | POLLIN))
              {
                char rapo[512];
                memset(rapo, 0, sizeof(rapo));
                gen_raport_2(rapo, data_raport[i]);
                write(raport_fd, rapo, strlen(rapo));

                pfds[i+3].events = 0;
                pfds[i+3].revents = 0;
                shutdown(pfds[i+3].fd, 2);

                connected--;
              }
            }
          }

          // try to send as much data as we can
        if((q_size > 0) & (connected > 0) & (cb->capacity > ((SEN_BLOCK_SIZE/GEN_BLOCK_SIZE) + 1)))
        {
          int can_send = q_size;
          if((q_size*SEN_BLOCK_SIZE) > (cb->capacity*GEN_BLOCK_SIZE))
            {
              can_send = cb->capacity/(SEN_BLOCK_SIZE/GEN_BLOCK_SIZE);
            }

          for(int j = 0; j < can_send; j++) // /192
          {
            int cfd = q_pop();

            char send_b[SEN_BLOCK_SIZE]; //SEN_BLOCK_SIZE
            memset(send_b, 0, SEN_BLOCK_SIZE);

            cb_pop(cb, send_b);

            send(cfd, send_b, sizeof(send_b), 0);
          }
        }
      }
    }

    close(dtimer_fd);
    close(rtimer_fd);
    close(producer_fd);

    return 0;
}
