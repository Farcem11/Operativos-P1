/* 
	gcc -pedantic -W -Wall logic.c -lm -o app
	./app
*/

#include <netinet/in.h>    
#include <stdio.h>    
#include <stdlib.h>    
#include <sys/socket.h>    
#include <sys/stat.h>    
#include <sys/types.h>    
#include <unistd.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

char webpage[] = 
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html>"
	"<body>"
		"<img src='/images/image.jpg'/>"
		"<H1>Hello world</H1>"
	"</body>"
"</html>";

int main() 
{
/*
    int x,y,n;
    unsigned char *data = stbi_load("image.jpg", &x, &y, &n, 0);
    printf("X = %d - Y = %d - N = %d\n", x, y, n);
    printf("%s\n", data);
    stbi_image_free(data);
*/
   int create_socket, new_socket;    
   socklen_t addrlen;    
   int bufsize = 1024;    
   char *buffer = malloc(bufsize);    
   struct sockaddr_in address;    
 
   if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) > 0)
   {    
      printf("The socket was created\n");
   }
    
   address.sin_family = AF_INET;    
   address.sin_addr.s_addr = INADDR_ANY;    
   address.sin_port = htons(8000);    
    
   if (bind(create_socket, (struct sockaddr *) &address, sizeof(address)) == 0)
   {    
      printf("Binding Socket\n");
   }
    
    
   while (1) 
   {  
		if (listen(create_socket, 10) < 0) 
		{    
			perror("server: listen");    
			exit(1);    
		}    

		if ((new_socket = accept(create_socket, (struct sockaddr *) &address, &addrlen)) < 0) 
		{    
			perror("server: accept");    
			exit(1);    
		}    

		if (new_socket > 0)
		{    
			printf("The Client is connected...\n");
		}

		recv(new_socket, buffer, bufsize, 0);    
		printf("%s\n", buffer);    
      	write(new_socket, webpage, sizeof(webpage) - 1);
		close(new_socket);
   }    
   close(create_socket);    
   return 0;    
}
