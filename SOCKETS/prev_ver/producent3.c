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

//#define PORT 12345
#define NANOSEC 1000000
#define BUFF_SIZE 1024*1024*1.25
#define DATA_SIZE 640

static int connected = 0;

struct dataraport {
  int consumer_id;
  char* ip_addr;
  int data_blocks;
};

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

//-------------- RAPORTS ----------------
void generate_raport(char* raport)
{
  char temp[1024];
  memset(raport, 0, 1024);
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
  sprintf(temp, " -> Storage: \n\n");
  strcat(raport, temp);
}

void gen_raport_1(char* raport, struct sockaddr_in B) //new connection
{
  char temp[1024];
  memset(raport, 0, 1024);
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
  char temp[1024];
  memset(raport, 0, 1024);
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
   * INIT BUFFER TODO:
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

    /************** POLL **************/
    uint64_t dtimer_ticks, rtimer_ticks;
    char read_data[8];
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

    int dticks_counter = 0; //??
    int rticks_counter = 0; //??

    /************** CREATE DATA RAPORT STRUCTURE ***************/
    struct dataraport* data_raport;
    data_raport = malloc(sizeof(struct dataraport) * consumer_max);

    /***********************************************************
    * MAIN LOOP
    ***********************************************************/

    while(1)
    {
      returned_fds = poll(pfds, 10, 5000);

      if(returned_fds > 0)
      {
          if(pfds[0].revents == POLLIN)
          {
            read(dtimer_fd, &dtimer_ticks, sizeof(dtimer_ticks));

            //for loop
            cb_push(cb, 'A');

            dticks_counter++;
            returned_fds--;
          }

          if(pfds[1].revents == POLLIN)
          {
            read(rtimer_fd, &rtimer_ticks, sizeof(rtimer_ticks));

            char rapo[1024];
            memset(rapo, 0, sizeof(rapo));
            generate_raport(rapo);

            write(raport_fd, rapo, sizeof(rapo));

            rticks_counter++;
            returned_fds--;
            //TODO: set circle buffer vars = 0
            //cb->generated = 0;
            //cb->sold = 0;
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
            write(raport_fd, rapo, sizeof(rapo));
          }

          //każda dalsza ilość fds, trzeba przeleciec wszystkie
          if(returned_fds > 0)
          {
            for(int i = 0; i < consumer_counter; i++)
            {
              if(pfds[i+3].revents == POLLIN)
              {
                //konsument wysłał, sprawdzamy, czy 4 bajty, jak tak, to wysyłamy porcję danych
                char recvmes[4];
                char* s = "cos tam\n";
                read(pfds[i+3].fd, recvmes, sizeof(recvmes));

                //TODO: sprawdzić, czy wygenerowalismy odpowiednia ilosc danych, jak nie to robimy break, żeby się wygenerowały
                // jeżeli nie, ustawiamy wszystkie .events na 0, jak sie wygeneruje już, to spowrotem na IN
                send(pfds[i+3].fd, s, sizeof(s), 0 ); //powinno być osobno, pod tym ifem, jeżeli w kolejce, to wysylamy

                //add 1 block to structure
                data_raport[i].data_blocks++;

                returned_fds--;
                if(returned_fds == 0) //when we read all fds no need to continue
                  break;
              }
              else if(pfds[i+3].revents == (POLLHUP | POLLERR | POLLIN))
              {
                char rapo[1024];
                memset(rapo, 0, sizeof(rapo));
                gen_raport_2(rapo, data_raport[i]);
                write(raport_fd, rapo, sizeof(rapo));

                pfds[i+3].events = 0;
                pfds[i+3].revents = 0;
                close(pfds[i+3].fd);

                connected--;
              }
            }

            //tutaj bedzie pop z wysyłaniem wiadomości
            //if capacity>112KB pop
            //while(!pop)

          }
      }

      sleep(1);
    }

    close(dtimer_fd);
    close(rtimer_fd);
    close(producer_fd);


    return 0;
}
