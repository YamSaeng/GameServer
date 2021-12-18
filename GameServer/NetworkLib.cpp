#include "pch.h"
#include "NetworkLib.h"
#include <WS2tcpip.h>
#include <process.h>

CNetworkLib::CNetworkLib()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		DWORD Error = WSAGetLastError();
		wprintf(L"WSAStartup Error %d\n", Error);
	}

	// SessionArray �̸� �غ�
	for (int SessionCount = SERVER_SESSION_MAX - 1; SessionCount >= 0; SessionCount--)
	{
		_SessionArray[SessionCount] = new st_Session;
		_SessionArray[SessionCount]->IOBlock = (st_IOBlock*)_aligned_malloc(sizeof(st_IOBlock), 16);
		memset(&_SessionArray[SessionCount]->SendPacket, 0, sizeof(CMessage*) * 500);
		_SessionArray[SessionCount]->IOBlock->IOCount = 0;
		_SessionArray[SessionCount]->SendPacketCount = 0;
		// �̸� �غ�Ǿ� �ִ� ������ Index�� �־���
		_SessionArrayIndexs.Push(SessionCount);
	}

	_SessionID = 1;

	_AcceptTotal = 0;
	_AcceptTPS = 0;
	_RecvPacketTPS = 0;
	_SendPacketTPS = 0;
}

CNetworkLib::~CNetworkLib()
{

}

st_Session* CNetworkLib::FindSession(__int64 SessionID)
{
	//���� ID�� �̿��� ������ ����Ǿ� �ִ� ���� �ε����� �����´�.
	int SessionIndex = GET_SESSIONINDEX(SessionID);

	// �̹� ������ �۾����̰ų� �۾����� ������ �����̶�� �׳� ������.
	if (_SessionArray[SessionIndex]->IOBlock->IsRelease == 1)
	{		
		return nullptr;
	}

	/*
		�����ϰ� ������ ���״µ� IOCount�� ���� 1�̶�� ������ �۾����̰ų� ����� �۾��� ������ ����(0���� ���� �� ���̴ϱ�)�̹Ƿ�
		Ȯ�� �۾��� ���ش�. ������ Increment�� �������������ϱ� 1�� �ٽ� ���ҽ��Ѽ� �װ��� 0�̶�� ReleaseSession�� ȣ�����ش�.
	*/
	if (InterlockedIncrement64(&_SessionArray[SessionIndex]->IOBlock->IOCount) == 1)
	{
		if (InterlockedDecrement64(&_SessionArray[SessionIndex]->IOBlock->IOCount) == 0)
		{
			//LOG(L"ERROR", en_LOG_LEVEL::LEVEL_ERROR, L"IOCount is 0!!!!!!");
			ReleaseSession(_SessionArray[SessionIndex]);
		}

		return nullptr;
	}

	/*
		ã�� ������ ���� ���̵� �Է¹��� SessionID�� �ٸ��ٸ�
	*/
	if (SessionID != _SessionArray[SessionIndex]->SessionId)
	{
		if (InterlockedDecrement64(&_SessionArray[SessionIndex]->IOBlock->IOCount) == 0)
		{
			ReleaseSession(_SessionArray[SessionIndex]);
		}

		return nullptr;
	}

	return _SessionArray[SessionIndex];
}

void CNetworkLib::ReturnSession(st_Session* Session)
{
	if (InterlockedDecrement64(&Session->IOBlock->IOCount) == 0)
	{
		ReleaseSession(Session);
	}
}

