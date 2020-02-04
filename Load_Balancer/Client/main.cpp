#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN

#pragma region IncludeLibrary
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#pragma endregion
#pragma region DefaultValue
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 5059
#pragma endregion
#pragma region IncludeHeaderFiles
#include "communicationFuncs.h"
#include "clientMethods.h"
#include "workerToClientOK.h"
#pragma endregion

int __cdecl main(int argc, char **argv)
{
	#pragma region InitVariable
	int sendMessagecount = 0;
	int iResult = -1; // variable used to store function return value
	char recvbuf[DEFAULT_BUFLEN];
	char* message = nullptr;
	SOCKET connectSocket; // socket used to communicate with server
	HANDLE recvHandle;
	#pragma endregion
	connectSocket = SetConnectedSocket(DEFAULT_PORT);
	#pragma region CreateThread
	recvHandle = CreateThread(NULL,
		0,
		RecvMessageOK,
		&connectSocket,
		0,
		NULL
	);
	#pragma endregion

	if (connectSocket == 1) {
		printf("Press enter to exit...\n");
		getchar();
		return 0;
	}
	printf("Client is started...\nPress enter to exit...\n");

	while (true) {
		if (_kbhit()) {
			break;
		}
		#pragma region Set
		FD_SET set;
		FD_ZERO(&set);
		FD_SET(connectSocket, &set);
		#pragma endregion

		timeval timeVal;
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		iResult = select(0, NULL, &set, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {	//error
			printf("Select failed with error: %d\n", WSAGetLastError());
			break;
		}
		else if (iResult == 0) {
			printf("I'm waiting...\n");
			continue;
		}
		else if (FD_ISSET(connectSocket, &set)) { // send
			#pragma region SendRandomMessageEvery1000ms
			//message = GenerateMessage();
			//iResult = send(connectSocket, message, (int)strlen(message) + 1, 0);

			//if (iResult == SOCKET_ERROR)
			//{
			//	printf("send failed with error: %d\n", WSAGetLastError());
			//	break;
			//	//closesocket(connectSocket);
			//	//WSACleanup();
			//	//return 1;
			//}
			//sendMessagecount++;
			//printf("\n\tSend message count: %ld\n", sendMessagecount);
			//printf("Bytes Sent: %ld\nMessage: %s\n", iResult, message);
			//Sleep(1000);
			#pragma endregion

			#pragma region SendRandomMessageEvery100ms
			/*message = GenerateMessage();
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
			Sleep(100);*/
			#pragma endregion

			#pragma region Send20MessageLength10B
			while (sendMessagecount < 20000) {
				if (_kbhit())
					break;
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
				Sleep(500);
			}
			#pragma endregion
			printf("Client sent all messages...\n");
			getchar();
			break;
			
		}
	}
	#pragma region Clenup
	free(message);
	closesocket(connectSocket);
	CloseHandle(recvHandle);
	WSACleanup();
	#pragma endregion
	printf("Client is successfully shutdown...\n");
	getchar();
	return 0;
}


