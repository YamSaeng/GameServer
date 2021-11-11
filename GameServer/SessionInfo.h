#pragma once
#include "RingBuffer.h"
#include "LockFreeQue.h"
#include "Message.h"
#include "pch.h"

#define SESSION_CHARACTER_MAX 3

class CPlayer;

struct st_IOBlock
{
	LONG64 IOCount;		//IO작업 횟수를 기록해둘 변수 및 현재 세션을 사용하고 있는지에 대한 확인 
	LONG64 IsRelease;   //Release를 했는지 안했는지 기록
};

struct st_Session
{
	int64 SessionId;		// 게임서버에서 발급한 SessionId
	int64 AccountId;		// Account서버에서 발급한 AccointId
	SOCKET ClientSock;		// 데이터 송수신 소켓
	SOCKET CloseSock;		// Disconnect 호출시 종료 절차용 소켓
	SOCKADDR_IN ClientAddr; // 접속한 클라 주소

	RingBuffer RecvRingBuf;
	CLockFreeQue<CMessage*> SendRingBuf;

	OVERLAPPED RecvOverlapped = {}; // WSARecv 통지용
	OVERLAPPED SendOverlapped = {}; // WSASend 통지용

	st_IOBlock* IOBlock = nullptr;
	LONG IsSend;		// 해당 세션에 대해 WSASend작업을 하고 있는지 안하고 있는지 판단해줄 변수 1 = WSASend 작업중 0 = WSASend 작업중 아님

	CMessage* SendPacket[500]; //세션이 보내는 패킷을 담아둘 배열
	LONG SendPacketCount; //해당 세션이 몇개의 패킷을 보내고 있는지 기록해주는 변수

	wstring LoginId;
	wstring CreateCharacterName;

	int16 SectorX;
	int16 SectorY;

	int32 Token;
	bool IsLogin;

	// 마지막으로 받은 핑 패킷 시간
	int64 PingPacketTime;

	// 플레이어 캐릭터가 할당된 인덱스를 저장
	int32 MyPlayerIndexes[SESSION_CHARACTER_MAX];
	// 현재 조종하고 있는 캐릭터의 인덱스를 저장
	int32 MyPlayerIndex;	

	//char DebugArray[100000];
	//int DebugArrayCount;
};