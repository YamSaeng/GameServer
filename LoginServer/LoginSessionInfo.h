#pragma once

#include "RingBuffer.h"
#include "LockFreeQue.h"
#include "Message.h"

#define SESSION_SEND_PACKET_MAX 500

struct st_IOBlock
{
	LONG64 IOCount;
	LONG64 IsRelease;
};

struct st_LoginSession
{
	int64 SessionId;
	int64 AccountId;
	SOCKET ClientSock;
	SOCKET CloseSock;
	SOCKADDR_IN ClientAddr;

	CRingBuffer RecvRingBuf;
	CLockFreeQue<CMessage*> SendRingBuf;

	OVERLAPPED RecvOverlapped = {};
	OVERLAPPED SendOverlapped = {};

	st_IOBlock* IOBlock = nullptr;
	LONG IsSend;

	CMessage* SendPacket[SESSION_SEND_PACKET_MAX];
	LONG SendPacketCount;	

	bool IsCancelIO;
};