#pragma once

DWORD WINAPI SendAndRecvWorkerMessage(void *vargp);

DWORD WINAPI SendAndRecvWorkerMessage(void *vargp) {
	SOCKET socket = *(SOCKET*)vargp;
	char recvbuf[DEFAULT_BUFLEN];
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
							send(clientTemp->client->acceptedSocket, mes, 1 + 1, 0);
							EnterCriticalSection(&CriticalSectionForOutput);
							printf("Worker send LB: %s...\n", (recvbuf + currentLength));
							LeaveCriticalSection(&CriticalSectionForOutput);
						}
						else {
							EnterCriticalSection(&CriticalSectionForOutput);
							printf("Klijent (id = %d) ne postoji ili se ugasio pa ne mozemo da mu posaljemo poruku o uspenom slanju...\n", sucId);
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
						//int brojPoruka = 0;
						//if (temp != NULL)
						//	brojPoruka++;
						while (temp != NULL) {
							//brojPoruka++;
							//printf("%s\n", temp->message);
							if (temp->worker->acceptedSocket == socket) {
								temp->worker->counter -= numOfMsg;
							}
							temp = temp->next;
						}
						//printf("Ukupan broj poruka:%d\n", brojPoruka);
						while (i < numOfMsg) {
							EnterCriticalSection(&CriticalSectionForReorQueue);
							if(reorQueue != NULL)
								Enqueue(reorQueue, recvbuf + 5 + 4 + i * 4 + prevMsgLength, nextMsg);
							LeaveCriticalSection(&CriticalSectionForReorQueue);

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
						//currentLength += msgLength;
						//currentLength = currentLength + prevMsgLength + nextMsg + 4;
						//EnterCriticalSection(&CriticalSectionForReorQueue); //rizicno mesto za kriticnu sekciju jer moramo znati kada je otpustiti
						MergeSortWorkerList(&headWorkers);

						bool access = false;
						EnterCriticalSection(&CriticalSectionForReorQueue);
						if (reorQueue != NULL) {
							while (reorQueue->size > 0) {
								//LeaveCriticalSection(&CriticalSectionForReorQueue);
								access = true;
								//MergeSortWorkerList(&headWorkers);
								//EnterCriticalSection(&CriticalSectionForReorQueue);
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
									printf("(myThreadFunWorker) send");
									free(deq);
									LeaveCriticalSection(&CriticalSectionForOutput);
								}
								if (iResult == SOCKET_ERROR) {
									EnterCriticalSection(&CriticalSectionForOutput);
									printf("(myThreadFunWorker) send failed with error: %d\n", WSAGetLastError());
									LeaveCriticalSection(&CriticalSectionForOutput);
									closesocket(headWorkers->worker->acceptedSocket);
									WSACleanup();
									return 1;
								}
								MergeSortWorkerList(&headWorkers);
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
						//MergeSortWorkerList(&headWorkers);
						DestroyQueue(reorQueue);
						reorQueue = NULL;
						Sleep(100);
						ReleaseSemaphore(ReorganizeSemaphoreEnd, 1, NULL);
					}
					/*else if (*(char*)recvbuf == 'O' && *(char*)(recvbuf + 1) == 'K') {
						EnterCriticalSection(&CriticalSectionForOutput);
						printf("recv message for worker: OK\n");
						LeaveCriticalSection(&CriticalSectionForOutput);
					}*/
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