void CNetworkLib::SendPacket(__int64 SessionID, CMessage* Packet)
{
	// FindSession�� ���� ���� ������ ����ϰ� �ִٴ� ǥ�÷� IOCount�� ���������ְ�
	// �Լ� ���������� ���ҽ����ش�.
	st_Session* SendSession = FindSession(SessionID);
	// ���࿡ ã�� ������ ���ٸ� �׳� ������.
	if (SendSession == nullptr)
	{
		return;
	}

	// ��Ŷ�� ����� �ְ� ��ȣȭ �� �� �����ۿ� ��Ŷ�� �ּҸ� ��´�.
	Packet->Encode();
	Packet->AddRetCount();

	SendSession->SendRingBuf.Enqueue(Packet);

	// WSASend ���
	SendPost(SendSession);

	// FindSession���� ���������� IOCount�� �ٿ��ش�.
	ReturnSession(SendSession);
}

void CNetworkLib::SendPost(st_Session* SendSession)
{
	int SendRingBufUseSize;
	int SendBufCount = 0;
	WSABUF SendBuf[500];

	do
	{
		/*
			IsSend�� SENDING_DO(true)�� �ٲٸ鼭 ������ ���� SENDING_DO_NOT(false)���� Ȯ���Ѵ�.
			������ ���� 0(false)�̶�� ���� �ٲٸ鼭 �����Ѱ��� ���ϴµ�, �̰��� ���� ���� WSASend�� �Ŵ°��� �ǹ��ϴ°��̰�
			�ٸ� ��Ŀ �����尡 �Ͼ�� �������� �����ص� IsSend�� 0�� �ɶ������� WSASend�۾��� ���� �ʰ� �����ش�.
		*/
		if (InterlockedExchange(&SendSession->IsSend, SENDING_DO) == SENDING_DO_NOT)
		{
			SendRingBufUseSize = SendSession->SendRingBuf.GetUseSize();
			/*
				�������� �Ӵµ� �ٸ� �����忡�� �Ѳ����� ���������� �������� ������
				Send�۾����� ����ϰ� �ٽ��ѹ� �����Ͱ� ��� �ִ��� Ȯ���Ѵ�.
				�ٽ� �ѹ� �����Ͱ� ��� �ִ��� Ȯ���ϴ°��� ������ ����.
				�ι�° CAS ( IsSend�� flase�� �ٲٴ� �κ� ) �� ������� �ʰ� �ٸ� �����尡 �Ͼ�� SendPost�� �Ѵٰ� ��������.
				SendPost�� �Ѵٴ°��� ���� �����Ͱ� �����ؼ� WSASend�� �Ǵٴ� ���ε�
				�ٸ� �����忡�� ù��° �ٿ��� ������ ���� �ʾ� �Լ��� ����������
				������� ���ƿ͵� ( Send�� �����Ͱ� ť�� �Ǿ����� ) �׳� ���������� �ȴ�.
				���� �ѹ��� �����͸� Ȯ�����ְ� ������ �ٽ� �ö󰡼� �����͸� ������ ���ش�.
			*/
			if (SendRingBufUseSize == 0)
			{
				InterlockedExchange(&SendSession->IsSend, SENDING_DO_NOT);

				if (!SendSession->SendRingBuf.IsEmpty())
				{
					continue;
				}
				else
				{
					return;
				}
			}
		}
		else
		{
			return;
		}
	} while (0);

	// �ش� ������ �������� ��Ŷ�� ������ �����Ѵ�.
	SendSession->SendPacketCount = SendBufCount = SendRingBufUseSize;

	// �������� ��Ŷ�� ������ŭ SendRingBuf���� �̾Ƴ��� WSABuf�� ��´�.
	for (int i = 0; i < SendBufCount; i++)
	{		
		CMessage* Packet = nullptr;
		if (!SendSession->SendRingBuf.Dequeue(&Packet))
		{
			break;
		}

		// ��Ŷ�� ������ŭ TPS ���
		InterlockedIncrement(&_SendPacketTPS);

		SendBuf[i].buf = Packet->GetHeaderBufferPtr();
		SendBuf[i].len = Packet->GetUseBufferSize();

		// Send�Ϸ�Ǹ� �ݳ��ϱ����� ���� ��Ŷ�� �����صд�.
		SendSession->SendPacket[i] = Packet;
	}

	// Send�ɱ����� SendOverlapped�� �ʱ�ȭ���ش�.
	memset(&SendSession->SendOverlapped, 0, sizeof(OVERLAPPED));

	// IO Count�� ���������ش�.
	InterlockedIncrement64(&SendSession->IOBlock->IOCount);
	
	int WSASendRetval = WSASend(SendSession->ClientSock, SendBuf, SendBufCount, NULL, 0, (LPWSAOVERLAPPED)&SendSession->SendOverlapped, NULL);
	if (WSASendRetval == SOCKET_ERROR)
	{
		DWORD Error = WSAGetLastError();
		if (Error != ERROR_IO_PENDING)
		{
			//-------------------------------------------------------------------------
			//WSAENOBUFS(10055)
			//�ý��ۿ� ���� ������ �����ϰų� ť�� �������� ���� �۾��� �� �� ������
			//-------------------------------------------------------------------------
			//wprintf(L"WSASend Fail %d\n", Error);
			if (Error == WSAENOBUFS)
			{
				//�α� ���ܾ���
			}

			if (InterlockedDecrement64(&SendSession->IOBlock->IOCount) == 0)
			{
				ReleaseSession(SendSession);
			}
		}
	}
}

