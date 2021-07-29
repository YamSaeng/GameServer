#include "pch.h"
#include "GameServer.h"
#include "DBBind.h"
#include "DBConnection.h"
#include "DBConnectionPool.h"
#include <process.h>

CGameServer::CGameServer()
{
	//timeBeginPeriod(1);
	_UpdateThread = nullptr;
	//Nonsignaled상태 자동리셋
	_UpdateWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	_WorkThreadEnd = false;

	//_JobMemoryPool = new CObjectPoolFreeList<st_JOB>(0);
	//_ClientMemoryPool = new CObjectPoolFreeList< st_CLIENT>(0);

	_JobMemoryPool = new CMemoryPoolTLS<st_JOB>(0);
	_ClientMemoryPool = new CMemoryPoolTLS<st_CLIENT>(0);

	_UpdateTPS = 0;
	_UpdateWakeCount = 0;
}

CGameServer::~CGameServer()
{
	CloseHandle(_UpdateWakeEvent);
	//timeEndPeriod(1);
}

void CGameServer::Start(const WCHAR* OpenIP, int32 Port)
{
	CNetworkLib::Start(OpenIP, Port);
	_UpdateThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThreadProc, this, 0, NULL);
	CloseHandle(_UpdateThread);
}

unsigned __stdcall CGameServer::UpdateThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_WorkThreadEnd)
	{
		WaitForSingleObject(Instance->_UpdateWakeEvent, INFINITE);

		Instance->_UpdateWakeCount++;

		while (!Instance->_ChatServerMessageQue.IsEmpty())
		{
			st_JOB* Job = nullptr;

			if (!Instance->_ChatServerMessageQue.Dequeue(&Job))
			{
				break;
			}

			switch (Job->Type)
			{
			case NEW_CLIENT_JOIN:
				Instance->CreateNewClient(Job->SessionID);
				break;
			case DISCONNECT_CLIENT:
				Instance->DeleteClient(Job->SessionID);
				break;
			case MESSAGE:
				Instance->PacketProc(Job->SessionID, Job->Message);
				break;
			default:
				Instance->Disconnect(Job->SessionID);
				break;
			}

			Instance->_UpdateTPS++;
			Instance->_JobMemoryPool->Free(Job);
		}
	}
	return 0;
}

unsigned __stdcall CGameServer::HeartBeatCheckThreadProc(void* Argument)
{
	return 0;
}

void CGameServer::CreateNewClient(int64 SessionID)
{
	st_CLIENT* NewClient = _ClientMemoryPool->Alloc();
	NewClient->SessionID = SessionID;
	NewClient->AccountNo = 0;

	memset(NewClient->ClientID, 0, sizeof(NewClient->ClientID));
	memset(NewClient->NickName, 0, sizeof(NewClient->NickName));

	NewClient->SectorX = -1;
	NewClient->SectorY = -1;

	NewClient->IsLogin = false;

	memset(NewClient->SessionKey, 0, sizeof(NewClient->SessionKey));

	NewClient->RecvPacketTime = timeGetTime();

	_ClientMap.insert(pair<int64, st_CLIENT*>(NewClient->SessionID, NewClient));

	CMessage* ResClientConnectedMessage = MakePacketResClientConnected();	
	SendPacket(NewClient->SessionID, ResClientConnectedMessage);
	ResClientConnectedMessage->Free();
}

void CGameServer::DeleteClient(int64 SessionID)
{
	// 받은 세션 아이디로 클라이언트 찾아서 메모리풀에 반환 및 맵에서 삭제
	st_CLIENT* Client = nullptr;
	unordered_map<int64, st_CLIENT*>::iterator ClientFindIterator;
	ClientFindIterator = _ClientMap.find(SessionID);
	if (ClientFindIterator != _ClientMap.end())
	{
		Client = (*ClientFindIterator).second;

		if (Client->SectorX != -1 && Client->SectorY != -1)
		{
			_SectorList[Client->SectorY][Client->SectorX].remove(Client->SessionID);
		}

		_ClientMemoryPool->Free(Client);
		_ClientMap.erase(ClientFindIterator);
	}
	else
	{

	}
}

void CGameServer::PacketProc(int64 SessionID, CMessage* Message)
{
	WORD MessageType;
	*Message >> MessageType;

	switch (MessageType)
	{
	case en_PACKET_CS_GAME_REQ_LOGIN:
		PacketProcReqLogin(SessionID, Message);
		break;
	case en_PACKET_C2S_GAME_LOGIN:
		PacketProcReqLoginTest(SessionID, Message);
		break;
	case en_PACKET_CS_GAME_REQ_SECTOR_MOVE:
		PacketProcReqSectorMove(SessionID, Message);
		break;
	case en_PACKET_CS_GAME_REQ_MESSAGE:
		PacketProcReqMessage(SessionID, Message);
		break;
	case en_PACKET_CS_GAME_REQ_HEARTBEAT:
		PacketProcReqHeartBeat(SessionID, Message);
		break;
	default:
		Disconnect(SessionID);
		break;
	}

	Message->Free();
}

