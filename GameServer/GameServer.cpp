#include "pch.h"
#include "GameServer.h"
#include "DBBind.h"
#include "DBConnection.h"
#include "DBConnectionPool.h"
#include "DBStoreProcedure.h"
#include "DataManager.h"
#include "ChannelManager.h"
#include "ObjectManager.h"
#include "Inventory.h"
#include <process.h>

CGameServer::CGameServer()
{
	//timeBeginPeriod(1);
	_AuthThread = nullptr;
	_NetworkThread = nullptr;
	_DataBaseThread = nullptr;

	// Nonsignaled���� �ڵ�����
	_AuthThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_NetworkThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_DataBaseWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	_AuthThreadEnd = false;
	_NetworkThreadEnd = false;
	_DataBaseThreadEnd = false;

	_JobMemoryPool = new CMemoryPoolTLS<st_Job>(0);

	_AuthThreadTPS = 0;
	_AuthThreadWakeCount = 0;

	_NetworkThreadTPS = 0;

	_GameObjectId = 0;
}

CGameServer::~CGameServer()
{
	CloseHandle(_NetworkThreadWakeEvent);
	//timeEndPeriod(1);
}

void CGameServer::Start(const WCHAR* OpenIP, int32 Port)
{
	CNetworkLib::Start(OpenIP, Port);

	G_ObjectManager->MonsterSpawn(100, 1, en_GameObjectType::SLIME);
	G_ObjectManager->MonsterSpawn(20, 1, en_GameObjectType::BEAR);

	G_ObjectManager->GameServer = this;

	_AuthThread = (HANDLE)_beginthreadex(NULL, 0, AuthThreadProc, this, 0, NULL);
	_NetworkThread = (HANDLE)_beginthreadex(NULL, 0, NetworkThreadProc, this, 0, NULL);
	_DataBaseThread = (HANDLE)_beginthreadex(NULL, 0, DataBaseThreadProc, this, 0, NULL);

	CloseHandle(_AuthThread);
	CloseHandle(_NetworkThread);
	CloseHandle(_DataBaseThread);
}

unsigned __stdcall CGameServer::AuthThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_AuthThreadEnd)
	{
		WaitForSingleObject(Instance->_AuthThreadWakeEvent, INFINITE);

		Instance->_AuthThreadWakeCount++;

		while (!Instance->_GameServerAuthThreadMessageQue.IsEmpty())
		{
			st_Job* Job = nullptr;

			if (!Instance->_GameServerAuthThreadMessageQue.Dequeue(&Job))
			{
				break;
			}

			switch (Job->Type)
			{
			case AUTH_NEW_CLIENT_JOIN:
				Instance->CreateNewClient(Job->SessionID);
				break;
			case AUTH_DISCONNECT_CLIENT:
				Instance->DeleteClient(Job->Session);
				break;
			case AUTH_MESSAGE:
				break;
			default:
				Instance->Disconnect(Job->SessionID);
				break;
			}

			Instance->_AuthThreadTPS++;
			Instance->_JobMemoryPool->Free(Job);
		}
	}
	return 0;
}