void CNetworkLib::RecvPost(st_Session* RecvSession, bool IsAcceptRecvPost)
{
	int RecvBufCount = 0;
	WSABUF RecvBuf[2];

	// ���� RecvRingBuf���� �ѹ��� ������ �ִ� ����� �д´�.
	int DirectEnqueSize = RecvSession->RecvRingBuf.GetDirectEnqueueSize();
	// ���� �ִ� RecvRingBuf�� ���� ����� �д´�.
	int RecvRingBufFreeSize = RecvSession->RecvRingBuf.GetFreeSize();

	// ���� ���� �ִ� RecvRingBuf�� ���� ����� �ѹ��� ���� �� �ִ� ������ ���� ũ�ٸ�
	// ���� RecvRingBuf�� 2���� �������� ������ ������ Ȯ�� �� �� �մ�.
	if (RecvRingBufFreeSize > DirectEnqueSize)
	{
		// WSARecvBuf�� ������ 2���� ���
		// ù��° Buf�� RecvRingBuf�� Rear�ּ� �������� DirectEnqueSize��
		RecvBufCount = 2;
		RecvBuf[0].buf = RecvSession->RecvRingBuf.GetRearBufferPtr();
		RecvBuf[0].len = DirectEnqueSize;
		
		// �ι�° Buf�� RecvRing�� ó�� �ּҿ� DirectEnqueSize�� �� ��
		// �� �� �κк��� ���۵Ǵ� ���� �����Ǳ��̸� ����ش�.
		RecvBuf[1].buf = RecvSession->RecvRingBuf.GetBufferPtr();
		RecvBuf[1].len = RecvRingBufFreeSize - DirectEnqueSize;
	}
	else
	{		
		// �� ���� ��쿡�� Rear�ּ� �������� DirectEnqueSize�� �ִ´�.
		RecvBufCount = 1;
		RecvBuf[0].buf = RecvSession->RecvRingBuf.GetRearBufferPtr();
		RecvBuf[0].len = DirectEnqueSize;
	}

	// Recv�ɱ� ���� RecvOverlapped�� �ʱ�ȭ �����ش�.
	memset(&RecvSession->RecvOverlapped, 0, sizeof(OVERLAPPED));

	/*
		IO Count�� ������Ű�� WSARecv�� �Ǵ�.
		WSARecv�� �ɰ� IO Count�� ������Ű�� WSARecv�� ���������� �Ϸ������� �ﰢ������ �ٷοͼ�
		�ٸ� �����尡 ����� IO Count�� ���ҽ�ų�� ������ ���ü��� �ִ� ��Ȳ�� �����.
	*/
	if (IsAcceptRecvPost == false)
	{
		InterlockedIncrement64(&RecvSession->IOBlock->IOCount);
	}

	DWORD Flags = 0;

	int WSARecvRetval = WSARecv(RecvSession->ClientSock, RecvBuf, RecvBufCount, NULL, &Flags, (LPWSAOVERLAPPED)&RecvSession->RecvOverlapped, NULL);
	if (WSARecvRetval == SOCKET_ERROR)
	{
		DWORD Error = WSAGetLastError();
		if (Error != ERROR_IO_PENDING)
		{
			//-------------------------------------------------------------------------
			//WSAENOBUFS(10055)
			//�ý��ۿ� ���� ������ �����ϰų� ť�� �������� ���� �۾��� �� �� ������
			//-------------------------------------------------------------------------
			//wprintf(L"WSARecv Fail %d\n", Error);
			if (Error == WSAENOBUFS)
			{
				//���� �α� �����ִ� �۾� �ʿ�
			}

			if (InterlockedDecrement64(&RecvSession->IOBlock->IOCount) == 0)
			{
				ReleaseSession(RecvSession);
			}
		}
	}
}

