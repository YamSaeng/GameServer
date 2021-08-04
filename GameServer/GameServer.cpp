#include "pch.h"
#include "GameServer.h"
#include "DBBind.h"
#include "DBConnection.h"
#include "DBConnectionPool.h"
#include "DBStoreProcedure.h"
#include "DataManager.h"
#include <process.h>

CGameServer::CGameServer()
{
	//timeBeginPeriod(1);
	_UpdateThread = nullptr;
	_DataBaseThread = nullptr;
	
	// Nonsignaled���� �ڵ�����
	_UpdateWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_DataBaseWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	_UpdateThreadEnd = false;
	_DataBaseThreadEnd = false;	

	_JobMemoryPool = new CMemoryPoolTLS<st_JOB>(0);
	_ClientMemoryPool = new CMemoryPoolTLS<st_CLIENT>(0);

	_UpdateTPS = 0;
	_UpdateWakeCount = 0;

	_GameObjectId = 0;
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
	_DataBaseThread = (HANDLE)_beginthreadex(NULL, 0, DataBaseThreadProc, this, 0, NULL);
	CloseHandle(_UpdateThread);
	CloseHandle(_DataBaseThread);
}

unsigned __stdcall CGameServer::UpdateThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_UpdateThreadEnd)
	{
		WaitForSingleObject(Instance->_UpdateWakeEvent, INFINITE);

		Instance->_UpdateWakeCount++;

		while (!Instance->_GameServerCommonMessageQue.IsEmpty())
		{
			st_JOB* Job = nullptr;

			if (!Instance->_GameServerCommonMessageQue.Dequeue(&Job))
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

unsigned __stdcall CGameServer::DataBaseThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_DataBaseThreadEnd)
	{
		WaitForSingleObject(Instance->_DataBaseWakeEvent, INFINITE);

		Instance->_DataBaseThreadWakeCount++;

		while (!Instance->_GameServerDataBaseMessageQue.IsEmpty())
		{
			st_JOB* Job = nullptr;

			if (!Instance->_GameServerDataBaseMessageQue.Dequeue(&Job))
			{
				break;
			}

			switch (Job->Type)
			{
			case DATA_BASE_ACCOUNT_CHECK:	
				Instance->PacketProcReqAccountCheck(Job->SessionID, Job->Message);
				break;	
			case DATA_BASE_CHARACTER_CHECK:
				Instance->PacketProcReqCreateCharacterNameCheck(Job->SessionID, Job->Message);
				break;
			}

			Instance->_DataBaseThreadTPS++;
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
	NewClient->AccountID = 0;

	memset(NewClient->ClientID, 0, sizeof(NewClient->ClientID));	

	NewClient->SectorX = -1;
	NewClient->SectorY = -1;

	NewClient->IsLogin = false;

	NewClient->Token = 0;

	NewClient->RecvPacketTime = timeGetTime();	

	memset(NewClient->MyPlayers, 0, sizeof(CPlayer*) * 5);
	NewClient->MyPlayer = nullptr;

	_ClientMap.insert(pair<int64, st_CLIENT*>(NewClient->SessionID, NewClient));

	CMessage* ResClientConnectedMessage = MakePacketResClientConnected();	
	SendPacket(NewClient->SessionID, ResClientConnectedMessage);
	ResClientConnectedMessage->Free();
}

void CGameServer::DeleteClient(int64 SessionID)
{
	// ���� ���� ���̵�� Ŭ���̾�Ʈ ã�Ƽ� �޸�Ǯ�� ��ȯ �� �ʿ��� ����
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
	case en_PACKET_C2S_GAME_REQ_LOGIN:
		PacketProcReqLogin(SessionID, Message);
		break;
	case en_PACKET_C2S_GAME_CREATE_CHARACTER:
		PacketProcReqCreateCharacter(SessionID, Message);
		break;
	case en_PACKET_C2S_GAME_ENTER:
		PacketProcReqEnterGame(SessionID, Message);
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
// �α��� ��û
// int AccountID
// WCHAR ID[20]
// WCHAR NickName[20]
// int Token
//----------------------------------------------------------------------------
void CGameServer::PacketProcReqLogin(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);

	int32 AccountID;
	int32 Token;
	
	if (Client != nullptr)
	{
		// AccountID ����
		*Message >> AccountID;
		Client->AccountID = AccountID;

		int32 IdLen;
		*Message >> IdLen;
		// Client ID ����
		Message->GetData(Client->ClientID, IdLen);

		// ��ū ����
		*Message >> Token;
		Client->Token = Token;

		if (Client->AccountID != 0 && Client->AccountID != AccountID)
		{
			Disconnect(Client->SessionID);
			return;
		}
		
		////�ߺ� �α��� Ȯ��
		//map<int64, st_CLIENT*>::iterator ClientFindIterator;	

		//for (ClientFindIterator = _ClientMap.begin(); ClientFindIterator != _ClientMap.end(); ++ClientFindIterator)
		//{
		//	//�ߺ� �α��� �ǴܵǸ� ���������
		//	if ((*ClientFindIterator).second->AccountNo == AccountNo)
		//	{				
		//		if ((*ClientFindIterator).second->SessionID != SessionID)
		//		{
		//			Disconnect((*ClientFindIterator).second->SessionID);
		//			break;
		//		}				
		//	}
		//}		

		st_JOB* DBAccountCheckJob = _JobMemoryPool->Alloc();
		DBAccountCheckJob->Type = DATA_BASE_ACCOUNT_CHECK;
		DBAccountCheckJob->SessionID = Client->SessionID;
		DBAccountCheckJob->Message = nullptr;

		_GameServerDataBaseMessageQue.Enqueue(DBAccountCheckJob);		
		SetEvent(_DataBaseWakeEvent);	
	}
	else
	{
		// �ش� Ŭ�� ����
		bool Status = LOGIN_FAIL;

		CMessage* LoginMessage = MakePacketResLogin(Status, 0, nullptr);
		SendPacket(Client->SessionID, LoginMessage);
		LoginMessage->Free();
	}	
}

void CGameServer::PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);

	if (Client)
	{
		// ĳ���� �̸� ����
		int32 IdLen;
		*Message >> IdLen;

		WCHAR CharacterName[20];	
		Message->GetData(CharacterName, IdLen);	

		// ĳ���� �̸� ����
		Client->CreateCharacterName = CharacterName;

		st_JOB* DBCharacterCheckJob = _JobMemoryPool->Alloc();		
		DBCharacterCheckJob->Type = DATA_BASE_CHARACTER_CHECK;
		DBCharacterCheckJob->SessionID = Client->SessionID;
		DBCharacterCheckJob->Message = nullptr;

		_GameServerDataBaseMessageQue.Enqueue(DBCharacterCheckJob);
		SetEvent(_DataBaseWakeEvent);
	}
	else
	{

	}
}

