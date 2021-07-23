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
	HANDLE _UpdateWakeEvent;

	//WorkerThread ����� ����
	bool _WorkThreadEnd;

	static unsigned __stdcall UpdateThreadProc(void* Argument);
	static unsigned __stdcall HeartBeatCheckThreadProc(void* Argument);

	void CreateNewClient(int64 SessionID);
	void DeleteClient(int64 SessionID);

	void PacketProc(int64 SessionID, CMessage* Message);

	//----------------------------------------------------------------
	//��Ŷó�� �Լ�
	//1. �α��� ��û
	//2. ���� �̵� ��û
	//3. ä�� ������ ��û
	//4. ��Ʈ��Ʈ
	//----------------------------------------------------------------
	void PacketProcReqLogin(int64 SessionID, CMessage* Message);
	void PacketProcReqSectorMove(int64 SessionID, CMessage* Message);
	void PacketProcReqMessage(int64 SessionID, CMessage* Message);
	void PacketProcReqHeartBeat(int64 SessionID, CMessage* Message);
	//----------------------------------------------------------------
	//��Ŷ���� �Լ�
	//1. �α��� ��û ����
	//2. ���� �̵� ��û ����
	//3. ä�� ������ ��û ����
	//----------------------------------------------------------------
	CMessage* MakePacketResLogin(int64 AccountNo, BYTE Status);
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
	CLockFreeQue<st_JOB*> _ChatServerMessageQue;

	// ä�ü��� ������ Ŭ��
	unordered_map<int64, st_CLIENT*> _ClientMap;

	int64 _UpdateWakeCount; // Update �����尡 �Ͼ Ƚ��	
	int64 _UpdateTPS; // Update �����尡 1�ʿ� �۾��� ó����

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