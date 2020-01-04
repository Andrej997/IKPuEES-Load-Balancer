#pragma comment(lib, "Ws2_32.lib")
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5059"
#define DEFAULT_PORT_WORKER "27016"

#include "structs.h"
#include "functionDefs.h"
#include "communiocationFuncs.h"
#include "clientList.h"
#include "workerList.h"
#include "threadFuncs.h"

int globalIdClient = 123456;
int globalIdWorker = 1;

int index = 0;
Node *headClients = NULL; // list of clients
NodeW *headWorkers = NULL; // list of workers

int main(void) {
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

	do
	{
		struct sockaddr_in clientAddress;
		int addrlen = sizeof(clientAddress);

		timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;

		FD_SET set;
		FD_ZERO(&set);

		FD_SET(listenSocket, &set);

		iResult = select(0, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			printf("select failed with error: %d\n", WSAGetLastError());
		}
		else if (iResult == 0) {
			printf("I'm waiting\n");
			continue;
		}
		else if (FD_ISSET(listenSocket, &set)) {
			Client *newClient = (Client*)malloc(sizeof(Client));
			newClient->acceptedSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &addrlen);
			if (newClient->acceptedSocket == INVALID_SOCKET)
			{
				printf("accept failed with error: %d\n", WSAGetLastError());
				closesocket(listenSocket);
				WSACleanup();
				return 1;
			}
			char *clientip = new char[20];
			strcpy(clientip, inet_ntoa(clientAddress.sin_addr));
			printf("Client port: %d, IP address: %s is accepted\n", clientAddress.sin_port, clientip);

			DWORD threadId;

			newClient->id = globalIdClient++;
			newClient->acceptedSocket = newClient->acceptedSocket;
			newClient->ipAdr = clientip;
			newClient->port = clientAddress.sin_port;
			newClient->thread = CreateThread(NULL,
				0,
				myThreadFun,
				&newClient->acceptedSocket,
				0,
				&threadId
			);

			printf("-----------\n\tClients[%d]\nid: %d\nip Address: %s\nport: %d\nthread id: threadId:%d \n\n\tis accepted\n----------------\n"
				, index, newClient->id, newClient->ipAdr, newClient->port, threadId);


			AddAtEnd(&headClients, newClient);

			++index;
		}
		else if (FD_ISSET(listenSocketWorker, &set)) {
			// slicna logika 
		}
	} while (1);

	// free all nodes from list
	FreeList(headClients);
	
	closesocket(listenSocket);
	WSACleanup();

	return 0;
}

