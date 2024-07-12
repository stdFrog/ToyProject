#include "Stack.h"

void CreateStack(Stack** S){
	if((*S) != NULL){return;}

	(*S) = (Stack*)malloc(sizeof(Stack));
	(*S)->First = (*S)->Last = NULL;
	(*S)->Size = 0;
}

Node* GetNode(Stack* S, int idx){
	Node* Current = S->First;
	while(Current != NULL && (--idx) >= 0){
		Current = Current->Next;
	}

	return Current;
}

void Remove(Stack** S, Node* Target){
	if((*S)->First == Target){
		(*S)->First = Target->Next;
	}else{
		Node* Current = (*S)->First;
		while(Current != NULL && Current->Next != Target){
			Current = Current->Next;
		}

		// 찾은 경우 예외 분기
		if(Current != NULL){
			Current->Next = Target->Next;
		}
	}
}

void DestroyStack(Stack* S){
	int cnt = S->Size;
	Node* Current = NULL;

	for(int i=0; i<cnt; i++){
		Current = GetNode(S, 0);
		if(Current != NULL){
			Remove(&S, Current);
			DestroyNode(Current);
		}
	}
}

Node* CreateNode(int Value){
	Node* NewNode = (Node*)malloc(sizeof(Node));

	NewNode->Kick = 0;
	NewNode->Data = Value;
	NewNode->Next = NULL;

	return NewNode;
}

void DestroyNode(Node* Target){
	free(Target);
}

void Push(Stack* S, Node* NewNode){
	if(S->First == NULL){
		S->First = NewNode;
		S->Last = NewNode;
	}else{
		Node* Temp = S->First;
		S->First = NewNode;
		S->First->Next = Temp;
	}

	S->Size++;
}

Node* Pop(Stack* S){
	if(S->First == NULL) {return NULL;}

	Node* Temp = S->First;
	if(S->First == S->Last){
		S->Last = NULL;
	}

	// NULL 포함
	S->First = S->First->Next;
	S->Size--;

	return Temp;
}

int Top(Stack* S){
	return S->First->Data;
}

int IsEmpty(Stack* S){
	return (S->Size == 0);
}
