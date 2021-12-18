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

	// SessionArray 미리 준비
	for (int SessionCount = SERVER_SESSION_MAX - 1; SessionCount >= 0; SessionCount--)
	{
		_SessionArray[SessionCount] = new st_Session;
		_SessionArray[SessionCount]->IOBlock = (st_IOBlock*)_aligned_malloc(sizeof(st_IOBlock), 16);
		memset(&_SessionArray[SessionCount]->SendPacket, 0, sizeof(CMessage*) * 500);
		_SessionArray[SessionCount]->IOBlock->IOCount = 0;
		_SessionArray[SessionCount]->SendPacketCount = 0;
		// 미리 준비되어 있는 세션의 Index를 넣어줌
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
	//세션 ID를 이용해 세션이 저장되어 있는 세션 인덱스를 가져온다.
	int SessionIndex = GET_SESSIONINDEX(SessionID);

	// 이미 릴리즈 작업중이거나 작업중일 예정인 세션이라면 그냥 나간다.
	if (_SessionArray[SessionIndex]->IOBlock->IsRelease == 1)
	{		
		return nullptr;
	}

	/*
		안전하게 증가를 시켰는데 IOCount의 값이 1이라면 릴리즈 작업중이거나 릴리즈를 작업할 예정인 세션(0에서 증가 한 것이니까)이므로
		확인 작업을 해준다. 위에서 Increment로 증가시켜줬으니까 1을 다시 감소시켜서 그값이 0이라면 ReleaseSession을 호출해준다.
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
		찾을 세션의 세션 아이디가 입력받은 SessionID와 다르다면
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
	// FindSession을 통해 현재 세션을 사용하고 있다는 표시로 IOCount를 증가시켜주고
	// 함수 나가기전에 감소시켜준다.
	st_Session* SendSession = FindSession(SessionID);
	// 만약에 찾은 세션이 없다면 그냥 나간다.
	if (SendSession == nullptr)
	{
		return;
	}

	// 패킷에 헤더를 넣고 암호화 한 후 링버퍼에 패킷의 주소를 담는다.
	Packet->Encode();
	Packet->AddRetCount();

	SendSession->SendRingBuf.Enqueue(Packet);

	// WSASend 등록
	SendPost(SendSession);

	// FindSession에서 증가시켜준 IOCount를 줄여준다.
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
			IsSend를 SENDING_DO(true)로 바꾸면서 그전의 값이 SENDING_DO_NOT(false)인지 확인한다.
			그전의 값이 0(false)이라면 내가 바꾸면서 진입한것을 말하는데, 이것은 내가 직접 WSASend를 거는것을 의미하는것이고
			다른 워커 쓰레드가 일어나서 이쪽으로 진입해도 IsSend가 0이 될때까지는 WSASend작업을 하지 않게 막아준다.
		*/
		if (InterlockedExchange(&SendSession->IsSend, SENDING_DO) == SENDING_DO_NOT)
		{
			SendRingBufUseSize = SendSession->SendRingBuf.GetUseSize();
			/*
				보내려고 왓는데 다른 쓰레드에서 한꺼번에 보내버려서 보낼것이 없으면
				Send작업중을 취소하고 다시한번 데이터가 들어 있는지 확인한다.
				다시 한번 데이터가 들어 있는지 확인하는것은 다음과 같다.
				두번째 CAS ( IsSend를 flase로 바꾸는 부분 ) 이 실행되지 않고 다른 쓰레드가 일어나서 SendPost를 한다고 가정하자.
				SendPost를 한다는것은 들어온 데이터가 존재해서 WSASend를 건다는 것인데
				다른 쓰레드에서 첫번째 줄에서 조건이 맞지 않아 함수를 빠져나가고
				원래대로 돌아와도 ( Send할 데이터가 큐잉 되었지만 ) 그냥 빠져나가게 된다.
				따라서 한번더 데이터를 확인해주고 있으면 다시 올라가서 데이터를 보내게 해준다.
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

	// 해당 세션이 보내야할 패킷의 개수를 지정한다.
	SendSession->SendPacketCount = SendBufCount = SendRingBufUseSize;

	// 보내야할 패킷의 개수만큼 SendRingBuf에서 뽑아내서 WSABuf에 담는다.
	for (int i = 0; i < SendBufCount; i++)
	{		
		CMessage* Packet = nullptr;
		if (!SendSession->SendRingBuf.Dequeue(&Packet))
		{
			break;
		}

		// 패킷의 개수만큼 TPS 기록
		InterlockedIncrement(&_SendPacketTPS);

		SendBuf[i].buf = Packet->GetHeaderBufferPtr();
		SendBuf[i].len = Packet->GetUseBufferSize();

		// Send완료되면 반납하기위해 보낼 패킷을 보관해둔다.
		SendSession->SendPacket[i] = Packet;
	}

	// Send걸기전에 SendOverlapped를 초기화해준다.
	memset(&SendSession->SendOverlapped, 0, sizeof(OVERLAPPED));

	// IO Count를 증가시켜준다.
	InterlockedIncrement64(&SendSession->IOBlock->IOCount);
	
	int WSASendRetval = WSASend(SendSession->ClientSock, SendBuf, SendBufCount, NULL, 0, (LPWSAOVERLAPPED)&SendSession->SendOverlapped, NULL);
	if (WSASendRetval == SOCKET_ERROR)
	{
		DWORD Error = WSAGetLastError();
		if (Error != ERROR_IO_PENDING)
		{
			//-------------------------------------------------------------------------
			//WSAENOBUFS(10055)
			//시스템에 버퍼 공간이 부족하거나 큐가 가득차서 소켓 작업을 할 수 없을때
			//-------------------------------------------------------------------------
			//wprintf(L"WSASend Fail %d\n", Error);
			if (Error == WSAENOBUFS)
			{
				//로그 남겨야함
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

	// 현재 RecvRingBuf에서 한번에 넣을수 있는 사이즈를 읽는다.
	int DirectEnqueSize = RecvSession->RecvRingBuf.GetDirectEnqueueSize();
	// 남아 있는 RecvRingBuf의 공간 사이즈를 읽는다.
	int RecvRingBufFreeSize = RecvSession->RecvRingBuf.GetFreeSize();

	// 만약 남아 있는 RecvRingBuf의 공간 사이즈가 한번에 넣을 수 있는 사이즈 보다 크다면
	// 현재 RecvRingBuf가 2개의 공간으로 나뉘어 있음을 확인 할 수 잇다.
	if (RecvRingBufFreeSize > DirectEnqueSize)
	{
		// WSARecvBuf의 개수를 2개로 삼고
		// 첫번째 Buf는 RecvRingBuf의 Rear주소 시작점과 DirectEnqueSize를
		RecvBufCount = 2;
		RecvBuf[0].buf = RecvSession->RecvRingBuf.GetRearBufferPtr();
		RecvBuf[0].len = DirectEnqueSize;
		
		// 두번째 Buf는 RecvRing의 처음 주소와 DirectEnqueSize를 뺀 값
		// 즉 앞 부분부터 시작되는 남은 공간의길이를 담아준다.
		RecvBuf[1].buf = RecvSession->RecvRingBuf.GetBufferPtr();
		RecvBuf[1].len = RecvRingBufFreeSize - DirectEnqueSize;
	}
	else
	{		
		// 그 외의 경우에는 Rear주소 시작점과 DirectEnqueSize를 넣는다.
		RecvBufCount = 1;
		RecvBuf[0].buf = RecvSession->RecvRingBuf.GetRearBufferPtr();
		RecvBuf[0].len = DirectEnqueSize;
	}

	// Recv걸기 전에 RecvOverlapped를 초기화 시켜준다.
	memset(&RecvSession->RecvOverlapped, 0, sizeof(OVERLAPPED));

	/*
		IO Count를 증가시키고 WSARecv를 건다.
		WSARecv를 걸고 IO Count를 증가시키면 WSARecv가 성공했을때 완료통지가 즉각적으로 바로와서
		다른 쓰레드가 깨어난후 IO Count를 감소시킬때 음수가 나올수도 있는 상황이 생긴다.
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
			//시스템에 버퍼 공간이 부족하거나 큐가 가득차서 소켓 작업을 할 수 없을때
			//-------------------------------------------------------------------------
			//wprintf(L"WSARecv Fail %d\n", Error);
			if (Error == WSAENOBUFS)
			{
				//따로 로그 남겨주는 작업 필요
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
	// RecvRingBuf의 Rear값을 완료 통지 받은 Transferred 값만큼 뒤로 민다.
	// Recv 완료통지를 받으면 메세지 단위로 뽑아내어 한번에 다 처리해주고 WSARecv를 다시 건다.		
	// ---------------------------------------------------------------------------------------
	RecvCompleteSession->RecvRingBuf.MoveRear(Transferred);	

	CMessage::st_ENCODE_HEADER EncodeHeader;
	CMessage* Packet = CMessage::Alloc();

	for (;;)
	{
		Packet->Clear();

		//최소 헤더크기만큼은 데이터가 왔는지 확인한다.
		if (RecvCompleteSession->RecvRingBuf.GetUseSize() < sizeof(CMessage::st_ENCODE_HEADER))
		{
			break;
		}

		//헤더를 뽑아본다.
		RecvCompleteSession->RecvRingBuf.Peek((char*)&EncodeHeader, sizeof(CMessage::st_ENCODE_HEADER));
		if (EncodeHeader.PacketLen + sizeof(CMessage::st_ENCODE_HEADER) > RecvCompleteSession->RecvRingBuf.GetUseSize())
		{
			// 패킷코드 확인
			if (EncodeHeader.PacketCode != 119)
			{
				Disconnect(RecvCompleteSession->SessionId);
				break;
			}
			else
			{				
				// 패킷코드는 맞는데 PacketLen의 크기를 의도적으로 이상하게 보낼 경우 
				// 다시 크기 정해줌
				EncodeHeader.PacketLen = RecvCompleteSession->RecvRingBuf.GetUseSize();
			}
		}

		InterlockedIncrement(&_RecvPacketTPS);

		// 헤더 크기 만큼 RecvRingBuf _Front 움직이기 ( 헤더 확인 했으므로 )
		RecvCompleteSession->RecvRingBuf.MoveFront(sizeof(CMessage::st_ENCODE_HEADER));
		// 패킷 길이 만큼 RecvRingBuf에서 뽑아서 Packet에 넣기
		RecvCompleteSession->RecvRingBuf.Dequeue(Packet->GetRearBufferPtr(), EncodeHeader.PacketLen);
		// 헤더 셋팅해주기
		Packet->SetHeader((char*)&EncodeHeader, sizeof(CMessage::st_ENCODE_HEADER));
		// _Rear 움직여주기
		Packet->MoveWritePosition(EncodeHeader.PacketLen);

		//디코딩시 에러가 나면 해당 세션 종료
		if (!Packet->Decode())
		{
			Disconnect(RecvCompleteSession->SessionId);
			break;
		}

		OnRecv(RecvCompleteSession->SessionId, Packet);
	}

	// 패킷 반납
	Packet->Free(); 

	// WSARecv 걸기
	RecvPost(RecvCompleteSession);
}

void CNetworkLib::SendNotifyComplete(st_Session* SendCompleteSession)
{
	//-------------------------------------------------------------
	//	완료 통지된 패킷 정리
	//	해당 세션이 보낸 패킷의 개수만큼
	//  메모리풀로 반납한다.	
	//-------------------------------------------------------------
	for (int i = 0; i < SendCompleteSession->SendPacketCount; i++)
	{
		SendCompleteSession->SendPacket[i]->Free();
	}

	SendCompleteSession->SendPacketCount = 0;

	//-------------------------------------------------------------
	//	SendFlag를 false로 바꿔서 WSASend를 걸수 있게 해준다.
	//-------------------------------------------------------------
	InterlockedExchange(&SendCompleteSession->IsSend, SENDING_DO_NOT);
	//-----------------------------------------------------
	// Send 버퍼의 크기가 0보다 크다면 
	// WSASend를 걸어준다.
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
	//리슨소켓 생성
	_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_ListenSock == INVALID_SOCKET)
	{
		wprintf_s(L"NetworkLib Start ListenSock Create Fail\n");
		return false;
	}

	//바인드
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

	//소켓 송신버퍼 크기를 0으로 설정
	//int Optval;
	//setsockopt(_ListenSock, SOL_SOCKET, SO_SNDBUF, (char*)&Optval, 0);

	//리슨
	int ListenResult = listen(_ListenSock, SOMAXCONN);
	if (ListenResult == SOCKET_ERROR)
	{
		return false;
	}

	//IO Port 생성
	_HCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (_HCP == NULL)
	{
		DWORD Error = WSAGetLastError();
		wprintf(L"CreateIoCompletionPort %d\n", Error);
	}

	SYSTEM_INFO SI;
	GetSystemInfo(&SI);

	//Accept 쓰레드
	HANDLE hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThreadProc, this, 0, NULL);
	CloseHandle(hAcceptThread);

	// 워커 쓰레드 생성 코어 개수 만큼
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

#pragma region Worker 쓰레드
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
							
				// MyOverlapped가 nullptr일때는 내부적으로 IOCP의 에러가 난것으로
				// Error값을 확인하고 워커 쓰레드를 종료해준다.
				if (MyOverlapped == nullptr)
				{
					DWORD GQCSError = WSAGetLastError();
					wprintf(L"MyOverlapped Null %d\n", GQCSError);
					return -1;
				}
				
				//---------------------------------------------------------------------------
				// Transferred가 0이 왔다는 것은 클라에서 FIN 패킷을 보냈다는 것을 의미하므로
				// 우선 ClientSocket을 INVALID_SOCKET으로 바꿔서 send를 막고
				// 예약되어 있었던 IO 작업들에 대해 CancelIoEx를 이용해 모두 취소시킨다.
				//---------------------------------------------------------------------------
				if (Transferred == 0)
				{					
					NotifyCompleteSession->ClientSock = INVALID_SOCKET;
					CancelIoEx((HANDLE)NotifyCompleteSession->CloseSock, NULL);
					break;
				}

				/*
					Recv 통지 완료
				*/
				if (MyOverlapped == &NotifyCompleteSession->RecvOverlapped)
				{
					Instance->RecvNotifyComplete(NotifyCompleteSession, Transferred);
				}
				/*
					Send 통지 완료
				*/
				else if (MyOverlapped == &NotifyCompleteSession->SendOverlapped)
				{
					Instance->SendNotifyComplete(NotifyCompleteSession);
				}
			} while (0);

			//IO Count가 0일 때 릴리즈
			if (InterlockedDecrement64(&NotifyCompleteSession->IOBlock->IOCount) == 0)
			{
				Instance->ReleaseSession(NotifyCompleteSession);
			}
		}
	}

	return 0;
}
#pragma endregion

