#pragma once

typedef struct Node {
	Message *message;
	struct Node *next;
}Node;

void AddAtEnd(Node **head, Message *message) {
	Node* new_node = (struct Node*) malloc(sizeof(struct Node));

	new_node->message = message;
	new_node->next = NULL;

	if (*head == NULL) {
		*head = new_node;
		return;
	}

	Node *last = *head;
	while (last->next != NULL)
		last = last->next;
	last->next = new_node;

	return;
}

char* Convert(Message message) {
	size_t len = 0;
	len = snprintf(NULL, len, "%d,%d,%s", message.clientId, message.size, message.message);

	char *apstr = (char*)calloc(1, sizeof *apstr * len + 1);
	if (!apstr) {
		fprintf(stderr, "%s() error: virtual memory allocation failed.\n", __func__);
	}

	if (snprintf(apstr, len + 1, "%d,%d,%s", message.clientId, message.size, message.message) > len + 1)
	{
		fprintf(stderr, "%s() error: snprintf returned truncated result.\n", __func__);
		return NULL;
	}

	return apstr;
}

char* ConvertToString(Node *head ,int numOfMsg) {
	char *retVal = (char*)malloc(numOfMsg * sizeof(Message));
	Node *temp = head;

	int i = 0;
	while (i < numOfMsg) {
		memcpy(retVal + i * sizeof(Message), Convert(*temp->message), sizeof(Message));
		temp = temp->next;
		++i;
	}
	return retVal;
}

void FreeMessages(Node *head, int numOfMsg) {
	Node *temp;
	int i = 0;
	while (i < numOfMsg) {
		temp = head;
		head = head->next;
		free(temp);
		++i;
	}
	return;
}