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
#define GEN_BLOCK_SIZE 640
#define SEN_BLOCK_SIZE 120*1024
// :640 = 192

typedef struct buffer {
  char* buffer;
} buffer;

typedef struct c_buff
{
    buffer* buffer;
    int max;
    int capacity;
    size_t head;
    int tail;
    int generated;
    int sold;

} c_buff;

void cb_init(c_buff* cbuf, int max)
{
    cbuf->buffer = malloc(max/GEN_BLOCK_SIZE); //1,25MB
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
        cbuf->tail++; //nadpisujemy dane
    cbuf->head++;
    cbuf->capacity++;
    cbuf->generated++;

    //dodajemy pojedyncze 640bajtów
    cbuf->buffer[idx].buffer = malloc(GEN_BLOCK_SIZE);
    memset(cbuf->buffer[idx].buffer, data, GEN_BLOCK_SIZE);
}

int cb_pop(c_buff* cbuf, char* temp)
{
    int idx = (cbuf->tail) % cbuf->max;
    cbuf->capacity--;
    cbuf->sold++;

    for(int i = 0; i < (SEN_BLOCK_SIZE/GEN_BLOCK_SIZE); i++)
      strcat(temp, cbuf->buffer[cbuf->tail++].buffer);

    return temp;
}

int main(int argc, char* argv[])
{
  c_buff* cb;
  cb = malloc(sizeof(c_buff));

  cb_init(cb, BUFF_SIZE);

  /*cb_push(cb, 'A');
  cb_push(cb, 'B');
  cb_push(cb, 'C');
  printf("%s, %s\n", cb->buffer[0], cb->buffer[1]);
  char* temp = malloc(2*640);
  cb_pop(cb, temp);
  printf("%d, %s\n", cb->capacity, temp);//, cb_pop(cb));*/
/*
char* t = malloc(100);
  strcpy(t, "some string to test");
  char* t2 = "ada";
  //memcpy(t[6], t2, strlen(t2));
  strcat(t, t2);
  printf("%s\n", t);*/
  char* str_loop = "abcdefghijklmnopqrstuwxyzABCDEFGHIJKLMNOPQRSTUWXYZ";

  char* str_prod_point = str_loop;
  while(1)
  {


    if(*str_prod_point == '\0')
      str_prod_point = str_loop;

    printf("%c\n", *str_prod_point);

    str_prod_point++;

    sleep(1);

}
  return 0;
}
