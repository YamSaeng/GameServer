#pragma once
#pragma comment(lib,"winmm")
#include "NetworkLib.h"
#include "CommonProtocol.h"
#include "GameServerInfo.h"
#include "MemoryPoolTLS.h"
#include "Heap.h"

class CGameServerMessage;

class CGameServer : public CNetworkLib
{
private:
	// 인증 쓰레드
	HANDLE _AuthThread;
	// 네트워크 쓰레드
	HANDLE _NetworkThread;
	// 데이터베이스 쓰레드
	HANDLE _DataBaseThread;
	// 타이머잡 쓰레드
	HANDLE _TimerJobThread;
	// 로직 쓰레드
	HANDLE _LogicThread;

	// 인증 쓰레드 깨우기 이벤트
	HANDLE _AuthThreadWakeEvent;
	// 네트워크 쓰레드 깨우기 이벤트
	HANDLE _NetworkThreadWakeEvent;	
	// 타이머잡 쓰레드 깨우기 이벤트
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

	//-------------------------------------------------------
	// 인증 쓰레드 ( 클라 접속, 클라 비접속 )
	//-------------------------------------------------------
	static unsigned __stdcall AuthThreadProc(void* Argument);
	static unsigned __stdcall NetworkThreadProc(void* Argument);
	static unsigned __stdcall DataBaseThreadProc(void* Argument);
	static unsigned __stdcall TimerJobThreadProc(void* Argument);
	static unsigned __stdcall LogicThreadProc(void* Argument);
	static unsigned __stdcall HeartBeatCheckThreadProc(void* Argument);

	//---------------------------------
	// 캐릭터 기본 스킬 생성
	//---------------------------------
	void NewPlayerDefaultSkillCreate(st_Session* Session,int8& CharacterCreateSlotIndex);
	
	//------------------------
	// 오브젝트 자연 회복 추가
	//------------------------
	void ObjectAutoRecovery(int16 ObjectId, en_GameObjectType GameObjectType, int16 Level);

	//------------------------------------
	// 클라 접속 기본 정보 셋팅
	//------------------------------------
	void CreateNewClient(int64 SessionId);
	//------------------------------------
	// 클라 삭제 
	//------------------------------------
	void DeleteClient(st_Session* Session);

	//--------------------------------------------------
	// 네트워크 패킷 처리
	//--------------------------------------------------
	void PacketProc(int64 SessionId, CMessage* Message);

	//----------------------------------------------------------------
	// 네트워크 패킷처리 함수
	//----------------------------------------------------------------
		
