#pragma once

typedef struct Message {
	int size; // size of current message
	int clientId;
	char *message;
}Message;

