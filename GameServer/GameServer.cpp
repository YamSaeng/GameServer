#include "pch.h"
#include "GameServer.h"
#include "DBBind.h"
#include "DBConnection.h"
#include "DBConnectionPool.h"
#include "DBStoreProcedure.h"
#include "DataManager.h"
#include "ChannelManager.h"
#include "ObjectManager.h"
#include <process.h>

CGameServer::CGameServer()
{
	//timeBeginPeriod(1);
	_UpdateThread = nullptr;
	_DataBaseThread = nullptr;
	
	// Nonsignaled상태 자동리셋
	_UpdateWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_DataBaseWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	_UpdateThreadEnd = false;
	_DataBaseThreadEnd = false;	
	_LogicThreadEnd = false;

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

	G_ObjectManager->MonsterSpawn(150, 1);
	G_ObjectManager->GameServer = this;

	_UpdateThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThreadProc, this, 0, NULL);
	_DataBaseThread = (HANDLE)_beginthreadex(NULL, 0, DataBaseThreadProc, this, 0, NULL);		

	_GameLogicThread = (HANDLE)_beginthreadex(NULL, 0, GameLogicThreadProc, this, 0, NULL);

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

unsigned __stdcall CGameServer::GameLogicThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_LogicThreadEnd)
	{
		// 채널 목록 돌면서 업데이트
		G_ChannelManager->Update();
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
		NewClient->MyPlayers[i] = (CPlayer*)G_ObjectManager->ObjectCreate(en_GameObjectType::PLAYER);
	}

	NewClient->MyPlayer = nullptr;

	_ClientMap.insert(pair<int64, st_CLIENT*>(NewClient->SessionId, NewClient));

	CMessage* ResClientConnectedMessage = MakePacketResClientConnected();	
	SendPacket(NewClient->SessionId, ResClientConnectedMessage);
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
			_SectorList[Client->SectorY][Client->SectorX].remove(Client->SessionId);
		}

		if (Client->MyPlayer != nullptr)
		{
			CChannel* Channel = G_ChannelManager->Find(1);
			Channel->LeaveChannel(Client->MyPlayer);

			vector<int64> DeSpawnObjectIds;
			// 나간 대상 제외하고 주위 섹터 플레이어들한테 해당 클라가 나갓다고 알림
			DeSpawnObjectIds.push_back(Client->MyPlayer->_GameObjectInfo.ObjectId);
			CMessage* ResLeaveGame = MakePacketResDeSpawn(1, DeSpawnObjectIds);
			SendPacketAroundSector(Client, ResLeaveGame);
			ResLeaveGame->Free();
		}		
		
		for (int i = 0; i < 5; i++)
		{
			G_ObjectManager->ObjectReturn(en_GameObjectType::PLAYER, Client->MyPlayers[i]);
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
	case en_PACKET_C2S_MOUSE_POSITION_OBJECT_INFO:
		PacketProcReqMousePositionObjectInfo(SessionID, Message);
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
// 로그인 요청
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
		// AccountID 셋팅
		*Message >> AccountID;
		Client->AccountId = AccountID;

		int8 IdLen;
		*Message >> IdLen;

		WCHAR ClientId[20];
		memset(ClientId, 0, sizeof(WCHAR) * 20);
		Message->GetData(ClientId, IdLen);
		
		Client->ClientId = ClientId;

		// 토큰 셋팅
		*Message >> Token;
		Client->Token = Token;

		if (Client->AccountId != 0 && Client->AccountId != AccountID)
		{
			Disconnect(Client->SessionId);
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

		st_JOB* DBAccountCheckJob = _JobMemoryPool->Alloc();
		DBAccountCheckJob->Type = DATA_BASE_ACCOUNT_CHECK;
		DBAccountCheckJob->SessionID = Client->SessionId;
		DBAccountCheckJob->Message = nullptr;

		_GameServerDataBaseMessageQue.Enqueue(DBAccountCheckJob);		
		SetEvent(_DataBaseWakeEvent);	
	}
	else
	{
		// 해당 클라가 없음
		bool Status = LOGIN_FAIL;		
	}	
}

