#pragma once

/*
	SendPost���� IOCoint�� �������� ���� 1�̶�� �̹� ���� Release�۾����̹Ƿ�
	���ҽ�Ű�� ������������Ѵ�.
*/
//�������� �޴µ� ������� ������
//���� �ϳ��� �ּ� ������� �־��ٰ� �ǴܵǸ� ���� �ϳ��� Ư���� ���� ��������
//�̾Ƴ� �� �� ���� Ȯ���ϴ� �۾��� ���ش�.

//�ٸ�������δ� throw try catch ����� ����Ѵ�.
//���ο����� throw�� ������ �ٱ��ʿ��� try catch�� throw�� �޾Ƴ����ϴµ� �޾� ���� �κ��� Proc���� �ѹ��� �޾��ش�.
//����ȭ���� ������ throw��ü�� ���������Ѵ�.
//����ȭ���� �������� �ȸ���� ���࿡ �ٸ������� throw�� �߻��ߴٸ� Proc���� try catch�Ҷ� �޾Ƴ� �� �����ϱ�
//���������� ���ʿ��� throw�� ���� �� �� throw�� ����� �ִ� ������ 
//throw��ü�� ��� �ִ� ������ ����ȭ���ۿ� ���� ���������� Ÿ�԰� ������ �����ִ� ����ȭ������ ���� ���θ� ������ �ְ� catch���� ��������Ѵ�.
//catch�� �Ҷ��� �ᱹ ���������� �ϴ� �۾��� ��������ν� �Ͱ�ȴ�.

//_DefaultMessageBuf�� ������ �����Ͱ� BUFFER_DEFAULT���� ũ�� �����Ҵ� ���� ������
//���� �޼����� �� ������ �����ؾ���

//9�� 23��
/*
	���ڿ� �������ִ� �����ε� ������ �ʿ�
	�Լ� �ȿ��� ���̸� �˾Ƴ��� ���縦 ���ִ� �������� ����������
*/
#include <time.h>

#include "MemoryPoolTLS.h"
#include "GameObject.h"

//-------------------------------------------------------------------------------------
//�����͸� ����Ʈ ������ ���ο��� �����ϴ� ���ۿ� �ִ´�.
//������ 
//Code(1Byte) - Len(2Byte) - Rand XOR Code(1Byte) - CheckSum(1Byte) - PayLoad(Len Byte)
/*
	Code = 1�������� �޼��� üŷ
	Len = �޼��� ����
	Rand XOR Code = 0~255 �� ������ ��
	CheckSum = PayLoad �κ��� 1Byte�� ��� ���ؼ� % 256�� ��
*/
//-------------------------------------------------------------------------------------
class CMessage
{
public:
#pragma pack(push,1)
	struct st_ENCODE_HEADER
	{
		BYTE PacketCode; //��Ŷ 1�� �˻� �ڵ�

		WORD PacketLen; //��Ŷ ����

		BYTE RandXORCode; //���� XOR ��
		BYTE CheckSum; //üũ��
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
	int32 _UseBufferSize; //���� ��� ũ��	
	BYTE _Key; //���� Ű��
public:
	//ûũ �޸�Ǯ
	static CMemoryPoolTLS<CMessage> _ObjectPoolFreeList;
	LONG* _RetCount; //�ش� �޼����� ��� ���̰� �ִ��� Ȯ��		
	bool _IsEncode; //EnCoding �ߴ��� ���ߴ��� ����
	CMessage();
	CMessage(int32 BufferSize);
	virtual ~CMessage();

	/*
		��Ŷ �ı�
	*/
	void Destroy(void);
	/*
		��Ŷ û��
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

#pragma region ������ �ֱ�	
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
#pragma region ������ ����	
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
		��� ����
	*/
	void SetHeader(char* Header, char Size);

	//�޸�Ǯ�κ��� �޼��� �ϳ� �Ҵ�
	static CMessage* Alloc();
	void Free();
	void AddRetCount();

	//��Ŷ ���ڵ�
	bool Encode();
	//��Ŷ ���ڵ�
	bool Decode();
};