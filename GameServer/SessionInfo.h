#pragma once
#include "RingBuffer.h"
#include "LockFreeQue.h"
#include "Message.h"
#include "pch.h"

#define SESSION_CHARACTER_MAX 3

class CPlayer;

struct st_IOBlock
{
	LONG64 IOCount;		//IO�۾� Ƚ���� ����ص� ���� �� ���� ������ ����ϰ� �ִ����� ���� Ȯ�� 
	LONG64 IsRelease;   //Release�� �ߴ��� ���ߴ��� ���
};

struct st_Session
{
	int64 SessionId;		// ���Ӽ������� �߱��� SessionId
	int64 AccountId;		// Account�������� �߱��� AccointId
	SOCKET ClientSock;		// ������ �ۼ��� ����
	SOCKET CloseSock;		// Disconnect ȣ��� ���� ������ ����
	SOCKADDR_IN ClientAddr; // ������ Ŭ�� �ּ�

	RingBuffer RecvRingBuf;
	CLockFreeQue<CMessage*> SendRingBuf;

	OVERLAPPED RecvOverlapped = {}; // WSARecv ������
	OVERLAPPED SendOverlapped = {}; // WSASend ������

	st_IOBlock* IOBlock = nullptr;
	LONG IsSend;		// �ش� ���ǿ� ���� WSASend�۾��� �ϰ� �ִ��� ���ϰ� �ִ��� �Ǵ����� ���� 1 = WSASend �۾��� 0 = WSASend �۾��� �ƴ�

	CMessage* SendPacket[500]; //������ ������ ��Ŷ�� ��Ƶ� �迭
	LONG SendPacketCount; //�ش� ������ ��� ��Ŷ�� ������ �ִ��� ������ִ� ����

	wstring LoginId;
	wstring CreateCharacterName;

	int16 SectorX;
	int16 SectorY;

	int32 Token;
	bool IsLogin;

	// ���������� ���� �� ��Ŷ �ð�
	int64 PingPacketTime;

	// �÷��̾� ĳ���Ͱ� �Ҵ�� �ε����� ����
	int32 MyPlayerIndexes[SESSION_CHARACTER_MAX];
	// ���� �����ϰ� �ִ� ĳ������ �ε����� ����
	int32 MyPlayerIndex;	

	//char DebugArray[100000];
	//int DebugArrayCount;
};