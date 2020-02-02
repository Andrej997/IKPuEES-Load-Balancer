#pragma once

void DeleteAllThreads(Node *headC, NodeW *headW);
void CloseMainThread(HANDLE thread);

void DeleteAllThreads(Node *headC, NodeW *headW) {
	while (headW != NULL && headC != NULL) {
		if (headW != NULL)
			CloseHandle(headW->worker->thread);
		headW = headW->next;

		if (headC != NULL)
			CloseHandle(headC->client->thread);
		headC = headC->next;
	}
	return;
}

void CloseMainThread(HANDLE thread) {
	CloseHandle(thread);
	return;
}