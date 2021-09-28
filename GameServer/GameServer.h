#pragma once
#pragma comment(lib,"winmm")
#include "NetworkLib.h"
#include "CommonProtocol.h"
#include "GameServerInfo.h"
#include "MemoryPoolTLS.h"
#include "Heap.h"

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
	HANDLE _TimerJobThread;
	HANDLE _LogicThread;

	HANDLE _AuthThreadWakeEvent;
	HANDLE _NetworkThreadWakeEvent;	
	HANDLE _TimerThreadWakeEvent;	

	// AuthThread 종료용 변수
	bool _AuthThreadEnd;
	// WorkerThread 종료용 변수
	bool _NetworkThreadEnd;
	// DataBaseThread 종료용 변수
	bool _DataBaseThreadEnd;
	// LogicThread 종료용 변수
	bool _LogicThreadEnd;
	// TimerJobThread 종료용 변수
	bool _TimerJobThreadEnd;

	// TimerJobThread 전용 Lock
	SRWLOCK _TimerJobLock;

	static unsigned __stdcall AuthThreadProc(void* Argument);
	static unsigned __stdcall NetworkThreadProc(void* Argument);
	static unsigned __stdcall DataBaseThreadProc(void* Argument);
	static unsigned __stdcall TimerJobThreadProc(void* Argument);
	static unsigned __stdcall LogicThreadProc(void* Argument);
	static unsigned __stdcall HeartBeatCheckThreadProc(void* Argument);

	void CreateNewClient(int64 SessionId);
	void DeleteClient(st_Session* Session);

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
	void PacketProcReqMeleeAttack(int64 SessionID, CMessage* Message);
	void PacketProcReqMagic(int64 SessionId, CMessage* Message);
	void PacketProcReqMousePositionObjectInfo(int64 SessionID, CMessage* Message);
	void PacketProcReqObjectStateChange(int64 SessionId, CMessage* Message);
	void PacketProcReqChattingMessage(int64 SessionId, CMessage* Message);
	void PacketProcReqItemToInventory(int64 SessionId, CMessage* Message);
	void PacketProcReqItemSwap(int64 SessionId, CMessage* Message);
	void PacketProcReqQuickSlotSave(int64 SessionId, CMessage* Message);
	void PacketProcReqQuickSlotSwap(int64 SessionId, CMessage* Message);
	void PacketProcReqSectorMove(int64 SessionID, CMessage* Message);
	void PacketProcReqMessage(int64 SessionID, CMessage* Message);
	void PacketProcReqHeartBeat(int64 SessionID, CMessage* Message);

	//-----------------------------------------------------------------------------------------------
	// DB 요청 처리 함수
	// 1. 로그인 요청에서 추가로 AccountServer에 입력받은 Account가 있는지 확인하고 최종 로그인 검사	
	// 2. 캐릭터 생성 요청에서 추가로 DB에 입력한 해당 캐릭터가 있는지 확인
	// 3. 아이템 생성
	// 4. 캐릭터 인벤토리에 있는 Item을 DB에 저장
	// 5. 캐릭터 인벤토리에서 아이템 Swap
	// 6. 캐릭터 인벤토리에 있는 Gold를 DB에 저장
	// 7. 캐릭터 정보 클라에게 전송
	//-----------------------------------------------------------------------------------------------
	void PacketProcReqDBAccountCheck(int64 SessionID, CMessage* Message);
	void PacketProcReqDBCreateCharacterNameCheck(int64 SessionID, CMessage* Message);
	void PacketProcReqDBItemCreate(CMessage* Message);
	void PacketProcReqDBItemToInventorySave(int64 SessionId, CMessage* Message);
	void PacketProcReqDBItemSwap(int64 SessionId, CMessage* Message);
	void PacketProcReqDBGoldSave(int64 SessionId, CMessage* Message);
	void PacketProcReqDBCharacterInfoSend(int64 SessionId, CMessage* Message);
	void PacketProcReqDBQuickSlotBarSlotSave(int64 SessionId, CMessage* Message);
	void PacketProcReqDBQuickSlotSwap(int64 SessionId, CMessage* Message);


	void PacketProcTimerAttackEnd(int64 SessionId, CMessage* Message);
	void PacketProcTimerSpellEnd(int64 SessionId, CMessage* Message);
	void PacketProcTimerCoolTimeEnd(int64 SessionId, CMessage* Message);

	//----------------------------------------------------------------
	//패킷조합 함수
	//1. 클라이언트 접속 응답
	//2. 로그인 요청 응답
	//3. 캐릭터 생성 요청 응답
	//4. 게임 입장 요청 응답
	//5. 마우스 위치 오브젝트 정보 요청 응답
	//6. 골드 저장 요청 응답 
	//7. 아이템 스왑 요청 응답
	//8. 공격 요청 응답 
	//9. 마법 공격 요청 응답
	//10. HP 변경 요청 응답
	//11. 오브젝트 상태 변경 요청 응답
	//12. 이동 요청 응답
	//13. 오브젝트 스폰 요청 응답
	//14. 오브젝트 디스폰 요청 응답	
	//15. 오브젝트 죽음 응답
	//16. 채팅 요청 응답
	//17. 아이템 저장 요청 응답
	//18. 오브젝트 위치 조정
	//----------------------------------------------------------------
	CMessage* MakePacketResClientConnected();
	CMessage* MakePacketResLogin(bool Status, int8 PlayerCount, CGameObject** MyPlayersInfo);
	CMessage* MakePacketResCreateCharacter(bool IsSuccess, st_GameObjectInfo CreateCharacterObjectInfo);
	CMessage* MakePacketResEnterGame(st_GameObjectInfo ObjectInfo);	
	CMessage* MakePacketResMousePositionObjectInfo(int64 AccountId, int64 PreviousChoiceObjectId, st_GameObjectInfo ObjectInfo);
	CMessage* MakePacketGoldSave(int64 AccountId, int64 ObjectId, int64 GoldCount, int16 SliverCount, int16 BronzeCount, int16 ItemCount, int16 ItemType, bool ItemGainPrint = true);
	CMessage* MakePacketResMessage(int64 AccountNo, WCHAR* ID, WCHAR* NickName, WORD MessageLen, WCHAR* Message);
	CMessage* MakePacketResItemSwap(int64 AccountId, int64 ObjectId, st_ItemInfo SwapAItemInfo, st_ItemInfo SwapBItemInfo);
	CMessage* MakePacketResQuickSlotBarSlotUpdate(st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo);
	CMessage* MakePacketQuickSlotCreate(int8 QuickSlotBarSize, int8 QuickSlotBarSlotSize, vector<st_QuickSlotBarSlotInfo> QuickslotBarSlotInfos);
	CMessage* MakePacketError(int64 PlayerId, en_ErrorType ErrorType, wstring ErrorMessage);
	CMessage* MakePacketCoolTime(int64 PlayerId, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, float SkillCoolTime, float SkillCoolTimeSpeed);
	CMessage* MakePacketResQuickSlotSwap(int64 AccountId, int64 PlayerId, st_QuickSlotBarSlotInfo SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo SwapBQuickSlotInfo);
