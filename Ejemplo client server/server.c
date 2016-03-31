/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
/*http://www.linuxhowtos.org/C_C++/socket.htm*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg);

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, n, counter;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;

     if (argc != 2) {
         printf("Setting default port (8000)\n");
	 portno = 8000;
     } else {
	 portno = atoi(argv[1]);
     }

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");

     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     
     //Transferencia de datos
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
        (struct sockaddr *) &cli_addr, 
        &clilen);
     if (newsockfd < 0) 
        error("ERROR on accept");
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0)
	error("ERROR reading from socket");
     n = write(newsockfd,"I got your message",18);
     if (n < 0)
	error("ERROR writing to socket");
     printf("Here is the message: %s\n",buffer);

     //Cierre de socket
     close(newsockfd);
     close(sockfd);
     return 0; 
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
