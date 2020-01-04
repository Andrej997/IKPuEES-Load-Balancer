#pragma once

bool InitializeWindowsSockets();
void InitializeSelect(SOCKET *socket);
void SetNonblocking(SOCKET *socket);
SOCKET SetConnectedSocket(u_short port);
DWORD WINAPI myThreadFun(void *vargp);
