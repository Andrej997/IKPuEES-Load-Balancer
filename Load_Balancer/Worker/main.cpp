#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <string.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016

#include "funcDefinitions.h"
#include "communicationFuncs.h"
#include "structs.h"
#include "list.h"

int __cdecl main(int argc, char **argv)
{
	Node *headMessages;

	// socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;
	// variable used to store function return value
	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	// message to send
	const char *messageToSend = "this is a test";

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(DEFAULT_PORT);
	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}

	unsigned long nonBlockingMode = 1;
	iResult = ioctlsocket(connectSocket, FIONBIO, &nonBlockingMode);
	if (iResult == SOCKET_ERROR) {
		printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
	}
	//char* message = "adadas";
	while (true) {

		FD_SET set;
		FD_ZERO(&set);
		FD_SET(connectSocket, &set);

		FD_SET recvset;
		FD_ZERO(&recvset);
		FD_SET(connectSocket, &recvset);

		timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;

		int iResult = select(0, &recvset, &set, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			//error
			printf("Select failed with error: %d\n", WSAGetLastError());
		}
		else if (iResult == 0) {
			printf("I'm waiting...\n");
			continue;
		}
		else if (FD_ISSET(connectSocket, &recvset)) { // recv
			iResult = recv(connectSocket, recvbuf, DEFAULT_BUFLEN, 0);
			if (iResult > 0)
			{
				printf("Message received from server: %s\n", recvbuf);
				Message *message = (Message*)malloc(sizeof(Message));
				message->size = *(int*)recvbuf;
				strcpy(message->message, recvbuf + sizeof(int));
				AddAtEnd(&headMessages, message->message, message->size);
				free(message);
			}
			else if (iResult == 0)
			{
				// connection was closed gracefully
				printf("Connection with server closed.\n");
				closesocket(connectSocket);
			}
			else
			{
				// there was an error during recv
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
			}
		}
		else if (FD_ISSET(connectSocket, &set)) { // send
			//message = "A";
			//iResult = send(connectSocket, message, (int)strlen(message) + 1, 0);

			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			}

			printf("Bytes Sent: %ld\n", iResult);
			//getchar();
			Sleep(3000);
		}
		else {
			//nesto
		}
	}

	//free(message);
	// cleanup
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}



