#define _winsock_deprecated_no_warnings
#pragma comment(lib, "ws2_32.lib")
#define win32_lean_and_mean

#pragma region IncludeLibrary
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#pragma endregion

#pragma region IncludeHeaderFiles
#include "structs.h"
#include "communiocationFuncs.h"
#include "clientList.h"
#include "workerList.h"
#include "externData.h"
#include "ringBuffer.h"
#include "redistribution.h"
#include "dispecher.h"
#include "clientToLoadBalancer.h"
#include "loadBalancerToWorker.h"
#include "killThreadAndHandle.h"
#include "queue.h"
#pragma endregion

#pragma region GlobalVariable
int globalIdClient = 123456;
int globalIdWorker = 1;
int indexClient = 0; // index counter for clients
int indexWorker = 0; // index counter for workers
Node *headClients = NULL; // list of clients
NodeW *headWorkers = NULL; // list of workers
Queue* primaryQueue = NULL;
Queue* tempQueue = NULL;
Queue* secondaryQueue = NULL;
Queue* reorQueue = NULL;
CRITICAL_SECTION
	CriticalSectionForQueue,
	CriticalSectionForOutput,
	CriticalSectionForReorQueue;
HANDLE
	WriteSemaphore,
	WriteSemaphoreTemp,
	ReadSemaphore,
	CreateQueueSemaphore,
	CreatedQueueSemaphore,
	ReorganizeSemaphoreStart,
	ReorganizeSemaphoreEnd,
	TrueSemaphore;
#pragma endregion

