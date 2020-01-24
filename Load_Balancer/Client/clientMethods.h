#pragma once

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