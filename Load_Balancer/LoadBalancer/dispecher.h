#pragma once

DWORD WINAPI Dispecher(void *vargp);

DWORD WINAPI Dispecher(void *vargp) {
	
	while (true) {
		HANDLE semaphores[3] = { ReorganizeSemaphoreStart, ReorganizeSemaphoreEnd, TrueSemaphore };

		int result = WaitForMultipleObjects(3, semaphores, FALSE, INFINITE);

		if (result == WAIT_OBJECT_0) { //zapoceta reorganizacija
			//printf("Pre wait ReorganizeSemaphoreEnd..\n");
			WaitForSingleObject(ReorganizeSemaphoreEnd, INFINITE);
			//printf("Prosao wait ReorganizeSemaphoreEnd..\n");
		}
		else if (result == WAIT_OBJECT_0 + 1) {
			continue;
		}
		else {
			ReleaseSemaphore(TrueSemaphore, 1, NULL);
		}
		if (headWorkers != NULL) {
			EnterCriticalSection(&CriticalSectionForQueue);
			if (primaryQueue->size > 0 && headWorkers != NULL) {
				LeaveCriticalSection(&CriticalSectionForQueue);
				WaitForSingleObject(ReadSemaphore, INFINITE);

				EnterCriticalSection(&CriticalSectionForQueue);
				char* deq = Dequeue(primaryQueue);
				LeaveCriticalSection(&CriticalSectionForQueue);

				ReleaseSemaphore(ReadSemaphore, 1, NULL);
				ReleaseSemaphore(WriteSemaphore, 1, NULL);

				char strlenMessageString[4];
				for (int i = 0; i < 4; i++) {
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
				EnterCriticalSection(&CriticalSectionForOutput);
				EnterCriticalSection(&CriticalSectionForQueue);
				printf("primaryqueue->size = %d \n", primaryQueue->size);
				LeaveCriticalSection(&CriticalSectionForQueue);
				LeaveCriticalSection(&CriticalSectionForOutput);

				++headWorkers->worker->counter;
				//MergeSortWorkerList(&headWorkers);
				MoveToEnd(&headWorkers);
				//Sleep(1000);
				free(deq);
				//MoveToEnd(&headWorkers);

			}
			else {	// posto nije ispunjen bio uslov, moze da se napusti kriticna sekcija
				LeaveCriticalSection(&CriticalSectionForQueue);
			}
			//MergeSortWorkerList(&headWorkers);
			//MoveToEnd(&headWorkers);
		}
	}
	return 0;
}
