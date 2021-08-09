#include "pch.h"
#include "GameServer.h"
#include "DBBind.h"
#include "DBConnection.h"
#include "DBConnectionPool.h"
#include "DBStoreProcedure.h"
#include "DataManager.h"
#include "ChannelManager.h"
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
	NewClient->SessionId = SessionID;
	NewClient->AccountId = 0;

	NewClient->SectorX = -1;
	NewClient->SectorY = -1;

	NewClient->IsLogin = false;

	NewClient->Token = 0;

	NewClient->RecvPacketTime = timeGetTime();	

	for (int i = 0; i < 5; i++)
	{
		NewClient->MyPlayers[i] = new CPlayer();
	}

	NewClient->MyPlayer = nullptr;

	_ClientMap.insert(pair<int64, st_CLIENT*>(NewClient->SessionId, NewClient));

	CMessage* ResClientConnectedMessage = MakePacketResClientConnected();	
	SendPacket(NewClient->SessionId, ResClientConnectedMessage);
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
			_SectorList[Client->SectorY][Client->SectorX].remove(Client->SessionId);
		}

		CChannel* Channel =  G_ChannelManager->Find(1);
		Channel->LeaveChannel(Client->MyPlayer);	
		
		// ���� ��� �����ϰ� ���� ���� �÷��̾������ �ش� Ŭ�� �����ٰ� �˸�
		CMessage* ResLeaveGame = MakePacketResDeSpawn(Client->AccountId, Client->MyPlayer->_GameObjectInfo.ObjectId);		
		SendPacketSector(Client, ResLeaveGame);
		ResLeaveGame->Free();
		
		for (int i = 0; i < 5; i++)
		{
			delete Client->MyPlayers[i];
			Client->MyPlayers[i] = nullptr;
		}

		Client->MyPlayer = nullptr;

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
	case en_PACKET_C2S_MOVE:
		PacketProcReqMove(SessionID, Message);
		break;
	case en_PACKET_C2S_ATTACK:
		PacketProcReqAttack(SessionID, Message);
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

	int64 AccountID;
	int32 Token;

	if (Client != nullptr)
	{
		// AccountID ����
		*Message >> AccountID;
		Client->AccountId = AccountID;

		int8 IdLen;
		*Message >> IdLen;

		WCHAR ClientId[20];
		memset(ClientId, 0, sizeof(WCHAR) * 20);
		Message->GetData(ClientId, IdLen);
		
		Client->ClientId = ClientId;

		// ��ū ����
		*Message >> Token;
		Client->Token = Token;

		if (Client->AccountId != 0 && Client->AccountId != AccountID)
		{
			Disconnect(Client->SessionId);
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
		DBAccountCheckJob->SessionID = Client->SessionId;
		DBAccountCheckJob->Message = nullptr;

		_GameServerDataBaseMessageQue.Enqueue(DBAccountCheckJob);		
		SetEvent(_DataBaseWakeEvent);	
	}
	else
	{
		// �ش� Ŭ�� ����
		bool Status = LOGIN_FAIL;		
	}	
}