public:
	CMessage* MakePacketResAttack(int64 PlayerDBId, int64 TargetId, en_SkillType SkillType, int32 Damage, bool IsCritical);
	CMessage* MakePacketResMagic(int64 ObjectId, bool SpellStart, en_SkillType SkillType = en_SkillType::SKILL_TYPE_NONE, float SpellTime = 0.0f);
	CMessage* MakePacketResChangeObjectStat(int64 ObjectId, st_StatInfo ChangeObjectStatInfo);
	CMessage* MakePacketResObjectState(int64 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState);
	CMessage* MakePacketResMove(int64 AccountId, int64 ObjectId, en_GameObjectType ObjectType, st_PositionInfo PositionInfo);
	CMessage* MakePacketResSpawn(int32 ObjectInfosCount, vector<st_GameObjectInfo> ObjectInfos);
	CMessage* MakePacketResDeSpawn(int32 DeSpawnObjectCount, vector<int64> DeSpawnObjectIds);
	CMessage* MakePacketResDie(int64 DieObjectId);
	CMessage* MakePacketResChattingMessage(int64 PlayerDBId, en_MessageType MessageType, st_Color Color, wstring ChattingMessage);		
	CMessage* MakePacketResItemToInventory(int64 TargetObjectId, st_ItemInfo ItemInfo,int16 ItemEach, bool ItemGainPrint = true);
	CMessage* MakePacketResSyncPosition(int64 TargetObjectId, st_PositionInfo SyncPosition);	
	CMessage* MakePacketResSkillToSkillBox(int64 TargetObjectId, st_SkillInfo SkillInfo);	
public:
	//------------------------------------
	// Job 메모리풀
	//------------------------------------
	CMemoryPoolTLS<st_Job>* _JobMemoryPool;

	//------------------------------------
	// TimerJob 메모리풀
	//------------------------------------
	CMemoryPoolTLS<st_TimerJob>* _TimerJobMemoryPool;
	//------------------------------------
	// Job 큐
	//------------------------------------
	CLockFreeQue<st_Job*> _GameServerAuthThreadMessageQue;
	CLockFreeQue<st_Job*> _GameServerNetworkThreadMessageQue;
	CLockFreeQue<st_Job*> _GameServerDataBaseThreadMessageQue;

	//--------------------------------------
	// TimerJob 우선순위 큐
	//--------------------------------------
	CHeap<int64,st_TimerJob*>* _TimerHeapJob;

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

	HANDLE _DataBaseWakeEvent;
	
	CGameServer();
	~CGameServer();

	void Start(const WCHAR* OpenIP, int32 Port);

	virtual void OnClientJoin(int64 SessionID) override;
	virtual void OnRecv(int64 SessionID, CMessage* Packet) override;
	virtual void OnClientLeave(st_Session* LeaveSession) override;
	virtual bool OnConnectionRequest(const wchar_t ClientIP, int32 Port) override;

	void SendPacketSector(CSector* Sector, CMessage* Message);
	void SendPacketAroundSector(st_Vector2Int CellPosition, CMessage* Message);
	void SendPacketAroundSector(st_Session* Session, CMessage* Message, bool SendMe = false);
};