//cd Documents/Operativos-P1/Servers | gcc -W -Wall -pthread -o thread thread.c | ./thread


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <assert.h>
#include <sched.h>
#include <errno.h>
#include <pthread.h>

int listenfd = 0; 
int connfd = 0;
struct sockaddr_in serv_addr; 
socklen_t serv_len;

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

void* connection_handler(void* socket_desc)
{
	printf("Entered handler!\n");
	int connfd = *(int*)socket_desc;
	char sendBuff[512] = {0};
	int readed = recv(connfd, sendBuff, sizeof(sendBuff), 0);
	if(readed > 0)
	{
		char* fileNameFromBrowser = calloc(256, sizeof(char));

		fileNameFromBrowser = getSubStringRight(sendBuff,"/");
		fileNameFromBrowser = getSubStringLeft(fileNameFromBrowser," ");

		if(strcmp(fileNameFromBrowser, "client") != 0)
			printf("Se solicito el archivo: %s\n", fileNameFromBrowser);
		else
			printf("Se solicito el archivo: %s\n", getSubStringRight(sendBuff," "));

		unsigned char* imageData = (unsigned char*)calloc(256, sizeof(unsigned char));
		char* fileNameFromClient = calloc(256, sizeof(char));
		char filePath[125] = {0};
		char confirmBuff[32] = {0};
		unsigned char buff[256] = {0};
		
		if(strcmp(getSubStringLeft(sendBuff," "), "/client") != 0)
		{
			//Browser

			strcpy(filePath, "Files/");
			strcat(filePath, fileNameFromBrowser);
			FILE* file;
			if( access(filePath, F_OK) != -1 )
			{
				file = fopen(filePath,"rb");
				
			  	write(connfd, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"));
			  	write(connfd, "Content-Type: ", strlen("Content-Type: "));
				
			  	getHttpHeaderType(fileNameFromBrowser, &connfd);
			  	
			  	write(connfd, "Connection: keep-alive", strlen("Connection: keep-alive"));
			  	write(connfd, "\r\n\r\n", strlen("\r\n\r\n"));

			  	while(1)
	            {
	                //First read file in chunks of 256 bytes
	                bzero(imageData,256);
	                int nread = fread(imageData,1,256,file);        

	                //If read was success, send data.
	                if(nread > 0)
	                {
	                    write(connfd, imageData, nread);
	                }

	                //There is something tricky going on with read .. 
	                //Either there was error, or we reached end of file.
	                if (nread < 256)
	                {
	                    break;
	                }
	            }
				fclose(file);
			}
			else
			{
				file = fopen("Files/404.jpg","rb");
				
			  	write(connfd, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"));
			  	write(connfd, "Content-Type: ", strlen("Content-Type: "));
				
			  	getHttpHeaderType("404.jpg", &connfd);
			  	
			  	write(connfd, "Connection: keep-alive", strlen("Connection: keep-alive"));
			  	write(connfd, "\r\n\r\n", strlen("\r\n\r\n"));

			  	while(1)
	            {
	                //First read file in chunks of 256 bytes
	                bzero(imageData,256);
	                int nread = fread(imageData,1,256,file);        

	                //If read was success, send data.
	                if(nread > 0)
	                {
	                    write(connfd, imageData, nread);
	                }

	                //There is something tricky going on with read .. 
	                //Either there was error, or we reached end of file.
	                if (nread < 256)
	                {
	                    break;
	                }
	            }
				fclose(file);
			}
		}
		else
		{
			//Client
			fileNameFromClient = getSubStringRight(sendBuff," ");
	        //Receive file name
			
			strcpy(filePath, "Files/");
	        strcat(filePath, fileNameFromClient);

	        //Open the file that we wish to transfer
	        FILE* fp;
	        if( access(filePath, F_OK) == -1 )
	        {
	            printf("Error opening file. File does not exist\n");
	            strcpy(confirmBuff, "1");
	            write(connfd,confirmBuff,sizeof(confirmBuff));
	        }
	        else
	        {
	        	fp = fopen(filePath,"rb");
	            strcpy(confirmBuff, "0");

	            write(connfd,confirmBuff,sizeof(confirmBuff));
	            //Read data from file and send it
	            while(1)
	            {
	                //First read file in chunks of 256 bytes
	                bzero(buff,256);
	                int nread = fread(buff,1,256,fp);        

	                //If read was success, send data.
	                if(nread > 0)
	                {
	                    write(connfd, buff, nread);
	                }

	                //There is something tricky going on with read .. 
	                //Either there was error, or we reached end of file.
	                if (nread < 256)
	                {
	                    if (ferror(fp))
	                        printf("Error reading\n");
	                    break;
	                }
	            }
	            fclose(fp);
	        }
		}
    }
    close(connfd);
    printf("Out handler\n");
	return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2){
    	printf("ERROR: Input must be like this: ./prethread numberOfThreads\n");
    	return 1;
    }

    pthread_t *threadCreator; //Thread to set
    int iret; //Thread creation code
    int numberOfThreads = atoi(argv[1]); //Number
    int *threadActiveList;
    pthread_t *threadList[numberOfThreads];

    threadActiveList = (int*) calloc (numberOfThreads,sizeof(int));

    bzero(threadList, sizeof(numberOfThreads));

    //Assign thread on list
    for(int threadCount = 0; threadCount < numberOfThreads; threadCount++){
    	threadList[threadCount] = threadCreator;   	
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '\0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8000); 

    printf("%s\n", "\nBinding socket...");
    while(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0);
	printf("%s\n", "Socket binded\n"); 

    listen(listenfd,25);
    
    while(1)
    {
    	int connfd = accept(listenfd, (struct sockaddr*)&serv_addr, &serv_len);

    	for (int threadCount = 0; threadCount < numberOfThreads; threadCount++)
	    {
	        if(pthread_kill(threadList[threadCount], 0) == 0)
	        {
	            printf("Thread no %d is still running.\n", threadCount);
	        }
	        else 
	        {
	            //pthread_join(threadListToCheck[threadCount], NULL);
	            threadActiveList[threadCount] = 0;
	            printf("Thread no %d is waiting.\n", threadCount);
	        }
	    }

		for (int threadCount = 0; threadCount < numberOfThreads; threadCount++)
	    {
	        //Thread occupied
	        if(threadList[threadCount] == 0)
	        {
	            printf("Thread %d occupied. Moving to thread %d\n", threadCount, threadCount+1);
	        }
	        //Thread is free!
	        else 
	        {
	            iret = pthread_create( &threadList[threadCount], NULL, connection_handler, "Thread ready!");
	            if(iret)
	            {
	                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
	                exit(1);
	            }
	            printf("Thread no %d created\n", threadCount);
	            threadActiveList[threadCount] = 1;
	            break;
	        }
	    }    
    }
    return 1;
}

