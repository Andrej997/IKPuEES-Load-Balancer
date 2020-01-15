#pragma once

extern Node *headClients;
extern NodeW *headWorkers;

extern Queue* primaryQueue;
extern CRITICAL_SECTION CriticalSectionForQueue;

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
				/*int idClient = -1;
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

				while (recvbuf)
				{

				}*/

				EnterCriticalSection(&CriticalSectionForQueue);
				printf("Strlen(recvBuffer) = %d\n", strlen(recvbuf));

				Enqueue(primaryQueue, recvbuf);
				printf("Queue: size = %d, capacity = %d, front: %d, rear = %d \n\nSTART\n", primaryQueue->size, primaryQueue->capacity, primaryQueue->front, primaryQueue->rear);
				for (int i = 0; i < primaryQueue->capacity; i++)
				{
					printf("%c", primaryQueue->array[i]);
				}
				printf("\nEND\n\n");
				LeaveCriticalSection(&CriticalSectionForQueue);
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

			printf("Server bytes Sent: %ld (Message: %s)\n", iResult, messageToSend);
			printf("Wait 2 sec . . .\n");
			Sleep(2000);
		}
	}
} 

DWORD WINAPI myThreadFunWorker(void *vargp) {
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
			if (primaryQueue->size != 0) {

				EnterCriticalSection(&CriticalSectionForQueue);
				char* deq = Dequeue(primaryQueue);
				LeaveCriticalSection(&CriticalSectionForQueue);

				char strlenMessageString[4];

				for (int i = 0; i < 4; i++)
				{
					strlenMessageString[i] = deq[i];
					//queue->front = (queue->front + 1) % queue->capacity;
				}
				int strlenMessageInt = *(int*)strlenMessageString;

				iResult = send(socket, deq, strlenMessageInt + 5, 0);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(socket);
					WSACleanup();
					return 1;
				}
			}
			else {
				const char *messageToSend = "OK.";
				iResult = send(socket, messageToSend, (int)strlen(messageToSend) + 1, 0);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(socket);
					WSACleanup();
					return 1;
				}
			}

			
			Sleep(3000);
		}
	}
}