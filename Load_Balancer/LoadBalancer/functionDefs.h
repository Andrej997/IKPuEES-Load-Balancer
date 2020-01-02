#pragma once

bool InitializeWindowsSockets();
void InitializeSelect(SOCKET *socket);
void SetNonblocking(SOCKET *socket);