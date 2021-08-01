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
	// FindSession을 통해 현재 세션을 사용하고 있다는 표시로 IOCount를 증가시켜주고
	// 함수 나가기전에 감소시켜준다.
	st_SESSION* SendSession = FindSession(SessionID);
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

void CNetworkLib::SendPost(st_SESSION* SendSession)
{
	int SendRingBufUseSize;
	int SendBufCount = 0;
	WSABUF SendBuf[200];

	do
	{
		/*
			IsSend를 1(true)로 바꾸면서 그전의 값이 0(false)인지 확인한다.
			그전의 값이 0(false)이라면 내가 바꾸면서 진입한것을 말하는데, 이것은 내가 직접 WSASend를 거는것을 의미하는것이고
			다른 워커 쓰레드가 일어나서 이쪽으로 진입해도 IsSend가 0이 될때까지는 WSASend작업을 하지 않게 막아준다.
		*/
		if (InterlockedExchange(&SendSession->IsSend, SENDING_DO) == SENDING_DO_NOT)
		{
			/*
				UseBufferSize를 먼저 구한 후 DirectDequeSize를 구한다.
				DirectDequeSize를 먼저 구하게 되면
			*/

			//DirectDequeSize = SendSession->SendRingBuf.GetDirectDequeueSize();
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

void CNetworkLib::RecvNotifyComplete(st_SESSION* RecvCompleteSession, const DWORD& Transferred)
{
	/*
		RecvRingBuf의 Rear값을 완료 통지 받은 Transferred 값만큼 뒤로 민다.
		Recv 완료통지를 받으면 메세지 단위로 뽑아내어 한번에 다 처리해주고
		WSARecv를 다시 건다.
	*/	

	RecvCompleteSession->RecvRingBuf.MoveRear(Transferred);
	
	if (RecvCompleteSession->RecvRingBuf.GetUseSize() < Transferred)
	{
		CRASH("RecvRingBuf에 들어가 있는 데이터보다 Transfereed가 더 큼");
		Disconnect(RecvCompleteSession->SessionID);
		return;
	}

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

		//디코딩시 에러가 나면 해당 세션 종료
		if (!Packet->Decode())
		{
			Disconnect(RecvCompleteSession->SessionID);
			break;
		}

		OnRecv(RecvCompleteSession->SessionID, Packet);
	}

	// 패킷 반납
	Packet->Free(); 

	if (!RecvCompleteSession->IsCancelIO)
	{
		RecvPost(RecvCompleteSession);
	}
}

void CNetworkLib::SendNotifyComplete(st_SESSION* SendCompleteSession)
{
	/*
		//완료 통지된 패킷 정리//
		해당 세션이 보낸 패킷의 개수를 뽑아두고 나서
		Send링버퍼에서 직렬화버퍼의 주소를 뽑아낸 후에 삭제시키고
		개수를 1씩 차감하면서 반복한다.
	*/
	for (int i = 0; i < SendCompleteSession->SendPacketCount; i++)
	{
		SendCompleteSession->SendPacket[i]->Free();
	}

	SendCompleteSession->SendPacketCount = 0;

	/*
		SendFlag를 false로 바꿔서 WSASend를 걸수 있게 해준다.
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

	//Disconnect 진행중인 것을 확인
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

	//모니터링 변수 출력용 쓰레드
	//HANDLE hServerStatePrintThread = (HANDLE)_beginthreadex(NULL, 0, ServerStatePrintProc, this, 0, NULL);
	//CloseHandle(hServerStatePrintThread);

	//워커 쓰레드 생성
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

#pragma region Worker 쓰레드
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
					MyOverlapped가 nullptr일때는 내부적으로 IOCP의 에러가 난것으로
					Error값을 확인하고 워커 쓰레드를 종료해준다.
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
					Transferred가 0이라는 것은 클라쪽에서 FIN( 종료 패킷 )을 보냇다는 것으로 처음에는 closesocket을 호출하였으나,
					소켓 재사용 문제로 인하여 의도치 않은 에러가 발생했다. 이후에 shutdown을 호출해서 소켓 재사용을 막으려 했지만, shutdown을 사용한다면
					만약 상대방이 먹통이거나 반응 할 수 없는 경우 WSARecv가 걸려있을때 FIN이 오지 않기 때문에 해당 세션이 유령 세션으로 남을 수 있게 된다.
					그렇기 때문에 소켓의 재사용을 막으면서 완료통지도 받게 하려면 우선 IO가 걸려있는것을 모두 다 취소해주고
					do while을 빠져나가서 WSASend WSARecv작업을 안걸게 해주며 IOCount가 0이되게 유도해서 ReleaseSession을 호출해 closesocket이 한번만 호출 되도록 유도하는 방법을 사용한다.
				*/
				if (Transferred == 0)
				{
					// 우선 0이 온 세션 소켓에 INVALID_SOCKET를 넣어서 send, recv를 막아준 후
					// 이전에 예약되어 있었던 IO 작업들에 대해 CancelIo를 호출하여 취소시킨다.
					InterlockedExchange(&NotifyCompleteSession->ClientSock, INVALID_SOCKET);
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
				소켓의 송신용 버퍼를 0으로 둬서 해당 소켓으로 데이터를 WSASend할시 비동기로 처리하도록 유도한다.
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
			// 위에서 IOCount를 증가시켜 줫으므로 Accepct에 한해 Recv를 걸때 IOCount를 증가시키지 않는다.
			Instance->RecvPost(NewSession, true);
			//memcpy(&NewSession->DebugArray[NewSession->DebugArrayCount], "ACRP ", 5);
			//NewSession->DebugArrayCount += 5;

			InterlockedIncrement64(&Instance->_SessionCount);
		}
	}

	return 0;
}
#pragma endregion

#pragma region 세션 할당 해제
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

	/*
		이미 closesocket 작업을 한 것인지 아닌지 판단
		1로 바꾸려 햇는데 0이면 closesocket작업을 하지 않은 것이므로 closesocket실행
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
	//세션 ID를 이용해 세션이 저장되어 있는 세션 인덱스를 가져온다.
	int SessionIndex = GET_SESSIONINDEX(SessionID);

	/*
		이미 릴리즈 작업중이거나 작업중일 예정인 세션이라면 그냥 나간다.
	*/
	if (_SessionArray[SessionIndex]->IOBlock->IsRelease == true ||
		_SessionArray[SessionIndex]->IsCancelIO == true)
	{
		//LOG(L"ERROR", en_LOG_LEVEL::LEVEL_ERROR, L"Func SendPacket SendSession is Release!!!!!!");
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