void CGameServer::PacketProcReqEnterGame(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);

	if (Client)
	{
		// �α��� ���� �ƴϸ� ������.
		if (!Client->IsLogin)
		{
			Disconnect(Client->SessionID);
			return;
		}

		int64 AccountNo;
		*Message >> AccountNo;

		if (Client->AccountID != AccountNo)
		{
			Disconnect(Client->SessionID);
			return;
		}

		int32 EnterGameCharacterNameLen;
		*Message >> EnterGameCharacterNameLen;

		WCHAR EnterGameCharacterName[20];
		Message->GetData(EnterGameCharacterName, EnterGameCharacterNameLen);
				
		// Ŭ�� ������ �ִ� ĳ�� �߿� ��Ŷ���� ���� ĳ���Ͱ� �ִ��� Ȯ���Ѵ�.
		for (int i = 0; i < 5; i++)
		{
			if (Client->MyPlayers[i]->_GameObjectInfo.ObjectName == EnterGameCharacterName)
			{
				Client->MyPlayer = Client->MyPlayers[i];
				break;
			}
		}

		// ä�� ����
	}
}

//---------------------------------------------------------------------------------
//���� �̵� ��û
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

	//Ŭ�� �α��� ������ �Ǵ�
	if (!Client->IsLogin)
	{
		Disconnect(Client->SessionID);
		return;
	}

	*Message >> AccountNo;

	if (Client->AccountID != AccountNo)
	{
		Disconnect(Client->SessionID);
		return;
	}

	*Message >> SectorX;
	*Message >> SectorY;

	//���� X Y ��ǥ ���� �˻�
	if (SectorX < 0 || SectorX >= SECTOR_X_MAX || SectorY < 0 || SectorY >= SECTOR_Y_MAX)
	{
		Disconnect(Client->SessionID);
		return;
	}

	//���� ���Ϳ��� Ŭ�� ����
	if (Client->SectorX != -1 && Client->SectorY != -1)
	{
		_SectorList[Client->SectorY][Client->SectorX].remove(Client->SessionID);
	}

	Client->SectorY = SectorY;
	Client->SectorX = SectorX;

	_SectorList[Client->SectorY][Client->SectorX].push_back(Client->SessionID);

	CMessage* SectorMoveResMessage = MakePacketResSectorMove(Client->AccountID, Client->SectorX, Client->SectorY);
	SendPacket(Client->SessionID, SectorMoveResMessage);
	SectorMoveResMessage->Free();
}

