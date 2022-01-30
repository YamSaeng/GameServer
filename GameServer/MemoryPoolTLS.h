#pragma once
#include "ObjectPoolFreeList.h"

//------------------------------
//청크 메모리풀 TLS(쓰레드 지역 저장소) 사용
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

		st_CHUNK_BLOCK _ChunkDatas[CHUNK_DATA_MAX]; // 청크 블록안에 있는 데이터
		LONG _ChunkIndex; // 청크 데이터 중 할당 갯수
		LONG _FreeCount;  // 청크 데이터 중 반환 갯수
	public:
		CChunk()
		{
			_ChunkIndex = CHUNK_DATA_MAX;
			_FreeCount = CHUNK_DATA_MAX;
		}
	};

	DWORD _TlsIndex;
	CObjectPoolFreeList<CChunk>* _ChunkObjectFreeList; // 청크를 관리하는 메모리풀
	bool _IsPlacementNew;
	LONG _AllocCount;  // 청크 개수
	LONG _UseCount;    // 청크 낱개 개수
	LONG _ReturnCount; // 청크 반납 개수
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
	//청크 만들어서 청크안에 있는 블럭 반환 ( 블럭 100개 )
	//청크 블럭 100개 반환시 새로 할당받아서 재 사용
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

		//새로 할당
		if (Chunk->_ChunkIndex == 0)
		{
			CChunk* NewChunk = _ChunkObjectFreeList->Alloc();

			NewChunk->_ChunkIndex = CHUNK_DATA_MAX;
			NewChunk->_FreeCount = CHUNK_DATA_MAX;

			InterlockedIncrement(&_AllocCount);
			InterlockedAdd(&_UseCount, 100);

			TlsSetValue(_TlsIndex, NewChunk); //쓰레드 지역저장소에 새로 받은 청크를 등록해둔다.`
		}

		InterlockedDecrement(&_UseCount);
		InterlockedIncrement(&_ReturnCount);

		return &Chunk->_ChunkDatas[Chunk->_ChunkIndex].Data;
	}

	//100 개 소모시 청크 반환
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