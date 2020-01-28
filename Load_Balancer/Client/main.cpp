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

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 5059

#include "communicationFuncs.h"
#include "clientMethods.h"
#include "threadFuncs.h"

int __cdecl main(int argc, char **argv)
{
	SOCKET connectSocket = SetConnectedSocket(DEFAULT_PORT);
	HANDLE recvHandle = CreateThread(NULL,
		0,
		myThreadFunForRecv,
		&connectSocket,
		0,
		NULL
	);
	int sendMessagecount = 0;
	// socket used to communicate with server
	
	if (connectSocket == 1) {
		printf("Press enter to exit");
		getchar();
		return 0;
	}
	// variable used to store function return value
	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	// message to send
	const char *messageToSend = "this is a test";
	
	char* message = nullptr;
	while (true) {

		FD_SET set;
		FD_ZERO(&set);
		FD_SET(connectSocket, &set);

		timeval timeVal;
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		int iResult = select(0, NULL, &set, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			//error
			printf("Select failed with error: %d\n", WSAGetLastError());
		}
		else if (iResult == 0) {
			printf("I'm waiting...\n");
			continue;
		}
		else if (FD_ISSET(connectSocket, &set)) { // send
			//message = GenerateMessage();
			//while (sendMessagecount < 20) {
				message = Generate10BMsg();
				iResult = send(connectSocket, message, (int)strlen(message) + 1, 0);

				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}
				sendMessagecount++;
				printf("\n\tSend message count: %ld\n", sendMessagecount);
				printf("Bytes Sent: %ld\nMessage: %s\n", iResult, message);
				//getchar();
				//Sleep(2000);
				Sleep(100);
			//}
			//break;
		}
		else {
			//nesto
		}
	}

	free(message);
	// cleanup
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}
