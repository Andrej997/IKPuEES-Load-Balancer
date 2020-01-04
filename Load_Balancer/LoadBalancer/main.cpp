#pragma comment(lib, "Ws2_32.lib")
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5059"
#define DEFAULT_PORT_WORKER "27016"

//#include "list.h"
#include "structs.h"
#include "functionDefs.h"
#include "communiocationFuncs.h"
#include "clientList.h"
#include "threadFuncs.h"

int globalIdClient = 123456;
int globalIdWorker = 1;

int index = 0;
Node *headClients = NULL; // list of clients

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

		/*FD_SET writeSet;
		FD_ZERO(&writeSet);*/

		FD_SET(listenSocket, &set);
		
		/*Node *temp = headClients;
		while (temp != NULL) {
			FD_SET(temp->client->acceptedSocket, &set);
			FD_SET(temp->client->acceptedSocket, &writeSet);
			temp = temp->next;
		}
		free(temp);*/

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
		//else {
		//	Node *temp = headClients;
		//	while (temp != NULL) {
		//		if (FD_ISSET(temp->client->acceptedSocket, &set)) {
		//			iResult = ioctlsocket(temp->client->acceptedSocket, FIONBIO, &nonBlockingMode);
		//			if (iResult == SOCKET_ERROR) {
		//				printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
		//			}
		//			iResult = recv(temp->client->acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
		//			if (iResult > 0) {
		//				printf("Message received from client(port: %d, ip: %s, id: %d): %s.\n", temp->client->port, temp->client->ipAdr, temp->client->id, recvbuf);
		//			}
		//			else if (iResult == 0) {
		//				// connection was closed gracefully
		//				printf("Connection with client closed.\n");
		//				closesocket(temp->client->acceptedSocket);
		//				/*
		//				for (int j = i; j < index - 1; ++j) {
		//					acceptedSocket[j] = acceptedSocket[j + 1];
		//				}
		//				acceptedSocket[index - 1] = 0;
		//				--index; */
		//				//*****************************treba sortirati niz klijenata i izbaciti tog koji je pukao ili se ugasio
		//			}
		//			else {
		//				// there was an error during recv
		//				printf("recv failed with error: %d\n", WSAGetLastError());
		//				closesocket(temp->client->acceptedSocket);
		//				/*
		//				for (int j = i; j < index - 1; ++j) {
		//					acceptedSocket[j] = acceptedSocket[j + 1];
		//				}
		//				acceptedSocket[index - 1] = 0;
		//				--index; */
		//				//***************************** i ovde
		//			}
		//		}
		//		else if (FD_ISSET(temp->client->acceptedSocket, &writeSet)) {
		//			const char *messageToSend = "OK.";
		//			iResult = send(temp->client->acceptedSocket, messageToSend, (int)strlen(messageToSend) + 1, 0);
		//			/*
		//			if (iResult == SOCKET_ERROR)
		//			{
		//				printf("send failed with error: %d\n", WSAGetLastError());
		//				closesocket(acceptedSocket[i]);
		//				WSACleanup();
		//				return 1;
		//			}
		//			*/
		//			printf("Server bytes Sent: %ld\n", iResult);
		//			printf("Wait 2 sec . . .\n");
		//			Sleep(2000);
		//		}
		//		temp = temp->next;
		//	}
		//	free(temp);
		//}
		
	} while (1);

	// free all nodes from list
	FreeList(headClients);
	
	closesocket(listenSocket);
	WSACleanup();

	return 0;
}