void CNetworkLib::RecvNotifyComplete(st_Session* RecvCompleteSession, const DWORD& Transferred)
{
	// ---------------------------------------------------------------------------------------
	// RecvRingBuf�� Rear���� �Ϸ� ���� ���� Transferred ����ŭ �ڷ� �δ�.
	// Recv �Ϸ������� ������ �޼��� ������ �̾Ƴ��� �ѹ��� �� ó�����ְ� WSARecv�� �ٽ� �Ǵ�.		
	// ---------------------------------------------------------------------------------------
	RecvCompleteSession->RecvRingBuf.MoveRear(Transferred);	

	CMessage::st_ENCODE_HEADER EncodeHeader;
	CMessage* Packet = CMessage::Alloc();

	for (;;)
	{
		Packet->Clear();

		//�ּ� ���ũ�⸸ŭ�� �����Ͱ� �Դ��� Ȯ���Ѵ�.
		if (RecvCompleteSession->RecvRingBuf.GetUseSize() < sizeof(CMessage::st_ENCODE_HEADER))
		{
			break;
		}

		//����� �̾ƺ���.
		RecvCompleteSession->RecvRingBuf.Peek((char*)&EncodeHeader, sizeof(CMessage::st_ENCODE_HEADER));
		if (EncodeHeader.PacketLen + sizeof(CMessage::st_ENCODE_HEADER) > RecvCompleteSession->RecvRingBuf.GetUseSize())
		{
			// ��Ŷ�ڵ� Ȯ��
			if (EncodeHeader.PacketCode != 119)
			{
				Disconnect(RecvCompleteSession->SessionId);
				break;
			}
			else
			{				
				// ��Ŷ�ڵ�� �´µ� PacketLen�� ũ�⸦ �ǵ������� �̻��ϰ� ���� ��� 
				// �ٽ� ũ�� ������
				EncodeHeader.PacketLen = RecvCompleteSession->RecvRingBuf.GetUseSize();
			}
		}

		InterlockedIncrement(&_RecvPacketTPS);

		// ��� ũ�� ��ŭ RecvRingBuf _Front �����̱� ( ��� Ȯ�� �����Ƿ� )
		RecvCompleteSession->RecvRingBuf.MoveFront(sizeof(CMessage::st_ENCODE_HEADER));
		// ��Ŷ ���� ��ŭ RecvRingBuf���� �̾Ƽ� Packet�� �ֱ�
		RecvCompleteSession->RecvRingBuf.Dequeue(Packet->GetRearBufferPtr(), EncodeHeader.PacketLen);
		// ��� �������ֱ�
		Packet->SetHeader((char*)&EncodeHeader, sizeof(CMessage::st_ENCODE_HEADER));
		// _Rear �������ֱ�
		Packet->MoveWritePosition(EncodeHeader.PacketLen);

		//���ڵ��� ������ ���� �ش� ���� ����
		if (!Packet->Decode())
		{
			Disconnect(RecvCompleteSession->SessionId);
			break;
		}

		OnRecv(RecvCompleteSession->SessionId, Packet);
	}

	// ��Ŷ �ݳ�
	Packet->Free(); 

	// WSARecv �ɱ�
	RecvPost(RecvCompleteSession);
}