	//----------------------------------------------------------
	// 로그인 요청 처리
	//----------------------------------------------------------
	void PacketProcReqLogin(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------------
	// 캐릭터 생성 요청 처리
	//-------------------------------------------------------------------
	void PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------
	// 게임 입장 요청 처리
	//-------------------------------------------------------------
	void PacketProcReqEnterGame(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------
	// 이동 요청 처리
	//--------------------------------------------------------
	void PacketProcReqMove(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------------
	// 공격 요청 처리
	//---------------------------------------------------------------
	void PacketProcReqMelee(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------
	// 마법 요청 처리
	//---------------------------------------------------------
	void PacketProcReqMagic(int64 SessionId, CMessage* Message);
	//---------------------------------------------------------
	// 마법 요청 취소 처리
	//---------------------------------------------------------
	void PacketProcReqMagicCancel(int64 SessionId, CMessage* Message);
	//----------------------------------------------------------------------------
	// 마우스 위치 오브젝트 정보 요청 처리
	//----------------------------------------------------------------------------
	void PacketProcReqMousePositionObjectInfo(int64 SessionId, CMessage* Message);
	//----------------------------------------------------------------------
	// 오브젝트 상태 변경 요청 처리
	//----------------------------------------------------------------------
	void PacketProcReqObjectStateChange(int64 SessionId, CMessage* Message);
	//--------------------------------------------------------------------
	// 채팅 메세지 요청 처리
	//--------------------------------------------------------------------
	void PacketProcReqChattingMessage(int64 SessionId, CMessage* Message);	
	//------------------------------------------------------------
	// 아이템 인벤토리 스왑 요청 처리
	//------------------------------------------------------------
	void PacketProcReqItemSwap(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// 퀵슬롯 저장 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotSave(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// 퀵슬롯 스왑 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotSwap(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// 퀵슬롯 초기화 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotInit(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// 제작템 제작 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqCraftingConfirm(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// 아이템 사용 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqItemUse(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// 아이템 줍기 요청 처리
	//------------------------------------------------------------------
	void PacketProcReqItemLooting(int64 SessionId, CMessage* Message);
		
	void PacketProcReqHeartBeat(int64 SessionID, CMessage* Message);

	//-----------------------------------------------------------------------------------------------
	// DB 요청 처리 함수
	//-----------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------
	// AccountID가 AccountDB에 있는지 체크
	//-------------------------------------------------------------------
	void PacketProcReqDBAccountCheck(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------------------------
	// 생성요청한 캐릭터가 DB에 있는지 확인 후 캐릭터 생성
	//-------------------------------------------------------------------------------
	void PacketProcReqDBCreateCharacterNameCheck(int64 SessionID, CMessage* Message);
	//-------------------------------------------------
	// 아이템 생성 후 DB 저장
	//-------------------------------------------------
	void PacketProcReqDBItemCreate(CMessage* Message);
	//------------------------------------------------------------------
	// 인벤토리 테이블에 루팅한 아이템 저장
	//------------------------------------------------------------------
	void PacketProcReqDBLootingItemToInventorySave(int64 SessionId, CGameServerMessage* Message);
	//------------------------------------------------------------------
	// 인벤토리 테이블에 제작템 저장
	//------------------------------------------------------------------
	void PacketProcReqDBCraftingItemToInventorySave(int64 SessionId, CGameServerMessage* Message);
	//---------------------------------------------------------------
	// 인벤토리 테이블에 아이템 스왑
	//---------------------------------------------------------------
	void PacketProcReqDBItemSwap(int64 SessionId, CMessage* Message);
	//---------------------------------------------------------------
	// 인벤토리 테이블에 아이템 업데이트
	//---------------------------------------------------------------
	void PacketProcReqDBItemUpdate(int64 SessionId, CGameServerMessage* Message);
	//---------------------------------------------------------------
	// 인벤토리에 돈 저장
	//---------------------------------------------------------------
	void PacketProcReqDBGoldSave(int64 SessionId, CMessage* Message);
	//-----------------------------------------------------------------------
	// 게임 서버 접속시 캐릭터 정보를 DB에서 가져와서 클라에 전송
	//-----------------------------------------------------------------------
	void PacketProcReqDBCharacterInfoSend(int64 SessionId, CMessage* Message);
	//--------------------------------------------------------------------------
	// 퀵슬롯 테이블에 요청 스킬 저장
	//--------------------------------------------------------------------------
	void PacketProcReqDBQuickSlotBarSlotSave(int64 SessionId, CGameServerMessage* Message);
	//--------------------------------------------------------------------
	// 퀵슬롯바에서 퀵슬롯 스왑
	//--------------------------------------------------------------------
	void PacketProcReqDBQuickSlotSwap(int64 SessionId, CMessage* Message);
	//--------------------------------------------------------------------
	// 퀵슬롯바 초기화
	//--------------------------------------------------------------------
	void PacketProcReqDBQuickSlotInit(int64 SessionId, CMessage* Message);
	//--------------------------------------------------------------------
	// 접속 종료시 플레이어 정보 DB에 기록
	//--------------------------------------------------------------------
	void PacketProcReqDBLeavePlayerInfoSave(int64 SessionId, CMessage* Message);

	//------------------------------------------------------------------
	// 타이머 잡 요청 처리 함수
	//------------------------------------------------------------------

	//----------------------------------------------------------------
	// 밀리공격 끝 처리 함수
	//----------------------------------------------------------------
	void PacketProcTimerAttackEnd(int64 SessionId, CGameServerMessage* Message);
	//----------------------------------------------------------------
	// 마법공격 끝 처리 함수
	//----------------------------------------------------------------
	void PacketProcTimerSpellEnd(int64 SessionId, CGameServerMessage* Message);
	//----------------------------------------------------------------
	// 재사용대기 시간 끝 처리 함수
	//----------------------------------------------------------------
	void PacketProcTimerCastingTimeEnd(int64 SessionId, CGameServerMessage* Message);
	//----------------------------------------------------------------
	// 오브젝트 스폰
	//----------------------------------------------------------------
	void PacketProcTimerObjectSpawn(CGameServerMessage* Message);
	//----------------------------------------------------------------
	// 오브젝트 상태 변경
	//----------------------------------------------------------------
	void PacketProcTimerObjectStateChange(int64 SessionId, CGameServerMessage* Message);
	//----------------------------------------------------------------
	// 오브젝트 도트 처리
	//----------------------------------------------------------------
	void PacketProcTimerDot(int64 SessionId, CGameServerMessage* Message);

	//--------------------------------------
	// 패킷조합 함수		
	//--------------------------------------
	
	//--------------------------------------
	// 클라이언트 접속 응답 패킷 조합
	//--------------------------------------
	CGameServerMessage* MakePacketResClientConnected();
	//---------------------------------------------------------------------------------------
	// 로그인 요청 응답 패킷 조합
	//---------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResLogin(bool Status, int8 PlayerCount, CPlayer** MyPlayersInfo);
	//--------------------------------------------------------------------------------------------------
	// 캐릭터 생성 요청 응답 패킷 조합
	//--------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCreateCharacter(bool IsSuccess, st_GameObjectInfo CreateCharacterObjectInfo);
	//-------------------------------------------------------------
	// 게임서버 입장 요청 응답 패킷 조합
	//-------------------------------------------------------------
	CGameServerMessage* MakePacketResEnterGame(st_GameObjectInfo ObjectInfo);
	//-------------------------------------------------------------------------------------------------------------------------
	// 게임서버 마우스 위치 오브젝트 정보 요청 응답 패킷 조합
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMousePositionObjectInfo(int64 AccountId, int64 PreviousChoiceObjectId, st_GameObjectInfo ObjectInfo);
	//-------------------------------------------------------------------------------------------------------------------------
	// 게임서버 돈 저장 요청 응답 패킷 조합
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResGoldSave(int64 AccountId, int64 ObjectId, int64 GoldCount, int16 SliverCount, int16 BronzeCount, int16 ItemCount, int16 ItemType, bool ItemGainPrint = true);
	//-------------------------------------------------------------------------------------------------------------------------
	// 게임서버 인벤토리 생성 패킷 조합
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketInventoryCreate(int8 InventorySize, vector<CItem*> InventoryItems);
	//-------------------------------------------------------------------------------------------------------------------------
	// 게임서버 아이템 스왑 요청 응답 패킷 조합
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResItemSwap(int64 AccountId, int64 ObjectId, st_ItemInfo SwapAItemInfo, st_ItemInfo SwapBItemInfo);
	//-------------------------------------------------------------------------------------------------------------------------
	// 게임서버 아이템 인벤토리 저장 요청 응답 패킷 조합
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResItemToInventory(int64 TargetObjectId, CItem* InventoryItem, int16 ItemEach, bool ItemGainPrint = true);
	//-----------------------------------------------------------------------------------------
	// 게임서버 인벤토리 아이템 업데이트
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketInventoryItemUpdate(int64 PlayerId, st_ItemInfo UpdateItemInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 인벤토리 아이템 사용 요청 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketInventoryItemUse(int64 PlayerId, st_ItemInfo& UseItemInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 장비 착용 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketEquipmentUpdate(int64 PlayerId, st_ItemInfo& EquipmentItemInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 퀵슬롯 등록 요청 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResQuickSlotBarSlotSave(st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 퀵슬롯 생성 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketQuickSlotCreate(int8 QuickSlotBarSize, int8 QuickSlotBarSlotSize, vector<st_QuickSlotBarSlotInfo> QuickslotBarSlotInfos);
	//-----------------------------------------------------------------------------------------
	// 게임서버 퀵슬롯 스왑 요청 응답 패킷 조합 
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResQuickSlotSwap(int64 AccountId, int64 PlayerId, st_QuickSlotBarSlotInfo SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo SwapBQuickSlotInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 퀵슬롯 초기화 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResQuickSlotInit(int64 AccountId, int64 PlayerId, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, int16 QuickSlotKey);
	//-----------------------------------------------------------------------------------------
	// 게임서버 에러 메세지 생성 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketError(int64 PlayerId, en_ErrorType ErrorType, wstring ErrorMessage);
	//-----------------------------------------------------------------------------------------
	// 게임서버 재사용대기시간 출력 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketCoolTime(int64 PlayerId, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, float SkillCoolTime, float SkillCoolTimeSpeed);
	//-----------------------------------------------------------------------------------------
	// 게임서버 제작템 목록 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketCraftingList(int64 AccountId, int64 PlayerId, vector<st_CraftingItemCategory> CraftingItemList);	
	//-----------------------------------------------------------------------------------------
	// 게임서버 스킬 취소 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketMagicCancel(int64 AccountId, int64 PlayerId);
	//-----------------------------------------------------------------------------------------
	// 게임서버 경험치 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketExperience(int64 AccountId, int64 PlayerId, int64 GainExp, int64 CurrentExp, int64 RequireExp, int64 TotalExp);
public:
	//-----------------------------------------------------------------------------------------
	// 게임서버 공격요청 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResAttack(int64 PlayerDBId, int64 TargetId, en_SkillType SkillType, int32 Damage, bool IsCritical);
	//-----------------------------------------------------------------------------------------
	// 게임서버 마법요청 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMagic(int64 ObjectId, bool SpellStart, en_SkillType SkillType = en_SkillType::SKILL_TYPE_NONE, float SpellTime = 0.0f);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 스탯 변경 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChangeObjectStat(int64 ObjectId, st_StatInfo ChangeObjectStatInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 상태 변경 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChangeObjectState(int64 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState);
	//-----------------------------------------------------------------------------------------
	// 게임서버 이동 요청 응답 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMove(int64 AccountId, int64 ObjectId, en_GameObjectType ObjectType, st_PositionInfo PositionInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 정찰 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketPatrol(int64 ObjectId, en_GameObjectType ObjectType, st_PositionInfo PositionInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 스폰 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResObjectSpawn(int32 ObjectInfosCount, vector<st_GameObjectInfo> ObjectInfos);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 디스폰 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResObjectDeSpawn(int32 DeSpawnObjectCount, vector<int64> DeSpawnObjectIds);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 죽음 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketObjectDie(int64 DieObjectId);
	//-----------------------------------------------------------------------------------------
	// 게임서버 메세지 ( 채팅, 시스템 ) 요청 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChattingBoxMessage(int64 PlayerDBId, en_MessageType MessageType, st_Color Color, wstring ChattingMessage);
	//-----------------------------------------------------------------------------------------
	// 게임서버 오브젝트 위치 싱크 맞추기 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSyncPosition(int64 TargetObjectId, st_PositionInfo SyncPosition);
	//-----------------------------------------------------------------------------------------
	// 게임서버 스킬 저장 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSkillToSkillBox(int64 TargetObjectId, st_SkillInfo* SkillInfo);
	//-----------------------------------------------------------------------------------------
	// 게임서버 이펙트 출력 패킷 조합
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketEffect(int64 TargetObjectId, en_EffectType EffectType, float PrintEffectTime);
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
	// 네트워크 쓰레드 활성화된 횟수
	int64 _NetworkThreadWakeCount;
	// 네트워크 쓰레드 TPS
	int64 _NetworkThreadTPS; 

	// DB 쓰레드 깨우기 이벤트
	HANDLE _DataBaseWakeEvent;
	// DB 쓰레드 활성화된 횟수
	int64 _DataBaseThreadWakeCount;
	// DB 쓰레드 TPS
	int64 _DataBaseThreadTPS;	
	
	// 타이머 잡 쓰레드 활성화된 횟수
	int64 _TimerJobThreadWakeCount;
	// 타이머 잡 쓰레드 TPS
	int64 _TimerJobThreadTPS;

	CGameServer();
	~CGameServer();
	
	//------------------------------------------
	// 게임 서버 시작
	//------------------------------------------
	void GameServerStart(const WCHAR* OpenIP, int32 Port);

	//--------------------------------------------------
	// 새로운 클라이언트 접속 
	//--------------------------------------------------
	virtual void OnClientJoin(int64 SessionID) override;
	//--------------------------------------------------------------
	// 네트워크 패킷 처리
	//--------------------------------------------------------------
	virtual void OnRecv(int64 SessionID, CMessage* Packet) override;
	//--------------------------------------------------------------
	// 클라이언트 떠남
	//--------------------------------------------------------------
	virtual void OnClientLeave(st_Session* LeaveSession) override;
	virtual bool OnConnectionRequest(const wchar_t ClientIP, int32 Port) override;
		
	//--------------------------------------------------------------
	// 위치값을 기준으로 메세지 주위 섹터에 전송
	//--------------------------------------------------------------
	void SendPacketAroundSector(st_Vector2Int CellPosition, CMessage* Message);
	//--------------------------------------------------------------
	// Session을 기준으로 주위 섹터에 전송
	//--------------------------------------------------------------
	void SendPacketAroundSector(st_Session* Session, CMessage* Message, bool SendMe = false);
	
	//--------------------------------------------------------------
	// 스킬 쿨타임 타이머 잡 생성
	//--------------------------------------------------------------
	void SkillCoolTimeTimerJobCreate(CPlayer* Player, int64 CastingTime, st_SkillInfo* CoolTimeSkillInfo, en_TimerJobType TimerJobType, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex);
	//--------------------------------------------------------------
	// 오브젝트 스폰 타이머 잡 생성
	//--------------------------------------------------------------
	void SpawnObjectTimeTimerJobCreate(int16 SpawnObjectType, st_Vector2Int SpawnPosition, int64 SpawnTime);
	//--------------------------------------------------------------
	// 오브젝트 상태 변경 타이머 잡 생성
	//--------------------------------------------------------------
	void ObjectStateChangeTimerJobCreate(CGameObject* Target, en_CreatureState ChangeState, int64 ChangeTime, int64 SessionId = 0);
	//--------------------------------------------------------------
	// 오브젝트 도트 타이머 잡 생성
	//--------------------------------------------------------------
	void ObjectDotTimerCreate(CGameObject* Target, en_DotType DotType, int64 DotTime, int32 HPPoint, int32 MPPoint, int64 DotTotalTime = 0, int64 SessionId = 0);
};