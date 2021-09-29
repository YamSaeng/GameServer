#pragma once
#pragma comment(lib,"winmm")
#include "NetworkLib.h"
#include "CommonProtocol.h"
#include "GameServerInfo.h"
#include "MemoryPoolTLS.h"
#include "Heap.h"

class CGameServer : public CNetworkLib
{
private:
	// ���� ������
	HANDLE _AuthThread;
	// ��Ʈ��ũ ������
	HANDLE _NetworkThread;
	// �����ͺ��̽� ������
	HANDLE _DataBaseThread;
	// Ÿ�̸��� ������
	HANDLE _TimerJobThread;
	// ���� ������
	HANDLE _LogicThread;

	// ���� ������ ����� �̺�Ʈ
	HANDLE _AuthThreadWakeEvent;
	// ��Ʈ��ũ ������ ����� �̺�Ʈ
	HANDLE _NetworkThreadWakeEvent;	
	// Ÿ�̸��� ������ ����� �̺�Ʈ
	HANDLE _TimerThreadWakeEvent;	

	// AuthThread ����� ����
	bool _AuthThreadEnd;
	// WorkerThread ����� ����
	bool _NetworkThreadEnd;
	// DataBaseThread ����� ����
	bool _DataBaseThreadEnd;
	// LogicThread ����� ����
	bool _LogicThreadEnd;
	// TimerJobThread ����� ����
	bool _TimerJobThreadEnd;

	// TimerJobThread ���� Lock
	SRWLOCK _TimerJobLock;

	//-------------------------------------------------------
	// ���� ������ ( Ŭ�� ����, Ŭ�� ������ )
	//-------------------------------------------------------
	static unsigned __stdcall AuthThreadProc(void* Argument);
	static unsigned __stdcall NetworkThreadProc(void* Argument);
	static unsigned __stdcall DataBaseThreadProc(void* Argument);
	static unsigned __stdcall TimerJobThreadProc(void* Argument);
	static unsigned __stdcall LogicThreadProc(void* Argument);
	static unsigned __stdcall HeartBeatCheckThreadProc(void* Argument);

