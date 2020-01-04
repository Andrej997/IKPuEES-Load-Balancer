#pragma once

bool InitializeWindowsSockets();
void InitializeSelect(SOCKET *socket);
void SetNonblocking(SOCKET *socket);
SOCKET SetListenSocket(PCSTR port);
DWORD WINAPI myThreadFun(void *vargp);