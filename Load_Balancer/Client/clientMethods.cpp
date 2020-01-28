#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>

#include "clientMethods.h"

char* GenerateMessage() {

	srand(time(NULL));
	int random = (rand() % 20) + 10;

	char* message = (char*)malloc(sizeof(char) * (random + 1));

	memset(message, 'a', random * sizeof(char));
	memset(message + random, NULL, 1);

	return message;
}

char* Generate10BMsg() {
	int bytes = 10;
	char* message = (char*)malloc(sizeof(char) * (bytes + 1));

	memset(message, 'a', bytes * sizeof(char));
	memset(message + bytes, NULL, 1);

	return message;
}
