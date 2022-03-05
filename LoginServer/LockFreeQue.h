#pragma once
#include "ObjectPoolFreeList.h"

template <class DATA>
class CLockFreeQue
{
private:
	struct st_Node
	{
		DATA Data;
		st_Node* Next;
	};

	struct st_Check_Node
	{
		st_Node* TopNode;
		LONG64 NodeCheckValue;
	};

	//struct st_Debug
	//{
	//	DWORD ThreadID;
	//	char Log[10];
	//	//원본 Front or Rear 노드
	//	st_Node *FrontRearNode;
	//	LONG64 FrontRearNodeCheckValue;
	//	st_Node *FrontRearNodeNext;
	//	//복사 Front or Rear 노드
	//	st_Node *FrontRearNodeCopy;
	//	LONG64 FrontRearNodeCopyCheckValue;
	//	st_Node *FrontRearNodeCopyNext;
	//	//새로 Enque하는 노드
	//	st_Node *EnqueNode;
	//	DATA EnqueNodeData;
	//	LONG Size;
	//	st_Node* NextNode;
	//};

	//st_Debug DebugArray[1000000];
	//int DebugLogCount;
	//int DebugArrayLoopCount;

	st_Check_Node* _Front;
	st_Check_Node* _Rear;

	st_Node* _DummyNode;

	CObjectPoolFreeList<st_Node>* _ObjectPoolFreeList;
	LONG64 _LockCheckCount;
	SRWLOCK _LockFreeLock;
	LONG _Size;
public:
	CLockFreeQue();
	~CLockFreeQue();

	int Enqueue(DATA Data);
	bool Dequeue(DATA* Data);
	LONG GetUseSize();
	bool IsEmpty();
	void ClearBuffer();

	/*void Debug(DWORD ThreadID, const char *Msg,
		st_Node *FrontRearNode, LONG64 FrontRearNodeCheckValue, st_Node *FrontRearNodeNext,
		st_Node *FrontRearNodeCopy, LONG64 FrontRearNodeCopyCheckValue, st_Node *FrontRearNodeCopyNext,
		st_Node* EnqueNode, DATA EnqueNodeData,
		LONG Size, st_Node* NextNode =nullptr);*/
};

template<class DATA>
CLockFreeQue<DATA>::CLockFreeQue()
{
	InitializeSRWLock(&_LockFreeLock);

	//memset(DebugArray, 0, sizeof(DebugArray));

	//DebugLogCount = 0;
	//DebugArrayLoopCount = 0;

	_Size = 0;
	_LockCheckCount = 0;

	_ObjectPoolFreeList = new CObjectPoolFreeList<st_Node>();

	_DummyNode = _ObjectPoolFreeList->Alloc();
	_DummyNode->Next = nullptr;

	_Front = (st_Check_Node*)_aligned_malloc(sizeof(st_Check_Node), 16);
	_Front->TopNode = _DummyNode;
	_Front->NodeCheckValue = 0;

	_Rear = (st_Check_Node*)_aligned_malloc(sizeof(st_Check_Node), 16);
	_Rear->TopNode = _DummyNode;
	_Rear->NodeCheckValue = 0;
}

template<class DATA>
CLockFreeQue<DATA>::~CLockFreeQue()
{

}