//----------------------------------------------------------------------------
//로그인 요청
//INT64 AccountNo
//WCHAR ID[20]	 //null 포함
//WCHAR NickName[20] //null 포함
//char SessionKey[64]
//----------------------------------------------------------------------------
void CGameServer::PacketProcReqLogin(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);

	int64 AccountNo;
	BYTE Status = LOGIN_SUCCESS;

	*Message >> AccountNo;

	if (Client != nullptr)
	{
		if (Client->AccountNo != 0 && Client->AccountNo != AccountNo)
		{
			Disconnect(Client->SessionID);
			return;
		}

		////중복 로그인 확인
		//map<int64, st_CLIENT*>::iterator ClientFindIterator;	

		//for (ClientFindIterator = _ClientMap.begin(); ClientFindIterator != _ClientMap.end(); ++ClientFindIterator)
		//{
		//	//중복 로그인 판단되면 끊어버리기
		//	if ((*ClientFindIterator).second->AccountNo == AccountNo)
		//	{				
		//		if ((*ClientFindIterator).second->SessionID != SessionID)
		//		{
		//			Disconnect((*ClientFindIterator).second->SessionID);
		//			break;
		//		}				
		//	}
		//}

		// AccountDB 접근해서 세션키를 이용해서 Account를 찾은 후 찾았으면 로그인 처리				

		//AccountNo 셋팅
		Client->AccountNo = AccountNo;
		//Client ID 셋팅
		Message->GetData(Client->ClientID, sizeof(WCHAR) * 20);
		//Client NickName 셋팅
		Message->GetData(Client->NickName, sizeof(WCHAR) * 20);
		//세션키 셋팅
		Message->GetData(Client->SessionKey, sizeof(char) * 64);

		Client->IsLogin = true;
	}
	else
	{
		//저장되어 잇는 클라가 없음
		Status = LOGIN_FAIL;
	}

	CMessage* LoginMessage = MakePacketResLogin(Client->AccountNo, Status);
	SendPacket(Client->SessionID, LoginMessage);
	LoginMessage->Free();
}

//----------------------------------------------------------------------------
// 로그인 요청
// int AccountID
// int Token
//----------------------------------------------------------------------------
void CGameServer::PacketProcReqLoginTest(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);

	int32 AccountID;
	int32 Token;

	*Message >> AccountID;
	*Message >> Token;

	if (Client != nullptr)
	{
		if (Client->AccountNo != 0 && Client->AccountNo != AccountID)
		{
			Disconnect(Client->SessionID);
			return;
		}

		Client->AccountNo = AccountID;
		CDBConnection* DBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);		
		const wchar_t* AccountTokenGet = L"{CALL dbo.spInsertGold(?,?,?)}";
		//CDBBind(CDBConnection & DBConnection, const WCHAR * Query)
		CDBBind<0, 2> DBBind(*DBConnection,AccountTokenGet);
		DBBind.BindParam<int32>(0, AccountID);

		int OutAccountId;
		DBBind.BindCol<int32>(0, OutAccountId);
		int OutToken;
		DBBind.BindCol<int32>(1, OutToken);

		DBBind.Execute();


	}
	else
	{

	}


	// DB 접근해서 Token 확인해야함	
}

//---------------------------------------------------------------------------------
//섹터 이동 요청
//WORD Type
//INT64 AccountNo
//WORD SectorX
//WORD SectorY
//---------------------------------------------------------------------------------
void CGameServer::PacketProcReqSectorMove(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);
	int64 AccountNo;
	WORD SectorX;
	WORD SectorY;

	//클라가 로그인 중인지 판단
	if (!Client->IsLogin)
	{
		Disconnect(Client->SessionID);
		return;
	}

	*Message >> AccountNo;

	if (Client->AccountNo != AccountNo)
	{
		Disconnect(Client->SessionID);
		return;
	}

	*Message >> SectorX;
	*Message >> SectorY;

	//섹터 X Y 좌표 범위 검사
	if (SectorX < 0 || SectorX >= SECTOR_X_MAX || SectorY < 0 || SectorY >= SECTOR_Y_MAX)
	{
		Disconnect(Client->SessionID);
		return;
	}

	//기존 섹터에서 클라 제거
	if (Client->SectorX != -1 && Client->SectorY != -1)
	{
		_SectorList[Client->SectorY][Client->SectorX].remove(Client->SessionID);
	}

	Client->SectorY = SectorY;
	Client->SectorX = SectorX;

	_SectorList[Client->SectorY][Client->SectorX].push_back(Client->SessionID);

	CMessage* SectorMoveResMessage = MakePacketResSectorMove(Client->AccountNo, Client->SectorX, Client->SectorY);
	SendPacket(Client->SessionID, SectorMoveResMessage);
	SectorMoveResMessage->Free();
}

