#pragma once
#include "NetworkLib.h"
#include "LoginServerInfo.h"

class CLoginServer : public CNetworkLib
{
public:
	CLoginServer();
	~CLoginServer();	
	void LoginServerStart(const WCHAR* OpenIP, int32 Port);
private:
	HANDLE _AuthThread;	

	HANDLE _DataBaseThread;

	HANDLE _AuthThreadWakeEvent;
	HANDLE _DataBaseThreadWakeEvent;

	bool _AuthThreadEnd;
	bool _DataBaseThreadEnd;

	CLockFreeQue<st_LoginServerJob*> _AuthThreadMessageQue;		
	
	//--------------------------------------------------------
	// ������ ���̽� ť
	//--------------------------------------------------------
	CLockFreeQue<st_LoginServerJob*> _DataBaseThreadMessageQue;
	//--------------------------------------------------------
	// �� �޸�Ǯ
	//--------------------------------------------------------
	CMemoryPoolTLS<st_LoginServerJob>* _LoginServerJobMemoryPool;

	//-------------------------------------------------------
	// ���� ������ ( Ŭ�� ����, Ŭ�� ���� ���� )
	//-------------------------------------------------------
	static unsigned __stdcall AuthThreadProc(void* Argument);	
	static unsigned __stdcall DataBaseThreadProc(void* Argument);	

	virtual void OnClientJoin(int64 SessionID) override;
	virtual void OnClientLeave(st_LoginSession* LeaveSession) override;
	virtual bool OnConnectionRequest(const wchar_t ClientIP, int32 Port) override;

	virtual void OnRecv(int64 SessionID, CMessage* Packet) override;
	
	//------------------------------------------------
	// ��Ŷ ó��
	//------------------------------------------------
	void PacketProc(int64 SessionID, CMessage* Packet);
	//-------------------------------------------------------------
	// ȸ������ ��û ó��
	//-------------------------------------------------------------
	void PacketProcReqAccountNew(int64 SessionID, CMessage* Packet);
	//-------------------------------------------------------------
	// �α��� ��û ó��
	//-------------------------------------------------------------
	void PacketProcReqAccountLogin(int64 SessionID, CMessage* Packet);
	//-------------------------------------------------------------
	// �α׾ƿ� ��û ó��
	//-------------------------------------------------------------
	void PacketProcReqAccountLogOut(int64 SessionID, CMessage* Packet);
	//-------------------------------------------------------------
	// �α��� ���� ���� ó��
	//-------------------------------------------------------------
	void PacketProcReqLoginStateChange(int64 SessionID, CMessage* Packet);

	//------------------------------------------------
	// ���ο� ���� �ֱ�
	//------------------------------------------------
	void AccountNew(int64 SessionID, CMessage* Packet);
	//------------------------------------------------
	// �α��� 
	//------------------------------------------------
	void AccountLogIn(int64 SessionID, CMessage* Packet);	
	//-------------------------------------------------------------
	// �α��� ���� ����
	//-------------------------------------------------------------
	void AccountLogInStateChange(int64 SessionID, CMessage* Packet);
	
	void DeleteClient(st_LoginSession* Session);

	//-----------------------------------
	// ���� ��� ��������
	//-----------------------------------
	vector<st_ServerInfo> GetServerList();
	
	//-------------------------------------------------------
	// ���ο� ���� �ֱ� ���� ��Ŷ ����
	//-------------------------------------------------------
	CMessage* MakePacketResAccountNew(bool AccountNewSuccess);
	//-----------------------------------------------------------------------------------------------------
	// �α��� ���� ��Ŷ ���� 
	//-----------------------------------------------------------------------------------------------------
	CMessage* MakePacketResAccountLogin(en_LoginInfo LoginInfo, int64 AccountID, wstring AccountName, int8 TokenLen, BYTE* Token, vector<st_ServerInfo> ServerLists);
};

