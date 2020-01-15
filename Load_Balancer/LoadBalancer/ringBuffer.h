#pragma once
#include <stdlib.h>
#include <string.h>



extern Queue* primaryQueue;
extern Queue* tempQueue;
extern Queue* secondaryQueue;

//bool PrimaryToSecondary();
//void DestroyQueue(Queue*);



Queue* CreateQueue(int cap) {
	Queue* queue = (Queue*)malloc(sizeof(Queue));
	queue->size = 0;
	queue->capacity = cap;
	queue->rear = 0;
	queue->front = 0;
	queue->array = (char*)malloc(sizeof(char) * cap);
	return queue;
}

bool IsEmpty(Queue* queue) {
	if (queue->size == 0)
		return true;
	return false;
}

bool IsFull(Queue* queue, int strlenMessage) {
	//if ((queue->size + strlenMessage + sizeof(int)) >= (queue->capacity)) {
	if ((queue->size + strlenMessage + sizeof(int)) >= (queue->capacity * 0.7)) { //upisom poruke bi popunjenost presla 70%, ne dozvoljavamo!

		return true;
	}
	return false;
}

bool Enqueue(Queue* queue, char* message, int lengthMess) {
	bool success = false;
	//int strlenMessage = strlen(message);
	int strlenMessage = lengthMess;
	//printf("-------Enqueue-------Start\n");
	if (!IsFull(queue, strlenMessage)) {
		for (int i = 0; i < 4; i++)
		{
			queue->array[queue->rear] = ((char*)(&strlenMessage))[i];
			queue->rear = (queue->rear + 1) % queue->capacity;
		}
		for (int i = 0; i < strlenMessage; i++)
		{
			queue->array[queue->rear] = message[i];
			queue->rear = (queue->rear + 1) % queue->capacity;
		}
		queue->size += (strlenMessage + sizeof(int));
		success = true;
	}
	else {
		secondaryQueue = CreateQueue(queue->capacity * 2); //kreiramo novi buffer koji je od starog veci duplo
		//mislim da bi ovaj deo kreiranja sekundarnog i prepisivanje u njega trebali da izbacimo u novi thread recimo
		//kako bi se moglo nastaviti sa prijemom poruka i upisivanjem u privremeni
		//PrimaryToSecondary();
		tempQueue = CreateQueue(queue->capacity);
		//tempQueue = CreateQueue(queue->capacity * 0.4); //kreiramo privremeni queue koji je 40% kapaciteta glavnog 
		//i upisujemo pristigle poruke

		if (!IsFull(tempQueue, strlenMessage)) {
			for (int i = 0; i < 4; i++)
			{
				tempQueue->array[tempQueue->rear] = ((char*)(&strlenMessage))[i];
				tempQueue->rear = (tempQueue->rear + 1) % tempQueue->capacity;
			}
			for (int i = 0; i < strlenMessage; i++)
			{
				tempQueue->array[tempQueue->rear] = message[i];
				tempQueue->rear = (tempQueue->rear + 1) % tempQueue->capacity;
			}
			tempQueue->size += (strlenMessage + sizeof(int));
			success = true;
		}
		else { //napunio nam se i privremeni, dodati logiku da pauziramo klijente ili tako nesto
			printf("Temp queue is full!!!\n");
		}
		PrimaryToSecondary();
	}
	//printf("-------Enqueue-------End\n");

	return success;
}
char* Dequeue(Queue* queue) {
	char* message;
	//printf("-------Dequeue-------Start\n");
	if (!IsEmpty(queue)) {
		char strlenMessageString[4];

		for (int i = 0; i < 4; i++)
		{
			strlenMessageString[i] = queue->array[queue->front];
			queue->array[queue->front] = 'x';
			queue->front = (queue->front + 1) % queue->capacity;
		}
		int strlenMessageInt = *(int*)strlenMessageString;

		message = (char*)malloc((sizeof(char) * strlenMessageInt) + 4);

		//strcpy(message, strlenMessageString);

		for (int i = 0; i < 4; i++)
		{
			message[i] = strlenMessageString[i];
		}

		for (int i = 0; i < strlenMessageInt; i++)
		{
			message[i + 4] = queue->array[queue->front];
			queue->array[queue->front] = 'x';
			queue->front = (queue->front + 1) % queue->capacity;
		}
		queue->size -= strlenMessageInt + sizeof(int);
		//printf("-------Dequeue-------End\n");

		return message;
	}

	return NULL;
}

bool PrimaryToSecondary() {

	int secondaryIndex = 0;
	/*for (int i = primaryQueue->front; i < primaryQueue->size; i = (i + 1) % primaryQueue->capacity)
	{
		secondaryQueue->array[secondaryIndex++] = primaryQueue->array[i];
	}*/
	/*for (int i = 0; i < primaryQueue->size; i = (i + 1) % primaryQueue->capacity)
	{
		secondaryQueue->array[secondaryIndex++] = primaryQueue->array[primaryQueue->front++];
	}*/
	for (int i = 0; i < primaryQueue->size; i++)
	{
		secondaryQueue->array[secondaryIndex++] = primaryQueue->array[primaryQueue->front];
		primaryQueue->front = (primaryQueue->front + 1) % primaryQueue->capacity;
	}
	for (int i = 0; i < tempQueue->size; i++)
	{
		secondaryQueue->array[secondaryIndex++] = tempQueue->array[i];
	}

	secondaryQueue->front = 0;
	secondaryQueue->size = primaryQueue->size + tempQueue->size;
	secondaryQueue->rear = secondaryQueue->size;

	DestroyQueue(primaryQueue);
	DestroyQueue(tempQueue);

	primaryQueue = secondaryQueue;
	//primaryQueue->array = secondaryQueue->array;

	//DestroyQueue(secondaryQueue);
	secondaryQueue = NULL;

	return true;
}

void DestroyQueue(Queue* queue) {

	free(queue->array);
	free(queue);
}