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

#define PORT 12345
#define BUFF_SIZE 1024*1024*1.25

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

//----------------------------------------------------------

int main(int argc, char* argv[])
{
    c_buff* cb;
    cb = malloc(sizeof(c_buff));

    cb_init(cb, 100);

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
     * NAWIAZANIE POLACZENIA *
     *******************************************************/


    int producer_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(producer_fd == -1)
    {
        perror("Creating producer error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in A;
    socklen_t addr_lenA = sizeof(A);

    A.sin_family = AF_INET;
    A.sin_port = htons(PORT);

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

    if(listen(producer_fd, 50) == -1)
    {  
        perror("Listen error\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in B;
    int addr_lenB = sizeof(B);

    int consumer_fd = accept(producer_fd, (struct sockaddr*)&A, (socklen_t*)&addr_lenA);
    if(consumer_fd == -1)
    {
        perror("Accept error\n");
        exit(EXIT_FAILURE);
    }

    char buff[100];
    char* s = "Wysylam";
    memset(buff, 0, sizeof(buff));
    printf("accepted\n");

    //read(consumer_fd, buff, sizeof(buff));
    send(consumer_fd,s, sizeof(s), 0 );

    printf("%s\n\n", buff);
    


    return 0;
}
