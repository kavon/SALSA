/*
 * SwLinkedList.h
 *
 *	Single writer linked list
 *	Uses HP 0 & 1
 *
 */

#ifndef SWLINKEDLIST_H_
#define SWLINKEDLIST_H_

#include "SPChunk.h"
#include "hp/hp.h"


class SwNode {
public:
	SwNode(SPChunk* c);
	SPChunk* chunk;
	int consumerIdx;
	SwNode* next;
};

class SwLinkedList {
public:

	class SwLinkedListIterator {
	public:
		SwLinkedListIterator(SwLinkedList* list);
		//skips removed nodes but doesn't release them
		SwNode* next();
		//reset to head
		void reset(SwLinkedList* list);
	private:
		SwNode* curr;
		HP::HPLocal hpLoc;
	};

	friend class SwLinkedListIterator;

	SwLinkedList();
	virtual ~SwLinkedList();
	void append(SwNode* nodeToAdd);
	SwNode* append(SPChunk* chunkToAdd);


private:
	SwNode* head;
};





#endif /* SWLINKEDLIST_H_ */