void CNetworkLib::SendNotifyComplete(st_Session* SendCompleteSession)
{
	//-------------------------------------------------------------
	//	�Ϸ� ������ ��Ŷ ����
	//	�ش� ������ ���� ��Ŷ�� ������ŭ
	//  �޸�Ǯ�� �ݳ��Ѵ�.	
	//-------------------------------------------------------------
	for (int i = 0; i < SendCompleteSession->SendPacketCount; i++)
	{
		SendCompleteSession->SendPacket[i]->Free();
	}

	SendCompleteSession->SendPacketCount = 0;

	//-------------------------------------------------------------
	//	SendFlag�� false�� �ٲ㼭 WSASend�� �ɼ� �ְ� ���ش�.
	//-------------------------------------------------------------
	InterlockedExchange(&SendCompleteSession->IsSend, SENDING_DO_NOT);
	//-----------------------------------------------------
	// Send ������ ũ�Ⱑ 0���� ũ�ٸ� 
	// WSASend�� �ɾ��ش�.
	//-----------------------------------------------------
	if (SendCompleteSession->SendRingBuf.GetUseSize() > 0)
	{
		SendPost(SendCompleteSession);
	}
}

void CNetworkLib::Disconnect(__int64 SessionID)
{
	st_Session* DisconnectSession = FindSession(SessionID);
	if (DisconnectSession == nullptr)
	{
		ReturnSession(DisconnectSession);
		return;
	}		

	if (DisconnectSession->IOBlock->IOCount == 0)
	{
		ReleaseSession(DisconnectSession);
	}
	else
	{
		CancelIoEx((HANDLE)DisconnectSession->ClientSock, NULL);
	}

	ReturnSession(DisconnectSession);
}

bool CNetworkLib::Start(const WCHAR* OpenIP, int Port)
{
	//�������� ����
	_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_ListenSock == INVALID_SOCKET)
	{
		wprintf_s(L"NetworkLib Start ListenSock Create Fail\n");
		return false;
	}

	//���ε�
	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	InetPton(AF_INET, OpenIP, &ServerAddr.sin_addr);
	ServerAddr.sin_port = htons(Port);

	int BindResult = bind(_ListenSock, (SOCKADDR*)&ServerAddr, sizeof(SOCKADDR_IN));
	if (BindResult == SOCKET_ERROR)
	{
		return false;
	}

	//���� �۽Ź��� ũ�⸦ 0���� ����
	//int Optval;
	//setsockopt(_ListenSock, SOL_SOCKET, SO_SNDBUF, (char*)&Optval, 0);

	//����
	int ListenResult = listen(_ListenSock, SOMAXCONN);
	if (ListenResult == SOCKET_ERROR)
	{
		return false;
	}

	//IO Port ����
	_HCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (_HCP == NULL)
	{
		DWORD Error = WSAGetLastError();
		wprintf(L"CreateIoCompletionPort %d\n", Error);
	}

	SYSTEM_INFO SI;
	GetSystemInfo(&SI);

	//Accept ������
	HANDLE hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThreadProc, this, 0, NULL);
	CloseHandle(hAcceptThread);

	// ��Ŀ ������ ���� �ھ� ���� ��ŭ
	for (int i = 0; i < (int)SI.dwNumberOfProcessors; i++)
	{
		HANDLE hWorkerThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThreadProc, this, 0, NULL);
		CloseHandle(hWorkerThread);
	}

	return true;
}

void CNetworkLib::Stop()
{

}

