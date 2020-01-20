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

void deleteNodeW(NodeW **head_ref, int key)
{
	NodeW* temp = *head_ref, *prev = NULL;

	if (temp != NULL && temp->worker->acceptedSocket == key) {
		*head_ref = temp->next;
		free(temp);
		return;
	}

	while (temp != NULL && temp->worker->acceptedSocket != key) {
		prev = temp;
		temp = temp->next;
	}

	if (temp == NULL)
		return;

	prev->next = temp->next;

	free(temp);
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

NodeW* SortedMerge(NodeW* a, NodeW* b){
	NodeW* result = NULL;

	// startni slucajevi
	if (a == NULL)
		return (b);
	else if (b == NULL)
		return (a);

	if (a->worker->counter <= b->worker->counter) {
		result = a;
		result->next = SortedMerge(a->next, b);
	}
	else {
		result = b;
		result->next = SortedMerge(a, b->next);
	}
	return (result);
}

void FrontBackSplit(NodeW* source, NodeW** frontRef, NodeW** backRef){
	NodeW* fast;
	NodeW* slow;
	slow = source;
	fast = source->next;

	/* Advance 'fast' two nodes, and advance 'slow' one node */
	while (fast != NULL) {
		fast = fast->next;
		if (fast != NULL) {
			slow = slow->next;
			fast = fast->next;
		}
	}

	/* 'slow' is before the midpoint in the list, so split it in two
	at that point. */
	*frontRef = source;
	*backRef = slow->next;
	slow->next = NULL;
}

void MergeSortWorkerList(NodeW **headRef) {
	NodeW* head = *headRef;
	NodeW* a;
	NodeW* b;

	if ((head == NULL) || (head->next == NULL)) {
		return;
	}

	// podeli listu na dva dela
	FrontBackSplit(head, &a, &b);

	// rekurzivno sortiraj podliste
	MergeSortWorkerList(&a);
	MergeSortWorkerList(&b);

	// spoji dve podliste
	*headRef = SortedMerge(a, b); 
}

int GetAllMessages(NodeW *head) {
	int retVal = 0;
	NodeW *temp = head;
	while (temp != NULL) {
		retVal += temp->worker->counter;
		temp = temp->next;
	}
	return retVal;
}

int GetNumOfWorkers(NodeW *head) {
	int retVal = 0;
	NodeW *temp = head;
	while (temp != NULL) {
		++retVal;
		temp = temp->next;
	}
	return retVal;
}

/*
	Ulazni param je broj poruka koje treba da sadrzi svaki worker.
	Izlazni je niz brojava poruka koje Worker treba da posalje.
*/
int* GiveMe(int numOfMsgPerWorker, NodeW *head) {
	int brojWorkera = GetNumOfWorkers(head);
	int *retArr = (int*)malloc(brojWorkera * sizeof(int));
	NodeW *temp = head;
	int i = 0;
	while (temp != NULL) {
		int tempMsgPerWorker = temp->worker->counter - numOfMsgPerWorker;
		*(retArr + i) = tempMsgPerWorker;
		++i;
		temp = temp->next;
	}
	return retArr;
}