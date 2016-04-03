//gcc -W -Wall -o client client.c
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

int stringToInt(char* pString)
{
    return (int) strtol(pString, (char **)NULL, 10);
}

int main(int argc,char *argv[])
{
    int n, noFile;
    int sockfd = 0;
    int bytesReceived = 0;
    char recvBuff[1024];
    char confirmBuff[32];
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;

    if (argc != 4)
    {
        printf("ERROR: The input is: ./client port ip file\n");
        return 1;
    }

    //Create a socket first
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error: Could not create socket \n");
        return 1;
    }
    printf("%s:%d/%s\n",argv[1], stringToInt(argv[2]), argv[3]);

    char *fileNameRetrieve = argv[3];    
    uint16_t portToUse = stringToInt(argv[2]);
    char *ipToAddress = argv[1];

    //Initialize sockaddr_in data structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portToUse);
    serv_addr.sin_addr.s_addr = inet_addr(ipToAddress); //"127.0.0.1"

    //Attempt a connection
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error: Connect Failed \n");
        return 1;
    }
    
    //Envia nombre del archivo
    bzero(recvBuff,256);
    bzero(confirmBuff,32);
    strcpy(recvBuff, "/client ");
    printf("%s\n", fileNameRetrieve);
    strcat(recvBuff, fileNameRetrieve);

    n = write(sockfd,recvBuff,sizeof(recvBuff));
    if (n < 0) 
        printf("ERROR writing to socket");

    //Recibe confirmación
    n = read(sockfd,confirmBuff,31);
    noFile = strcmp(confirmBuff,"1");
    if (noFile == 0)
    {
        printf("File does not exist on server\n");
        return 1;
    }

    printf("File exists on server\n");

    //Create file where data will be stored
    FILE *fp = fopen(fileNameRetrieve, "wb"); 
    if(fp == NULL)
    {
        printf("Error opening file");
        return 1;
    }
    bzero(recvBuff,256);
    //Receive data in chunks of 256 bytes
    while((bytesReceived = read(sockfd, recvBuff, 256)) > 0)
    {
        printf("Bytes received %d\n",bytesReceived);    
        fwrite(recvBuff, 1,bytesReceived,fp);
    }

    if(bytesReceived < 0)
    {
        printf("\n Read Error\n");
    }

    fclose(fp);

    return 0;
}
