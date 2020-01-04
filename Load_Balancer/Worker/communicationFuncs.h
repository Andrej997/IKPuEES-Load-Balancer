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

		FD_SET recvset;
		FD_ZERO(&recvset);
		FD_SET(*socket, &recvset);

		timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;

		int iResult = select(0, &recvset, &set, NULL, &timeVal);
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

SOCKET SetConnectedSocket(u_short port) {
	SOCKET connectSocket = INVALID_SOCKET;

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(port);
	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}

	SetNonblocking(&connectSocket);

	return connectSocket;
}