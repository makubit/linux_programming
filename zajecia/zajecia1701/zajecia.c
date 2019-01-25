/* * * *
 * Cwiczenia 17.01.2019
 *  serwer klient -> serwer oczekuje aż w strumieniu pojawi się napis 'hello', to serwer robi timestamp i wysyła w formie tekstowej (clock_gettime);
 * Prosty kient, który wyśle odbierze, może przyjąc parametr, który będzie ograniczał pętlę wysyłania 'hello' do serwera
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345

int main(int argc, char* argv[])
{
   int server_fd;
   int new_fd;
   int n;
   char buff[100];
   char* message = "Hello\n";
   ssize_t length;

   if((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
       perror("socket sserver_fd error\n");

   struct sockaddr_in A;
   A.sin_family = AF_INET;
   A.sin_port = htons(PORT);

   if(inet_aton(argv[1], &A.sin_addr) == -1)
       perror("inet aton errror\n");

   if(bind(server_fd, (struct sockaddr*)&A, sizeof(struct sockaddr_in)) == -1)
       perror("bind error\n");

   struct sockaddr_in B;
   socklen_t Blen = sizeof(B);

   n = recvfrom(server_fd, &buff, length, NULL, (struct sockaddr*)&B, (socklen_t *)&Blen);
   printf("%s", buff);



    return 0;
}
