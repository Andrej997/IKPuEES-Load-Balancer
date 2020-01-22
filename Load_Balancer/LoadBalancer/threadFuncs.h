#pragma once

#include "ringBuffer.h"

#pragma region Extern variables
extern Node *headClients;
extern NodeW *headWorkers;

extern Queue* primaryQueue;
extern Queue* tempQueue;
extern Queue* secondaryQueue;
Queue* reorQueue;
extern CRITICAL_SECTION CriticalSectionForQueue;
extern CRITICAL_SECTION CriticalSectionForOutput;
CRITICAL_SECTION CriticalSectionForReorQueue;

extern HANDLE WriteSemaphore, WriteSemaphoreTemp, ReadSemaphore, CreateQueueSemaphore;
extern HANDLE ReorganizeSemaphoreStart, ReorganizeSemaphoreEnd, TrueSemaphore;
#pragma endregion

DWORD WINAPI myThreadFun(void *vargp) {
	SOCKET socket = *(SOCKET*)vargp;
	char recvbuf[DEFAULT_BUFLEN];

	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;
	struct sockaddr_in clientAddress;
	int addrlen = sizeof(clientAddress);

	while (true) {
		#pragma region SET
		FD_SET set;
		FD_ZERO(&set);

		FD_SET writeSet;
		FD_ZERO(&writeSet);

		FD_SET(socket, &set);
		FD_SET(socket, &writeSet);
		#pragma endregion

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
						EnterCriticalSection(&CriticalSectionForOutput);
						printf("Thread id = %d. Client id: %d, port: %d, IP address: %s. Message: %s\n", GetCurrentThreadId(), temp->client->id, temp->client->port, temp->client->ipAdr, recvbuf);
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
				int lengthMessage = 4 + 4 + strlen(ipAddrClient) + strlen(recvbuf);
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
					else if(i < (8 + strlen(ipAddrClient))) {
						z = 0;
						message[i] = ipAddrClient[y];
						y++;
					}
					else {
						message[i] = recvbuf[z];
						z++;
					}
					
				}
				
				bool more = false;
				EnterCriticalSection(&CriticalSectionForOutput);
				printf("Strlen(recvBuffer) = %d\n", strlen(recvbuf));
				LeaveCriticalSection(&CriticalSectionForOutput);

				//HANDLE semaphores[2] = { WriteSemaphore, WriteSemaphoreTemp };

				//int result = WaitForMultipleObjects(2, semaphores, FALSE, INFINITE);

				//if (result == WAIT_OBJECT_0 + 1) {
				//	printf("WriteSemaphoreTemp...\n");
				//	Enqueue(tempQueue, message, lengthMessage);
				//	ReleaseSemaphore(WriteSemaphoreTemp, 1, NULL);
				//	printf("TempQueue: size = %d, capacity = %d, front: %d, rear = %d \n\nSTART\n", tempQueue->size, tempQueue->capacity, tempQueue->front, tempQueue->rear);
				//	for (int i = 0; i < tempQueue->capacity; i++)
				//	{
				//		printf("%c", tempQueue->array[i]);
				//	}
				//	printf("\nEND\n\n");
				//}
				//else if (result == WAIT_OBJECT_0) {
				//	printf("WriteSemaphore...\n");
				//	//WaitForSingleObject(WriteSemaphore, INFINITE);
				//	EnterCriticalSection(&CriticalSectionForQueue);
				//	Enqueue(primaryQueue, message, lengthMessage);
				//	if (primaryQueue->size > (primaryQueue->capacity * 0.5))
				//		more = true;
				//	LeaveCriticalSection(&CriticalSectionForQueue);
				//	if (more)
				//		ReleaseSemaphore(ReadSemaphore, 2, NULL);
				//	else
				//		ReleaseSemaphore(ReadSemaphore, 1, NULL);
				//}
				//

				
				//printf("WriteSemaphore...\n");
				WaitForSingleObject(WriteSemaphore, INFINITE);
				EnterCriticalSection(&CriticalSectionForQueue);
				Enqueue(primaryQueue, message, lengthMessage);
				if (primaryQueue->size > (primaryQueue->capacity * 0.5))
					more = true;
				LeaveCriticalSection(&CriticalSectionForQueue);
				if (more)
					ReleaseSemaphore(ReadSemaphore, 2, NULL);
				else
					ReleaseSemaphore(ReadSemaphore, 1, NULL);


				EnterCriticalSection(&CriticalSectionForOutput);
				EnterCriticalSection(&CriticalSectionForQueue);

				printf("Queue: size = %d, capacity = %d, front: %d, rear = %d \n\nSTART\n", primaryQueue->size, primaryQueue->capacity, primaryQueue->front, primaryQueue->rear);
				for (int i = 0; i < primaryQueue->capacity; i++)
				{
					printf("%c", primaryQueue->array[i]);
				}
				printf("\nEND\n\n");

				LeaveCriticalSection(&CriticalSectionForQueue);
				LeaveCriticalSection(&CriticalSectionForOutput);

				free(message);
			}
			else if (iResult == 0)
			{
				printf("Connection with client closed.\n");
				closesocket(socket);
				break;
			}
			else
			{
				// there was an error during recv
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(socket);
				break;
			}
		}
		/*else if (FD_ISSET(socket, &writeSet))
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
		}*/
	}
	deleteNode(&headClients, socket);
	return 0;
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
			if (*(char*)recvbuf == 'r') {
				printf("MATER MU\n\n\n");
				int numOfMsg = *(int*)(recvbuf + 1);
				int i = 0;
				int msgLength = *(int*)(recvbuf + 5);
				while (i < numOfMsg) {
					
					//EnterCriticalSection(&CriticalSectionForReorQueue);
					//Enqueue(reorQueue, recvbuf, msgLength);
					//LeaveCriticalSection(&CriticalSectionForReorQueue);
					++i;
					int nextMsg = *(int*)(recvbuf + 9 + msgLength);
					msgLength += nextMsg;
				}

			}
			if (iResult > 0)
			{
				Node *temp = headClients;
				while (temp != NULL) {
					if (socket == temp->client->acceptedSocket) {
						EnterCriticalSection(&CriticalSectionForOutput);
						printf("Thread id = %d. Client id: %d, port: %d, IP address: %s. Message: %s\n", GetCurrentThreadId(), temp->client->id, temp->client->port, temp->client->ipAdr, recvbuf);
						LeaveCriticalSection(&CriticalSectionForOutput);
						
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
				break;
			}
		}
		else if (FD_ISSET(socket, &writeSet))
		{
			/*const char *messageToSend = "OK.";
			iResult = send(socket, messageToSend, (int)strlen(messageToSend) + 1, 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(socket);
				WSACleanup();
				return 1;
			}
			Sleep(3000);*/
		}
	}
	deleteNodeW(&headWorkers, socket);
	return 0;
}

