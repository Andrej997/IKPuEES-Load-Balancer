#pragma once

typedef struct Node {
	char *message;
	struct Node *next;
}Node;


void AddAtEnd(Node **head, char *new_data /*Message->message*/, size_t data_size/*Message->size*/) {
	Node* new_node = (struct Node*) malloc(sizeof(struct Node));

	new_node->message = (char*)malloc(sizeof(data_size));
	for (int i = 21, j = 0; i < data_size; ++i, ++j)
		*(char*)(new_node->message + j) = *(char*)(new_data + i);
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