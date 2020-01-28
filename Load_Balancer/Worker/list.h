#pragma once

typedef struct Node {
	Message *message;
	struct Node *next;
}Node;

extern Node *headMessages;

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

void FreeMessages(Node **head, int numOfMsg) {
	Node *temp;
	int i = 0;
	while (i < numOfMsg) {
		temp = headMessages;
		headMessages = headMessages->next;
		free(temp);
		++i;
	}
	return;
}

char* ConvertToString(Node *head, int numOfMsg, int *a) {
	Node *temp = head;
	int len = 0;
	int len1 = 0;
	int sum = 0;

	for (int i = 0; i < numOfMsg; i++)
	{
		len1 += temp->message->size;
		temp = temp->next;
	}
	*a = len1 + 5 + numOfMsg * 4;
	char *retVal = (char*)malloc(len1 + 4 * numOfMsg + 1);
	retVal[0] = 'r';
	union {
		int num;
		char buf[4];
	}MyU;
	MyU.num = numOfMsg;
	retVal[1] = MyU.buf[0];
	retVal[2] = MyU.buf[1];
	retVal[3] = MyU.buf[2];
	retVal[4] = MyU.buf[3];
	temp = head;

	memcpy(retVal + 1 + 4, temp->message->message, temp->message->size + 4);
	int i = 1;
	int cnt = temp->message->size + 4 + 1 + 4;
	while (i < numOfMsg) {
		temp = temp->next;
		memcpy(retVal + cnt, temp->message->message, temp->message->size + 4);
		cnt += temp->message->size + 4;
		++i;
	}
	FreeMessages(&head, numOfMsg);
	return retVal;
}