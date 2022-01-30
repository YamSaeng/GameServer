#pragma once
#include "ObjectPoolFreeList.h"

//------------------------------
//ûũ �޸�Ǯ TLS(������ ���� �����) ���
//------------------------------
template <class DATA>
class CMemoryPoolTLS
{
private:
	enum en_MEMORY_POOL_TLS_INIT
	{
		CHUNK_DATA_MAX = 100
	};

	class CChunk
	{
	private:
		friend CMemoryPoolTLS;
		struct st_CHUNK_BLOCK
		{
			__int64 ChunkBlockCheckCode;
			CChunk* Chunk;
			DATA Data;
		};

		st_CHUNK_BLOCK _ChunkDatas[CHUNK_DATA_MAX]; // ûũ ��Ͼȿ� �ִ� ������
		LONG _ChunkIndex; // ûũ ������ �� �Ҵ� ����
		LONG _FreeCount;  // ûũ ������ �� ��ȯ ����
	public:
		CChunk()
		{
			_ChunkIndex = CHUNK_DATA_MAX;
			_FreeCount = CHUNK_DATA_MAX;
		}
	};

	DWORD _TlsIndex;
	CObjectPoolFreeList<CChunk>* _ChunkObjectFreeList; // ûũ�� �����ϴ� �޸�Ǯ
	bool _IsPlacementNew;
	LONG _AllocCount;  // ûũ ����
	LONG _UseCount;    // ûũ ���� ����
	LONG _ReturnCount; // ûũ �ݳ� ����
public:
	CMemoryPoolTLS(bool Placement = false)
	{
		_IsPlacementNew = Placement;
		_TlsIndex = TlsAlloc();

		if (TLS_OUT_OF_INDEXES == _TlsIndex)
		{

		}

		_ChunkObjectFreeList = new CObjectPoolFreeList<CChunk>();

		_AllocCount = 0;
		_UseCount = 0;
		_ReturnCount = 0;
	}

	~CMemoryPoolTLS()
	{
		if (_ChunkObjectFreeList != nullptr)
		{
			delete _ChunkObjectFreeList;
		}
	}

	//--------------------------------------------------
	//ûũ ���� ûũ�ȿ� �ִ� �� ��ȯ ( �� 100�� )
	//ûũ �� 100�� ��ȯ�� ���� �Ҵ�޾Ƽ� �� ���
	//--------------------------------------------------
	DATA* Alloc()
	{
		CChunk* Chunk = (CChunk*)TlsGetValue(_TlsIndex);
		if (Chunk == nullptr)
		{
			Chunk = _ChunkObjectFreeList->Alloc();

			Chunk->_ChunkIndex = CHUNK_DATA_MAX;
			Chunk->_FreeCount = CHUNK_DATA_MAX;

			InterlockedIncrement(&_AllocCount);
			InterlockedAdd(&_UseCount, 100);

			TlsSetValue(_TlsIndex, Chunk);
		}

		Chunk->_ChunkIndex--;

		Chunk->_ChunkDatas[Chunk->_ChunkIndex].ChunkBlockCheckCode = 0x52522525;
		Chunk->_ChunkDatas[Chunk->_ChunkIndex].Chunk = Chunk;

		if (_IsPlacementNew)
		{
			new (&Chunk->_ChunkDatas[Chunk->_ChunkIndex].Data) DATA;
		}

		//���� �Ҵ�
		if (Chunk->_ChunkIndex == 0)
		{
			CChunk* NewChunk = _ChunkObjectFreeList->Alloc();

			NewChunk->_ChunkIndex = CHUNK_DATA_MAX;
			NewChunk->_FreeCount = CHUNK_DATA_MAX;

			InterlockedIncrement(&_AllocCount);
			InterlockedAdd(&_UseCount, 100);

			TlsSetValue(_TlsIndex, NewChunk); //������ ��������ҿ� ���� ���� ûũ�� ����صд�.`
		}

		InterlockedDecrement(&_UseCount);
		InterlockedIncrement(&_ReturnCount);

		return &Chunk->_ChunkDatas[Chunk->_ChunkIndex].Data;
	}

	//100 �� �Ҹ�� ûũ ��ȯ
	bool Free(DATA* FreeData)
	{
		typename CChunk::st_CHUNK_BLOCK* FreeChunkBlock = (typename CChunk::st_CHUNK_BLOCK*)((char*)FreeData - sizeof(CChunk*) - sizeof(__int64));

		if (FreeChunkBlock->ChunkBlockCheckCode != 0x52522525)
		{
			return false;
		}

		InterlockedDecrement(&_ReturnCount);		

		if (InterlockedDecrement(&FreeChunkBlock->Chunk->_FreeCount) == 0)
		{
			InterlockedDecrement(&_AllocCount);
			_ChunkObjectFreeList->Free(FreeChunkBlock->Chunk);
			return true;
		}

		return false;
	}

	LONG GetAllocCount()
	{
		return _AllocCount;
	}

	LONG GetUseCount()
	{
		return _UseCount;
	}

	LONG ReturnCount()
	{
		return _ReturnCount;
	}
};