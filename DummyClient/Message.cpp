#include "Message.h"

CMemoryPoolTLS<CMessage> CMessage::_ObjectPoolFreeList(0);

CMessage::CMessage()
{
	memset(_MessageBuf, 0, sizeof(_MessageBuf));
	_Header = 0;
	_Front = 5;
	_Rear = 5;
	_BufferSize = en_PACKET::BUFFER_DEFAULT;
	_UseBufferSize = 0;
	_IsEncode = false;
	_Key = 50;

	if (_RetCount == nullptr)
	{
		_RetCount = (LONG*)malloc(sizeof(LONG));
		*_RetCount = 0;
	}
}

CMessage::CMessage(int32 BufferSize)
{

}

CMessage::~CMessage()
{
	if (_RetCount != nullptr)
	{
		free(_RetCount);
		_RetCount = nullptr;
	}
}

void CMessage::Destroy(void)
{
	memset(_MessageBuf, 0, sizeof(_MessageBuf));
}

void CMessage::Clear(void)
{
	_Header = 0;
	_Front = 5;
	_Rear = 5;
	_UseBufferSize = 0;
	_IsEncode = false;
}

int32 CMessage::GetBufferSize(void)
{
	return _BufferSize;
}

int32 CMessage::GetUseBufferSize(void)
{
	return _UseBufferSize;
}

char* CMessage::GetBufferPtr(void)
{
	return _MessageBuf;
}

char* CMessage::GetHeaderBufferPtr(void)
{
	return &_MessageBuf[_Header];
}

char* CMessage::GetFrontBufferPtr(void)
{
	return &_MessageBuf[_Front];
}

char* CMessage::GetRearBufferPtr(void)
{
	return &_MessageBuf[_Rear];
}

int32 CMessage::MoveWritePosition(int32 Size)
{
	_Rear += Size;
	_UseBufferSize += Size;
	return 0;
}

int32 CMessage::MoveReadPosition(int32 Size)
{
	_Front += Size;
	return 0;
}

CMessage& CMessage::operator=(CMessage& Message)
{
	memcpy(this, &Message, sizeof(CMessage));
	return *(this);
}

CMessage& CMessage::operator<<(BYTE Value)
{
	memcpy(&_MessageBuf[_Rear], &Value, sizeof(BYTE));
	_Rear += sizeof(BYTE);
	_UseBufferSize += sizeof(BYTE);
	return *(this);
}

CMessage& CMessage::operator<<(int8 Value)
{
	memcpy(&_MessageBuf[_Rear], &Value, sizeof(char));
	_Rear += sizeof(char);
	_UseBufferSize += sizeof(char);
	return *(this);
}

CMessage& CMessage::operator<<(bool Value)
{
	memcpy(&_MessageBuf[_Rear], &Value, sizeof(bool));
	_Rear += sizeof(bool);
	_UseBufferSize += sizeof(bool);
	return *(this);
}

CMessage& CMessage::operator<<(int16 Value)
{
	memcpy(&_MessageBuf[_Rear], &Value, sizeof(int16));
	_Rear += sizeof(int16);
	_UseBufferSize += sizeof(int16);
	return *(this);
}

CMessage& CMessage::operator<<(uint16 Value)
{
	memcpy(&_MessageBuf[_Rear], &Value, sizeof(uint16));
	_Rear += sizeof(uint16);
	_UseBufferSize += sizeof(uint16);
	return *(this);
}

CMessage& CMessage::operator<<(int32 Value)
{
	memcpy(&_MessageBuf[_Rear], &Value, sizeof(int));
	_Rear += sizeof(int32);
	_UseBufferSize += sizeof(int32);
	return *(this);
}

CMessage& CMessage::operator<<(DWORD Value)
{
	memcpy(&_MessageBuf[_Rear], &Value, sizeof(DWORD));
	_Rear += sizeof(DWORD);
	_UseBufferSize += sizeof(DWORD);
	return *(this);
}

CMessage& CMessage::operator<<(float Value)
{
	memcpy(&_MessageBuf[_Rear], &Value, sizeof(float));
	_Rear += sizeof(float);
	_UseBufferSize += sizeof(float);
	return *(this);
}

CMessage& CMessage::operator<<(int64 Value)
{
	memcpy(&_MessageBuf[_Rear], &Value, sizeof(__int64));
	_Rear += sizeof(__int64);
	_UseBufferSize += sizeof(__int64);
	return *(this);
}

CMessage& CMessage::operator<<(double Value)
{
	memcpy(&_MessageBuf[_Rear], &Value, sizeof(double));
	_Rear += sizeof(double);
	_UseBufferSize += sizeof(double);
	return *(this);
}

CMessage& CMessage::operator<<(uint32 Value)
{
	memcpy(&_MessageBuf[_Rear], &Value, sizeof(uint32));
	_Rear += sizeof(uint32);
	_UseBufferSize += sizeof(uint32);
	return *(this);
}