int main(void) {
	#pragma region InitVariable
	int iResult; // variable used to store function return value
	char recvbuf[DEFAULT_BUFLEN]; // Buffer used for storing incoming data
	char sendbuf[DEFAULT_BUFLEN]; // Buffer used for sending data
	int addrlen = 0;
	SOCKET listenSocket, listenSocketWorker;
	#pragma endregion
	#pragma region CreateSemaphores
	WriteSemaphore = CreateSemaphore(0, MAX_COUNT_SEMAPHORE, MAX_COUNT_SEMAPHORE, NULL);
	WriteSemaphoreTemp = CreateSemaphore(0, 0, 1, NULL);
	ReadSemaphore = CreateSemaphore(0, 0, MAX_COUNT_SEMAPHORE, NULL);
	CreateQueueSemaphore = CreateSemaphore(0, 0, 1, NULL);
	CreatedQueueSemaphore = CreateSemaphore(0, 0, 1, NULL);
	ReorganizeSemaphoreStart = CreateSemaphore(0, 0, 2, NULL);
	ReorganizeSemaphoreEnd = CreateSemaphore(0, 0, 1, NULL);
	TrueSemaphore = CreateSemaphore(0, 1, 1, NULL);
	#pragma endregion
	#pragma region CreateCriticalSection
	InitializeCriticalSection(&CriticalSectionForOutput);
	InitializeCriticalSection(&CriticalSectionForQueue);
	#pragma endregion
	primaryQueue = CreateQueue(INITIAL_CAPACITY_BUFFER);
	listenSocket = SetListenSocket(DEFAULT_PORT);  // Socket used for listening for new clients 
	listenSocketWorker = SetListenSocket(DEFAULT_PORT_WORKER);  // Socket used for listening for new workers

	printf("Server initialized, waiting for clients...\n");
	printf("Press enter to exit...\n");

	#pragma region CreateThreads
	HANDLE dispecher = CreateThread(NULL,
		0,
		Dispecher,
		NULL,
		0,
		NULL
	);
	HANDLE threadForQueue = CreateThread(NULL,
		0,
		WorkWithQueue,
		NULL,
		0,
		NULL
	);
	HANDLE redistributioner = CreateThread(NULL,
		0,
		Redistributioner,
		NULL,
		0,
		NULL
	);
	#pragma endregion
	do
	{
		struct sockaddr_in address;
		addrlen = sizeof(address);
		timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;
		#pragma region Set
		FD_SET set;
		FD_ZERO(&set);
		FD_SET(listenSocket, &set);
		FD_SET(listenSocketWorker, &set);
		#pragma endregion

		iResult = select(0, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			printf("select failed with error: %d\n", WSAGetLastError());
		}
		else if (iResult == 0) {
			if (_kbhit()) {
				break;
			}
			printf("Wait client...\n");
			continue;
		}
		else if (FD_ISSET(listenSocket, &set)) {  //pristigao zahtev za konekciju novog klijenta
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
			//printf("Client port: %d, IP address: %s is accepted\n", address.sin_port, clientip);
			#pragma region CreateAndInitThreadForClient
			DWORD threadId;
			newClient->id = globalIdClient++;
			newClient->acceptedSocket = newClient->acceptedSocket;
			newClient->ipAdr = clientip;
			newClient->port = address.sin_port;
			newClient->thread = CreateThread(NULL,
				0,
				RecvClientMessage,
				&newClient->acceptedSocket,
				0,
				&threadId
			);
			#pragma endregion
			#pragma region PrintfInfo
			printf(
				"-----------\n\tClients[%d]\nid: %d\nip Address: %s\nport: %d\nthreadId:%d \n\n\tis accepted\n----------------\n"
				, indexClient,
				newClient->id,
				newClient->ipAdr,
				newClient->port,
				threadId
			);
			#pragma endregion
			AddAtEnd(&headClients, newClient);
			++indexClient;
		}
		else if (FD_ISSET(listenSocketWorker, &set)) {  //pristigao zahtev za konekciju novog worker-a
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
			//printf("Worker port: %d, IP address: %s is accepted\n", address.sin_port, workerip);
			#pragma region CreateAndInitThreadForWorker
			DWORD threadId;
			newWorker->id = globalIdWorker++;
			newWorker->counter = 0;
			newWorker->acceptedSocket = newWorker->acceptedSocket;
			newWorker->ipAdr = workerip;
			newWorker->port = address.sin_port;
			newWorker->thread = CreateThread(NULL,
				0,
				SendAndRecvWorkerMessage,
				&newWorker->acceptedSocket,
				0,
				&threadId
			);
			#pragma endregion
			#pragma region PrintfInfo
			printf(
				"-----------\nWorker[%d]\nid: %d\nip Address: %s\nport: %d\nthreadId:%d \n\n\tis accepted\n----------------\n"
				, indexWorker,
				newWorker->id,
				newWorker->ipAdr,
				newWorker->port,
				threadId
			);
			#pragma endregion
			AddAtEnd(&headWorkers, newWorker);
			if (indexWorker > 0)
				ReleaseSemaphore(ReorganizeSemaphoreStart, 2, NULL);

			++indexWorker;
		}
	} while (1);
	#pragma region DeleteThread
	DeleteAllThreads(headClients, headWorkers);
	CloseMainThread(dispecher);
	CloseMainThread(threadForQueue);
	CloseMainThread(redistributioner);
	#pragma endregion
	#pragma region FreeList 
	FreeList(headClients); // free all nodes from list
	FreeList(headWorkers); // free all nodes from list
	#pragma endregion
	#pragma region CloseSocket
	closesocket(listenSocket);
	closesocket(listenSocketWorker);
	#pragma endregion
	WSACleanup();
	#pragma region CloseHandle
	CloseHandle(WriteSemaphore);
	CloseHandle(WriteSemaphoreTemp);
	CloseHandle(ReadSemaphore);
	CloseHandle(CreateQueueSemaphore);
	CloseHandle(CreatedQueueSemaphore);
	CloseHandle(ReorganizeSemaphoreStart);
	CloseHandle(ReorganizeSemaphoreEnd);
	CloseHandle(TrueSemaphore);
	#pragma endregion
	#pragma region DestroyQueue
	DestroyQueue(primaryQueue);
	if (tempQueue != NULL)
		DestroyQueue(tempQueue);
	if (secondaryQueue != NULL)
		DestroyQueue(secondaryQueue);
	#pragma endregion
	Sleep(100);
	#pragma region DeleteCriticalSection
	DeleteCriticalSection(&CriticalSectionForQueue);
	DeleteCriticalSection(&CriticalSectionForOutput);
	#pragma endregion

	return 0;
}


