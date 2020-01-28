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
Node *headMessages;
#include "threadFuncs.h"

int __cdecl main(int argc, char **argv)
{
	// number of messages that this worker contains
	int msgCount = 0; 
	headMessages = NULL;

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
	const char *messageToSend = "this is a test";

	while (true) {
		FD_SET set;
		FD_ZERO(&set);
		FD_SET(connectSocket, &set);

		FD_SET recvset;
		FD_ZERO(&recvset);
		FD_SET(connectSocket, &recvset);

		timeval timeVal;
		timeVal.tv_sec = 0;
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
				//printf("Wait...\n");

				// za reorganizaciju
				if (*(char*)recvbuf == 'r') {
					printf("Worker recv: %s\n", recvbuf);
					int numOfMgs = *(int*)(recvbuf + 1);
					int a = 0;
					
					char *reorMessage = ConvertToString(headMessages, numOfMgs, &a);

					iResult = select(0, NULL, &set, NULL, &timeVal);
					if (FD_ISSET(connectSocket, &set)) {
						//SetNonblocking(&connectSocket);
						iResult = send(connectSocket, reorMessage, a + 1, 0);
						if (iResult == SOCKET_ERROR)
								{
									printf("send failed with error: %d\n", WSAGetLastError());
							//		closesocket(connectSocket);
							//		WSACleanup();
							//		return 1;
								}
					}
					printf("Vraceno %d ...\n", numOfMgs);
					Node* temp = headMessages;
					int brojPoruka = 0;
					if (temp != NULL)
						brojPoruka++;
					while (temp->next != NULL) {
						brojPoruka++;
						//printf("%s\n", temp->message);
						temp = temp->next;
					}
					printf("Ukupan broj poruka:%d\n", brojPoruka);
				}
				else if (*(char*)recvbuf != 'O') {	//aaaaaaaaaa
					printf("Message received from server(");
					Message *message = (Message*)malloc(sizeof(Message));
					message->size = *(int*)recvbuf;
					message->message = (char*)malloc(message->size);
					message->clientId = *(int*)(recvbuf + 4);
					++msgCount;
					
					printf("%d) : ", message->size - 17);
					for (int i = 0; i < message->size + 4; i++)
					{
						message->message[i] = recvbuf[i];
						if(i > 21)
							printf("%c", message->message[i]);
					}
					printf("\n");
					printf("ClientId : %d\n", message->clientId);
					//printf("\t\tCurrent messages count : %d\n", msgCount);
					AddAtEnd(&headMessages, message);
					Node* temp = headMessages;
					int brojPoruka = 0;
					if (temp != NULL)
						brojPoruka++;
					while (temp->next != NULL) {
						brojPoruka++;
						temp = temp->next;
					}
					printf("Ukupan broj poruka:%d\n", brojPoruka);
					//free(message);
					union {
						int id;
						char buff[4];
					}MyU;
					MyU.id = message->clientId;
					char* messageOK = (char*)malloc(1 + sizeof(int));
					messageOK[0] = 's';
					//memset(messageOK + 1, *MyU.buff, 5);
					messageOK[1] = MyU.buff[0];
					messageOK[2] = MyU.buff[1];
					messageOK[3] = MyU.buff[2];
					messageOK[4] = MyU.buff[3];
					iResult = send(connectSocket, messageOK, (int)strlen(messageOK) + 1, 0);
					if (iResult > 0)
					{
						printf("Sent OK.\n");
					}
					else if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(connectSocket);
						WSACleanup();
						return 1;
					}
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
		else if (FD_ISSET(connectSocket, &set)) { // send
			
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



