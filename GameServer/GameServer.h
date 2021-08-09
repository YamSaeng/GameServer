#pragma once
#pragma comment(lib,"winmm")
#include "NetworkLib.h"
#include "CommonProtocol.h"
#include "GameServerInfo.h"
#include "MemoryPoolTLS.h"

// 기본 메세지 처리 쓰레드와
// DB 질의 처리 쓰레드가 2개 존재
// 따라서 기본 메세지 처리에서 st_Client가 사라지면
// DB 질의 처리 쓰레드에서 사라진 st_Client를 대상으로 DB처리 해줄 수 있다.
// 처리 고민 해야함

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
	//3. 게임 입장 요청
	//4. 움직임 요청
	//5. 공격 요청
	//6. 하트비트
	//----------------------------------------------------------------	
	void PacketProcReqLogin(int64 SessionID, CMessage* Message);
	void PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message);
	void PacketProcReqEnterGame(int64 SessionID, CMessage* Message);
	void PacketProcReqMove(int64 SessionID, CMessage* Message);	
	void PacketProcReqAttack(int64 SessionID, CMessage* Message);
	void PacketProcReqSectorMove(int64 SessionID, CMessage* Message);
	void PacketProcReqMessage(int64 SessionID, CMessage* Message);
	void PacketProcReqHeartBeat(int64 SessionID, CMessage* Message);

	//-----------------------------------------------------------------------------------------------
	// DB 요청 처리 함수
	// 1. 로그인 요청에서 추가로 AccountServer에 입력받은 Account가 있는지 확인하고 최종 로그인 검사	
	// 2. 캐릭터 생성 요청에서 추가로 DB에 입력한 해당 캐릭터가 있는지 확인
	//-----------------------------------------------------------------------------------------------
	void PacketProcReqAccountCheck(int64 SessionID, CMessage* Message);
	void PacketProcReqCreateCharacterNameCheck(int64 SessionID, CMessage* Message);

	//----------------------------------------------------------------
	//패킷조합 함수
	//1. 클라이언트 접속 응답
	//2. 로그인 요청 응답
	//3. 게임 입장 요청 응답
	//4. 움직임 요청 응답
	//5. 공격 요청 응답
	//6. 오브젝트 스폰 
	//----------------------------------------------------------------
	CMessage* MakePacketResClientConnected();
	CMessage* MakePacketResLogin(bool Status, int32 PlayerCount, int32 PlayerDBId, wstring PlayersName);
	CMessage* MakePacketResCreateCharacter(bool IsSuccess, int32 PlayerDBId, wstring PlayerName);
	CMessage* MakePacketResEnterGame(int64 AccountId, int32 PlayerDBId, wstring EnterPlayerName, st_GameObjectInfo ObjectInfo);
	CMessage* MakePacketResMove(int64 AccountId, int32 PlayerDBId,bool Cango,st_PositionInfo PositionInfo);
	CMessage* MakePacketResAttack(int64 AccountId, int32 PlayerDBId, en_MoveDir Dir);
	CMessage* MakePacketResSpawn(int64 AccountId, int32 PlayerDBId, int32 ObjectInfosCount, wstring* SpawnObjectName, st_GameObjectInfo* ObjectInfos);
	CMessage* MakePacketResDeSpawn(int64 AccountId, int32 PlayerDBId);
	CMessage* MakePacketResMessage(int64 AccountNo, WCHAR* ID, WCHAR* NickName, WORD MessageLen, WCHAR* Message);

	st_CLIENT* FindClient(int64 SessionID);
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

	void SendPacketSector(st_CLIENT* Client, CMessage* Message, bool SendMe = false);

	//------------------------------------------------------------------------------
	//자신 주위 8섹터들에게 메세지를 전달한다.
	//SendMe = false 나에게는 보내지 않는다.
	//SendMe = true 나에게도 보낸다.
	//------------------------------------------------------------------------------
	void SendPacketBroadcast(CMessage* Message);
};