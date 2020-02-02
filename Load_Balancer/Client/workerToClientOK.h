#pragma once

DWORD WINAPI RecvMessageOK(void *vargp);

DWORD WINAPI RecvMessageOK(void *vargp) {
	SOCKET socket = *(SOCKET*)vargp;
	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	while (true) {
		#pragma region Set
		FD_SET recvset;
		FD_ZERO(&recvset);
		FD_SET(socket, &recvset);
		#pragma endregion

		int iResult = select(0, &recvset, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR) { //error
			printf("Select failed with error: %d\n", WSAGetLastError());
			closesocket(socket);
			break;
		}
		else if (iResult == 0) {
			//printf("I'm waiting...\n");
			continue;
		}
		else if (FD_ISSET(socket, &recvset)) { // recv
			char recvbuf[DEFAULT_BUFLEN];
			iResult = recv(socket, recvbuf, DEFAULT_BUFLEN, 0);
			if (iResult > 0) {
				if (*(char*)recvbuf == 's')
					printf("Message is stored\n");
			}
			else if (iResult == 0) { // connection was closed gracefully
				printf("Connection with server closed.\n");
				closesocket(socket);
				break;
			}
			else { // there was an error during recv
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(socket);
				printf("LB crashed!\nPress enter to exit");
				getchar();
				break;
			}
		}
	}
	return 0;
}