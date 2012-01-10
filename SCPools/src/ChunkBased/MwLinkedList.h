/*
 * LinkedList.h
 *
 *  Created on: Dec 29, 2011
 */

#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include "Chunk.h"
#include "atomicMarkedReference.h"

class Node {
public:
	Node(Chunk* c);
	Chunk* chunk;
	int consumerIdx;
	markable_ref next;
};

class MwLinkedList {
public:
	MwLinkedList();
	virtual ~MwLinkedList();

	Node* append(Chunk* c);
	Node* append(Chunk* c, int consumerIdx);
	Node* get(int idx);
	bool remove(Node* toRemove);
private:
	Node* head;
};

#endif /* LINKEDLIST_H_ */
