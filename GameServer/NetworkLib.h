#pragma once
#pragma comment(lib,"ws2_32")

#include "SessionInfo.h"
#include "LockFreeStack.h"

#define CLOSE_SOCKET_DO_NOT 0
#define CLOSE_SOCKET_DO  1

#define SENDING_DO_NOT 0
#define SENDING_DO 1

#define USING_DO_NOT 0 
#define USING_DO 1

#define SERVER_SESSION_MAX 15000

// ���� �ε��� �ֱ� 16��Ʈ �������� �а� INDEX ����
#define ADD_SESSIONID_INDEX(SESSIONID,INDEX)	((SESSIONID << 0x10) | ((short)INDEX))
// ���� ���̵� ��� 
#define GET_SESSIOINID(SESSIONID)	((SESSIONID & 0xFFFFFF00) >> 0x10)
#define GET_SESSIONINDEX(SESSIONID)	(SESSIONID & 0xFFFF)

using namespace std;

class CNetworkLib
{
private:
	HANDLE _HCP; // IOCP Handle
	SOCKET _ListenSock; // ��������

	__int64 _SessionID;
	
	//---------------------------------------------------------
	// IOCP ��Ŀ ������
	//---------------------------------------------------------
	static unsigned __stdcall WorkerThreadProc(void* Argument);

	//---------------------------------------------------------
	// Accept ���� ������
	//---------------------------------------------------------
	static unsigned __stdcall AcceptThreadProc(void* Argument);

	//---------------------------------------------------------
	// ���� ���������� ���ǹݳ�
	// IOCount Ȯ�� Release ������ Ȯ��
	//---------------------------------------------------------
	void ReleaseSession(st_Session* ReleaseSession);	

	//------------------------------------
	// WSASend�� �����Ѵ�.
	//------------------------------------
	void SendPost(st_Session* SendSession);

	//-------------------------------------------------------------------
	// WSARecv�� �����Ѵ�.
	//-------------------------------------------------------------------
	void RecvPost(st_Session* RecvSession, bool IsAcceptRecvPost = false);

	//--------------------------------------------------------------------------------
	// Recv�Ϸ�� ȣ���ϴ� �Լ�
	//--------------------------------------------------------------------------------
	void RecvNotifyComplete(st_Session* RecvCompleteSession, const DWORD& Transferred);
	//--------------------------------------------------------------------------------
	// Send�Ϸ�� ȣ���ϴ� �Լ�
	//--------------------------------------------------------------------------------
	void SendNotifyComplete(st_Session* SendCompleteSession);

	//-----------------------------------------------------------------
	// Ŭ�� ���ӽ� Accept ȣ���ϰ� ������ Ŭ�� ������� ȣ���ϴ� �Լ�
	//-----------------------------------------------------------------
	virtual void OnClientJoin(__int64 SessionID) = 0;

	//-----------------------------------------------------------
	// RecvNotifyComplete �ȿ��� ȣ���ϴ� �Լ���
	// �ش� �������� �� �޼����� OnRecv�� ���� �����Ѵ�.
	//-----------------------------------------------------------
	virtual void OnRecv(__int64 SessionID, CMessage* Packet) = 0;

	//-------------------------------------------------
	// ReleaseSession �ȿ��� ȣ�� 
	// �ݳ��Ǵ� ������ �������� IOCount�� 0�� �ɶ� ȣ��
	//-------------------------------------------------
	virtual void OnClientLeave(st_Session *LeaveSession) = 0;

	virtual bool OnConnectionRequest(const wchar_t ClientIP, int Port) = 0;

protected:
	//--------------------------------------------
	// ���� ������ �迭
	//--------------------------------------------
	st_Session* _SessionArray[SERVER_SESSION_MAX];

	//------------------------------------------------
	// ���� ����Ʈ �迭 �ε����� ������������ �����ص�
	//------------------------------------------------	
	CLockFreeStack<int32> _SessionArrayIndexs;

	//-----------------------------------------
	// ����ID�� ���� ���� �ε����� ã�� �迭���� ������ �����´�.
	//-----------------------------------------
	st_Session* FindSession(__int64 SessionID);

	//---------------------------------------------------------------------------------------------------------------
	// ���������� IOCount�� ���ҽ����ִ� �Լ� ( ����Ѵٴ� �ǹ��� IOCount�� 1 �ٿ������ν� �ݳ� �Ѵٴ� �ǹ̸� ���´�)
	//---------------------------------------------------------------------------------------------------------------
	void ReturnSession(st_Session* Session);
public:
	//-------------------------
	// ����Ǿ� �ִ� ���� ����
	//-------------------------
	LONG64 _SessionCount;

	//--------------------------
	// ���� ������ Accept�� Ƚ��
	//--------------------------
	__int64 _AcceptTotal;

	//--------------------
	// 1�ʴ� Aceppt�� Ƚ��
	//--------------------
	int _AcceptTPS;

	//------------------------
	// 1�ʴ� Recv, Send�� Ƚ��
	//------------------------
	LONG _RecvPacketTPS;
	LONG _SendPacketTPS;

	CNetworkLib();

	//------------------------------------------------------------
	// ��ӹ��� Ŭ�������� �Ҹ��ڸ� ȣ���ϱ� ���� virtual�� �ٿ���
	//------------------------------------------------------------
	virtual ~CNetworkLib();

	void SendPacket(__int64 SessionID, CMessage* Packet);
	void Disconnect(__int64 SessionID);

	//---------------------------------------
	//���� ���� �Լ�
	//---------------------------------------
	bool Start(const WCHAR* OpenIP, int Port);

	//----------------
	//���� ���� �Լ�
	//----------------
	void Stop();
};