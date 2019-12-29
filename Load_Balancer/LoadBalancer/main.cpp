#pragma comment(lib, "Ws2_32.lib")
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27016"
#define MC 2

struct Client {
	int id;
	//dodati thread kada ovako normalno profunkcionise xD
	SOCKET socket; //nemam pojma kako se radi sa ovim, to je samo neki broj
	char* ipAdr;
	int port;
};

bool InitializeWindowsSockets();
void InitializeSelect(SOCKET *socket);

int globalIdClient = 123456;

int index = 0;
int  main(void)
{
	Client clients[2]; //doraditi da se dinamicki menja ovaj broj preko realloc-a

	// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;
	// Socket used for communication with client
	SOCKET acceptedSocket[MC];// = INVALID_SOCKET;
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

	unsigned long nonBlockingMode = 1;
	iResult = ioctlsocket(listenSocket, FIONBIO, &nonBlockingMode);
	if (iResult == SOCKET_ERROR) {
		printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
	}

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

		FD_SET writeSet;
		FD_ZERO(&writeSet);

		if (index != MC)
			FD_SET(listenSocket, &set);
		for (int i = 0; i < index; ++i) {
			FD_SET(acceptedSocket[i], &set);
			FD_SET(acceptedSocket[i], &writeSet);
		}

		iResult = select(0, &set, &writeSet, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			printf("select failed with error: %d\n", WSAGetLastError());
		}
		else if (iResult == 0) {
			printf("I'm waiting\n");
			continue;
		}
		else if (FD_ISSET(listenSocket, &set)) {
			acceptedSocket[index] = accept(listenSocket, (struct sockaddr *)&clientAddress, &addrlen);
			if (acceptedSocket[index] == INVALID_SOCKET)
			{
				printf("accept failed with error: %d\n", WSAGetLastError());
				closesocket(listenSocket);
				WSACleanup();
				return 1;
			}
			char *clientip = new char[20];
			strcpy(clientip, inet_ntoa(clientAddress.sin_addr));
			printf("Client port: %d, IP address: %s is accepted\n", clientAddress.sin_port, clientip);

			clients[index].id = globalIdClient++;
			clients[index].socket = acceptedSocket[index];
			clients[index].ipAdr = clientip;
			clients[index].port = clientAddress.sin_port;

			++index;
		}
		else {
			for (int i = 0; i < index; ++i) {
				if (FD_ISSET(acceptedSocket[i], &set)) {
					iResult = ioctlsocket(acceptedSocket[i], FIONBIO, &nonBlockingMode);
					if (iResult == SOCKET_ERROR) {
						printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
					}
					iResult = recv(acceptedSocket[i], recvbuf, DEFAULT_BUFLEN, 0);
					if (iResult > 0)
					{
						printf("Message received from client(port: %d, ip: %s, id: %d): %s.\n",clients[i].port, clients[i].ipAdr, clients[i].id, recvbuf);
					}
					else if (iResult == 0)
					{
						// connection was closed gracefully
						printf("Connection with client closed.\n");
						closesocket(acceptedSocket[i]);
						for (int j = i; j < index - 1; ++j) {
							acceptedSocket[j] = acceptedSocket[j + 1];
						}
						acceptedSocket[index - 1] = 0;
						--index;	
						//*****************************treba sortirati niz klijenata i izbaciti tog koji je pukao ili se ugasio
					}
					else
					{
						// there was an error during recv
						printf("recv failed with error: %d\n", WSAGetLastError());
						closesocket(acceptedSocket[i]);
						for (int j = i; j < index - 1; ++j) {
							acceptedSocket[j] = acceptedSocket[j + 1];
						}
						acceptedSocket[index - 1] = 0;
						--index;
						//***************************** i ovde
					}
				}
				else if (FD_ISSET(acceptedSocket[i], &writeSet))
				{
					const char *messageToSend = "OK.";
					iResult = send(acceptedSocket[i], messageToSend, (int)strlen(messageToSend) + 1, 0);
					if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(acceptedSocket[i]);
						WSACleanup();
						return 1;
					}

					printf("Server bytes Sent: %ld\n", iResult);
					printf("Wait 2 sec . . .\n");
					Sleep(2000);
				}
			}
		}
		
	} while (1);

	for (int i = 0; i < index; ++i) {
		iResult = shutdown(acceptedSocket[i], SD_SEND);
		if (iResult == SOCKET_ERROR)
		{
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(acceptedSocket[i]);
			WSACleanup();
			return 1;
		}
	}
	
	closesocket(listenSocket);
	WSACleanup();

	return 0;
}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

void InitializeSelect(SOCKET *socket) {
	while (true) {
		FD_SET set;
		FD_ZERO(&set);
		FD_SET(*socket, &set);

		timeval timeVal;
		timeVal.tv_sec = 2;
		timeVal.tv_usec = 0;

		int iResult = select(0, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			//error
		}
		else if (iResult == 0) {
			printf("I'm waiting...\n");
			continue;
		}
		else if (iResult > 0) {
			break;
		}
	}
}
