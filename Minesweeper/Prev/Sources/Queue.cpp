#include "..\Headers\Queue.h"
#include <stdlib.h>

Queue* CreateQueue(){
	Queue* NewQueue = (Queue*)malloc(sizeof(Queue));
	NewQueue->Front = NewQueue->Rear = NULL;
	NewQueue->Count = 0;

	return NewQueue;
}

void DestroyQueue(Queue* Q){
	while(!IsEmpty(Q)){
		Node* Popped = Dequeue(Q);
		DestroyNode(Popped);
	}

	free(Q);
}

void Enqueue(Queue* Q, Node* N){
	if(Q->Front == NULL){
		Q->Front = N;
		Q->Rear = N;
		Q->Count++;
	}else{
		Q->Rear->Next = N;
		Q->Rear = N;
		Q->Count++;
	}
}

Node* Dequeue(Queue* Q){
	Node* Front = Q->Front;

	if(Q->Front->Next == NULL){
		Q->Front = NULL;
		Q->Rear = NULL;
	}else{
		Q->Front = Q->Front->Next;
	}

	Q->Count--;
	return Front;
}

Node* CreateNode(ElementType X, ElementType Y){
	Node* NewNode = (Node*)malloc(sizeof(Node));
	NewNode->x = X;
	NewNode->y = Y;
	NewNode->Next = NULL;

	return NewNode;
}

void DestroyNode(Node* N){
	free(N);
}

int IsEmpty(Queue* Q){
	return (Q->Front == NULL);
}
