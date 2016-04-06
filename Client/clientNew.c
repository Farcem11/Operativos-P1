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
#include <ctype.h>
#include <pthread.h>

uint16_t portToUse;
char* ipToAddress;

typedef struct Array
{
  int Size;
  char** Data;
} Array;

int count_char_in_string(char* pString, char pChar);
Array* split(char* pString, char pChar);
char* TrimWhitespace(char* str);
int stringToInt(char* pString);

char recvBuff[1024];
int n;

int count_char_in_string(char* pString, char pChar)
{
  int count = 0;
  unsigned int i;
  for (i = 0; i < strlen(pString); i++)
    if (pString[i] == pChar)
      count++;
  return count;
}

Array* split(char* pString, char pChar)
{
  char* token;
  char* string;
  Array* array = (Array*)malloc(sizeof(Array));
  array->Data = malloc(sizeof(char*) * count_char_in_string(pString, pChar) + 1);
  array->Size = count_char_in_string(pString, pChar) + 1;
  int i = 0;
 
  string = strdup(pString);

  if (string != NULL) 
  {
    while ((token = strsep(&string, ",")) != NULL)
    {
      array->Data[i] = malloc(sizeof(char) * strlen(token));
      strcpy( array->Data[i++], token);
    }
  }
  return array;
}

char* TrimWhitespace(char* pString)
{
  char* end;

  while(isspace(*pString))
    pString++;

  if(*pString == 0)
    return pString;

  end = pString + strlen(pString) - 1;
  while(end > pString && isspace(*end))
    end--;

  *(end+1) = 0;

  return pString;
}

int stringToInt(char* pString)
{
    return (int) strtol(pString, (char **)NULL, 10);
}

void* connection_handler(void* fileNameToRetrieveOnServer)
{
    char* fileNameRetrieve = *(char**)fileNameToRetrieveOnServer;
    printf("%s", "Request of file ");
    printf("%s\n", fileNameRetrieve);
    int n;
    int sockfd = 0;
    int bytesReceived = 0;
    char* recvBuff = calloc(256, sizeof(char));
    
    struct sockaddr_in serv_addr;

    //Create a socket first
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error: Could not create socket \n");
        return 0;
    }

    //Initialize sockaddr_in data structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portToUse);
    serv_addr.sin_addr.s_addr = inet_addr(ipToAddress);

    //Attempt a connection
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error: Connect Failed \n");
        return 0;
    }
    
    //Envia nombre del archivo
    strcpy(recvBuff, "/client ");
    strcat(recvBuff, fileNameRetrieve);

    n = send(sockfd,recvBuff,strlen(recvBuff) + 1,0);
    if (n < 0) 
        printf("ERROR writing to socket");

    bytesReceived = recv(sockfd, recvBuff, 256, 0);
    //Recibe confirmación
    if(bytesReceived > 0)
    {
        //Create file where data will be stored
        FILE *fp = fopen(fileNameRetrieve, "wb"); 
        if(fp == NULL)
        {
            printf("Error opening file");
            return 0;
        }
        int totalBytes = 0;
        //Receive data in chunks of 256 bytes
        while(1)
        {
            totalBytes += bytesReceived;
            fwrite(recvBuff, 1, bytesReceived,fp);
            if (bytesReceived < 256)
            {
                break;
            }
            bytesReceived = recv(sockfd, recvBuff, 256, 0);
        }
        if(bytesReceived < 0)
        {
            printf("\n Read Error\n");
        }
        printf("Bytes received: %d\n", totalBytes);
        fclose(fp);
    }
    else
    {
        printf("%s", fileNameRetrieve);
        printf("%s\n", " is not in server");
    }
    return 0;
}

int main(int argc,char *argv[])
{
    int i = 0;

    if (argc != 4)
    {
        printf("ERROR: The input is: ./client port ip file\n");
        return 1;
    }

    char *fileNameRetrieve = argv[3];    
    portToUse = stringToInt(argv[2]);
    ipToAddress = argv[1];
    
    Array* arrayFileNames = split(fileNameRetrieve, ',');

    for (i = 0; i < arrayFileNames->Size; i++)
    {
        arrayFileNames->Data[i] = TrimWhitespace(arrayFileNames->Data[i]);
        
        pthread_t tid;
        if( pthread_create(&tid, NULL, connection_handler, (void*) &arrayFileNames->Data[i]) < 0)
        {
            perror("could not create thread");
        }
        pthread_join(tid, NULL);
    }
    return 0;
}
