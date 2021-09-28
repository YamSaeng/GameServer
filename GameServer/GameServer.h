#pragma once
#pragma comment(lib,"winmm")
#include "NetworkLib.h"
#include "CommonProtocol.h"
#include "GameServerInfo.h"
#include "MemoryPoolTLS.h"
#include "Heap.h"

// �⺻ �޼��� ó�� �������
// DB ���� ó�� �����尡 2�� ����
// ���� �⺻ �޼��� ó������ st_Client�� �������
// DB ���� ó�� �����忡�� ����� st_Client�� ������� DBó�� ���� �� �ִ�.
// ó�� ��� �ؾ���

class CGameServer : public CNetworkLib
{
private:
	//---------------------------------------------------
	//���� ����Ʈ �迭
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
	// ��Ŷó�� �Լ�
	// 1. �α��� ��û
	// 2. ĳ���� ���� ��û 
	// 3. ���� ���� ��û
	// 4. ������ ��û
	// 6. ���콺 ��ġ ������Ʈ ���� ��û
	// 7. ä�� ��û
	// 8. ������ �κ��丮 �ֱ� ��û
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
	// DB ��û ó�� �Լ�
	// 1. �α��� ��û���� �߰��� AccountServer�� �Է¹��� Account�� �ִ��� Ȯ���ϰ� ���� �α��� �˻�	
	// 2. ĳ���� ���� ��û���� �߰��� DB�� �Է��� �ش� ĳ���Ͱ� �ִ��� Ȯ��
	// 3. ������ ����
	// 4. ĳ���� �κ��丮�� �ִ� Item�� DB�� ����
	// 5. ĳ���� �κ��丮���� ������ Swap
	// 6. ĳ���� �κ��丮�� �ִ� Gold�� DB�� ����
	// 7. ĳ���� ���� Ŭ�󿡰� ����
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
	//��Ŷ���� �Լ�
	//1. Ŭ���̾�Ʈ ���� ����
	//2. �α��� ��û ����
	//3. ĳ���� ���� ��û ����
	//4. ���� ���� ��û ����
	//5. ���콺 ��ġ ������Ʈ ���� ��û ����
	//6. ��� ���� ��û ���� 
	//7. ������ ���� ��û ����
	//8. ���� ��û ���� 
	//9. ���� ���� ��û ����
	//10. HP ���� ��û ����
	//11. ������Ʈ ���� ���� ��û ����
	//12. �̵� ��û ����
	//13. ������Ʈ ���� ��û ����
	//14. ������Ʈ ���� ��û ����	
	//15. ������Ʈ ���� ����
	//16. ä�� ��û ����
	//17. ������ ���� ��û ����
	//18. ������Ʈ ��ġ ����
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
	// Network ������ TPS
	int64 _NetworkThreadTPS; 

	// DB ������ Ȱ��ȭ�� Ƚ��
	int64 _DataBaseThreadWakeCount;
	// DB ������ TPS
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