template<class DATA>
int CLockFreeQue<DATA>::Enqueue(DATA Data)
{
	st_Check_Node Rear;
	st_Node* Next;

	st_Node* EnqueNode = _ObjectPoolFreeList->Alloc();
	EnqueNode->Data = Data;
	EnqueNode->Next = nullptr;
	//Debug(GetCurrentThreadId(), "CurACCurAC", nullptr, 0, nullptr, nullptr, 0, nullptr, EnqueNode, EnqueNode->Data, _Size);

	LONG64 LockCheckCount = InterlockedIncrement64(&_LockCheckCount);

	for (;;)
	{
		//Debug(GetCurrentThreadId(), "PE0RearCop", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, nullptr, 0, nullptr, EnqueNode, EnqueNode->Data, _Size);
		Rear.NodeCheckValue = _Rear->NodeCheckValue;
		//Debug(GetCurrentThreadId(), "PE1RearCop", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, Rear.TopNode, 0, Rear.TopNode->Next, EnqueNode, EnqueNode->Data, _Size);
		Rear.TopNode = _Rear->TopNode;
		Next = Rear.TopNode->Next;
		//Debug(GetCurrentThreadId(), "ReECopComp", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, Rear.TopNode, Rear.NodeCheckValue, Rear.TopNode->Next, EnqueNode, EnqueNode->Data, _Size, Next);

		/*
			Next가 nullptr 마지막 노드의 다음 노드가 비어있을 때
		*/
		if (Next == nullptr)
		{
			//Debug(GetCurrentThreadId(), "EnFirsCA11", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, Rear.TopNode, Rear.NodeCheckValue, Rear.TopNode->Next, EnqueNode, EnqueNode->Data, _Size);

			if (InterlockedCompareExchangePointer((PVOID*)&_Rear->TopNode->Next, EnqueNode, nullptr) == nullptr)
			{
				//Debug(GetCurrentThreadId(), "EnSeNxCA22", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, Rear.TopNode, Rear.NodeCheckValue, Rear.TopNode->Next, EnqueNode, EnqueNode->Data, _Size);
				InterlockedCompareExchange128((volatile LONG64*)_Rear, LockCheckCount, (LONG64)EnqueNode, (LONG64*)&Rear);
				//Debug(GetCurrentThreadId(), "EnSeNxCA23", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, Rear.TopNode, Rear.NodeCheckValue, Rear.TopNode->Next, EnqueNode, EnqueNode->Data, _Size);
				break;
			}
			else
			{
				//Debug(GetCurrentThreadId(), "EnSeCAfail", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, Rear.TopNode, Rear.NodeCheckValue, Rear.TopNode->Next, EnqueNode, EnqueNode->Data, _Size);
			}
		}
		else
		{
			//Debug(GetCurrentThreadId(), "EnSeNnCA22", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, Rear.TopNode, Rear.NodeCheckValue, Rear.TopNode->Next, EnqueNode, EnqueNode->Data, _Size);
			InterlockedCompareExchange128((volatile LONG64*)_Rear, LockCheckCount, (LONG64)Rear.TopNode->Next, (LONG64*)&Rear);
			//Debug(GetCurrentThreadId(), "EnSeNnCA23", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, Rear.TopNode, Rear.NodeCheckValue, Rear.TopNode->Next, EnqueNode, EnqueNode->Data, _Size);
		}
	}

	InterlockedIncrement(&_Size);
	//Debug(GetCurrentThreadId(), "EnSizeIncr", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, Rear.TopNode, Rear.NodeCheckValue, Rear.TopNode->Next, EnqueNode, EnqueNode->Data, _Size);
	return 0;
}

template<class DATA>
bool CLockFreeQue<DATA>::Dequeue(DATA* Data)
{
	st_Check_Node Front;
	st_Check_Node Rear;

	st_Node* FrontNext;

	//Size가 1인데 _Front->Next가 nullptr인 경우 찾기 ( 정상적인 상황 )
	if (InterlockedDecrement(&_Size) < 0)
	{
		InterlockedIncrement(&_Size);
		Data = nullptr;
		return false;
	}

	LONG64 LockCheckCount = InterlockedIncrement64(&_LockCheckCount);

	for (;;)
	{
		//Debug(GetCurrentThreadId(), "Pr0FronCop", _Front->TopNode, _Front->NodeCheckValue, _Front->TopNode->Next, nullptr, 0, nullptr, nullptr, 0, _Size);
		Front.NodeCheckValue = _Front->NodeCheckValue;
		//Debug(GetCurrentThreadId(), "Pr1FronCop", _Front->TopNode, _Front->NodeCheckValue, _Front->TopNode->Next, Front.TopNode, 0, Front.TopNode->Next, nullptr, 0, _Size);
		Front.TopNode = _Front->TopNode;
		//Debug(GetCurrentThreadId(), "FronCopCom", _Front->TopNode, _Front->NodeCheckValue, _Front->TopNode->Next, Front.TopNode, Front.NodeCheckValue, Front.TopNode->Next, nullptr, 0, _Size);
		//Debug(GetCurrentThreadId(), "PD0RearCop", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, nullptr, 0, nullptr, nullptr, 0, _Size);
		Rear.NodeCheckValue = _Rear->NodeCheckValue;
		//Debug(GetCurrentThreadId(), "PD1RearCop", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, Rear.TopNode, 0, Rear.TopNode->Next, nullptr, 0, _Size);
		Rear.TopNode = _Rear->TopNode;
		//Debug(GetCurrentThreadId(), "ReDCopComp", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, Rear.TopNode, Rear.NodeCheckValue, Rear.TopNode->Next, nullptr, 0, _Size);

		FrontNext = Front.TopNode->Next;
		//Debug(GetCurrentThreadId(), "FrNexCopCo", _Front->TopNode, _Front->NodeCheckValue, _Front->TopNode->Next, Front.TopNode, Front.NodeCheckValue, Front.TopNode->Next, nullptr, 0, _Size);

		/*
			Rear가 옮겨져 있지 않다면 여기서 뒤로 한번 밀어주고 재시도 한다.
		*/
		if (Rear.TopNode->Next != nullptr)
		{
			//Debug(GetCurrentThreadId(), "DeqRearPus", _Rear->TopNode, _Rear->NodeCheckValue, _Rear->TopNode->Next, Rear.TopNode, Rear.NodeCheckValue, Rear.TopNode->Next, nullptr, 0, _Size, FrontNext);
			InterlockedCompareExchange128((volatile LONG64*)_Rear, LockCheckCount, (LONG64)Rear.TopNode->Next, (LONG64*)&Rear);
		}
		else
		{
			if (FrontNext != nullptr)
			{
				*Data = FrontNext->Data;
				//Debug(GetCurrentThreadId(), "PrDeqFront", _Front->TopNode, _Front->NodeCheckValue, _Front->TopNode->Next, Front.TopNode, Front.NodeCheckValue, Front.TopNode->Next, nullptr, 0, _Size, FrontNext);
				if (InterlockedCompareExchange128((volatile LONG64*)_Front, LockCheckCount, (LONG64)FrontNext, (LONG64*)&Front))
				{
					//Debug(GetCurrentThreadId(), "CoDeqFront", _Front->TopNode, _Front->NodeCheckValue, _Front->TopNode->Next, Front.TopNode, Front.NodeCheckValue, Front.TopNode->Next, nullptr, 0, _Size, FrontNext);					
					_ObjectPoolFreeList->Free(Front.TopNode);
					return true;
				}
				else
				{
					//Debug(GetCurrentThreadId(), "CoDequFail", _Front->TopNode, _Front->NodeCheckValue, _Front->TopNode->Next, Front.TopNode, Front.NodeCheckValue, Front.TopNode->Next, nullptr, 0, _Size, FrontNext);
				}
			}
			else
			{
				//Debug(GetCurrentThreadId(), "FroNexNull", _Front->TopNode, _Front->NodeCheckValue, _Front->TopNode->Next, Front.TopNode, Front.NodeCheckValue, Front.TopNode->Next, nullptr, 0, _Size, FrontNext);
			}
		}
	}
}

