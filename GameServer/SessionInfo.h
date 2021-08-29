#pragma once
#include "RingBuffer.h"
#include "LockFreeQue.h"
#include "Message.h"
#include "pch.h"

#define SESSION_CHARACTER_MAX 3

class CPlayer;

struct st_IO_BLOCK
{
	LONG64 IOCount;		//IO�۾� Ƚ���� ����ص� ���� �� ���� ������ ����ϰ� �ִ����� ���� Ȯ�� 
	LONG64 IsRelease;   //Release�� �ߴ��� ���ߴ��� ���
};

struct st_SESSION
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

	st_IO_BLOCK* IOBlock = nullptr;
	LONG IsSend;		// �ش� ���ǿ� ���� WSASend�۾��� �ϰ� �ִ��� ���ϰ� �ִ��� �Ǵ����� ���� 1 = WSASend �۾��� 0 = WSASend �۾��� �ƴ�
	LONG IsCancelIO;	// CancelIO�� ȣ�� �ߴ��� �Ǵ����ִ� ���� 1 = ȣ�� �� 0 = ȣ�� ����

	CMessage* SendPacket[500]; //������ ������ ��Ŷ�� ��Ƶ� �迭
	LONG SendPacketCount; //�ش� ������ ��� ��Ŷ�� ������ �ִ��� ������ִ� ����

	wstring LoginId;
	wstring CreateCharacterName;

	int16 SectorX;
	int16 SectorY;

	int32 Token;
	bool IsLogin;

	DWORD RecvPacketTime;

	CPlayer* MyPlayers[SESSION_CHARACTER_MAX];
	CPlayer* MyPlayer;

	//char DebugArray[100000];
	//int DebugArrayCount;
};