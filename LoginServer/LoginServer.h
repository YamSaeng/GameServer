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
	// 데이터 베이스 큐
	//--------------------------------------------------------
	CLockFreeQue<st_LoginServerJob*> _DataBaseThreadMessageQue;
	//--------------------------------------------------------
	// 잡 메모리풀
	//--------------------------------------------------------
	CMemoryPoolTLS<st_LoginServerJob>* _LoginServerJobMemoryPool;

	//-------------------------------------------------------
	// 인증 쓰레드 ( 클라 접속, 클라 접속 종료 )
	//-------------------------------------------------------
	static unsigned __stdcall AuthThreadProc(void* Argument);	
	static unsigned __stdcall DataBaseThreadProc(void* Argument);	

	virtual void OnClientJoin(int64 SessionID) override;
	virtual void OnClientLeave(st_LoginSession* LeaveSession) override;
	virtual bool OnConnectionRequest(const wchar_t ClientIP, int32 Port) override;

	virtual void OnRecv(int64 SessionID, CMessage* Packet) override;
	
	//------------------------------------------------
	// 패킷 처리
	//------------------------------------------------
	void PacketProc(int64 SessionID, CMessage* Packet);
	//-------------------------------------------------------------
	// 회원가입 요청 처리
	//-------------------------------------------------------------
	void PacketProcReqAccountNew(int64 SessionID, CMessage* Packet);
	//-------------------------------------------------------------
	// 로그인 요청 처리
	//-------------------------------------------------------------
	void PacketProcReqAccountLogin(int64 SessionID, CMessage* Packet);
	//-------------------------------------------------------------
	// 로그아웃 요청 처리
	//-------------------------------------------------------------
	void PacketProcReqAccountLogOut(int64 SessionID, CMessage* Packet);
	//-------------------------------------------------------------
	// 로그인 상태 변경 처리
	//-------------------------------------------------------------
	void PacketProcReqLoginStateChange(int64 SessionID, CMessage* Packet);

	//------------------------------------------------
	// 새로운 계정 넣기
	//------------------------------------------------
	void AccountNew(int64 SessionID, CMessage* Packet);
	//------------------------------------------------
	// 로그인 
	//------------------------------------------------
	void AccountLogIn(int64 SessionID, CMessage* Packet);	
	//-------------------------------------------------------------
	// 로그인 상태 변경
	//-------------------------------------------------------------
	void AccountLogInStateChange(int64 SessionID, CMessage* Packet);
	
	void DeleteClient(st_LoginSession* Session);

	//-----------------------------------
	// 서버 목록 가져오기
	//-----------------------------------
	vector<st_ServerInfo> GetServerList();
	
	//-------------------------------------------------------
	// 새로운 계정 넣기 응답 패킷 조합
	//-------------------------------------------------------
	CMessage* MakePacketResAccountNew(bool AccountNewSuccess);
	//-----------------------------------------------------------------------------------------------------
	// 로그인 응답 패킷 조합 
	//-----------------------------------------------------------------------------------------------------
	CMessage* MakePacketResAccountLogin(en_LoginInfo LoginInfo, int64 AccountID, wstring AccountName, int8 TokenLen, BYTE* Token, vector<st_ServerInfo> ServerLists);
};

