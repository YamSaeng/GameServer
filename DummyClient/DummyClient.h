#pragma once
#pragma comment(lib,"ws2_32")

#include "ClientInfo.h"
#include "LockFreeStack.h"

#define DUMMY_CLIENT_MAX 1000

// ���� �ε��� �ֱ� 16��Ʈ �������� �а� INDEX ����
#define ADD_CLIENTID_INDEX(CLIENTID,INDEX)	((CLIENTID << 0x10) | ((short)INDEX))
// ���� ���̵� ��� 
#define GET_CLIENTID(CLIENTID)	((CLIENTID & 0xFFFFFF00) >> 0x10)
#define GET_CLIENTINDEX(CLIENTID)	(CLIENTID & 0xFFFF)

class CDummyClient
{
private:
	enum en_DummyClientMessage
	{
		CHAT_MSG,
		MOVE,
		MELEE_ATTACK,
		DISCONNECT
	};

	HANDLE _DummyClientHCP;	

	static unsigned __stdcall WorkerThreadProc(void* Argument);
		
	static unsigned __stdcall ConnectThreadProc(void* Argument);

	static unsigned __stdcall SendProc(void* Argument);

	void ReleaseClient(st_Client* ReleaseClient);

	void SendPost(st_Client* SendClient);

	void RecvPost(st_Client* RecvClient, bool IsConnectRecvPost = false);

	void RecvNotifyComplete(st_Client* RecvCompleteClient, const DWORD& Transferred);

	void SendNotifyComplete(st_Client* SendCompleteClient);

	void OnRecv(int64 ClientID, CMessage* Packet);

protected:
	st_Client* _ClientArray[DUMMY_CLIENT_MAX];

	CLockFreeStack<int32> _ClientArrayIndexs;

	st_Client* FindClient(int64 ClientID);

	void ReturnClient(st_Client* Client);

public:
	int64 _DummyClientId;	

	LONG _SendPacketTPS;

	CDummyClient();
	~CDummyClient();

	void SendPacket(int64 ClientID, CMessage* Packet);
	void Disconnect(int64 ClientID);
};