//---------------------------------------------------------------------------------
//채팅 보내기 요청
//WORD	Type
//INT64 AccountNo
//WORD MessageLen
//WCHAR Message[MessageLen/2] // null 미포함
//---------------------------------------------------------------------------------
void CGameServer::PacketProcReqMessage(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);
	int64 AccountNo;
	int MessageLen = 0;
	WORD CSMessageLen;
	WCHAR CSMessage[1024];

	memset(CSMessage, 0, sizeof(CSMessage));

	if (!Client->IsLogin)
	{
		Disconnect(Client->SessionID);
		return;
	}

	*Message >> AccountNo;
	if (Client->AccountNo != AccountNo)
	{
		Disconnect(Client->SessionID);
		return;
	}

	*Message >> CSMessageLen;
	MessageLen = Message->GetData(CSMessage, CSMessageLen);
	if (MessageLen != CSMessageLen)
	{
		Disconnect(Client->SessionID);
		return;
	}

	/**Message >> CSMessageLen;
	int MessageUseBufSize = Message->GetUseBufferSize();

	if (MessageUseBufSize - 5 != CSMessageLen)
	{
		Disconnect(Client->SessionID);
		return;
	}

	Message->GetData(CSMessage, CSMessageLen);	*/

	CMessage* ChattingResMessage = nullptr;
	ChattingResMessage = MakePacketResMessage(Client->AccountNo, Client->ClientID, Client->NickName, CSMessageLen, CSMessage);
	SendPacketAround(Client, ChattingResMessage, true);
	ChattingResMessage->Free();
}

//---------------------------------------------------------------------------------
//하트 비트
//WORD Type
//---------------------------------------------------------------------------------
void CGameServer::PacketProcReqHeartBeat(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);

	Client->RecvPacketTime = timeGetTime();
}

CMessage* CGameServer::MakePacketResClientConnected()
{
	CMessage* ClientConnetedMessage = CMessage::Alloc();
	if (ClientConnetedMessage == nullptr)
	{
		CRASH("ClientConnectdMessage가 nullptr");
	}

	ClientConnetedMessage->Clear();

	*ClientConnetedMessage << (uint16)en_PACKET_S2C_GAME_CLIENT_CONNECTED;	

	return ClientConnetedMessage;
}

//---------------------------------------------------------------
//로그인 요청 응답 패킷 만들기 함수
//WORD Type
//BYTE Status  //0 : 실패  1 : 성공
//INT64 AccountNo
//---------------------------------------------------------------
CMessage* CGameServer::MakePacketResLogin(int64 AccountNo, BYTE Status)
{
	CMessage* LoginMessage = CMessage::Alloc();
	if (LoginMessage == nullptr)
	{
		return nullptr;
	}

	LoginMessage->Clear();

	*LoginMessage << (WORD)en_PACKET_CS_GAME_RES_LOGIN;
	*LoginMessage << Status;
	*LoginMessage << AccountNo;

	return LoginMessage;
}

//-----------------------------------------------------------------
//섹터 이동 요청 응답 패킷 만들기 함수
//WORD Type
//INT64 AccountNo
//WORD SectorX
//WORD SectorY
//-----------------------------------------------------------------
CMessage* CGameServer::MakePacketResSectorMove(int64 AccountNo, WORD SectorX, WORD SectorY)
{
	CMessage* SectorMoveMessage = CMessage::Alloc();
	if (SectorMoveMessage == nullptr)
	{
		return nullptr;
	}

	SectorMoveMessage->Clear();

	*SectorMoveMessage << (WORD)en_PACKET_CS_GAME_RES_SECTOR_MOVE;
	*SectorMoveMessage << AccountNo;
	*SectorMoveMessage << SectorX;
	*SectorMoveMessage << SectorY;

	return SectorMoveMessage;
}

//-------------------------------------------------------
//채팅 보내기 요청 응답 패킷 만들기 함수
//WORD	Type
//INT64	AccountNo
//WCHAR	ID[20]	// null 포함
//WCHAR	Nickname[20] // null 포함
//WORD	MessageLen
//WCHAR	Message[MessageLen / 2] // null 미포함
//-------------------------------------------------------
CMessage* CGameServer::MakePacketResMessage(int64 AccountNo, WCHAR* ID, WCHAR* NickName, WORD MessageLen, WCHAR* Message)
{
	CMessage* ChattingMessage = CMessage::Alloc();
	if (ChattingMessage == nullptr)
	{
		return nullptr;
	}

	ChattingMessage->Clear();

	*ChattingMessage << (WORD)en_PACKET_CS_GAME_RES_MESSAGE;
	*ChattingMessage << AccountNo;
	ChattingMessage->InsertData(ID, sizeof(WCHAR) * 20);
	ChattingMessage->InsertData(NickName, sizeof(WCHAR) * 20);
	*ChattingMessage << MessageLen;
	ChattingMessage->InsertData(Message, sizeof(WCHAR) * (MessageLen / 2));

	return ChattingMessage;
}

