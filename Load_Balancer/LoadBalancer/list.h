#pragma once

typedef struct Node {
	void *data;
	struct Node *next;
}Node;

void AddAtBeg(Node **head, void *new_data, size_t data_size) {
	Node *new_node = (Node*)malloc(sizeof(Node));
	new_node->data = malloc(data_size);
	new_node->next = (*head);

	for (int i = 0; i < data_size; ++i) 
		*(char*)(new_node->data + i) = *(char*)(new_data + i);
	
	(*head) = new_node;
}

void AddAtEnd(Node **head, void *new_data, size_t data_size) {
	Node* new_node = (struct Node*) malloc(sizeof(struct Node));
	
	new_node->data = malloc(sizeof(data_size));
	for (int i = 0; i < data_size; ++i)
		*(char*)(new_node->data + i) = *(char*)(new_data + i);
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