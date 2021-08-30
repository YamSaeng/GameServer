#pragma once
#pragma comment(lib,"winmm")
#include "NetworkLib.h"
#include "CommonProtocol.h"
#include "GameServerInfo.h"
#include "MemoryPoolTLS.h"

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
	HANDLE _GameLogicThread;

	HANDLE _AuthThreadWakeEvent;
	HANDLE _NetworkThreadWakeEvent;
	HANDLE _DataBaseWakeEvent;

	bool _AuthThreadEnd;
	// WorkerThread ����� ����
	bool _NetworkThreadEnd;
	// DataBaseThread ����� ����
	bool _DataBaseThreadEnd;

	// ���Ӽ������� �����Ǵ� ������Ʈ���� ���̵�
	int64 _GameObjectId;

	static unsigned __stdcall AuthThreadProc(void* Argument);
	static unsigned __stdcall NetworkThreadProc(void* Argument);
	static unsigned __stdcall DataBaseThreadProc(void* Argument);
	static unsigned __stdcall HeartBeatCheckThreadProc(void* Argument);

	void CreateNewClient(int64 SessionId);
	void DeleteClient(st_SESSION* Session);

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
	void PacketProcReqAttack(int64 SessionID, CMessage* Message);
	void PacketProcReqMousePositionObjectInfo(int64 SessionID, CMessage* Message);
	void PacketProcReqObjectStateChange(int64 SessionId, CMessage* Message);
	void PacketProcReqChattingMessage(int64 SessionId, CMessage* Message);
	void PacketProcReqItemToInventory(int64 SessionId, CMessage* Message);
	void PacketProcReqSectorMove(int64 SessionID, CMessage* Message);
	void PacketProcReqMessage(int64 SessionID, CMessage* Message);
	void PacketProcReqHeartBeat(int64 SessionID, CMessage* Message);

	//-----------------------------------------------------------------------------------------------
	// DB ��û ó�� �Լ�
	// 1. �α��� ��û���� �߰��� AccountServer�� �Է¹��� Account�� �ִ��� Ȯ���ϰ� ���� �α��� �˻�	
	// 2. ĳ���� ���� ��û���� �߰��� DB�� �Է��� �ش� ĳ���Ͱ� �ִ��� Ȯ��
	// 3. ĳ���� �κ��丮�� �ִ� Item�� DB�� ����
	// 4. ĳ���� �κ��丮�� �ִ� Gold�� DB�� ����
	// 5. ĳ���� ���� Ŭ�󿡰� ����
	//-----------------------------------------------------------------------------------------------
	void PacketProcReqAccountCheck(int64 SessionID, CMessage* Message);
	void PacketProcReqCreateCharacterNameCheck(int64 SessionID, CMessage* Message);
	void PacketProcReqDBItemToInventorySave(int64 SessionId, CMessage* Message);
	void PacketProcReqGoldSave(int64 SessionId, CMessage* Message);
	void PacketProcReqCharacterInfoSend(int64 SessionId, CMessage* Message);

	//----------------------------------------------------------------
	//��Ŷ���� �Լ�
	//1. Ŭ���̾�Ʈ ���� ����
	//2. �α��� ��û ����
	//3. ĳ���� ���� ��û ����
	//4. ���� ���� ��û ����
	//5. ���콺 ��ġ ������Ʈ ���� ��û ����
	//5. ���� ��û ����
	//6. ������Ʈ ���� 
	//7. ������Ʈ ����
	//8. HP ����
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
	// Job �޸�Ǯ
	//------------------------------------
	CMemoryPoolTLS<st_Job>* _JobMemoryPool;
	//------------------------------------
	// Job ť
	//------------------------------------
	CLockFreeQue<st_Job*> _GameServerAuthThreadMessageQue;
	CLockFreeQue<st_Job*> _GameServerNetworkThreadMessageQue;
	CLockFreeQue<st_Job*> _GameServerDataBaseThreadMessageQue;

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