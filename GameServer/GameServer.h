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
	// �α��� ������ ����� ����
	SOCKET _LoginServerSock;
	// ���� �����ͺ��̽� ������
	HANDLE _UserDataBaseThread;
	// ���� �����ͺ��̽� ������
	HANDLE _WorldDataBaseThread;
	// Ÿ�̸��� ������
	HANDLE _TimerJobThread;
	// ���� ������
	HANDLE _LogicThread;	

	// Ÿ�̸��� ������ ����� �̺�Ʈ
	HANDLE _TimerThreadWakeEvent;
	
	// WorkerThread ����� ����
	bool _NetworkThreadEnd;
	// User DataBaseThread ����� ����
	bool _UserDataBaseThreadEnd;
	// World DataBaseThread ����� ����
	bool _WorldDataBaseThreadEnd;	

	// TimerJobThread ����� ����
	bool _TimerJobThreadEnd;
	// LogicThread ����� ����
	bool _LogicThreadEnd;
	
	// TimerJobThread ���� Lock
	SRWLOCK _TimerJobLock;	
			
	//----------------------------------------------------------
	// ���� �����ͺ��̽� ������ ( ���� ������ ���̽� �۾� ó�� )
	//----------------------------------------------------------
	static unsigned __stdcall UserDataBaseThreadProc(void* Argument);
	//----------------------------------------------------------
	// ���� �����ͺ��̽� ������ ( ���� ������ ���̽� �۾� ó�� )
	//----------------------------------------------------------
	static unsigned __stdcall WorldDataBaseThreadProc(void* Argument);

	//----------------------------------------------------------
	// Ÿ�̸� �� ������ ( Ÿ�̸� �� ó�� )
	//----------------------------------------------------------
	static unsigned __stdcall TimerJobThreadProc(void* Argument);
	//--------------------------------------------------------
	// ����ó�� ������ 
	//--------------------------------------------------------
	static unsigned __stdcall LogicThreadProc(void* Argument);		

	//---------------------------------
	// ĳ���� ��ų ����
	//---------------------------------
	void PlayerSkillCreate(int64& AccountId, st_GameObjectInfo& NewCharacterInfo, int8& CharacterCreateSlotIndex);	

	//------------------------------------
	// Ŭ�� ���� �⺻ ���� ����
	//------------------------------------
	void CreateNewClient(int64 SessionId);
	//------------------------------------
	// Ŭ�� ���� 
	//------------------------------------
	void DeleteClient(st_Session* Session);

	//-------------------------------------------------------
	// �α��� ������ ��Ŷ ����
	//-------------------------------------------------------
	void SendPacketToLoginServer(CGameServerMessage* Message);
	//--------------------------------------------------
	// ��Ʈ��ũ ��Ŷ ó��
	//--------------------------------------------------
	void PacketProc(int64 SessionId, CMessage* Message);

	//----------------------------------------------------------------
	// ��Ʈ��ũ ��Ŷó�� �Լ�
	//----------------------------------------------------------------
		
	//----------------------------------------------------------
	// �α��� ��û ó��
	//----------------------------------------------------------
	void PacketProcReqLogin(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------------
	// ĳ���� ���� ��û ó��
	//-------------------------------------------------------------------
	void PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------
	// ���� ���� ��û ó��
	//-------------------------------------------------------------
	void PacketProcReqEnterGame(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------
	// ĳ���� ���� ��û ó��
	//-------------------------------------------------------------
	void PacketProcReqCharacterInfo(int64 SessionID, CMessage* Message);

	//---------------------------------------------------------
	// �̵� ��û ó��
	//---------------------------------------------------------
	void PacketProcReqMove(int64 SessionID, CMessage* Message);	
	//------------------------------------------------------------
	// �̵� ���� ó��
	//------------------------------------------------------------
	void PacketProcReqMoveStop(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------------
	// ���� ��û ó��
	//---------------------------------------------------------------
	void PacketProcReqMelee(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------
	// ���� ��û ó��
	//---------------------------------------------------------
	void PacketProcReqMagic(int64 SessionId, CMessage* Message);
	//---------------------------------------------------------
	// ���� ��û ��� ó��
	//---------------------------------------------------------
	void PacketProcReqMagicCancel(int64 SessionId, CMessage* Message);
	//----------------------------------------------------------------------------
	// ���콺 ��ġ ������Ʈ ���� ��û ó��
	//----------------------------------------------------------------------------
	void PacketProcReqMousePositionObjectInfo(int64 SessionId, CMessage* Message);
	//----------------------------------------------------------------------
	// ������Ʈ ���� ���� ��û ó��
	//----------------------------------------------------------------------
	void PacketProcReqObjectStateChange(int64 SessionId, CMessage* Message);
	//--------------------------------------------------------------------
	// ä�� �޼��� ��û ó��
	//--------------------------------------------------------------------
	void PacketProcReqChattingMessage(int64 SessionId, CMessage* Message);	
	//------------------------------------------------------------
	// ������ �κ��丮 ���� ��û ó��
	//------------------------------------------------------------
	void PacketProcReqItemSelect(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------
	// ������ ���� ��û ó��
	//------------------------------------------------------------
	void PacketProcReqItemPlace(int64 SessionId, CMessage* Message);	
	//------------------------------------------------------------------
	// ������ ���� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotSave(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ������ ���� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotSwap(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ������ �ʱ�ȭ ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotInit(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ������ ���� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqCraftingConfirm(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ������ ��� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqItemUse(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ������ �ݱ� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqItemLooting(int64 SessionId, CMessage* Message);
	//--------------------------------------------------------
	// �� ��Ŷ ó��
	//--------------------------------------------------------
	void PacketProcReqPong(int64 SessionID, CMessage* Message);

	//-----------------------------------------------------------------------------------------------
	// DB ��û ó�� �Լ�
	//-----------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------
	// AccountID�� AccountDB�� �ִ��� üũ
	//-------------------------------------------------------------------
	void PacketProcReqDBAccountCheck(int64 SessionID, CMessage* Message);
	//-------------------------------------------------------------------------------
	// ������û�� ĳ���Ͱ� DB�� �ִ��� Ȯ�� �� ĳ���� ����
	//-------------------------------------------------------------------------------
	void PacketProcReqDBCreateCharacterNameCheck(int64 SessionID, CMessage* Message);
	//-------------------------------------------------
	// ������ ���� �� DB ����
	//-------------------------------------------------
	void PacketProcReqDBItemCreate(CMessage* Message);
	//------------------------------------------------------------------
	// �κ��丮 ���̺� ������ ������ ����
	//------------------------------------------------------------------
	void PacketProcReqDBLootingItemToInventorySave(int64 SessionId, CGameServerMessage* Message);
	//------------------------------------------------------------------
	// �κ��丮 ���̺� ������ ����
	//------------------------------------------------------------------
	void PacketProcReqDBCraftingItemToInventorySave(int64 SessionId, CGameServerMessage* Message);
	//-------------------------------------------------------------------------
	// �κ��丮 ���̺� ������ ����
	//-------------------------------------------------------------------------
	void PacketProcReqDBItemPlace(int64 SessionId, CGameServerMessage* Message);	
	//---------------------------------------------------------------
	// �κ��丮 ���̺� ������ ������Ʈ
	//---------------------------------------------------------------
	void PacketProcReqDBItemUpdate(int64 SessionId, CGameServerMessage* Message);
	//---------------------------------------------------------------
	// �κ��丮�� �� ����
	//---------------------------------------------------------------
	void PacketProcReqDBGoldSave(int64 SessionId, CMessage* Message);
	//-----------------------------------------------------------------------
	// ���� ���� ���ӽ� ĳ���� ������ DB���� �����ͼ� Ŭ�� ����
	//-----------------------------------------------------------------------
	void PacketProcReqDBCharacterInfoSend(int64 SessionId, CMessage* Message);
	//--------------------------------------------------------------------------
	// ������ ���̺� ��û ��ų ����
	//--------------------------------------------------------------------------
	void PacketProcReqDBQuickSlotBarSlotSave(int64 SessionId, CGameServerMessage* Message);
	//--------------------------------------------------------------------
	// �����Թٿ��� ������ ����
	//--------------------------------------------------------------------
	void PacketProcReqDBQuickSlotSwap(int64 SessionId, CMessage* Message);
	//--------------------------------------------------------------------
	// �����Թ� �ʱ�ȭ
	//--------------------------------------------------------------------
	void PacketProcReqDBQuickSlotInit(int64 SessionId, CMessage* Message);
	//--------------------------------------------------------------------
	// ���� ����� �÷��̾� ���� DB�� ���
	//--------------------------------------------------------------------
	void PacketProcReqDBLeavePlayerInfoSave(CPlayer* MyPlayer);

	//------------------------------------------------------------------
	// Ÿ�̸� �� ��û ó�� �Լ�
	//------------------------------------------------------------------

	//----------------------------------------------------------------
	// �и����� �� ó�� �Լ�
	//----------------------------------------------------------------
	void PacketProcTimerAttackEnd(int64 SessionId, CGameServerMessage* Message);
	//----------------------------------------------------------------
	// �������� �� ó�� �Լ�
	//----------------------------------------------------------------
	void PacketProcTimerSpellEnd(int64 SessionId, CGameServerMessage* Message);
	//----------------------------------------------------------------
	// ������ �ð� �� ó�� �Լ�
	//----------------------------------------------------------------
	void PacketProcTimerCastingTimeEnd(int64 SessionId, CGameServerMessage* Message);
	//----------------------------------------------------------------
	// ������Ʈ ����
	//----------------------------------------------------------------
	void PacketProcTimerObjectSpawn(CGameServerMessage* Message);
	//----------------------------------------------------------------
	// ������Ʈ ���� ����
	//----------------------------------------------------------------
	void PacketProcTimerObjectStateChange(int64 SessionId, CGameServerMessage* Message);
	//----------------------------------------------------------------
	// ������Ʈ ��Ʈ ó��
	//----------------------------------------------------------------
	void PacketProcTimerDot(int64 SessionId, CGameServerMessage* Message);
	//----------------------------------------------------------------
	// �� ó��
	//----------------------------------------------------------------
	void PacketProcTimerPing(int64 SessionId);

	//--------------------------------------
	// ��Ŷ���� �Լ�		
	//--------------------------------------
	
	//--------------------------------------
	// Ŭ���̾�Ʈ ���� ���� ��Ŷ ����
	//--------------------------------------
	CGameServerMessage* MakePacketResClientConnected();
	//---------------------------------------------------------------------------------------
	// �α��� ��û ���� ��Ŷ ����
	//---------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResLogin(bool& Status,int8& PlayerCount, int32* MyPlayerIndexes);
	//--------------------------------------------------------------------------------------------------
	// ĳ���� ���� ��û ���� ��Ŷ ����
	//--------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResCreateCharacter(bool IsSuccess, st_GameObjectInfo& CreateCharacterObjectInfo);
	//-------------------------------------------------------------
	// ���Ӽ��� ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------
	CGameServerMessage* MakePacketResEnterGame(bool EnterGameSuccess, st_GameObjectInfo* ObjectInfo);
	//-------------------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ���콺 ��ġ ������Ʈ ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMousePositionObjectInfo(int64 AccountId, int64 PreviousChoiceObjectId, st_GameObjectInfo ObjectInfo);
	//-------------------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� �� ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResGoldSave(int64 AccountId, int64 ObjectId, int64 GoldCount, int16 SliverCount, int16 BronzeCount, int16 ItemCount, int16 ItemType, bool ItemGainPrint = true);	
	//---------------------------------------------------------------------------------------------
	// ���Ӽ��� �κ��丮 ������ ���� ��û ���� ��Ŷ ����
	//---------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSelectItem(int64 AccountId, int64 ObjectId, CItem* SelectItem);	
	//---------------------------------------------------------------------------------------------
	// ���Ӽ��� �κ��丮 ������ ���� ��û ���� ��Ŷ ����
	//---------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResPlaceItem(int64 AccountId, int64 ObjectId, CItem* PlaceItem, CItem* OverlapItem);
	//-------------------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ������ �κ��丮 ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResItemToInventory(int64 TargetObjectId, CItem* InventoryItem, int16 ItemEach, bool IsExist, bool ItemGainPrint = true);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� �κ��丮 ������ ������Ʈ
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketInventoryItemUpdate(int64 PlayerId, st_ItemInfo UpdateItemInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� �κ��丮 ������ ��� ��û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketInventoryItemUse(int64 PlayerId, st_ItemInfo& UseItemInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ��� ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketEquipmentUpdate(int64 PlayerId, st_ItemInfo& EquipmentItemInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ��� ��û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResQuickSlotBarSlotSave(st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketQuickSlotCreate(int8 QuickSlotBarSize, int8 QuickSlotBarSlotSize, vector<st_QuickSlotBarSlotInfo> QuickslotBarSlotInfos);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��û ���� ��Ŷ ���� 
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResQuickSlotSwap(int64 AccountId, int64 PlayerId, st_QuickSlotBarSlotInfo SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo SwapBQuickSlotInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ �ʱ�ȭ ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResQuickSlotInit(int64 AccountId, int64 PlayerId, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, int16 QuickSlotKey);		
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketCraftingList(int64 AccountId, int64 PlayerId, vector<st_CraftingItemCategory> CraftingItemList);		
	//-------------------------------------------------
	// ���Ӽ��� �� ��Ŷ ����
	//-------------------------------------------------
	CGameServerMessage* MakePacketPing();	
	
	CItem* NewItemCrate(st_ItemInfo& NewItemInfo);
public:
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���ݿ�û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResAttack(int64 PlayerDBId, int64 TargetId, en_SkillType SkillType, int32 Damage, bool IsCritical);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMagic(int64 ObjectId, bool SpellStart, en_SkillType SkillType = en_SkillType::SKILL_TYPE_NONE, float SpellTime = 0.0f);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChangeObjectStat(int64 ObjectId, st_StatInfo ChangeObjectStatInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChangeObjectState(int64 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���� ������Ʈ ���� ���� ��Ŷ ���� 
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChangeMonsterObjectState(int64 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState, en_MonsterState MonsterState);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� �̵� ��û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMove(int64 AccountId, int64 ObjectId, bool CanMove, st_PositionInfo PositionInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���� �̵� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMonsterMove(int64 ObjectId, en_GameObjectType ObjectType, bool CanMove, st_PositionInfo PositionInfo, en_MonsterState MonsterState);
	//------------------------------------------------------------------------------------------------------
	// ���Ӽ��� �̵� ���� ��û ���� ��Ŷ ����
	//------------------------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResMoveStop(int64 AccountId, int64 ObjectId, st_PositionInfo PositionInto);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketPatrol(int64 ObjectId, en_GameObjectType ObjectType, bool CanMove, st_PositionInfo PositionInfo, en_MonsterState MonsterState);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResObjectSpawn(int32 ObjectInfosCount, vector<CGameObject*> ObjectInfos);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResObjectDeSpawn(int32 DeSpawnObjectCount, vector<CGameObject*> DeSpawnObjects);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketObjectDie(int64 DieObjectId);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� �޼��� ( ä��, �ý��� ) ��û ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResChattingBoxMessage(int64 PlayerDBId, en_MessageType MessageType, st_Color Color, wstring ChattingMessage);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ��ġ ��ũ ���߱� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSyncPosition(int64 TargetObjectId, st_PositionInfo SyncPosition);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ��ų ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketResSkillToSkillBox(int64 TargetObjectId, st_SkillInfo* SkillInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ����Ʈ ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketEffect(int64 TargetObjectId, en_EffectType EffectType, float PrintEffectTime);
	//---------------------------------------------------------------------------------
	// ���Ӽ��� ���� ��Ŷ ����
	//---------------------------------------------------------------------------------
	CGameServerMessage* MakePacketBuf(int64 TargetObjectId, float SkillCoolTime, float SkillCoolTimeSpeed, st_SkillInfo* SkillInfo);	
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ����ġ ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketExperience(int64 AccountId, int64 PlayerId, int64 GainExp, int64 CurrentExp, int64 RequireExp, int64 TotalExp);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ��ų ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketMagicCancel(int64 AccountId, int64 PlayerId);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ð� ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketCoolTime(int64 PlayerId, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, float SkillCoolTime, float SkillCoolTimeSpeed);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ��ų ���� �޼��� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CGameServerMessage* MakePacketSkillError(int64 PlayerId, en_SkillErrorType ErrorType, const WCHAR* SkillName, int16 SkillDistance = 0);
	//---------------------------------------------------
	// �α��� ���� �α׾ƿ� ��û ��Ŷ ����
	//---------------------------------------------------
	CGameServerMessage* MakePacketLogOut(int64 AccountID);
public:
	//---------------------------------------------------------
	// PlayerJob �޸�Ǯ
	//---------------------------------------------------------
	CMemoryPoolTLS<CPlayer::st_PlayerJob>* _PlayerJobMemoryPool;
	//-----------------------------------------------
	// Job �޸�Ǯ
	//-----------------------------------------------
	CMemoryPoolTLS<st_GameServerJob>* _GameServerJobMemoryPool;

	//------------------------------------
	// TimerJob �޸�Ǯ
	//------------------------------------
	CMemoryPoolTLS<st_TimerJob>* _TimerJobMemoryPool;

	//------------------------------------
	// Job ť
	//------------------------------------
	CLockFreeQue<st_GameServerJob*> _GameServerUserDataBaseThreadMessageQue;
	CLockFreeQue<st_GameServerJob*> _GameServerWorldDataBaseThreadMessageQue;	

	//--------------------------------------
	// TimerJob �켱���� ť
	//--------------------------------------
	CHeap<int64,st_TimerJob*>* _TimerHeapJob;

	int64 _LogicThreadFPS;
	// ��Ʈ��ũ ������ Ȱ��ȭ�� Ƚ��
	int64 _NetworkThreadWakeCount;
	// ��Ʈ��ũ ������ TPS
	int64 _NetworkThreadTPS;	

	// User DB ������ ����� �̺�Ʈ
	HANDLE _UserDataBaseWakeEvent;
	// World DB ������ ����� �̺�Ʈ
	HANDLE _WorldDataBaseWakeEvent;
	// DB ������ Ȱ��ȭ�� Ƚ��
	int64 _DataBaseThreadWakeCount;
	// DB ������ TPS
	int64 _DataBaseThreadTPS;	
	
	// Ÿ�̸� �� ������ Ȱ��ȭ�� Ƚ��
	int64 _TimerJobThreadWakeCount;
	// Ÿ�̸� �� ������ TPS
	int64 _TimerJobThreadTPS;

	CGameServer();
	~CGameServer();
	
	//------------------------------------------
	// ���� ���� ����
	//------------------------------------------
	void GameServerStart(const WCHAR* OpenIP, int32 Port);

	//------------------------------------------------------
	// �α��� ������ ����
	//------------------------------------------------------
	void LoginServerConnect(const WCHAR* ConnectIP, int32 Port);

	//--------------------------------------------------
	// ���ο� Ŭ���̾�Ʈ ���� 
	//--------------------------------------------------
	virtual void OnClientJoin(int64 SessionID) override;
	//--------------------------------------------------------------
	// ��Ʈ��ũ ��Ŷ ó��
	//--------------------------------------------------------------
	virtual void OnRecv(int64 SessionID, CMessage* Packet) override;
	//--------------------------------------------------------------
	// Ŭ���̾�Ʈ ����
	//--------------------------------------------------------------
	virtual void OnClientLeave(st_Session* LeaveSession) override;
	virtual bool OnConnectionRequest(const wchar_t ClientIP, int32 Port) override;	
	
	//--------------------------------------------------------------
	// ��ġ���� �������� �޼��� ���� ���Ϳ� ����
	//--------------------------------------------------------------
	void SendPacketAroundSector(st_Vector2Int CellPosition, CMessage* Message);
	//--------------------------------------------------------------
	// Session�� �������� ���� ���Ϳ� ����
	//--------------------------------------------------------------
	void SendPacketAroundSector(st_Session* Session, CMessage* Message, bool SendMe = false);
	//--------------------------------------------------------------
	// �þ߹��� �������� ��Ŷ ����
	//--------------------------------------------------------------
	void SendPacketFieldOfView(vector<st_FieldOfViewInfo> FieldOfViewObject, CMessage* Message, CGameObject* Self = nullptr);
	//--------------------------------------------------------------
	// ������Ʈ�� �������� �þߺ� �ȿ� �ִ� �÷��̾� ������� ��Ŷ ����
	//--------------------------------------------------------------
	void SendPacketFieldOfView(CGameObject* Object, CMessage* Message);
	//--------------------------------------------------------------
	// Session �������� �þߺ� �ȿ� �÷��̾� ������� ��Ŷ ����
	//--------------------------------------------------------------
	void SendPacketFieldOfView(st_Session* Session, CMessage* Message, bool SendMe = false);
	
	//--------------------------------------------------------------
	// ��ų ��Ÿ�� Ÿ�̸� �� ����
	//--------------------------------------------------------------
	void SkillCoolTimeTimerJobCreate(CPlayer* Player, int64 CastingTime, st_SkillInfo* CoolTimeSkillInfo, en_TimerJobType TimerJobType, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex);
	//--------------------------------------------------------------
	// ������Ʈ ���� Ÿ�̸� �� ����
	//--------------------------------------------------------------
	void SpawnObjectTimeTimerJobCreate(int16 SpawnObjectType, st_Vector2Int SpawnPosition, int64 SpawnTime);
	//--------------------------------------------------------------
	// ������Ʈ ���� ���� Ÿ�̸� �� ����
	//--------------------------------------------------------------
	void ObjectStateChangeTimerJobCreate(CGameObject* Target, en_CreatureState ChangeState, int64 ChangeTime);
	//--------------------------------------------------------------
	// ������Ʈ ��Ʈ Ÿ�̸� �� ����
	//--------------------------------------------------------------
	st_TimerJob* ObjectDotTimerCreate(CGameObject* Target, en_DotType DotType, int64 DotTime, int32 HPPoint, int32 MPPoint, int64 DotTotalTime = 0, int64 SessionId = 0);

	//-------------------------------------------
	// �� Ÿ�̸� �� ����
	//-------------------------------------------
	void PingTimerCreate(st_Session* PingSession);
};