//nc -u 127.0.0.1 12345
//ljakfhdskjfhafs
//
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#define PORT 12345

int main(int argc, char* argv[])
{
   ssize_t n;
   int klient_fd;
   char* message = "Hello\n";
   char buff[100];

   if((klient_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
       perror("klient_fd socket error\n");

   struct sockaddr_in A;
   A.sin_family = AF_INET;
   A.sin_port = htons(PORT);

   if(inet_aton(argv[1], &A.sin_addr) == -1)
       perror("inet_aton error\n");

   socklen_t A_len = sizeof(A);
   n = sendto(klient_fd, message, sizeof(message), NULL, (struct sockaddr*)&A, A_len);
   printf("send\n");


    return 0;
}
