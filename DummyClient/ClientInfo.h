#pragma once
#include "GameObjectInfo.h"
#include "LockFreeQue.h"
#include "RingBuffer.h"
#include "Message.h"
#include "CommonProtocol.h"

struct st_IOBlock
{
	LONG64 IOCount;
	LONG64 IsRelease;
};

struct st_Client
{
	int64 ClientId;
	SOCKET ServerSocket;
	SOCKET CloseSocket;
	SOCKADDR_IN ServerAddr;

	CRingBuffer RecvRingBuf;
	CLockFreeQue<CMessage*> SendRingBuf;	

	OVERLAPPED RecvOverlapped = {};
	OVERLAPPED SendOverlapped = {};

	st_IOBlock* IOBlock = nullptr;
	LONG IsSend;

	CMessage* SendPacket[500];
	LONG SendPacketCount;

	int64 AccountId;
	wstring LoginId;

	int32 X;
	int32 Y;

	WCHAR ChatMsg[256];

	bool IsLogin;
	bool IsEnterGame;

	LONG IsConnected;		

	st_GameObjectInfo MyCharacterGameObjectInfo;

	int64 ClientSendMessageTime;	
};