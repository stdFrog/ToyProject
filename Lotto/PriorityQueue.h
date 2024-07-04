#ifndef __PRIORITY_QUEUE_H_
#define __PRIORITY_QUEUE_H_
#include <memory.h>
#include <stdlib.h>
typedef int Type;

typedef struct tag_PQNode{
	Type Priority;
	void* Data;
}PQNode;

typedef struct tag_PriorityQueue{
	PQNode* Nodes;
	int Capacity;
	int UsedSize;
}PriorityQueue;

PriorityQueue* CreateQueue(int Initialize);
void DestroyQueue(PriorityQueue* PQ);
void Enqueue(PriorityQueue* PQ, PQNode NewData);
void Dequeue(PriorityQueue* PQ, PQNode* Root);
int GetParent(int Index);
int GetLeftChild(int Index);
void SwapNodes(PriorityQueue* PQ, int Index1, int Index2);
int IsEmpty(PriorityQueue* PQ);

#endif
