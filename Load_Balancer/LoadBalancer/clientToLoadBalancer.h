#pragma once

DWORD WINAPI RecvClientMessage(void *vargp);

DWORD WINAPI RecvClientMessage(void *vargp) {
	SOCKET socket = *(SOCKET*)vargp;
	//char recvbuf[DEFAULT_BUFLEN];
	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;
	struct sockaddr_in clientAddress;
	int addrlen = sizeof(clientAddress);
	int numberRecv = 0;
	while (true) {
		#pragma region SET
		FD_SET set;
		FD_ZERO(&set);
		FD_SET writeSet;
		FD_ZERO(&writeSet);
		FD_SET(socket, &set);
		FD_SET(socket, &writeSet);
		#pragma endregion

		char recvbuf[DEFAULT_BUFLEN];
		int iResult = select(0, &set, &writeSet, NULL, &timeVal);

		if (FD_ISSET(socket, &set)) {
			SetNonblocking(&socket);
			if (iResult == SOCKET_ERROR) {
				printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
				break;
			}
			iResult = recv(socket, recvbuf, DEFAULT_BUFLEN, 0);

			int currentLength = 0;

			if (iResult > 0)
			{
				Node *temp = headClients;
				while (temp != NULL) {
					if (socket == temp->client->acceptedSocket) {
						EnterCriticalSection(&CriticalSectionForOutput);
						printf("Recv client message:\nThread id = %d.\nClient id: %d.\nPort: %d\nIP address: %s.\nLength message: %d\n\n", GetCurrentThreadId(), temp->client->id, temp->client->port, temp->client->ipAdr, iResult);
						LeaveCriticalSection(&CriticalSectionForOutput);
						break;
					}
					temp = temp->next;
				}
				int idClient = -1;
				char idClientStr[4];
				char* ipAddrClient = NULL;
				int portClient = -1;
				char portClientStr[4];

				if (temp != NULL) {
					idClient = temp->client->id;
					for (int i = 0; i < 4; i++)
					{
						idClientStr[i] = ((char*)(&idClient))[i];
					}
					ipAddrClient = temp->client->ipAdr;
					portClient = temp->client->port;
					for (int i = 0; i < 4; i++)
					{
						portClientStr[i] = ((char*)(&portClient))[i];
					}
				}

				int lengthCurrentMessage = 0;

				while (currentLength < iResult) {
					lengthCurrentMessage = strlen(recvbuf + currentLength);
					currentLength += lengthCurrentMessage + 1;
					int lengthMessage = 4 + 4 + strlen(ipAddrClient) + lengthCurrentMessage;
					char* message = (char*)malloc(lengthMessage);
					for (int i = 0, y = 0, z = 0; i < lengthMessage; i++)
					{
						if (i <= 3) {
							message[i] = idClientStr[y];
							y++;
						}
						else if (i <= 7) {
							y = 0;
							message[i] = portClientStr[z];
							z++;
						}
						else if (i < (8 + strlen(ipAddrClient))) {
							z = 0;
							message[i] = ipAddrClient[y];
							y++;
						}
						else {
							message[i] = recvbuf[z];
							z++;
						}
					}
				
					/*EnterCriticalSection(&CriticalSectionForOutput);
					printf("lengthCurrentMessage = %d\n", lengthCurrentMessage);
					LeaveCriticalSection(&CriticalSectionForOutput);*/

					WaitForSingleObject(WriteSemaphore, INFINITE);

					EnterCriticalSection(&CriticalSectionForQueue);
					Enqueue(primaryQueue, message, lengthMessage);
					//if (primaryQueue->size > (primaryQueue->capacity * 0.5))
					LeaveCriticalSection(&CriticalSectionForQueue);

					ReleaseSemaphore(ReadSemaphore, 1, NULL);
					//ReleaseSemaphore(WriteSemaphore, 1, NULL);
					
					/*EnterCriticalSection(&CriticalSectionForOutput);
					EnterCriticalSection(&CriticalSectionForQueue);
					printf("Start\n");
					printf("Queue: size = %d, capacity = %d, front: %d, rear = %d \n\nSTART\n", primaryQueue->size, primaryQueue->capacity, primaryQueue->front, primaryQueue->rear);
					for (int i = 0; i < primaryQueue->capacity; i++)
					{
						printf("%c", primaryQueue->array[i]);
					}
					printf("\nEND\n\n");
					LeaveCriticalSection(&CriticalSectionForQueue);
					LeaveCriticalSection(&CriticalSectionForOutput);*/
					numberRecv++;
					printf("Total recv client message: %d\n", numberRecv);
					free(message);
				}

			}
			else if (iResult == 0)
			{
				printf("Connection with client closed.\n");
				closesocket(socket);
				break;
			}
			else
			{	// there was an error during recv
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(socket);
				break;
			}
		}
	}
	deleteNode(&headClients, socket);
	return 0;
}