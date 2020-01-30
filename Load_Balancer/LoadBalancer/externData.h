#pragma once

#pragma region DefaultValue
#define CAPACITY_REOR_QUEUE 500
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5059"
#define DEFAULT_PORT_WORKER "27016"
#define MAX_COUNT_SEMAPHORE 1000
#define INITIAL_CAPACITY_BUFFER 100
#pragma endregion

#pragma region ExternVariables
extern Node *headClients;
extern NodeW *headWorkers;
extern Queue* primaryQueue;
extern Queue* tempQueue;
extern Queue* secondaryQueue;
extern Queue* reorQueue;
extern CRITICAL_SECTION
	CriticalSectionForQueue,
	CriticalSectionForOutput,
	CriticalSectionForReorQueue;
extern HANDLE
	WriteSemaphore,
	WriteSemaphoreTemp,
	ReadSemaphore,
	CreateQueueSemaphore,
	CreatedQueueSemaphore;
extern HANDLE
	ReorganizeSemaphoreStart,
	ReorganizeSemaphoreEnd,
	TrueSemaphore;
#pragma endregion