CMessage& CMessage::operator<<(uint64 Value)
{
	memcpy(&_MessageBuf[_Rear], &Value, sizeof(uint64));
	_Rear += sizeof(uint64);
	_UseBufferSize += sizeof(uint64);
	return *(this);
}

CMessage& CMessage::operator>>(BYTE& Value)
{
	memcpy(&Value, &_MessageBuf[_Front], sizeof(BYTE));
	_Front += sizeof(BYTE);
	_UseBufferSize -= sizeof(BYTE);
	return *(this);
}

CMessage& CMessage::operator>>(int8& Value)
{
	memcpy(&Value, &_MessageBuf[_Front], sizeof(char));
	_Front += sizeof(char);
	_UseBufferSize -= sizeof(char);
	return *(this);
}

CMessage& CMessage::operator>>(bool& Value)
{
	memcpy(&Value, &_MessageBuf[_Front], sizeof(bool));
	_Front += sizeof(bool);
	_UseBufferSize -= sizeof(bool);
	return *(this);
}

CMessage& CMessage::operator>>(int16& Value)
{
	memcpy(&Value, &_MessageBuf[_Front], sizeof(int16));
	_Front += sizeof(int16);
	_UseBufferSize -= sizeof(int16);
	return *(this);
}

CMessage& CMessage::operator>>(WORD& Value)
{
	memcpy(&Value, &_MessageBuf[_Front], sizeof(WORD));
	_Front += sizeof(WORD);
	_UseBufferSize -= sizeof(WORD);
	return *(this);
}

CMessage& CMessage::operator>>(int32& Value)
{
	memcpy(&Value, &_MessageBuf[_Front], sizeof(int));
	_Front += sizeof(int32);
	_UseBufferSize -= sizeof(int);
	return *(this);
}

CMessage& CMessage::operator>>(DWORD& Value)
{
	memcpy(&Value, &_MessageBuf[_Front], sizeof(DWORD));
	_Front += sizeof(DWORD);
	_UseBufferSize -= sizeof(DWORD);
	return *(this);
}

CMessage& CMessage::operator>>(float& Value)
{
	memcpy(&Value, &_MessageBuf[_Front], sizeof(float));
	_Front += sizeof(float);
	_UseBufferSize -= sizeof(float);
	return *(this);
}

CMessage& CMessage::operator>>(int64& Value)
{
	memcpy(&Value, &_MessageBuf[_Front], sizeof(__int64));
	_Front += sizeof(int64);
	_UseBufferSize -= sizeof(__int64);
	return *(this);
}

CMessage& CMessage::operator>>(double& Value)
{
	memcpy(&Value, &_MessageBuf[_Front], sizeof(double));
	_Front += sizeof(double);
	_UseBufferSize -= sizeof(double);
	return *(this);
}

CMessage& CMessage::operator>>(uint64& Value)
{
	memcpy(&Value, &_MessageBuf[_Front], sizeof(uint64));
	_Front += sizeof(uint64);
	_UseBufferSize -= sizeof(uint64);
	return *(this);
}

int CMessage::GetData(char* Dest, int32 Size)
{
	memcpy(Dest, &_MessageBuf[_Front], Size);
	_Front += Size;
	_UseBufferSize -= Size;
	return Size;
}

int CMessage::GetData(wchar_t* Dest, int32 Size)
{
	memcpy(Dest, &_MessageBuf[_Front], Size);
	_Front += Size;
	_UseBufferSize -= Size;	
	return Size;
}

int32 CMessage::GetData(wstring& Dest, int32 Size)
{
	// 유니코드 문자열 생성후 wstring에 넣어서 반환
	WCHAR StringToWCHAR[256] = { 0 };
	memcpy(StringToWCHAR, &_MessageBuf[_Front], Size);
	wstring RetWString = StringToWCHAR;
	Dest = RetWString;
	_Front += Size;
	_UseBufferSize -= Size;
	return Size;	
}

int CMessage::InsertData(const char* Src, int32 Size)
{
	memcpy(&_MessageBuf[_Rear], Src, Size);
	_Rear += Size;
	_UseBufferSize += Size;
	return Size;
}

int CMessage::InsertData(char* Src, int32 Size)
{
	memcpy(&_MessageBuf[_Rear], Src, Size);
	_Rear += Size;
	_UseBufferSize += Size;
	return Size;
}

int CMessage::InsertData(const wchar_t* Src, int32 Size)
{
	memcpy(&_MessageBuf[_Rear], Src, Size);
	_Rear += Size;
	_UseBufferSize += Size;
	return Size;
}

int CMessage::InsertData(wchar_t* Src, int32 Size)
{
	memcpy(&_MessageBuf[_Rear], Src, Size);
	_Rear += Size;
	_UseBufferSize += Size;
	return Size;
}