unsigned __stdcall CGameServer::NetworkThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_NetworkThreadEnd)
	{
		while (!Instance->_GameServerNetworkThreadMessageQue.IsEmpty())
		{
			st_Job* Job = nullptr;

			if (!Instance->_GameServerNetworkThreadMessageQue.Dequeue(&Job))
			{
				break;
			}

			switch (Job->Type)
			{
			case MESSAGE:
				Instance->PacketProc(Job->SessionID, Job->Message);
				break;
			default:
				Instance->Disconnect(Job->SessionID);
				break;
			}

			Instance->_NetworkThreadTPS++;
			Instance->_JobMemoryPool->Free(Job);
		}

		G_ChannelManager->Update();
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

		while (!Instance->_GameServerDataBaseThreadMessageQue.IsEmpty())
		{
			st_Job* Job = nullptr;

			if (!Instance->_GameServerDataBaseThreadMessageQue.Dequeue(&Job))
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

void CGameServer::CreateNewClient(int64 SessionId)
{
	st_SESSION* Session = FindSession(SessionId);
	Session->AccountId = 0;

	Session->SectorX = -1;
	Session->SectorY = -1;

	Session->IsLogin = false;

	Session->Token = 0;

	Session->RecvPacketTime = timeGetTime();

	for (int i = 0; i < 5; i++)
	{
		Session->MyPlayers[i] = (CPlayer*)G_ObjectManager->ObjectCreate(en_GameObjectType::PLAYER);
	}

	Session->MyPlayer = nullptr;

	CMessage* ResClientConnectedMessage = MakePacketResClientConnected();
	SendPacket(Session->SessionId, ResClientConnectedMessage);
	ResClientConnectedMessage->Free();

	ReturnSession(Session);
}

void CGameServer::DeleteClient(st_SESSION* Session)
{
	if (Session == nullptr)
	{
		return;
	}

	if (Session->MyPlayer != nullptr)
	{
		CChannel* Channel = G_ChannelManager->Find(1);
		Channel->LeaveChannel(Session->MyPlayer);

		vector<int64> DeSpawnObjectIds;
		DeSpawnObjectIds.push_back(Session->MyPlayer->_GameObjectInfo.ObjectId);
		CMessage* ResLeaveGame = MakePacketResDeSpawn(1, DeSpawnObjectIds);
		SendPacketAroundSector(Session, ResLeaveGame);
		ResLeaveGame->Free();

		for (int i = 0; i < 5; i++)
		{
			G_ObjectManager->Remove(Session->MyPlayers[i], 1);			
			Session->MyPlayers[i]->_NetworkState = en_ObjectNetworkState::LEAVE;
			Session->MyPlayers[i] = nullptr;
		}
	}

	Session->MyPlayer = nullptr;

	Session->ClientSock = INVALID_SOCKET;
	closesocket(Session->CloseSock);

	int64 Index = GET_SESSIONINDEX(Session->SessionId);
	_SessionArrayIndexs.Push(GET_SESSIONINDEX(Session->SessionId));
}

void CGameServer::PacketProc(int64 SessionId, CMessage* Message)
{
	WORD MessageType;
	*Message >> MessageType;

	switch (MessageType)
	{
	case en_PACKET_C2S_GAME_REQ_LOGIN:
		PacketProcReqLogin(SessionId, Message);
		break;
	case en_PACKET_C2S_GAME_CREATE_CHARACTER:
		PacketProcReqCreateCharacter(SessionId, Message);
		break;
	case en_PACKET_C2S_GAME_ENTER:
		PacketProcReqEnterGame(SessionId, Message);
		break;
	case en_PACKET_C2S_MOVE:
		PacketProcReqMove(SessionId, Message);
		break;
	case en_PACKET_C2S_ATTACK:
		PacketProcReqAttack(SessionId, Message);
		break;
	case en_PACKET_C2S_MOUSE_POSITION_OBJECT_INFO:
		PacketProcReqMousePositionObjectInfo(SessionId, Message);
		break;
	case en_PACKET_C2S_MESSAGE:
		PacketProcReqChattingMessage(SessionId, Message);
		break;
	case en_PACKET_C2S_ITEM_TO_INVENTORY:
		PacketProcReqItemToInventory(SessionId, Message);
		break;
	case en_PACKET_CS_GAME_REQ_SECTOR_MOVE:
		PacketProcReqSectorMove(SessionId, Message);
		break;
	case en_PACKET_CS_GAME_REQ_MESSAGE:
		PacketProcReqMessage(SessionId, Message);
		break;
	case en_PACKET_CS_GAME_REQ_HEARTBEAT:
		PacketProcReqHeartBeat(SessionId, Message);
		break;
	default:
		Disconnect(SessionId);
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
	st_SESSION* Session = FindSession(SessionID);

	int64 AccountId;
	int32 Token;

	if (Session != nullptr)
	{
		// AccountID ����
		*Message >> AccountId;

		// �ߺ� �α��� Ȯ��
		for (st_SESSION* FindSession : _SessionArray)
		{
			if (FindSession->AccountId == AccountId)
			{
				// ���� ������ �ߺ� �Ǵ� ���� ���� ����
				Disconnect(Session->SessionId);
				ReturnSession(Session);
				return;
			}
		}

		Session->AccountId = AccountId;

		int8 IdLen;
		*Message >> IdLen;

		WCHAR ClientId[20];
		memset(ClientId, 0, sizeof(WCHAR) * 20);
		Message->GetData(ClientId, IdLen);

		Session->LoginId = ClientId;

		// ��ū ����
		*Message >> Token;
		Session->Token = Token;

		if (Session->AccountId != 0 && Session->AccountId != AccountId)
		{
			Disconnect(Session->SessionId);
			return;
		}

		// DB ť�� ��û�ϱ� �� IOCount�� �������Ѽ� Session�� �ݳ� �ȵǵ��� ����
		Session->IOBlock->IOCount++;

		st_Job* DBAccountCheckJob = _JobMemoryPool->Alloc();
		DBAccountCheckJob->Type = DATA_BASE_ACCOUNT_CHECK;
		DBAccountCheckJob->SessionID = Session->SessionId;
		DBAccountCheckJob->Message = nullptr;

		_GameServerDataBaseThreadMessageQue.Enqueue(DBAccountCheckJob);
		SetEvent(_DataBaseWakeEvent);
	}
	else
	{
		// �ش� Ŭ�� ����
		bool Status = LOGIN_FAIL;
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionID);

	if (Session)
	{
		// ĳ���� �̸� ����
		int8 CharacterNameLen;
		*Message >> CharacterNameLen;

		WCHAR CharacterName[20];
		memset(CharacterName, 0, sizeof(WCHAR) * 20);
		Message->GetData(CharacterName, CharacterNameLen);

		// ĳ���� �̸� ����
		Session->CreateCharacterName = CharacterName;

		Session->IOBlock->IOCount++;

		st_Job* DBCharacterCheckJob = _JobMemoryPool->Alloc();
		DBCharacterCheckJob->Type = DATA_BASE_CHARACTER_CHECK;
		DBCharacterCheckJob->SessionID = Session->SessionId;
		DBCharacterCheckJob->Message = nullptr;

		_GameServerDataBaseThreadMessageQue.Enqueue(DBCharacterCheckJob);
		SetEvent(_DataBaseWakeEvent);
	}
	else
	{

	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqEnterGame(int64 SessionID, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionID);

	if (Session)
	{
		// �α��� ���� �ƴϸ� ������.
		if (!Session->IsLogin)
		{
			Disconnect(Session->SessionId);
			return;
		}

		int64 AccountId;
		*Message >> AccountId;

		if (Session->AccountId != AccountId)
		{
			Disconnect(Session->SessionId);
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
			if (Session->MyPlayers[i]->_GameObjectInfo.ObjectName == EnterGameCharacterName)
			{
				Session->MyPlayer = Session->MyPlayers[i];
				Session->MyPlayer->_NetworkState = en_ObjectNetworkState::LIVE;
				break;
			}
		}

		// ObjectManager�� �÷��̾� �߰� �� ��Ŷ ����
		G_ObjectManager->Add(Session->MyPlayer, 1);

		// ������ �� �����϶�� �˷���
		CMessage* ResEnterGamePacket = MakePacketResEnterGame(Session->MyPlayer->_GameObjectInfo);
		SendPacket(Session->SessionId, ResEnterGamePacket);
		ResEnterGamePacket->Free();

		vector<st_GameObjectInfo> SpawnObjectInfo;
		SpawnObjectInfo.push_back(Session->MyPlayer->_GameObjectInfo);

		// �ٸ� �÷��̾������ ���� �����϶�� �˷���
		CMessage* ResSpawnPacket = MakePacketResSpawn(1, SpawnObjectInfo);
		SendPacketAroundSector(Session, ResSpawnPacket);
		ResSpawnPacket->Free();

		SpawnObjectInfo.clear();

		// ������ �ٸ� ������Ʈ���� �����϶�� �˷���						
		st_GameObjectInfo* SpawnGameObjectInfos;

		vector<CGameObject*> AroundObjects = G_ChannelManager->Find(1)->GetAroundObjects(Session->MyPlayer, 10);

		if (AroundObjects.size() > 0)
		{
			SpawnGameObjectInfos = new st_GameObjectInfo[AroundObjects.size()];

			for (int32 i = 0; i < AroundObjects.size(); i++)
			{
				SpawnObjectInfo.push_back(AroundObjects[i]->_GameObjectInfo);
			}

			CMessage* ResOtherObjectSpawnPacket = MakePacketResSpawn((int32)AroundObjects.size(), SpawnObjectInfo);
			SendPacket(Session->SessionId, ResOtherObjectSpawnPacket);
			ResOtherObjectSpawnPacket->Free();

			delete[] SpawnGameObjectInfos;
		}
	}
	else
	{

	}

	ReturnSession(Session);
}

// int64 AccountId
// int32 PlayerDBId
// char Dir
void CGameServer::PacketProcReqMove(int64 SessionID, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionID);

	do
	{
		if (Session)
		{
			// Ŭ�� �α��������� Ȯ��
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			// AccountId�� �̰�
			int64 AccountId;
			*Message >> AccountId;

			// Ŭ�� ����ִ� AccountId�� ���� AccoountId�� �ٸ��� Ȯ��
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			// PlayerDBId�� �̴´�.
			int32 PlayerDBId;
			*Message >> PlayerDBId;

			// Ŭ�� �������� ĳ���Ͱ� �ִ��� Ȯ��
			if (Session->MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �������� ĳ���Ͱ� ������ ObjectId�� �ٸ��� Ȯ��
				if (Session->MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			// Ŭ�� �������� ���Ⱚ�� �̴´�.
			char ReqMoveDir;
			*Message >> ReqMoveDir;

			// ���Ⱚ�� ���� �븻���� ���� �̾ƿ´�.
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

			CPlayer* MyPlayer = Session->MyPlayer;
			MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = MoveDir;
			MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;

			// �÷��̾��� ���� ��ġ�� �о�´�.
			st_Vector2Int PlayerPosition;
			PlayerPosition._X = MyPlayer->GetPositionInfo().PositionX;
			PlayerPosition._Y = MyPlayer->GetPositionInfo().PositionY;

			CChannel* Channel = G_ChannelManager->Find(1);

			// ������ ��ġ�� ��´�.	
			st_Vector2Int CheckPosition = PlayerPosition + DirVector2Int;

			// ������ ��ġ�� ���� �ִ��� Ȯ��
			bool IsCanGo = Channel->_Map->Cango(CheckPosition);
			bool ApplyMoveExe;
			if (IsCanGo == true)
			{
				// �� �� ������ �÷��̾� ��ġ ����
				ApplyMoveExe = Channel->_Map->ApplyMove(MyPlayer, CheckPosition);
			}

			// ���� ������ ���� �� ���� �÷��̾�鿡�� �˷�����
			CMessage* ResMyMoveOtherPacket = MakePacketResMove(Session->AccountId, MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
			SendPacketAroundSector(Session, ResMyMoveOtherPacket, true);
			ResMyMoveOtherPacket->Free();

			MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		}
		else
		{

		}
	} while (0);	

	ReturnSession(Session);
}

// int64 AccountId
// int32 PlayerDBId
// char Dir
void CGameServer::PacketProcReqAttack(int64 SessionID, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionID);

	do
	{
		int64 AccountId;
		int32 ObjectId;

		if (Session)
		{
			// �α��� ������ Ȯ��
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> AccountId;

			// AccountId�� �´��� Ȯ��
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> ObjectId;

			// �����ϰ� �ִ� �÷��̾ �ִ��� Ȯ��
			if (Session->MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �����ϰ� �ִ� �÷��̾��� ObjectId�� Ŭ�� ���� ObjectId�� ������ Ȯ��
				if (Session->MyPlayer->_GameObjectInfo.ObjectId != ObjectId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			CPlayer* MyPlayer = Session->MyPlayer;

			// ������ ����
			int8 ReqMoveDir;
			*Message >> ReqMoveDir;

			// ���� ����
			int16 ReqAttackRange;
			*Message >> ReqAttackRange;

			// ���� �Ÿ�
			int8 Distance;
			*Message >> Distance;
			
			// ���� ���� ĳ����
			en_MoveDir MoveDir = (en_MoveDir)ReqMoveDir;
			MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = MoveDir;

			// ���� ���� ĳ����
			en_AttackRange AttackRange = (en_AttackRange)ReqAttackRange;

			st_Vector2Int FrontCell;
			vector<CGameObject*> Targets;
			en_AttackType AttackType = NONE_ATTACK;
			switch (AttackRange)
			{
			case NORMAL_ATTACK:
			case FORWARD_ATTACK:
			{
				AttackType = PLAYER_NORMAL_ATTACK;

				FrontCell = MyPlayer->GetFrontCellPosition(MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, Distance);

				CGameObject* Target = MyPlayer->_Channel->_Map->Find(FrontCell);
				if (Target != nullptr)
				{
					Targets.push_back(Target);
				}
			}
			break;
			case AROUND_ONE_ATTACK:
			{
				AttackType = PLAYER_RANGE_ATTACK;

				vector<st_Vector2Int> TargetPositions;

				TargetPositions = MyPlayer->GetAroundCellPosition(MyPlayer->GetCellPosition(), Distance);
				for (st_Vector2Int TargetPosition : TargetPositions)
				{
					CGameObject* Target = MyPlayer->_Channel->_Map->Find(TargetPosition);
					if (Target != nullptr)
					{
						Targets.push_back(Target);
					}
				}
			}
			break;
			}

			// ũ��Ƽ�� �Ǵ�
			random_device RD;
			mt19937 Gen(RD());			

			uniform_int_distribution<int> CriticalPointCreate(0, 100);
			int32 CriticalPoint = CriticalPointCreate(Gen);			

			// �� ĳ������ ũ��Ƽ�� ����Ʈ���� ���� ������ ũ��Ƽ�÷� �Ǵ��Ѵ�.
			bool IsCritical = false;			
			if (CriticalPoint < MyPlayer->_GameObjectInfo.ObjectStatInfo.CriticalPoint)
			{
				IsCritical = true;			
			}

			// ���� ���� �鿡�� ���� ���� �����ϰ� �ִٰ� �˷���
			CMessage* ResMyAttackOtherPacket = MakePacketResAttack(Session->AccountId, MyPlayer->_GameObjectInfo.ObjectId, MoveDir, AttackType, IsCritical);
			SendPacketAroundSector(Session, ResMyAttackOtherPacket, true);
			ResMyAttackOtherPacket->Free();

			if (Targets.size() > 0)
			{
				for (CGameObject* Target : Targets)
				{
					st_Vector2Int PreviousTargetPosition = Target->GetCellPosition();
					Target->OnDamaged(MyPlayer, IsCritical ? MyPlayer->_GameObjectInfo.ObjectStatInfo.Attack * 2 : MyPlayer->_GameObjectInfo.ObjectStatInfo.Attack);

					CMessage* ResChangeHPPacket = MakePacketResChangeHP(Target->_GameObjectInfo.ObjectId,
						IsCritical ? MyPlayer->_GameObjectInfo.ObjectStatInfo.Attack * 2 : MyPlayer->_GameObjectInfo.ObjectStatInfo.Attack,
						Target->_GameObjectInfo.ObjectStatInfo.HP,
						Target->_GameObjectInfo.ObjectStatInfo.MaxHP,
						IsCritical,
						PreviousTargetPosition._X,
						PreviousTargetPosition._Y
					);
					SendPacketAroundSector(Session, ResChangeHPPacket, true);
					ResChangeHPPacket->Free();
				}
			}
		}
	} while (0);	

	ReturnSession(Session);
}

// int64 AccountId
// int32 PlayerDBId
// int32 X
// int32 Y
void CGameServer::PacketProcReqMousePositionObjectInfo(int64 SessionID, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionID);

	do
	{
		if (Session)
		{
			int64 AccountId;
			int32 ObjectId;

			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> AccountId;

			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> ObjectId;

			if (Session->MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				if (Session->MyPlayer->_GameObjectInfo.ObjectId != ObjectId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			int32 X;
			int32 Y;

			*Message >> X;
			*Message >> Y;

			CChannel* Channel = G_ChannelManager->Find(1);

			st_Vector2Int FindPosition;
			FindPosition._X = X;
			FindPosition._Y = Y;
						
			CGameObject* FindObject = Channel->_Map->Find(FindPosition);

			if (FindObject != nullptr)
			{
				CPlayer* Player = (CPlayer*)FindObject;

				CMessage* ResMousePositionObjectInfo = MakePacketMousePositionObjectInfo(Session->AccountId, Player->_GameObjectInfo.ObjectId, Player->_GameObjectInfo);
				SendPacket(Session->SessionId, ResMousePositionObjectInfo);
				ResMousePositionObjectInfo->Free();
			}
		}
	} while (0);	

	ReturnSession(Session);
}

// int32 ObjectId
// string Message
void CGameServer::PacketProcReqChattingMessage(int64 SessionId, CMessage* Message)
{
	// ���� ���
	st_SESSION* Session = FindSession(SessionId);

	int64 AccountId;
	int32 PlayerDBId;

	if (Session)
	{
		// �α��� ������ Ȯ��
		if (!Session->IsLogin)
		{
			Disconnect(Session->SessionId);
			return;
		}

		*Message >> AccountId;

		// AccountId�� �´��� Ȯ��
		if (Session->AccountId != AccountId)
		{
			Disconnect(Session->SessionId);
			return;
		}

		*Message >> PlayerDBId;

		// �����ϰ� �ִ� �÷��̾ �ִ��� Ȯ�� 
		if (Session->MyPlayer == nullptr)
		{
			Disconnect(Session->SessionId);
			return;
		}
		else
		{
			// �����ϰ� �ִ� �÷��̾�� ���۹��� PlayerId�� ������ Ȯ��
			if (Session->MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
			{
				Disconnect(Session->SessionId);
				return;
			}
		}

		// ä�� �޼��� ���� 
		int8 ChattingMessageLen;
		*Message >> ChattingMessageLen;

		// ä�� �޼��� ������
		wstring ChattingMessage;
		Message->GetData(ChattingMessage, ChattingMessageLen);

		// ä�� ã��
		CChannel* Channel = G_ChannelManager->Find(1);
		// ���� �÷��̾� ��ȯ
		vector<CPlayer*> Players = Channel->GetAroundPlayer(Session->MyPlayer, 10, false);

		// ���� �÷��̾�� ä�� �޼��� ����
		for (CPlayer* Player : Players)
		{
			CMessage* ResChattingMessage = MakePacketResChattingMessage(PlayerDBId, en_MessageType::CHATTING, ChattingMessage);
			SendPacket(Player->_SessionId, ResChattingMessage);
			ResChattingMessage->Free();
		}
	}

	// ���� �ݳ�
	ReturnSession(Session);
}

void CGameServer::PacketProcReqItemToInventory(int64 SessionId, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionId);
	
	do
	{
		if (Session)
		{
			int64 AccountId;
			int64 ItemId;

			// �α��� ������ Ȯ��
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> AccountId;

			// AccountId Ȯ��
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			// ItemId�� ObjectType�� �д´�.
			*Message >> ItemId;

			int8 ObjectType;

			*Message >> ObjectType;

			// �������� ObjectManager�� �ִ��� Ȯ���Ѵ�.
			CItem* Item = (CItem*)(G_ObjectManager->Find(ItemId, (en_GameObjectType)ObjectType));			
			if (Item != nullptr)
			{
				int64 TargetObjectId;

				*Message >> TargetObjectId;

				CPlayer* TargetPlayer = (CPlayer*)(G_ObjectManager->Find(TargetObjectId, en_GameObjectType::PLAYER));
				if (TargetPlayer == nullptr)
				{
					break;
				}
								
				int32 EmptySlot = 0;

				// �������� �̹� ���� �ϴ��� Ȯ���Ѵ�.
				// ���� �ϸ� ������ 1 ������Ų��.
				bool IsExistItem = TargetPlayer->_Inventory.IsExistItem(Item->_ItemInfo.ItemType);

				// ���� ���� ���� ���
				if (IsExistItem == false)
				{
					// �� ���Թ�ȣ�� ���ͼ� �κ��丮�� �߰��Ѵ�.
					if (TargetPlayer->_Inventory.GetEmptySlot(&EmptySlot))
					{
						Item->_ItemInfo.ItemDBId = Item->_GameObjectInfo.ObjectId;
						Item->_ItemInfo.SlotNumber = EmptySlot;
						TargetPlayer->_Inventory.Add(Item);
					}
				}				

				// �������� �ߺ��Ǿ� ������ �ռ� �������� Count�� 1 ���������� �״� �ش� �������� �ݳ��ϰ�,
				// �� ���� ��쿡�� �ݳ����� �ʴ´�.
				G_ObjectManager->Remove(Item, 1, IsExistItem);

				CMessage* ResItemToInventoryPacket = MakePacketResItemToInventory(TargetPlayer->_GameObjectInfo.ObjectId, Item->_ItemInfo);
				SendPacket(TargetPlayer->_SessionId, ResItemToInventoryPacket);
				ResItemToInventoryPacket->Free();
			}
			else
			{
				CRASH("�ش� �������� G_ObjectManager�� ����");
			}
		}
	} while (0);

	ReturnSession(Session);	
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
	st_SESSION* Session = FindSession(SessionID);

	int64 AccountNo;
	WORD SectorX;
	WORD SectorY;

	if (Session)
	{
		//Ŭ�� �α��� ������ �Ǵ�
		if (!Session->IsLogin)
		{
			Disconnect(Session->SessionId);
			return;
		}

		*Message >> AccountNo;

		if (Session->AccountId != AccountNo)
		{
			Disconnect(Session->SessionId);
			return;
		}

		*Message >> SectorX;
		*Message >> SectorY;

		//���� X Y ��ǥ ���� �˻�
		if (SectorX < 0 || SectorX >= SECTOR_X_MAX || SectorY < 0 || SectorY >= SECTOR_Y_MAX)
		{
			Disconnect(Session->SessionId);
			return;
		}

		//���� ���Ϳ��� Ŭ�� ����
		if (Session->SectorX != -1 && Session->SectorY != -1)
		{
			_SectorList[Session->SectorY][Session->SectorX].remove(Session->SessionId);
		}

		Session->SectorY = SectorY;
		Session->SectorX = SectorX;

		_SectorList[Session->SectorY][Session->SectorX].push_back(Session->SessionId);

		/*CMessage* SectorMoveResMessage = MakePacketResSectorMove(Client->AccountId, Client->SectorX, Client->SectorY);
		SendPacket(Client->SessionId, SectorMoveResMessage);
		SectorMoveResMessage->Free();*/
	}
	else
	{

	}

	ReturnSession(Session);
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
	st_SESSION* Session = FindSession(SessionID);

	int64 AccountNo;
	int MessageLen = 0;
	WORD CSMessageLen;
	WCHAR CSMessage[1024];

	memset(CSMessage, 0, sizeof(CSMessage));

	if (!Session->IsLogin)
	{
		Disconnect(Session->SessionId);
		return;
	}

	*Message >> AccountNo;
	if (Session->AccountId != AccountNo)
	{
		Disconnect(Session->SessionId);
		return;
	}

	*Message >> CSMessageLen;
	MessageLen = Message->GetData(CSMessage, CSMessageLen);
	if (MessageLen != CSMessageLen)
	{
		Disconnect(Session->SessionId);
		return;
	}

	*Message >> CSMessageLen;
	int MessageUseBufSize = Message->GetUseBufferSize();

	if (MessageUseBufSize - sizeof(CMessage::st_ENCODE_HEADER) != CSMessageLen)
	{
		Disconnect(Session->SessionId);
		return;
	}

	Message->GetData(CSMessage, CSMessageLen);

	CMessage* ChattingResMessage = nullptr;
	//ChattingResMessage = MakePacketResMessage(Client->AccountID, Client->ClientID, Client->NickName, CSMessageLen, CSMessage);
	ChattingResMessage->Free();

	ReturnSession(Session);
}

//---------------------------------------------------------------------------------
//��Ʈ ��Ʈ
//WORD Type
//---------------------------------------------------------------------------------
void CGameServer::PacketProcReqHeartBeat(int64 SessionID, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionID);

	Session->RecvPacketTime = timeGetTime();

	ReturnSession(Session);
}

void CGameServer::PacketProcReqAccountCheck(int64 SessionID, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionID);

	// DB���� ��û�ϱ� ���� �������״� IOCount�� ����
	Session->IOBlock->IOCount--;

	bool Status = LOGIN_SUCCESS;

	if (Session)
	{
		int64 ClientAccountId = Session->AccountId;
		int32 Token = Session->Token;

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
			Session->IsLogin = true;
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
			int32 PlayerCriticalPoint;
			float PlayerSpeed;

			ClientPlayersGet.OutPlayerDBID(PlayerId);
			ClientPlayersGet.OutPlayerName(PlayerName);
			ClientPlayersGet.OutLevel(PlayerLevel);
			ClientPlayersGet.OutCurrentHP(PlayerCurrentHP);
			ClientPlayersGet.OutMaxHP(PlayerMaxHP);
			ClientPlayersGet.OutAttack(PlayerAttack);
			ClientPlayersGet.OutCriticalPoint(PlayerCriticalPoint);
			ClientPlayersGet.OutSpeed(PlayerSpeed);

			ClientPlayersGet.Execute();

			int PlayerCount = 0;

			while (ClientPlayersGet.Fetch())
			{
				// �÷��̾� ���� ����
				Session->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectId = PlayerId;
				Session->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectName = PlayerName;
				Session->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectStatInfo.Level = PlayerLevel;
				Session->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectStatInfo.HP = PlayerCurrentHP;
				Session->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectStatInfo.MaxHP = PlayerMaxHP;
				Session->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectStatInfo.Attack = PlayerAttack;
				Session->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectStatInfo.CriticalPoint = PlayerCriticalPoint;
				Session->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectStatInfo.Speed = PlayerSpeed;
				Session->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
				Session->MyPlayers[PlayerCount]->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
				Session->MyPlayers[PlayerCount]->_GameObjectInfo.OwnerObjectId = 0;
				Session->MyPlayers[PlayerCount]->_SessionId = Session->SessionId;				

				PlayerCount++;
			}

			// Ŭ�󿡰� �α��� ���� ��Ŷ ����
			CMessage* ResLoginMessage = MakePacketResLogin(Status, PlayerCount, Session->MyPlayers[0]->_GameObjectInfo.ObjectId, Session->MyPlayers[0]->_GameObjectInfo.ObjectName);
			SendPacket(Session->SessionId, ResLoginMessage);
			ResLoginMessage->Free();

			G_DBConnectionPool->Push(en_DBConnect::GAME, GameServerDBConnection);
		}
		else
		{
			Session->IsLogin = false;
			Disconnect(SessionID);
		}

		ReturnSession(Session);
	}
	else
	{
		// Ŭ�� ���� ������ ���
	}
}

void CGameServer::PacketProcReqCreateCharacterNameCheck(int64 SessionID, CMessage* Message)
{
	int32 PlayerDBId;

	st_SESSION* Session = FindSession(SessionID);

	// DB���� ��û�ϱ� ���� �������״� IOCount�� ����
	Session->IOBlock->IOCount--;

	if (Session)
	{
		// ��û�� Ŭ���� ������ ĳ������ �̸��� �����´�.
		wstring CreateCharacterName = Session->CreateCharacterName;

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
			NewCharacterPush.InAccountID(Session->AccountId);
			NewCharacterPush.InCharacterName(Session->CreateCharacterName);
			NewCharacterPush.InLevel(NewCharacterStatus.Level);
			NewCharacterPush.InCurrentHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InMaxHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InAttack(NewCharacterStatus.Attack);
			NewCharacterPush.InCriticalPoint(NewCharacterStatus.CriticalPoint);
			NewCharacterPush.InSpeed(NewCharacterStatus.Speed);

			NewCharacterPush.Execute();

			G_DBConnectionPool->Push(en_DBConnect::GAME, NewCharacterPushDBConnection);

			// �ռ� ������ ĳ������ DBId�� AccountId�� �̿��� ���´�.
			CDBConnection* PlayerDBIDGetDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerPlayerDBIDGet PlayerDBIDGet(*PlayerDBIDGetDBConnection);
			PlayerDBIDGet.InAccountID(Session->AccountId);

			PlayerDBIDGet.OutPlayerDBID(PlayerDBId);

			PlayerDBIDGet.Execute();

			PlayerDBIDGet.Fetch();

			// DB���� �о�� DBId�� ����
			Session->MyPlayers[0]->_GameObjectInfo.ObjectId = PlayerDBId;
			// ĳ������ �̸��� ����
			Session->MyPlayers[0]->_GameObjectInfo.ObjectName = Session->CreateCharacterName;

			Session->MyPlayers[0]->_GameObjectInfo.ObjectStatInfo.Level = NewCharacterStatus.Level;
			Session->MyPlayers[0]->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
			Session->MyPlayers[0]->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
			Session->MyPlayers[0]->_GameObjectInfo.ObjectStatInfo.Attack = NewCharacterStatus.Attack;
			Session->MyPlayers[0]->_GameObjectInfo.ObjectStatInfo.CriticalPoint = NewCharacterStatus.CriticalPoint;
			Session->MyPlayers[0]->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
			Session->MyPlayers[0]->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			Session->MyPlayers[0]->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
			Session->MyPlayers[0]->_SessionId = Session->SessionId;

			G_DBConnectionPool->Push(en_DBConnect::GAME, PlayerDBIDGetDBConnection);
		}
		else
		{
			// ĳ���Ͱ� �̹� DB�� �ִ� ���
			PlayerDBId = Session->MyPlayers[0]->_GameObjectInfo.ObjectId;
		}

		// ĳ���� ���� ���� ����
		CMessage* ResCreateCharacterMessage = MakePacketResCreateCharacter(!CharacterNameFind, PlayerDBId, Session->CreateCharacterName);
		SendPacket(Session->SessionId, ResCreateCharacterMessage);
		ResCreateCharacterMessage->Free();
	}
	else
	{

	}

	ReturnSession(Session);
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
		int8 PlayerNameLen = (int8)(PlayersName.length() * 2);
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

	int8 PlayerNameLen = (int8)(PlayerName.length() * 2);
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
	int8 EnterPlayerNameLen = (int8)(ObjectInfo.ObjectName.length() * 2);
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
	*ResEnterGamePacket << ObjectInfo.ObjectStatInfo.CriticalPoint;
	*ResEnterGamePacket << ObjectInfo.ObjectStatInfo.Speed;

	// ObjectType
	*ResEnterGamePacket << (int8)ObjectInfo.ObjectType;

	return ResEnterGamePacket;
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
	int8 ObjectNameLen = (int8)(ObjectInfo.ObjectName.length() * 2);
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
	*ResEnterGamePacket << ObjectInfo.ObjectStatInfo.CriticalPoint;
	*ResEnterGamePacket << ObjectInfo.ObjectStatInfo.Speed;

	// ObjectType
	*ResEnterGamePacket << (int8)ObjectInfo.ObjectType;

	return ResEnterGamePacket;
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

// int64 AccountId
// int32 PlayerDBId
// char Dir
CMessage* CGameServer::MakePacketResAttack(int64 AccountId, int32 PlayerDBId, en_MoveDir Dir, en_AttackType AttackType, bool IsCritical)
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
	*ResAttackMessage << (int8)AttackType;
	*ResAttackMessage << IsCritical;

	return ResAttackMessage;
}

// int64 AccountId
// int32 PlayerDBId
// int32 HP
CMessage* CGameServer::MakePacketResChangeHP(int32 PlayerDBId, int32 Damage, int32 CurrentHP, int32 MaxHP, bool IsCritical, int32 TargetPositionX, int32 TargetPositionY)
{
	CMessage* ResChangeHPPacket = CMessage::Alloc();
	if (ResChangeHPPacket == nullptr)
	{
		return nullptr;
	}

	ResChangeHPPacket->Clear();

	*ResChangeHPPacket << (WORD)en_PACKET_S2C_CHANGE_HP;
	*ResChangeHPPacket << PlayerDBId;

	*ResChangeHPPacket << Damage;
	*ResChangeHPPacket << CurrentHP;
	*ResChangeHPPacket << MaxHP;
	*ResChangeHPPacket << IsCritical;
	*ResChangeHPPacket << TargetPositionX;
	*ResChangeHPPacket << TargetPositionY;

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

	// Spawn ������Ʈ ����
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
		*ResSpawnPacket << ObjectInfos[i].ObjectStatInfo.CriticalPoint;
		*ResSpawnPacket << ObjectInfos[i].ObjectStatInfo.Speed;

		// ObjectType
		*ResSpawnPacket << (int8)ObjectInfos[i].ObjectType;
		*ResSpawnPacket << ObjectInfos[i].OwnerObjectId;
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

// int32 PlayerDBId
// wstring ChattingMessage
CMessage* CGameServer::MakePacketResChattingMessage(int32 PlayerDBId, en_MessageType MessageType, wstring ChattingMessage)
{
	CMessage* ResChattingMessage = CMessage::Alloc();
	if (ResChattingMessage == nullptr)
	{
		return nullptr;
	}

	ResChattingMessage->Clear();

	*ResChattingMessage << (WORD)en_PACKET_S2C_MESSAGE;
	*ResChattingMessage << PlayerDBId;
	*ResChattingMessage << (WORD)MessageType;

	int8 PlayerNameLen = (int8)(ChattingMessage.length() * 2);
	*ResChattingMessage << PlayerNameLen;
	ResChattingMessage->InsertData(ChattingMessage.c_str(), PlayerNameLen);

	return ResChattingMessage;
}

CMessage* CGameServer::MakePacketResItemToInventory(int64 TargetObjectId, st_ItemInfo ItemInfo)
{
	CMessage* ResItemToInventoryMessage = CMessage::Alloc();
	if (ResItemToInventoryMessage == nullptr)
	{
		return nullptr;
	}

	*ResItemToInventoryMessage << (WORD)en_PACKET_S2C_ITEM_TO_INVENTORY;
	*ResItemToInventoryMessage << TargetObjectId;
	*ResItemToInventoryMessage << ItemInfo.ItemDBId;
	*ResItemToInventoryMessage << ItemInfo.Count;
	*ResItemToInventoryMessage << ItemInfo.SlotNumber;
	*ResItemToInventoryMessage << ItemInfo.IsEquipped;
	*ResItemToInventoryMessage << (int16)ItemInfo.ItemType;

	int8 ItemNameLen = (int8)(ItemInfo.ItemName.length() * 2);
	*ResItemToInventoryMessage << ItemNameLen;
	ResItemToInventoryMessage->InsertData(ItemInfo.ItemName.c_str(), ItemNameLen);

	int8 ThumbnailImagePathLen = (int)(ItemInfo.ThumbnailImagePath.length() * 2);
	*ResItemToInventoryMessage << ThumbnailImagePathLen;
	ResItemToInventoryMessage->InsertData(ItemInfo.ThumbnailImagePath.c_str(), ThumbnailImagePathLen);

	return ResItemToInventoryMessage;
}

void CGameServer::OnClientJoin(int64 SessionID)
{
	st_Job* ClientJoinJob = _JobMemoryPool->Alloc();
	ClientJoinJob->Type = AUTH_NEW_CLIENT_JOIN;
	ClientJoinJob->SessionID = SessionID;
	ClientJoinJob->Message = nullptr;
	_GameServerAuthThreadMessageQue.Enqueue(ClientJoinJob);
	SetEvent(_AuthThreadWakeEvent);
}

void CGameServer::OnRecv(int64 SessionID, CMessage* Packet)
{
	st_Job* NewMessageJob = _JobMemoryPool->Alloc();
	CMessage* JobMessage = CMessage::Alloc();
	JobMessage->Clear();
	JobMessage->SetHeader(Packet->GetBufferPtr(), sizeof(CMessage::st_ENCODE_HEADER));
	JobMessage->InsertData(Packet->GetFrontBufferPtr(), Packet->GetUseBufferSize() - sizeof(CMessage::st_ENCODE_HEADER));

	NewMessageJob->Type = MESSAGE;
	NewMessageJob->SessionID = SessionID;
	NewMessageJob->Message = JobMessage;
	_GameServerNetworkThreadMessageQue.Enqueue(NewMessageJob);
	SetEvent(_NetworkThreadWakeEvent);
}

void CGameServer::OnClientLeave(st_SESSION* LeaveSession)
{
	st_Job* ClientLeaveJob = _JobMemoryPool->Alloc();
	ClientLeaveJob->Type = AUTH_DISCONNECT_CLIENT;
	ClientLeaveJob->SessionID = LeaveSession->SessionId;
	ClientLeaveJob->Session = LeaveSession;
	ClientLeaveJob->Message = nullptr;

	_GameServerAuthThreadMessageQue.Enqueue(ClientLeaveJob);
	SetEvent(_AuthThreadWakeEvent);
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
			SendPacket(Player->_SessionId, Message);
		}
	}
}

void CGameServer::SendPacketAroundSector(st_SESSION* Session, CMessage* Message, bool SendMe)
{
	CChannel* Channel = G_ChannelManager->Find(1);
	vector<CSector*> Sectors = Channel->GetAroundSectors(Session->MyPlayer->GetCellPosition(), 10);

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
				if (Session->SessionId != Player->_SessionId)
				{
					SendPacket(Player->_SessionId, Message);
				}
			}
		}
	}
}