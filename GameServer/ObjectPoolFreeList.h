#pragma once
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<new.h>

//노드들을 내부적으로 스택 형식으로 관리하고
//사용중인 노드 개수와 총 노드 개수를 비교해서
//스택에 노드가 있으면 반환해주고 없다면 새로 할당하여 반환해준다.

template <class DATA>
class CObjectPoolFreeList
{
private:
#pragma pack(push,1)
	struct st_BLOCK_NODE
	{
		DATA Data;				  //실제 데이터
		st_BLOCK_NODE* NextBlock; //블록들을 리스트 형태로 관리 하고 있기에 다음 블록의 주소를 담아둘 변수
	};
#pragma pack(pop)
#pragma pack(push,1)
	struct st_CHECK_BLOCK_NODE
	{
		st_BLOCK_NODE* TopNode;
		LONG64 NodeCheckValue;    //락프리스택 체크할때 쓰일 변수값
	};
#pragma pack(pop)
	st_CHECK_BLOCK_NODE* _TopNode; //스택의 Top과 동일한 기능
	LONG _AllocCount;			   //총 블럭 개수
	LONG _UseCount;				   //사용 중인 블럭 개수
	LONG64 _LockFreeCheckCount;    //락프리 체크용 카운트

	bool _IsPlacementNew;
public:
	CObjectPoolFreeList(int BlockNum);
	virtual ~CObjectPoolFreeList();

	DATA* Alloc(void);
	bool Free(DATA* Data);

	int GetAllocCount(void);
	int GetUseCount(void);
	bool IsAlloc();
};

template<class DATA>
CObjectPoolFreeList<DATA>::CObjectPoolFreeList(int BlockNum)
{
	_TopNode = (st_CHECK_BLOCK_NODE*)_aligned_malloc(sizeof(st_CHECK_BLOCK_NODE), 16);
	_TopNode->TopNode = nullptr;
	_TopNode->NodeCheckValue = 0;
	_AllocCount = 0;
	_UseCount = 0;
	_LockFreeCheckCount = 0;
}

template<class DATA>
CObjectPoolFreeList<DATA>::~CObjectPoolFreeList()
{
	//bool IsDestructorCall;
	//if (_UseCount > 0)
	//{
	//	CObjectPoolException ObjectPoolException;
	//}

	//if (_TopNode)
	//{
	//	for (int i = 0; i < _AllocCount; i++)
	//	{
	//		char *p = (char*)_TopNode;
	//		char *q = p;
	//		//p += sizeof(st_BLOCK_NODE);
	//		p += (sizeof(st_BLOCK_NODE*) + sizeof(int) + sizeof(bool));
	//		memcpy(&IsDestructorCall, p, sizeof(bool));

	//		/*
	//			소멸자를 콜하라는 것이 false이면 여기서 일괄적으로 소멸자를 호출해준다.
	//		*/
	//		if (!IsDestructorCall)
	//		{
	//			p += sizeof(bool);
	//			DATA* FreeData = (DATA*)p;
	//			FreeData->~DATA();
	//		}

	//		_TopNode = _TopNodeNode.NextBlock;

	//		if (!_FreeNode)
	//		{
	//			free(q);
	//		}
	//	}

	//	if (_FreeNode)
	//	{
	//		free(_FreeNode);
	//	}
	//}
}

//----------------------------------------------------------
//새로운 노드 할당
//사용 할 수 있는 블럭이 없을경우 새로 할당하여 반환해주고
//사용 할 수 있는 블럭이 있을경우 보관중인 블럭을 반환해준다.
//----------------------------------------------------------  
template<class DATA>
DATA* CObjectPoolFreeList<DATA>::Alloc(void)
{
	st_BLOCK_NODE* AllocNode;
	st_CHECK_BLOCK_NODE Top;
	int AllocCount = _AllocCount;
	int UseCount = _UseCount;

	InterlockedIncrement(&_UseCount);

	if (AllocCount > _UseCount) //사용 할 수 있는 블럭이 있는 경우
	{
		LONG64 LockFreeCheckCount = InterlockedIncrement64(&_LockFreeCheckCount);

		do
		{
			Top.NodeCheckValue = _TopNode->NodeCheckValue;
			Top.TopNode = _TopNode->TopNode;

			AllocNode = Top.TopNode;
		} while (!InterlockedCompareExchange128((volatile LONG64*)_TopNode, LockFreeCheckCount, (LONG64)_TopNode->TopNode->NextBlock, (LONG64*)&Top));

		new (AllocNode) DATA;
	}
	else //사용 할 수 있는 블럭이 없을 경우
	{
		//새로운 노드 할당해주고 생성자 호출 후 넘겨줌

		AllocNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
		memset(AllocNode, 0, sizeof(st_BLOCK_NODE));
		new (AllocNode) DATA;

		/*
			할당 수와 사용 수를 늘려준다.
		*/
		InterlockedIncrement(&_AllocCount);
	}

	return (DATA*)&AllocNode->Data;
}

//-----------------------------------------------
//사용 다한 블럭을 반환한다.
//-----------------------------------------------
template<class DATA>
bool CObjectPoolFreeList<DATA>::Free(DATA* Data)
{
	/*
		받은 데이터를 메모리풀 안에 넣기 ( Push )
	*/
	st_CHECK_BLOCK_NODE Top;
	st_BLOCK_NODE* FreeNode = (st_BLOCK_NODE*)Data;

	do
	{
		Top.NodeCheckValue = _TopNode->NodeCheckValue;
		Top.TopNode = _TopNode->TopNode;

		FreeNode->NextBlock = Top.TopNode;
	} while (!InterlockedCompareExchange128((volatile LONG64*)_TopNode, Top.NodeCheckValue, (LONG64)FreeNode, (LONG64*)&Top));

	InterlockedDecrement(&_UseCount);

	return true;
}

template<class DATA>
int CObjectPoolFreeList<DATA>::GetAllocCount(void)
{
	return _AllocCount;
}

template<class DATA>
int CObjectPoolFreeList<DATA>::GetUseCount(void)
{
	return _UseCount;
}

template<class DATA>
bool CObjectPoolFreeList<DATA>::IsAlloc()
{
	return false;
}
