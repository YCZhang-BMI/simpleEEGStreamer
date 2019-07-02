#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#pragma comment(lib,"ws2_32.lib")

#pragma warning(disable:4996)

#define BUFFER_LENGTH 4000
SOCKET sockfd;
SOCKET serverSock;
SOCKET cntSock;
unsigned long sampleCount;
int flag;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexc = PTHREAD_MUTEX_INITIALIZER;

double * buffer;
// double * bufPtr;

unsigned long long swap64(unsigned long long value)
{
    unsigned long long ret;
    ret = value << 56;
    ret |= (value&0x000000000000FF00) << 40;
    ret |= (value&0x0000000000FF0000) << 24;
    ret |= (value&0x00000000FF000000) << 8;
    ret |= (value&0x000000FF00000000) >> 8;
    ret |= (value&0x0000FF0000000000) >> 24;
    ret |= (value&0x00FF000000000000) >> 40;
    ret |= value >> 56;
    return ret;
}


void *tcpthread(void *addr)
{
    printf("Thread 1 created.\n");
    char headbuf[16];
    // unsigned long eegbuf[256];
    // unsigned long emgbuf[32];
    double bufDouble[160];
    // char recEegBuf[516];
    // char recEmgBuf[128]
    // mxArray *sampPtr;
    char charBuf[1264];
    
    char * ptr;
    int i,j;
    unsigned long sampleNum;
	char message[]="(sendCommand cmd_ListenToAmp 0 0 0)\r\n\n\n";
    int q;
    if (send(sockfd, message,strlen(message),0)<0)
    {
        perror("YMA:SocketIO:sendFailed\n");
        pthread_exit(NULL);
    }
    else
    {
        printf("command line sent\n");
    }
    while (flag)
    {
        if (recv(sockfd, headbuf , 16 ,0)<0)
        {
            continue;
            printf("not received, keep receiving\n");
        }
        sampleNum = (unsigned long)swap64(*(unsigned long long*)(headbuf+8))/1264;
        

        printf("sample count = %ld\n",sampleCount);
        for (i=0;i<sampleNum;i++)
        {
            // recv(sockfd, ptr,80,0);
            // recv(sockfd, eegbuf,516,0);
            // recv(sockfd, ptr,540,0);
            // recv(sockfd, emgbuf,128,0);
            q = recv(sockfd ,charBuf,1264,MSG_WAITALL);
			// printf("%d ",q);
            for (j=0;j<128;j++)
            {
                bufDouble[j] = *(long *)(charBuf+j*4+80) * 0.0001552204291 ;
            }
            for (j=0;j<32;j++)
            {
                bufDouble[128+j] = *(long *)(charBuf+j*4+1136) * (-0.00111758708);
            }
			// printf("%d ",i);
			// printf("%zd\n",sizeof(bufDouble));
			if(pthread_mutex_lock(&mutexc)!=0){
				printf("can not lock");			
			}
            memmove(buffer+160*(4000+i),bufDouble,sizeof(bufDouble));
            if(pthread_mutex_unlock(&mutexc)!=0){
				printf("can not unlock");			
			}
        }
		if(pthread_mutex_lock(&mutexc)!=0){
			printf("can not lock");			
		}
		if(pthread_mutex_lock(&mutex)!=0){
			printf("can not lock");			
		}
        memmove(buffer,buffer+sampleNum*160,BUFFER_LENGTH*8*160);
		sampleCount = sampleCount + sampleNum;
		if(pthread_mutex_unlock(&mutex)!=0){
			printf("can not unlock");			
		}
		if(pthread_mutex_unlock(&mutexc)!=0){
			printf("can not unlock");			
		}
        // sampleCount = sampleCount + sampleNum;
        // sampPtr =  mexGetVariable("base", "sampleNum");
        // mxGetPr(sampPtr)[0]+=sampleNum;
        // mexPutVariable("base", "sampleNum",sampPtr);
        // mexPutVariable("base","Buffer",buffer);
        
    }
    closesocket(sockfd);
    printf("client to amp server shut down\n");
    pthread_exit(NULL);
    return(0);
    
    
}


