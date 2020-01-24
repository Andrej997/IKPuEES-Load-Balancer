#pragma once

unsigned long nonBlockingMode = 1;

bool InitializeWindowsSockets();
void InitializeSelect(SOCKET *socket);
void SetNonblocking(SOCKET *socket);
SOCKET SetConnectedSocket(u_short port);