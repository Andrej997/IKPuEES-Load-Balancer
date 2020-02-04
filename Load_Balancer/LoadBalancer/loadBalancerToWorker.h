#pragma once
DWORD WINAPI SendAndRecvWorkerMessage(void *vargp);

DWORD WINAPI SendAndRecvWorkerMessage(void *vargp) {
	SOCKET socket = *(SOCKET*)vargp;
	char recvbuf[DEFAULT_BUFLEN];
	//reorMessageCount = 0;
	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;
	struct sockaddr_in clientAddress;
	int addrlen = sizeof(clientAddress);
	while (true) {
		#pragma region Set
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
				EnterCriticalSection(&CriticalSectionForOutput);
				printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
				LeaveCriticalSection(&CriticalSectionForOutput);
				break;
			}
			int currentLength = 0;

			iResult = recv(socket, recvbuf, DEFAULT_BUFLEN, 0);
			if (iResult > 0){	// poruka uspesno primljena
				
				int lengthCurrentMessage = 0;

				while (currentLength < iResult) {
					if (*(char*)(recvbuf + currentLength) == 's') {
						int sucId = *(int*)(recvbuf + currentLength + 1);
						Node *clientTemp = FindClient(headClients, sucId);
						const char* mes = "s";
						if (clientTemp != NULL) {
							//if (reorMessageCount == 0) {
								send(clientTemp->client->acceptedSocket, mes, 1 + 1, 0);
								EnterCriticalSection(&CriticalSectionForOutput);
								printf("Recv worker message: %s...\n", (recvbuf + currentLength));
								LeaveCriticalSection(&CriticalSectionForOutput);
							//}
							//else {
							//	reorMessageCount--;
							//}
						}
						else {
							EnterCriticalSection(&CriticalSectionForOutput);
							printf("Client[%d] not exist or don't active...\n", sucId);
							LeaveCriticalSection(&CriticalSectionForOutput);
						}
						//free(clientTemp);
						//free(mes);
						currentLength += 5;
					}
					else if (*(char*)recvbuf == 'r') {
						int numOfMsg = *(int*)(recvbuf + 1);
						int i = 0;
						int prevMsgLength = 0;
						int msgLength = *(int*)(recvbuf + 5);
						int nextMsg = msgLength;
						NodeW* temp = headWorkers;
						while (temp != NULL) {
							if (temp->worker->acceptedSocket == socket) {
								temp->worker->counter -= numOfMsg;
							}
							temp = temp->next;
						}
						while (i < numOfMsg) {
							EnterCriticalSection(&CriticalSectionForReorQueue);
							if(reorQueue != NULL)
								Enqueue(reorQueue, recvbuf + 5 + 4 + i * 4 + prevMsgLength, nextMsg);
							LeaveCriticalSection(&CriticalSectionForReorQueue);
							//reorMessageCount++;
							prevMsgLength += nextMsg;
							
							if ((i + 1) != numOfMsg) {
								nextMsg = *(int*)(recvbuf + 9 + i * 4 + msgLength);
								msgLength += nextMsg;
							}
							else {
								currentLength = 5 + 4 + i * 4 + prevMsgLength + 1;
							}
							++i;
						}
						MergeSortWorkerList(&headWorkers);

						bool access = false;
						EnterCriticalSection(&CriticalSectionForReorQueue);
						if (reorQueue != NULL) {
							while (reorQueue->size > 0) {
								access = true;
								//MergeSortWorkerList(&headWorkers);
								char* deq = Dequeue(reorQueue);
								LeaveCriticalSection(&CriticalSectionForReorQueue);

								
								char strlenMessageString[4];
								for (int i = 0; i < 4; i++)
								{
									strlenMessageString[i] = deq[i];
								}
								int strlenMessageInt = *(int*)strlenMessageString;

								int iResult = send(headWorkers->worker->acceptedSocket, deq, strlenMessageInt + 5, 0);
								if (iResult > 0) {
									EnterCriticalSection(&CriticalSectionForOutput);
									printf("Load balancer send message from reor queue..\n");
									free(deq);
									LeaveCriticalSection(&CriticalSectionForOutput);
								}
								if (iResult == SOCKET_ERROR) {
									EnterCriticalSection(&CriticalSectionForOutput);
									printf("Load balancer send message from reor queue failed with error: %d\n", WSAGetLastError());
									LeaveCriticalSection(&CriticalSectionForOutput);
									closesocket(headWorkers->worker->acceptedSocket);
									WSACleanup();
									return 1;
								}
								//MergeSortWorkerList(&headWorkers);
								++headWorkers->worker->counter;
								EnterCriticalSection(&CriticalSectionForReorQueue);
								if (reorQueue == NULL) {
									LeaveCriticalSection(&CriticalSectionForReorQueue);
									break;
								}
								else {
									LeaveCriticalSection(&CriticalSectionForReorQueue);
								}
							}
						}
						else {
							LeaveCriticalSection(&CriticalSectionForReorQueue);
						}
						
						if (!access)
							LeaveCriticalSection(&CriticalSectionForReorQueue);
						MergeSortWorkerList(&headWorkers);
						DestroyQueue(reorQueue);
						reorQueue = NULL;
						Sleep(10);
						ReleaseSemaphore(ReorganizeSemaphoreEnd, 1, NULL);
					}
					/*NodeW *temp = headWorkers;
					while (temp != NULL) {
						if (socket == temp->worker->acceptedSocket) {
							EnterCriticalSection(&CriticalSectionForOutput);
							printf("Thread id = %d. Worker id: %d, port: %d, IP address: %s. Message: %s\n", GetCurrentThreadId(), temp->worker->id, temp->worker->port, temp->worker->ipAdr, recvbuf);
							LeaveCriticalSection(&CriticalSectionForOutput);
							break;
						}
						temp = temp->next;
					}*/
				}
			}
			else if (iResult == 0) {
				EnterCriticalSection(&CriticalSectionForOutput);
				printf("Connection with worker closed.\n");
				LeaveCriticalSection(&CriticalSectionForOutput);
				break;
			}
			else {  // there was an error during recv
				EnterCriticalSection(&CriticalSectionForOutput);
				printf("recv failed with error: %d\n", WSAGetLastError());
				LeaveCriticalSection(&CriticalSectionForOutput);
				closesocket(socket);
				break;
			}
		}
	}
	deleteNodeW(&headWorkers, socket);
	return 0;
}