#pragma region Worker ������
unsigned __stdcall CNetworkLib::WorkerThreadProc(void* Argument)
{
	CNetworkLib* Instance = (CNetworkLib*)Argument;

	if (Instance)
	{
		st_Session* NotifyCompleteSession = nullptr;
		for (;;)
		{
			DWORD Transferred = 0;
			OVERLAPPED* MyOverlapped = nullptr;
			int CompleteRet;
			DWORD GQCSError;

			do
			{
				CompleteRet = GetQueuedCompletionStatus(Instance->_HCP, &Transferred, (PULONG_PTR)&NotifyCompleteSession, (LPOVERLAPPED*)&MyOverlapped, INFINITE);
							
				// MyOverlapped�� nullptr�϶��� ���������� IOCP�� ������ ��������
				// Error���� Ȯ���ϰ� ��Ŀ �����带 �������ش�.
				if (MyOverlapped == nullptr)
				{
					DWORD GQCSError = WSAGetLastError();
					wprintf(L"MyOverlapped Null %d\n", GQCSError);
					return -1;
				}
				
				//---------------------------------------------------------------------------
				// Transferred�� 0�� �Դٴ� ���� Ŭ�󿡼� FIN ��Ŷ�� ���´ٴ� ���� �ǹ��ϹǷ�
				// �켱 ClientSocket�� INVALID_SOCKET���� �ٲ㼭 send�� ����
				// ����Ǿ� �־��� IO �۾��鿡 ���� CancelIoEx�� �̿��� ��� ��ҽ�Ų��.
				//---------------------------------------------------------------------------
				if (Transferred == 0)
				{					
					NotifyCompleteSession->ClientSock = INVALID_SOCKET;
					CancelIoEx((HANDLE)NotifyCompleteSession->CloseSock, NULL);
					break;
				}

				/*
					Recv ���� �Ϸ�
				*/
				if (MyOverlapped == &NotifyCompleteSession->RecvOverlapped)
				{
					Instance->RecvNotifyComplete(NotifyCompleteSession, Transferred);
				}
				/*
					Send ���� �Ϸ�
				*/
				else if (MyOverlapped == &NotifyCompleteSession->SendOverlapped)
				{
					Instance->SendNotifyComplete(NotifyCompleteSession);
				}
			} while (0);

			//IO Count�� 0�� �� ������
			if (InterlockedDecrement64(&NotifyCompleteSession->IOBlock->IOCount) == 0)
			{
				Instance->ReleaseSession(NotifyCompleteSession);
			}
		}
	}

	return 0;
}
#pragma endregion