void *serverthread(void *addr)
{
    printf("Thread 2 created.\n");
    char *recvBuf = (char *)malloc(30);
	char *headSendBuf =(char *)malloc(sizeof(unsigned long));
	char *sendBuf = (char *)malloc(sizeof(double)*160*BUFFER_LENGTH);
    int matSC;
	int pacSN;
    while (flag)
    {
		memset(recvBuf,0,30*sizeof(*recvBuf));
        if (recv(cntSock,recvBuf,30,0)<=0)
        {
            flag = 0;
            printf("connection closed\n");
        }
        else
        {
			if(pthread_mutex_lock(&mutex)!=0){
				printf("can not lock");			
			}
			memcpy(headSendBuf,&sampleCount,sizeof(unsigned long));
			memcpy(sendBuf,buffer,160*8*BUFFER_LENGTH);
			if(pthread_mutex_unlock(&mutex)!=0){
				printf("can not unlock");			
			}
			
            printf("%s\n",recvBuf);
            matSC = atoi(recvBuf);
			pacSN = *(unsigned long*)headSendBuf-matSC;

            if (matSC == 0)
            {
                sampleCount = 1;
				send(cntSock,(char *)&sampleCount,sizeof(unsigned long),0);
				send(cntSock,sendBuf,sizeof(double)*BUFFER_LENGTH*160,0);
			} else if (sampleCount-matSC>BUFFER_LENGTH||matSC>sampleCount)
			{
				send(cntSock,headSendBuf,sizeof(unsigned long),0);
				send(cntSock,sendBuf,sizeof(double)*BUFFER_LENGTH*160,0);				
			} else 
			{
				
				send(cntSock,headSendBuf,sizeof(unsigned long),0);
				send(cntSock,(sendBuf+(BUFFER_LENGTH-pacSN)*160*8),sizeof(double)*pacSN*160,0);	
			}

        }
        
        
    }
    closesocket(cntSock);
    closesocket(serverSock);
    printf("buffer server shut down\n");
    return(0);
    
}

void main (int argc, char *argv[])
{
    printf("Buffer initializing...\n");
    flag = 1;
    pthread_t thread;
    pthread_t sThread;
    sampleCount = 0;
    WSADATA wsa;
    int len;
    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2,2),&wsa)!=0)
    {
        printf("Failed. Error Code : %d\n",WSAGetLastError());
        exit(1);
    }
    printf("Winsock Initialized.\n");
    
    
    
    buffer = (double *)malloc((1000+BUFFER_LENGTH) * 160*8);
    // bufPtr = mxGetData(buffer);
    struct sockaddr_in client_addr;
    memset(&client_addr,0,sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(argv[1]); /////
    client_addr.sin_port = htons(atoi(argv[2]));
    
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET )
    {
        perror("YMA:SocketIO:socketFailed");
        exit(1);
    }
    
    if (connect(sockfd, (struct sockaddr *)&client_addr , sizeof(struct sockaddr_in))<0)
    {
        perror("YMA:SocketIO:connectFailed");
        exit(1);
    }
    
    struct sockaddr_in addr;
    struct sockaddr_in client;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.S_un.S_addr = INADDR_ANY;
    
    
    
    if ((serverSock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET )
    {
        perror("YMA:SocketIO:serverSocketFailed");
        exit(1);
    }
    
    bind(serverSock, (struct sockaddr *)&addr, sizeof(addr));
    if (listen(serverSock, 3)<0)
    {
        printf("listen failed\n");
    }
    
    printf("Waiting for connection...");
    printf("11111\n");
    len = sizeof(struct sockaddr_in);
    cntSock = accept(serverSock,(struct sockaddr *)&client, &len);
    
    
    
    
    
    printf("11111\n");
    if (pthread_create(&thread, NULL, tcpthread, NULL))
    {
        // if (pthread_create(&thread, NULL, tcpthread, NULL))
        perror("YMA:MexIO:threadFailed");
    }
    if (pthread_create(&sThread, NULL, serverthread, NULL))
        // if (pthread_create(&thread, NULL, tcpthread, NULL))
        perror("YMA:MexIO:threadFailed");
    
    printf("11111\n");
//     Sleep(20000);
    pthread_join(sThread,NULL);
	pthread_join(thread,NULL);
    
}