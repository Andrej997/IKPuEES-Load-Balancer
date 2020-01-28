//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#pragma comment(lib, "Ws2_32.lib")
//#define WIN32_LEAN_AND_MEAN
//
//#include <windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <conio.h>
//#include <time.h>
//#include <string.h>
//
//#define DEFAULT_BUFLEN 512
//#define DEFAULT_PORT 27016
//
//#include "communicationFuncs.h"
//#include "structs.h"
//#include "threadFuncs.h"
//
//extern Node *headClients;
//
//DWORD WINAPI myThreadFun(void *vargp) {
//	SOCKET socket = *(SOCKET*)vargp;
//	char recvbuf[DEFAULT_BUFLEN];
//
//	timeval timeVal;
//	timeVal.tv_sec = 1;
//	timeVal.tv_usec = 0;
//	struct sockaddr_in clientAddress;
//	int addrlen = sizeof(clientAddress);
//
//	while (true) {
//		FD_SET set;
//		FD_ZERO(&set);
//
//		FD_SET writeSet;
//		FD_ZERO(&writeSet);
//
//		FD_SET(socket, &set);
//		FD_SET(socket, &writeSet);
//
//		int iResult = select(0, &set, &writeSet, NULL, &timeVal);
//
//		if (FD_ISSET(socket, &set)) {
//			SetNonblocking(&socket);
//			if (iResult == SOCKET_ERROR) {
//				printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
//			}
//			iResult = recv(socket, recvbuf, DEFAULT_BUFLEN, 0);
//			if (iResult > 0)
//			{
//				/*Node *temp = headClients;
//				while (temp != NULL) {
//					if (socket == temp->client->acceptedSocket) {
//						printf("Thread id = %d. Client id: %d, port: %d, IP address: %s. Message: %s\n", GetCurrentThreadId(), temp->client->id, temp->client->port, temp->client->ipAdr, recvbuf);
//						break;
//					}
//					temp = temp->next;
//				}*/
//			}
//			else if (iResult == 0)
//			{
//				printf("Connection with client closed.\n");
//				closesocket(socket);
//			}
//			else
//			{
//				// there was an error during recv
//				printf("recv failed with error: %d\n", WSAGetLastError());
//				closesocket(socket);
//			}
//		}
//		else if (FD_ISSET(socket, &writeSet))
//		{
//			const char *messageToSend = "OK.";
//			iResult = send(socket, messageToSend, (int)strlen(messageToSend) + 1, 0);
//			if (iResult == SOCKET_ERROR)
//			{
//				printf("send failed with error: %d\n", WSAGetLastError());
//				closesocket(socket);
//				WSACleanup();
//				return 1;
//			}
//
//			printf("Server bytes Sent: %ld\n", iResult);
//			printf("Wait 2 sec . . .\n");
//			Sleep(2000);
//		}
//	}
//}