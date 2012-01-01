/*
 * hp.c
 *  A straightforward hazard pointer implementation.
 *
 *  Implemented as described in:
 *  "Hazard Pointers: Safe Memory Reclamation for Lock-Free Objects"
 *  (http://www.research.ibm.com/people/m/michael/ieeetpds-2004.pdf)
 *
 *      Author: Elad Gidron
 */

#include "hp.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
// AUX ///
void SwapStacks(Stack* s1, Stack* s2);
void scan(HPLocal localData);
void prepareForReuse(void* data);
///////

/*** Data structures ****/
//Data structures
//A struct used to initialize the hazard pointers system
//generated by initHPData()
struct HPData_t{
	int HP_COUNT;  //The number of hazard pointers per thread
	int THREAD_COUNT; //Total number of threads
	int REC_COUNT; //Number of elements that can be free'd before scan() is called.
};


//A record that holds the hazard pointers of each thread.
struct HPRecord_t{
	struct HPRecord_t* next; //Next record in the list.
	void* hp[]; //The hazard pointers
};

//Hold local data structures needed for each thread.
struct HPLocal_t{
	HPRecord* HPRecHead; //head of the HPRecord list
	HPRecord* localRecord; //The current thread HPRecord
	Stack rlist; //List of retired nodes that were not free'd
	Stack temp; //Stack for internal usage
	Set plist; //Set for internal usage
	HPData hpData; //The HP parameters.
};

/*** External functions ***/


HPData initHPData(int hpCountPerThread, int ThreadCount, int recCount) {
	HPData res = (HPData)malloc(sizeof(struct HPData_t));
	assert(res != NULL);
	res->HP_COUNT = hpCountPerThread;
	res->THREAD_COUNT = ThreadCount;
	//res->REC_COUNT = hpCountPerThread*ThreadCount;
	res->REC_COUNT = recCount;
	return res;
}

/*
 * Adds a thread to the HPReord list.
 * Returns an HPLocal var that the thread will hold locally,
 * so it will be used later when accessing hazard pointers.
 * Head - the head of the HPRecord list, or NULL if this is the first element
 * hpData - An HPData object created by initHPData
 */
HPLocal threadRegister(HPRecord* head, HPData hpData) {
	int i;
	//init record
	HPRecord* record = (HPRecord*) malloc(sizeof(HPRecord)+(sizeof(void*) * hpData->HP_COUNT));
	assert(record != NULL);
	record->next = NULL;
	for (i = 0; i < hpData->HP_COUNT; i++) {
		record->hp[i] = NULL;
	}

	//init thread local struct
	HPLocal res = (HPLocal) malloc(sizeof(HPLocalT));
	assert(res != NULL);

	res->localRecord = record;

	if (head == NULL)
		res->HPRecHead = record;
	else
		res->HPRecHead = head;

	int stackSize=  hpData->THREAD_COUNT*hpData->HP_COUNT > hpData->REC_COUNT ? hpData->THREAD_COUNT*hpData->HP_COUNT : hpData->REC_COUNT;

	res->rlist = stackInit(stackSize);
	res->temp = stackInit(stackSize);
	res->plist = setInit(stackSize);
	res->hpData = hpData;
	if (head != NULL) {
		//add record to list
		HPRecord* last = head;
		while (last->next != NULL)
			last = last->next;
		last->next = record;
	}

	return res;
}

// Marks that the node needs to be freed and calls scan if needed.
void retireNode(void* addr, ReclaimationFunc reclaimationFunc, HPLocal localData) {
	stackPush(localData->rlist, addr, reclaimationFunc);
	if (stackCount(localData->rlist) >= localData->hpData->REC_COUNT) {
		scan(localData);
	}
}

void setHP(int idx, void* addr, HPLocal localData) {
	if (idx >= localData->hpData->HP_COUNT) {
		printf("IDX: %d max: %d \n",idx, localData->hpData->HP_COUNT );
		assert(0);
	}
	localData->localRecord->hp[idx] = addr;
}

HPRecord* getHPListHead(HPLocal localData) {
	return localData->HPRecHead;
}

// Goes over the list of nodes that needs to be freed
// and frees them if no HP points at them.
void scan(HPLocal localData) {
	int i;
	//Stage 1: scan HP list and insert non-null values to plist
	Set plist = localData->plist;
	setReset(plist);
	HPRecord* curr = localData->HPRecHead;
	while (curr != NULL) {
		for (i = 0; i < localData->hpData->HP_COUNT; i++) {
			if (curr->hp[i] != NULL) {
				setAdd(plist, (int) (curr->hp[i]));
			}
		}
		curr = curr->next;
	}
	//Stage 2: search plist
	//uses two stacks instead of allocating a new one each time scan() is called
	SwapStacks(&localData->rlist, &localData->temp);
	stackReset(localData->rlist);
	ReclaimationData* recData = stackPop(localData->temp);
	while (recData != NULL) {
		if (setContains(plist, (int) recData->address)) {
			stackPush(localData->rlist, recData->address, recData->reclaimationFunc);
		} else {
			recData->reclaimationFunc(recData->address);

		}
		recData = stackPop(localData->temp);
	}
	setReset(plist);
}

//void prepareForReuse(void* data) {
//	//simple implementation - can be implemented with reuse
//	free(data);
//}

void SwapStacks(Stack* s1, Stack* s2) {
	Stack temp = *s1;
	*s1 = *s2;
	*s2 = temp;
}

