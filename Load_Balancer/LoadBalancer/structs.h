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
	SOCKET acceptedSocket;
	int port;
	char* ipAdr;
}Worker;

typedef struct MessageForWorker {
	int length;
	char *message;
}MessageForWorker;

typedef struct Queue {
	int front;
	int rear;
	int size;
	int capacity;
	char* array;
}Queue;