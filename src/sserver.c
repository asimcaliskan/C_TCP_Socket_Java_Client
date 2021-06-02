#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_THREAD_NUMBER 3
#define MAX_CONNECTION_REQUEST 3
#define BUFFER_SIZE 1024

//SOCKET ATTRIBUTES<
int portNumber;
int numberOfChannels;
char videoTextPaths[3][100];
//SOCKET ATTRIBUTES>

//VIDEO TEXT BUFFERS<
char *videoBuffers[3];
int videoBufferSizes[3];
//VIDEO TEXT BUFFERS>

//CHANNEL FLAGS<
int isChannel_0_streaming = 0;
int isChannel_1_streaming = 0;
int isChannel_2_streaming = 0;
//CHANNEL FLAGS>

pthread_mutex_t threadLock;


void* threadFunction(void* threadArgument)
{
    //pthread_mutex_lock(&threadLock);
    char buffer[BUFFER_SIZE + 1];
    //char tempBuffer[BUFFER_SIZE + 1];
    int clientSocket = *(( int* )threadArgument);
    int videoBufferSeek = 0;
    while(1){
        recv(clientSocket, buffer, BUFFER_SIZE, 0);
        int clientChannelRequest = atoi(buffer) - 1;
        memset(buffer, 0, BUFFER_SIZE + 1);
        //memset(tempBuffer, 0, BUFFER_SIZE);

        //WRITE VIDEO BUFFERS TO BUFFERS<
        int bufferCounter = 0;
        while(1)
        {
            char tempChar = videoBuffers[clientChannelRequest][videoBufferSeek];
            if(tempChar != '\n' && tempChar != '\0')
            {
                buffer[bufferCounter++] = tempChar;
                videoBufferSeek++;
            }
            else if(tempChar == '\n'){
                buffer[bufferCounter++] = tempChar;
                videoBufferSeek++;
                break;
            }
            else
            {
                buffer[bufferCounter++] = '\n';
                videoBufferSeek = 0;
                break;
            }
        }
        //WRITE VIDEO BUFFERS TO BUFFERS>

        //sprintf(buffer, "%s", tempBuffer);
        //printf("%s\n", tempBuffer);
        send(clientSocket, buffer, BUFFER_SIZE, 0);
    }
    //while(1);
    //pthread_mutex_unlock(&threadLock);
    pthread_exit(NULL);
}

void readVideoTexts()
{
    int video = 0;
    for(video = 0; video < numberOfChannels; video++)
    {
        char ch;
        FILE *fp;
        fp = fopen(videoTextPaths[video], "r"); // read mode
        if (fp == NULL)
        {
            perror("Error while opening the file.\n");
            exit(EXIT_FAILURE);
        }
        fseek(fp, 0L, SEEK_END);
        int sizeOfFile = ftell(fp);
        videoBufferSizes[video] = sizeOfFile;
        fseek(fp, 0L, SEEK_SET);
        videoBuffers[video] = malloc(sizeof(char) * (sizeOfFile + 1));
        size_t newSizeOfFile = fread(videoBuffers[video], sizeof(char), sizeOfFile, fp);
        fclose(fp);
    }
}

int main(int argc, char *argv[])
{
    //GET ARGUMENTS<
    portNumber = atoi(argv[1]);
    numberOfChannels = atoi(argv[2]);
    sprintf(videoTextPaths[0], "%s", argv[3]);
    sprintf(videoTextPaths[1], "%s", argv[4]);
    sprintf(videoTextPaths[2], "%s", argv[5]);
    //GET ARGUMENTS>
    
    //READ VIDEO TEXT FILES<
    readVideoTexts();
    //READ VIDEO TEXT FILES>

    pthread_t threadID_0, threadID_1, threadID_2;

    if (pthread_mutex_init(&threadLock, NULL) != 0) {
        printf("MUTEX INITIALIZE HAS FAILED\n");
        return 1;
    }

    //SOCKET INITIALIZE<
    int serverSocket;
	struct sockaddr_in serverAddress;
	struct sockaddr_storage serverStorage;
	socklen_t addressSize;
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(serverAddress.sin_zero, '\0', sizeof(serverAddress.sin_zero));	
	bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    //SOCKET INITIALIZE>



    //STREAMS ON THREADS<
    int permission = 1;
    while(1)
    {
        if(listen(serverSocket, MAX_CONNECTION_REQUEST) == 0)
	    {
	        printf("LISTENING\n");
	    }
        int newClientSocket;
        addressSize = sizeof(serverStorage);
        if((newClientSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addressSize)) < 0)
		{
			perror("accept");
        	exit(0);
		}
        //int ID_0 = 0;
        if(isChannel_0_streaming == 0 && permission == 1)
        {
            permission = 0;
            printf("CHANNEL 1 IS STREAMING!!!\n");
            isChannel_0_streaming = 1;
            if(pthread_create(&threadID_0, NULL, threadFunction, &newClientSocket) != 0)
            {
                printf("THREAD 0 CREAT ERROR!!!");
            }
        }
        //int ID_1 = 1;
        if(isChannel_1_streaming == 0 && permission == 1)
        {
            permission = 0;
            printf("CHANNEL 2 IS STREAMING!!!\n");
            isChannel_1_streaming = 1;
            if(pthread_create(&threadID_1, NULL, threadFunction, &newClientSocket) != 0)
            {
                printf("THREAD 1 CREAT ERROR!!!");
            }
        }
        //int ID_2 = 2;
        if(isChannel_2_streaming == 0 && permission == 1)
        {
            permission = 0;
            printf("CHANNEL 0 IS STREAMING!!!\n");
            isChannel_2_streaming = 1;
            if(pthread_create(&threadID_2, NULL, threadFunction, &newClientSocket) != 0)
            {
                printf("THREAD 2 CREAT ERROR!!!");
            }
        }
        permission = 1;
    }
    //STREAMS ON THREADS>
 
    //WAIT THREADS TO TERMINATE<
    pthread_join(threadID_0, NULL);
    pthread_join(threadID_1, NULL);
    pthread_join(threadID_2, NULL);
    //WAIT THREADS TO TERMINATE>


    pthread_mutex_destroy(&threadLock);
    exit(0);
}