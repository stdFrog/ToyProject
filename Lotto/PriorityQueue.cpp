#include "PriorityQueue.h"

PriorityQueue* CreateQueue(int Initialize){

	PriorityQueue* NewPQ = (PriorityQueue*)malloc(sizeof(PriorityQueue));
	NewPQ->Capacity = Initialize;
	NewPQ->UsedSize = 0;
	NewPQ->Nodes = (PQNode*)malloc(sizeof(PQNode) * NewPQ->Capacity);

	return NewPQ;
}

void DestroyQueue(PriorityQueue* PQ){
	free(PQ->Nodes);
	free(PQ);
}

void Enqueue(PriorityQueue* PQ, PQNode NewData){
	int CurrentPosition = PQ->UsedSize;
	int ParentPosition = GetParent(CurrentPosition);

	if(PQ->UsedSize == PQ->Capacity){
		if(PQ->Capacity == 0){
			PQ->Capacity =1;
		}
		PQ->Capacity *= 2;
		PQ->Nodes = (PQNode*)realloc(PQ->Nodes, sizeof(PQNode) * PQ->Capacity);
	}

	PQ->Nodes[CurrentPosition] = NewData;

	while(CurrentPosition > 0 && PQ->Nodes[CurrentPosition].Priority < PQ->Nodes[ParentPosition].Priority)
	{
		SwapNodes(PQ, CurrentPosition, ParentPosition);

		CurrentPosition = ParentPosition;
		ParentPosition = GetParent(CurrentPosition);
	}
	PQ->UsedSize++;
}

void Dequeue(PriorityQueue* PQ, PQNode* Root){
	int ParentPosition = 0;
	int LeftPosition = 0;
	int RightPosition = 0;

	// MinNode라는 노드 하나 생성해서 넘김
	// 루트 노드의 값을 Root(MinNode) 노드에 복사함
	// 하나의 데이터만 받으며 데이터 복사 후 PQ의 루트 노드를 0으로 채움
	memcpy(Root, &PQ->Nodes[0], sizeof(PQNode));
	memset(&PQ->Nodes[0], 0, sizeof(PQNode));

	// UsedSize는 언제나 +1이기 때문에 (스택과 동일)
	// 트리 중 가장 깊은 곳 최우측 노드를 불러옴 (=UsedSize--)
	// 현재 루트 노드의 값은 0이며, 루트 노드의 값을 최우측노드의 값으로 바꿈
	PQ->UsedSize--;
	SwapNodes(PQ, 0, PQ->UsedSize);

	// 루트노드기 때문에 0을 그냥 넘김
	LeftPosition = GetLeftChild(0);
	RightPosition = LeftPosition+1;

	// 이중 루프로 감싸도 되고 while문으로 계속 돌려도 됨 중간에
	// if문으로 탈출시킴
	// 다 만든 후에야 대충 감이 잡히는거고
	// 처음 만들 땐 종료 조건이 뭔지도 잘 모름
	while(1){
		int SelectedChild = 0;

		if(LeftPosition >= PQ->UsedSize){break;}

		if(RightPosition >= PQ->UsedSize){
			SelectedChild = LeftPosition;
		}else{
			if(PQ->Nodes[LeftPosition].Priority > PQ->Nodes[RightPosition].Priority){
				SelectedChild = RightPosition;
			}else{
				SelectedChild = LeftPosition;
			}
		}

		if(PQ->Nodes[SelectedChild].Priority < PQ->Nodes[ParentPosition].Priority){
			SwapNodes(PQ, ParentPosition, SelectedChild);
			ParentPosition = SelectedChild;
		}else{
			break;
		}
		LeftPosition = GetLeftChild(ParentPosition);
		RightPosition = LeftPosition+1;
	}
	if(PQ->UsedSize < (PQ->Capacity /2)){
		PQ->Capacity /= 2;
		PQ->Nodes = (PQNode*)realloc(PQ->Nodes, sizeof(PQNode) * PQ->Capacity);
	}
}

int GetParent(int Index){
	return (int)((Index-1)/2);
}

int GetLeftChild(int Index){
	return (int)((Index*2)+1);
}

void SwapNodes(PriorityQueue* PQ, int Index1, int Index2){
	int CopySize = sizeof(PQNode);
	PQNode* Temp = (PQNode*)malloc(sizeof(CopySize));

	memcpy(Temp, &PQ->Nodes[Index1], CopySize);
	memcpy(&PQ->Nodes[Index1], &PQ->Nodes[Index2], CopySize);
	memcpy(&PQ->Nodes[Index2], Temp, CopySize);

	free(Temp);
}

int IsEmpty(PriorityQueue* PQ){
	return (PQ->UsedSize==0);
}