void CGameServer::PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);

	if (Client)
	{
		// ĳ���� �̸� ����
		int8 CharacterNameLen;
		*Message >> CharacterNameLen;

		WCHAR CharacterName[20];	
		memset(CharacterName, 0, sizeof(WCHAR) * 20);
		Message->GetData(CharacterName, CharacterNameLen);

		// ĳ���� �̸� ����
		Client->CreateCharacterName = CharacterName;

		st_JOB* DBCharacterCheckJob = _JobMemoryPool->Alloc();		
		DBCharacterCheckJob->Type = DATA_BASE_CHARACTER_CHECK;
		DBCharacterCheckJob->SessionID = Client->SessionId;
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
			Disconnect(Client->SessionId);
			return;
		}

		int64 AccountId;
		*Message >> AccountId;

		if (Client->AccountId != AccountId)
		{
			Disconnect(Client->SessionId);
			return;
		}

		int8 EnterGameCharacterNameLen;
		*Message >> EnterGameCharacterNameLen;

		WCHAR EnterGameCharacterName[20];
		memset(EnterGameCharacterName, 0, sizeof(WCHAR) * 20);
		Message->GetData(EnterGameCharacterName, EnterGameCharacterNameLen);
				
		// Ŭ�� ������ �ִ� ĳ�� �߿� ��Ŷ���� ���� ĳ���Ͱ� �ִ��� Ȯ���Ѵ�.
		for (int i = 0; i < 5; i++)
		{
			if (Client->MyPlayers[i]->_PlayerName == EnterGameCharacterName)
			{
				Client->MyPlayer = Client->MyPlayers[i];
				break;
			}
		}
				
		// ������ ä���� ã��
		CChannel* Channel = G_ChannelManager->Find(1);				
		// ä�� ����
		Channel->EnterChannel(Client->MyPlayer);

		CMessage* ResEnterGamePacket = MakePacketResEnterGame(Client->AccountId, Client->MyPlayer->_GameObjectInfo.ObjectId, Client->MyPlayer->_PlayerName, Client->MyPlayer->_GameObjectInfo);
		SendPacket(Client->SessionId, ResEnterGamePacket);
		ResEnterGamePacket->Free();	
					
		// �ٸ� �÷��̾������ ���� �����϶�� �˷���
		CMessage* ResSpawnPacket = MakePacketResSpawn(Client->AccountId, Client->MyPlayer->_GameObjectInfo.ObjectId, 1, &Client->MyPlayer->_PlayerName, &Client->MyPlayer->_GameObjectInfo);
		SendPacketSector(Client, ResSpawnPacket);
		ResSpawnPacket->Free();				
		
		// ������ �ٸ� ������Ʈ���� �����϶�� �˷���				
		wstring* SpawnObjectNames;
		st_GameObjectInfo* SpawnGameObjectInfos;

		vector<CPlayer*> AroundPlayers = Channel->GetAroundPlayer(Client->MyPlayer, 10);
		
		if (AroundPlayers.size() > 0)
		{
			SpawnObjectNames = new wstring[AroundPlayers.size()];
			SpawnGameObjectInfos = new st_GameObjectInfo[AroundPlayers.size()];

			for (int i = 0; i < AroundPlayers.size(); i++)
			{
				SpawnObjectNames[i] = AroundPlayers[i]->_PlayerName;
				SpawnGameObjectInfos[i] = AroundPlayers[i]->_GameObjectInfo;
			}

			CMessage* ResOtherObjectSpawnPacket = MakePacketResSpawn(Client->AccountId, Client->MyPlayer->_GameObjectInfo.ObjectId, AroundPlayers.size(), SpawnObjectNames, SpawnGameObjectInfos);
			SendPacket(Client->SessionId, ResOtherObjectSpawnPacket);
			ResOtherObjectSpawnPacket->Free();
		}		
	}
	else
	{

	}
}

