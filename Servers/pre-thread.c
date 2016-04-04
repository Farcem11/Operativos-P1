//Compile using gcc pre-thread.c -lpthread -o prethread

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_message_function( void *ptr );

int main(int argc, char *argv[])
{
    pthread_t *threadCreator; //Thread to set
    int iret; //Thread creation code

    if (argc != 2){
    	printf("ERROR: Input must be like this: ./prethread numberOfThreads\n");
    	return 1;
    }

    int numberOfThreads = atoi(argv[1]);
    pthread_t *threadList[numberOfThreads];

    //Assign thread on list
    for(int threadCount = 0; threadCount < numberOfThreads; threadCount++){
    	threadList[threadCount] = threadCreator;   	
    }

    //Creator. Check if thread is working
    for (int threadCount = 0; threadCount < numberOfThreads; threadCount++)
    {
     	//Thread occupied
     	if(pthread_kill(threadList[threadCount], 0) == 0)
		{
    		printf("Thread %d occupied. Moving to thread %d\n", threadCount, threadCount+1);
		}
		//Thread is free!
		else 
		{
			iret = pthread_create( &threadList[threadCount], NULL, print_message_function, "Thread ready!");
			if(iret)
     		{
     			fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
     			return 1;
     		}
     		printf("Thread no %d created\n", threadCount);
     		break;
		}
    }

    //Another creator for testing
    for (int threadCount = 0; threadCount < numberOfThreads; threadCount++)
    {
     	if(pthread_kill(threadList[threadCount], 0) == 0)
		{
    		printf("Thread %d occupied. Moving to thread %d\n", threadCount, threadCount+1);
		}
		else 
		{
			iret = pthread_create( &threadList[threadCount], NULL, print_message_function, "Thread ready!");
			if(iret)
     		{
     			fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
     			return 1;
     		}
     		printf("Thread no %d created\n", threadCount);
     		break;
		}
    }

     /* Wait till threads are complete before main continues. Unless we  */
     /* wait we run the risk of executing an exit which will terminate   */
     /* the process and all threads before the threads have completed.   */

    //Stopping dead threads
    while(1){
    	for (int threadCount = 0; threadCount < numberOfThreads; threadCount++)
    	{
     		if(pthread_kill(threadList[threadCount], 0) == 0)
			{
    			printf("Thread no %d is still running. Can't join now\n", threadCount);
			}
			else 
			{
				pthread_join(threadList[threadCount], NULL);
				printf("Thread no %d is stopped\n", threadCount);
			}
    	}
    }
    

    exit(EXIT_SUCCESS);
}

void *print_message_function( void *ptr )
{
     char *message;
     message = (char *) ptr;
     printf("%s \n", message);
     long int i = 0;
     while(i < 9999999){
     	i++;
     }
     printf("Dickbutt 1\n");
}

