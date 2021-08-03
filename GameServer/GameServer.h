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
	//���� ����Ʈ �迭
	//---------------------------------------------------
	list<int64> _SectorList[SECTOR_Y_MAX][SECTOR_X_MAX];

	HANDLE _UpdateThread;
	HANDLE _DataBaseThread;

	HANDLE _UpdateWakeEvent;
	HANDLE _DataBaseWakeEvent;

	// WorkerThread ����� ����
	bool _UpdateThreadEnd;
	// DataBaseThread ����� ����
	bool _DataBaseThreadEnd;

	// ���Ӽ������� �����Ǵ� ������Ʈ���� ���̵�
	int64 _GameObjectId;

	static unsigned __stdcall UpdateThreadProc(void* Argument);
	static unsigned __stdcall DataBaseThreadProc(void* Argument);
	static unsigned __stdcall HeartBeatCheckThreadProc(void* Argument);

	void CreateNewClient(int64 SessionID);
	void DeleteClient(int64 SessionID);

	void PacketProc(int64 SessionID, CMessage* Message);
	
	//----------------------------------------------------------------
	//��Ŷó�� �Լ�
	//1. �α��� ��û
	//2. ĳ���� ���� ��û 
	//3. ���� �̵� ��û
	//4. ä�� ������ ��û
	//5. ��Ʈ��Ʈ
	//----------------------------------------------------------------	
	void PacketProcReqLogin(int64 SessionID, CMessage* Message);
	void PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message);
	void PacketProcReqSectorMove(int64 SessionID, CMessage* Message);
	void PacketProcReqMessage(int64 SessionID, CMessage* Message);
	void PacketProcReqHeartBeat(int64 SessionID, CMessage* Message);

	//----------------------------------------------------------------
	// DB ��û ó�� �Լ�
	//----------------------------------------------------------------
	void PacketProcReqAccountCheck(int64 SessionID, CMessage* Message);
	void PacketProcReqCreateCharacterNameCheck(int64 SessionID, CMessage* Message);

	//----------------------------------------------------------------
	//��Ŷ���� �Լ�
	//1. Ŭ���̾�Ʈ ���� ����
	//2. �α��� ��û ����
	//3. ���� �̵� ��û ����
	//4. ä�� ������ ��û ����
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
	// Job �޸�Ǯ
	//------------------------------------
	CMemoryPoolTLS<st_JOB>* _JobMemoryPool;
	//------------------------------------
	//������ Ŭ���̾�Ʈ�� ������ �޸�Ǯ
	//------------------------------------
	CMemoryPoolTLS<st_CLIENT>* _ClientMemoryPool;
	//------------------------------------
	// Job ť
	//------------------------------------
	CLockFreeQue<st_JOB*> _GameServerCommonMessageQue;
	CLockFreeQue<st_JOB*> _GameServerDataBaseMessageQue;

	// ä�ü��� ������ Ŭ��
	unordered_map<int64, st_CLIENT*> _ClientMap;

	int64 _UpdateWakeCount; // Update �����尡 �Ͼ Ƚ��	
	int64 _UpdateTPS; // Update �����尡 1�ʿ� �۾��� ó����

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
	//�ڽ� ���� 8���͵鿡�� �޼����� �����Ѵ�.
	//SendMe = false �����Դ� ������ �ʴ´�.
	//SendMe = true �����Ե� ������.
	//------------------------------------------------------------------------------
	void SendPacketAround(st_CLIENT* Client, CMessage* Message, bool SendMe = false);
	void SendPacketBroadcast(CMessage* Message);
};