//---------------------------------------------------------------------------------
//ä�� ������ ��û
//WORD	Type
//INT64 AccountNo
//WORD MessageLen
//WCHAR Message[MessageLen/2] // null ������
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
	if (Client->AccountID != AccountNo)
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
	//ChattingResMessage = MakePacketResMessage(Client->AccountID, Client->ClientID, Client->NickName, CSMessageLen, CSMessage);
	SendPacketAround(Client, ChattingResMessage, true);
	ChattingResMessage->Free();
}

//---------------------------------------------------------------------------------
//��Ʈ ��Ʈ
//WORD Type
//---------------------------------------------------------------------------------
void CGameServer::PacketProcReqHeartBeat(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);

	Client->RecvPacketTime = timeGetTime();	
}

void CGameServer::PacketProcReqAccountCheck(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);
	bool Status = LOGIN_SUCCESS;

	if (Client)
	{
		__int64 ClientAccountID = Client->AccountID;
		int Token = Client->Token;

		// AccountServer�� �Է¹��� AccountID�� �ִ��� Ȯ��
	
		// AccountNo�� Token���� AccountServerDB �����ؼ� �����Ͱ� �ִ��� Ȯ��
		CDBConnection* TokenDBConnection = G_DBConnectionPool->Pop(en_DBConnect::TOKEN);
		
		SP::CDBAccountTokenGet AccountTokenGet(*TokenDBConnection);
		AccountTokenGet.InAccountID(ClientAccountID);
		
		int DBToken = 0;		
		TIMESTAMP_STRUCT LoginSuccessTime;
		TIMESTAMP_STRUCT TokenExpiredTime;

		AccountTokenGet.OutToken(DBToken);		
		AccountTokenGet.OutLoginsuccessTime(LoginSuccessTime);		
		AccountTokenGet.OutTokenExpiredTime(TokenExpiredTime);

		AccountTokenGet.Execute();

		AccountTokenGet.Fetch();
		
		G_DBConnectionPool->Push(en_DBConnect::TOKEN, TokenDBConnection);

		// DB ��ū�� Ŭ��κ��� �� ��ū�� ������ �α��� ��������
		if (Token == DBToken)
		{
			Client->IsLogin = true;
			// Ŭ�� �����ϰ� �ִ� �÷��̾���� DB�κ��� �ܾ�´�.
			CDBConnection* GameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			
			SP::CDBGameServerPlayersGet ClientPlayersGet(*GameServerDBConnection);
			ClientPlayersGet.InAccountID(ClientAccountID);

			int64 PlayerID;
			WCHAR PlayerName[100];
			int32 PlayerLevel;
			int32 PlayerCurrentHP;
			int32 PlayerMaxHP;
			int32 PlayerAttack;
			float PlayerSpeed;
			
			ClientPlayersGet.OutPlayerDBID(PlayerID);
			ClientPlayersGet.OutPlayerName(PlayerName);		
			ClientPlayersGet.OutLevel(PlayerLevel);
			ClientPlayersGet.OutCurrentHP(PlayerCurrentHP);
			ClientPlayersGet.OutMaxHP(PlayerMaxHP);
			ClientPlayersGet.OutAttack(PlayerAttack);
			ClientPlayersGet.OutSpeed(PlayerSpeed);

			ClientPlayersGet.Execute();

			int PlayerCount = 0;
			st_PlayerObjectInfo PlayerInfos[5];		
			
			while (ClientPlayersGet.Fetch())
			{								
				// �÷��̾� ���� ����
				PlayerInfos[PlayerCount].ObjectId = PlayerID;
				PlayerInfos[PlayerCount].ObjectName = PlayerName;
				PlayerInfos[PlayerCount].ObjectStatInfo.Level = PlayerLevel;
				PlayerInfos[PlayerCount].ObjectStatInfo.HP = PlayerCurrentHP;
				PlayerInfos[PlayerCount].ObjectStatInfo.MaxHP = PlayerMaxHP;
				PlayerInfos[PlayerCount].ObjectStatInfo.Attack = PlayerAttack;
				PlayerInfos[PlayerCount].ObjectStatInfo.Speed = PlayerSpeed;

				// �÷��̾� ���� ���� ĳ���� ���� �� ����
				CPlayer* Player = new CPlayer(PlayerInfos[PlayerCount]);
				Client->MyPlayers[PlayerCount] = Player;				
				PlayerCount++;
			}							
			
			// Ŭ�󿡰� �α��� ���� ��Ŷ ����
			CMessage* ResLoginMessage = MakePacketResLogin(Status, PlayerCount, PlayerInfos[0].ObjectName);
			SendPacket(Client->SessionID, ResLoginMessage);
			ResLoginMessage->Free();

			G_DBConnectionPool->Push(en_DBConnect::GAME, GameServerDBConnection);
		}
		else
		{
			Client->IsLogin = false;
			Disconnect(SessionID);
		}					
	}
	else
	{
		// Ŭ�� ���� ������ ���
	}
}

