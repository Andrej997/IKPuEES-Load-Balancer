#pragma once

bool InitializeWindowsSockets();
void InitializeSelect(SOCKET *socket);
SOCKET SetConnectedSocket(PCSTR port);
char* GenerateMessage();