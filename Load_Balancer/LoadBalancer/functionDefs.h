#pragma once

bool InitializeWindowsSockets();
void InitializeSelect(SOCKET *socket);
void SetNonblocking(SOCKET *socket);
SOCKET SetListenSocket(PCSTR port);
DWORD WINAPI myThreadFun(void *vargp);
bool PrimaryToSecondary();
void DestroyQueue(Queue*);
bool Enqueue(Queue* queue, char* message, int length);
char* Dequeue(Queue* queue);