void CGameServer::PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message)
{
	st_CLIENT* Client = FindClient(SessionID);

	if (Client)
	{
		// 캐릭터 이름 길이
		int8 CharacterNameLen;
		*Message >> CharacterNameLen;

		WCHAR CharacterName[20];	
		memset(CharacterName, 0, sizeof(WCHAR) * 20);
		Message->GetData(CharacterName, CharacterNameLen);

		// 캐릭터 이름 셋팅
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
		// 로그인 중이 아니면 나간다.
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
				
		// 클라가 가지고 있는 캐릭 중에 패킷으로 받은 캐릭터가 있는지 확인한다.
		for (int i = 0; i < 5; i++)
		{
			if (Client->MyPlayers[i]->_GameObjectInfo.ObjectName == EnterGameCharacterName)
			{
				Client->MyPlayer = Client->MyPlayers[i];
				break;
			}
		}
						
		// ObjectManager에 플레이어 추가 및 패킷 전송
		G_ObjectManager->Add(Client->MyPlayer, 1);		

		// 나한테 나 생성하라고 알려줌
		CMessage* ResEnterGamePacket = MakePacketResEnterGame(Client->MyPlayer->_GameObjectInfo);
		SendPacket(Client->SessionId, ResEnterGamePacket);
		ResEnterGamePacket->Free();	
					
		vector<st_GameObjectInfo> SpawnObjectInfo;
		SpawnObjectInfo.push_back(Client->MyPlayer->_GameObjectInfo);

		// 다른 플레이어들한테 나를 생성하라고 알려줌
		CMessage* ResSpawnPacket = MakePacketResSpawn(1, SpawnObjectInfo);
		SendPacketAroundSector(Client, ResSpawnPacket);
		ResSpawnPacket->Free();				
		
		SpawnObjectInfo.clear();

		// 나한테 다른 오브젝트들을 생성하라고 알려줌						
		st_GameObjectInfo* SpawnGameObjectInfos;

		vector<CGameObject*> AroundObjects = G_ChannelManager->Find(1)->GetAroundObjects(Client->MyPlayer, 10);
		
		if (AroundObjects.size() > 0)
		{			
			SpawnGameObjectInfos = new st_GameObjectInfo[AroundObjects.size()];

			for (int32 i = 0; i < AroundObjects.size(); i++)
			{				
				SpawnObjectInfo.push_back(AroundObjects[i]->_GameObjectInfo);
			}

			CMessage* ResOtherObjectSpawnPacket = MakePacketResSpawn(AroundObjects.size(), SpawnObjectInfo);
			SendPacket(Client->SessionId, ResOtherObjectSpawnPacket);
			ResOtherObjectSpawnPacket->Free();
		
			delete[] SpawnGameObjectInfos;
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

			//G_Logger->WriteStdOut(en_Color::YELLOW, L"Y : %d X : %d To Move \n\n", MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY, MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX);
		}		
		else
		{
			//G_Logger->WriteStdOut(en_Color::RED, L"Cant Move\n");
		}
			
		// 내가 움직인 것을 내 주위 플레이어들에게 알려야함
		CMessage* ResMyMoveOtherPacket = MakePacketResMove(Client->AccountId, MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
		SendPacketAroundSector(Client, ResMyMoveOtherPacket, true);
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

		CPlayer* MyPlayer = Client->MyPlayer;

		char ReqMoveDir;
		*Message >> ReqMoveDir;

		en_MoveDir MoveDir = (en_MoveDir)ReqMoveDir;			
		MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = MoveDir;

		st_Vector2Int FrontCell = MyPlayer->GetFrontCellPosition(MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir);
		CGameObject* Target = MyPlayer->_Channel->_Map->Find(FrontCell);		
		
		// 주위 섹터 들에게 내가 지금 공격하고 있다고 알려줌
		CMessage* ResMyAttackOtherPacket = MakePacketResAttack(Client->AccountId, MyPlayer->_GameObjectInfo.ObjectId, MoveDir);
		SendPacketAroundSector(Client, ResMyAttackOtherPacket,true);
		ResMyAttackOtherPacket->Free();

		if (Target != nullptr)
		{
			Target->OnDamaged(MyPlayer, 10);

			G_Logger->WriteStdOut(en_Color::YELLOW, L"Attack");
			
			CMessage* ResChangeHPPacket = MakePacketResChangeHP(Target->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectStatInfo.HP, Target->_GameObjectInfo.ObjectStatInfo.MaxHP);
			SendPacketAroundSector(Client, ResChangeHPPacket, true);
			ResChangeHPPacket->Free();
		}
	}
}

// int64 AccountId
// int32 PlayerDBId
// int32 X
// int32 Y
void CGameServer::PacketProcReqMousePositionObjectInfo(int64 SessionID, CMessage* Message)
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

		int X;
		int Y;

		*Message >> X;
		*Message >> Y;

		CChannel* Channel =  G_ChannelManager->Find(1);

		st_Vector2Int FindPosition;
		FindPosition._X = X;
		FindPosition._Y = Y;

		CGameObject* FindObject =  Channel->_Map->Find(FindPosition);

		if (FindObject != nullptr)
		{
			CPlayer* Player = (CPlayer*)FindObject;

			CMessage* ResMousePositionObjectInfo = MakePacketMousePositionObjectInfo(Client->AccountId, Player->_GameObjectInfo.ObjectId, Player->_GameObjectInfo);
			SendPacket(Client->SessionId, ResMousePositionObjectInfo);
			ResMousePositionObjectInfo->Free();
		}
	}
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

	if (Client)
	{
		//클라가 로그인 중인지 판단
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

		//섹터 X Y 좌표 범위 검사
		if (SectorX < 0 || SectorX >= SECTOR_X_MAX || SectorY < 0 || SectorY >= SECTOR_Y_MAX)
		{
			Disconnect(Client->SessionId);
			return;
		}

		//기존 섹터에서 클라 제거
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
//하트 비트
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

		// AccountServer에 입력받은 AccountID가 있는지 확인한다.
	
		// AccountNo와 Token으로 AccountServerDB 접근해서 데이터가 있는지 확인
		CDBConnection* TokenDBConnection = G_DBConnectionPool->Pop(en_DBConnect::TOKEN);
		
		SP::CDBAccountTokenGet AccountTokenGet(*TokenDBConnection);
		AccountTokenGet.InAccountID(ClientAccountId); // AccountId 입력
		
		int DBToken = 0;		
		TIMESTAMP_STRUCT LoginSuccessTime;
		TIMESTAMP_STRUCT TokenExpiredTime;

		AccountTokenGet.OutToken(DBToken); // 토큰 받아옴
		AccountTokenGet.OutLoginsuccessTime(LoginSuccessTime);		
		AccountTokenGet.OutTokenExpiredTime(TokenExpiredTime);

		AccountTokenGet.Execute();

		AccountTokenGet.Fetch();
		
		G_DBConnectionPool->Push(en_DBConnect::TOKEN, TokenDBConnection); // 풀 반납

		// DB 토큰과 클라로부터 온 토큰이 같으면 로그인 최종성공
		if (Token == DBToken)
		{
			Client->IsLogin = true;
			// 클라가 소유하고 있는 플레이어들을 DB로부터 긁어온다.
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
				// 플레이어 정보 셋팅
				Client->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectId = PlayerId;				
				Client->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectName = PlayerName;				
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
			
			// 클라에게 로그인 응답 패킷 보냄
			CMessage* ResLoginMessage = MakePacketResLogin(Status, PlayerCount, Client->MyPlayers[0]->_GameObjectInfo.ObjectId, Client->MyPlayers[0]->_GameObjectInfo.ObjectName);
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
		// 클라 접속 끊겻을 경우
	}
}

