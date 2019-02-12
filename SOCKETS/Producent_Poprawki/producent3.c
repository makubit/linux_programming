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

static long int NANOSEC = 1000000000;
#define BUFF_SIZE 1024*1024*1.25
#define GEN_BLOCK_SIZE 640
#define SEN_BLOCK_SIZE 112*1024
// :640 = 179

char* str_loop = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static int PRODUCTION = 1;
static int START_PRODUCTION = 1;
static int connected = 0;
static int consumer_max = 50;

struct dataraport {
  int consumer_id;
  char* ip_addr;
  int data_blocks;
};

/*****************************************************
 * CIRCULAR BUFFER
 ****************************************************/

 typedef struct tc_buff
 {
     char* buff;
     int max;
     int capacity;
     size_t head;
     int tail;
     int generated;
     int sold;
 } c_buff;

 void cb_init(c_buff* cbuf, int max)
 {
     cbuf->buff = (char*)malloc(max);
     cbuf->capacity = 0; //ile mamy na stanie
     cbuf->head = 0;
     cbuf->tail = 0;
     cbuf->max = max;
     cbuf->generated = 0; //co 5 sekund zmiana
     cbuf->sold = 0; //co 5 sekund zmiana
 }

 void cb_push(c_buff* cbuf, int data)
 {
     if((cbuf->head + (GEN_BLOCK_SIZE)) >= cbuf->max)
     {
        char* temp = (char*)malloc((GEN_BLOCK_SIZE) * sizeof(char));
        memset(temp, data, GEN_BLOCK_SIZE);
        memcpy(&cbuf->buff[cbuf->head], temp, (GEN_BLOCK_SIZE));
        cbuf->head = 0;
        cbuf->capacity+=(GEN_BLOCK_SIZE);
     }
     else //add only if capacity < max
     {
       //add 640 bytes
       char* temp = (char*)malloc((GEN_BLOCK_SIZE)*sizeof(char));
       memset(temp, data, (GEN_BLOCK_SIZE));
       memcpy(&cbuf->buff[cbuf->head], temp, (GEN_BLOCK_SIZE));

       cbuf->capacity+=(GEN_BLOCK_SIZE);
       cbuf->head+=(GEN_BLOCK_SIZE);
       cbuf->generated++;
     }
 }

 char* cb_pop(c_buff* cbuf, char* data)
 {
     cbuf->capacity-=(SEN_BLOCK_SIZE); //
     cbuf->sold++;

     if((cbuf->tail + (SEN_BLOCK_SIZE)) >= cbuf->max)
       cbuf->tail = 0;

     memcpy(data, &cbuf->buff[cbuf->tail], (SEN_BLOCK_SIZE));

     cbuf->tail += (SEN_BLOCK_SIZE);

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
  sprintf(temp, " -> In stock: %d :: Percentage-wise: %.2lf%% \n -> In last 5 secs generated: %d, sold: %d\n\n", cb->capacity, (float)(cb->capacity)/(float)(cb->max)*100, cb->generated*(GEN_BLOCK_SIZE), cb->sold*(SEN_BLOCK_SIZE));
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
  sprintf(temp, " -> Ilosc przeslanych bloków: %d \n\n", data_raport.data_blocks);
  strcat(raport, temp);
}

 void do_getopt(int argc, char* argv[], int* port, char** addr, char** raport_path, float* pace_val)
 {
   int c;
   char* tempbuff = NULL;

   while((c = getopt(argc, argv, "r:t:")) != -1)
     {
       switch(c)
       {
           case 'r':
               *raport_path = (char*)malloc(sizeof(optarg));
               strcpy(*raport_path, optarg);
               break;
           case 't':
               tempbuff = (char*)malloc(sizeof(optarg));
               strcpy(tempbuff, optarg);
               *pace_val = convert_to_float18(tempbuff);
               break;
           case '?':
               display_help();
               break;
           break;
       }
     }

     if((*raport_path == NULL) || (*pace_val == 0) || (argv[optind] == NULL))
     {
       printf(" Mandatory parameters: -r <path>, -t <val>, [<addr>]:port\n");
       display_help();
       exit(EXIT_FAILURE);
     }

     //---------------------------------------------
     if(argv[optind][0] == '[')
     {
       char* first_par = strtok(argv[optind], "[");
       first_par = strtok(first_par, ":]");
       char* second_par = strtok(0, ":]");
       strcpy(first_par, *addr);

       *port = strtol(second_par, NULL, 0);

       if(*port <= 0)
       {
         printf(" port error: wrong port number\n");
         display_help();
         exit(EXIT_FAILURE);
       }
     }
     else {
       *port = strtol(argv[optind], NULL, 0);

       if(*port <= 0)
       {
         printf(" port error: wrong port number\n");
         display_help();
         exit(EXIT_FAILURE);
       }
     }
 }

 void set_timersfd(struct itimerspec* dts, struct itimerspec* rts, float pace_val, int dtimer_fd, int rtimer_fd)
 {
   dts->it_interval.tv_sec = (long)(pace_val);
   dts->it_interval.tv_nsec = (long)(pace_val * NANOSEC) % NANOSEC;
   dts->it_value.tv_sec = (long)(pace_val);
   dts->it_value.tv_nsec = (long)(pace_val * NANOSEC) % NANOSEC;

   rts->it_interval.tv_sec = 5;
   rts->it_interval.tv_nsec = 0;
   rts->it_value.tv_sec = 5;
   rts->it_value.tv_nsec = 0;

   if(timerfd_settime(dtimer_fd, 0, dts, NULL) < 0)
   {
       perror("Set time in dtimer error\n");
       close(dtimer_fd);
       close(rtimer_fd);
       exit(EXIT_FAILURE);
   }

   if(timerfd_settime(rtimer_fd, 0, rts, NULL) < 0)
   {
       perror("Set time in rtimer error\n");
       close(dtimer_fd);
       close(rtimer_fd);
       exit(EXIT_FAILURE);
   }
 }

 void create_socket(struct sockaddr_in* A, int port, char* addr, int producer_fd)
 {
   socklen_t addr_lenA = sizeof(*A);

   A->sin_family = AF_INET;
   A->sin_port = htons(port);

   if(inet_aton(addr, &(A->sin_addr)) == -1)
   {
       perror("Aton inet error: Invalid address or address not supported\n");
       exit(EXIT_FAILURE);
   }

   if(bind(producer_fd, (const struct sockaddr*)A, addr_lenA) == -1)
   {
       perror("bind error\n");
       exit(EXIT_FAILURE);
   }
 }

 void set_pollfds(struct pollfd* pfds, int dtimer_fd, int rtimer_fd, int producer_fd)
 {
   pfds[0].fd = dtimer_fd;
   pfds[0].events = POLLIN;
   pfds[1].fd = rtimer_fd;
   pfds[1].events = POLLIN;
   pfds[2].fd = producer_fd;
   pfds[2].events = POLLIN;
 }

