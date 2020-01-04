#pragma once

typedef struct Client {
	int id;
	HANDLE thread;
	SOCKET acceptedSocket;
	char* ipAdr;
	int port;
}Client;

typedef struct Worker {
	int id;
	int counter;
	HANDLE thread;
	SOCKET *socket;
}Worker;