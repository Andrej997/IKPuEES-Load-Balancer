#pragma once

extern NodeW *headWorkers;

void Redistribution() {
	// ako se konektovao prvi klijent nema smisla da se vrsi redistribucija
	if (GetNumOfWorkers(headWorkers) > 1) {
		int numOfMsg = GetAllMessages(headWorkers);
		int numOfWorkers = GetNumOfWorkers(headWorkers);
		int msgPerWorker = numOfMsg / numOfWorkers;
		int msgLeft = numOfMsg - (msgPerWorker * numOfWorkers);
		// dobije niz 
		// ako je pozitivan broj toliko poruka treba da vrati
		// ako je negativan broj toliko poruka treba da primi
		int *arrOfMsg = GiveMe(msgPerWorker, headWorkers);
		DWORD redistributionerId;
		HANDLE redistributioner = CreateThread(NULL,
			0,
			Redistributioner,
			(LPVOID)arrOfMsg,
			0,
			&redistributionerId
		);
	}
	/*else {
		ReleaseSemaphore(ReorganizeSemaphore, 1, NULL);
	}*/
}