#pragma once
#include <string.h>

#define BUFFER_DEFUALT_SIZE	(int)RingBuffer::eDefaultInfo::DefaultSize
#define BLANK				(int)RingBuffer::eDefaultInfo::Blank

//원형 큐
class RingBuffer
{
private:
	char* _Buffer;
	int _PeekFront;
	int _PeekRear;
	int _Front;
	int _Rear;
	int _BufferMaxSize;

public:
	enum class eDefaultInfo
	{
		DefaultSize = 10001,
		Blank = 1
	};

private:
	void Init(int bufferSize)
	{
		_Front = 0;
		_Rear = 0;
		_PeekFront = 0;
		_PeekRear = 0;
		_BufferMaxSize = bufferSize;

		_Buffer = new char[_BufferMaxSize];
		memset(_Buffer, 0, _BufferMaxSize);
	}

public:
	RingBuffer(void)
	{
		Init(BUFFER_DEFUALT_SIZE);
	}

	RingBuffer(int bufferSize)
	{
		Init(bufferSize);
	}

	~RingBuffer()
	{
		delete[] _Buffer;
	}

	int GetBufferSize(void)
	{
		return _BufferMaxSize;
	}

	int GetPeekUseSize(void)
	{
		int Front = _PeekFront;
		int Rear = _PeekRear;
		int UseSize = 0;

		if (Front <= Rear)
		{
			UseSize = Rear - Front;
		}
		else
		{
			UseSize = (_BufferMaxSize - Front) + Rear;
		}

		return UseSize;
	}

	//현재 사용하고 있는 버퍼 크기 반환
	int GetUseSize(void)
	{
		int Front = _Front;
		int Rear = _Rear;
		int UseSize = 0;

		//Rear가 Front보다 크거나 같을때
		if (Rear >= Front)
		{
			UseSize = Rear - Front;
		}
		else
		{
			UseSize = (_BufferMaxSize - Front) + Rear;
		}

		return UseSize;
	}

	//남아 있는 공간 사이즈
	int GetFreeSize(void)
	{
		int Front = _Front;
		int Rear = _Rear;
		int FreeSize = 0;

		if (Front > Rear)
		{
			FreeSize = Front - Rear - BLANK;
		}
		else
		{
			FreeSize = (_BufferMaxSize - Rear) + Front - BLANK;
		}

		return FreeSize;
	}

	int GetDirectEnqueueSize(void)
	{
		int Front = _Front;
		int Rear = _Rear;
		int Size = 0;

		if (Front > Rear)
		{
			Size = Front - Rear - BLANK;
		}
		else
		{
			if (0 == Front)
			{
				Size = _BufferMaxSize - Rear - BLANK;
			}
			else
			{
				Size = _BufferMaxSize - Rear;
			}
		}

		return Size;
	}

	int GetDirectDequeueSize(void)
	{
		int TempFront = _Front;
		int TempRear = _Rear;
		int Size = 0;

		if (TempRear >= TempFront)
		{
			Size = TempRear - TempFront;
		}
		else
		{
			Size = _BufferMaxSize - TempFront;
		}

		return Size;
	}

	int GetPeekDirectDequeSize(void)
	{
		int Front = _PeekFront;
		int Rear = _PeekRear;
		int DequeSize = 0;

		if (Rear >= Front)
		{
			DequeSize = Rear - Front;
		}
		else
		{
			DequeSize = _BufferMaxSize - Front;
		}

		return DequeSize;
	}

	int Enqueue(char* Data, int Size)
	{
		int DirectEnqSize = GetDirectEnqueueSize();
		int FreeSize = GetFreeSize();

		if (Size > FreeSize)
		{
			Size = FreeSize;
		}

		if (Size <= DirectEnqSize)
		{
			memcpy_s(&_Buffer[_Rear], Size, Data, Size);
		}
		else
		{
			memcpy_s(&_Buffer[_Rear], DirectEnqSize, Data, DirectEnqSize);
			memcpy_s(&_Buffer[0], Size - DirectEnqSize, Data + DirectEnqSize, Size - DirectEnqSize);
		}

		_PeekRear = (_PeekRear + Size) % _BufferMaxSize;
		_Rear = (_Rear + Size) % _BufferMaxSize;

		return Size;
	}


	int Dequeue(char* Dest, int Size)
	{
		int directDeqSize = GetDirectDequeueSize();
		int useSize = GetUseSize();

		if (useSize < Size)
		{
			Size = useSize;
		}

		if (Size <= directDeqSize)
		{
			memcpy_s(Dest, Size, &_Buffer[_Front], Size);
		}
		else
		{
			memcpy_s(Dest, directDeqSize, &_Buffer[_Front], directDeqSize);
			memcpy_s(Dest + directDeqSize, Size - directDeqSize, &_Buffer[0], Size - directDeqSize);
		}

		_Front = (_Front + Size) % _BufferMaxSize;

		return Size;
	}

	int Peek(char* Dest, int Size)
	{
		int directDeqSize = GetDirectDequeueSize();
		int useSize = GetUseSize();

		if (useSize < Size)
		{
			Size = useSize;
		}

		if (Size <= directDeqSize)
		{
			memcpy_s(Dest, Size, &_Buffer[_Front], Size);
		}
		else
		{
			memcpy_s(Dest, directDeqSize, &_Buffer[_Front], directDeqSize);
			memcpy_s(Dest + directDeqSize, Size - directDeqSize, &_Buffer[0], Size - directDeqSize);
		}

		if (*Dest == 0)
		{
			int a = 0;
		}
		return Size;
	}

	int NextPeek(char* Dest, int Size)
	{
		int PeekDirectDeqSize = GetPeekDirectDequeSize();
		int UseSize = GetPeekUseSize();

		if (UseSize < Size)
		{
			Size = UseSize;
		}

		if (Size == 0)
		{
			return -1;
		}

		if (Size <= PeekDirectDeqSize)
		{
			memcpy_s(Dest, Size, &_Buffer[_PeekFront], Size);
		}
		else
		{
			memcpy_s(Dest, PeekDirectDeqSize, &_Buffer[_PeekFront], PeekDirectDeqSize);
			memcpy_s(Dest + PeekDirectDeqSize, Size - PeekDirectDeqSize, &_Buffer[0], Size - PeekDirectDeqSize);
		}

		_PeekFront = (_PeekFront + Size) % _BufferMaxSize;

		return Size;
	}

	int MoveRear(int Size)
	{
		int freeSize = GetFreeSize();
		if (freeSize < Size)
		{
			Size = freeSize;
		}

		_Rear = (_Rear + Size) % _BufferMaxSize;

		return Size;
	}

	int MoveFront(int Size)
	{
		int useSize = GetUseSize();
		if (useSize < Size)
		{
			Size = useSize;
		}

		_Front = (_Front + Size) % _BufferMaxSize;

		return Size;
	}

	void ClearBuffer(void)
	{
		_PeekFront = _PeekRear = _Front = _Rear = 0;
	}

	bool IsEmpty(void)
	{
		return (_Front == _Rear);
	}

	char* GetFrontBufferPtr(void)
	{
		return &_Buffer[_Front];
	}

	char* GetRearBufferPtr(void)
	{
		return &_Buffer[_Rear];
	}

	char* GetBufferPtr(void)
	{
		return &_Buffer[0];
	}	
};