st_CLIENT* CGameServer::FindClient(int64 SessionID)
{
	unordered_map<int64, st_CLIENT*>::iterator ClientFindIterator;
	ClientFindIterator = _ClientMap.find(SessionID);
	if (ClientFindIterator != _ClientMap.end())
	{
		return (*ClientFindIterator).second;
	}
	else
	{
		return nullptr;
	}
}

void CGameServer::GetSectorAround(int16 SectorX, int16 SectorY, st_SECTOR_AROUND* SectorAround)
{
	//왼쪽 위 지점부터 구하기 위해 좌표 변경
	SectorX--;
	SectorY--;

	SectorAround->Count = 0;

	for (int32 Y = 0; Y < 3; Y++)
	{
		if (SectorY + Y < 0 || SectorY + Y >= SECTOR_Y_MAX)
		{
			continue;
		}

		for (int32 X = 0; X < 3; X++)
		{
			if (SectorX + X < 0 || SectorX + X >= SECTOR_X_MAX)
			{
				continue;
			}

			SectorAround->Around[SectorAround->Count].X = SectorX + X;
			SectorAround->Around[SectorAround->Count].Y = SectorY + Y;
			SectorAround->Count++;
		}
	}
}

void CGameServer::OnClientJoin(int64 SessionID)
{
	st_JOB* ClientJoinJob = _JobMemoryPool->Alloc();
	ClientJoinJob->Type = NEW_CLIENT_JOIN;
	ClientJoinJob->SessionID = SessionID;
	ClientJoinJob->Message = nullptr;
	_ChatServerMessageQue.Enqueue(ClientJoinJob);
	SetEvent(_UpdateWakeEvent);
}

void CGameServer::OnRecv(int64 SessionID, CMessage* Packet)
{
	st_JOB* NewMessageJob = _JobMemoryPool->Alloc();
	CMessage* JobMessage = CMessage::Alloc();
	JobMessage->Clear();
	JobMessage->SetHeader(Packet->GetBufferPtr(), sizeof(CMessage::st_ENCODE_HEADER));
	JobMessage->InsertData(Packet->GetFrontBufferPtr(), Packet->GetUseBufferSize() - sizeof(CMessage::st_ENCODE_HEADER));

	NewMessageJob->Type = MESSAGE;
	NewMessageJob->SessionID = SessionID;
	NewMessageJob->Message = JobMessage;
	_ChatServerMessageQue.Enqueue(NewMessageJob);
	SetEvent(_UpdateWakeEvent);
}

void CGameServer::OnClientLeave(int64 SessionID)
{
	st_JOB* ClientLeaveJob = _JobMemoryPool->Alloc();
	ClientLeaveJob->Type = DISCONNECT_CLIENT;
	ClientLeaveJob->SessionID = SessionID;
	ClientLeaveJob->Message = nullptr;
	_ChatServerMessageQue.Enqueue(ClientLeaveJob);
	SetEvent(_UpdateWakeEvent);
}

bool CGameServer::OnConnectionRequest(const wchar_t ClientIP, int32 Port)
{
	return false;
}

//클라 중심으로 9방향 클라들에게 메세지를 뿌린다.
void CGameServer::SendPacketAround(st_CLIENT* Client, CMessage* Message, bool SendMe)
{
	st_SECTOR_AROUND AroundSector;
	GetSectorAround(Client->SectorX, Client->SectorY, &AroundSector);

	list<int64>::iterator SectorIterator;
	for (int32 i = 0; i < AroundSector.Count; i++)
	{
		for (SectorIterator = _SectorList[AroundSector.Around[i].Y][AroundSector.Around[i].X].begin();
			SectorIterator != _SectorList[AroundSector.Around[i].Y][AroundSector.Around[i].X].end();
			++SectorIterator)
		{
			if (SendMe == true)
			{
				SendPacket((*SectorIterator), Message);
			}
			else
			{
				if (Client->SessionID != (*SectorIterator))
				{
					SendPacket((*SectorIterator), Message);
				}
			}
		}
	}

}

void CGameServer::SendPacketBroadcast(CMessage* Message)
{
	st_CLIENT* Client = nullptr;
	unordered_map<int64, st_CLIENT*>::iterator ClientIterator;

	for (ClientIterator = _ClientMap.begin(); ClientIterator != _ClientMap.end(); ++ClientIterator)
	{
		SendPacket((*ClientIterator).first, Message);
	}
}