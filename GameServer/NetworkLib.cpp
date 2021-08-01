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
	//SYSLOG_DIRECTORY(L"NetWorkLibLog");
	//SYSLOG_LEVEL(LOG::LEVEL_ERROR);
	for (int SessionCount = SERVER_SESSION_MAX - 1; SessionCount >= 0; SessionCount--)
	{
		_SessionArray[SessionCount] = new st_SESSION;
		_SessionArray[SessionCount]->IOBlock = (st_IO_BLOCK*)_aligned_malloc(sizeof(st_IO_BLOCK), 16);
		memset(&_SessionArray[SessionCount]->SendPacket, 0, sizeof(CMessage*) * 500);
		_SessionArray[SessionCount]->IOBlock->IOCount = 0;
		_SessionArray[SessionCount]->SendPacketCount = 0;
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

void CNetworkLib::SendPacket(__int64 SessionID, CMessage* Packet)
{
	// FindSession�� ���� ���� ������ ����ϰ� �ִٴ� ǥ�÷� IOCount�� ���������ְ�
	// �Լ� ���������� ���ҽ����ش�.
	st_SESSION* SendSession = FindSession(SessionID);
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

void CNetworkLib::SendPost(st_SESSION* SendSession)
{
	int SendRingBufUseSize;
	int SendBufCount = 0;
	WSABUF SendBuf[200];

	do
	{
		/*
			IsSend�� 1(true)�� �ٲٸ鼭 ������ ���� 0(false)���� Ȯ���Ѵ�.
			������ ���� 0(false)�̶�� ���� �ٲٸ鼭 �����Ѱ��� ���ϴµ�, �̰��� ���� ���� WSASend�� �Ŵ°��� �ǹ��ϴ°��̰�
			�ٸ� ��Ŀ �����尡 �Ͼ�� �������� �����ص� IsSend�� 0�� �ɶ������� WSASend�۾��� ���� �ʰ� �����ش�.
		*/
		if (InterlockedExchange(&SendSession->IsSend, SENDING_DO) == SENDING_DO_NOT)
		{
			/*
				UseBufferSize�� ���� ���� �� DirectDequeSize�� ���Ѵ�.
				DirectDequeSize�� ���� ���ϰ� �Ǹ�
			*/

			//DirectDequeSize = SendSession->SendRingBuf.GetDirectDequeueSize();
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
			if (SendRingBufUseSize /*DirectDequeSize */ == 0)
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

	if (SendSession->IsExit == 1)
	{
		return;
	}

	SendSession->SendPacketCount = SendBufCount = SendRingBufUseSize;

	for (int i = 0; i < SendBufCount; i++)
	{
		CMessage* Packet = nullptr;
		if (!SendSession->SendRingBuf.Dequeue(&Packet))
		{
			break;
		}

		InterlockedIncrement(&_SendPacketTPS);

		SendBuf[i].buf = Packet->GetHeaderBufferPtr();
		SendBuf[i].len = Packet->GetUseBufferSize();

		SendSession->SendPacket[i] = Packet;
	}

	memset(&SendSession->SendOverlapped, 0, sizeof(OVERLAPPED));

	InterlockedIncrement64(&SendSession->IOBlock->IOCount);

	if (SendSession->ClientSock == INVALID_SOCKET)
	{
		//wprintf(L"SendPost ClientSocket is INVALID_SOCKET!! IOCount %d SendSession IsExit %d \n",SendSession->IOCount,SendSession->IsExit);		
	}

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

void CNetworkLib::RecvPost(st_SESSION* RecvSession, bool IsAcceptRecvPost)
{
	int RecvBufCount = 0;
	WSABUF RecvBuf[2];

	int DirectEnqueSize = RecvSession->RecvRingBuf.GetDirectEnqueueSize();
	int RecvRingBufFreeSize = RecvSession->RecvRingBuf.GetFreeSize();

	if (RecvRingBufFreeSize > DirectEnqueSize)
	{
		RecvBufCount = 2;
		RecvBuf[0].buf = RecvSession->RecvRingBuf.GetRearBufferPtr();
		RecvBuf[0].len = DirectEnqueSize;

		RecvBuf[1].buf = RecvSession->RecvRingBuf.GetBufferPtr();
		RecvBuf[1].len = RecvRingBufFreeSize - DirectEnqueSize;
	}
	else
	{
		RecvBufCount = 1;
		RecvBuf[0].buf = RecvSession->RecvRingBuf.GetRearBufferPtr();
		RecvBuf[0].len = DirectEnqueSize;
	}

	if (RecvSession->IsExit == true)
	{
		return;
	}

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

void CNetworkLib::RecvNotifyComplete(st_SESSION* RecvCompleteSession, const DWORD& Transferred)
{
	/*
		RecvRingBuf�� Rear���� �Ϸ� ���� ���� Transferred ����ŭ �ڷ� �δ�.
		Recv �Ϸ������� ������ �޼��� ������ �̾Ƴ��� �ѹ��� �� ó�����ְ�
		WSARecv�� �ٽ� �Ǵ�.
	*/	

	RecvCompleteSession->RecvRingBuf.MoveRear(Transferred);
	
	if (RecvCompleteSession->RecvRingBuf.GetUseSize() < Transferred)
	{
		CRASH("RecvRingBuf�� �� �ִ� �����ͺ��� Transfereed�� �� ŭ");
		Disconnect(RecvCompleteSession->SessionID);
		return;
	}

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
			if (EncodeHeader.PacketCode != 119)
			{
				Disconnect(RecvCompleteSession->SessionID);
				break;
			}
			else
			{
				EncodeHeader.PacketLen = RecvCompleteSession->RecvRingBuf.GetUseSize();
			}
		}

		InterlockedIncrement(&_RecvPacketTPS);

		RecvCompleteSession->RecvRingBuf.MoveFront(sizeof(CMessage::st_ENCODE_HEADER));

		RecvCompleteSession->RecvRingBuf.Dequeue(Packet->GetRearBufferPtr(), EncodeHeader.PacketLen);
		Packet->SetHeader((char*)&EncodeHeader, sizeof(CMessage::st_ENCODE_HEADER));
		Packet->MoveWritePosition(EncodeHeader.PacketLen);

		//���ڵ��� ������ ���� �ش� ���� ����
		if (!Packet->Decode())
		{
			Disconnect(RecvCompleteSession->SessionID);
			break;
		}

		OnRecv(RecvCompleteSession->SessionID, Packet);
	}

	// ��Ŷ �ݳ�
	Packet->Free(); 

	if (!RecvCompleteSession->IsCancelIO)
	{
		RecvPost(RecvCompleteSession);
	}
}

void CNetworkLib::SendNotifyComplete(st_SESSION* SendCompleteSession)
{
	/*
		//�Ϸ� ������ ��Ŷ ����//
		�ش� ������ ���� ��Ŷ�� ������ �̾Ƶΰ� ����
		Send�����ۿ��� ����ȭ������ �ּҸ� �̾Ƴ� �Ŀ� ������Ű��
		������ 1�� �����ϸ鼭 �ݺ��Ѵ�.
	*/
	for (int i = 0; i < SendCompleteSession->SendPacketCount; i++)
	{
		SendCompleteSession->SendPacket[i]->Free();
	}

	SendCompleteSession->SendPacketCount = 0;

	/*
		SendFlag�� false�� �ٲ㼭 WSASend�� �ɼ� �ְ� ���ش�.
	*/
	InterlockedExchange(&SendCompleteSession->IsSend, SENDING_DO_NOT);

	if (SendCompleteSession->SendRingBuf.GetUseSize() > 0)
	{
		SendPost(SendCompleteSession);
	}
}

void CNetworkLib::Disconnect(__int64 SessionID)
{
	st_SESSION* DisconnectSession = FindSession(SessionID);

	if (DisconnectSession == nullptr)
	{
		return;
	}

	//Disconnect �������� ���� Ȯ��
	if (InterlockedExchange(&DisconnectSession->IsCancelIO, 1) != 0)
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
		DisconnectSession->ClientSock = INVALID_SOCKET;
		DisconnectSession->IsExit = true;
		CancelIoEx((HANDLE)DisconnectSession->CloseSock, NULL);
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

	//����͸� ���� ��¿� ������
	//HANDLE hServerStatePrintThread = (HANDLE)_beginthreadex(NULL, 0, ServerStatePrintProc, this, 0, NULL);
	//CloseHandle(hServerStatePrintThread);

	//��Ŀ ������ ����
	for (int i = 0; i < (int)SI.dwNumberOfProcessors * 2; i++)
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
		st_SESSION* NotifyCompleteSession = nullptr;
		for (;;)
		{
			DWORD Transferred = 0;
			OVERLAPPED* MyOverlapped = nullptr;
			int CompleteRet;
			DWORD GQCSError;

			do
			{
				CompleteRet = GetQueuedCompletionStatus(Instance->_HCP, &Transferred, (PULONG_PTR)&NotifyCompleteSession, (LPOVERLAPPED*)&MyOverlapped, INFINITE);

				/*
					MyOverlapped�� nullptr�϶��� ���������� IOCP�� ������ ��������
					Error���� Ȯ���ϰ� ��Ŀ �����带 �������ش�.
				*/
				if (CompleteRet == 0)
				{
					GQCSError = WSAGetLastError();
				}
				if (MyOverlapped == nullptr)
				{
					DWORD Error = WSAGetLastError();
					wprintf(L"MyOverlapped Null %d\n", Error);
					return -1;
				}

				/*
					Transferred�� 0�̶�� ���� Ŭ���ʿ��� FIN( ���� ��Ŷ )�� �����ٴ� ������ ó������ closesocket�� ȣ���Ͽ�����,
					���� ���� ������ ���Ͽ� �ǵ�ġ ���� ������ �߻��ߴ�. ���Ŀ� shutdown�� ȣ���ؼ� ���� ������ ������ ������, shutdown�� ����Ѵٸ�
					���� ������ �����̰ų� ���� �� �� ���� ��� WSARecv�� �ɷ������� FIN�� ���� �ʱ� ������ �ش� ������ ���� �������� ���� �� �ְ� �ȴ�.
					�׷��� ������ ������ ������ �����鼭 �Ϸ������� �ް� �Ϸ��� �켱 IO�� �ɷ��ִ°��� ��� �� ������ְ�
					do while�� ���������� WSASend WSARecv�۾��� �Ȱɰ� ���ָ� IOCount�� 0�̵ǰ� �����ؼ� ReleaseSession�� ȣ���� closesocket�� �ѹ��� ȣ�� �ǵ��� �����ϴ� ����� ����Ѵ�.
				*/
				if (Transferred == 0)
				{
					// �켱 0�� �� ���� ���Ͽ� INVALID_SOCKET�� �־ send, recv�� ������ ��
					// ������ ����Ǿ� �־��� IO �۾��鿡 ���� CancelIo�� ȣ���Ͽ� ��ҽ�Ų��.
					InterlockedExchange(&NotifyCompleteSession->ClientSock, INVALID_SOCKET);
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
			st_SESSION* NewSession;
			int NewSessionIndex;

			Instance->_SessionArrayIndexs.Pop(&NewSessionIndex);
			NewSession = Instance->_SessionArray[NewSessionIndex];
			memset(&NewSession->RecvOverlapped, 0, sizeof(OVERLAPPED));
			memset(&NewSession->SendOverlapped, 0, sizeof(OVERLAPPED));
			//memset(&NewSession->DebugArray, 0, sizeof(NewSession->DebugArray));
			//NewSession->DebugArrayCount = 0;
			NewSession->SessionID = ADD_SESSIONID_INDEX(Instance->_SessionID, NewSessionIndex);
			NewSession->ClientAddr = ClientAddr;
			NewSession->ClientSock = ClientSock;
			NewSession->CloseSock = ClientSock;
			NewSession->IsCloseSocket = CLOSE_SOCKET_DO_NOT;
			NewSession->IsSend = SENDING_DO_NOT;
			NewSession->IsCancelIO = 0;
			NewSession->IsExit = 0;
			NewSession->IsSend = SENDING_DO_NOT;

			//Instance->_SessionArray[NewSessionIndex]->SendPacketCount = 0;

			//int optval = 0;
			/*
				������ �۽ſ� ���۸� 0���� �ּ� �ش� �������� �����͸� WSASend�ҽ� �񵿱�� ó���ϵ��� �����Ѵ�.
			*/
			//setsockopt(NewSession->ClientSock, SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(optval));

			HANDLE SocketIORet = CreateIoCompletionPort((HANDLE)NewSession->ClientSock, Instance->_HCP, (ULONG_PTR)NewSession, 0);

			Instance->_SessionArray[NewSessionIndex]->IOBlock->IOCount++;
			Instance->_SessionArray[NewSessionIndex]->IOBlock->IsRelease = 0;

			Instance->_SessionID++;
			//Instance->OnConnectionRequest(*ClientIP,ClientAddr.sin_port);
			Instance->OnClientJoin(NewSession->SessionID);
			//memcpy(&NewSession->DebugArray[NewSession->DebugArrayCount], "ACCJ ", 5);
			//NewSession->DebugArrayCount += 5;
			// ������ IOCount�� �������� �Z���Ƿ� Accepct�� ���� Recv�� �ɶ� IOCount�� ������Ű�� �ʴ´�.
			Instance->RecvPost(NewSession, true);
			//memcpy(&NewSession->DebugArray[NewSession->DebugArrayCount], "ACRP ", 5);
			//NewSession->DebugArrayCount += 5;

			InterlockedIncrement64(&Instance->_SessionCount);
		}
	}

	return 0;
}
#pragma endregion

#pragma region ���� �Ҵ� ����
void CNetworkLib::ReleaseSession(st_SESSION* ReleaseSession)
{
	__int64 ReleaseSessionId = ReleaseSession->SessionID;
	char* pRecvOverlapped = (char*)&ReleaseSession->RecvOverlapped;
	char* pSendOverlapped = (char*)&ReleaseSession->SendOverlapped;
	int ReleaseSessionIOcount = ReleaseSession->IOBlock->IOCount;

	st_IO_BLOCK CompareBlock;
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

	/*
		�̹� closesocket �۾��� �� ������ �ƴ��� �Ǵ�
		1�� �ٲٷ� �޴µ� 0�̸� closesocket�۾��� ���� ���� ���̹Ƿ� closesocket����
	*/
	if (InterlockedExchange(&ReleaseSession->IsCloseSocket, CLOSE_SOCKET_DO) == CLOSE_SOCKET_DO_NOT)
	{
		InterlockedExchange(&ReleaseSession->ClientSock, INVALID_SOCKET);
		closesocket(ReleaseSession->CloseSock);
	}

	OnClientLeave(ReleaseSession->SessionID);

	__int64 Index = GET_SESSIONINDEX(ReleaseSession->SessionID);
	_SessionArrayIndexs.Push(GET_SESSIONINDEX(ReleaseSession->SessionID));

	InterlockedDecrement64(&_SessionCount);
	if (ReleaseSessionId != ReleaseSession->SessionID)
	{
		//wprintf(L"ReleaseSession SessionID Different !! ReleaseSessionID %d ReleaseSession->SessionID %d\n", ReleaseSessionId, ReleaseSession->SessionID);
	}
}
#pragma endregion

st_SESSION* CNetworkLib::FindSession(__int64 SessionID)
{
	//���� ID�� �̿��� ������ ����Ǿ� �ִ� ���� �ε����� �����´�.
	int SessionIndex = GET_SESSIONINDEX(SessionID);

	/*
		�̹� ������ �۾����̰ų� �۾����� ������ �����̶�� �׳� ������.
	*/
	if (_SessionArray[SessionIndex]->IOBlock->IsRelease == true ||
		_SessionArray[SessionIndex]->IsCancelIO == true)
	{
		//LOG(L"ERROR", en_LOG_LEVEL::LEVEL_ERROR, L"Func SendPacket SendSession is Release!!!!!!");
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
	if (SessionID != _SessionArray[SessionIndex]->SessionID)
	{
		if (InterlockedDecrement64(&_SessionArray[SessionIndex]->IOBlock->IOCount) == 0)
		{
			ReleaseSession(_SessionArray[SessionIndex]);
		}

		return nullptr;
	}

	return _SessionArray[SessionIndex];
}

void CNetworkLib::ReturnSession(st_SESSION* ReturnSession)
{
	if (InterlockedDecrement64(&ReturnSession->IOBlock->IOCount) == 0)
	{
		ReleaseSession(ReturnSession);
	}
}

