//Compile using gcc -W -Wall pre-thread.c -pthread -o prethread

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

typedef struct ThreadArg
{
  int threadIndex;
  int socket;
} ThreadArg;

int numberOfThreads;
int* availableThreads;
pthread_t* threads;
int Mbs = 20*1024*1024;
int connfd;
pthread_mutex_t mutex;

int stringToInt(char* pString)
{
    return (int) strtol(pString, (char **)NULL, 10);
}

//Checks if there are threads working, in case there is any active ones 
int threadChecker()
{
    int threadCount;
    for(threadCount = 0; threadCount < numberOfThreads; threadCount++)
    {
        if (availableThreads[threadCount] == 0)
        {
            availableThreads[threadCount] = 1;
            return threadCount;
        }
    }
    return -1;
}

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


void* connection_handler(void* args)
{
    ThreadArg* arguments = *(ThreadArg**)args;

    int index = arguments->threadIndex;
    int connfd = arguments->socket;

    printf("Thread %d is responding client %d request: ", index, connfd);

    char sendBuff[2048] = {0};
    int readed = recv(connfd, sendBuff, sizeof(sendBuff), 0);
    
    if(readed > 0)
    {
        char* fileNameFromBrowser = calloc(256, sizeof(char));

        fileNameFromBrowser = getSubStringRight(sendBuff,"/");
        fileNameFromBrowser = getSubStringLeft(fileNameFromBrowser," ");

        if(strcmp(fileNameFromBrowser, "client") != 0)
            printf("file: %s\n", fileNameFromBrowser);
        else
            printf("file: %s\n", getSubStringRight(sendBuff," "));

        unsigned char* imageData = (unsigned char*)calloc(Mbs, sizeof(unsigned char));
        char* fileNameFromClient = calloc(256, sizeof(char));
        char filePath[125] = {0};
        unsigned char buff[256] = {0};
        FILE* file;
        unsigned long fileLen;
        if(strcmp(getSubStringLeft(sendBuff," "), "/client") != 0)
        {
            //Browser
            strcpy(filePath, "Files/");
            strcat(filePath, fileNameFromBrowser);
            FILE* file;
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
                printf("%s no esta en el server\n", fileNameFromBrowser);
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
            
            strcpy(filePath, "Files/");
            strcat(filePath, fileNameFromClient);

            //Open the file that we wish to transfer
            FILE* fp;
            if( access(filePath, F_OK) == -1 )
            {
                printf("Error opening file. File does not exist\n");
                send(connfd, "", 0, 0);
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
        free(imageData);
    }
    threads[index] = 0;
    close(connfd);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("ERROR: Input must be like this: ./prethread port numberOfThreads\n");
        return 1;
    }

    int threadCount;

    numberOfThreads = stringToInt(argv[2]);
    struct sockaddr_in serv_addr; 
    socklen_t serv_len;
    
    availableThreads = calloc(numberOfThreads, sizeof(int));
    threads = calloc(numberOfThreads, sizeof(pthread_t));

    for(threadCount = 0; threadCount < numberOfThreads; threadCount++)
        threads[threadCount] = 0;

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&serv_addr, '\0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8002); 

    printf("%s\n", "\nBinding socket...");
    while(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0);
    printf("%s\n", "Socket binded, listening at port 8002\n"); 

    listen(listenfd,100);
    pthread_mutex_init(&mutex, NULL);

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)&serv_addr, &serv_len);

        pthread_mutex_lock(&mutex);
        for(threadCount = 0; threadCount < numberOfThreads; threadCount++)
        {
            if(threads[threadCount] == 0)
            {
                ThreadArg* threadArg = (ThreadArg*)malloc(sizeof(threadArg));
            
                threadArg->socket = connfd;
                threadArg->threadIndex = threadCount;
                pthread_create(&threads[threadCount], NULL, connection_handler, (void*) &threadArg);
                break;
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}