void CGameServer::PacketProcReqCreateCharacterNameCheck(int64 SessionID, CMessage* Message)
{
	int32 PlayerDBID;
	st_CLIENT* Client = FindClient(SessionID);

	if (Client)
	{
		wstring CreateCharacterName = Client->CreateCharacterName;

		CDBConnection* FindCharacterNameGameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

		SP::CDBGameServerCharacterNameGet CharacterNameGet(*FindCharacterNameGameServerDBConnection);
		CharacterNameGet.InCharacterName(CreateCharacterName);

		CharacterNameGet.Execute();

		bool CharacterNameFind = CharacterNameGet.Fetch();

		G_DBConnectionPool->Push(en_DBConnect::GAME, FindCharacterNameGameServerDBConnection);

		if (!CharacterNameFind)
		{
			// ���� 1�� �ش��ϴ� ĳ���� ���� �о��
			auto FindStatus = G_Datamanager->_Status.find(1);
			st_StatusData NewCharacterStatus = *(*FindStatus).second;

			CDBConnection* NewCharacterPushDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

			SP::CDBGameServerCreateCharacterPush NewCharacterPush(*NewCharacterPushDBConnection);
			NewCharacterPush.InAccountID(Client->AccountID);
			NewCharacterPush.InCharacterName(Client->CreateCharacterName);
			NewCharacterPush.InLevel(NewCharacterStatus.Level);
			NewCharacterPush.InCurrentHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InMaxHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InAttack(NewCharacterStatus.Attack);
			NewCharacterPush.InSpeed(NewCharacterStatus.Speed);

			NewCharacterPush.Execute();

			G_DBConnectionPool->Push(en_DBConnect::GAME, NewCharacterPushDBConnection);

			CDBConnection* PlayerDBIDGetDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerPlayerDBIDGet PlayerDBIDGet(*PlayerDBIDGetDBConnection);
			PlayerDBIDGet.OutPlayerDBID(PlayerDBID);

			PlayerDBIDGet.Execute();

			PlayerDBIDGet.Fetch();

			Client->MyPlayers[0]->_PlayerDBId = PlayerDBID;
			Client->MyPlayers[0]->_PlayerName = Client->CreateCharacterName;

			G_DBConnectionPool->Push(en_DBConnect::GAME, PlayerDBIDGetDBConnection);
		}		

		CMessage* ResCreateCharacterMessage = MakePacketResCreateCharacter(PlayerDBID, Client->CreateCharacterName);
		SendPacket(Client->SessionID, ResCreateCharacterMessage);
		ResCreateCharacterMessage->Free();
	}
	else
	{

	}
}