#pragma region Accept 쓰레드
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

			//오버랩 초기화 
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
				소켓의 송신용 버퍼를 0으로 둬서 해당 소켓으로 데이터를 WSASend할시 비동기로 처리하도록 유도한다.
			*/
			//setsockopt(NewSession->ClientSock, SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(optval));

			HANDLE SocketIORet = CreateIoCompletionPort((HANDLE)NewSession->ClientSock, Instance->_HCP, (ULONG_PTR)NewSession, 0);

			//------------------------------------------------------------------------------------------------------------------------------
			// IOCount를 증가시켜주고 RecvPost를 건다.
			// IOCount를 증가시키지 않고 RecvPost를 걸면 새로 할당 받은 Session에 대해 Closesocket을 할 가능성이 생긴다.
			// 상황은 다음과 같다.
			// 우선 하나의 세션에 대해서 ReleaseSession이 호출되어서 IOCount와 IsRelease를 검사하는 부분에서 쓰레드 소유권이 넘어가서 멈추고
			// 컨텐츠에서 FindSession을 이용해 해당 Session을 찾아 냈을 경우에 진입하여 IOCount를 증가시키지만 이전 값이 0이기 때문에 
			// ReleaseSession을 한번 더 호출하게 된다.
			// 즉, A 라는 Session에 대해 ReleaseSession이 2번 호출된 상황인데, 
			// 첫번째 호출한 쓰레드에서는 현재 IOCount와 IsRelease를 검사하는 부분에서 멈춰있고,
			// 두번째 호출한 쓰레드에서 A Session에 대해 Release를 진행하고, Index를 계산해서 해당 Session을 반납까지 진행 한다.
			//
			// 이때 새로운 클라가 접속을 하면 해당 Index를 빼내서 초기화 작업을 하고 IsRelease가 0이 되는 순간,
			// 새로운 클라가 할당 받은 Session과 첫번째 호출한 쓰레드가 소유하고 있는 Session은 같은 위치를 바라보고 있는 상황이고,
			// 이와 같은 상황에서 첫번째 호출한 쓰레드로 소유권이 넘어가서 실행을 하게 되면,
			// IOCount와 IsRelease가 둘다 0 이기 때문에 통과해서 Release를 진행하고 해당 Session을 반납까지 하게 되는 상황이 생긴다.
			// 따라서 IsRelease를 0으로 초기화 하기 전에 IOCount를 1 증가시켜서, 다른 쓰레드에서 해당 Session에 대해 Release 대기 하고 잇더라도
			// IOCount가 1 이기때문에 Release를 방지할 수 있도록 한다.
			//---------------------------------------------------------------------------------------------------------------------------------

			Instance->_SessionArray[NewSessionIndex]->IOBlock->IOCount++;
			Instance->_SessionArray[NewSessionIndex]->IOBlock->IsRelease = 0;

			Instance->_SessionID++;
			//Instance->OnConnectionRequest(*ClientIP,ClientAddr.sin_port);
			Instance->OnClientJoin(NewSession->SessionId);			
			// 위에서 IOCount를 증가시켜 줫으므로 Accepct에 한해 Recv를 걸때 IOCount를 증가시키지 않는다.
			Instance->RecvPost(NewSession, true);			

			InterlockedIncrement64(&Instance->_SessionCount);
		}
	}

	return 0;
}
#pragma endregion

