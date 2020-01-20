#pragma once

extern NodeW *headWorkers;

void Redistribution() {
	int numOfMsg = GetAllMessages(headWorkers);
	int numOfWorkers = GetNumOfWorkers(headWorkers);
	int msgPerWorker = numOfMsg / numOfWorkers;
	int msgLeft = numOfMsg - (msgPerWorker * numOfWorkers);
	// dobije niz 
	// ako je pozitivan broj toliko poruka treba da vrati
	// ako je negativan broj toliko poruka treba da primi
	int *arrOfMsg = GiveMe(msgPerWorker, headWorkers);
}