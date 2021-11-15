#pragma once
#include "Type.h"

#include <time.h>

#include "MemoryPoolTLS.h"

//-------------------------------------------------------------------------------------
//데이터를 바이트 단위로 내부에서 관리하는 버퍼에 넣는다.
//구조로 
//Code(1Byte) - Len(2Byte) - Rand XOR Code(1Byte) - CheckSum(1Byte) - PayLoad(Len Byte)
/*
	Code = 1차적으로 메세지 체킹
	Len = 메세지 길이
	Rand XOR Code = 0~255 중 랜덤한 수
	CheckSum = PayLoad 부분을 1Byte씩 모두 더해서 % 256한 값
*/
//-------------------------------------------------------------------------------------
class CMessage
{
public:
#pragma pack(push,1)
	struct st_ENCODE_HEADER
	{
		BYTE PacketCode; //패킷 1차 검사 코드

		WORD PacketLen; //패킷 길이

		BYTE RandXORCode; //랜덤 XOR 값
		BYTE CheckSum; //체크섬
	};
#pragma pack(pop)
protected:
	enum en_PACKET
	{
		BUFFER_DEFAULT = 10000
	};
	char _MessageBuf[BUFFER_DEFAULT];

	int32 _Front;
	int32 _Rear;
	int32 _Header;

	int32 _BufferSize;
	int32 _UseBufferSize; //버퍼 사용 크기	
	BYTE _Key; //고정 키값
public:
	//청크 메모리풀
	static CMemoryPoolTLS<CMessage> _ObjectPoolFreeList;
	LONG* _RetCount; //해당 메세지가 몇개나 쓰이고 있는지 확인		
	bool _IsEncode; //EnCoding 했는지 안했는지 여부
	CMessage();
	CMessage(int32 BufferSize);
	virtual ~CMessage();

	/*
		패킷 파괴
	*/
	void Destroy(void);
	/*
		패킷 청소
	*/
	void Clear(void);

	int32 GetBufferSize(void);
	int32 GetUseBufferSize(void);
	char* GetBufferPtr(void);
	char* GetHeaderBufferPtr(void);
	char* GetFrontBufferPtr(void);
	char* GetRearBufferPtr(void);
	int32 MoveWritePosition(int32 Size);
	int32 MoveReadPosition(int32 Size);

#pragma region 데이터 넣기	
	CMessage& operator = (CMessage& Message);
	CMessage& operator << (BYTE Value);
	CMessage& operator << (int8 Value);
	CMessage& operator << (bool Value);

	CMessage& operator << (int16 Value);
	CMessage& operator << (uint16 Value);

	CMessage& operator << (int32 Value);
	CMessage& operator << (DWORD Value);
	CMessage& operator << (float Value);

	CMessage& operator << (int64 Value);
	CMessage& operator << (double Value);
	CMessage& operator << (uint32 Value);
	CMessage& operator << (uint64 Value);	

	int32 GetData(char* Dest, int32 Size);
	int32 GetData(wchar_t* Dest, int32 Size);
	int32 GetData(wstring& Dest, int32 Size);
#pragma endregion
#pragma region 데이터 빼기	
	CMessage& operator >> (BYTE& Value);
	CMessage& operator >> (int8& Value);
	CMessage& operator >> (bool& Value);

	CMessage& operator >> (int16& Value);
	CMessage& operator >> (WORD& Value);

	CMessage& operator >> (int32& Value);
	CMessage& operator >> (DWORD& Value);
	CMessage& operator >> (float& Value);

	CMessage& operator >> (int64& Value);
	CMessage& operator >> (double& Value);
	CMessage& operator >> (uint64& Value);
		
	int InsertData(const char* Src, int32 Size);
	int InsertData(char* Src, int32 Size);
	int InsertData(const wchar_t* src, int32 Size);
	int InsertData(wchar_t* Src, int32 Size);
	void InsertData(wstring Data);
#pragma endregion

	/*
		헤더 셋팅
	*/
	void SetHeader(char* Header, char Size);

	//메모리풀로부터 메세지 하나 할당
	static CMessage* Alloc();
	void Free();
	void AddRetCount();

	//패킷 인코딩
	bool Encode();
	//패킷 디코딩
	bool Decode();
};