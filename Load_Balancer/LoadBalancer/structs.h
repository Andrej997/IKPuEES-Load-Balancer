#pragma once

typedef struct Client {
	int id;
	//dodati thread kada ovako normalno profunkcionise xD
	SOCKET socket; //nemam pojma kako se radi sa ovim, to je samo neki broj
	SOCKET acceptedSocket;
	char* ipAdr;
	int port;
}Client;

typedef struct Worker {
	int id;
	int counter;
	//HANDLE thread;
	SOCKET *socket;
}Worker;