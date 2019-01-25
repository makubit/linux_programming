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

int main(int argc, char* argv[])
{
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
    memset(buff, 0, sizeof(buff));
    printf("accepted\n");

    read(consumer_fd, buff, sizeof(buff));

    printf("%s\n\n", buff);
    


    return 0;
}