	//------------------------------------
	// Ŭ�� ���� �⺻ ���� ����
	//------------------------------------
	void CreateNewClient(int64 SessionId);
	//------------------------------------
	// Ŭ�� ���� 
	//------------------------------------
	void DeleteClient(st_Session* Session);

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
	//---------------------------------------------------------
	// �̵� ��û ó��
	//--------------------------------------------------------
	void PacketProcReqMove(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------------
	// ���� ��û ó��
	//---------------------------------------------------------------
	void PacketProcReqMeleeAttack(int64 SessionID, CMessage* Message);
	//---------------------------------------------------------
	// ���� ��û ó��
	//---------------------------------------------------------
	void PacketProcReqMagic(int64 SessionId, CMessage* Message);
	//----------------------------------------------------------------------------
	// ���콺 ��ġ ������Ʈ ���� ��û ó��
	//----------------------------------------------------------------------------
	void PacketProcReqMousePositionObjectInfo(int64 SessionID, CMessage* Message);
	//----------------------------------------------------------------------
	// ������Ʈ ���� ���� ��û ó��
	//----------------------------------------------------------------------
	void PacketProcReqObjectStateChange(int64 SessionId, CMessage* Message);
	//--------------------------------------------------------------------
	// ä�� �޼��� ��û ó��
	//--------------------------------------------------------------------
	void PacketProcReqChattingMessage(int64 SessionId, CMessage* Message);
	//--------------------------------------------------------------------
	// ������ �κ��丮 ���� ��û ó��
	//--------------------------------------------------------------------
	void PacketProcReqItemToInventory(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------
	// ������ �κ��丮 ���� ��û ó��
	//------------------------------------------------------------
	void PacketProcReqItemSwap(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ������ ���� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotSave(int64 SessionId, CMessage* Message);
	//------------------------------------------------------------------
	// ������ ���� ��û ó��
	//------------------------------------------------------------------
	void PacketProcReqQuickSlotSwap(int64 SessionId, CMessage* Message);
	//-------------------------------------------------------------
	//-------------------------------------------------------------
	void PacketProcReqHeartBeat(int64 SessionID, CMessage* Message);

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
	//-------------------------------------------------
	// �κ��丮 ���̺� ������ ����
	//-------------------------------------------------------------------------
	void PacketProcReqDBItemToInventorySave(int64 SessionId, CMessage* Message);
	//---------------------------------------------------------------
	// �κ��丮 ���̺� ������ ����
	//---------------------------------------------------------------
	void PacketProcReqDBItemSwap(int64 SessionId, CMessage* Message);
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
	void PacketProcReqDBQuickSlotBarSlotSave(int64 SessionId, CMessage* Message);
	//--------------------------------------------------------------------
	// �����Թٿ��� ������ ����
	//--------------------------------------------------------------------
	void PacketProcReqDBQuickSlotSwap(int64 SessionId, CMessage* Message);


	//------------------------------------------------------------------
	// Ÿ�̸� �� ��û ó�� �Լ�
	//------------------------------------------------------------------

	//----------------------------------------------------------------
	// �и����� �� ó�� �Լ�
	//----------------------------------------------------------------
	void PacketProcTimerAttackEnd(int64 SessionId, CMessage* Message);
	//----------------------------------------------------------------
	// �������� �� ó�� �Լ�
	//----------------------------------------------------------------
	void PacketProcTimerSpellEnd(int64 SessionId, CMessage* Message);
	//----------------------------------------------------------------
	// ������ �ð� �� ó�� �Լ�
	//----------------------------------------------------------------
	void PacketProcTimerCoolTimeEnd(int64 SessionId, CMessage* Message);	

	//--------------------------------------
	// ��Ŷ���� �Լ�		
	//--------------------------------------
	
	//--------------------------------------
	// Ŭ���̾�Ʈ ���� ���� ��Ŷ ����
	//--------------------------------------
	CMessage* MakePacketResClientConnected();
	//---------------------------------------------------------------------------------------
	// �α��� ��û ���� ��Ŷ ����
	//---------------------------------------------------------------------------------------
	CMessage* MakePacketResLogin(bool Status, int8 PlayerCount, CGameObject** MyPlayersInfo);
	//--------------------------------------------------------------------------------------------------
	// ĳ���� ���� ��û ���� ��Ŷ ����
	//--------------------------------------------------------------------------------------------------
	CMessage* MakePacketResCreateCharacter(bool IsSuccess, st_GameObjectInfo CreateCharacterObjectInfo);
	//-------------------------------------------------------------
	// ���Ӽ��� ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------
	CMessage* MakePacketResEnterGame(st_GameObjectInfo ObjectInfo);
	//-------------------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ���콺 ��ġ ������Ʈ ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------------------------------------------------------------------
	CMessage* MakePacketResMousePositionObjectInfo(int64 AccountId, int64 PreviousChoiceObjectId, st_GameObjectInfo ObjectInfo);
	//-------------------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� �� ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------------------------------------------------------------------
	CMessage* MakePacketResGoldSave(int64 AccountId, int64 ObjectId, int64 GoldCount, int16 SliverCount, int16 BronzeCount, int16 ItemCount, int16 ItemType, bool ItemGainPrint = true);
	//-------------------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------------------------------------------------------------------
	CMessage* MakePacketResItemSwap(int64 AccountId, int64 ObjectId, st_ItemInfo SwapAItemInfo, st_ItemInfo SwapBItemInfo);
	//-------------------------------------------------------------------------------------------------------------------------
	// ���Ӽ��� ������ �κ��丮 ���� ��û ���� ��Ŷ ����
	//-------------------------------------------------------------------------------------------------------------------------
	CMessage* MakePacketResItemToInventory(int64 TargetObjectId, st_ItemInfo ItemInfo, int16 ItemEach, bool ItemGainPrint = true);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ��� ��û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketResQuickSlotBarSlotSave(st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketQuickSlotCreate(int8 QuickSlotBarSize, int8 QuickSlotBarSlotSize, vector<st_QuickSlotBarSlotInfo> QuickslotBarSlotInfos);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��û ���� ��Ŷ ���� 
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketResQuickSlotSwap(int64 AccountId, int64 PlayerId, st_QuickSlotBarSlotInfo SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo SwapBQuickSlotInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���� �޼��� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketError(int64 PlayerId, en_ErrorType ErrorType, wstring ErrorMessage);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������ð� ��� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketCoolTime(int64 PlayerId, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, float SkillCoolTime, float SkillCoolTimeSpeed);	
public:
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ���ݿ�û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketResAttack(int64 PlayerDBId, int64 TargetId, en_SkillType SkillType, int32 Damage, bool IsCritical);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketResMagic(int64 ObjectId, bool SpellStart, en_SkillType SkillType = en_SkillType::SKILL_TYPE_NONE, float SpellTime = 0.0f);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketChangeObjectStat(int64 ObjectId, st_StatInfo ChangeObjectStatInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketResObjectState(int64 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� �̵� ��û ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketResMove(int64 AccountId, int64 ObjectId, en_GameObjectType ObjectType, st_PositionInfo PositionInfo);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketResObjectSpawn(int32 ObjectInfosCount, vector<st_GameObjectInfo> ObjectInfos);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketResObjectDeSpawn(int32 DeSpawnObjectCount, vector<int64> DeSpawnObjectIds);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketObjectDie(int64 DieObjectId);
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� �޼��� ( ä��, �ý��� ) ��û ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketResChattingBoxMessage(int64 PlayerDBId, en_MessageType MessageType, st_Color Color, wstring ChattingMessage);		
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ��ġ ��ũ ���߱� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketResSyncPosition(int64 TargetObjectId, st_PositionInfo SyncPosition);	
	//-----------------------------------------------------------------------------------------
	// ���Ӽ��� ��ų ���� ��Ŷ ����
	//-----------------------------------------------------------------------------------------
	CMessage* MakePacketResSkillToSkillBox(int64 TargetObjectId, st_SkillInfo SkillInfo);	
public:
	//------------------------------------
	// Job �޸�Ǯ
	//------------------------------------
	CMemoryPoolTLS<st_Job>* _JobMemoryPool;

	//------------------------------------
	// TimerJob �޸�Ǯ
	//------------------------------------
	CMemoryPoolTLS<st_TimerJob>* _TimerJobMemoryPool;

	//------------------------------------
	// Job ť
	//------------------------------------
	CLockFreeQue<st_Job*> _GameServerAuthThreadMessageQue;
	CLockFreeQue<st_Job*> _GameServerNetworkThreadMessageQue;
	CLockFreeQue<st_Job*> _GameServerDataBaseThreadMessageQue;

	//--------------------------------------
	// TimerJob �켱���� ť
	//--------------------------------------
	CHeap<int64,st_TimerJob*>* _TimerHeapJob;

	// ���� ������ Ȱ��ȭ�� Ƚ��
	int64 _AuthThreadWakeCount;
	// ���� ������ TPS
	int64 _AuthThreadTPS;
	// ��Ʈ��ũ ������ Ȱ��ȭ�� Ƚ��
	int64 _NetworkThreadWakeCount;
	// ��Ʈ��ũ ������ TPS
	int64 _NetworkThreadTPS; 

	// DB ������ ����� �̺�Ʈ
	HANDLE _DataBaseWakeEvent;
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
};