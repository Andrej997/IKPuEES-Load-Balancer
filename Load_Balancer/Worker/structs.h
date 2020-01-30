#pragma once

typedef struct Message {
	int size; // size of current message
	int clientId;
	char *message;
}Message;

typedef struct Node {
	Message *message;
	struct Node *next;
}Node;

