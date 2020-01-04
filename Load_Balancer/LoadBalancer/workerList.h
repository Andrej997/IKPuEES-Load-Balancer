#pragma once

typedef struct NodeW {
	Worker *worker;
	struct NodeW *next;
}NodeW;

void AddAtEnd(NodeW **head, Worker *new_data) {
	NodeW* new_node = (struct NodeW*) malloc(sizeof(struct NodeW));

	new_node->worker = (Worker*)malloc(sizeof(Worker));
	new_node->worker = new_data;
	new_node->next = NULL;

	if (*head == NULL) {
		*head = new_node;
		return;
	}

	NodeW *last = *head;
	while (last->next != NULL)
		last = last->next;
	last->next = new_node;
	return;
}

void FreeList(NodeW *head) {
	NodeW *temp;
	while (head != NULL) {
		temp = head;
		CloseHandle(temp->worker->thread);
		head = head->next;
		free(temp);
	}
	return;
}