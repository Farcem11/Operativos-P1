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

int main(int argc,char *argv[])
{
    int sockfd = 0;
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
    
    char *fileNameRetrieve = argv[3];
    Array* arrayFileNames = split(fileNameRetrieve, ',');
    for (int i = 0; i < arrayFileNames->Size; i++)
    {
        arrayFileNames->Data[i] = TrimWhitespace(arrayFileNames->Data[i]);
    }

    for (int i = 0; i < arrayFileNames->Size; i++)
    {
        int result = transferenceProcess(arrayFileNames->Data[i], sockfd);
        if (result)
            printf("ERROR: Trasnferring wasn't successful");
    }

    return 0;
}

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

int transferenceProcess(char* fileNameToRetrieveOnServer, int socketToWork){
    int bytesReceived = 0;
    int noFile;
    char confirmBuff[32];

    //Envia nombre del archivo
    bzero(recvBuff,256);
    bzero(confirmBuff,32);
    strcpy(recvBuff, "/client ");
    printf("%s\n", fileNameToRetrieveOnServer);
    strcat(recvBuff, fileNameToRetrieveOnServer);

    n = write(socketToWork,recvBuff,sizeof(recvBuff));
    if (n < 0) 
        printf("ERROR writing to socket");

    //Recibe confirmación
    n = read(socketToWork,confirmBuff,31);
    noFile = strcmp(confirmBuff,"1");
    if (noFile == 0)
    {
        printf("File does not exist on server\n");
        return 1;
    }

    printf("File exists on server\n");

    //Create file where data will be stored
    FILE *fp = fopen(fileNameToRetrieveOnServer, "wb"); 
    if(fp == NULL)
    {
        printf("Error opening file");
        return 1;
    }
    bzero(recvBuff,256);
    //Receive data in chunks of 256 bytes
    while((bytesReceived = read(socketToWork, recvBuff, 256)) > 0)
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