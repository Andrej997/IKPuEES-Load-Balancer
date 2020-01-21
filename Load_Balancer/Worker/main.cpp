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
	// number of messages that this worker contains
	int msgCount = 0; 

	Node *headMessages = NULL;

	// socket used to communicate with server
	SOCKET connectSocket = SetConnectedSocket(DEFAULT_PORT);
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
				//printf("Message received from server: %s\n", recvbuf);
				printf("Wait...\n");

				// za reorganizaciju
				if (*(char*)recvbuf == 'r') {
					int numOfMgs = *(int*)recvbuf;
					//char *reorMessage = ConvertToString(headMessages, numOfMgs);
					//send(connectSocket, reorMessage, (int)strlen(reorMessage) + 1, 0);
					//FreeMessages(headMessages, numOfMgs);
				}
				if (*(char*)recvbuf != 'O') {
					printf("Message received from server(");
					Message *message = (Message*)malloc(sizeof(Message));
					message->size = *(int*)recvbuf;
					message->message = (char*)malloc(message->size);
					message->clientId = *(int*)(recvbuf + 4);
					++msgCount;
					
					//strcpy_s(recvbuf + sizeof(int), message->size, message->message);
					printf("%d) : ", message->size - 17);
					for (int i = 21; i < message->size + 4; i++)
					{
						message->message[i] = recvbuf[i];
						//if(i > 3)
						printf("%c", message->message[i]);
					}
					printf("\n");
					printf("ClientId : %d\n", message->clientId);
					printf("\t\tCurrent messages count : %d\n", msgCount);
					AddAtEnd(&headMessages, message);
					//free(message);
				}
				
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
				printf("LB crashed!\nPress enter to exit");
				getchar();
				break;
			}
		}
		//else if (FD_ISSET(connectSocket, &set)) { // send
		//	//message = "A";
		//	iResult = send(connectSocket, message, (int)strlen(message) + 1, 0);

		//	if (iResult == SOCKET_ERROR)
		//	{
		//		printf("send failed with error: %d\n", WSAGetLastError());
		//		closesocket(connectSocket);
		//		WSACleanup();
		//		return 1;
		//	}

		//	printf("Bytes Sent: %ld\n", iResult);
		//	//getchar();
		//	Sleep(3000);
		//}
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



