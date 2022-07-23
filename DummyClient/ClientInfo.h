#pragma once
#include "GameObjectInfo.h"
#include "LockFreeQue.h"
#include "RingBuffer.h"
#include "Message.h"
#include "CommonProtocol.h"
#include "QuickSlotManager.h"
#include "SkillBox.h"

#define DUMMY_CLIENT_MAX 600

#define DUMMY_CLIENT_RE_CONNECT_TIME 200
#define DUMMY_CLIENT_SEND_TIME 200
#define DUMMY_CLIENT_LOGIN_TIME 500

#define DUMMY_CLIENT_DISCONNECT 20//50

enum en_DummyClientNetworkState
{
	CONNECTED,
	READY_LOGIN,
	REQ_LOGIN,
	IN_LOGIN,	
	IN_ENTER_GAME,
	WAIT_CHARACTER_INFOSEND,
	REQ_RELEASE,
	RELEASE
};

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
	en_DummyClientNetworkState DummyClientNetworkState;	
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
		
	bool IsCancelIO;		

	st_GameObjectInfo MyCharacterGameObjectInfo;

	int64 DummyClientLoginTime;
	int64 ClientSendMessageTime;
	int64 ClientReConnectTime;		
	int64 ClientReMoveTime;	
	int64 ClientChatTime;	

	CSkillBox _SkillBox;
	CQuickSlotManager _QuickSlotManager;
};