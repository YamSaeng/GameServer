#pragma once
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<new.h>

//������ ���������� ���� �������� �����ϰ�
//������� ��� ������ �� ��� ������ ���ؼ�
//���ÿ� ��尡 ������ ��ȯ���ְ� ���ٸ� ���� �Ҵ��Ͽ� ��ȯ���ش�.

template <class DATA>
class CObjectPoolFreeList
{
private:
#pragma pack(push,1)
	struct st_BLOCK_NODE
	{
		DATA Data;				  //���� ������
		st_BLOCK_NODE* NextBlock; //��ϵ��� ����Ʈ ���·� ���� �ϰ� �ֱ⿡ ���� ����� �ּҸ� ��Ƶ� ����
	};
#pragma pack(pop)
#pragma pack(push,1)
	struct st_CHECK_BLOCK_NODE
	{
		st_BLOCK_NODE* TopNode;
		LONG64 NodeCheckValue;    //���������� üũ�Ҷ� ���� ������
	};
#pragma pack(pop)
	st_CHECK_BLOCK_NODE* _TopNode; //������ Top�� ������ ���
	LONG _AllocCount;			   //�� �� ����
	LONG _UseCount;				   //��� ���� �� ����
	LONG64 _LockFreeCheckCount;    //������ üũ�� ī��Ʈ

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
	//			�Ҹ��ڸ� ���϶�� ���� false�̸� ���⼭ �ϰ������� �Ҹ��ڸ� ȣ�����ش�.
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
//���ο� ��� �Ҵ�
//��� �� �� �ִ� ���� ������� ���� �Ҵ��Ͽ� ��ȯ���ְ�
//��� �� �� �ִ� ���� ������� �������� ���� ��ȯ���ش�.
//----------------------------------------------------------  
template<class DATA>
DATA* CObjectPoolFreeList<DATA>::Alloc(void)
{
	st_BLOCK_NODE* AllocNode;
	st_CHECK_BLOCK_NODE Top;
	int AllocCount = _AllocCount;
	int UseCount = _UseCount;

	InterlockedIncrement(&_UseCount);

	if (AllocCount > _UseCount) //��� �� �� �ִ� ���� �ִ� ���
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
	else //��� �� �� �ִ� ���� ���� ���
	{
		//���ο� ��� �Ҵ����ְ� ������ ȣ�� �� �Ѱ���

		AllocNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
		memset(AllocNode, 0, sizeof(st_BLOCK_NODE));
		new (AllocNode) DATA;

		/*
			�Ҵ� ���� ��� ���� �÷��ش�.
		*/
		InterlockedIncrement(&_AllocCount);
	}

	return (DATA*)&AllocNode->Data;
}

//-----------------------------------------------
//��� ���� ���� ��ȯ�Ѵ�.
//-----------------------------------------------
template<class DATA>
bool CObjectPoolFreeList<DATA>::Free(DATA* Data)
{
	/*
		���� �����͸� �޸�Ǯ �ȿ� �ֱ� ( Push )
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
