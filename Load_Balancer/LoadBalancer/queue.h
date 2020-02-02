#pragma once

DWORD WINAPI WorkWithQueue(void *vargp);

DWORD WINAPI WorkWithQueue(void *vargp) {
	while (true) {
		WaitForSingleObject(CreateQueueSemaphore, INFINITE);
		//tempQueue = CreateQueue(primaryQueue->capacity * 0.4); //kreriamo pomocni sa 40% kapaciteta primarnog
		tempQueue = CreateQueue(primaryQueue->capacity); //kreriamo pomocni sa 40% kapaciteta primarnog
		secondaryQueue = CreateQueue(primaryQueue->capacity * 2); //kreiramo novi buffer koji je od starog veci duplo

		ReleaseSemaphore(CreatedQueueSemaphore, 1, NULL);
	}
	return 0;
}
