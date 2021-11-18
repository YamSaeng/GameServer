#pragma once
#include<stdlib.h>

template <typename KEY, typename DATA>
struct st_HEAP_NODE
{
	KEY Key;
	DATA Data;	
};

template<typename KEY, typename DATA>
class CHeap
{
private:
	st_HEAP_NODE<KEY, DATA> *_HeapArray;
	int _HeapSize;
	int _UseSize;
public:	
	CHeap() {};
	CHeap(int HeapSize);
	~CHeap();

	void InsertHeap(KEY Key, DATA InsertData);
	DATA PopHeap();
	DATA Peek();

	int GetUseSize();
	int GetHeapSize();
	void Clear();
};

//template<typename KEY, typename DATA>
//CMemoryPoolTLS<st_HEAP_NODE<KEY,DATA>> CHeap<KEY, DATA>::_HeapMemoryPool(0);

template<typename KEY, typename DATA>
CHeap<KEY, DATA>::CHeap(int HeapSize)
{	
	_HeapArray = (st_HEAP_NODE<KEY, DATA>*)malloc(sizeof(st_HEAP_NODE<KEY, DATA>)*HeapSize);
	memset(_HeapArray, 0, sizeof(st_HEAP_NODE<KEY, DATA>)*HeapSize);
	_HeapSize = HeapSize;
	_UseSize = 0;
}

template<typename KEY, typename DATA>
CHeap<KEY, DATA>::~CHeap()
{
	free(_HeapArray);
	_HeapArray = nullptr;
}

template<typename KEY, typename DATA>
void CHeap<KEY, DATA>::InsertHeap(KEY Key, DATA InsertData)
{
	if (_UseSize == _HeapSize)
	{
		return;
	}

	//맨뒤에 데이터를 삽입한다.
	_HeapArray[_UseSize].Key = Key;
	_HeapArray[_UseSize].Data = InsertData;

	//개수 증가
	_UseSize++;

	//삽입한 위치 
	int Now = _UseSize - 1;

	while (Now > 0)
	{
		//삽입한 위치 부모
		int ParentIndex = (Now - 1) / 2;

		//부모 값이 삽입한 위치 값보다 크다면 나감
		if (_HeapArray[ParentIndex].Key < _HeapArray[Now].Key)
		{
			break;
		}

		//아니라면 부모 위치 데이터와 바꿔줌
		st_HEAP_NODE<KEY, DATA> Temp = _HeapArray[Now];
		_HeapArray[Now] = _HeapArray[ParentIndex];
		_HeapArray[ParentIndex] = Temp;

		//삽입한 위치를 부모로
		Now = ParentIndex;
	}	
}

template<typename KEY, typename DATA>
DATA CHeap<KEY, DATA>::PopHeap()
{	
	int LeftPosition = 0;
	int RightPosition = 0;

	//첫번째 데이터 저장
	DATA ReturnData = _HeapArray[0].Data;
	//맨 뒤에 있는 값을 앞으로 옮겨줌
	_HeapArray[0] = _HeapArray[_UseSize - 1];	

	_UseSize--;

	int Selected = 0;

	while (1)
	{
		//왼쪽 자식 위치
		int LeftPosition = Selected * 2 + 1;
		//오른쪽 자식 위치
		int RightPosition = LeftPosition + 1;

		//현재 위치를 부모로 잡아줌
		int NowParent = Selected;

		//왼쪽 자식 위치가 현재 사용중인 사이즈 (_UseSize보다 작거나 같고)
		//왼쪽 자식의 기준이 되는 값이 부모보다 크다면
		if (_UseSize >= LeftPosition && _HeapArray[LeftPosition].Key < _HeapArray[NowParent].Key)
		{
			//값을 바꿔주기 위해 NowParent의 값을 왼쪽 자식 위치로 잡아줌
			NowParent = LeftPosition;
		}

		//오른쪽 자식 위치가 현재 사용중인 사이즈 (_UseSize보다 작거나 같고)
		//오른쪽 자식의 기준이 되는 값이 부모보다 크다면
		if (_UseSize >= RightPosition && _HeapArray[RightPosition].Key < _HeapArray[NowParent].Key)
		{
			//값을 바꿔주기 위해 NowParent의 값을 오른쪽 자식 위치로 잡아준다.
			NowParent = RightPosition;
		}
		//만약 오른쪽 자식이 왼쪽 자식 보다 더 크다면 자연스럽게 오른쪽 자식의 값으로 바꿔줄 위치가 바뀜

		//왼쪽 자식 오른쪽 자식 둘다 부모보다 크지 않다면 탈출
		if (NowParent == Selected)
		{
			break;
		}

		//선택된 위치와 부모의 값을 바꿔줌
		st_HEAP_NODE<KEY, DATA> Temp = _HeapArray[Selected];
		_HeapArray[Selected] = _HeapArray[NowParent];
		_HeapArray[NowParent] = Temp;

		Selected = NowParent;
	}

	return ReturnData;
}

template<typename KEY, typename DATA>
DATA CHeap<KEY, DATA>::Peek()
{
	return _HeapArray[0].Data;
}

template<typename KEY, typename DATA>
int CHeap<KEY, DATA>::GetUseSize()
{
	return _UseSize;
}

template<typename KEY, typename DATA>
int CHeap<KEY, DATA>::GetHeapSize()
{
	return _HeapSize;
}

template<typename KEY, typename DATA>
void CHeap<KEY, DATA>::Clear()
{
	memset(_HeapArray, 0, sizeof(st_HEAP_NODE<KEY,DATA>) * _HeapSize);
	_UseSize = 0;
}