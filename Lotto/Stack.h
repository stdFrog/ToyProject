#ifndef __STACK_H_
#define __STACK_H_
#include <stdlib.h>

typedef struct tag_Node{
	struct tag_Node* Next;
	int Data;
	int Kick;
}Node;

typedef struct tag_Stack{
	struct tag_Node* First;
	struct tag_Node* Last;
	int Size;
}Stack;

void CreateStack(Stack** S);
Node* GetNode(Stack* S, int idx);
void Remove(Stack** S, Node* Target);
void DestroyStack(Stack* S);

Node* CreateNode(int Value);
void DestroyNode(Node* Target);

void Push(Stack* S, Node* NewNode);
Node* Pop(Stack* S);

int Top(Stack* S);
int IsEmpty(Stack* S);

#endif
