#pragma comment(lib, "Ws2_32.lib")
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5059"
#define DEFAULT_PORT_WORKER "27016"
#define MAX_COUNT_SEMAPHORE 100

#include "structs.h"
#include "functionDefs.h"
#include "communiocationFuncs.h"
#include "clientList.h"
#include "workerList.h"
#include "threadFuncs.h"
#include "ringBuffer.h"

int globalIdClient = 123456;
int globalIdWorker = 1;

int index = 0; // index counter for clients
int indexWorker = 0; // index counter for workers
Node *headClients = NULL; // list of clients
NodeW *headWorkers = NULL; // list of workers

int capacityBuffer = 100;
Queue* primaryQueue = NULL;
Queue* tempQueue = NULL;
Queue* secondaryQueue = NULL;

CRITICAL_SECTION CriticalSectionForQueue;
CRITICAL_SECTION CriticalSectionForOutput;
HANDLE WriteSemaphore, ReadSemaphore;

int main(void) {
	WriteSemaphore = CreateSemaphore(0, MAX_COUNT_SEMAPHORE, MAX_COUNT_SEMAPHORE, NULL);
	ReadSemaphore = CreateSemaphore(0, 0, MAX_COUNT_SEMAPHORE, NULL);

	InitializeCriticalSection(&CriticalSectionForOutput);
	InitializeCriticalSection(&CriticalSectionForQueue);

	primaryQueue = CreateQueue(capacityBuffer);
	// Socket used for listening for new clients 
	SOCKET listenSocket = SetListenSocket(DEFAULT_PORT);

	// Socket used for listening for new workers
	SOCKET listenSocketWorker = SetListenSocket(DEFAULT_PORT_WORKER);

	// variable used to store function return value
	int iResult;
	// Buffer used for storing incoming data
	char recvbuf[DEFAULT_BUFLEN];
	char sendbuf[DEFAULT_BUFLEN];

	printf("Server initialized, waiting for clients.\n");

	DWORD dispecherId;
	HANDLE dispecher = CreateThread(NULL,
		0,
		Dispecher,
		NULL,
		0,
		&dispecherId
	);

	do
	{
		struct sockaddr_in address;
		int addrlen = sizeof(address);

		timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;

		FD_SET set;
		FD_ZERO(&set);

		FD_SET(listenSocket, &set);
		FD_SET(listenSocketWorker, &set);

		iResult = select(0, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			printf("select failed with error: %d\n", WSAGetLastError());
		}
		else if (iResult == 0) {
			//printf("I'm waiting\n");
			continue;
		}
		else if (FD_ISSET(listenSocket, &set)) {
			Client *newClient = (Client*)malloc(sizeof(Client));
			newClient->acceptedSocket = accept(listenSocket, (struct sockaddr *)&address, &addrlen);
			if (newClient->acceptedSocket == INVALID_SOCKET)
			{
				printf("accept failed with error: %d\n", WSAGetLastError());
				closesocket(listenSocket);
				WSACleanup();
				return 1;
			}
			char *clientip = new char[20];
			strcpy(clientip, inet_ntoa(address.sin_addr));
			printf("Client port: %d, IP address: %s is accepted\n", address.sin_port, clientip);

			DWORD threadId;

			newClient->id = globalIdClient++;
			newClient->acceptedSocket = newClient->acceptedSocket;
			newClient->ipAdr = clientip;
			newClient->port = address.sin_port;
			newClient->thread = CreateThread(NULL,
				0,
				myThreadFun,
				&newClient->acceptedSocket,
				0,
				&threadId
			);

			printf("-----------\n\tClients[%d]\nid: %d\nip Address: %s\nport: %d\nthreadId:%d \n\n\tis accepted\n----------------\n"
				, index, newClient->id, newClient->ipAdr, newClient->port, threadId);


			AddAtEnd(&headClients, newClient);

			++index;
		}
		else if (FD_ISSET(listenSocketWorker, &set)) {
			Worker *newWorker = (Worker*)malloc(sizeof(Worker));
			newWorker->acceptedSocket = accept(listenSocketWorker, (struct sockaddr *)&address, &addrlen);
			if (newWorker->acceptedSocket == INVALID_SOCKET)
			{
				printf("accept failed with error: %d\n", WSAGetLastError());
				closesocket(listenSocketWorker);
				WSACleanup();
				return 1;
			}
			char *workerip = new char[20];
			strcpy(workerip, inet_ntoa(address.sin_addr));
			printf("Client port: %d, IP address: %s is accepted\n", address.sin_port, workerip);

			DWORD threadId;

			newWorker->id = globalIdWorker++;
			newWorker->counter = 0;
			newWorker->acceptedSocket = newWorker->acceptedSocket;
			newWorker->ipAdr = workerip;
			newWorker->port = address.sin_port;
			newWorker->thread = CreateThread(NULL,
				0,
				myThreadFunWorker,
				&newWorker->acceptedSocket,
				0,
				&threadId
			);

			printf("-----------\nWorker[%d]\nid: %d\nip Address: %s\nport: %d\nthreadId:%d \n\n\tis accepted\n----------------\n"
				, indexWorker, newWorker->id, newWorker->ipAdr, newWorker->port, threadId);


			AddAtEnd(&headWorkers, newWorker);

			++indexWorker;
		} 

	} while (1);

	// free all nodes from list
	FreeList(headClients);
	FreeList(headWorkers);
	
	closesocket(listenSocket);
	closesocket(listenSocketWorker);
	WSACleanup();

	CloseHandle(WriteSemaphore);
	CloseHandle(ReadSemaphore);

	Sleep(2000);
	DeleteCriticalSection(&CriticalSectionForQueue);
	DeleteCriticalSection(&CriticalSectionForOutput);

	return 0;
}