#pragma region Accept ������
unsigned __stdcall CNetworkLib::AcceptThreadProc(void* Argument)
{
	CNetworkLib* Instance = (CNetworkLib*)Argument;

	if (Instance)
	{
		for (;;)
		{
			SOCKADDR_IN ClientAddr;
			int AddrLen = sizeof(ClientAddr);
			SOCKET ClientSock = accept(Instance->_ListenSock, (SOCKADDR*)&ClientAddr, &AddrLen);
			if (ClientSock == INVALID_SOCKET)
			{
				DWORD Error = WSAGetLastError();
				wprintf(L"Accept Error %d\n", Error);
				break;
			}

			Instance->_AcceptTotal++;
			Instance->_AcceptTPS++;

			WCHAR ClientIP[16] = { 0 };
			WCHAR ClientInfo[50] = { 0 };
			//InetNtop(AF_INET, &ClientAddr.sin_addr, ClientIP, 16);
			//Log(L"SYSTEM", en_LOG_LEVEL::LOG_SYSTEM_LEVEL, L"Connect Client IP : %s Port %d \n",ClientIP, ntohs(ClientAddr.sin_port));
			//wprintf(L"Connet Client IP : %s Port %d\n", ClientIP, ntohs(ClientAddr.sin_port));

			//������ �ʱ�ȭ 
			st_Session* NewSession;
			int NewSessionIndex;

			Instance->_SessionArrayIndexs.Pop(&NewSessionIndex);
			NewSession = Instance->_SessionArray[NewSessionIndex];
			memset(&NewSession->RecvOverlapped, 0, sizeof(OVERLAPPED));
			memset(&NewSession->SendOverlapped, 0, sizeof(OVERLAPPED));
			//memset(&NewSession->DebugArray, 0, sizeof(NewSession->DebugArray));
			//NewSession->DebugArrayCount = 0;
			NewSession->SessionId = ADD_SESSIONID_INDEX(Instance->_SessionID, NewSessionIndex);
			NewSession->ClientAddr = ClientAddr;
			NewSession->ClientSock = ClientSock;
			NewSession->CloseSock = ClientSock;
			NewSession->IsSend = SENDING_DO_NOT;

			//Instance->_SessionArray[NewSessionIndex]->SendPacketCount = 0;

			//int optval = 0;
			/*
				������ �۽ſ� ���۸� 0���� �ּ� �ش� �������� �����͸� WSASend�ҽ� �񵿱�� ó���ϵ��� �����Ѵ�.
			*/
			//setsockopt(NewSession->ClientSock, SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(optval));

			HANDLE SocketIORet = CreateIoCompletionPort((HANDLE)NewSession->ClientSock, Instance->_HCP, (ULONG_PTR)NewSession, 0);

			//------------------------------------------------------------------------------------------------------------------------------
			// IOCount�� ���������ְ� RecvPost�� �Ǵ�.
			// IOCount�� ������Ű�� �ʰ� RecvPost�� �ɸ� ���� �Ҵ� ���� Session�� ���� Closesocket�� �� ���ɼ��� �����.
			// ��Ȳ�� ������ ����.
			// �켱 �ϳ��� ���ǿ� ���ؼ� ReleaseSession�� ȣ��Ǿ IOCount�� IsRelease�� �˻��ϴ� �κп��� ������ �������� �Ѿ�� ���߰�
			// ���������� FindSession�� �̿��� �ش� Session�� ã�� ���� ��쿡 �����Ͽ� IOCount�� ������Ű���� ���� ���� 0�̱� ������ 
			// ReleaseSession�� �ѹ� �� ȣ���ϰ� �ȴ�.
			// ��, A ��� Session�� ���� ReleaseSession�� 2�� ȣ��� ��Ȳ�ε�, 
			// ù��° ȣ���� �����忡���� ���� IOCount�� IsRelease�� �˻��ϴ� �κп��� �����ְ�,
			// �ι�° ȣ���� �����忡�� A Session�� ���� Release�� �����ϰ�, Index�� ����ؼ� �ش� Session�� �ݳ����� ���� �Ѵ�.
			//
			// �̶� ���ο� Ŭ�� ������ �ϸ� �ش� Index�� ������ �ʱ�ȭ �۾��� �ϰ� IsRelease�� 0�� �Ǵ� ����,
			// ���ο� Ŭ�� �Ҵ� ���� Session�� ù��° ȣ���� �����尡 �����ϰ� �ִ� Session�� ���� ��ġ�� �ٶ󺸰� �ִ� ��Ȳ�̰�,
			// �̿� ���� ��Ȳ���� ù��° ȣ���� ������� �������� �Ѿ�� ������ �ϰ� �Ǹ�,
			// IOCount�� IsRelease�� �Ѵ� 0 �̱� ������ ����ؼ� Release�� �����ϰ� �ش� Session�� �ݳ����� �ϰ� �Ǵ� ��Ȳ�� �����.
			// ���� IsRelease�� 0���� �ʱ�ȭ �ϱ� ���� IOCount�� 1 �������Ѽ�, �ٸ� �����忡�� �ش� Session�� ���� Release ��� �ϰ� �մ���
			// IOCount�� 1 �̱⶧���� Release�� ������ �� �ֵ��� �Ѵ�.
			//---------------------------------------------------------------------------------------------------------------------------------

			Instance->_SessionArray[NewSessionIndex]->IOBlock->IOCount++;
			Instance->_SessionArray[NewSessionIndex]->IOBlock->IsRelease = 0;

			Instance->_SessionID++;
			//Instance->OnConnectionRequest(*ClientIP,ClientAddr.sin_port);
			Instance->OnClientJoin(NewSession->SessionId);			
			// ������ IOCount�� �������� �Z���Ƿ� Accepct�� ���� Recv�� �ɶ� IOCount�� ������Ű�� �ʴ´�.
			Instance->RecvPost(NewSession, true);			

			InterlockedIncrement64(&Instance->_SessionCount);
		}
	}

	return 0;
}
#pragma endregion