DWORD WINAPI Dispecher(void *vargp) {
	while (true) {
		// dispecer staje ako se prijavio novi worker
		//int pom = WaitForSingleObject(ReorganizeSemaphoreEnd, INFINITE);

		HANDLE semaphores[3] = { ReorganizeSemaphoreStart, ReorganizeSemaphoreEnd, TrueSemaphore };

		int result = WaitForMultipleObjects(3, semaphores, FALSE, INFINITE);
		if (result == WAIT_OBJECT_0) { //zapoceta reorganizacija
			WaitForSingleObject(ReorganizeSemaphoreEnd, INFINITE);
			//continue;
		}
		else if (result == WAIT_OBJECT_0 + 1) {
			continue;
		}
		else {
			ReleaseSemaphore(TrueSemaphore, 1, NULL);
		}

		EnterCriticalSection(&CriticalSectionForQueue);
		if (primaryQueue->size != 0 && headWorkers != NULL) {

			bool isEmpty = true;

			WaitForSingleObject(ReadSemaphore, INFINITE);

			char* deq = Dequeue(primaryQueue);
			//if (primaryQueue->size != 0)
				//isEmpty = false;
			LeaveCriticalSection(&CriticalSectionForQueue);

			//if (isEmpty)
				ReleaseSemaphore(WriteSemaphore, 1, NULL);
			//else
				//ReleaseSemaphore(ReadSemaphore, 1, NULL);

			char strlenMessageString[4];

			for (int i = 0; i < 4; i++)
			{
				strlenMessageString[i] = deq[i];
			}
			int strlenMessageInt = *(int*)strlenMessageString;

			int iResult = send(headWorkers->worker->acceptedSocket, deq, strlenMessageInt + 5, 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(headWorkers->worker->acceptedSocket);
				WSACleanup();
				return 1;
			}
			++headWorkers->worker->counter;

			MergeSortWorkerList(&headWorkers);

			Sleep(1000);
		}
		else { 
			// posto nije ispunjen bio uslov, moze da se napusti kriticna sekcija
			LeaveCriticalSection(&CriticalSectionForQueue);
		}
	}
	return 0;
}

