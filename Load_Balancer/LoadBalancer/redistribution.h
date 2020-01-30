#pragma once
DWORD WINAPI Redistributioner(void *vargp);

DWORD WINAPI Redistributioner(void *vargp) {
	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;
	while (true) {
		FD_SET set;
		FD_ZERO(&set);

		WaitForSingleObject(ReorganizeSemaphoreStart, INFINITE); //cekamo na semaforu dok ne dobijemo signal da pocnemo reorganizaciju

		InitializeCriticalSection(&CriticalSectionForReorQueue);
		reorQueue = CreateQueue(CAPACITY_REOR_QUEUE);

		int numOfWorkers = GetNumOfWorkers(headWorkers);
		int numOfMsg = GetAllMessages(headWorkers);
		int msgPerWorker = numOfMsg / numOfWorkers;
		int *arrOfMsg = GiveMe(msgPerWorker, headWorkers);
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
			int iResult = send(temp->worker->acceptedSocket, msg, 6, 0);
			if (iResult > 0) {
				EnterCriticalSection(&CriticalSectionForOutput);
				//printf("Redistributioner send: %c%c%c%c on worker with accepted socket: %d\n", msg[0], msg[1], msg[2], msg[3], temp->worker->acceptedSocket);
				LeaveCriticalSection(&CriticalSectionForOutput);
			}
			if (iResult == SOCKET_ERROR) {
				EnterCriticalSection(&CriticalSectionForOutput);
				printf("Redistributioner messsage: send failed with error: %d\n", WSAGetLastError());
				LeaveCriticalSection(&CriticalSectionForOutput);
				//closesocket(headWorkers->worker->acceptedSocket);
				//WSACleanup();
				return 1;
			}
			free(msg);
			temp = temp->next;
			++i;
		}
		free(arrOfMsg);
		Sleep(1000);
	}
	return 0;
}