#pragma region ���� �Ҵ� ����
void CNetworkLib::ReleaseSession(st_Session* ReleaseSession)
{
	__int64 ReleaseSessionId = ReleaseSession->SessionId;
	char* pRecvOverlapped = (char*)&ReleaseSession->RecvOverlapped;
	char* pSendOverlapped = (char*)&ReleaseSession->SendOverlapped;
	int ReleaseSessionIOcount = ReleaseSession->IOBlock->IOCount;

	st_IOBlock CompareBlock;
	CompareBlock.IOCount = 0;
	CompareBlock.IsRelease = 0;

	//memcpy(&ReleaseSession->DebugArray[ReleaseSession->DebugArrayCount], "RSRS", 4);
	//ReleaseSession->DebugArrayCount += 4;

	/*
		Release �Ϸ� ���� ���ǿ����ؼ� ���� CAS�� ����
		IOCount ( ��������� �ƴ��� Ȯ�� )��
		IsRelease ( Release�� �� ���� ���� Ȯ�� )��
		�˻����ش�.
	*/
	if (!InterlockedCompareExchange128((LONG64*)ReleaseSession->IOBlock, (LONG64)true, (LONG64)0, (LONG64*)&CompareBlock))
	{
		return;
	}

	ReleaseSession->RecvRingBuf.ClearBuffer();

	/*
		SendPacket���� �����ϰ� SendPost���� ������ ���
	*/

	//Release�� �������� SendPacket�� �ɸ� ���ɼ��� �ִ�.
	//���� SendPacket���� Enque�� �ϰ� �׳� �����ٸ� ���� ���ۿ� ������ ���� ���� �� �����Ƿ� ���⼭ �ѹ��� �˻��ؼ� �ݳ��Ѵ�.
	while (!ReleaseSession->SendRingBuf.IsEmpty())
	{
		CMessage* DeletePacket = nullptr;
		ReleaseSession->SendRingBuf.Dequeue(&DeletePacket);
		DeletePacket->Free();
	}

	/*
		SendPost������ ���������� Release�� ź ���
		SendPost���� �����ؼ� Alloc�� �ް� ������ ���� ��Ŷ�� �����ص� SendPacket �迭�� ����� �ص�����
		Release�� �����ؼ� ������ ���ϰ� �������� ��� ���⼭ �ѹ��� Ȯ���ؼ� ���������� �ݳ��Ѵ�.
	*/
	for (int i = 0; i < ReleaseSession->SendPacketCount; i++)
	{
		ReleaseSession->SendPacket[i]->Free();
		ReleaseSession->SendPacket[i] = nullptr;
	}

	ReleaseSession->SendPacketCount = 0;

	/*
		�׷����� �ұ��ϰ� �����ִٸ� �α׸� ���д�.
	*/
	if (ReleaseSession->SendRingBuf.GetUseSize() > 0)
	{
		int SendPacketCompleteCount = ReleaseSession->SendRingBuf.GetUseSize();
		for (int i = 0; i < SendPacketCompleteCount; i++)
		{
			CMessage* DeletePacket = nullptr;
			ReleaseSession->SendRingBuf.Dequeue(&DeletePacket);
			DeletePacket->Free();
		}
	}	

	OnClientLeave(ReleaseSession);	

	InterlockedDecrement64(&_SessionCount);	
}
#pragma endregion