#pragma region 세션 할당 해제
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
		Release 하러 들어온 세션에대해서 더블 CAS를 통해
		IOCount ( 사용중인지 아닌지 확인 )와
		IsRelease ( Release를 한 세션 인지 확인 )를
		검사해준다.
	*/
	if (!InterlockedCompareExchange128((LONG64*)ReleaseSession->IOBlock, (LONG64)true, (LONG64)0, (LONG64*)&CompareBlock))
	{
		return;
	}

	ReleaseSession->RecvRingBuf.ClearBuffer();

	/*
		SendPacket에는 성공하고 SendPost에는 실패한 경우
	*/

	//Release를 들어왔지만 SendPacket은 걸릴 가능성이 있다.
	//만약 SendPacket에서 Enque만 하고 그냥 빠진다면 샌드 버퍼에 내용이 남아 있을 수 있으므로 여기서 한번더 검사해서 반납한다.
	while (!ReleaseSession->SendRingBuf.IsEmpty())
	{
		CMessage* DeletePacket = nullptr;
		ReleaseSession->SendRingBuf.Dequeue(&DeletePacket);
		DeletePacket->Free();
	}

	/*
		SendPost까지는 성공햇지만 Release를 탄 경우
		SendPost까지 진입해서 Alloc을 받고 세션이 보낼 패킷을 저장해둔 SendPacket 배열에 기록은 해뒀지만
		Release로 진입해서 해제를 못하고 남아있을 경우 여기서 한번더 확인해서 최종적으로 반납한다.
	*/
	for (int i = 0; i < ReleaseSession->SendPacketCount; i++)
	{
		ReleaseSession->SendPacket[i]->Free();
		ReleaseSession->SendPacket[i] = nullptr;
	}

	ReleaseSession->SendPacketCount = 0;

	/*
		그럼에도 불구하고 남아있다면 로그를 찍어둔다.
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
