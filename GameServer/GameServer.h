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

	HANDLE _NetworkThread;
	HANDLE _DataBaseThread;
	HANDLE _GameLogicThread;

	HANDLE _NetworkThreadWakeEvent;
	HANDLE _DataBaseWakeEvent;

	// WorkerThread ����� ����
	bool _NetworkThreadEnd;
	// DataBaseThread ����� ����
	bool _DataBaseThreadEnd;
	// LogicThread ����� ����
	bool _LogicThreadEnd;

	// ���Ӽ������� �����Ǵ� ������Ʈ���� ���̵�
	int64 _GameObjectId;

	static unsigned __stdcall NetworkThreadProc(void* Argument);
	static unsigned __stdcall DataBaseThreadProc(void* Argument);
	static unsigned __stdcall GameLogicThreadProc(void* Argument);
	static unsigned __stdcall HeartBeatCheckThreadProc(void* Argument);

	void CreateNewClient(int64 SessionID);
	void DeleteClient(int64 SessionID);

	void PacketProc(int64 SessionId, CMessage* Message);
	
	//----------------------------------------------------------------
	//��Ŷó�� �Լ�
	//1. �α��� ��û
	//2. ĳ���� ���� ��û 
	//3. ���� ���� ��û
	//4. ������ ��û
	//5. ���� ��û
	//6. ��Ʈ��Ʈ
	//----------------------------------------------------------------	
	void PacketProcReqLogin(int64 SessionID, CMessage* Message);
	void PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message);
	void PacketProcReqEnterGame(int64 SessionID, CMessage* Message);
	void PacketProcReqMove(int64 SessionID, CMessage* Message);	
	void PacketProcReqAttack(int64 SessionID, CMessage* Message);
	void PacketProcReqMousePositionObjectInfo(int64 SessionID, CMessage* Message);
	void PacketProcReqSectorMove(int64 SessionID, CMessage* Message);
	void PacketProcReqMessage(int64 SessionID, CMessage* Message);
	void PacketProcReqHeartBeat(int64 SessionID, CMessage* Message);

	//-----------------------------------------------------------------------------------------------
	// DB ��û ó�� �Լ�
	// 1. �α��� ��û���� �߰��� AccountServer�� �Է¹��� Account�� �ִ��� Ȯ���ϰ� ���� �α��� �˻�	
	// 2. ĳ���� ���� ��û���� �߰��� DB�� �Է��� �ش� ĳ���Ͱ� �ִ��� Ȯ��
	//-----------------------------------------------------------------------------------------------
	void PacketProcReqAccountCheck(int64 SessionID, CMessage* Message);
	void PacketProcReqCreateCharacterNameCheck(int64 SessionID, CMessage* Message);

	//----------------------------------------------------------------
	//��Ŷ���� �Լ�
	//1. Ŭ���̾�Ʈ ���� ����
	//2. �α��� ��û ����
	//3. ���� ���� ��û ����
	//4. ������ ��û ����
	//5. ���� ��û ����
	//6. ������Ʈ ���� 
	//7. ������Ʈ ����
	//8. HP ����
	//----------------------------------------------------------------
	CMessage* MakePacketResClientConnected();
	CMessage* MakePacketResLogin(bool Status, int32 PlayerCount, int32 PlayerDBId, wstring PlayersName);
	CMessage* MakePacketResCreateCharacter(bool IsSuccess, int32 PlayerDBId, wstring PlayerName);
	CMessage* MakePacketResEnterGame(st_GameObjectInfo ObjectInfo);	
	CMessage* MakePacketResAttack(int64 AccountId, int32 PlayerDBId, en_MoveDir Dir);	
	CMessage* MakePacketMousePositionObjectInfo(int64 AccountId, int32 PlayerDBId, st_GameObjectInfo ObjectInfo);
	CMessage* MakePacketResMessage(int64 AccountNo, WCHAR* ID, WCHAR* NickName, WORD MessageLen, WCHAR* Message);

	st_CLIENT* FindClient(int64 SessionID);
public:
	CMessage* MakePacketResChangeHP(int32 PlayerDBId, int32 CurrentHP, int32 MaxHP);
	CMessage* MakePacketResObjectState(int32 ObjectId,en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState);
	CMessage* MakePacketResMove(int64 AccountId, int32 ObjectId, en_GameObjectType ObjectType, st_PositionInfo PositionInfo);
	CMessage* MakePacketResSpawn(int32 ObjectInfosCount, vector<st_GameObjectInfo> ObjectInfos);
	CMessage* MakePacketResDeSpawn(int32 DeSpawnObjectCount, vector<int64> DeSpawnObjectIds);
	CMessage* MakePacketResDie(int64 DieObjectId);
public:
	//------------------------------------
	// Job �޸�Ǯ
	//------------------------------------
	CMemoryPoolTLS<st_Job>* _JobMemoryPool;
	//------------------------------------
	//������ Ŭ���̾�Ʈ�� ������ �޸�Ǯ
	//------------------------------------
	CMemoryPoolTLS<st_CLIENT>* _ClientMemoryPool;
	//------------------------------------
	// Job ť
	//------------------------------------
	CLockFreeQue<st_Job*> _GameServerCommonMessageQue;
	CLockFreeQue<st_Job*> _GameServerDataBaseMessageQue;

	// ä�ü��� ������ Ŭ��
	unordered_map<int64, st_CLIENT*> _ClientMap;

	int64 _NetworkThreadWakeCount; // Update �����尡 �Ͼ Ƚ��	
	int64 _NetworkThreadTPS; // Update �����尡 1�ʿ� �۾��� ó����

	int64 _DataBaseThreadWakeCount;
	int64 _DataBaseThreadTPS;

	CGameServer();
	~CGameServer();

	void Start(const WCHAR* OpenIP, int32 Port);

	virtual void OnClientJoin(int64 SessionID) override;
	virtual void OnRecv(int64 SessionID, CMessage* Packet) override;
	virtual void OnClientLeave(int64 SessionID) override;
	virtual bool OnConnectionRequest(const wchar_t ClientIP, int32 Port) override;
		
	void SendPacketSector(CSector* Sector, CMessage* Message);
	void SendPacketAroundSector(st_Vector2Int CellPosition, CMessage* Message);
	void SendPacketAroundSector(st_CLIENT* Client, CMessage* Message, bool SendMe = false);

	//------------------------------------------------------------------------------
	//�ڽ� ���� 8���͵鿡�� �޼����� �����Ѵ�.
	//SendMe = false �����Դ� ������ �ʴ´�.
	//SendMe = true �����Ե� ������.
	//------------------------------------------------------------------------------
	void SendPacketBroadcast(CMessage* Message);
};