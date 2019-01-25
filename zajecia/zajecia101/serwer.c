#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345

int main(int argc, char* argv[])
{
  //AF_INET
  //struct sockaddr_in A;
  //char*Host
  //inet_aton(Host, &A.sin_addr)
  
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        perror("creating socket error\n");
        exit(1);
    }
    

    struct sockaddr_in A; 
    socklen_t addr_len = sizeof(A);
    
    A.sin_family = AF_INET;
    A.sin_port = htons(PORT);
    //A.sin_addr =

    if(inet_aton(argv[1], &a.sin_addr) == -1)
    {
        perror("inet error\n");
        exit(2);
    }

    if(bind(sockfd, (const struct sockaddr*)&A, addr_len) == -1)
    {
        perror("bind perror\n");
        exit(3);
    }

    int time = 10;
    
    while(time--)
        if(listen(sockfd, 50) == -1)
            perror("liten error\n");

    struct sockaddr_in B;
    socklen_t addr_B = sizeof(B);

    if(new_fd = accept(sockfd, (const struct sockaddr*)&B, addr_B) == -1)
        perror("accept error\n");

    char buf[100];
    
    read(new_fd, buf, sizeof(buf));

    

    return 0;
}

/*int polaczenie(int sock_fd)
{
    struct sockaddr_in peer;
    socklen_t addrlen;

    int new_socket = accpet, sockfd, (struct sockaddr *)&peer, &addr
}*/

/*void rejestracja(int sockfd, char* Host, in_port_t Port)
{
    struct sockaddr_in A;

    A.sin_family = 
}*/