DWORD WINAPI WorkWithQueue(void *vargp) {
	while (true) {
		//printf("Pre poziva WaitForSingleObject(CreateQueueSemaphore, INFINITE);\n");

		WaitForSingleObject(CreateQueueSemaphore, INFINITE);
		
		tempQueue = CreateQueue(primaryQueue->capacity * 0.4); //kreriamo pomocni sa 40% kapaciteta primarnog

		secondaryQueue = CreateQueue(primaryQueue->capacity * 2); //kreiramo novi buffer koji je od starog veci duplo
		//printf("tempQueue i secondaryQueue su kreirani, zelimo da poruke nastave da se primaju u tempQueue\n");
		ReleaseSemaphore(CreatedQueueSemaphore, 1, NULL);

		//ReleaseSemaphore(ReadSemaphore, 10, NULL);

		//PrimaryToSecondary();

		//Sleep(10000);


		//WaitForSingleObject(WriteSemaphoreTemp, 4000);

		//TempToPrimary();

		//ReleaseSemaphore(WriteSemaphore, 1, NULL);

		
		Sleep(3000); //svake 3 sekunde proveramo
	}
	return 0;
}

DWORD WINAPI Redistributioner(void *vargp) {
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	while (true) {
		FD_SET set;
		FD_ZERO(&set);

		WaitForSingleObject(ReorganizeSemaphoreStart, INFINITE); //cekamo na semaforu dok ne dobijemo signal da pocnemo reorganizaciju

		int numOfWorkers = GetNumOfWorkers(headWorkers);
		int numOfMsg = GetAllMessages(headWorkers);
		int msgPerWorker = numOfMsg / numOfWorkers;
		int *arrOfMsg = GiveMe(msgPerWorker, headWorkers);
		//int *arr = (int*)vargp;
		int i = 0;
		NodeW *temp = headWorkers;
		union {
			int num;
			char byte[4];
		}myUnion;
		while (i < numOfWorkers - 1) {
			char *msg = (char*)malloc(1 /*r*/ + sizeof(int));
			myUnion.num = arrOfMsg[i];
			msg[0] = 'r';
			memcpy(msg + 1, myUnion.byte, 4); // salje se npr. r5 i worker zna da je 'r' za reorganizaciju i '5' broj poruka
			int iResult=0;// = send(temp->worker->acceptedSocket, msg, 6, 0);
			if (iResult == SOCKET_ERROR) {
				EnterCriticalSection(&CriticalSectionForOutput);
				printf("Redistributioner messsage: send failed with error: %d\n", WSAGetLastError());
				LeaveCriticalSection(&CriticalSectionForOutput);
				//closesocket(headWorkers->worker->acceptedSocket);
				//WSACleanup();
				//return 1;
			}
			free(msg);
			char recvbuf[DEFAULT_BUFLEN];
			//Sleep(5900);
			//Sleep(1000);
			//while (true) {
			//	FD_SET set1;
			//	FD_ZERO(&set1);
			//	iResult = select(0, &set1, NULL, NULL, &timeVal);

			//	//if (FD_ISSET(temp->worker->acceptedSocket, &set1)) {
			//		//SetNonblocking(&temp->worker->acceptedSocket);
			//		iResult = recv(temp->worker->acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
			//		if (iResult == SOCKET_ERROR) {
			//			EnterCriticalSection(&CriticalSectionForOutput);
			//			printf("Redistributioner messsage: recv failed with error: %d\n", WSAGetLastError());
			//			LeaveCriticalSection(&CriticalSectionForOutput);
			//			//closesocket(headWorkers->worker->acceptedSocket);
			//			//WSACleanup();
			//			//return 1;
			//		}
			//		else if (iResult > 0)
			//		{
			//			printf("Stiglo je\n\n");
			//			break;
			//		}
			//	//}
			//}
			//
			
			temp = temp->next;
			++i;
		}

		ReleaseSemaphore(ReorganizeSemaphoreEnd, 1, NULL); //posto smo zavrsili reorganizaciju saljemo signal da moze dispacher da nastavi dalje
		Sleep(2000);
	}
	
	return 0;
}