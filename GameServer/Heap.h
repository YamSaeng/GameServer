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

	int GetUseSize();
	int GetHeapSize();
	void Clear();
};

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

	//�ǵڿ� �����͸� �����Ѵ�.
	_HeapArray[_UseSize].Key = Key;
	_HeapArray[_UseSize].Data = InsertData;

	//���� ����
	_UseSize++;

	//������ ��ġ 
	int Now = _UseSize - 1;

	while (Now > 0)
	{
		//������ ��ġ �θ�
		int ParentIndex = (Now - 1) / 2;

		//�θ� ���� ������ ��ġ ������ ũ�ٸ� ����
		if (_HeapArray[ParentIndex].Key > _HeapArray[Now].Key)
		{
			break;
		}

		//�ƴ϶�� �θ� ��ġ �����Ϳ� �ٲ���
		st_HEAP_NODE<KEY, DATA> Temp = _HeapArray[Now];
		_HeapArray[Now] = _HeapArray[ParentIndex];
		_HeapArray[ParentIndex] = Temp;

		//������ ��ġ�� �θ��
		Now = ParentIndex;
	}	
}

template<typename KEY, typename DATA>
DATA CHeap<KEY, DATA>::PopHeap()
{	
	int LeftPosition = 0;
	int RightPosition = 0;

	//ù��° ������ ����
	DATA ReturnData = _HeapArray[0].Data;
	//�� �ڿ� �ִ� ���� ������ �Ű���
	_HeapArray[0] = _HeapArray[_UseSize - 1];	

	_UseSize--;

	int Selected = 0;

	while (1)
	{
		//���� �ڽ� ��ġ
		int LeftPosition = Selected * 2 + 1;
		//������ �ڽ� ��ġ
		int RightPosition = LeftPosition + 1;

		//���� ��ġ�� �θ�� �����
		int NowParent = Selected;

		//���� �ڽ� ��ġ�� ���� ������� ������ (_UseSize���� �۰ų� ����)
		//���� �ڽ��� ������ �Ǵ� ���� �θ𺸴� ũ�ٸ�
		if (_UseSize >= LeftPosition && _HeapArray[LeftPosition].Key > _HeapArray[NowParent].Key)
		{
			//���� �ٲ��ֱ� ���� NowParent�� ���� ���� �ڽ� ��ġ�� �����
			NowParent = LeftPosition;
		}

		//������ �ڽ� ��ġ�� ���� ������� ������ (_UseSize���� �۰ų� ����)
		//������ �ڽ��� ������ �Ǵ� ���� �θ𺸴� ũ�ٸ�
		if (_UseSize >= RightPosition && _HeapArray[RightPosition].Key > _HeapArray[NowParent].Key)
		{
			//���� �ٲ��ֱ� ���� NowParent�� ���� ������ �ڽ� ��ġ�� ����ش�.
			NowParent = RightPosition;
		}
		//���� ������ �ڽ��� ���� �ڽ� ���� �� ũ�ٸ� �ڿ������� ������ �ڽ��� ������ �ٲ��� ��ġ�� �ٲ�

		//���� �ڽ� ������ �ڽ� �Ѵ� �θ𺸴� ũ�� �ʴٸ� Ż��
		if (NowParent == Selected)
		{
			break;
		}

		//���õ� ��ġ�� �θ��� ���� �ٲ���
		st_HEAP_NODE<KEY, DATA> Temp = _HeapArray[Selected];
		_HeapArray[Selected] = _HeapArray[NowParent];
		_HeapArray[NowParent] = Temp;

		Selected = NowParent;
	}

	return ReturnData;
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