#pragma once
#pragma comment(lib,"ws2_32")

#include "SessionInfo.h"
#include "LockFreeStack.h"

#define CLOSE_SOCKET_DO_NOT 0
#define CLOSE_SOCKET_DO  1

#define SENDING_DO_NOT 0
#define SENDING_DO 1

#define USING_DO_NOT 0 
#define USING_DO 1

#define SERVER_SESSION_MAX 15000

// 세션 인덱스 넣기 16비트 왼쪽으로 밀고 INDEX 넣음
#define ADD_SESSIONID_INDEX(SESSIONID,INDEX)	((SESSIONID << 0x10) | ((short)INDEX))
// 세션 아이디 얻기 
#define GET_SESSIOINID(SESSIONID)	((SESSIONID & 0xFFFFFF00) >> 0x10)
#define GET_SESSIONINDEX(SESSIONID)	(SESSIONID & 0xFFFF)

using namespace std;

class CNetworkLib
{
private:
	HANDLE _HCP; // IOCP Handle
	SOCKET _ListenSock; // 리슨소켓

	__int64 _SessionID;
	
	//---------------------------------------------------------
	// IOCP 워커 쓰레드
	//---------------------------------------------------------
	static unsigned __stdcall WorkerThreadProc(void* Argument);

	//---------------------------------------------------------
	// Accept 전용 쓰레드
	//---------------------------------------------------------
	static unsigned __stdcall AcceptThreadProc(void* Argument);

	//---------------------------------------------------------
	// 연결 끊어졌을때 세션반납
	// IOCount 확인 Release 중인지 확인
	//---------------------------------------------------------
	void ReleaseSession(st_Session* ReleaseSession);	

	//------------------------------------
	// WSASend를 예약한다.
	//------------------------------------
	void SendPost(st_Session* SendSession);

	//-------------------------------------------------------------------
	// WSARecv를 예약한다.
	//-------------------------------------------------------------------
	void RecvPost(st_Session* RecvSession, bool IsAcceptRecvPost = false);

	//--------------------------------------------------------------------------------
	// Recv완료시 호출하는 함수
	//--------------------------------------------------------------------------------
	void RecvNotifyComplete(st_Session* RecvCompleteSession, const DWORD& Transferred);
	//--------------------------------------------------------------------------------
	// Send완료시 호출하는 함수
	//--------------------------------------------------------------------------------
	void SendNotifyComplete(st_Session* SendCompleteSession);

	//-----------------------------------------------------------------
	// 클라 접속시 Accept 호출하고 접속한 클라를 대상으로 호출하는 함수
	//-----------------------------------------------------------------
	virtual void OnClientJoin(__int64 SessionID) = 0;

	//-----------------------------------------------------------
	// RecvNotifyComplete 안에서 호출하는 함수로
	// 해당 세션으로 온 메세지를 OnRecv를 통해 전달한다.
	//-----------------------------------------------------------
	virtual void OnRecv(__int64 SessionID, CMessage* Packet) = 0;

	//-------------------------------------------------
	// ReleaseSession 안에서 호출 
	// 반납되는 세션을 기준으로 IOCount가 0이 될때 호출
	//-------------------------------------------------
	virtual void OnClientLeave(st_Session *LeaveSession) = 0;

	virtual bool OnConnectionRequest(const wchar_t ClientIP, int Port) = 0;

protected:
	//--------------------------------------------
	// 세션 포인터 배열
	//--------------------------------------------
	st_Session* _SessionArray[SERVER_SESSION_MAX];

	//------------------------------------------------
	// 세션 포인트 배열 인덱스를 스택형식으로 보관해둠
	//------------------------------------------------	
	CLockFreeStack<int32> _SessionArrayIndexs;

	//-----------------------------------------
	// 세션ID를 통해 세션 인덱스를 찾고 배열에서 세션을 가져온다.
	//-----------------------------------------
	st_Session* FindSession(__int64 SessionID);

	//---------------------------------------------------------------------------------------------------------------
	// 증가시켜준 IOCount를 감소시켜주는 함수 ( 사용한다는 의미인 IOCount를 1 줄여줌으로써 반납 한다는 의미를 갖는다)
	//---------------------------------------------------------------------------------------------------------------
	void ReturnSession(st_Session* Session);
public:
	//-------------------------
	// 연결되어 있는 세션 개수
	//-------------------------
	LONG64 _SessionCount;

	//--------------------------
	// 서버 열리고 Accept한 횟수
	//--------------------------
	__int64 _AcceptTotal;

	//--------------------
	// 1초당 Aceppt한 횟수
	//--------------------
	int _AcceptTPS;

	//------------------------
	// 1초당 Recv, Send한 횟수
	//------------------------
	LONG _RecvPacketTPS;
	LONG _SendPacketTPS;

	CNetworkLib();

	//------------------------------------------------------------
	// 상속받은 클래스들의 소멸자를 호출하기 위해 virtual을 붙여줌
	//------------------------------------------------------------
	virtual ~CNetworkLib();

	void SendPacket(__int64 SessionID, CMessage* Packet);
	void Disconnect(__int64 SessionID);

	//---------------------------------------
	//서버 시작 함수
	//---------------------------------------
	bool Start(const WCHAR* OpenIP, int Port);

	//----------------
	//서버 멈춤 함수
	//----------------
	void Stop();
};