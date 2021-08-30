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

	HANDLE _AuthThread;
	HANDLE _NetworkThread;
	HANDLE _DataBaseThread;
	HANDLE _GameLogicThread;

	HANDLE _AuthThreadWakeEvent;
	HANDLE _NetworkThreadWakeEvent;
	HANDLE _DataBaseWakeEvent;

	bool _AuthThreadEnd;
	// WorkerThread 종료용 변수
	bool _NetworkThreadEnd;
	// DataBaseThread 종료용 변수
	bool _DataBaseThreadEnd;

	// 게임서버에서 생성되는 오브젝트들의 아이디
	int64 _GameObjectId;

	static unsigned __stdcall AuthThreadProc(void* Argument);
	static unsigned __stdcall NetworkThreadProc(void* Argument);
	static unsigned __stdcall DataBaseThreadProc(void* Argument);
	static unsigned __stdcall HeartBeatCheckThreadProc(void* Argument);

	void CreateNewClient(int64 SessionId);
	void DeleteClient(st_SESSION* Session);

	void PacketProc(int64 SessionId, CMessage* Message);

	//----------------------------------------------------------------
	// 패킷처리 함수
	// 1. 로그인 요청
	// 2. 캐릭터 생성 요청 
	// 3. 게임 입장 요청
	// 4. 움직임 요청
	// 6. 마우스 위치 오브젝트 정보 요청
	// 7. 채팅 요청
	// 8. 아이템 인벤토리 넣기 요청
	//----------------------------------------------------------------	
	void PacketProcReqLogin(int64 SessionID, CMessage* Message);
	void PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message);
	void PacketProcReqEnterGame(int64 SessionID, CMessage* Message);
	void PacketProcReqMove(int64 SessionID, CMessage* Message);
	void PacketProcReqAttack(int64 SessionID, CMessage* Message);
	void PacketProcReqMousePositionObjectInfo(int64 SessionID, CMessage* Message);
	void PacketProcReqObjectStateChange(int64 SessionId, CMessage* Message);
	void PacketProcReqChattingMessage(int64 SessionId, CMessage* Message);
	void PacketProcReqItemToInventory(int64 SessionId, CMessage* Message);
	void PacketProcReqSectorMove(int64 SessionID, CMessage* Message);
	void PacketProcReqMessage(int64 SessionID, CMessage* Message);
	void PacketProcReqHeartBeat(int64 SessionID, CMessage* Message);

	//-----------------------------------------------------------------------------------------------
	// DB 요청 처리 함수
	// 1. 로그인 요청에서 추가로 AccountServer에 입력받은 Account가 있는지 확인하고 최종 로그인 검사	
	// 2. 캐릭터 생성 요청에서 추가로 DB에 입력한 해당 캐릭터가 있는지 확인
	// 3. 캐릭터 인벤토리에 있는 Item을 DB에 저장
	// 4. 캐릭터 인벤토리에 있는 Gold를 DB에 저장
	// 5. 캐릭터 정보 클라에게 전송
	//-----------------------------------------------------------------------------------------------
	void PacketProcReqAccountCheck(int64 SessionID, CMessage* Message);
	void PacketProcReqCreateCharacterNameCheck(int64 SessionID, CMessage* Message);
	void PacketProcReqDBItemToInventorySave(int64 SessionId, CMessage* Message);
	void PacketProcReqGoldSave(int64 SessionId, CMessage* Message);
	void PacketProcReqCharacterInfoSend(int64 SessionId, CMessage* Message);

	//----------------------------------------------------------------
	//패킷조합 함수
	//1. 클라이언트 접속 응답
	//2. 로그인 요청 응답
	//3. 캐릭터 생성 요청 응답
	//4. 게임 입장 요청 응답
	//5. 마우스 위치 오브젝트 정보 요청 응답
	//5. 공격 요청 응답
	//6. 오브젝트 스폰 
	//7. 오브젝트 디스폰
	//8. HP 변경
	//----------------------------------------------------------------
	CMessage* MakePacketResClientConnected();
	CMessage* MakePacketResLogin(bool Status, int8 PlayerCount, CGameObject** MyPlayersInfo);
	CMessage* MakePacketResCreateCharacter(bool IsSuccess, st_GameObjectInfo CreateCharacterObjectInfo);
	CMessage* MakePacketResEnterGame(st_GameObjectInfo ObjectInfo);	
	CMessage* MakePacketMousePositionObjectInfo(int64 AccountId, st_GameObjectInfo ObjectInfo);
	CMessage* MakePacketGoldSave(int64 AccountId, int64 ObjectId, int64 GoldCount, int8 SliverCount, int8 BronzeCount);
	CMessage* MakePacketResMessage(int64 AccountNo, WCHAR* ID, WCHAR* NickName, WORD MessageLen, WCHAR* Message);
public:
	CMessage* MakePacketResAttack(int64 AccountId, int64 PlayerDBId, en_MoveDir Dir, en_AttackType AttackType, bool IsCritical);
	CMessage* MakePacketResChangeHP(int64 PlayerDBId, int32 Damage, int32 CurrentHP, int32 MaxHP, bool IsCritical, int32 TargetPositionX, int32 TargetPositionY);
	CMessage* MakePacketResObjectState(int64 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState);
	CMessage* MakePacketResMove(int64 AccountId, int64 ObjectId, en_GameObjectType ObjectType, st_PositionInfo PositionInfo);
	CMessage* MakePacketResSpawn(int32 ObjectInfosCount, vector<st_GameObjectInfo> ObjectInfos);
	CMessage* MakePacketResDeSpawn(int32 DeSpawnObjectCount, vector<int64> DeSpawnObjectIds);
	CMessage* MakePacketResDie(int64 DieObjectId);
	CMessage* MakePacketResChattingMessage(int64 PlayerDBId, en_MessageType MessageType, wstring ChattingMessage);
	CMessage* MakePacketResItemToInventory(int64 TargetObjectId, st_ItemInfo ItemInfo);
public:
	//------------------------------------
	// Job 메모리풀
	//------------------------------------
	CMemoryPoolTLS<st_Job>* _JobMemoryPool;
	//------------------------------------
	// Job 큐
	//------------------------------------
	CLockFreeQue<st_Job*> _GameServerAuthThreadMessageQue;
	CLockFreeQue<st_Job*> _GameServerNetworkThreadMessageQue;
	CLockFreeQue<st_Job*> _GameServerDataBaseThreadMessageQue;

	// 인증 쓰레드 활성화된 횟수
	int64 _AuthThreadWakeCount;
	// 인증 쓰레드 TPS
	int64 _AuthThreadTPS;
	// Network 쓰레드 TPS
	int64 _NetworkThreadTPS; 

	// DB 쓰레드 활성화된 횟수
	int64 _DataBaseThreadWakeCount;
	// DB 쓰레드 TPS
	int64 _DataBaseThreadTPS;

	CGameServer();
	~CGameServer();

	void Start(const WCHAR* OpenIP, int32 Port);

	virtual void OnClientJoin(int64 SessionID) override;
	virtual void OnRecv(int64 SessionID, CMessage* Packet) override;
	virtual void OnClientLeave(st_SESSION* LeaveSession) override;
	virtual bool OnConnectionRequest(const wchar_t ClientIP, int32 Port) override;

	void SendPacketSector(CSector* Sector, CMessage* Message);
	void SendPacketAroundSector(st_Vector2Int CellPosition, CMessage* Message);
	void SendPacketAroundSector(st_SESSION* Session, CMessage* Message, bool SendMe = false);
};