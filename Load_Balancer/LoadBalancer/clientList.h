#pragma once

typedef struct Node {
	Client *client;
	struct Node *next;
}Node;

void AddAtEnd(Node **head, Client *new_data) {
	Node* new_node = (struct Node*) malloc(sizeof(struct Node));

	new_node->client = (Client*)malloc(sizeof(Client));
	new_node->client = new_data;
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

void FreeList(Node *head) {
	Node *temp;
	while (head != NULL) {
		temp = head;
		CloseHandle(temp->client->thread);
		head = head->next;
		free(temp);
	}
	return;
}