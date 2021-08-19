#pragma once
#include "RingBuffer.h"
#include "LockFreeQue.h"
#include "Message.h"
#include "pch.h"

struct st_IO_BLOCK
{
	LONG64 IOCount;		//IO작업 횟수를 기록해둘 변수 및 현재 세션을 사용하고 있는지에 대한 확인 
	LONG64 IsRelease;   //Release를 했는지 안했는지 기록
};

struct st_SESSION
{
	int64 SessionId;		// 게임서버에서 발급한 SessionId
	SOCKET ClientSock;		// 데이터 송수신 소켓
	SOCKET CloseSock;		// Disconnect 호출시 종료 절차용 소켓
	SOCKADDR_IN ClientAddr; // 접속한 클라 주소

	RingBuffer RecvRingBuf;
	CLockFreeQue<CMessage*> SendRingBuf;

	OVERLAPPED RecvOverlapped = {}; // WSARecv 통지용
	OVERLAPPED SendOverlapped = {}; // WSASend 통지용

	st_IO_BLOCK* IOBlock = nullptr;
	LONG IsSend;		// 해당 세션에 대해 WSASend작업을 하고 있는지 안하고 있는지 판단해줄 변수 1 = WSASend 작업중 0 = WSASend 작업중 아님
	LONG IsCloseSocket; // closesocket을 햇는지 안햇는지의 여부를 판단해주는 변수 1 = 닫앗던 소켓 0 = 안닫은 소켓
	LONG IsCancelIO;	// CancelIO를 호출 했는지 판단해주는 변수 1 = 호출 함 0 = 호출 안함

	CMessage* SendPacket[500]; //세션이 보내는 패킷을 담아둘 배열
	LONG SendPacketCount; //해당 세션이 몇개의 패킷을 보내고 있는지 기록해주는 변수

	//char DebugArray[100000];
	//int DebugArrayCount;
};