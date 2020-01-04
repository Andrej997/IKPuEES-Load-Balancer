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
#include "threadFuncs.h"

int __cdecl main(int argc, char **argv)
{
	Node *headMessages;

	// socket used to communicate with server
	SOCKET connectSocket = SetConnectedSocket(DEFAULT_PORT);
	// variable used to store function return value
	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	// message to send
	const char *messageToSend = "this is a test";

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
				strcpy_s(recvbuf + sizeof(int), message->size, message->message);
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