//------------------------ MAIN LOOP -----------------------
//----------------------------------------------------------

void do_poll_loop(int dtimer_fd, int rtimer_fd, int producer_fd, c_buff* cb, struct dataraport* data_raport, int raport_fd)
{
  /************** POLL **************/
  uint64_t dtimer_ticks, rtimer_ticks;
  int returned_fds = 0;
  int consumer_counter = 0;
  int reallocs = 0;

  char* str_prod_point = str_loop;

  struct pollfd* pfds;
  pfds = (struct pollfd*)malloc(sizeof(struct pollfd) * 53); ////
  set_pollfds(pfds, dtimer_fd, rtimer_fd, producer_fd);

  int dticks_counter = 0;
  int rticks_counter = 0;

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
            //for(int k = 0; k < dtimer_ticks; k++)
            //{
              if(*str_prod_point == '\0')
                str_prod_point = str_loop;

              cb_push(cb, *str_prod_point++);
            //}
            dticks_counter++;
            returned_fds--;

          if(cb->capacity >= cb->max)
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
              if(!recv(pfds[i+3].fd, recvmes, sizeof(recvmes), 0))
              {
                char rapo[512];
                memset(rapo, 0, sizeof(rapo));
                gen_raport_2(rapo, data_raport[i]);
                write(raport_fd, rapo, strlen(rapo));

                close(pfds[i+3].fd);
                pfds[i+3].fd = 0;
                pfds[i+3].events = 0;

                connected--;
              }
              else {
                q_push(pfds[i+3].fd);

                data_raport[i].data_blocks++; //requested

                returned_fds--;
                if(returned_fds == 0) //when we read all fds no need to continue
                  break;
              }
            }
          }
        }

        // try to send as much data as we can
        if((q_size > 0) & (connected > 0) & (cb->capacity > (SEN_BLOCK_SIZE)))
        {
          int can_send = q_size;
          if((q_size*(SEN_BLOCK_SIZE)) > (cb->capacity))
              can_send = cb->capacity/(SEN_BLOCK_SIZE);

            printf("cansend: %d, %d\n", can_send, cb->capacity);

            for(int j = 0; j < can_send; j++) // /192
            {
              int cfd = q_pop();

              char send_b[SEN_BLOCK_SIZE];
              memset(send_b, 0, (SEN_BLOCK_SIZE));

              cb_pop(cb, send_b);
              send(cfd, send_b, (SEN_BLOCK_SIZE), 0);
            }
          }
      }
  }
}

//----------------------------------------------------------
//----------------------------------------------------------

int main(int argc, char* argv[])
{
    char* raport_path = NULL;
    float pace_val = 0;
    int port = 0;
    char* addr = "localhost";

    do_getopt(argc, argv, &port, &addr, &raport_path, &pace_val);

    /**************** INIT BUFFER *************************/
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
    set_timersfd(&dts, &rts, pace_val, dtimer_fd, rtimer_fd);

    /************* CREATE SOCKET **********/
    struct sockaddr_in A;
    create_socket(&A, port, addr, producer_fd);

    /************** CREATE DATA RAPORT STRUCTURE ***************/
    struct dataraport* data_raport;
    data_raport = (struct dataraport*)malloc(sizeof(struct dataraport) * consumer_max); //!

    /***********************************************************
    * MAIN LOOP
    ***********************************************************/
    do_poll_loop(dtimer_fd, rtimer_fd, producer_fd, cb, data_raport, raport_fd);

    close(dtimer_fd);
    close(rtimer_fd);
    close(producer_fd);

    return 0;
}
