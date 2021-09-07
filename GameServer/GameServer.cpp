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
#include <atlbase.h>

CGameServer::CGameServer()
{
	//timeBeginPeriod(1);
	_AuthThread = nullptr;
	_NetworkThread = nullptr;
	_DataBaseThread = nullptr;

	// Nonsignaled상태 자동리셋
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

	G_ObjectManager->MonsterSpawn(200, 1, en_GameObjectType::SLIME);
	G_ObjectManager->MonsterSpawn(200, 1, en_GameObjectType::BEAR);

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
			case en_MESSAGE_TYPE::AUTH_NEW_CLIENT_JOIN:
				Instance->CreateNewClient(Job->SessionId);
				break;
			case en_MESSAGE_TYPE::AUTH_DISCONNECT_CLIENT:
				Instance->DeleteClient(Job->Session);
				break;
			case en_MESSAGE_TYPE::AUTH_MESSAGE:
				break;
			default:
				Instance->Disconnect(Job->SessionId);
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
			case en_MESSAGE_TYPE::NETWORK_MESSAGE:
				Instance->PacketProc(Job->SessionId, Job->Message);
				break;
			default:
				Instance->Disconnect(Job->SessionId);
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
			case en_MESSAGE_TYPE::DATA_BASE_ACCOUNT_CHECK:
				Instance->PacketProcReqDBAccountCheck(Job->SessionId, Job->Message);
				break;
			case en_MESSAGE_TYPE::DATA_BASE_CHARACTER_CHECK:
				Instance->PacketProcReqDBCreateCharacterNameCheck(Job->SessionId, Job->Message);
				break;
			case en_MESSAGE_TYPE::DATA_BASE_ITEM_CREATE:
				Instance->PacketProcReqDBItemCreate(Job->Message);
				break;
			case en_MESSAGE_TYPE::DATA_BASE_ITEM_INVENTORY_SAVE:
				Instance->PacketProcReqDBItemToInventorySave(Job->SessionId, Job->Message);
				break;
			case en_MESSAGE_TYPE::DATA_BASE_ITEM_SWAP:
				Instance->PacketProcReqDBItemSwap(Job->SessionId, Job->Message);
				break;
			case en_MESSAGE_TYPE::DATA_BASE_GOLD_SAVE:
				Instance->PacketProcReqDBGoldSave(Job->SessionId, Job->Message);
				break;
			case en_MESSAGE_TYPE::DATA_BASE_CHARACTER_INFO_SEND:
				Instance->PacketProcReqDBCharacterInfoSend(Job->SessionId, Job->Message);
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
	// 세션 찾기
	st_SESSION* Session = FindSession(SessionId);

	// 기본 정보 셋팅
	Session->AccountId = 0;
	Session->IsLogin = false;
	Session->Token = 0;
	Session->RecvPacketTime = timeGetTime();

	for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
	{
		Session->MyPlayers[i] = (CPlayer*)G_ObjectManager->ObjectCreate(en_GameObjectType::MELEE_PLAYER);
	}

	Session->MyPlayer = nullptr;

	// 게임 서버 접속 성공을 클라에게 알려준다.
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

		for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
		{
			Session->MyPlayers[i]->_NetworkState = en_ObjectNetworkState::LEAVE;
			G_ObjectManager->Remove(Session->MyPlayers[i], 1);
			Session->MyPlayers[i] = nullptr;
		}
	}

	Session->MyPlayer = nullptr;
	Session->AccountId = 0;

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
	case en_PACKET_TYPE::en_PACKET_C2S_GAME_REQ_LOGIN:
		PacketProcReqLogin(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_GAME_CREATE_CHARACTER:
		PacketProcReqCreateCharacter(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_GAME_ENTER:
		PacketProcReqEnterGame(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_MOVE:
		PacketProcReqMove(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_ATTACK:
		PacketProcReqMeleeAttack(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_MAGIC_ATTACK:
		PacketProcReqMagic(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_MOUSE_POSITION_OBJECT_INFO:
		PacketProcReqMousePositionObjectInfo(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_OBJECT_STATE_CHANGE:
		PacketProcReqObjectStateChange(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_MESSAGE:
		PacketProcReqChattingMessage(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_ITEM_TO_INVENTORY:
		PacketProcReqItemToInventory(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_ITEM_SWAP:
		PacketProcReqItemSwap(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_CS_GAME_REQ_SECTOR_MOVE:
		PacketProcReqSectorMove(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_CS_GAME_REQ_MESSAGE:
		PacketProcReqMessage(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_CS_GAME_REQ_HEARTBEAT:
		PacketProcReqHeartBeat(SessionId, Message);
		break;
	default:
		Disconnect(SessionId);
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
	st_SESSION* Session = FindSession(SessionID);

	int64 AccountId;
	int32 Token;

	if (Session != nullptr)
	{
		// AccountID 셋팅
		*Message >> AccountId;

		// 중복 로그인 확인
		for (st_SESSION* FindSession : _SessionArray)
		{
			if (FindSession->AccountId == AccountId)
			{
				// 새로 접속한 중복 되는 유저 연결 끊음
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

		// 토큰 셋팅
		*Message >> Token;
		Session->Token = Token;

		if (Session->AccountId != 0 && Session->AccountId != AccountId)
		{
			Disconnect(Session->SessionId);
			ReturnSession(Session);
			return;
		}

		// DB 큐에 요청하기 전 IOCount를 증가시켜서 Session이 반납 안되도록 막음
		InterlockedIncrement64(&Session->IOBlock->IOCount);

		st_Job* DBAccountCheckJob = _JobMemoryPool->Alloc();
		DBAccountCheckJob->Type = en_MESSAGE_TYPE::DATA_BASE_ACCOUNT_CHECK;
		DBAccountCheckJob->SessionId = Session->SessionId;
		DBAccountCheckJob->Message = nullptr;

		_GameServerDataBaseThreadMessageQue.Enqueue(DBAccountCheckJob);
		SetEvent(_DataBaseWakeEvent);
	}
	else
	{
		// 해당 클라가 없음
		bool Status = LOGIN_FAIL;
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqCreateCharacter(int64 SessionID, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionID);

	if (Session)
	{
		// 클라에서 선택한 플레이어 오브젝트 타입
		int16 ReqGameObjectType;
		*Message >> ReqGameObjectType;

		// 캐릭터 이름 길이
		int8 CharacterNameLen;
		*Message >> CharacterNameLen;

		WCHAR CharacterName[20];
		memset(CharacterName, 0, sizeof(WCHAR) * 20);
		Message->GetData(CharacterName, CharacterNameLen);

		// 캐릭터 이름 셋팅
		Session->CreateCharacterName = CharacterName;

		// 클라에서 선택한 플레이어 인덱스
		byte CharacterCreateSlotIndex;
		*Message >> CharacterCreateSlotIndex;

		InterlockedIncrement64(&Session->IOBlock->IOCount);

		CMessage* DBReqChatacerCreateMessage = CMessage::Alloc();
		DBReqChatacerCreateMessage->Clear();

		*DBReqChatacerCreateMessage << ReqGameObjectType;
		*DBReqChatacerCreateMessage << CharacterCreateSlotIndex;

		st_Job* DBCharacterCheckJob = _JobMemoryPool->Alloc();
		DBCharacterCheckJob->Type = en_MESSAGE_TYPE::DATA_BASE_CHARACTER_CHECK;
		DBCharacterCheckJob->SessionId = Session->SessionId;
		DBCharacterCheckJob->Message = DBReqChatacerCreateMessage;

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

	do
	{
		if (Session)
		{
			// 로그인 중이 아니면 나간다.
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			int64 AccountId;
			*Message >> AccountId;

			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			int8 EnterGameCharacterNameLen;
			*Message >> EnterGameCharacterNameLen;

			WCHAR EnterGameCharacterName[20];
			memset(EnterGameCharacterName, 0, sizeof(WCHAR) * 20);
			Message->GetData(EnterGameCharacterName, EnterGameCharacterNameLen);

			bool FindName = false;
			// 클라가 가지고 있는 캐릭 중에 패킷으로 받은 캐릭터가 있는지 확인한다.
			for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
			{
				if (Session->MyPlayers[i]->_GameObjectInfo.ObjectName == EnterGameCharacterName)
				{
					FindName = true;
					Session->MyPlayer = Session->MyPlayers[i];
					Session->MyPlayer->_NetworkState = en_ObjectNetworkState::LIVE;
					break;
				}
			}

			if (FindName == false)
			{
				CRASH("클라가 가지고 있는 캐릭 중 패킷으로 받은 캐릭터가 없음");
				break;
			}

			// ObjectManager에 플레이어 추가 및 패킷 전송
			G_ObjectManager->Add(Session->MyPlayer, 1);

			// 나한테 나 생성하라고 알려줌
			CMessage* ResEnterGamePacket = MakePacketResEnterGame(Session->MyPlayer->_GameObjectInfo);
			SendPacket(Session->SessionId, ResEnterGamePacket);
			ResEnterGamePacket->Free();

			vector<st_GameObjectInfo> SpawnObjectInfo;
			SpawnObjectInfo.push_back(Session->MyPlayer->_GameObjectInfo);

			// 다른 플레이어들한테 나를 생성하라고 알려줌
			CMessage* ResSpawnPacket = MakePacketResSpawn(1, SpawnObjectInfo);
			SendPacketAroundSector(Session, ResSpawnPacket);
			ResSpawnPacket->Free();

			SpawnObjectInfo.clear();

			// 나한테 다른 오브젝트들을 생성하라고 알려줌						
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

			// DB 큐에 요청하기 전 IOCount를 증가시켜서 Session이 반납 안되도록 막음
			InterlockedIncrement64(&Session->IOBlock->IOCount);

			st_Job* DBCharacterInfoSendJob = _JobMemoryPool->Alloc();
			DBCharacterInfoSendJob->Type = en_MESSAGE_TYPE::DATA_BASE_CHARACTER_INFO_SEND;
			DBCharacterInfoSendJob->SessionId = Session->SessionId;
			DBCharacterInfoSendJob->Message = nullptr;

			_GameServerDataBaseThreadMessageQue.Enqueue(DBCharacterInfoSendJob);
			SetEvent(_DataBaseWakeEvent);
		}
		else
		{
			break;
		}
	} while (0);

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
			// 클라가 로그인중인지 확인
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			// AccountId를 뽑고
			int64 AccountId;
			*Message >> AccountId;

			// 클라에 들어있는 AccountId와 뽑은 AccoountId가 다른지 확인
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			// PlayerDBId를 뽑는다.
			int64 PlayerDBId;
			*Message >> PlayerDBId;

			// 클라가 조종중인 캐릭터가 있는지 확인
			if (Session->MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종중인 캐릭터가 있으면 ObjectId가 다른지 확인
				if (Session->MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			// 클라가 전송해준 방향값을 뽑는다.
			char ReqMoveDir;
			*Message >> ReqMoveDir;

			// 방향값에 따라 노말벡터 값을 뽑아온다.
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

			// 플레이어의 현재 위치를 읽어온다.
			st_Vector2Int PlayerPosition;
			PlayerPosition._X = MyPlayer->GetPositionInfo().PositionX;
			PlayerPosition._Y = MyPlayer->GetPositionInfo().PositionY;

			CChannel* Channel = G_ChannelManager->Find(1);

			// 움직일 위치를 얻는다.	
			st_Vector2Int CheckPosition = PlayerPosition + DirVector2Int;

			// 움직일 위치로 갈수 있는지 확인
			bool IsCanGo = Channel->_Map->Cango(CheckPosition);
			bool ApplyMoveExe;
			if (IsCanGo == true)
			{
				// 갈 수 있으면 플레이어 위치 적용
				ApplyMoveExe = Channel->_Map->ApplyMove(MyPlayer, CheckPosition);
			}

			// 내가 움직인 것을 내 주위 플레이어들에게 알려야함
			CMessage* ResMyMoveOtherPacket = MakePacketResMove(Session->AccountId, MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
			SendPacketAroundSector(Session, ResMyMoveOtherPacket, true);
			ResMyMoveOtherPacket->Free();

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
void CGameServer::PacketProcReqMeleeAttack(int64 SessionID, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionID);

	do
	{
		int64 AccountId;
		int32 ObjectId;

		if (Session)
		{
			// 로그인 중인지 확인
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> AccountId;

			// AccountId가 맞는지 확인
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> ObjectId;

			// 조종하고 있는 플레이어가 있는지 확인
			if (Session->MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종하고 있는 플레이어의 ObjectId와 클라가 보낸 ObjectId가 같은지 확인
				if (Session->MyPlayer->_GameObjectInfo.ObjectId != ObjectId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			CPlayer* MyPlayer = Session->MyPlayer;

			// 공격한 방향
			int8 ReqMoveDir;
			*Message >> ReqMoveDir;

			// 공격 종류
			int16 ReqAttackRange;
			*Message >> ReqAttackRange;

			// 공격 타입
			int16 ReqAttackType;
			*Message >> ReqAttackType;

			// 공격 거리
			int8 Distance;
			*Message >> Distance;

			// 공격 방향 캐스팅
			en_MoveDir MoveDir = (en_MoveDir)ReqMoveDir;
			MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = MoveDir;

			st_Vector2Int FrontCell;
			vector<CGameObject*> Targets;

			// 공격종류 확인
			switch ((en_AttackRange)ReqAttackRange)
			{
			case en_AttackRange::NORMAL_ATTACK:
			case en_AttackRange::FORWARD_ATTACK:
			case en_AttackRange::MAGIC_ATTACK:
			{
				FrontCell = MyPlayer->GetFrontCellPosition(MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, Distance);
				CGameObject* Target = MyPlayer->_Channel->_Map->Find(FrontCell);
				if (Target != nullptr)
				{
					Targets.push_back(Target);
				}
			}
			break;
			case en_AttackRange::AROUND_ONE_ATTACK:
			{
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

			switch ((en_AttackType)ReqAttackType)
			{
			case en_AttackType::MELEE_PLAYER_NORMAL_ATTACK:
			case en_AttackType::MELEE_PLAYER_CHOHONE_ATTACK:
			case en_AttackType::MELEE_PLAYER_SHAEHONE_ATTACK:
			case en_AttackType::MELEE_PLAYER_AROUND_ATTACK:
			case en_AttackType::MAGIC_PLAYER_NORMAL_ATTACK:
				MyPlayer->SetAttackMeleeType((en_AttackType)ReqAttackType, Targets);
				break;
			default:
				CRASH("ReqAttack AttackType Error");
				break;
			}
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqMagic(int64 SessionId, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionId);

	if (Session)
	{
		do
		{
			int64 AccountId;
			int32 ObjectId;

			// 로그인중인지 확인
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> AccountId;

			// AccountId 확인
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> ObjectId;

			// 조종하고 잇는 캐릭이 있는지 확인
			if (Session->MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종하고 있는 캐릭의 ObjectId와 클라가 전송한 ObjectId가 같은지 확인
				if (Session->MyPlayer->_GameObjectInfo.ObjectId != ObjectId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			CPlayer* MyPlayer = Session->MyPlayer;

			// 공격한 방향
			int8 ReqMoveDir;
			*Message >> ReqMoveDir;

			// 공격 종류
			int16 ReqAttackRange;
			*Message >> ReqAttackRange;

			// 공격 타입
			int16 ReqAttackType;
			*Message >> ReqAttackType;

			// 공격 거리
			int8 Distance;
			*Message >> Distance;

			// 타겟 ObjectId
			int64 TargetObjectId;
			*Message >> TargetObjectId;

			// 타겟 ObjectType
			int8 TargetObjectType;
			*Message >> TargetObjectType;

			vector<CGameObject*> Targets;

			// 공격 종류 확인
			switch ((en_AttackRange)ReqAttackRange)
			{
			case en_AttackRange::MAGIC_ATTACK:
				// 타겟이 ObjectManager에 존재하는지 확인
				CGameObject* FindGameObject = G_ObjectManager->Find(TargetObjectId, (en_GameObjectType)TargetObjectType);
				if (FindGameObject != nullptr)
				{
					Targets.push_back(FindGameObject);
				}
				else
				{
					// 타겟을 찾을 수 없음
				}
				break;
			}

			// 공격 타입 확인하고 공격 타입 셋팅
			switch ((en_AttackType)ReqAttackType)
			{
			case en_AttackType::MAGIC_PLAYER_FIRE_ATTACK:
				MyPlayer->SetAttackMagicType((en_AttackType)ReqAttackType, Targets);
				break;
			default:
				CRASH("ReqAttack AttackType Error");
				break;
			}
		} while (0);

		ReturnSession(Session);
	}
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

				CMessage* ResMousePositionObjectInfo = MakePacketMousePositionObjectInfo(Session->AccountId, Player->_GameObjectInfo);
				SendPacket(Session->SessionId, ResMousePositionObjectInfo);
				ResMousePositionObjectInfo->Free();
			}
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqObjectStateChange(int64 SessionId, CMessage* Message)
{
	// 세션 얻기
	st_SESSION* Session = FindSession(SessionId);

	int64 AccountId;
	int64 PlayerDBId;
	int16 StateChange;

	if (Session)
	{
		do
		{
			// 로그인 중인지 확인
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				return;
			}

			*Message >> AccountId;

			// AccountId가 맞는지 확인
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				return;
			}

			*Message >> PlayerDBId;

			// 조종하고 있는 플레이어가 있는지 확인 
			if (Session->MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				return;
			}
			else
			{
				// 조종하고 있는 플레이어와 전송받은 PlayerId가 같은지 확인
				if (Session->MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					return;
				}
			}

			*Message >> StateChange;

			CMessage* ResObjectStatePakcet = nullptr;

			switch ((en_StateChange)StateChange)
			{
			case en_StateChange::MOVE_TO_STOP:
				if (Session->MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::MOVING
					|| Session->MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::IDLE)
				{
					Session->MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

					ResObjectStatePakcet = MakePacketResObjectState(Session->MyPlayer->_GameObjectInfo.ObjectId, Session->MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, Session->MyPlayer->_GameObjectInfo.ObjectType, Session->MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
					SendPacketAroundSector(Session, ResObjectStatePakcet, true);
					ResObjectStatePakcet->Free();
				}
				else
				{
					Disconnect(Session->SessionId);
					goto error;
				}
				break;
			case en_StateChange::SPELL_TO_IDLE:
				if (Session->MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::ATTACK
					|| Session->MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::SPELL)
				{
					Session->MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

					ResObjectStatePakcet = MakePacketResObjectState(Session->MyPlayer->_GameObjectInfo.ObjectId, Session->MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, Session->MyPlayer->_GameObjectInfo.ObjectType, Session->MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
					SendPacketAroundSector(Session, ResObjectStatePakcet, true);
					ResObjectStatePakcet->Free();
				}
				else
				{
					Disconnect(Session->SessionId);
					goto error;
				}
				break;
			default:
				break;
			}
		} while (0);
	error:
		ReturnSession(Session);
	}
}

// int32 ObjectId
// string Message
void CGameServer::PacketProcReqChattingMessage(int64 SessionId, CMessage* Message)
{
	// 세션 얻기
	st_SESSION* Session = FindSession(SessionId);

	int64 AccountId;
	int32 PlayerDBId;

	if (Session)
	{
		// 로그인 중인지 확인
		if (!Session->IsLogin)
		{
			Disconnect(Session->SessionId);
			return;
		}

		*Message >> AccountId;

		// AccountId가 맞는지 확인
		if (Session->AccountId != AccountId)
		{
			Disconnect(Session->SessionId);
			return;
		}

		*Message >> PlayerDBId;

		// 조종하고 있는 플레이어가 있는지 확인 
		if (Session->MyPlayer == nullptr)
		{
			Disconnect(Session->SessionId);
			return;
		}
		else
		{
			// 조종하고 있는 플레이어와 전송받은 PlayerId가 같은지 확인
			if (Session->MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
			{
				Disconnect(Session->SessionId);
				return;
			}
		}

		// 채팅 메세지 길이 
		int8 ChattingMessageLen;
		*Message >> ChattingMessageLen;

		// 채팅 메세지 얻어오기
		wstring ChattingMessage;
		Message->GetData(ChattingMessage, ChattingMessageLen);

		// 채널 찾고
		CChannel* Channel = G_ChannelManager->Find(1);
		// 주위 플레이어 반환
		vector<CPlayer*> Players = Channel->GetAroundPlayer(Session->MyPlayer, 10, false);

		// 주위 플레이어에게 채팅 메세지 전송
		for (CPlayer* Player : Players)
		{
			CMessage* ResChattingMessage = MakePacketResChattingMessage(PlayerDBId, en_MessageType::CHATTING, st_Color::White(), ChattingMessage);
			SendPacket(Player->_SessionId, ResChattingMessage);
			ResChattingMessage->Free();
		}
	}

	// 세션 반납
	ReturnSession(Session);
}

void CGameServer::PacketProcReqItemToInventory(int64 SessionId, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionId);

	do
	{
		if (Session)
		{
			InterlockedIncrement64(&Session->IOBlock->IOCount);

			int64 AccountId;
			int64 ItemId;

			// 로그인 중인지 확인
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> AccountId;

			// AccountId 확인
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			// ItemId와 ObjectType을 읽는다.
			*Message >> ItemId;

			int8 ObjectType;

			*Message >> ObjectType;

			// 아이템이 ObjectManager에 있는지 확인한다.
			CItem* Item = (CItem*)(G_ObjectManager->Find(ItemId, (en_GameObjectType)ObjectType));
			if (Item != nullptr)
			{
				int64 TargetObjectId;
				*Message >> TargetObjectId;

				int8 TargetObjectType;
				*Message >> TargetObjectType;

				CPlayer* TargetPlayer = (CPlayer*)(G_ObjectManager->Find(TargetObjectId, (en_GameObjectType)TargetObjectType));
				if (TargetPlayer == nullptr)
				{
					break;
				}

				bool IsExistItem = false;

				// 아이템 정보가 코인이라면
				if (Item->_ItemInfo.ItemType == en_ItemType::ITEM_TYPE_BRONZE_COIN
					|| Item->_ItemInfo.ItemType == en_ItemType::ITEM_TYPE_SLIVER_COIN
					|| Item->_ItemInfo.ItemType == en_ItemType::ITEM_TYPE_GOLD_COIN)
				{
					// 코인 저장
					TargetPlayer->_Inventory.AddCoin(Item);

					st_Job* DBGoldSaveJob = _JobMemoryPool->Alloc();
					DBGoldSaveJob->Type = en_MESSAGE_TYPE::DATA_BASE_GOLD_SAVE;
					DBGoldSaveJob->SessionId = TargetPlayer->_SessionId;

					CMessage* DBGoldSaveMessage = CMessage::Alloc();
					DBGoldSaveMessage->Clear();

					*DBGoldSaveMessage << TargetPlayer->_AccountId;
					*DBGoldSaveMessage << TargetPlayer->_GameObjectInfo.ObjectId;
					*DBGoldSaveMessage << ItemId;
					*DBGoldSaveMessage << TargetPlayer->_Inventory._GoldCoinCount;
					*DBGoldSaveMessage << TargetPlayer->_Inventory._SliverCoinCount;
					*DBGoldSaveMessage << TargetPlayer->_Inventory._BronzeCoinCount;
					*DBGoldSaveMessage << Item->_ItemInfo.ItemCount;
					*DBGoldSaveMessage << (int16)Item->_ItemInfo.ItemType;

					DBGoldSaveJob->Message = DBGoldSaveMessage;

					_GameServerDataBaseThreadMessageQue.Enqueue(DBGoldSaveJob);
					SetEvent(_DataBaseWakeEvent);
				}
				else
				{
					// 그 외 아이템이라면 
					int8 SlotIndex = 0;
					int16 ItemCount = 0;

					// 아이템이 이미 존재 하는지 확인한다.
					// 존재 하면 개수를 1 증가시킨다.
					IsExistItem = TargetPlayer->_Inventory.IsExistItem(Item->_ItemInfo.ItemType, &ItemCount, &SlotIndex);

					// 존재 하지 않을 경우
					if (IsExistItem == false)
					{
						if (TargetPlayer->_Inventory.GetEmptySlot(&SlotIndex))
						{
							Item->_ItemInfo.SlotIndex = SlotIndex;
							TargetPlayer->_Inventory.AddItem(SlotIndex, Item);

							ItemCount = 1;
						}
					}

					st_Job* DBInventorySaveJob = _JobMemoryPool->Alloc();
					DBInventorySaveJob->Type = en_MESSAGE_TYPE::DATA_BASE_ITEM_INVENTORY_SAVE;
					DBInventorySaveJob->SessionId = Session->SessionId;

					CMessage* DBSaveMessage = CMessage::Alloc();

					DBSaveMessage->Clear();

					// 중복 여부
					*DBSaveMessage << IsExistItem;
					// 타겟 ObjectId
					*DBSaveMessage << TargetPlayer->_GameObjectInfo.ObjectId;
					// 아이템 DBId
					*DBSaveMessage << Item->_ItemInfo.ItemDBId;
					// 아이템 개수
					*DBSaveMessage << ItemCount;
					// 아이템 추가한 개수
					*DBSaveMessage << (int16)1;
					// 아이템 슬롯 번호
					*DBSaveMessage << SlotIndex;
					// 아이템 착용 여부
					*DBSaveMessage << Item->_ItemInfo.IsEquipped;
					// 아이템 타입
					*DBSaveMessage << (int16)Item->_ItemInfo.ItemType;

					// AccoountId
					*DBSaveMessage << TargetPlayer->_AccountId;

					// 아이템 이름 길이
					int8 ItemNameLen = (int8)(Item->_ItemInfo.ItemName.length() * 2);
					*DBSaveMessage << ItemNameLen;
					// 아이템 이름
					DBSaveMessage->InsertData(Item->_ItemInfo.ItemName.c_str(), ItemNameLen);

					// 아이템 썸네일 경로 길이
					int8 ThumbnailImagePathLen = (int)(Item->_ItemInfo.ThumbnailImagePath.length() * 2);
					*DBSaveMessage << ThumbnailImagePathLen;
					// 아이템 썸네일 경로
					DBSaveMessage->InsertData(Item->_ItemInfo.ThumbnailImagePath.c_str(), ThumbnailImagePathLen);

					DBInventorySaveJob->Message = DBSaveMessage;

					_GameServerDataBaseThreadMessageQue.Enqueue(DBInventorySaveJob);
					SetEvent(_DataBaseWakeEvent);
				}

				// 아이템이 중복되어 있으면 앞서 아이템의 Count를 1 증가시켰을 테니 해당 아이템을 반납하고,
				// 그 외의 경우에는 반납하지 않는다.
				G_ObjectManager->Remove(Item, 1, IsExistItem);
			}
			else
			{

			}
		}
	} while (0);

	ReturnSession(Session);
}

// int64 AccountId
// int64 ObjectId
// int8 SwapIndexA
// int8 SwapIndexB
void CGameServer::PacketProcReqItemSwap(int64 SessionId, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionId);

	if (Session)
	{
		InterlockedIncrement64(&Session->IOBlock->IOCount);

		int64 AccountId;
		int64 PlayerDBId;

		do
		{
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> AccountId;

			// AccountId가 맞는지 확인
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> PlayerDBId;

			// 조종하고 있는 플레이어가 있는지 확인 
			if (Session->MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종하고 있는 플레이어와 전송받은 PlayerId가 같은지 확인
				if (Session->MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			int8 SwapIndexA;
			*Message >> SwapIndexA;

			int8 SwapIndexB;
			*Message >> SwapIndexB;

			st_Job* DBItemSwapJob = _JobMemoryPool->Alloc();
			DBItemSwapJob->Type = en_MESSAGE_TYPE::DATA_BASE_ITEM_SWAP;
			DBItemSwapJob->SessionId = Session->MyPlayer->_SessionId;

			CMessage* DBItemSwapMessage = CMessage::Alloc();
			DBItemSwapMessage->Clear();

			*DBItemSwapMessage << AccountId;
			*DBItemSwapMessage << PlayerDBId;
			*DBItemSwapMessage << SwapIndexA;
			*DBItemSwapMessage << SwapIndexB;

			DBItemSwapJob->Message = DBItemSwapMessage;

			_GameServerDataBaseThreadMessageQue.Enqueue(DBItemSwapJob);
			SetEvent(_DataBaseWakeEvent);

		} while (0);

		ReturnSession(Session);
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
	st_SESSION* Session = FindSession(SessionID);

	int64 AccountNo;
	WORD SectorX;
	WORD SectorY;

	if (Session)
	{
		//클라가 로그인 중인지 판단
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

		//섹터 X Y 좌표 범위 검사
		if (SectorX < 0 || SectorX >= SECTOR_X_MAX || SectorY < 0 || SectorY >= SECTOR_Y_MAX)
		{
			Disconnect(Session->SessionId);
			return;
		}

		//기존 섹터에서 클라 제거
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
//채팅 보내기 요청
//WORD	Type
//INT64 AccountNo
//WORD MessageLen
//WCHAR Message[MessageLen/2] // null 미포함
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
//하트 비트
//WORD Type
//---------------------------------------------------------------------------------
void CGameServer::PacketProcReqHeartBeat(int64 SessionID, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionID);

	Session->RecvPacketTime = timeGetTime();

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBAccountCheck(int64 SessionID, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionID);

	bool Status = LOGIN_SUCCESS;

	if (Session)
	{
		InterlockedDecrement64(&Session->IOBlock->IOCount);

		int64 ClientAccountId = Session->AccountId;
		int32 Token = Session->Token;

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
			Session->IsLogin = true;
			// 클라가 소유하고 있는 플레이어들을 DB로부터 긁어온다.
			CDBConnection* GameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

			SP::CDBGameServerPlayersGet ClientPlayersGet(*GameServerDBConnection);
			ClientPlayersGet.InAccountID(ClientAccountId);

			int64 PlayerId;
			WCHAR PlayerName[100];
			int8 PlayerIndex;
			int32 PlayerLevel;
			int32 PlayerCurrentHP;
			int32 PlayerMaxHP;
			int32 PlayerAttack;
			int16 PlayerCriticalPoint;
			float PlayerSpeed;
			int16 PlayerObjectType;

			ClientPlayersGet.OutPlayerDBID(PlayerId);
			ClientPlayersGet.OutPlayerName(PlayerName);
			ClientPlayersGet.OutPlayerIndex(PlayerIndex);
			ClientPlayersGet.OutLevel(PlayerLevel);
			ClientPlayersGet.OutCurrentHP(PlayerCurrentHP);
			ClientPlayersGet.OutMaxHP(PlayerMaxHP);
			ClientPlayersGet.OutAttack(PlayerAttack);
			ClientPlayersGet.OutCriticalPoint(PlayerCriticalPoint);
			ClientPlayersGet.OutSpeed(PlayerSpeed);
			ClientPlayersGet.OutPlayerObjectType(PlayerObjectType);

			ClientPlayersGet.Execute();

			int8 PlayerCount = 0;

			while (ClientPlayersGet.Fetch())
			{
				// 플레이어 정보 셋팅
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectId = PlayerId;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectName = PlayerName;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.Level = PlayerLevel;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.HP = PlayerCurrentHP;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.MaxHP = PlayerMaxHP;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.Attack = PlayerAttack;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.CriticalPoint = PlayerCriticalPoint;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.Speed = PlayerSpeed;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectType = (en_GameObjectType)PlayerObjectType;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.OwnerObjectId = 0;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)0;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.PlayerSlotIndex = PlayerIndex;
				Session->MyPlayers[PlayerIndex]->_SessionId = Session->SessionId;
				Session->MyPlayers[PlayerIndex]->_AccountId = Session->AccountId;

				PlayerCount++;
			}

			// 클라에게 로그인 응답 패킷 보냄
			CMessage* ResLoginMessage = MakePacketResLogin(Status, PlayerCount, (CGameObject**)Session->MyPlayers);
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
		// 클라 접속 끊겻을 경우
	}
}

void CGameServer::PacketProcReqDBCreateCharacterNameCheck(int64 SessionID, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionID);

	int64 PlayerDBId = 0;

	if (Session)
	{
		InterlockedDecrement64(&Session->IOBlock->IOCount);

		// 요청한 게임오브젝트 타입
		int16 ReqGameObjectType;
		*Message >> ReqGameObjectType;

		// 요청한 캐릭터 슬롯 인덱스
		int8 ReqCharacterCreateSlotIndex;
		*Message >> ReqCharacterCreateSlotIndex;

		Message->Free();

		// 요청한 클라의 생성할 캐릭터의 이름을 가져온다.
		wstring CreateCharacterName = Session->CreateCharacterName;

		// GameServerDB에 해당 캐릭터가 이미 있는지 확인한다.
		CDBConnection* FindCharacterNameGameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		// GameServerDB에 캐릭터 이름이 있는지 확인하는 프로시저 클래스
		SP::CDBGameServerCharacterNameGet CharacterNameGet(*FindCharacterNameGameServerDBConnection);
		CharacterNameGet.InCharacterName(CreateCharacterName);

		// DB 요청 실행
		CharacterNameGet.Execute();

		// 결과 받기
		bool CharacterNameFind = CharacterNameGet.Fetch();

		// DBConnection 반납
		G_DBConnectionPool->Push(en_DBConnect::GAME, FindCharacterNameGameServerDBConnection);

		// 캐릭터가 존재하지 않을 경우
		if (!CharacterNameFind)
		{
			// 요청한 ObjectType에 해당하는 캐릭터 정보 읽어옴
			auto FindStatus = G_Datamanager->_Status.find(ReqGameObjectType);
			st_StatusData NewCharacterStatus = *(*FindStatus).second;

			// 앞서 읽어온 캐릭터 정보를 토대로 DB에 저장
			// DBConnection Pool에서 DB연결을 위해서 하나를 꺼내온다.
			CDBConnection* NewCharacterPushDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			// GameServerDB에 새로운 캐릭터 저장하는 프로시저 클래스
			SP::CDBGameServerCreateCharacterPush NewCharacterPush(*NewCharacterPushDBConnection);
			NewCharacterPush.InAccountID(Session->AccountId);
			NewCharacterPush.InPlayerName(Session->CreateCharacterName);
			NewCharacterPush.InPlayerType(ReqGameObjectType);
			NewCharacterPush.InPlayerIndex(ReqCharacterCreateSlotIndex);
			NewCharacterPush.InLevel(NewCharacterStatus.Level);
			NewCharacterPush.InCurrentHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InMaxHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InAttack(NewCharacterStatus.Attack);
			NewCharacterPush.InCriticalPoint(NewCharacterStatus.CriticalPoint);
			NewCharacterPush.InSpeed(NewCharacterStatus.Speed);

			// DB 요청 실행
			NewCharacterPush.Execute();

			// DBConnection 반납
			G_DBConnectionPool->Push(en_DBConnect::GAME, NewCharacterPushDBConnection);

			// 앞서 저장한 캐릭터의 DBId를 AccountId를 이용해 얻어온다.
			CDBConnection* PlayerDBIDGetDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			// GameServerDB에 생성한 캐릭터의 PlayerDBId를 읽어오는 프로시저 클래스
			SP::CDBGameServerPlayerDBIDGet PlayerDBIDGet(*PlayerDBIDGetDBConnection);
			PlayerDBIDGet.InAccountID(Session->AccountId);
			PlayerDBIDGet.InPlayerSlotIndex(ReqCharacterCreateSlotIndex);

			PlayerDBIDGet.OutPlayerDBID(PlayerDBId);

			// DB 요청 실행
			PlayerDBIDGet.Execute();

			PlayerDBIDGet.Fetch();

			// DB에서 읽어온 DBId를 저장
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectId = PlayerDBId;
			// 캐릭터의 이름도 저장
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectName = Session->CreateCharacterName;

			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectStatInfo.Level = NewCharacterStatus.Level;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectStatInfo.Attack = NewCharacterStatus.Attack;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectStatInfo.CriticalPoint = NewCharacterStatus.CriticalPoint;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectType = (en_GameObjectType)ReqGameObjectType;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.OwnerObjectId = -1;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)0;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.PlayerSlotIndex = ReqCharacterCreateSlotIndex; // 캐릭터가 속한 슬롯
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_SessionId = Session->SessionId;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_AccountId = Session->AccountId;
			
			G_DBConnectionPool->Push(en_DBConnect::GAME, PlayerDBIDGetDBConnection);

			// Gold Table 생성
			CDBConnection* DBGoldTableCreateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerGoldTableCreatePush GoldTableCreate(*DBGoldTableCreateConnection);
			GoldTableCreate.InAccountDBId(Session->MyPlayers[ReqCharacterCreateSlotIndex]->_AccountId);
			GoldTableCreate.InPlayerDBId(Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectId);

			GoldTableCreate.Execute();

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBGoldTableCreateConnection);
		
			// 캐릭터 인벤토리 생성
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_Inventory.Init();
			
			// DB에 인벤토리 생성
			for (int8 SlotIndex = 0; SlotIndex < (int8)en_Inventory::INVENTORY_SIZE; SlotIndex++)
			{
				CDBConnection* DBItemToInventoryConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerItemCreateToInventory ItemToInventory(*DBItemToInventoryConnection);
				st_ItemInfo NewItem;
				NewItem.ItemDBId = 0;
				NewItem.ItemType = en_ItemType::ITEM_TYPE_NONE;
				NewItem.ItemName = L"";
				NewItem.ItemCount = 0;
				NewItem.SlotIndex = SlotIndex;
				NewItem.IsEquipped = false;
				NewItem.ThumbnailImagePath = L"";				

				int16 ItemType = (int16)NewItem.ItemType;

				ItemToInventory.InItemType(ItemType);
				ItemToInventory.InItemName(NewItem.ItemName);
				ItemToInventory.InItemCount(NewItem.ItemCount);
				ItemToInventory.InSlotIndex(SlotIndex);
				ItemToInventory.InIsEquipped(NewItem.IsEquipped);
				ItemToInventory.InThumbnailImagePath(NewItem.ThumbnailImagePath);
				ItemToInventory.InOwnerAccountId(Session->AccountId);
				ItemToInventory.InOwnerPlayerId(PlayerDBId);

				ItemToInventory.Execute();
			}			
		}
		else
		{
			// 캐릭터가 이미 DB에 있는 경우
			PlayerDBId = Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectId;
		}

		// 캐릭터 생성 응답 보냄
		CMessage* ResCreateCharacterMessage = MakePacketResCreateCharacter(!CharacterNameFind, Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo);
		SendPacket(Session->SessionId, ResCreateCharacterMessage);
		ResCreateCharacterMessage->Free();
	}
	else
	{

	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBItemCreate(CMessage* Message)
{
	int64 KillerId;
	*Message >> KillerId;

	int16 KillerObjectType;
	*Message >> KillerObjectType;

	int32 SpawnPositionX;
	*Message >> SpawnPositionX;

	int32 SpawnPositionY;
	*Message >> SpawnPositionY;

	int32 MonsterDataType;
	*Message >> MonsterDataType;

	Message->Free();

	bool Find = false;

	auto FindMonsterDropItem = G_Datamanager->_Monsters.find(MonsterDataType);
	st_MonsterData MonsterData = *(*FindMonsterDropItem).second;

	random_device RD;	
	mt19937 Gen(RD());	
	uniform_real_distribution<float> RandomDropPoint(0, 1); // 0.0 ~ 1.0	
	float RandomPoint = 100 * RandomDropPoint(Gen);

	int32 Sum = 0;

	st_ItemData DropItemData;
	for (st_DropData DropItem : MonsterData._DropItems)
	{
		Sum += DropItem.Probability;

		if (Sum >= RandomPoint)
		{
			Find = true;
			// 드랍 확정 되면 해당 아이템 읽어오기
			auto FindDropItemInfo = G_Datamanager->_Items.find(DropItem.ItemDataSheetId);
			if (FindDropItemInfo == G_Datamanager->_Items.end())
			{
				CRASH("DropItemInfo를 찾지 못함");
			}

			DropItemData = *(*FindDropItemInfo).second;

			uniform_int_distribution<int> RandomDropItemCount(DropItem.MinCount, DropItem.MaxCount);
			DropItemData.ItemCount = RandomDropItemCount(Gen);
			DropItemData.DataSheetId = DropItem.ItemDataSheetId;
			break;
		}
	}

	if (Find == true)
	{
		st_ItemInfo NewItemInfo;

		NewItemInfo.ItemType = DropItemData.ItemType;
		NewItemInfo.ItemName = (LPWSTR)CA2W(DropItemData.ItemName.c_str());
		NewItemInfo.ItemCount = DropItemData.ItemCount;
		NewItemInfo.IsEquipped = false;
		NewItemInfo.ThumbnailImagePath = (LPWSTR)CA2W(DropItemData.ThumbnailImagePath.c_str());

		int64 ItemDBId = 0;

		// 아이템 DB에 생성
		CDBConnection* DBCreateItemConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerCreateItem CreateItem(*DBCreateItemConnection);

		int16 ItemType = (int16)NewItemInfo.ItemType;
		bool ItemUse = false;
		CreateItem.InItemUse(ItemUse);
		CreateItem.InItemType(ItemType);
		CreateItem.InItemName(NewItemInfo.ItemName);
		CreateItem.InItemCount(NewItemInfo.ItemCount);
		CreateItem.InIsEquipped(NewItemInfo.IsEquipped);
		CreateItem.InThumbnailImagePath(NewItemInfo.ThumbnailImagePath);

		bool CreateItemSuccess = CreateItem.Execute();

		if (CreateItemSuccess == true)
		{
			G_DBConnectionPool->Push(en_DBConnect::GAME, DBCreateItemConnection);

			CDBConnection* DBItemDBIdGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerItemDBIdGet ItemDBIdGet(*DBItemDBIdGetConnection);
			ItemDBIdGet.OutItemDBId(ItemDBId);

			bool ItemDBIdGetSuccess = ItemDBIdGet.Execute();

			if (ItemDBIdGetSuccess == true)
			{
				ItemDBIdGet.Fetch();

				G_Logger->WriteStdOut(en_Color::RED, L"ItemDBId %d\n", ItemDBId);
				NewItemInfo.ItemDBId = ItemDBId;

				// 아이템 생성 후 스폰
				en_GameObjectType GameObjectType;
				switch (NewItemInfo.ItemType)
				{
				case en_ItemType::ITEM_TYPE_SLIMEGEL:
					GameObjectType = en_GameObjectType::SLIME_GEL;
					break;
				case en_ItemType::ITEM_TYPE_LEATHER:
					GameObjectType = en_GameObjectType::LEATHER;
					break;
				case en_ItemType::ITEM_TYPE_BRONZE_COIN:
					GameObjectType = en_GameObjectType::BRONZE_COIN;
					break;
				default:
					break;
				}

				// 아이템이 스폰 될 위치 지정
				st_Vector2Int SpawnPosition(SpawnPositionX, SpawnPositionY);

				// 생성할 아이템 정보 셋팅
				CItem* NewItem = (CItem*)G_ObjectManager->ObjectCreate(GameObjectType);
				NewItem->_ItemInfo = NewItemInfo;
				NewItem->_GameObjectInfo.ObjectType = GameObjectType;
				NewItem->_GameObjectInfo.ObjectId = NewItem->_ItemInfo.ItemDBId;
				NewItem->_GameObjectInfo.ObjectName = NewItemInfo.ItemName;
				NewItem->_GameObjectInfo.OwnerObjectId = KillerId;
				NewItem->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)KillerObjectType;
				NewItem->_SpawnPosition = SpawnPosition;
				NewItem->_ItemInfo.SlotIndex = -1;

				G_ObjectManager->Add(NewItem, 1);
			}

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBItemDBIdGetConnection);
		}
	}
}

//(WORD)ItemType
//int32 Count
//int32 SlotNumber;
//int64 OwnerAccountId;
//bool IsEquipped

void CGameServer::PacketProcReqDBItemToInventorySave(int64 SessionId, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionId);

	if (Session)
	{
		InterlockedDecrement64(&Session->IOBlock->IOCount);

		bool IsExistItem;
		*Message >> IsExistItem;

		int64 TargetObjectId;
		*Message >> TargetObjectId;

		int64 ItemDBId;
		*Message >> ItemDBId;

		int16 ItemCount;
		*Message >> ItemCount;

		int16 ItemEach;
		*Message >> ItemEach;

		int8 SlotIndex;
		*Message >> SlotIndex;

		bool IsEquipped;
		*Message >> IsEquipped;

		int16 ItemType;
		*Message >> ItemType;

		int64 OwnerAccountId;
		*Message >> OwnerAccountId;

		st_ItemInfo ItemInfo;
		ItemInfo.ItemType = (en_ItemType)(ItemType);
		ItemInfo.ItemCount = ItemCount;
		ItemInfo.SlotIndex = SlotIndex;
		ItemInfo.ItemDBId = ItemDBId;
		ItemInfo.IsEquipped = IsEquipped;

		int8 ItemNameLen;
		*Message >> ItemNameLen;
		wstring ItemName;
		Message->GetData(ItemName, ItemNameLen);
		ItemInfo.ItemName = ItemName;

		int8 ThumbnailImagePathLen;
		*Message >> ThumbnailImagePathLen;
		wstring ThumbnailImagePath;
		Message->GetData(ThumbnailImagePath, ThumbnailImagePathLen);
		ItemInfo.ThumbnailImagePath = ThumbnailImagePath;

		Message->Free();

		CDBConnection* ItemToInventorySaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

		// 아이템 Count 갱신
		if (IsExistItem == true)
		{
			SP::CDBGameServerItemRefreshPush ItemRefreshPush(*ItemToInventorySaveDBConnection);
			ItemRefreshPush.InAccountDBId(OwnerAccountId);
			ItemRefreshPush.InPlayerDBId(TargetObjectId);
			ItemRefreshPush.InItemType(ItemType);
			ItemRefreshPush.InCount(ItemCount);
			ItemRefreshPush.InSlotIndex(SlotIndex);

			ItemRefreshPush.Execute();
		}
		else
		{
			// 새로운 아이템 생성 후 Inventory DB 넣기			
			SP::CDBGameServerItemToInventoryPush ItemToInventoryPush(*ItemToInventorySaveDBConnection);
			ItemToInventoryPush.InItemType(ItemType);
			ItemToInventoryPush.InItemName(ItemName);
			ItemToInventoryPush.InItemCount(ItemCount);
			ItemToInventoryPush.InSlotIndex(SlotIndex);
			ItemToInventoryPush.InIsEquipped(IsEquipped);
			ItemToInventoryPush.InThumbnailImagePath(ThumbnailImagePath);
			ItemToInventoryPush.InOwnerAccountId(OwnerAccountId);
			ItemToInventoryPush.InOwnerPlayerId(TargetObjectId);

			ItemToInventoryPush.Execute();
		}

		G_DBConnectionPool->Push(en_DBConnect::GAME, ItemToInventorySaveDBConnection);

		// 클라 인벤토리에서 해당 아이템을 저장
		CMessage* ResItemToInventoryPacket = MakePacketResItemToInventory(TargetObjectId, ItemInfo, ItemEach);
		SendPacket(Session->SessionId, ResItemToInventoryPacket);
		ResItemToInventoryPacket->Free();

		vector<int64> DeSpawnItem;
		DeSpawnItem.push_back(ItemDBId);

		// 클라에게 해당 아이템 삭제하라고 알려줌
		CMessage* ResItemDeSpawnPacket = MakePacketResDeSpawn(1, DeSpawnItem);
		SendPacketAroundSector(Session, ResItemDeSpawnPacket, true);
		ResItemDeSpawnPacket->Free();

		bool ItemUse = true;
		// 아이템 DB에서 Inventory에 저장한 아이템 삭제	( 실제로 삭제하지는 않고 ItemUse를 1로 바꾼다 )	
		CDBConnection* ItemDeleteDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerItemDelete ItemDelete(*ItemDeleteDBConnection);
		ItemDelete.InItemDBId(ItemDBId);
		ItemDelete.InItemUse(ItemUse);

		ItemDelete.Execute();

		G_DBConnectionPool->Push(en_DBConnect::GAME, ItemDeleteDBConnection);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBItemSwap(int64 SessionId, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionId);

	if (Session)
	{
		InterlockedDecrement64(&Session->IOBlock->IOCount);

		int64 AccountId;
		*Message >> AccountId;

		int64 PlayerDBId;
		*Message >> PlayerDBId;

		int8 SwapAIndex;
		*Message >> SwapAIndex;

		int8 SwapBIndex;
		*Message >> SwapBIndex;

		Message->Free();

		// 스왑 요청한 A 아이템이 Inventory에 있는지 확인한다.
		CDBConnection* AItemCheckDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerItemCheck ADBItemCheck(*AItemCheckDBConnection);
		ADBItemCheck.InAccountDBId(AccountId);
		ADBItemCheck.InPlayerDBId(PlayerDBId);
		ADBItemCheck.InSlotIndex(SwapAIndex);

		int16 ADBItemType = -1;
		WCHAR AItemName[20] = { 0 };
		int16 AItemCount = -1;
		bool AItemEquipped = false;
		WCHAR AItemThumbnailImagePath[100] = { 0 };
		ADBItemCheck.OutItemType(ADBItemType);
		ADBItemCheck.OutItemName(AItemName);
		ADBItemCheck.OutItemCount(AItemCount);
		ADBItemCheck.OutItemIsEquipped(AItemEquipped);
		ADBItemCheck.OutItemThumbnailImagePath(AItemThumbnailImagePath);

		ADBItemCheck.Execute();

		ADBItemCheck.Fetch();

		// 스왑 요청할 A 아이템 정보 셋팅
		st_ItemInfo SwapAItemInfo;
		SwapAItemInfo.SlotIndex = SwapAIndex;
		SwapAItemInfo.ItemType = (en_ItemType)ADBItemType;
		SwapAItemInfo.ItemName = AItemName;
		SwapAItemInfo.ItemCount = AItemCount;
		SwapAItemInfo.IsEquipped = AItemEquipped;
		SwapAItemInfo.ThumbnailImagePath = AItemThumbnailImagePath;

		G_DBConnectionPool->Push(en_DBConnect::GAME, AItemCheckDBConnection);

		// 스왑 요청한 B 아이템이 Inventory에 있는지 확인한다.
		CDBConnection* BItemCheckDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerItemCheck DBItemCheck(*BItemCheckDBConnection);
		DBItemCheck.InAccountDBId(AccountId);
		DBItemCheck.InPlayerDBId(PlayerDBId);
		DBItemCheck.InSlotIndex(SwapBIndex);

		int16 BDBItemType = -1;
		WCHAR BItemName[20] = { 0 };
		int16 BItemCount = -1;
		bool BItemEquipped = false;		
		WCHAR BItemThumbnailImagePath[100] = { 0 };
		DBItemCheck.OutItemType(BDBItemType);
		DBItemCheck.OutItemName(BItemName);
		DBItemCheck.OutItemCount(BItemCount);
		DBItemCheck.OutItemIsEquipped(BItemEquipped);
		DBItemCheck.OutItemThumbnailImagePath(BItemThumbnailImagePath);

		DBItemCheck.Execute();

		DBItemCheck.Fetch();		

		// 스왑 요청할 B 아이템 정보 셋팅
		st_ItemInfo SwapBItemInfo;
		SwapBItemInfo.SlotIndex = SwapBIndex;
		SwapBItemInfo.ItemType = (en_ItemType)BDBItemType;
		SwapBItemInfo.ItemName = BItemName;
		SwapBItemInfo.ItemCount = BItemCount;
		SwapBItemInfo.IsEquipped = BItemEquipped;
		SwapBItemInfo.ThumbnailImagePath = BItemThumbnailImagePath;

		G_DBConnectionPool->Push(en_DBConnect::GAME, BItemCheckDBConnection);

		// 스왑 하기 위해서 임시 Temp 변수 생성
		st_ItemInfo Temp;
		Temp.ItemType = SwapAItemInfo.ItemType;
		Temp.ItemName = SwapAItemInfo.ItemName;
		Temp.ItemCount = SwapAItemInfo.ItemCount;
		Temp.IsEquipped = SwapAItemInfo.IsEquipped;
		Temp.ThumbnailImagePath = SwapAItemInfo.ThumbnailImagePath;

		// 데이터 스왑
		SwapAItemInfo.ItemDBId = 0;
		SwapAItemInfo.ItemType = SwapBItemInfo.ItemType;
		SwapAItemInfo.ItemName = SwapBItemInfo.ItemName;
		SwapAItemInfo.ItemCount = SwapBItemInfo.ItemCount;
		SwapAItemInfo.IsEquipped = SwapBItemInfo.IsEquipped;
		SwapAItemInfo.ThumbnailImagePath = SwapBItemInfo.ThumbnailImagePath;

		SwapBItemInfo.ItemDBId = 0;
		SwapBItemInfo.ItemType = Temp.ItemType;
		SwapBItemInfo.ItemName = Temp.ItemName;
		SwapBItemInfo.ItemCount = Temp.ItemCount;
		SwapBItemInfo.IsEquipped = Temp.IsEquipped;
		SwapBItemInfo.ThumbnailImagePath = Temp.ThumbnailImagePath;

		int16 AItemType = (int16)SwapAItemInfo.ItemType;
		int16 BItemType = (int16)SwapBItemInfo.ItemType;

		// ItemSwap 
		CDBConnection* ItemSwapConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerItemSwap ItemSwap(*ItemSwapConnection);
		ItemSwap.InAccountDBId(AccountId);
		ItemSwap.InPlayerDBId(PlayerDBId);

		ItemSwap.InAItemType(AItemType);
		ItemSwap.InAItemName(SwapAItemInfo.ItemName);
		ItemSwap.InAItemCount(SwapAItemInfo.ItemCount);
		ItemSwap.InAItemIsEquipped(SwapAItemInfo.IsEquipped);
		ItemSwap.InAItemThumbnailImagePath(SwapAItemInfo.ThumbnailImagePath);
		ItemSwap.InAItemSlotIndex(SwapAItemInfo.SlotIndex);

		ItemSwap.InBItemType(BItemType);
		ItemSwap.InBItemName(SwapBItemInfo.ItemName);
		ItemSwap.InBItemCount(SwapBItemInfo.ItemCount);
		ItemSwap.InBItemIsEquipped(SwapBItemInfo.IsEquipped);
		ItemSwap.InBItemThumbnailImagePath(SwapBItemInfo.ThumbnailImagePath);
		ItemSwap.InBItemSlotIndex(SwapBItemInfo.SlotIndex);

		// Item Swap 성공
		bool SwapSuccess = ItemSwap.Execute();
		if (SwapSuccess == true)
		{
			CPlayer* TargetPlayer = (CPlayer*)G_ObjectManager->Find(PlayerDBId, en_GameObjectType::MELEE_PLAYER);

			// Swap 요청한 플레이어의 인벤토리에서 아이템 Swap
			TargetPlayer->_Inventory.SwapItem(SwapAItemInfo, SwapBItemInfo);

			// Swap 요청 응답 보내기
			CMessage* ResItemSwapPacket = MakePacketResItemSwap(AccountId, PlayerDBId, SwapAItemInfo, SwapBItemInfo);
			SendPacket(Session->SessionId, ResItemSwapPacket);
			ResItemSwapPacket->Free();
		}

		G_DBConnectionPool->Push(en_DBConnect::GAME, ItemSwapConnection);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBGoldSave(int64 SessionId, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionId);

	if (Session)
	{
		InterlockedDecrement64(&Session->IOBlock->IOCount);

		int64 AccountId;
		*Message >> AccountId;

		int64 TargetId;
		*Message >> TargetId;

		int64 ItemDBId;
		*Message >> ItemDBId;

		int64 GoldCoinCount;
		*Message >> GoldCoinCount;

		int8 SliverCoinCount;
		*Message >> SliverCoinCount;

		int8 BronzeCoinCount;
		*Message >> BronzeCoinCount;

		int16 ItemCount;
		*Message >> ItemCount;

		int16 ItemType;
		*Message >> ItemType;

		Message->Free();
		
		CDBConnection* GoldSaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerGoldPush GoldSavePush(*GoldSaveDBConnection);
		GoldSavePush.InAccoountId(AccountId);
		GoldSavePush.InPlayerDBId(TargetId);
		GoldSavePush.InGoldCoin(GoldCoinCount);
		GoldSavePush.InSliverCoin(SliverCoinCount);
		GoldSavePush.InBronzeCoin(BronzeCoinCount);

		GoldSavePush.Execute();

		G_DBConnectionPool->Push(en_DBConnect::GAME, GoldSaveDBConnection);

		// 클라에게 돈 저장 결과 알려줌
		CMessage* ResGoldSaveMeesage = MakePacketGoldSave(AccountId, TargetId, GoldCoinCount, SliverCoinCount, BronzeCoinCount, ItemCount, ItemType);
		SendPacket(Session->SessionId, ResGoldSaveMeesage);
		ResGoldSaveMeesage->Free();

		// 돈 오브젝트 디스폰
		vector<int64> DeSpawnItem;
		DeSpawnItem.push_back(ItemDBId);

		CMessage* ResItemDeSpawnPacket = MakePacketResDeSpawn(1, DeSpawnItem);
		SendPacketAroundSector(Session, ResItemDeSpawnPacket, true);
		ResItemDeSpawnPacket->Free();

		bool ItemUse = true;
		// 아이템 DB에서 Inventory에 저장한 아이템 삭제 ( 아이템과 마찬가지로 ItemUse를 1로 바꾼다 )
		CDBConnection* GoldItemDeleteDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerItemDelete GoldItemDelete(*GoldItemDeleteDBConnection);
		GoldItemDelete.InItemDBId(ItemDBId);
		GoldItemDelete.InItemUse(ItemUse);

		GoldItemDelete.Execute();

		G_DBConnectionPool->Push(en_DBConnect::GAME, GoldItemDeleteDBConnection);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBCharacterInfoSend(int64 SessionId, CMessage* Message)
{
	st_SESSION* Session = FindSession(SessionId);

	if (Session)
	{
		do
		{
			InterlockedDecrement64(&Session->IOBlock->IOCount);

			int64 GoldCoin = 0;
			int8 SliverCoin = 0;
			int8 BronzeCoin = 0;

			// 캐릭터가 소유하고 있었던 골드 정보를 GoldTable에서 읽어온다.
			CDBConnection* DBCharacterGoldGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerGoldGet CharacterGoldGet(*DBCharacterGoldGetConnection);
			CharacterGoldGet.InAccountDBId(Session->MyPlayer->_AccountId);
			CharacterGoldGet.InPlayerDBId(Session->MyPlayer->_GameObjectInfo.ObjectId);

			CharacterGoldGet.OutGoldCoin(GoldCoin);
			CharacterGoldGet.OutSliverCoin(SliverCoin);
			CharacterGoldGet.OutBronzeCoin(BronzeCoin);			

			if (CharacterGoldGet.Execute() && CharacterGoldGet.Fetch())
			{
				// DB에서 읽어온 Gold를 Inventory에 저장한다.
				Session->MyPlayer->_Inventory._GoldCoinCount = GoldCoin;
				Session->MyPlayer->_Inventory._SliverCoinCount = SliverCoin;
				Session->MyPlayer->_Inventory._BronzeCoinCount = BronzeCoin;

				// DBConnection 반납하고
				G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterGoldGetConnection);

				// 클라에게 골드 정보를 보내준다.
				CMessage* ResGoldSaveMeesage = MakePacketGoldSave(Session->MyPlayer->_AccountId, Session->MyPlayer->_GameObjectInfo.ObjectId, GoldCoin, SliverCoin, BronzeCoin, 0, 0, false);
				SendPacket(Session->SessionId, ResGoldSaveMeesage);
				ResGoldSaveMeesage->Free();
			}			

			Session->MyPlayer->_Inventory.Init();

			// 캐릭터가 소유하고 있었던 Item 정보를 InventoryTable에서 읽어온다.
			CDBConnection* DBCharacterInventoryItemGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerInventoryItemGet CharacterInventoryItemGet(*DBCharacterInventoryItemGetConnection);
			CharacterInventoryItemGet.InAccountDBId(Session->MyPlayer->_AccountId);
			CharacterInventoryItemGet.InPlayerDBId(Session->MyPlayer->_GameObjectInfo.ObjectId);
			
			int16 ItemType;
			WCHAR ItemName[20] = { 0 };
			int16 ItemCount;
			int8 SlotIndex;
			bool IsEquipped;
			WCHAR ItemThumbnailImagePath[100] = { 0 };
			
			CharacterInventoryItemGet.OutItemType(ItemType);
			CharacterInventoryItemGet.OutItemName(ItemName);
			CharacterInventoryItemGet.OutItemCount(ItemCount);
			CharacterInventoryItemGet.OutSlotIndex(SlotIndex);
			CharacterInventoryItemGet.OutIsEquipped(IsEquipped);
			CharacterInventoryItemGet.OutItemThumbnailImagePath(ItemThumbnailImagePath);

			CharacterInventoryItemGet.Execute();

			while (CharacterInventoryItemGet.Fetch())
			{
				// 읽어온 데이터를 이용해서 ItemInfo를 조립
				st_ItemInfo ItemInfo;
				ItemInfo.ItemDBId = 0;
				ItemInfo.ItemType = (en_ItemType)ItemType;
				ItemInfo.ItemName = ItemName;
				ItemInfo.ItemCount = ItemCount;
				ItemInfo.SlotIndex = SlotIndex;
				ItemInfo.IsEquipped = IsEquipped;
				ItemInfo.ThumbnailImagePath = ItemThumbnailImagePath;

				// ItemType에 따라 아이템을 생성하고
				CItem* NewItem = nullptr;
				switch (ItemInfo.ItemType)
				{
				case en_ItemType::ITEM_TYPE_NONE:
					break;
				case en_ItemType::ITEM_TYPE_SLIMEGEL:
					NewItem = (CItem*)(G_ObjectManager->ObjectCreate(en_GameObjectType::SLIME_GEL));
					break;
				case en_ItemType::ITEM_TYPE_LEATHER:
					NewItem = (CItem*)(G_ObjectManager->ObjectCreate(en_GameObjectType::LEATHER));
					break;
				default:
					CRASH("의도치 않은 ItemType");
					break;
				}

				if (ItemInfo.ItemType != en_ItemType::ITEM_TYPE_NONE)
				{
					// 위에서 조립한 ItemInfo를 셋팅한다.
					NewItem->_ItemInfo = ItemInfo;
					// 인벤토리에 아이템을 추가하고
					Session->MyPlayer->_Inventory.AddItem(NewItem->_ItemInfo.SlotIndex, NewItem);

					// 클라에게 아이템 정보를 보내준다.
					CMessage* ResItemToInventoryPacket = MakePacketResItemToInventory(Session->MyPlayer->_GameObjectInfo.ObjectId, ItemInfo, ItemInfo.ItemCount, false);
					SendPacket(Session->SessionId, ResItemToInventoryPacket);
					ResItemToInventoryPacket->Free();
				}				
			}
		} while (0);

		ReturnSession(Session);
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
CMessage* CGameServer::MakePacketResLogin(bool Status, int8 PlayerCount, CGameObject** MyPlayersInfo)
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

	if (PlayerCount > 0)
	{
		for (int32 i = 0; i < PlayerCount; i++)
		{
			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo.ObjectId;

			int8 ObjectNameLen = (int8)(MyPlayersInfo[i]->_GameObjectInfo.ObjectName.length() * 2);
			*LoginMessage << ObjectNameLen;
			LoginMessage->InsertData(MyPlayersInfo[i]->_GameObjectInfo.ObjectName.c_str(), ObjectNameLen);

			*LoginMessage << (int8)(MyPlayersInfo[i]->_GameObjectInfo.ObjectPositionInfo.State);
			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo.ObjectPositionInfo.PositionX;
			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo.ObjectPositionInfo.PositionY;
			*LoginMessage << (int8)(MyPlayersInfo[i]->_GameObjectInfo.ObjectPositionInfo.MoveDir);

			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo.ObjectStatInfo.Level;
			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo.ObjectStatInfo.HP;
			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo.ObjectStatInfo.MaxHP;
			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo.ObjectStatInfo.Attack;
			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo.ObjectStatInfo.CriticalPoint;
			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo.ObjectStatInfo.Speed;
			*LoginMessage << (int16)(MyPlayersInfo[i]->_GameObjectInfo.ObjectType);
			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo.OwnerObjectId;
			*LoginMessage << (int16)MyPlayersInfo[i]->_GameObjectInfo.OwnerObjectType;
			*LoginMessage << (int8)(MyPlayersInfo[i]->_GameObjectInfo.PlayerSlotIndex);
		}
	}

	return LoginMessage;
}

// int32 PlayerDBId
// bool IsSuccess
// wstring PlayerName
CMessage* CGameServer::MakePacketResCreateCharacter(bool IsSuccess, st_GameObjectInfo CreateCharacterObjectInfo)
{
	CMessage* ResCreateCharacter = CMessage::Alloc();
	if (ResCreateCharacter == nullptr)
	{
		return nullptr;
	}

	ResCreateCharacter->Clear();

	*ResCreateCharacter << (WORD)en_PACKET_S2C_GAME_CREATE_CHARACTER;
	*ResCreateCharacter << IsSuccess;

	*ResCreateCharacter << CreateCharacterObjectInfo.ObjectId;

	int8 CreateCharacterObjectNameLen = (int8)(CreateCharacterObjectInfo.ObjectName.length() * 2);
	*ResCreateCharacter << CreateCharacterObjectNameLen;
	ResCreateCharacter->InsertData(CreateCharacterObjectInfo.ObjectName.c_str(), CreateCharacterObjectNameLen);

	// st_PositionInfo
	*ResCreateCharacter << (int8)(CreateCharacterObjectInfo.ObjectPositionInfo.State);
	*ResCreateCharacter << CreateCharacterObjectInfo.ObjectPositionInfo.PositionX;
	*ResCreateCharacter << CreateCharacterObjectInfo.ObjectPositionInfo.PositionY;
	*ResCreateCharacter << (int8)(CreateCharacterObjectInfo.ObjectPositionInfo.MoveDir);

	// st_StatInfo
	*ResCreateCharacter << CreateCharacterObjectInfo.ObjectStatInfo.Level;
	*ResCreateCharacter << CreateCharacterObjectInfo.ObjectStatInfo.HP;
	*ResCreateCharacter << CreateCharacterObjectInfo.ObjectStatInfo.MaxHP;
	*ResCreateCharacter << CreateCharacterObjectInfo.ObjectStatInfo.Attack;
	*ResCreateCharacter << CreateCharacterObjectInfo.ObjectStatInfo.CriticalPoint;
	*ResCreateCharacter << CreateCharacterObjectInfo.ObjectStatInfo.Speed;

	*ResCreateCharacter << (int16)(CreateCharacterObjectInfo.ObjectType);
	*ResCreateCharacter << CreateCharacterObjectInfo.OwnerObjectId;
	*ResCreateCharacter << (int16)CreateCharacterObjectInfo.OwnerObjectType;
	*ResCreateCharacter << (int8)(CreateCharacterObjectInfo.PlayerSlotIndex);

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
	*ResEnterGamePacket << (int16)ObjectInfo.ObjectType;

	*ResEnterGamePacket << ObjectInfo.OwnerObjectId;

	*ResEnterGamePacket << (int16)ObjectInfo.OwnerObjectType;

	*ResEnterGamePacket << (int8)ObjectInfo.PlayerSlotIndex;

	return ResEnterGamePacket;
}

// int64 AccountId
// int32 PlayerDBId
// st_GameObjectInfo ObjectInfo
CMessage* CGameServer::MakePacketMousePositionObjectInfo(int64 AccountId, st_GameObjectInfo ObjectInfo)
{
	CMessage* ResMousePositionObjectInfoPacket = CMessage::Alloc();
	if (ResMousePositionObjectInfoPacket == nullptr)
	{
		return nullptr;
	}

	ResMousePositionObjectInfoPacket->Clear();

	*ResMousePositionObjectInfoPacket << (WORD)en_PACKET_S2C_MOUSE_POSITION_OBJECT_INFO;
	*ResMousePositionObjectInfoPacket << AccountId;

	// ObjectId
	*ResMousePositionObjectInfoPacket << ObjectInfo.ObjectId;

	// EnterPlayerName
	int8 ObjectNameLen = (int8)(ObjectInfo.ObjectName.length() * 2);
	*ResMousePositionObjectInfoPacket << ObjectNameLen;
	ResMousePositionObjectInfoPacket->InsertData(ObjectInfo.ObjectName.c_str(), ObjectNameLen);

	// st_PositionInfo
	*ResMousePositionObjectInfoPacket << (int8)ObjectInfo.ObjectPositionInfo.State;
	*ResMousePositionObjectInfoPacket << ObjectInfo.ObjectPositionInfo.PositionX;
	*ResMousePositionObjectInfoPacket << ObjectInfo.ObjectPositionInfo.PositionY;
	*ResMousePositionObjectInfoPacket << (int8)ObjectInfo.ObjectPositionInfo.MoveDir;

	// st_StatInfo
	*ResMousePositionObjectInfoPacket << ObjectInfo.ObjectStatInfo.Level;
	*ResMousePositionObjectInfoPacket << ObjectInfo.ObjectStatInfo.HP;
	*ResMousePositionObjectInfoPacket << ObjectInfo.ObjectStatInfo.MaxHP;
	*ResMousePositionObjectInfoPacket << ObjectInfo.ObjectStatInfo.Attack;
	*ResMousePositionObjectInfoPacket << ObjectInfo.ObjectStatInfo.CriticalPoint;
	*ResMousePositionObjectInfoPacket << ObjectInfo.ObjectStatInfo.Speed;

	// ObjectType
	*ResMousePositionObjectInfoPacket << (int16)ObjectInfo.ObjectType;

	// OwnerObjectId, OwnerObjectType
	*ResMousePositionObjectInfoPacket << ObjectInfo.OwnerObjectId;
	*ResMousePositionObjectInfoPacket << (int16)ObjectInfo.OwnerObjectType;

	*ResMousePositionObjectInfoPacket << (int8)ObjectInfo.PlayerSlotIndex;

	return ResMousePositionObjectInfoPacket;
}

CMessage* CGameServer::MakePacketGoldSave(int64 AccountId, int64 ObjectId, int64 GoldCount, int8 SliverCount, int8 BronzeCount, int16 ItemCount, int16 ItemType, bool ItemGainPrint)
{
	CMessage* ResGoldSaveMessage = CMessage::Alloc();
	if (ResGoldSaveMessage == nullptr)
	{
		return nullptr;
	}

	ResGoldSaveMessage->Clear();

	*ResGoldSaveMessage << (WORD)en_PACKET_S2C_GOLD_SAVE;
	*ResGoldSaveMessage << AccountId;
	*ResGoldSaveMessage << ObjectId;
	*ResGoldSaveMessage << GoldCount;
	*ResGoldSaveMessage << SliverCount;
	*ResGoldSaveMessage << BronzeCount;
	*ResGoldSaveMessage << ItemCount;
	*ResGoldSaveMessage << ItemType;
	*ResGoldSaveMessage << ItemGainPrint;

	return ResGoldSaveMessage;
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

CMessage* CGameServer::MakePacketResItemSwap(int64 AccountId, int64 ObjectId, st_ItemInfo SwapAItemInfo, st_ItemInfo SwapBItemInfo)
{
	CMessage* ResItemSwapMessage = CMessage::Alloc();
	if (ResItemSwapMessage == nullptr)
	{
		return nullptr;
	}

	ResItemSwapMessage->Clear();

	*ResItemSwapMessage << (WORD)en_PACKET_S2C_ITEM_SWAP;
	*ResItemSwapMessage << AccountId;
	*ResItemSwapMessage << ObjectId;

	// AItemInfo	
	*ResItemSwapMessage << SwapAItemInfo.ItemDBId;
	*ResItemSwapMessage << SwapAItemInfo.ItemCount;
	*ResItemSwapMessage << SwapAItemInfo.SlotIndex;
	*ResItemSwapMessage << SwapAItemInfo.IsEquipped;
	*ResItemSwapMessage << (int16)SwapAItemInfo.ItemType;

	// AItem 이름
	int8 AItemNameLen = (int8)(SwapAItemInfo.ItemName.length() * 2);
	*ResItemSwapMessage << AItemNameLen;
	ResItemSwapMessage->InsertData(SwapAItemInfo.ItemName.c_str(), AItemNameLen);

	// AItem 썸네일 이미지 경로 
	int8 AItemThumbnailImagePathLen = (int8)(SwapAItemInfo.ThumbnailImagePath.length() * 2);
	*ResItemSwapMessage << AItemThumbnailImagePathLen;
	ResItemSwapMessage->InsertData(SwapAItemInfo.ThumbnailImagePath.c_str(), AItemThumbnailImagePathLen);

	// BItemInfo	
	*ResItemSwapMessage << SwapBItemInfo.ItemDBId;
	*ResItemSwapMessage << SwapBItemInfo.ItemCount;
	*ResItemSwapMessage << SwapBItemInfo.SlotIndex;
	*ResItemSwapMessage << SwapBItemInfo.IsEquipped;
	*ResItemSwapMessage << (int16)SwapBItemInfo.ItemType;

	// BItem 이름
	int8 BItemNameLen = (int8)(SwapBItemInfo.ItemName.length() * 2);
	*ResItemSwapMessage << BItemNameLen;
	ResItemSwapMessage->InsertData(SwapBItemInfo.ItemName.c_str(), BItemNameLen);

	// AItem 썸네일 이미지 경로 
	int8 BItemThumbnailImagePathLen = (int8)(SwapBItemInfo.ThumbnailImagePath.length() * 2);
	*ResItemSwapMessage << BItemThumbnailImagePathLen;
	ResItemSwapMessage->InsertData(SwapBItemInfo.ThumbnailImagePath.c_str(), BItemThumbnailImagePathLen);

	return ResItemSwapMessage;
}

// int64 AccountId
// int32 PlayerDBId
// char Dir
CMessage* CGameServer::MakePacketResAttack(int64 PlayerDBId, int64 TargetId, en_AttackType AttackType, int32 Damage, bool IsCritical)
{
	CMessage* ResAttackMessage = CMessage::Alloc();
	if (ResAttackMessage == nullptr)
	{
		return nullptr;
	}

	ResAttackMessage->Clear();

	*ResAttackMessage << (WORD)en_PACKET_S2C_ATTACK;
	*ResAttackMessage << PlayerDBId;
	*ResAttackMessage << TargetId;
	*ResAttackMessage << (int16)AttackType;
	*ResAttackMessage << Damage;
	*ResAttackMessage << IsCritical;

	return ResAttackMessage;
}

CMessage* CGameServer::MakePacketResMagic(int64 ObjectId)
{
	CMessage* ResMagicMessage = CMessage::Alloc();
	if (ResMagicMessage == nullptr)
	{
		return nullptr;
	}

	ResMagicMessage->Clear();

	*ResMagicMessage << (WORD)en_PACKET_S2C_MAGIC_ATTACK;
	*ResMagicMessage << ObjectId;

	return ResMagicMessage;
}

// int64 AccountId
// int32 PlayerDBId
// int32 HP
CMessage* CGameServer::MakePacketResChangeHP(int64 ObjectId, int32 CurrentHP, int32 MaxHP)
{
	CMessage* ResChangeHPPacket = CMessage::Alloc();
	if (ResChangeHPPacket == nullptr)
	{
		return nullptr;
	}

	ResChangeHPPacket->Clear();

	*ResChangeHPPacket << (WORD)en_PACKET_S2C_CHANGE_HP;
	*ResChangeHPPacket << ObjectId;

	*ResChangeHPPacket << CurrentHP;
	*ResChangeHPPacket << MaxHP;

	return ResChangeHPPacket;
}

CMessage* CGameServer::MakePacketResObjectState(int64 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState)
{
	CMessage* ResObjectStatePacket = CMessage::Alloc();
	if (ResObjectStatePacket == nullptr)
	{
		return nullptr;
	}

	ResObjectStatePacket->Clear();

	*ResObjectStatePacket << (WORD)en_PACKET_S2C_OBJECT_STATE_CHANGE;
	*ResObjectStatePacket << ObjectId;
	*ResObjectStatePacket << (int8)Direction;
	*ResObjectStatePacket << (int16)ObjectType;
	*ResObjectStatePacket << (int16)ObjectState;

	return ResObjectStatePacket;
}

// int64 AccountId
// int32 PlayerDBId
// bool CanGo
// st_PositionInfo PositionInfo
CMessage* CGameServer::MakePacketResMove(int64 AccountId, int64 ObjectId, en_GameObjectType ObjectType, st_PositionInfo PositionInfo)
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
	*ResMoveMessage << (int16)ObjectType;

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
		*ResSpawnPacket << ObjectInfos[i].ObjectStatInfo.CriticalPoint;
		*ResSpawnPacket << ObjectInfos[i].ObjectStatInfo.Speed;

		// ObjectType
		*ResSpawnPacket << (int16)ObjectInfos[i].ObjectType;
		*ResSpawnPacket << ObjectInfos[i].OwnerObjectId;
		*ResSpawnPacket << (int16)ObjectInfos[i].OwnerObjectType;
		*ResSpawnPacket << ObjectInfos[i].PlayerSlotIndex;
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
CMessage* CGameServer::MakePacketResChattingMessage(int64 PlayerDBId, en_MessageType MessageType, st_Color Color, wstring ChattingMessage)
{
	CMessage* ResChattingMessage = CMessage::Alloc();
	if (ResChattingMessage == nullptr)
	{
		return nullptr;
	}

	ResChattingMessage->Clear();

	*ResChattingMessage << (WORD)en_PACKET_S2C_MESSAGE;
	*ResChattingMessage << PlayerDBId;
	*ResChattingMessage << (int8)MessageType;

	*ResChattingMessage << Color._Red;
	*ResChattingMessage << Color._Green;
	*ResChattingMessage << Color._Blue;

	int8 PlayerNameLen = (int8)(ChattingMessage.length() * 2);
	*ResChattingMessage << PlayerNameLen;
	ResChattingMessage->InsertData(ChattingMessage.c_str(), PlayerNameLen);

	return ResChattingMessage;
}

CMessage* CGameServer::MakePacketResItemToInventory(int64 TargetObjectId, st_ItemInfo ItemInfo, int16 ItemEach, bool ItemGainPrint)
{
	CMessage* ResItemToInventoryMessage = CMessage::Alloc();
	if (ResItemToInventoryMessage == nullptr)
	{
		return nullptr;
	}

	ResItemToInventoryMessage->Clear();

	*ResItemToInventoryMessage << (WORD)en_PACKET_S2C_ITEM_TO_INVENTORY;
	*ResItemToInventoryMessage << TargetObjectId;
	// ItemInfo
	*ResItemToInventoryMessage << ItemInfo.ItemDBId;
	*ResItemToInventoryMessage << ItemInfo.ItemCount;
	*ResItemToInventoryMessage << ItemInfo.SlotIndex;
	*ResItemToInventoryMessage << ItemInfo.IsEquipped;
	*ResItemToInventoryMessage << (int16)ItemInfo.ItemType;

	// Item 이름
	int8 ItemNameLen = (int8)(ItemInfo.ItemName.length() * 2);
	*ResItemToInventoryMessage << ItemNameLen;
	ResItemToInventoryMessage->InsertData(ItemInfo.ItemName.c_str(), ItemNameLen);

	// 썸네일 이미지 경로 
	int8 ThumbnailImagePathLen = (int8)(ItemInfo.ThumbnailImagePath.length() * 2);
	*ResItemToInventoryMessage << ThumbnailImagePathLen;
	ResItemToInventoryMessage->InsertData(ItemInfo.ThumbnailImagePath.c_str(), ThumbnailImagePathLen);

	// 아이템 낱개 개수
	*ResItemToInventoryMessage << ItemEach;
	// 아이템 얻는 UI 출력 할 것인지 말것인지
	*ResItemToInventoryMessage << ItemGainPrint;

	return ResItemToInventoryMessage;
}

CMessage* CGameServer::MakePacketResSyncPosition(int64 TargetObjectId, st_PositionInfo SyncPosition)
{
	CMessage* ResSyncPositionMessage = CMessage::Alloc();
	if (ResSyncPositionMessage == nullptr)
	{
		return nullptr;
	}

	ResSyncPositionMessage->Clear();

	*ResSyncPositionMessage << (short)en_PACKET_S2C_SYNC_OBJECT_POSITION;
	*ResSyncPositionMessage << TargetObjectId;

	// st_Position
	// State
	*ResSyncPositionMessage << (int8)SyncPosition.State;

	// int32 PositionX, PositionY
	*ResSyncPositionMessage << SyncPosition.PositionX;
	*ResSyncPositionMessage << SyncPosition.PositionY;

	// MoveDir
	*ResSyncPositionMessage << (int8)SyncPosition.MoveDir;

	return ResSyncPositionMessage;
}

void CGameServer::OnClientJoin(int64 SessionID)
{
	st_Job* ClientJoinJob = _JobMemoryPool->Alloc();
	ClientJoinJob->Type = en_MESSAGE_TYPE::AUTH_NEW_CLIENT_JOIN;
	ClientJoinJob->SessionId = SessionID;
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

	NewMessageJob->Type = en_MESSAGE_TYPE::NETWORK_MESSAGE;
	NewMessageJob->SessionId = SessionID;
	NewMessageJob->Message = JobMessage;
	_GameServerNetworkThreadMessageQue.Enqueue(NewMessageJob);
	SetEvent(_NetworkThreadWakeEvent);
}

void CGameServer::OnClientLeave(st_SESSION* LeaveSession)
{
	st_Job* ClientLeaveJob = _JobMemoryPool->Alloc();
	ClientLeaveJob->Type = en_MESSAGE_TYPE::AUTH_DISCONNECT_CLIENT;
	ClientLeaveJob->SessionId = LeaveSession->SessionId;
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