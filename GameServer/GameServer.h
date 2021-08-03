#pragma once
#pragma comment(lib,"winmm")
#include "NetworkLib.h"
#include "CommonProtocol.h"
#include "GameServerInfo.h"
#include "MemoryPoolTLS.h"

class CGameServer : public CNetworkLib
{
private:
	//---------------------------------------------------
	//섹터 리스트 배열
	//---------------------------------------------------
	list<int64> _SectorList[SECTOR_Y_MAX][SECTOR_X_MAX];

	HANDLE _UpdateThread;
	HANDLE _DataBaseThread;

	HANDLE _UpdateWakeEvent;
	HANDLE _DataBaseWakeEvent;

	// WorkerThread 종료용 변수
	bool _UpdateThreadEnd;
	// DataBaseThread 종료용 변수
	bool _DataBaseThreadEnd;

	// 게임서버에서 생성되는 오브젝트들의 아이디
	int64 _GameObjectId;

	static unsigned __stdcall UpdateThreadProc(void* Argument);
	static unsigned __stdcall DataBaseThreadProc(void* Argument);
	static unsigned __stdcall HeartBeatCheckThreadProc(void* Argument);

	void CreateNewClient(int64 SessionID);
	void DeleteClient(int64 SessionID);

	void PacketProc(int64 SessionID, CMessage* Message);
	
	//----------------------------------------------------------------
	//패킷처리 함수
	//1. 로그인 요청
	//2. 캐릭터 생성 요청 
	//3. 섹터 이동 요청
	//4. 채팅 보내기 요청
	//5. 하트비트
	//----------------------------------------------------------------	
	void PacketProcReqLogin(int64 SessionID, CMessage* Message);
	void PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message);
	void PacketProcReqSectorMove(int64 SessionID, CMessage* Message);
	void PacketProcReqMessage(int64 SessionID, CMessage* Message);
	void PacketProcReqHeartBeat(int64 SessionID, CMessage* Message);

	//----------------------------------------------------------------
	// DB 요청 처리 함수
	//----------------------------------------------------------------
	void PacketProcReqAccountCheck(int64 SessionID, CMessage* Message);
	void PacketProcReqCreateCharacterNameCheck(int64 SessionID, CMessage* Message);

	//----------------------------------------------------------------
	//패킷조합 함수
	//1. 클라이언트 접속 응답
	//2. 로그인 요청 응답
	//3. 섹터 이동 요청 응답
	//4. 채팅 보내기 요청 응답
	//----------------------------------------------------------------
	CMessage* MakePacketResClientConnected();
	CMessage* MakePacketResLogin(bool Status, int32 PlayerCount, st_PlayerObjectInfo* Players);
	CMessage* MakePacketResCreateCharacter(int32 PlayerDBID, wstring PlayerName);
	CMessage* MakePacketResSectorMove(int64 AccountNo, WORD SectorX, WORD SectorY);
	CMessage* MakePacketResMessage(int64 AccountNo, WCHAR* ID, WCHAR* NickName, WORD MessageLen, WCHAR* Message);

	st_CLIENT* FindClient(int64 SessionID);

	void GetSectorAround(int16 SectorX, int16 SectorY, st_SECTOR_AROUND* SectorAround);
public:
	//------------------------------------
	// Job 메모리풀
	//------------------------------------
	CMemoryPoolTLS<st_JOB>* _JobMemoryPool;
	//------------------------------------
	//접속한 클라이언트를 관리할 메모리풀
	//------------------------------------
	CMemoryPoolTLS<st_CLIENT>* _ClientMemoryPool;
	//------------------------------------
	// Job 큐
	//------------------------------------
	CLockFreeQue<st_JOB*> _GameServerCommonMessageQue;
	CLockFreeQue<st_JOB*> _GameServerDataBaseMessageQue;

	// 채팅서버 접속한 클라
	unordered_map<int64, st_CLIENT*> _ClientMap;

	int64 _UpdateWakeCount; // Update 쓰레드가 일어난 횟수	
	int64 _UpdateTPS; // Update 쓰레드가 1초에 작업한 처리량

	int64 _DataBaseThreadWakeCount;
	int64 _DataBaseThreadTPS;

	CGameServer();
	~CGameServer();

	void Start(const WCHAR* OpenIP, int32 Port);

	virtual void OnClientJoin(int64 SessionID) override;
	virtual void OnRecv(int64 SessionID, CMessage* Packet) override;
	virtual void OnClientLeave(int64 SessionID) override;
	virtual bool OnConnectionRequest(const wchar_t ClientIP, int32 Port) override;

	//------------------------------------------------------------------------------
	//자신 주위 8섹터들에게 메세지를 전달한다.
	//SendMe = false 나에게는 보내지 않는다.
	//SendMe = true 나에게도 보낸다.
	//------------------------------------------------------------------------------
	void SendPacketAround(st_CLIENT* Client, CMessage* Message, bool SendMe = false);
	void SendPacketBroadcast(CMessage* Message);
};