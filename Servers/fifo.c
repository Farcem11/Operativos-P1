//gcc -W -Wall -o fifo fifo.c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

char* getFileExtension(char *pFileName) 
{
    char *dot = strchr(pFileName, '.');

    if(!dot || dot == pFileName) 
		return "";
    return dot + 1;
}

char* getSubStringLeft(char* str, char* delimiter)
{
	char* dest = calloc(strlen(str), sizeof(char));
	int num = strstr(str, delimiter) - str ;
	strncpy(dest, str, num);
	return dest;
}

char* getSubStringRight(char* str, char* delimiter)
{
	char* dest = calloc(strlen(str), sizeof(char));
	strcpy(dest, str);
	int num = strstr(str, delimiter) - str ;
	dest = dest + num;
	return ++dest;
}

void getHttpHeaderType(char *pFileName, int* connfd)
{

	if (strcmp(getFileExtension(pFileName), "html") == 0)
	{
		write(*connfd, "text/html\r\n", strlen("text/html\r\n"));
	}
	else if (strcmp(getFileExtension(pFileName), "jpg") == 0 || strcmp(getFileExtension(pFileName), "jpeg") == 0)
	{
		write(*connfd, "image/jpeg\r\n", strlen("image/jpeg\r\n"));
	}
	else if (strcmp(getFileExtension(pFileName), "png") == 0)
	{
		write(*connfd, "image/png\r\n", strlen("image/png\r\n"));
	}
	else if (strcmp(getFileExtension(pFileName), "bmp") == 0)
	{
		write(*connfd, "image/bmp\r\n", strlen("image/bmp\r\n"));
	}
	else
	{
		write(*connfd, "text/plain\r\n", strlen("text/plain\r\n"));
	}
}

int main()
{
	int Mbs = 20*1024*1024; //20 mbs maximo
    int listenfd = 0; 
    int connfd = 0;
    char filePath[125];
    struct sockaddr_in serv_addr; 
    char sendBuff[2048];
    socklen_t serv_len;
    char* fileNameFromBrowser = calloc(1025, sizeof(char));
    char* fileNameFromClient =  calloc(1025, sizeof(char));

	FILE *file;

	unsigned long fileLen;
	unsigned char* imageData = calloc(Mbs, sizeof(unsigned char));

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '\0', sizeof(serv_addr));
    memset(sendBuff, '\0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8000); 

    printf("%s\n", "Binding socket...");
    while(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0);
	printf("%s\n", "Socket binded, listening at port 8000"); 

    listen(listenfd, 100);

    while(1)
    {
    	connfd = accept(listenfd, (struct sockaddr*)&serv_addr, &serv_len);

    	int readed = recv(connfd, sendBuff, sizeof(sendBuff), 0);
    	if(readed > 0 && strcmp(getSubStringLeft(getSubStringRight(sendBuff,"/")," "), "favicon.ico") != 0)
    	{
			if(strcmp(getSubStringLeft(sendBuff," "), "/client") != 0)
			{
				//Browser
				fileNameFromBrowser = getSubStringRight(sendBuff,"/");
				fileNameFromBrowser = getSubStringLeft(fileNameFromBrowser," ");
				printf("Respondiendo a la solicitud del archivo: %s, desde el browser\n", fileNameFromBrowser);
				strcpy(filePath, "Files/");
				strcat(filePath, fileNameFromBrowser);

				if( access(filePath, F_OK) != -1 )
				{
					file = fopen(filePath,"rb");

					fseek(file, 0, SEEK_END);
					fileLen = ftell(file);
					fseek(file, 0, SEEK_SET);
					
					fread(imageData,1,fileLen,file);

					fclose(file);

				  	write(connfd, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"));
				  	write(connfd, "Content-Type: ", strlen("Content-Type: "));
					
				  	getHttpHeaderType(fileNameFromBrowser, &connfd);
				  	
				  	write(connfd, "Connection: keep-alive", strlen("Connection: keep-alive"));
				  	write(connfd, "\r\n\r\n", strlen("\r\n\r\n"));
				  	write(connfd, imageData, fileLen);
				}
				else
				{
					file = fopen("Files/404.jpg","rb");

					fseek(file, 0, SEEK_END);
					fileLen = ftell(file);
					fseek(file, 0, SEEK_SET);
					
					fread(imageData,1,fileLen,file);

					fclose(file);

				  	write(connfd, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"));
				  	write(connfd, "Content-Type: ", strlen("Content-Type: "));
					
				  	getHttpHeaderType("404.jpg", &connfd);
				  	
				  	write(connfd, "Connection: keep-alive", strlen("Connection: keep-alive"));
				  	write(connfd, "\r\n\r\n", strlen("\r\n\r\n"));
				  	write(connfd, imageData, fileLen);
				}
			}
			else
			{
				//Client
				fileNameFromClient = getSubStringRight(sendBuff," ");
		        //Receive file name

				printf("Respondiendo a la solicitud del archivo: %s, desde el cliente\n", fileNameFromClient);
				
				strcpy(filePath, "Files/");
		        strcat(filePath, fileNameFromClient);

		        //Open the file that we wish to transfer
		        
		        if( access(filePath, F_OK) == -1 )
		        {
		            printf("Error opening file. File does not exist\n");
		        }
		        else
		        {
					file = fopen(filePath,"rb");

					fseek(file, 0, SEEK_END);
					fileLen = ftell(file);
					fseek(file, 0, SEEK_SET);
					
					fread(imageData,1,fileLen,file);

					fclose(file);

				  	send(connfd, imageData, fileLen, 0);
		        }
			}
			memset(imageData, '\0', Mbs);
			memset(filePath, '\0', sizeof(filePath)); 
			memset(sendBuff, '\0', sizeof(sendBuff)); 
		}
		close(connfd);
		sleep(1);
    }
}