template<class DATA>
LONG CLockFreeQue<DATA>::GetUseSize()
{
	return _Size;
}

template<class DATA>
bool CLockFreeQue<DATA>::IsEmpty()
{
	if (_Size == 0)
	{
		if (_Front->TopNode == _Rear->TopNode)
		{
			if (_Front->TopNode->Next == nullptr)
			{
				return true;
			}
		}
	}

	return false;
}

template<class DATA>
void CLockFreeQue<DATA>::ClearBuffer()
{
	while (!IsEmpty())
	{
		st_Node* ReturnNode = _Front->TopNode;
		_Front->TopNode = _Front->TopNode->Next;
		_ObjectPoolFreeList->Free(ReturnNode);
		_Size--;
	}
}

//template<class DATA>
//void CLockFreeQue<DATA>::Debug(DWORD ThreadID, const char *Msg,
//	st_Node *FrontRearNode, LONG64 FrontRearNodeCheckValue, st_Node *FrontRearNodeNext,
//	st_Node *FrontRearNodeCopy, LONG64 FrontRearNodeCopyCheckValue, st_Node *FrontRearNodeCopyNext,
//	st_Node* EnqueNode, DATA EnqueNodeData,
//	LONG Size, st_Node* NextNode)
//{	
//	AcquireSRWLockExclusive(&_LockFreeLock);
//
//	//쓰레드 아이디 저장
//	DebugArray[DebugLogCount].ThreadID = ThreadID;
//
//	//메세지 저장
//	memcpy(DebugArray[DebugLogCount].Log, Msg, 10);
//
//	/*
//		원본 노드, 데이터, 원본 노드 넥스트 저장
//	*/
//	DebugArray[DebugLogCount].FrontRearNode = FrontRearNode;
//	if (FrontRearNode != nullptr)
//	{
//		DebugArray[DebugLogCount].FrontRearNodeCheckValue = FrontRearNodeCheckValue;
//	}
//
//	DebugArray[DebugLogCount].FrontRearNodeNext = FrontRearNodeNext;
//
//	/*
//		복사본 노드, 데이터, 복사본 노드 넥스트 저장
//	*/
//	DebugArray[DebugLogCount].FrontRearNodeCopy = FrontRearNodeCopy;
//	if (FrontRearNodeCopy != nullptr)
//	{
//		DebugArray[DebugLogCount].FrontRearNodeCopyCheckValue = FrontRearNodeCopyCheckValue;
//	}
//
//	DebugArray[DebugLogCount].FrontRearNodeCopyNext = FrontRearNodeCopyNext;
//
//	/*
//		넣는 노드, 데이터 저장
//	*/
//	DebugArray[DebugLogCount].EnqueNode = EnqueNode;
//	if (EnqueNode != nullptr)
//	{
//		DebugArray[DebugLogCount].EnqueNodeData = EnqueNode->Data;
//	}
//		
//	/*
//		사이즈 저장
//	*/
//	DebugArray[DebugLogCount].Size = Size;
//	DebugArray[DebugLogCount].NextNode = NextNode;
//	DebugLogCount++;
//	if (DebugLogCount >= 999999)
//	{		
//		DebugArrayLoopCount++;
//		DebugLogCount = 0;
//	}		
//
//	ReleaseSRWLockExclusive(&_LockFreeLock);
//}