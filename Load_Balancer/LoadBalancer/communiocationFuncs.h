#pragma once

unsigned long nonBlockingMode = 1;
//extern int index;
//extern Node* headClients;

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

void InitializeSelect(SOCKET *socket) {
	while (true) {
		FD_SET set;
		FD_ZERO(&set);
		FD_SET(*socket, &set);

		timeval timeVal;
		timeVal.tv_sec = 2;
		timeVal.tv_usec = 0;

		int iResult = select(0, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) {
			//error
		}
		else if (iResult == 0) {
			printf("I'm waiting...\n");
			continue;
		}
		else if (iResult > 0) {
			break;
		}
	}
}

void SetNonblocking(SOCKET *socket) {
	int iResult = ioctlsocket(*socket, FIONBIO, &nonBlockingMode);
	if (iResult == SOCKET_ERROR) {
		printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
	}
}

//DWORD WINAPI myThreadFun(void *vargp) {
//	SOCKET socket = *(SOCKET*)vargp;
//	char recvbuf[DEFAULT_BUFLEN];
//	//unsigned long nonBlockingMode = 1;
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
//			iResult = ioctlsocket(socket, FIONBIO, &nonBlockingMode);
//			if (iResult == SOCKET_ERROR) {
//				printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
//			}
//			iResult = recv(socket, recvbuf, DEFAULT_BUFLEN, 0);
//			if (iResult > 0)
//			{
//				for (int i = 0; i < index; i++)
//				{
//					if (socket == headClients[i].client->acceptedSocket) {
//						printf("Thread id = %d. Client id: %d, port: %d, IP address: %s. Message: %s\n", GetCurrentThreadId(), headClients[i].client->id, headClients[i].client->port, headClients[i].client->ipAdr, recvbuf);
//						break;
//					}
//				}
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