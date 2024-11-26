#ifndef __QUEUE_H_
#define __QUEUE_H_

typedef long int ElementType;

typedef struct tag_Node{
	ElementType x;
	ElementType y;

	struct tag_Node* Next;
}Node;

typedef struct tag_LinkedQueue{
	Node* Front;
	Node* Rear;
	unsigned int Count;
}Queue;

Queue* CreateQueue();
void DestroyQueue(Queue*);
void Enqueue(Queue*, Node*);
Node* Dequeue(Queue*);

Node* CreateNode(ElementType, ElementType);
void DestroyNode(Node*);

int IsEmpty(Queue*);

#endif
