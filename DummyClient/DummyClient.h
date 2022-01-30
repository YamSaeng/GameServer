#pragma once
#pragma comment(lib,"ws2_32")

#include "ClientInfo.h"
#include "LockFreeStack.h"

// 세션 인덱스 넣기 16비트 왼쪽으로 밀고 INDEX 넣음
#define ADD_CLIENTID_INDEX(CLIENTID,INDEX)	((CLIENTID << 0x10) | ((short)INDEX))
// 세션 아이디 얻기 
#define GET_CLIENTID(CLIENTID)	((CLIENTID & 0xFFFFFF00) >> 0x10)
#define GET_CLIENTINDEX(CLIENTID)	(CLIENTID & 0xFFFF)

class CDummyClient
{
private:
	enum en_DummyClientMessage
	{
		CHAT_MSG,
		MOVE		
	};
	
	HANDLE _DummyClientHCP;	

	bool _ConnectThreadProcEnd;

	static unsigned __stdcall WorkerThreadProc(void* Argument);
		
	static unsigned __stdcall ConnectThreadProc(void* Argument);

	static unsigned __stdcall SendThreadProc(void* Argument);

	void ReleaseClient(st_Client* ReleaseClient);

	void SendPost(st_Client* SendClient);

	void RecvPost(st_Client* RecvClient, bool IsConnectRecvPost = false);

	void RecvNotifyComplete(st_Client* RecvCompleteClient, const DWORD& Transferred);

	void SendNotifyComplete(st_Client* SendCompleteClient);

	void OnRecv(int64 ClientID, CMessage* Packet);

	HANDLE _ConnectThreadWakeEvent;

protected:
	st_Client* _ClientArray[DUMMY_CLIENT_MAX];

	CLockFreeStack<int32> _ClientArrayIndexs;

	st_Client* FindClient(int64 ClientID);

	void ReturnClient(st_Client* Client);

public:
	int64 _DummyClientId;	

	LONG _SendPacketTPS;
	LONG _RecvPacketTPS;	
	int64 _ClientCount = 0;
	int64 _ConnectCount = 0;
	int64 _LoginCount = 0;

	int64 _ConnectionTotal = 0;
	int64 _ConnectTPS = 0;
	int64 _DisconnectTPS = 0;	

	CDummyClient();
	~CDummyClient();

	void SendPacket(int64 ClientID, CMessage* Packet);
	void Disconnect(int64 ClientID);

	st_Client* FindById(int64& ObjectId);
};