// int64 AccountId
// int32 PlayerDBId
// char Dir
void CGameServer::PacketProcReqMove(int64 SessionID, CMessage* Message)
{	
	st_CLIENT* Client = FindClient(SessionID);

	if (Client)
	{
		if (!Client->IsLogin)
		{
			Disconnect(Client->SessionId);
			return;
		}

		int64 AccountId;
		*Message >> AccountId;

 		if (Client->AccountId != AccountId)
		{
			Disconnect(Client->SessionId);
			return;
		}

		int32 PlayerDBId;
		*Message >> PlayerDBId;

		if (Client->MyPlayer == nullptr)
		{
			Disconnect(Client->SessionId);
			return;
		}
		else
		{
			if (Client->MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
			{
				Disconnect(Client->SessionId);
				return;
			}
		}
				
		char ReqMoveDir;
		*Message >> ReqMoveDir;

		en_MoveDir MoveDir = (en_MoveDir)ReqMoveDir;
		
		st_Vector2Int DirVector2Int;

		switch (MoveDir)
		{
		case en_MoveDir::UP:			
			DirVector2Int = st_Vector2Int::Up();
			break;
		case en_MoveDir::DOWN:
			DirVector2Int = st_Vector2Int::Down();
			break;
		case en_MoveDir::LEFT:
			DirVector2Int = st_Vector2Int::Left();
			break;
		case en_MoveDir::RIGHT:
			DirVector2Int = st_Vector2Int::Right();
			break;
		}
		
		CPlayer* MyPlayer = Client->MyPlayer;		
		MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = MoveDir;
		MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;

		st_Vector2Int PlayerPosition;
		PlayerPosition._X = MyPlayer->GetPositionInfo().PositionX;
		PlayerPosition._Y = MyPlayer->GetPositionInfo().PositionY;
		
		//G_Logger->WriteStdOut(en_Color::WHITE, L"MoveReq Y : %d X : %d Dir : %d \n", MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY, MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX, MoveDir);

		CChannel* Channel = G_ChannelManager->Find(1);				

		st_Vector2Int CheckPosition = PlayerPosition + DirVector2Int;

		bool IsCanGo = Channel->_Map->Cango(CheckPosition);
		bool ApplyMoveExe;
		if (IsCanGo == true)
		{
			 ApplyMoveExe = Channel->_Map->ApplyMove(MyPlayer, CheckPosition);

			//G_Logger->WriteStdOut(en_Color::WHITE, L"Y : %d X : %d To Move \n\n", MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY, MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX);
		}		
		else
		{
			//G_Logger->WriteStdOut(en_Color::RED, L"Cant Move\n");
		}

		// ������ ������ ��� ����
		CMessage* ResMyMoveMePacket = MakePacketResMove(Client->AccountId, MyPlayer->_GameObjectInfo.ObjectId, IsCanGo, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
		SendPacket(Client->SessionId, ResMyMoveMePacket);
		ResMyMoveMePacket->Free();
		
		// ���� ������ ���� �� ���� �÷��̾�鿡�� �˷�����
		CMessage* ResMyMoveOtherPacket = MakePacketResMove(Client->AccountId, MyPlayer->_GameObjectInfo.ObjectId, IsCanGo, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
		SendPacketSector(Client, ResMyMoveOtherPacket);
		ResMyMoveOtherPacket->Free();

		MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
	}
	else
	{

	}
}

// int64 AccountId
// int32 PlayerDBId
// char Dir
void CGameServer::PacketProcReqAttack(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);
	int64 AccountId;
	int32 PlayerDBId;

	if (Client)
	{
		if (!Client->IsLogin)
		{
			Disconnect(Client->SessionId);
			return;
		}

		*Message >> AccountId;

		if (Client->AccountId != AccountId)
		{
			Disconnect(Client->SessionId);
			return;
		}

		*Message >> PlayerDBId;
		
		if (Client->MyPlayer == nullptr)
		{
			Disconnect(Client->SessionId);
			return;
		}
		else
		{
			if (Client->MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
			{
				Disconnect(Client->SessionId);
				return;
			}
		}

		char ReqMoveDir;
		*Message >> ReqMoveDir;

		en_MoveDir MoveDir = (en_MoveDir)ReqMoveDir;	
		
		Client->MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = MoveDir;

		// ���� ó�� �ؾ���
		CMessage* ResAttackPacket = MakePacketResAttack(Client->AccountId, Client->MyPlayer->_GameObjectInfo.ObjectId, MoveDir);
		SendPacket(Client->SessionId, ResAttackPacket);
		ResAttackPacket->Free();

		// ���� ���� �鿡�� ���� ���� �����ϰ� �ִٰ� �˷���
		CMessage* ResMyAttackOtherPacket = MakePacketResAttack(Client->AccountId, Client->MyPlayer->_GameObjectInfo.ObjectId, MoveDir);
		SendPacketSector(Client, ResMyAttackOtherPacket);
		ResMyAttackOtherPacket->Free();
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

	if (Client)
	{
		//Ŭ�� �α��� ������ �Ǵ�
		if (!Client->IsLogin)
		{
			Disconnect(Client->SessionId);
			return;
		}

		*Message >> AccountNo;

		if (Client->AccountId != AccountNo)
		{
			Disconnect(Client->SessionId);
			return;
		}

		*Message >> SectorX;
		*Message >> SectorY;

		//���� X Y ��ǥ ���� �˻�
		if (SectorX < 0 || SectorX >= SECTOR_X_MAX || SectorY < 0 || SectorY >= SECTOR_Y_MAX)
		{
			Disconnect(Client->SessionId);
			return;
		}

		//���� ���Ϳ��� Ŭ�� ����
		if (Client->SectorX != -1 && Client->SectorY != -1)
		{
			_SectorList[Client->SectorY][Client->SectorX].remove(Client->SessionId);
		}

		Client->SectorY = SectorY;
		Client->SectorX = SectorX;

		_SectorList[Client->SectorY][Client->SectorX].push_back(Client->SessionId);

		/*CMessage* SectorMoveResMessage = MakePacketResSectorMove(Client->AccountId, Client->SectorX, Client->SectorY);
		SendPacket(Client->SessionId, SectorMoveResMessage);
		SectorMoveResMessage->Free();*/
	}	
	else
	{

	}
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
		Disconnect(Client->SessionId);
		return;
	}

	*Message >> AccountNo;
	if (Client->AccountId != AccountNo)
	{
		Disconnect(Client->SessionId);
		return;
	}

	*Message >> CSMessageLen;
	MessageLen = Message->GetData(CSMessage, CSMessageLen);
	if (MessageLen != CSMessageLen)
	{
		Disconnect(Client->SessionId);
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
		__int64 ClientAccountId = Client->AccountId;
		int Token = Client->Token;

		// AccountServer�� �Է¹��� AccountID�� �ִ��� Ȯ���Ѵ�.
	
		// AccountNo�� Token���� AccountServerDB �����ؼ� �����Ͱ� �ִ��� Ȯ��
		CDBConnection* TokenDBConnection = G_DBConnectionPool->Pop(en_DBConnect::TOKEN);
		
		SP::CDBAccountTokenGet AccountTokenGet(*TokenDBConnection);
		AccountTokenGet.InAccountID(ClientAccountId); // AccountId �Է�
		
		int DBToken = 0;		
		TIMESTAMP_STRUCT LoginSuccessTime;
		TIMESTAMP_STRUCT TokenExpiredTime;

		AccountTokenGet.OutToken(DBToken); // ��ū �޾ƿ�
		AccountTokenGet.OutLoginsuccessTime(LoginSuccessTime);		
		AccountTokenGet.OutTokenExpiredTime(TokenExpiredTime);

		AccountTokenGet.Execute();

		AccountTokenGet.Fetch();
		
		G_DBConnectionPool->Push(en_DBConnect::TOKEN, TokenDBConnection); // Ǯ �ݳ�

		// DB ��ū�� Ŭ��κ��� �� ��ū�� ������ �α��� ��������
		if (Token == DBToken)
		{
			Client->IsLogin = true;
			// Ŭ�� �����ϰ� �ִ� �÷��̾���� DB�κ��� �ܾ�´�.
			CDBConnection* GameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			
			SP::CDBGameServerPlayersGet ClientPlayersGet(*GameServerDBConnection);
			ClientPlayersGet.InAccountID(ClientAccountId);

			int64 PlayerId;
			WCHAR PlayerName[100];
			int32 PlayerLevel;
			int32 PlayerCurrentHP;
			int32 PlayerMaxHP;
			int32 PlayerAttack;
			float PlayerSpeed;
			
			ClientPlayersGet.OutPlayerDBID(PlayerId);
			ClientPlayersGet.OutPlayerName(PlayerName);		
			ClientPlayersGet.OutLevel(PlayerLevel);
			ClientPlayersGet.OutCurrentHP(PlayerCurrentHP);
			ClientPlayersGet.OutMaxHP(PlayerMaxHP);
			ClientPlayersGet.OutAttack(PlayerAttack);
			ClientPlayersGet.OutSpeed(PlayerSpeed);

			ClientPlayersGet.Execute();

			int PlayerCount = 0;
			
			while (ClientPlayersGet.Fetch())
			{								
				// �÷��̾� ���� ����
				Client->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectId = PlayerId;				
				Client->MyPlayers[PlayerCount]->_PlayerName = PlayerName;				
				Client->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectStatInfo.Level = PlayerLevel;
				Client->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectStatInfo.HP = PlayerCurrentHP;
				Client->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectStatInfo.MaxHP = PlayerMaxHP;
				Client->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectStatInfo.Attack = PlayerAttack;
				Client->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectStatInfo.Speed = PlayerSpeed;
				Client->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
				Client->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;		
				Client->MyPlayers[PlayerCount]->_SessionId = Client->SessionId;
							
				PlayerCount++;
			}							
			
			// Ŭ�󿡰� �α��� ���� ��Ŷ ����
			CMessage* ResLoginMessage = MakePacketResLogin(Status, PlayerCount, Client->MyPlayers[0]->_GameObjectInfo.ObjectId, Client->MyPlayers[0]->_PlayerName);
			SendPacket(Client->SessionId, ResLoginMessage);
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
	int32 PlayerDBId;
	st_CLIENT* Client = FindClient(SessionID);

	if (Client)
	{
		// ��û�� Ŭ���� ������ ĳ������ �̸��� �����´�.
		wstring CreateCharacterName = Client->CreateCharacterName;

		// GameServerDB�� �ش� ĳ���Ͱ� �̹� �ִ��� Ȯ���Ѵ�.
		CDBConnection* FindCharacterNameGameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerCharacterNameGet CharacterNameGet(*FindCharacterNameGameServerDBConnection);
		CharacterNameGet.InCharacterName(CreateCharacterName);

		CharacterNameGet.Execute();

		bool CharacterNameFind = CharacterNameGet.Fetch();

		G_DBConnectionPool->Push(en_DBConnect::GAME, FindCharacterNameGameServerDBConnection);

		// ĳ���Ͱ� �������� ���� ���
		if (!CharacterNameFind)
		{
			// ���� 1�� �ش��ϴ� ĳ���� ���� �о��
			auto FindStatus = G_Datamanager->_Status.find(1);
			st_StatusData NewCharacterStatus = *(*FindStatus).second;

			// �ռ� �о�� ĳ���� ������ ���� DB�� ����
			CDBConnection* NewCharacterPushDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

			SP::CDBGameServerCreateCharacterPush NewCharacterPush(*NewCharacterPushDBConnection);
			NewCharacterPush.InAccountID(Client->AccountId);
			NewCharacterPush.InCharacterName(Client->CreateCharacterName);
			NewCharacterPush.InLevel(NewCharacterStatus.Level);
			NewCharacterPush.InCurrentHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InMaxHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InAttack(NewCharacterStatus.Attack);
			NewCharacterPush.InSpeed(NewCharacterStatus.Speed);

			NewCharacterPush.Execute();

			G_DBConnectionPool->Push(en_DBConnect::GAME, NewCharacterPushDBConnection);
			
			// �ռ� ������ ĳ������ DBId�� AccountId�� �̿��� ���´�.
			CDBConnection* PlayerDBIDGetDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerPlayerDBIDGet PlayerDBIDGet(*PlayerDBIDGetDBConnection);
			PlayerDBIDGet.InAccountID(Client->AccountId);

			PlayerDBIDGet.OutPlayerDBID(PlayerDBId);

			PlayerDBIDGet.Execute();

			PlayerDBIDGet.Fetch();

			// DB���� �о�� DBId�� ����
			Client->MyPlayers[0]->_GameObjectInfo.ObjectId = PlayerDBId;			
			// ĳ������ �̸��� ����
			Client->MyPlayers[0]->_PlayerName = Client->CreateCharacterName;
			
			Client->MyPlayers[0]->_GameObjectInfo.ObjectStatInfo.Level = NewCharacterStatus.Level;
			Client->MyPlayers[0]->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
			Client->MyPlayers[0]->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
			Client->MyPlayers[0]->_GameObjectInfo.ObjectStatInfo.Attack = NewCharacterStatus.Attack;
			Client->MyPlayers[0]->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
			Client->MyPlayers[0]->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			Client->MyPlayers[0]->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
			Client->MyPlayers[0]->_SessionId = Client->SessionId;

			G_DBConnectionPool->Push(en_DBConnect::GAME, PlayerDBIDGetDBConnection);
		}		
		else
		{
			// ĳ���Ͱ� �̹� DB�� �ִ� ���
			PlayerDBId = Client->MyPlayers[0]->_GameObjectInfo.ObjectId;			
		}

		// ĳ���� ���� ���� ����
		CMessage* ResCreateCharacterMessage = MakePacketResCreateCharacter(!CharacterNameFind, PlayerDBId, Client->CreateCharacterName);
		SendPacket(Client->SessionId, ResCreateCharacterMessage);
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
CMessage* CGameServer::MakePacketResLogin(bool Status, int32 PlayerCount, int32 PlayerDBId, wstring PlayersName)
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
	*LoginMessage << PlayerDBId;

	if (PlayerCount != 0)
	{			
		int8 PlayerNameLen = PlayersName.length() * 2;		
		*LoginMessage << PlayerNameLen;
		LoginMessage->InsertData(PlayersName.c_str(), PlayerNameLen);
	}

	return LoginMessage;
}

// int32 PlayerDBId
// bool IsSuccess
// wstring PlayerName
CMessage* CGameServer::MakePacketResCreateCharacter(bool IsSuccess, int32 PlayerDBId, wstring PlayerName)
{
	CMessage* ResCreateCharacter = CMessage::Alloc();
	if (ResCreateCharacter == nullptr)
	{
		return nullptr;
	}

	ResCreateCharacter->Clear();

	*ResCreateCharacter << (WORD)en_PACKET_S2C_GAME_CREATE_CHARACTER;
	*ResCreateCharacter << IsSuccess;
	*ResCreateCharacter << PlayerDBId;
	
	int8 PlayerNameLen = PlayerName.length() * 2;
	*ResCreateCharacter << PlayerNameLen;
	ResCreateCharacter->InsertData(PlayerName.c_str(), PlayerNameLen);

	return ResCreateCharacter;
}

CMessage* CGameServer::MakePacketResEnterGame(int64 AccountId, int32 PlayerDBId, wstring EnterPlayerName, st_GameObjectInfo ObjectInfo)
{	
	CMessage* ResEnterGamePacket = CMessage::Alloc();
	if (ResEnterGamePacket == nullptr)
	{
		return nullptr;
	}

	ResEnterGamePacket->Clear();

	*ResEnterGamePacket << (WORD)en_PACKET_S2C_GAME_ENTER;
	*ResEnterGamePacket << AccountId;
	*ResEnterGamePacket << PlayerDBId;	

	// ObjectId
	*ResEnterGamePacket << ObjectInfo.ObjectId;

	// EnterPlayerName
	int8 EnterPlayerNameLen = EnterPlayerName.length() * 2;
	*ResEnterGamePacket << EnterPlayerNameLen;
	ResEnterGamePacket->InsertData(EnterPlayerName.c_str(), EnterPlayerNameLen);

	// st_PositionInfo
	*ResEnterGamePacket << (int8)ObjectInfo.ObjectPositionInfo.State;
	*ResEnterGamePacket << ObjectInfo.ObjectPositionInfo.PositionX;
	*ResEnterGamePacket << ObjectInfo.ObjectPositionInfo.PositionY;
	*ResEnterGamePacket << (int8)ObjectInfo.ObjectPositionInfo.MoveDir;

	// st_StatInfo
	*ResEnterGamePacket << ObjectInfo.ObjectStatInfo.Level;
	*ResEnterGamePacket << ObjectInfo.ObjectStatInfo.HP;
	*ResEnterGamePacket << ObjectInfo.ObjectStatInfo.MaxHP;
	*ResEnterGamePacket << ObjectInfo.ObjectStatInfo.Attack;
	*ResEnterGamePacket << ObjectInfo.ObjectStatInfo.Speed;

	// ObjectType
	*ResEnterGamePacket << (int8)ObjectInfo.ObjectType;
	
	return ResEnterGamePacket;
}

// int64 AccountId
// int32 PlayerDBId
// bool CanGo
// st_PositionInfo PositionInfo
CMessage* CGameServer::MakePacketResMove(int64 AccountId, int32 PlayerDBId, bool Cango, st_PositionInfo PositionInfo)
{
	CMessage* ResMoveMessage = CMessage::Alloc();
	if (ResMoveMessage == nullptr)
	{
		return nullptr;
	}

	ResMoveMessage->Clear();

	*ResMoveMessage << (WORD)en_PACKET_S2C_MOVE;
	*ResMoveMessage << AccountId;
	*ResMoveMessage << PlayerDBId;
	*ResMoveMessage << Cango;
	
	/*en_CreatureState State;
	int32 PositionX;
	int32 PositionY;
	en_MoveDir MoveDir;*/

	// State
	*ResMoveMessage << (int8)PositionInfo.State;

	// int32 PositionX, PositionY
	*ResMoveMessage << PositionInfo.PositionX;
	*ResMoveMessage << PositionInfo.PositionY;

	// MoveDir
	*ResMoveMessage << (int8)PositionInfo.MoveDir;

	return ResMoveMessage;
}

// int64 AccountId
// int32 PlayerDBId
// char Dir
CMessage* CGameServer::MakePacketResAttack(int64 AccountId, int32 PlayerDBId, en_MoveDir Dir)
{
	CMessage* ResAttackMessage = CMessage::Alloc();
	if (ResAttackMessage == nullptr)
	{
		return nullptr;
	}

	ResAttackMessage->Clear();

	*ResAttackMessage << (WORD)en_PACKET_S2C_ATTACK;
	*ResAttackMessage << AccountId;
	*ResAttackMessage << PlayerDBId;	
	*ResAttackMessage << (int8)Dir;
	
	return ResAttackMessage;
}

// int64 AccountId
// int32 PlayerDBId
// st_GameObjectInfo GameObjectInfo
CMessage* CGameServer::MakePacketResSpawn(int64 AccountId, int32 PlayerDBId, int32 ObjectInfosCount, wstring* SpawnObjectName, st_GameObjectInfo* ObjectInfos)
{
	CMessage* ResSpawnPacket = CMessage::Alloc();
	if (ResSpawnPacket == nullptr)
	{
		return nullptr;
	}

	ResSpawnPacket->Clear();

	*ResSpawnPacket << (WORD)en_PACKET_S2C_SPAWN;
	*ResSpawnPacket << AccountId;
	*ResSpawnPacket << PlayerDBId;

	// Spawn ������Ʈ ����
	*ResSpawnPacket << ObjectInfosCount;

	for (int i = 0; i < ObjectInfosCount; i++)
	{
		// SpawnObjectId
		*ResSpawnPacket << ObjectInfos[i].ObjectId;

		// SpawnObjectName
		int8 SpawnObjectNameLen = (int8)(SpawnObjectName[i].length() * 2);
		*ResSpawnPacket << SpawnObjectNameLen;
		ResSpawnPacket->InsertData(SpawnObjectName[i].c_str(), SpawnObjectNameLen);

		// st_PositionInfo
		*ResSpawnPacket << (int8)ObjectInfos[i].ObjectPositionInfo.State;
		*ResSpawnPacket << ObjectInfos[i].ObjectPositionInfo.PositionX;
		*ResSpawnPacket << ObjectInfos[i].ObjectPositionInfo.PositionY;
		*ResSpawnPacket << (int8)ObjectInfos[i].ObjectPositionInfo.MoveDir;

		// st_StatInfo
		*ResSpawnPacket << ObjectInfos[i].ObjectStatInfo.Level;
		*ResSpawnPacket << ObjectInfos[i].ObjectStatInfo.HP;
		*ResSpawnPacket << ObjectInfos[i].ObjectStatInfo.MaxHP;
		*ResSpawnPacket << ObjectInfos[i].ObjectStatInfo.Attack;
		*ResSpawnPacket << ObjectInfos[i].ObjectStatInfo.Speed;

		// ObjectType
		*ResSpawnPacket << (int8)ObjectInfos[i].ObjectType;
	}	

	return ResSpawnPacket;
}

// int64 AccountId
// int32 PlayerDBId
CMessage* CGameServer::MakePacketResDeSpawn(int64 AccountId, int32 PlayerDBId)
{
	CMessage* ResDeSpawnPacket = CMessage::Alloc();
	if (ResDeSpawnPacket == nullptr)
	{
		return nullptr;
	}

	ResDeSpawnPacket->Clear();

	*ResDeSpawnPacket << (WORD)en_PACKET_S2C_DESPAWN;
	*ResDeSpawnPacket << AccountId;
	*ResDeSpawnPacket << PlayerDBId;

	return ResDeSpawnPacket;
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

void CGameServer::SendPacketSector(st_CLIENT* Client, CMessage* Message, bool SendMe)
{
	CChannel* Channel = G_ChannelManager->Find(1);
	vector<CSector*> Sectors = Channel->GetAroundSectors(Client->MyPlayer, 10);
	
	for (CSector* Sector : Sectors)
	{
		for (CPlayer* Player : Sector->GetPlayers())
		{
			if (SendMe == true)
			{
				SendPacket(Player->_SessionId, Message);				
			}
			else
			{
				if (Client->SessionId != Player->_SessionId)
				{
					SendPacket(Player->_SessionId, Message);
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