CMessage* CGameServer::MakePacketResClientConnected()
{
	CMessage* ClientConnetedMessage = CMessage::Alloc();
	if (ClientConnetedMessage == nullptr)
	{
		CRASH("ClientConnectdMessage�� nullptr");
	}

	ClientConnetedMessage->Clear();

	*ClientConnetedMessage << (uint16)en_PACKET_S2C_GAME_CLIENT_CONNECTED;	

	return ClientConnetedMessage;
}

//---------------------------------------------------------------
//�α��� ��û ���� ��Ŷ ����� �Լ�
//WORD Type
//BYTE Status  //0 : ����  1 : ����
//CPlayer Players
//---------------------------------------------------------------
CMessage* CGameServer::MakePacketResLogin(bool Status, int32 PlayerCount, wstring PlayersName)
{
	CMessage* LoginMessage = CMessage::Alloc();
	if (LoginMessage == nullptr)
	{
		return nullptr;
	}

	LoginMessage->Clear();

	*LoginMessage << (WORD)en_PACKET_S2C_GAME_RES_LOGIN;
	*LoginMessage << Status;		
	*LoginMessage << PlayerCount;

	if (PlayerCount != 0)
	{
		int32 PlayerNameLen = PlayersName.length() * 2;
		*LoginMessage << PlayerNameLen;
		LoginMessage->InsertData(PlayersName.c_str(), PlayerNameLen);
	}

	return LoginMessage;
}

CMessage* CGameServer::MakePacketResCreateCharacter(int32 PlayerDBID, wstring PlayerName)
{
	CMessage* ResCreateCharacter = CMessage::Alloc();
	if (ResCreateCharacter == nullptr)
	{
		return nullptr;
	}

	ResCreateCharacter->Clear();

	*ResCreateCharacter << (WORD)en_PACKET_S2C_GAME_CREATE_CHARACTER;
	*ResCreateCharacter << PlayerDBID;
	
	int32 PlayerNameLen = PlayerName.length() * 2;
	*ResCreateCharacter << PlayerNameLen;
	ResCreateCharacter->InsertData(PlayerName.c_str(), PlayerNameLen);

	return ResCreateCharacter;
}

//-----------------------------------------------------------------
//���� �̵� ��û ���� ��Ŷ ����� �Լ�
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
//ä�� ������ ��û ���� ��Ŷ ����� �Լ�
//WORD	Type
//INT64	AccountNo
//WCHAR	ID[20]	// null ����
//WCHAR	Nickname[20] // null ����
//WORD	MessageLen
//WCHAR	Message[MessageLen / 2] // null ������
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
	//���� �� �������� ���ϱ� ���� ��ǥ ����
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
	_GameServerCommonMessageQue.Enqueue(ClientJoinJob);
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
	_GameServerCommonMessageQue.Enqueue(NewMessageJob);
	SetEvent(_UpdateWakeEvent);
}

void CGameServer::OnClientLeave(int64 SessionID)
{
	st_JOB* ClientLeaveJob = _JobMemoryPool->Alloc();
	ClientLeaveJob->Type = DISCONNECT_CLIENT;
	ClientLeaveJob->SessionID = SessionID;
	ClientLeaveJob->Message = nullptr;
	_GameServerCommonMessageQue.Enqueue(ClientLeaveJob);
	SetEvent(_UpdateWakeEvent);
}

bool CGameServer::OnConnectionRequest(const wchar_t ClientIP, int32 Port)
{
	return false;
}

//Ŭ�� �߽����� 9���� Ŭ��鿡�� �޼����� �Ѹ���.
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