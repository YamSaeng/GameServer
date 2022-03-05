#pragma once
#include "NetworkLib.h"
#include "LoginServerInfo.h"

class CLoginServer : public CNetworkLib
{
public:
	void LoginServerStart(const WCHAR* OpenIP, int32 Port);
private:
	HANDLE _AuthThread;
	HANDLE _UserDataBaseThread; 

	HANDLE _DataBaseThread;

	HANDLE _AuthThreadWakeEvent;
	HANDLE _DataBaseThreadWakeEvent;

	bool _AuthThreadEnd;
	bool _DataBaseThreadEnd;

	CLockFreeQue<st_LoginServerJob*> _AuthThreadMessageQue;	

	CMemoryPoolTLS<st_LoginServerJob>* _LoginServerJobMemoryPool;

	//-------------------------------------------------------
	// 인증 쓰레드 ( 클라 접속, 클라 접속 종료 )
	//-------------------------------------------------------
	static unsigned __stdcall AuthThreadProc(void* Argument);	

	virtual void OnRecv(int64 SessionID, CMessage* Packet) override;
	
	void PacketProc(int64 SessionID, CMessage* Packet);
	void PacketProcReqAccountNew(int64 SessionID, CMessage* Packet);

	void AccountNew(int64 SessionID, CMessage* Packet);
	void AccountLogIn(int64 SessionID);
	void AccountLogOut(int64 SessionID);

	void DeleteClient(int64 SessionID);
};

