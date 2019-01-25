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

#define PORT 12345

int main(int argc, char* argv[])
{
   int consumer_fd = socket(AF_INET, SOCK_STREAM, 0);
   if(consumer_fd == -1)
   {
       perror("Creating consumer error\n");
       exit(EXIT_FAILURE);
   }

   struct sockaddr_in A;
   socklen_t addr_lenA = sizeof(A);

   A.sin_family = AF_INET;
   A.sin_port = htons(PORT);

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

   char* buff = "Read da message\n";
   send(consumer_fd, buff, strlen(buff), 0);
   printf("Message sent\n");





    return 0;
}
