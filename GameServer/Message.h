#pragma once

/*
	SendPost에서 IOCoint를 증가시켜 보니 1이라면 이미 누가 Release작업중이므로
	감소시키고 빠져나가줘야한다.
*/
//뽑으려고 햇는데 빈공간이 있으면
//변수 하나를 둬서 빈공간이 있었다고 판단되면 변수 하나에 특정한 값을 셋팅한후
//뽑아낸 후 그 값을 확인하는 작업을 해준다.

//다른방법으로는 throw try catch 방법을 사용한다.
//내부에서는 throw를 던지고 바깥쪽에서 try catch로 throw를 받아내야하는데 받아 내는 부분은 Proc에서 한번에 받아준다.
//직렬화버퍼 전용의 throw객체를 만들어줘야한다.
//직렬화버퍼 전용으로 안만들면 만약에 다른곳에서 throw가 발생했다면 Proc에서 try catch할때 받아낼 수 있으니까
//실질적으로 안쪽에서 throw를 던져 줄 때 throw에 담겨져 있는 내용은 
//throw객체가 담고 있는 내용은 직렬화버퍼에 들어온 프로토콜의 타입과 실제로 들어와있는 직렬화버퍼의 내용 전부를 가지고 있고 catch에서 보여줘야한다.
//catch를 할때는 결국 최종적으로 하는 작업은 연결종료로써 귀결된다.

//_DefaultMessageBuf에 복사할 데이터가 BUFFER_DEFAULT보다 크면 동적할당 해줄 것인지
//에러 메세지를 뱉어낼 것인지 결정해야함

//9월 23일
/*
	문자열 복사해주는 오버로딩 연산자 필요
	함수 안에서 길이를 알아내서 복사를 해주는 형식으로 만들어줘야함
*/
#include <time.h>

#include "MemoryPoolTLS.h"
#include "GameObject.h"

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