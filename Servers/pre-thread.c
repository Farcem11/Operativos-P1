//Compile using gcc pre-thread.c -lpthread -o prethread

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_message_function( void *ptr );
void threadChecker(int lengthList, pthread_t *threadListToCheck, int *threadStatusList);
void threadAssigner(int lengthList, pthread_t *threadListToCheck, int *threadStatusList);


int iret; //Thread creation code

int main(int argc, char *argv[])
{
    pthread_t *threadCreator; //Thread to set
    int numberOfThreads;


    if (argc != 2){
    	printf("ERROR: Input must be like this: ./prethread numberOfThreads\n");
    	return 1;
    }

    numberOfThreads = atoi(argv[1]);
    int threadActiveList[numberOfThreads];
    pthread_t *threadList[numberOfThreads];

    bzero(threadList, sizeof(numberOfThreads));
    bzero(threadActiveList, sizeof(numberOfThreads));

    //Assign thread on list
    for(int threadCount = 0; threadCount < numberOfThreads; threadCount++){
    	threadList[threadCount] = threadCreator;   	
    }

    threadAssigner(numberOfThreads, threadList, threadActiveList);
    threadAssigner(numberOfThreads, threadList, threadActiveList);

    //Stopping dead threads
    while(1){
        threadChecker(numberOfThreads, threadList, threadActiveList);
    }
    

    exit(EXIT_SUCCESS);
}

void *print_message_function( void *ptr )
{
     char *message;
     message = (char *) ptr;
     printf("%s \n", message);
}

//Checks if there are threads working, in case there is any active ones 
void threadChecker(int lengthList, pthread_t *threadListToCheck, int *threadStatusList)
{
    for (int threadCount = 0; threadCount < lengthList; threadCount++)
    {
        if(pthread_kill(threadListToCheck[threadCount], 0) == 0)
        {
            printf("Thread no %d is still running. Can't join now\n", threadCount);
        }
        else 
        {
            pthread_join(threadListToCheck[threadCount], NULL);
            threadStatusList[threadCount] = 0;
            printf("Thread no %d is stopped\n", threadCount);
        }
    }
}

void threadAssigner(int lengthList, pthread_t *threadListToCheck, int *threadStatusList)
{
    for (int threadCount = 0; threadCount < lengthList; threadCount++)
    {
        //Thread occupied
        if(threadListToCheck[threadCount] == 0)
        {
            printf("Thread %d occupied. Moving to thread %d\n", threadCount, threadCount+1);
        }
        //Thread is free!
        else 
        {
            iret = pthread_create( &threadListToCheck[threadCount], NULL, print_message_function, "Thread ready!");
            if(iret)
            {
                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
                exit(1);
            }
            printf("Thread no %d created\n", threadCount);
            threadStatusList[threadCount] = 1;
            break;
        }
    }
}
