#pragma comment(lib, "Ws2_32.lib")
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27016"

//#include "list.h"
#include "structs.h"
#include "functionDefs.h"
#include "communiocationFuncs.h"
#include "clientList.h"


int globalIdClient = 123456;

int index = 0;
Node *headClients = NULL; // list of clients

int main(void) {
	// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;

	// variable used to store function return value
	int iResult;
	// Buffer used for storing incoming data
	char recvbuf[DEFAULT_BUFLEN];
	char sendbuf[DEFAULT_BUFLEN];

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// Prepare address information structures
	addrinfo *resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address 
	// to socket
	iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Set socket at nonbloking mode
	SetNonblocking(&listenSocket);

	// Since we don't need resultingAddress any more, free it
	freeaddrinfo(resultingAddress);

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

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

DWORD WINAPI myThreadFun(void *vargp) {
	SOCKET socket = *(SOCKET*)vargp;
	char recvbuf[DEFAULT_BUFLEN];

	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	struct sockaddr_in clientAddress;
	int addrlen = sizeof(clientAddress);

	while (true) {
		FD_SET set;
		FD_ZERO(&set);

		FD_SET writeSet;
		FD_ZERO(&writeSet);

		FD_SET(socket, &set);
		FD_SET(socket, &writeSet);

		int iResult = select(0, &set, &writeSet, NULL, &timeVal);

		if (FD_ISSET(socket, &set)) {
			SetNonblocking(&socket);
			if (iResult == SOCKET_ERROR) {
				printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
			}
			iResult = recv(socket, recvbuf, DEFAULT_BUFLEN, 0);
			if (iResult > 0)
			{
				Node *temp = headClients;
				while (temp != NULL) {
					if (socket == temp->client->acceptedSocket) {
						printf("Thread id = %d. Client id: %d, port: %d, IP address: %s. Message: %s\n", GetCurrentThreadId(), temp->client->id, temp->client->port, temp->client->ipAdr, recvbuf);
						break;
					}
					temp = temp->next;
				}
			}
			else if (iResult == 0)
			{
				printf("Connection with client closed.\n");
				closesocket(socket);
			}
			else
			{
				// there was an error during recv
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(socket);
			}
		}
		else if (FD_ISSET(socket, &writeSet))
		{
			const char *messageToSend = "OK.";
			iResult = send(socket, messageToSend, (int)strlen(messageToSend) + 1, 0);
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(socket);
				WSACleanup();
				return 1;
			}

			printf("Server bytes Sent: %ld\n", iResult);
			printf("Wait 2 sec . . .\n");
			Sleep(2000);
		}
	}
}