void CMessage::InsertData(wstring Data)
{
	int8 Len = Data.length();
	memcpy(&_MessageBuf[_Rear], &Len, sizeof(int8));
	_Rear += sizeof(int8);
	_UseBufferSize += sizeof(int8);
	memcpy(&_MessageBuf[_Rear], Data.c_str(), Len);
	_Rear += Len;
	_UseBufferSize += Len;

}

void CMessage::SetHeader(char* Header, char Size)
{
	if (Size == 2)
	{
		_Header = 3;
		memcpy(&_MessageBuf[_Header], Header, Size);
		_UseBufferSize += Size;
	}
	else if (Size == 5)
	{
		memcpy(&_MessageBuf[_Header], Header, Size);
		_UseBufferSize += Size;
	}
}

CMessage* CMessage::Alloc()
{
	CMessage* AllocMessage = _ObjectPoolFreeList.Alloc();
	InterlockedIncrement(AllocMessage->_RetCount);	
	return AllocMessage;
}

void CMessage::Free()
{
	if (InterlockedDecrement(_RetCount) == 0)
	{
		_ObjectPoolFreeList.Free(this);
	}
}

void CMessage::AddRetCount()
{
	InterlockedIncrement(_RetCount);
}

bool CMessage::Encode()
{
	unsigned char CheckSum = 0;
	DWORD Sum = 0;
	/*
		Encode를 한번만 호출 가능하게 해준다.
	*/
	if (InterlockedCompareExchange((LONG*)&_IsEncode, true, false) == true)
	{
		return false;
	}

	srand(time(NULL));

	int32 PayLoadLen = _UseBufferSize;

	//----------------------------------------------------------------------
	//헤더 준비
	//----------------------------------------------------------------------
	st_ENCODE_HEADER EncodeHeader;
	EncodeHeader.PacketCode = 119;
	EncodeHeader.PacketLen = _UseBufferSize;
	EncodeHeader.RandXORCode = rand() % 256;

	//----------------------------------------------------------------------
	//체크섬 계산
	//모든자리를 다 더함
	//----------------------------------------------------------------------
	char* PayLoad = &_MessageBuf[_Front];
	for (int32 i = 0; i < _UseBufferSize; i++)
	{
		Sum += *PayLoad;
		PayLoad++;
	}

	//더한값을 256으로 % 연산
	CheckSum = Sum % 256;

	//체크섬 넣어줌
	EncodeHeader.CheckSum = CheckSum;

	//----------------------------------------------------------------------
	//헤더 셋팅
	//----------------------------------------------------------------------
	SetHeader((char*)&EncodeHeader, sizeof(st_ENCODE_HEADER));

	int32 P1 = 0;
	int32 E1 = 0;

	//----------------------------------------------------------------------
	//체크섬 부터 암호화
	//----------------------------------------------------------------------
	char* MovePoint = &_MessageBuf[_Front - 1];

	for (int32 i = 0; i < EncodeHeader.PacketLen + 1; i++)
	{
		P1 = *MovePoint ^ (P1 + EncodeHeader.RandXORCode + i + 1);
		E1 = P1 ^ (E1 + _Key + i + 1);
		*MovePoint = E1;
		MovePoint++;
	}

	return true;
}

bool CMessage::Decode()
{
	DWORD Sum = 0;	
	int32 P1 = 0;
	int32 E1 = 0;

	st_ENCODE_HEADER* DecodeHeader = (st_ENCODE_HEADER*)_MessageBuf;

	//PacketCode 검사
	if (DecodeHeader->PacketCode != 119)
	{
		return false;
	}

	//길이 검사
	if (DecodeHeader->PacketLen != _UseBufferSize - 5)
	{
		return false;
	}

	//----------------------------------------------------------------------
	//체크섬 부터 복호화 
	//----------------------------------------------------------------------
	char* MovePoint = &_MessageBuf[_Front - 1];
	int32 DecodePoint = 0;

	for (int32 i = 0; i < DecodeHeader->PacketLen + 1; i++)
	{
		P1 = *MovePoint ^ (DecodePoint + _Key + i + 1);
		DecodePoint = *MovePoint;
		E1 = P1 ^ (E1 + DecodeHeader->RandXORCode + i + 1);
		*MovePoint = E1;
		E1 = P1;
		MovePoint++;
	}

	//----------------------------------------------------------------------
	//복호화 한후 복호화 한 데이터를 토대로 체크섬을 구해서 검사한다.
	//----------------------------------------------------------------------
	unsigned char CheckSum = 0;
	char* PayLoad = &_MessageBuf[_Front];
	for (int32 i = 0; i < DecodeHeader->PacketLen; i++)
	{
		Sum += *PayLoad;
		PayLoad++;
	}

	CheckSum = Sum % 256;

	if (DecodeHeader->CheckSum != CheckSum)
	{
		return false;
	}

	// 인코딩 가능 하다고 알려줌
	InterlockedExchange((LONG*)&_IsEncode, false);

	return true;
}

