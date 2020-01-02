#pragma once

unsigned long nonBlockingMode = 1;

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