void CGameServer::PacketProcReqCreateCharacterNameCheck(int64 SessionID, CMessage* Message)
{
	int32 PlayerDBId;
	st_CLIENT* Client = FindClient(SessionID);

	if (Client)
	{
		// 요청한 클라의 생성할 캐릭터의 이름을 가져온다.
		wstring CreateCharacterName = Client->CreateCharacterName;

		// GameServerDB에 해당 캐릭터가 이미 있는지 확인한다.
		CDBConnection* FindCharacterNameGameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerCharacterNameGet CharacterNameGet(*FindCharacterNameGameServerDBConnection);
		CharacterNameGet.InCharacterName(CreateCharacterName);

		CharacterNameGet.Execute();

		bool CharacterNameFind = CharacterNameGet.Fetch();

		G_DBConnectionPool->Push(en_DBConnect::GAME, FindCharacterNameGameServerDBConnection);

		// 캐릭터가 존재하지 않을 경우
		if (!CharacterNameFind)
		{
			// 레벨 1에 해당하는 캐릭터 정보 읽어옴
			auto FindStatus = G_Datamanager->_Status.find(1);
			st_StatusData NewCharacterStatus = *(*FindStatus).second;

			// 앞서 읽어온 캐릭터 정보를 토대로 DB에 저장
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
			
			// 앞서 저장한 캐릭터의 DBId를 AccountId를 이용해 얻어온다.
			CDBConnection* PlayerDBIDGetDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerPlayerDBIDGet PlayerDBIDGet(*PlayerDBIDGetDBConnection);
			PlayerDBIDGet.InAccountID(Client->AccountId);

			PlayerDBIDGet.OutPlayerDBID(PlayerDBId);

			PlayerDBIDGet.Execute();

			PlayerDBIDGet.Fetch();

			// DB에서 읽어온 DBId를 저장
			Client->MyPlayers[0]->_GameObjectInfo.ObjectId = PlayerDBId;			
			// 캐릭터의 이름도 저장
			Client->MyPlayers[0]->_GameObjectInfo.ObjectName = Client->CreateCharacterName;
			
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
			// 캐릭터가 이미 DB에 있는 경우
			PlayerDBId = Client->MyPlayers[0]->_GameObjectInfo.ObjectId;			
		}

		// 캐릭터 생성 응답 보냄
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

CMessage* CGameServer::MakePacketResEnterGame(st_GameObjectInfo ObjectInfo)
{	
	CMessage* ResEnterGamePacket = CMessage::Alloc();
	if (ResEnterGamePacket == nullptr)
	{
		return nullptr;
	}

	ResEnterGamePacket->Clear();

	*ResEnterGamePacket << (WORD)en_PACKET_S2C_GAME_ENTER;

	// ObjectId
	*ResEnterGamePacket << ObjectInfo.ObjectId;

	// EnterPlayerName
	int8 EnterPlayerNameLen = ObjectInfo.ObjectName.length() * 2;
	*ResEnterGamePacket << EnterPlayerNameLen;
	ResEnterGamePacket->InsertData(ObjectInfo.ObjectName.c_str(), EnterPlayerNameLen);

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
// st_GameObjectInfo ObjectInfo
CMessage* CGameServer::MakePacketMousePositionObjectInfo(int64 AccountId, int32 PlayerDBId, st_GameObjectInfo ObjectInfo)
{
	CMessage* ResEnterGamePacket = CMessage::Alloc();
	if (ResEnterGamePacket == nullptr)
	{
		return nullptr;
	}

	ResEnterGamePacket->Clear();

	*ResEnterGamePacket << (WORD)en_PACKET_S2C_MOUSE_POSITION_OBJECT_INFO;
	*ResEnterGamePacket << AccountId;
	*ResEnterGamePacket << PlayerDBId;

	// ObjectId
	*ResEnterGamePacket << ObjectInfo.ObjectId;

	// EnterPlayerName
	int8 ObjectNameLen = ObjectInfo.ObjectName.length() * 2;
	*ResEnterGamePacket << ObjectNameLen;
	ResEnterGamePacket->InsertData(ObjectInfo.ObjectName.c_str(), ObjectNameLen);

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

// int64 AccountId
// int32 PlayerDBId
// int32 HP
CMessage* CGameServer::MakePacketResChangeHP(int32 PlayerDBId, int32 CurrentHP, int32 MaxHP)
{
	CMessage* ResChangeHPPacket = CMessage::Alloc();
	if (ResChangeHPPacket == nullptr)
	{
		return nullptr;
	}

	ResChangeHPPacket->Clear();

	*ResChangeHPPacket << (WORD)en_PACKET_S2C_CHANGE_HP;
	*ResChangeHPPacket << PlayerDBId;

	*ResChangeHPPacket << CurrentHP;
	*ResChangeHPPacket << MaxHP;

	return ResChangeHPPacket;
}

CMessage* CGameServer::MakePacketResObjectState(int32 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState)
{
	CMessage* ResObjectStatePacket = CMessage::Alloc();
	if (ResObjectStatePacket == nullptr)
	{
		return nullptr;
	}

	ResObjectStatePacket->Clear();

	*ResObjectStatePacket << (WORD)en_PACKET_S2C_OBJECT_STATE_CHANGE;
	*ResObjectStatePacket << ObjectId;
	*ResObjectStatePacket << (WORD)Direction;
	*ResObjectStatePacket << (WORD)ObjectType;
	*ResObjectStatePacket << (WORD)ObjectState;	

	return ResObjectStatePacket;
}

// int64 AccountId
// int32 PlayerDBId
// bool CanGo
// st_PositionInfo PositionInfo
CMessage* CGameServer::MakePacketResMove(int64 AccountId, int32 ObjectId, en_GameObjectType ObjectType, st_PositionInfo PositionInfo)
{
	CMessage* ResMoveMessage = CMessage::Alloc();
	if (ResMoveMessage == nullptr)
	{
		return nullptr;
	}

	ResMoveMessage->Clear();

	*ResMoveMessage << (WORD)en_PACKET_S2C_MOVE;
	*ResMoveMessage << AccountId;
	*ResMoveMessage << ObjectId;

	// ObjectType
	*ResMoveMessage << (WORD)ObjectType;

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
// st_GameObjectInfo GameObjectInfo
CMessage* CGameServer::MakePacketResSpawn(int32 ObjectInfosCount, vector<st_GameObjectInfo> ObjectInfos)
{
	CMessage* ResSpawnPacket = CMessage::Alloc();
	if (ResSpawnPacket == nullptr)
	{
		return nullptr;
	}

	ResSpawnPacket->Clear();

	*ResSpawnPacket << (WORD)en_PACKET_S2C_SPAWN;

	// Spawn 오브젝트 개수
	*ResSpawnPacket << ObjectInfosCount;

	for (int i = 0; i < ObjectInfosCount; i++)
	{
		// SpawnObjectId
		*ResSpawnPacket << ObjectInfos[i].ObjectId;

		// SpawnObjectName
		int8 SpawnObjectNameLen = (int8)(ObjectInfos[i].ObjectName.length() * 2);
		*ResSpawnPacket << SpawnObjectNameLen;
		ResSpawnPacket->InsertData(ObjectInfos[i].ObjectName.c_str(), SpawnObjectNameLen);

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
CMessage* CGameServer::MakePacketResDeSpawn(int32 DeSpawnObjectCount, vector<int64> DeSpawnObjectIds)
{
	CMessage* ResDeSpawnPacket = CMessage::Alloc();
	if (ResDeSpawnPacket == nullptr)
	{
		return nullptr;
	}

	ResDeSpawnPacket->Clear();

	*ResDeSpawnPacket << (WORD)en_PACKET_S2C_DESPAWN;	
	*ResDeSpawnPacket << DeSpawnObjectCount;

	for (int32 i = 0; i < DeSpawnObjectCount; i++)
	{
		*ResDeSpawnPacket << DeSpawnObjectIds[i];
	}

	return ResDeSpawnPacket;
}

CMessage* CGameServer::MakePacketResDie(int64 DieObjectId)
{
	CMessage* ResDiePacket = CMessage::Alloc();
	if (ResDiePacket == nullptr)
	{
		return nullptr;
	}

	ResDiePacket->Clear();

	*ResDiePacket << (WORD)en_PACKET_S2C_DIE;
	*ResDiePacket << DieObjectId;

	return ResDiePacket;
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

void CGameServer::SendPacketSector(CSector* Sector, CMessage* Message)
{
	for (CPlayer* Player : Sector->GetPlayers())
	{		
		SendPacket(Player->_SessionId, Message);
	}
}

void CGameServer::SendPacketAroundSector(st_Vector2Int CellPosition, CMessage* Message)
{
	CChannel* Channel = G_ChannelManager->Find(1);
	vector<CSector*> Sectors = Channel->GetAroundSectors(CellPosition, 10);

	for (CSector* Sector : Sectors)
	{
		for (CPlayer* Player : Sector->GetPlayers())
		{
			SendPacket(Player->_SessionId,Message);
		}
	}
}

void CGameServer::SendPacketAroundSector(st_CLIENT* Client, CMessage* Message, bool SendMe)
{
	CChannel* Channel = G_ChannelManager->Find(1);
	vector<CSector*> Sectors = Channel->GetAroundSectors(Client->MyPlayer->GetCellPosition(), 10);
	
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