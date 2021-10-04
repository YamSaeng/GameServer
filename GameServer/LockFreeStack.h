#pragma once

#include"ObjectPoolFreeList.h"

template <class DATA>
class CLockFreeStack
{
private:
	struct st_Node
	{
		DATA Data;
		st_Node* NextNode;
	};

	struct st_CheckNode
	{
		st_Node* TopNode;
		LONG64 NodeCheckValue;
	};

	LONG64 Count;
	st_CheckNode* _TopNode;

	LONG64 _LockFreeCheckCount;
	CObjectPoolFreeList<st_Node>* _ObjectPoolFreeList;
public:
	CLockFreeStack();
	~CLockFreeStack();

	LONG _LockFreeCount;


	void Push(DATA Data);
	bool Pop(DATA* Data);
};

template<class DATA>
CLockFreeStack<DATA>::CLockFreeStack()
{
	_ObjectPoolFreeList = new CObjectPoolFreeList<st_Node>();
	_TopNode = (st_CheckNode*)_aligned_malloc(sizeof(st_CheckNode), 16);
	_TopNode->TopNode = nullptr;
	_TopNode->NodeCheckValue = 0;

	_LockFreeCount = 0;
	_LockFreeCheckCount = 0;
}

template<class DATA>
inline CLockFreeStack<DATA>::~CLockFreeStack()
{

}

template<class DATA>
void CLockFreeStack<DATA>::Push(DATA Data)
{
	st_CheckNode Top;
	st_Node* NewNode = _ObjectPoolFreeList->Alloc();
	NewNode->Data = Data;

	LONG64 LockFreeCheckCount = InterlockedIncrement64(&_LockFreeCheckCount);

	do
	{
		Top.TopNode = _TopNode->TopNode;
		Top.NodeCheckValue = _TopNode->NodeCheckValue;

		NewNode->NextNode = _TopNode->TopNode;
	} while (!InterlockedCompareExchange128((volatile LONG64*)_TopNode, LockFreeCheckCount, (LONG64)NewNode, (LONG64*)&Top));

	InterlockedIncrement(&_LockFreeCount);
}

template<class DATA>
bool CLockFreeStack<DATA>::Pop(DATA* Data)
{
	st_CheckNode PopNode;
	st_Node* FreeNode;

	/*
		LockFreeCount가 음수라면 안에 내용물이 없다는 것이니까
		감소시킨것을 다시 돌려놓고 나간다.
	*/
	if (InterlockedDecrement(&_LockFreeCount) < 0)
	{
		InterlockedIncrement(&_LockFreeCount);
		return false;
	}

	LONG64 LockFreeCheckCount = InterlockedIncrement64(&_LockFreeCheckCount);

	do
	{
		PopNode.TopNode = _TopNode->TopNode;
		PopNode.NodeCheckValue = _TopNode->NodeCheckValue;
		FreeNode = PopNode.TopNode;
	} while (!InterlockedCompareExchange128((volatile LONG64*)_TopNode, LockFreeCheckCount, (LONG64)_TopNode->TopNode->NextNode, (LONG64*)&PopNode));

	*Data = PopNode.TopNode->Data;
	_ObjectPoolFreeList->Free(FreeNode);
	return true;
}
