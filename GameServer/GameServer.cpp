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
	_TimerJobThreadEnd = false;

	InitializeSRWLock(&_TimerJobLock);

	_JobMemoryPool = new CMemoryPoolTLS<st_Job>();
	_TimerJobMemoryPool = new CMemoryPoolTLS<st_TimerJob>();

	_TimerHeapJob = new CHeap<int64, st_TimerJob*>(1000);

	_AuthThreadTPS = 0;
	_AuthThreadWakeCount = 0;

	_NetworkThreadTPS = 0;
	
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
	G_ObjectManager->MonsterSpawn(50, 1, en_GameObjectType::BEAR);

	G_ObjectManager->GameServer = this;

	_AuthThread = (HANDLE)_beginthreadex(NULL, 0, AuthThreadProc, this, 0, NULL);
	_NetworkThread = (HANDLE)_beginthreadex(NULL, 0, NetworkThreadProc, this, 0, NULL);
	_DataBaseThread = (HANDLE)_beginthreadex(NULL, 0, DataBaseThreadProc, this, 0, NULL);
	_TimerJobThread = (HANDLE)_beginthreadex(NULL, 0, TimerJobThreadProc, this, 0, NULL);

	CloseHandle(_AuthThread);
	CloseHandle(_NetworkThread);
	CloseHandle(_DataBaseThread);
	CloseHandle(_TimerJobThread);
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
			case en_JobType::AUTH_NEW_CLIENT_JOIN:
				Instance->CreateNewClient(Job->SessionId);
				break;
			case en_JobType::AUTH_DISCONNECT_CLIENT:
				Instance->DeleteClient(Job->Session);
				break;
			case en_JobType::AUTH_MESSAGE:
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
			case en_JobType::NETWORK_MESSAGE:
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
			case en_JobType::DATA_BASE_ACCOUNT_CHECK:
				Instance->PacketProcReqDBAccountCheck(Job->SessionId, Job->Message);
				break;
			case en_JobType::DATA_BASE_CHARACTER_CHECK:
				Instance->PacketProcReqDBCreateCharacterNameCheck(Job->SessionId, Job->Message);
				break;
			case en_JobType::DATA_BASE_ITEM_CREATE:
				Instance->PacketProcReqDBItemCreate(Job->Message);
				break;
			case en_JobType::DATA_BASE_ITEM_INVENTORY_SAVE:
				Instance->PacketProcReqDBItemToInventorySave(Job->SessionId, Job->Message);
				break;
			case en_JobType::DATA_BASE_ITEM_SWAP:
				Instance->PacketProcReqDBItemSwap(Job->SessionId, Job->Message);
				break;
			case en_JobType::DATA_BASE_GOLD_SAVE:
				Instance->PacketProcReqDBGoldSave(Job->SessionId, Job->Message);
				break;
			case en_JobType::DATA_BASE_CHARACTER_INFO_SEND:
				Instance->PacketProcReqDBCharacterInfoSend(Job->SessionId, Job->Message);
				break;
			case en_JobType::DATA_BASE_QUICK_SLOT_SAVE:
				Instance->PacketProcReqDBQuickSlotBarSlotSave(Job->SessionId, Job->Message);
				break;
			}

			Instance->_DataBaseThreadTPS++;
			Instance->_JobMemoryPool->Free(Job);
		}
	}
	return 0;
}

unsigned __stdcall CGameServer::TimerJobThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_TimerJobThreadEnd)
	{			
		while (Instance->_TimerHeapJob->GetUseSize() != 0)
		{			
			AcquireSRWLockExclusive(&Instance->_TimerJobLock);
			// 맨 위 데이터의 시간을 확인한다.
			st_TimerJob* TimerJob = Instance->_TimerHeapJob->Peek();

			// 현재 시간을 구한다.
			int64 NowTick = GetTickCount64();

			// TimerJob에 기록되어 있는 시간 보다 현재 시간이 클 경우
			// 즉, 시간이 경과 되었으면
			if (TimerJob->ExecTick <= NowTick)
			{
				// TimerJob을 뽑고
				st_TimerJob* TimerJob = Instance->_TimerHeapJob->PopHeap();

				// Type에 따라 실행한다.
				switch (TimerJob->Type)
				{
				case en_TimerJobType::TIMER_ATTACK_END:
					Instance->PacketProcTimerAttackEnd(TimerJob->SessionId, TimerJob->Message);
					break;
				case en_TimerJobType::TIMER_SPELL_END:
					Instance->PacketProcTimerSpellEnd(TimerJob->SessionId, TimerJob->Message);
					break;
				default:
					break;
				}
			}
			else
			{
				break;
			}
			ReleaseSRWLockExclusive(&Instance->_TimerJobLock);
		}			
	
		Sleep(0);
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
	st_Session* Session = FindSession(SessionId);

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

void CGameServer::DeleteClient(st_Session* Session)
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
	case en_PACKET_TYPE::en_PACKET_C2S_MAGIC:
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
	case en_PACKET_TYPE::en_PACKET_C2S_QUICKSLOT_SAVE:
		PacketProcReqQuickSlotSave(SessionId, Message);
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
	st_Session* Session = FindSession(SessionID);

	int64 AccountId;
	int32 Token;

	if (Session != nullptr)
	{
		// AccountID 셋팅
		*Message >> AccountId;

		// 중복 로그인 확인
		for (st_Session* FindSession : _SessionArray)
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
		DBAccountCheckJob->Type = en_JobType::DATA_BASE_ACCOUNT_CHECK;
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
	st_Session* Session = FindSession(SessionID);

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
		DBCharacterCheckJob->Type = en_JobType::DATA_BASE_CHARACTER_CHECK;
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
	st_Session* Session = FindSession(SessionID);

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
			DBCharacterInfoSendJob->Type = en_JobType::DATA_BASE_CHARACTER_INFO_SEND;
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
	st_Session* Session = FindSession(SessionID);

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
	st_Session* Session = FindSession(SessionID);

	do
	{
		int64 AccountId;
		int64 ObjectId;

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

			// 스킬 종류
			int16 ReqSkillType;
			*Message >> ReqSkillType;					

			// 공격 방향 캐스팅
			en_MoveDir MoveDir = (en_MoveDir)ReqMoveDir;
			MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = MoveDir;

			st_Vector2Int FrontCell;
			vector<CGameObject*> Targets;
			CGameObject* Target = nullptr;
			CMessage* ResSyncPosition = nullptr;
					
			// 퀵바에 등록되지 않은 스킬을 요청했을 경우
			if ((en_SkillType)ReqSkillType == en_SkillType::SKILL_TYPE_NONE)
			{
				break;
			}

			// 스킬 지정
			MyPlayer->_SkillType = (en_SkillType)ReqSkillType;
			// 공격 상태로 변경
			MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;									
			// 클라에게 알려줘서 공격 애니메이션 출력
			CMessage* ResObjectStateChangePacket = MakePacketResObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
			SendPacketAroundSector(MyPlayer->GetCellPosition(), ResObjectStateChangePacket);
			ResObjectStateChangePacket->Free();
			
			// 0.5초 후에 Idle상태로 바꾸기 위해 TimerJob 등록
			st_TimerJob* TimerJob = _TimerJobMemoryPool->Alloc();
			TimerJob->ExecTick = GetTickCount64() + 500;
			TimerJob->SessionId = MyPlayer->_SessionId;
			TimerJob->Type = en_TimerJobType::TIMER_ATTACK_END;
		
			AcquireSRWLockExclusive(&_TimerJobLock);
			_TimerHeapJob->InsertHeap(TimerJob->ExecTick, TimerJob);			
			ReleaseSRWLockExclusive(&_TimerJobLock);

			// 타겟 위치 확인
			switch ((en_SkillType)ReqSkillType)
			{
			case en_SkillType::SKILL_TYPE_NONE:
				break;
			case en_SkillType::SKILL_KNIGHT_NORMAL:
			case en_SkillType::SKILL_SHAMAN_NORMAL:				
				FrontCell = MyPlayer->GetFrontCellPosition(MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);
				Target = MyPlayer->_Channel->_Map->Find(FrontCell);
				if (Target != nullptr)
				{
					Targets.push_back(Target);
				}
				break;
			case en_SkillType::SKILL_KNIGHT_CHOHONE:	
			{				
				if (MyPlayer->_SelectTarget != nullptr)
				{
					st_Vector2Int TargetPosition = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType)->GetCellPosition();
					st_Vector2Int MyPosition = MyPlayer->GetCellPosition();
					st_Vector2Int Direction = TargetPosition - MyPosition;

					int32 Distance = st_Vector2Int::Distance(TargetPosition, MyPosition);

					if (Distance <= 4)
					{
						Target = MyPlayer->_Channel->_Map->Find(TargetPosition);
						if (Target != nullptr)
						{
							Targets.push_back(Target);

							st_Vector2Int MyFrontCellPotision = MyPlayer->GetFrontCellPosition(MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);

							MyPlayer->_Channel->_Map->ApplyMove(Target, MyFrontCellPotision);
							ResSyncPosition = MakePacketResSyncPosition(Target->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectPositionInfo);
							SendPacketAroundSector(Target->GetCellPosition(), ResSyncPosition);
							ResSyncPosition->Free();
						}
					}
				}							
			}				
				break;
			case en_SkillType::SKILL_KNIGHT_SHAEHONE:
			{
				if (MyPlayer->_SelectTarget != nullptr)
				{
					st_Vector2Int TargetPosition = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType)->GetCellPosition();
					st_Vector2Int MyPosition = MyPlayer->GetCellPosition();
					st_Vector2Int Direction = TargetPosition - MyPosition;

					// 타겟이 어느 방향에 있는지 확인한다.
					en_MoveDir Dir = st_Vector2Int::GetDirectionFromVector(Direction);
					// 타겟과의 거리를 구한다.
					int32 Distance = st_Vector2Int::Distance(TargetPosition, MyPosition);

					G_Logger->WriteStdOut(en_Color::YELLOW, L"Distance %d\n", Distance);
					if (Distance <= 4)
					{
						Target = MyPlayer->_Channel->_Map->Find(TargetPosition);
						st_Vector2Int MovePosition;
						if (Target != nullptr)
						{
							Targets.push_back(Target);

							switch (Dir)
							{
							case en_MoveDir::UP:
								MovePosition = Target->GetFrontCellPosition(en_MoveDir::DOWN, 1);
								MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::UP;
								break;
							case en_MoveDir::DOWN:
								MovePosition = Target->GetFrontCellPosition(en_MoveDir::UP, 1);
								MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
								break;
							case en_MoveDir::LEFT:
								MovePosition = Target->GetFrontCellPosition(en_MoveDir::RIGHT, 1);
								MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::LEFT;
								break;
							case en_MoveDir::RIGHT:
								MovePosition = Target->GetFrontCellPosition(en_MoveDir::LEFT, 1);
								MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::RIGHT;
								break;
							default:
								break;
							}

							MyPlayer->_Channel->_Map->ApplyMove(MyPlayer, MovePosition);

							ResSyncPosition = MakePacketResSyncPosition(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
							SendPacketAroundSector(MyPlayer->GetCellPosition(), ResSyncPosition);
							ResSyncPosition->Free();
						}
					}				
				}				
			}				
				break;
			case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
			{
				vector<st_Vector2Int> TargetPositions;

				TargetPositions = MyPlayer->GetAroundCellPosition(MyPlayer->GetCellPosition(), 1);
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
			default:
				break;
			}			

			wstring SkillTypeString;
			wchar_t SkillTypeMessage[64] = L"0";
			wchar_t SkillDamageMessage[64] = L"0";

			// 타겟 데미지 적용
			for (CGameObject* Target : Targets)
			{
				// 크리티컬 판단
				random_device Seed;
				default_random_engine Eng(Seed());

				float CriticalPoint = MyPlayer->_GameObjectInfo.ObjectStatInfo.CriticalPoint / 1000.0f;
				bernoulli_distribution CriticalCheck(CriticalPoint);
				bool IsCritical = CriticalCheck(Eng);

				// 데미지 판단
				mt19937 Gen(Seed());
				uniform_int_distribution<int> DamageChoiceRandom(MyPlayer->_GameObjectInfo.ObjectStatInfo.MinAttackDamage, MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxAttackDamage);
				int32 ChoiceDamage = DamageChoiceRandom(Gen);
				int32 FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

				Target->OnDamaged(MyPlayer, FinalDamage);

				// 데미지 시스템 메세지 생성
				switch ((en_SkillType)ReqSkillType)
				{
				case en_SkillType::SKILL_TYPE_NONE:					
					CRASH("SkillType None");
					break;
				case en_SkillType::SKILL_KNIGHT_NORMAL:
					wsprintf(SkillTypeMessage, L"%s가 일반공격을 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
					break;
				case en_SkillType::SKILL_KNIGHT_CHOHONE:
					wsprintf(SkillTypeMessage, L"%s가 초혼비무를 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
					break;
				case en_SkillType::SKILL_KNIGHT_SHAEHONE:
					wsprintf(SkillTypeMessage, L"%s가 쇄혼비무를 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
					break;
				case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
					wsprintf(SkillTypeMessage, L"%s가 분쇄파동을 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
					break;
				case en_SkillType::SKILL_SHAMAN_NORMAL:
					wsprintf(SkillTypeMessage, L"%s가 일반공격을 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
					break;				
				default:
					break;
				}

				SkillTypeString = SkillTypeMessage;
				SkillTypeString = IsCritical ? L"치명타! " + SkillTypeString : SkillTypeString;

				// 데미지 시스템 메세지 전송
				CMessage* ResSkillSystemMessagePacket = MakePacketResChattingMessage(MyPlayer->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, IsCritical ? st_Color::Red() : st_Color::White(), SkillTypeString);
				SendPacketAroundSector(MyPlayer->GetCellPosition(), ResSkillSystemMessagePacket);
				ResSkillSystemMessagePacket->Free();

				// 공격 응답 메세지 전송
				CMessage* ResMyAttackOtherPacket = MakePacketResAttack(MyPlayer->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectId, (en_SkillType)ReqSkillType, FinalDamage, IsCritical);
				G_ObjectManager->GameServer->SendPacketAroundSector(MyPlayer->GetCellPosition(), ResMyAttackOtherPacket);
				ResMyAttackOtherPacket->Free();

				// HP 변경 메세지 전송
				CMessage* ResChangeHPPacket = G_ObjectManager->GameServer->MakePacketResChangeHP(Target->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectStatInfo.HP, Target->_GameObjectInfo.ObjectStatInfo.MaxHP);
				G_ObjectManager->GameServer->SendPacketAroundSector(Target->GetCellPosition(), ResChangeHPPacket);
				ResChangeHPPacket->Free();
			}			
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqMagic(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		do
		{
			int64 AccountId;
			int64 ObjectId;

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
			if (MyPlayer->_SelectTarget != nullptr)
			{
				// 공격한 방향
				int8 ReqMoveDir;
				*Message >> ReqMoveDir;

				int16 SkillType;
				*Message >> SkillType;

				vector<CGameObject*> Targets;

				CGameObject* FindGameObject = nullptr;				

				float SpellTime = 0.0f;				

				// 스킬 타입 확인
				switch ((en_SkillType)SkillType)
				{
					// 돌격 자세
				case en_SkillType::SKILL_KNIGHT_CHARGE_POSE:
					MyPlayer->_SpellTick = GetTickCount() + 100;
					SpellTime = 100.0f / 1000.0f;

					Targets.push_back(MyPlayer);
					break;
					// 불꽃 작살
				case en_SkillType::SKILL_SHAMNA_FLAME_HARPOON:
					MyPlayer->_SpellTick = GetTickCount64() + 500;
					SpellTime = 500.0f / 1000.0f;

					// 타겟이 ObjectManager에 존재하는지 확인
					FindGameObject = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType);
					if (FindGameObject != nullptr)
					{
						Targets.push_back(FindGameObject);
					}
					break;
					// 치유의 빛
				case en_SkillType::SKILL_SHAMAN_HEALING_LIGHT:
					MyPlayer->_SpellTick = GetTickCount64() + 1000;
					
					SpellTime = 1000.0f / 1000.0f;

					FindGameObject = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType);
					if (FindGameObject != nullptr)
					{
						Targets.push_back(FindGameObject);
					}
					break;
					// 치유의 바람
				case en_SkillType::SKILL_SHAMAN_HEALING_WIND:
					MyPlayer->_SpellTick = GetTickCount64() + 1500;

					SpellTime = 1500.0f / 1000.0f;

					FindGameObject = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType);
					if (FindGameObject != nullptr)
					{
						Targets.push_back(FindGameObject);
					}
					break;
				}
				
				// 스펠창 시작
				CMessage* ResMagicPacket = G_ObjectManager->GameServer->MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId, true, (en_SkillType)SkillType, SpellTime);
				G_ObjectManager->GameServer->SendPacketAroundSector(MyPlayer->GetCellPosition(), ResMagicPacket);
				ResMagicPacket->Free();

				MyPlayer->_SkillType = (en_SkillType)SkillType;

				if (Targets.size() >= 1)
				{					
					MyPlayer->SetTarget(Targets[0]);
					
					MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPELL;

					// 마법 스킬 모션 출력
					CMessage* ResObjectStateChangePacket = MakePacketResObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
					SendPacketAroundSector(MyPlayer->GetCellPosition(), ResObjectStateChangePacket);
					ResObjectStateChangePacket->Free();					

					// TimerJob 등록
					st_TimerJob* TimerJob = _TimerJobMemoryPool->Alloc();
					TimerJob->ExecTick = MyPlayer->_SpellTick;
					TimerJob->SessionId = MyPlayer->_SessionId;
					TimerJob->Type = en_TimerJobType::TIMER_SPELL_END;		

					AcquireSRWLockExclusive(&_TimerJobLock);
					_TimerHeapJob->InsertHeap(TimerJob->ExecTick, TimerJob);
					ReleaseSRWLockExclusive(&_TimerJobLock);					
				}
				else
				{
					// 선택한 대상이 없다고 클라에게 알려줘야함
				}
			}
			else
			{
				CRASH("선택한 대상이 없는데 스킬 요청");
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
	st_Session* Session = FindSession(SessionID);

	do
	{
		if (Session)
		{
			int64 AccountId;
			int64 PlayerId;

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

			*Message >> PlayerId;

			if (Session->MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				if (Session->MyPlayer->_GameObjectInfo.ObjectId != PlayerId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			int64 ObjectId;
			*Message >> ObjectId;

			int16 ObjectType;
			*Message >> ObjectType;

			CGameObject* FindObject = G_ObjectManager->Find(ObjectId, (en_GameObjectType)ObjectType);
			if (FindObject != nullptr)
			{
				int64 PreviousChoiceObject = 0;

				if (Session->MyPlayer->_SelectTarget != nullptr)
				{
					PreviousChoiceObject = Session->MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId;
				}				

				Session->MyPlayer->_SelectTarget = FindObject;

				CMessage* ResMousePositionObjectInfo = MakePacketResMousePositionObjectInfo(Session->AccountId, PreviousChoiceObject, FindObject->_GameObjectInfo);
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
	st_Session* Session = FindSession(SessionId);

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
	st_Session* Session = FindSession(SessionId);

	int64 AccountId;
	int64 PlayerDBId;

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
	st_Session* Session = FindSession(SessionId);

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
					DBGoldSaveJob->Type = en_JobType::DATA_BASE_GOLD_SAVE;
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
							TargetPlayer->_Inventory.AddItem(Item->_ItemInfo);

							ItemCount = 1;
						}
					}

					st_Job* DBInventorySaveJob = _JobMemoryPool->Alloc();
					DBInventorySaveJob->Type = en_JobType::DATA_BASE_ITEM_INVENTORY_SAVE;
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
					// 아이템 소비 타입
					*DBSaveMessage << (int16)Item->_ItemInfo.ItemConsumableType;
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
	st_Session* Session = FindSession(SessionId);

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
			DBItemSwapJob->Type = en_JobType::DATA_BASE_ITEM_SWAP;
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
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqQuickSlotSave(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

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

			st_Job* DBQuickSlotSaveJob = _JobMemoryPool->Alloc();
			DBQuickSlotSaveJob->Type = en_JobType::DATA_BASE_QUICK_SLOT_SAVE;
			DBQuickSlotSaveJob->SessionId = Session->MyPlayer->_SessionId;

			CMessage* DBQuickSlotSaveMessage = CMessage::Alloc();
			DBQuickSlotSaveMessage->Clear();

			*DBQuickSlotSaveMessage << AccountId;
			*DBQuickSlotSaveMessage << PlayerDBId;
			DBQuickSlotSaveMessage->InsertData(Message->GetFrontBufferPtr(), Message->GetUseBufferSize());
			Message->MoveReadPosition(Message->GetUseBufferSize());

			DBQuickSlotSaveJob->Message = DBQuickSlotSaveMessage;

			_GameServerDataBaseThreadMessageQue.Enqueue(DBQuickSlotSaveJob);
			SetEvent(_DataBaseWakeEvent);

		} while (0);
	}

	ReturnSession(Session);
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
	st_Session* Session = FindSession(SessionID);

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
	st_Session* Session = FindSession(SessionID);

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
	st_Session* Session = FindSession(SessionID);

	Session->RecvPacketTime = timeGetTime();

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBAccountCheck(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

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
			int32 PlayerMinAttack;
			int32 PlayerMaxAttack;
			int16 PlayerCriticalPoint;
			float PlayerSpeed;
			int16 PlayerObjectType;

			ClientPlayersGet.OutPlayerDBID(PlayerId);
			ClientPlayersGet.OutPlayerName(PlayerName);
			ClientPlayersGet.OutPlayerIndex(PlayerIndex);
			ClientPlayersGet.OutLevel(PlayerLevel);
			ClientPlayersGet.OutCurrentHP(PlayerCurrentHP);
			ClientPlayersGet.OutMaxHP(PlayerMaxHP);
			ClientPlayersGet.OutMinAttack(PlayerMinAttack);
			ClientPlayersGet.OutMaxAttack(PlayerMaxAttack);
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
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.MinAttackDamage = PlayerMinAttack;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.MaxAttackDamage = PlayerMaxAttack;
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
	st_Session* Session = FindSession(SessionID);

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

#pragma region 생성할 캐릭터가 DB에 있는지 확인
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
#pragma endregion		
#pragma region 캐릭터 생성 후 클라에 캐릭터 생성 응답 보내기
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
			NewCharacterPush.InMinAttack(NewCharacterStatus.MinAttackDamage);
			NewCharacterPush.InMaxAttack(NewCharacterStatus.MaxAttackDamage);
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
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectStatInfo.MinAttackDamage = NewCharacterStatus.MinAttackDamage;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectStatInfo.MaxAttackDamage = NewCharacterStatus.MaxAttackDamage;
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
			
			// DB에 인벤토리 생성
			for (int8 SlotIndex = 0; SlotIndex < (int8)en_Inventory::INVENTORY_SIZE; SlotIndex++)
			{
				CDBConnection* DBItemToInventoryConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerItemCreateToInventory ItemToInventory(*DBItemToInventoryConnection);
				st_ItemInfo NewItem;

				NewItem.ItemDBId = 0;
				NewItem.IsQuickSlotUse = false;
				NewItem.ItemType = en_ItemType::ITEM_TYPE_NONE;
				NewItem.ItemConsumableType = en_ConsumableType::NONE;
				NewItem.ItemName = L"";
				NewItem.ItemCount = 0;
				NewItem.SlotIndex = SlotIndex;
				NewItem.IsEquipped = false;
				NewItem.ThumbnailImagePath = L"";

				int16 ItemType = (int16)NewItem.ItemType;
				int16 ItemConsumableType = (int16)NewItem.ItemConsumableType;
								
				ItemToInventory.InQuickSlotUse(NewItem.IsQuickSlotUse);
				ItemToInventory.InItemType(ItemType);
				ItemToInventory.InItemConsumableType(ItemConsumableType);
				ItemToInventory.InItemName(NewItem.ItemName);
				ItemToInventory.InItemCount(NewItem.ItemCount);
				ItemToInventory.InSlotIndex(SlotIndex);
				ItemToInventory.InIsEquipped(NewItem.IsEquipped);
				ItemToInventory.InThumbnailImagePath(NewItem.ThumbnailImagePath);
				ItemToInventory.InOwnerAccountId(Session->AccountId);
				ItemToInventory.InOwnerPlayerId(PlayerDBId);

				ItemToInventory.Execute();
			}			

			// DB에 퀵슬롯바 생성
			for (int8 SlotIndex = 0; SlotIndex < (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SIZE; ++SlotIndex)
			{
				CDBConnection* DBQuickSlotCreateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerQuickSlotBarSlotCreate QuickSlotBarSlotCreate(*DBQuickSlotCreateConnection);
				
				int64 AccountDBId = Session->AccountId;
				int64 PlayerDBId = Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectId;
				int8 QuickSlotBarIndex = SlotIndex;
				int8 QuickSlotBarSlotIndex;
				wstring QuickSlotBarKey;
				int16 SkillType = (int16)(en_SkillType::SKILL_TYPE_NONE);
				int8 SkillLevel = 0;
				wstring SkillName;
				int32 SkillCoolTime = 0;
				wstring SkillThumbnailImagePath;
				WCHAR QuickSlotBarKeyString[10] = { 0 };
								
				for (int8 i = 0; i < (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SLOT_SIZE; ++i)
				{
					QuickSlotBarSlotIndex = i;

					wsprintf(QuickSlotBarKeyString, L"%d", i + 1);
					QuickSlotBarKey = QuickSlotBarKeyString;

					QuickSlotBarSlotCreate.InAccountDBId(AccountDBId);
					QuickSlotBarSlotCreate.InPlayerDBId(PlayerDBId);
					QuickSlotBarSlotCreate.InQuickSlotBarIndex(SlotIndex);
					QuickSlotBarSlotCreate.InQuickSlotBarSlotIndex(QuickSlotBarSlotIndex);
					QuickSlotBarSlotCreate.InQuickSlotKey(QuickSlotBarKey);
					QuickSlotBarSlotCreate.InSkillType(SkillType);
					QuickSlotBarSlotCreate.InSkillLevel(SkillLevel);
					QuickSlotBarSlotCreate.InSkillName(SkillName);
					QuickSlotBarSlotCreate.InSkillCoolTime(SkillCoolTime);
					QuickSlotBarSlotCreate.InSkillThumbnailImagePath(SkillThumbnailImagePath);

					QuickSlotBarSlotCreate.Execute();
				}
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
#pragma endregion

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

		NewItemInfo.IsQuickSlotUse = false;
		NewItemInfo.ItemType = DropItemData.ItemType;
		NewItemInfo.ItemConsumableType = DropItemData.ItemConsumableType;
		NewItemInfo.ItemName = (LPWSTR)CA2W(DropItemData.ItemName.c_str());
		NewItemInfo.ItemCount = DropItemData.ItemCount;
		NewItemInfo.IsEquipped = false;
		NewItemInfo.ThumbnailImagePath = (LPWSTR)CA2W(DropItemData.ThumbnailImagePath.c_str());

		int64 ItemDBId = 0;

		// 아이템 DB에 생성
		CDBConnection* DBCreateItemConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerCreateItem CreateItem(*DBCreateItemConnection);

		int16 ItemType = (int16)NewItemInfo.ItemType;
		int16 ItemConsumableeType = (int16)NewItemInfo.ItemConsumableType;

		bool ItemUse = false;		
		CreateItem.InItemUse(ItemUse);
		CreateItem.InIsQuickSlotUse(NewItemInfo.IsQuickSlotUse);
		CreateItem.InItemType(ItemType);
		CreateItem.InItemConsumableType(ItemConsumableeType);
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
				case en_ItemType::ITEM_TYPE_SKILL_BOOK:
					GameObjectType = en_GameObjectType::SKILL_BOOK;
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
	st_Session* Session = FindSession(SessionId);

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

		int16 ItemConsumableType;
		*Message >> ItemConsumableType;

		int64 OwnerAccountId;
		*Message >> OwnerAccountId;

		st_ItemInfo ItemInfo;
		ItemInfo.IsQuickSlotUse = false;
		ItemInfo.ItemType = (en_ItemType)(ItemType);
		ItemInfo.ItemConsumableType = (en_ConsumableType)(ItemConsumableType);
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
			ItemToInventoryPush.InIsQuickSlotUse(ItemInfo.IsQuickSlotUse);
			ItemToInventoryPush.InItemType(ItemType);
			ItemToInventoryPush.InItemConsumableType(ItemConsumableType);
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
	st_Session* Session = FindSession(SessionId);

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

		bool AIsQuickSlotUse = false;
		int16 ADBItemType = -1;
		int16 ADBItemConsumableType = -1;
		WCHAR AItemName[20] = { 0 };
		int16 AItemCount = -1;
		bool AItemEquipped = false;
		WCHAR AItemThumbnailImagePath[100] = { 0 };	
		ADBItemCheck.OutIsQuickSlotUse(AIsQuickSlotUse);
		ADBItemCheck.OutItemType(ADBItemType);
		ADBItemCheck.OutItemConsumableType(ADBItemConsumableType);
		ADBItemCheck.OutItemName(AItemName);
		ADBItemCheck.OutItemCount(AItemCount);
		ADBItemCheck.OutItemIsEquipped(AItemEquipped);
		ADBItemCheck.OutItemThumbnailImagePath(AItemThumbnailImagePath);

		ADBItemCheck.Execute();

		ADBItemCheck.Fetch();

		// 스왑 요청할 A 아이템 정보 셋팅
		st_ItemInfo SwapAItemInfo;		
		SwapAItemInfo.SlotIndex = SwapAIndex;
		SwapAItemInfo.IsQuickSlotUse = AIsQuickSlotUse;
		SwapAItemInfo.ItemType = (en_ItemType)ADBItemType;
		SwapAItemInfo.ItemConsumableType = (en_ConsumableType)ADBItemConsumableType;
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

		bool BIsQuickSlotUse = false;
		int16 BDBItemType = -1;
		int16 BDBItemConsumableType = -1;
		WCHAR BItemName[20] = { 0 };
		int16 BItemCount = -1;
		bool BItemEquipped = false;
		WCHAR BItemThumbnailImagePath[100] = { 0 };

		DBItemCheck.OutIsQuickSlotUse(BIsQuickSlotUse);
		DBItemCheck.OutItemType(BDBItemType);
		DBItemCheck.OutItemConsumableType(BDBItemConsumableType);
		DBItemCheck.OutItemName(BItemName);
		DBItemCheck.OutItemCount(BItemCount);
		DBItemCheck.OutItemIsEquipped(BItemEquipped);
		DBItemCheck.OutItemThumbnailImagePath(BItemThumbnailImagePath);

		DBItemCheck.Execute();

		DBItemCheck.Fetch();

		// 스왑 요청할 B 아이템 정보 셋팅
		st_ItemInfo SwapBItemInfo;		
		SwapBItemInfo.SlotIndex = SwapBIndex;
		SwapBItemInfo.IsQuickSlotUse = BIsQuickSlotUse;
		SwapBItemInfo.ItemType = (en_ItemType)BDBItemType;
		SwapBItemInfo.ItemConsumableType = (en_ConsumableType)BDBItemConsumableType;
		SwapBItemInfo.ItemName = BItemName;
		SwapBItemInfo.ItemCount = BItemCount;
		SwapBItemInfo.IsEquipped = BItemEquipped;
		SwapBItemInfo.ThumbnailImagePath = BItemThumbnailImagePath;

		G_DBConnectionPool->Push(en_DBConnect::GAME, BItemCheckDBConnection);

		// 스왑 하기 위해서 임시 Temp 변수 생성
		st_ItemInfo Temp;
		Temp.IsQuickSlotUse = SwapAItemInfo.IsQuickSlotUse;
		Temp.ItemType = SwapAItemInfo.ItemType;
		Temp.ItemConsumableType = SwapAItemInfo.ItemConsumableType;
		Temp.ItemName = SwapAItemInfo.ItemName;
		Temp.ItemCount = SwapAItemInfo.ItemCount;
		Temp.IsEquipped = SwapAItemInfo.IsEquipped;
		Temp.ThumbnailImagePath = SwapAItemInfo.ThumbnailImagePath;

		// 데이터 스왑
		SwapAItemInfo.ItemDBId = 0;
		SwapAItemInfo.IsQuickSlotUse = SwapBItemInfo.IsQuickSlotUse;
		SwapAItemInfo.ItemType = SwapBItemInfo.ItemType;
		SwapAItemInfo.ItemConsumableType = SwapBItemInfo.ItemConsumableType;
		SwapAItemInfo.ItemName = SwapBItemInfo.ItemName;
		SwapAItemInfo.ItemCount = SwapBItemInfo.ItemCount;
		SwapAItemInfo.IsEquipped = SwapBItemInfo.IsEquipped;
		SwapAItemInfo.ThumbnailImagePath = SwapBItemInfo.ThumbnailImagePath;

		SwapBItemInfo.ItemDBId = 0;
		SwapBItemInfo.IsQuickSlotUse = Temp.IsQuickSlotUse;
		SwapBItemInfo.ItemType = Temp.ItemType;
		SwapBItemInfo.ItemConsumableType = Temp.ItemConsumableType;
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

		ItemSwap.InAIsQuickSlotUse(SwapAItemInfo.IsQuickSlotUse);
		ItemSwap.InAItemType(AItemType);
		ItemSwap.InAItemConsumableType(ADBItemConsumableType);
		ItemSwap.InAItemName(SwapAItemInfo.ItemName);
		ItemSwap.InAItemCount(SwapAItemInfo.ItemCount);
		ItemSwap.InAItemIsEquipped(SwapAItemInfo.IsEquipped);
		ItemSwap.InAItemThumbnailImagePath(SwapAItemInfo.ThumbnailImagePath);
		ItemSwap.InAItemSlotIndex(SwapAItemInfo.SlotIndex);

		ItemSwap.InBIsQuickSlotUse(SwapBItemInfo.IsQuickSlotUse);
		ItemSwap.InBItemType(BItemType);
		ItemSwap.InBItemConsumableType(BDBItemConsumableType);
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
	st_Session* Session = FindSession(SessionId);

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

#pragma region 골드 테이블에 골드 저장
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
#pragma endregion		
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBCharacterInfoSend(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		do
		{
			InterlockedDecrement64(&Session->IOBlock->IOCount);

			int64 GoldCoin = 0;
			int8 SliverCoin = 0;
			int8 BronzeCoin = 0;

#pragma region 골드 정보 읽어오기
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
#pragma endregion			
			
#pragma region 퀵슬롯 정보 가져오기
			// 퀵슬롯 정보 초기화
			Session->MyPlayer->_QuickSlotManager.Init();

			vector<st_QuickSlotBarSlotInfo> QuickSlotBarSlotInfos;

			// 퀵슬롯 테이블 접근해서 해당 스킬이 등록되어 있는 모든 퀵슬롯 번호 가지고옴
			CDBConnection* DBQuickSlotBarGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotBarGet QuickSlotBarGet(*DBQuickSlotBarGetConnection);
			QuickSlotBarGet.InAccountDBId(Session->MyPlayer->_AccountId);
			QuickSlotBarGet.InPlayerDBId(Session->MyPlayer->_GameObjectInfo.ObjectId);

			int8 QuickSlotBarIndex;
			int8 QuickSlotBarSlotIndex;
			WCHAR QuickSlotKey[20] = { 0 };
			int16 QuickSlotSkillType;
			int8 QuickSlotSkillLevel;
			int32 QuickSlotSkillCoolTime;
			WCHAR QuickSlotSkillThumbnailImagePath[100] = { 0 };

			QuickSlotBarGet.OutQuickSlotBarIndex(QuickSlotBarIndex);
			QuickSlotBarGet.OutQuickSlotBarItemIndex(QuickSlotBarSlotIndex);
			QuickSlotBarGet.OutQuickSlotKey(QuickSlotKey);
			QuickSlotBarGet.OutQuickSlotSkillType(QuickSlotSkillType);
			QuickSlotBarGet.OutQuickSlotSkillLevel(QuickSlotSkillLevel);
			QuickSlotBarGet.OutQuickSlotSkillCoolTime(QuickSlotSkillCoolTime);
			QuickSlotBarGet.OutQuickSlotSkillThumbnailImagePath(QuickSlotSkillThumbnailImagePath);

			QuickSlotBarGet.Execute();
			
			while (QuickSlotBarGet.Fetch())
			{
				st_QuickSlotBarSlotInfo NewQuickSlotBarSlot;
				NewQuickSlotBarSlot.AccountDBId = Session->MyPlayer->_AccountId;
				NewQuickSlotBarSlot.PlayerDBId = Session->MyPlayer->_GameObjectInfo.ObjectId;
				NewQuickSlotBarSlot.QuickSlotBarIndex = QuickSlotBarIndex;
				NewQuickSlotBarSlot.QuickSlotBarSlotIndex = QuickSlotBarSlotIndex;
				NewQuickSlotBarSlot.QuickSlotKey = QuickSlotKey;
				NewQuickSlotBarSlot.QuickBarSkillInfo._SkillType = (en_SkillType)QuickSlotSkillType;
				NewQuickSlotBarSlot.QuickBarSkillInfo._SkillLevel = QuickSlotSkillLevel;
				NewQuickSlotBarSlot.QuickBarSkillInfo._SkillCoolTime = QuickSlotSkillCoolTime;
				NewQuickSlotBarSlot.QuickBarSkillInfo._SkillImagePath = QuickSlotSkillThumbnailImagePath;

				// 퀵슬롯에 등록한다.
				Session->MyPlayer->_QuickSlotManager.UpdateQuickSlotBar(NewQuickSlotBarSlot);	
				QuickSlotBarSlotInfos.push_back(NewQuickSlotBarSlot);
			}

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotBarGetConnection);
			// 클라에 퀵슬롯 생성하라고 알려줌
			CMessage* ResQuickSlotBarCreateMessage = MakePacketQuickSlotCreate((int8)en_QuickSlotBar::QUICK_SLOT_BAR_SIZE, (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SLOT_SIZE, QuickSlotBarSlotInfos);
			SendPacket(Session->SessionId, ResQuickSlotBarCreateMessage);
			ResQuickSlotBarCreateMessage->Free();
#pragma endregion


#pragma region 가방 아이템 정보 읽어오기
			// 인벤토리 초기화
			Session->MyPlayer->_Inventory.Init();

			// 캐릭터가 소유하고 있었던 Item 정보를 InventoryTable에서 읽어온다.
			CDBConnection* DBCharacterInventoryItemGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerInventoryItemGet CharacterInventoryItemGet(*DBCharacterInventoryItemGetConnection);
			CharacterInventoryItemGet.InAccountDBId(Session->MyPlayer->_AccountId);
			CharacterInventoryItemGet.InPlayerDBId(Session->MyPlayer->_GameObjectInfo.ObjectId);

			int16 ItemType;
			int16 ItemConsumableType;
			WCHAR ItemName[20] = { 0 };
			int16 ItemCount;
			int8 SlotIndex;
			bool IsEquipped;
			WCHAR ItemThumbnailImagePath[100] = { 0 };

			CharacterInventoryItemGet.OutItemType(ItemType);
			CharacterInventoryItemGet.OutItemConsumableType(ItemConsumableType);
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
				ItemInfo.ItemConsumableType = (en_ConsumableType)ItemConsumableType;
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
				case en_ItemType::ITEM_TYPE_SKILL_BOOK:
					NewItem = (CItem*)(G_ObjectManager->ObjectCreate(en_GameObjectType::SKILL_BOOK));
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
					Session->MyPlayer->_Inventory.AddItem(ItemInfo);

					// 클라에게 아이템 정보를 보내준다.
					CMessage* ResItemToInventoryPacket = MakePacketResItemToInventory(Session->MyPlayer->_GameObjectInfo.ObjectId, ItemInfo, ItemInfo.ItemCount, false);
					SendPacket(Session->SessionId, ResItemToInventoryPacket);
					ResItemToInventoryPacket->Free();
				}
			}

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterInventoryItemGetConnection);
#pragma endregion
#pragma region 스킬 정보 읽어오기
			// 캐릭터가 소유하고 있는 스킬 정보를 DB로부터 읽어온다.
			CDBConnection* DBCharacterSkillGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerSkillGet CharacterSkillGet(*DBCharacterSkillGetConnection);
			CharacterSkillGet.InAccountDBId(Session->MyPlayer->_AccountId);
			CharacterSkillGet.InPlayerDBId(Session->MyPlayer->_GameObjectInfo.ObjectId);

			bool IsQuickSlotUse;
			int16 SkillType;
			int8 SkillLevel;
			WCHAR SkillName[20] = { 0 };
			int32 SkillCoolTime;
			WCHAR SkillThumbnailImagePath[100] = { 0 };

			CharacterSkillGet.OutIsQuickSlotUse(IsQuickSlotUse);
			CharacterSkillGet.OutSkillType(SkillType);
			CharacterSkillGet.OutSkillLevel(SkillLevel);
			CharacterSkillGet.OutSkillName(SkillName);
			CharacterSkillGet.OutSkillCoolTime(SkillCoolTime);
			CharacterSkillGet.OutSkillThumbnailImagePath(SkillThumbnailImagePath);

			CharacterSkillGet.Execute();

			while (CharacterSkillGet.Fetch())
			{
				st_SkillInfo SkillInfo;
				SkillInfo._IsQuickSlotUse = IsQuickSlotUse;
				SkillInfo._SkillType = (en_SkillType)SkillType;
				SkillInfo._SkillLevel = SkillLevel;
				SkillInfo._SkillName = SkillName;
				SkillInfo._SkillCoolTime = SkillCoolTime;
				SkillInfo._SkillImagePath = SkillThumbnailImagePath;

				Session->MyPlayer->_SkillBox.AddSkill(SkillInfo);

				// 클라가 소유하고 있는 스킬 정보를 보내준다.
				CMessage* ResSkillToSkillBoxPacket = MakePacketResSkillToSkillBox(Session->MyPlayer->_GameObjectInfo.ObjectId, SkillInfo);
				SendPacket(Session->SessionId, ResSkillToSkillBoxPacket);
				ResSkillToSkillBoxPacket->Free();				
			}

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterSkillGetConnection);
#pragma endregion
		} while (0);		
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBQuickSlotBarSlotSave(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		do
		{
			InterlockedDecrement64(&Session->IOBlock->IOCount);

			int64 AccountId;
			*Message >> AccountId;

			int64 PlayerId;
			*Message >> PlayerId;

			int8 QuickSlotBarIndex;
			*Message >> QuickSlotBarIndex;

			int8 QuickSlotBarSlotIndex;
			*Message >> QuickSlotBarSlotIndex;

			int8 QuickSlotKeyLen;
			*Message >> QuickSlotKeyLen;

			wstring QuickSlotKey;
			Message->GetData(QuickSlotKey, QuickSlotKeyLen);

			int16 SkillType;
			*Message >> SkillType;

			int8 SkillLevel;
			*Message >> SkillLevel;

			int8 SkillNameLen;
			*Message >> SkillNameLen;

			wstring SkillName;
			Message->GetData(SkillName, SkillNameLen);

			int32 SkillCoolTime;
			*Message >> SkillCoolTime;						

			int8 SkillImagePathLen;
			*Message >> SkillImagePathLen;

			wstring SkillImagePath;
			Message->GetData(SkillImagePath, SkillImagePathLen);

			st_SkillInfo* FindSkill = Session->MyPlayer->_SkillBox.FindSkill((en_SkillType)SkillType);
			// 캐릭터가 해당 스킬을 가지고 있는지 확인
			if (FindSkill != nullptr)
			{
				FindSkill->_IsQuickSlotUse = true;

				st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo;
				QuickSlotBarSlotInfo.AccountDBId = AccountId;
				QuickSlotBarSlotInfo.PlayerDBId = PlayerId;
				QuickSlotBarSlotInfo.QuickSlotBarIndex = QuickSlotBarIndex;
				QuickSlotBarSlotInfo.QuickSlotBarSlotIndex = QuickSlotBarSlotIndex;
				QuickSlotBarSlotInfo.QuickSlotKey = QuickSlotKey;
				QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillType = (en_SkillType)SkillType;
				QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillLevel = SkillLevel;
				QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillName = SkillName;
				QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillCoolTime = SkillCoolTime;
				QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillImagePath = SkillImagePath;

				Session->MyPlayer->_QuickSlotManager.UpdateQuickSlotBar(QuickSlotBarSlotInfo);

				// DB에 퀵슬롯 정보 저장
				CDBConnection* DBQuickSlotUpdateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerQuickSlotBarSlotUpdate QuickSlotUpdate(*DBQuickSlotUpdateConnection);
				QuickSlotUpdate.InAccountDBId(AccountId);
				QuickSlotUpdate.InPlayerDBId(PlayerId);
				QuickSlotUpdate.InQuickSlotBarIndex(QuickSlotBarIndex);
				QuickSlotUpdate.InQuickSlotBarSlotIndex(QuickSlotBarSlotIndex);
				QuickSlotUpdate.InQuickSlotKey(QuickSlotKey);
				QuickSlotUpdate.InSkillType(SkillType);
				QuickSlotUpdate.InSkillLevel(SkillLevel);
				QuickSlotUpdate.InSkillName(SkillName);
				QuickSlotUpdate.InSkillCoolTime(SkillCoolTime);
				QuickSlotUpdate.InSkillThumbnailImagePath(SkillImagePath);

				QuickSlotUpdate.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotUpdateConnection);		

				CMessage* ResQuickSlotUpdateMessage = MakePacketResQuickSlotBarSlotUpdate(QuickSlotBarSlotInfo);
				SendPacket(Session->SessionId, ResQuickSlotUpdateMessage);
				ResQuickSlotUpdateMessage->Free();
			}
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcTimerAttackEnd(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		CPlayer* MyPlayer = Session->MyPlayer;
		
		MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		MyPlayer->_SkillType = en_SkillType::SKILL_TYPE_NONE;

		CMessage* ResObjectStateMessage = MakePacketResObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
		SendPacketAroundSector(MyPlayer->GetCellPosition(), ResObjectStateMessage);
		ResObjectStateMessage->Free();
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcTimerSpellEnd(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		CPlayer* MyPlayer = Session->MyPlayer;
				
		wstring MagicSystemString;

		wchar_t SpellMessage[64] = L"0";

		// 크리티컬 판단 준비
		random_device RD;
		mt19937 Gen(RD());

		uniform_int_distribution<int> CriticalPointCreate(0, 100);

		bool IsCritical = false;

		int16 CriticalPoint = CriticalPointCreate(Gen);
		// 크리티컬 판정
		// 내 캐릭터의 크리티컬 포인트보다 값이 작으면 크리티컬로 판단한다.				
		if (CriticalPoint < MyPlayer->_GameObjectInfo.ObjectStatInfo.CriticalPoint)
		{
			IsCritical = true;
		}

		int32 FinalDamage = 0;

		switch (MyPlayer->_SkillType)
		{
		case en_SkillType::SKILL_SHAMNA_FLAME_HARPOON:
		{
			uniform_int_distribution<int> DamageChoiceRandom(MyPlayer->_GameObjectInfo.ObjectStatInfo.MinAttackDamage, MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxAttackDamage);
			int32 ChoiceDamage = DamageChoiceRandom(Gen);
			FinalDamage = 40;// IsCritical ? ChoiceDamage * 2 : ChoiceDamage

			// 데미지 처리
			MyPlayer->GetTarget()->OnDamaged(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s가 불꽃작살을 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

			MagicSystemString = SpellMessage;
		}
		break;
		case en_SkillType::SKILL_SHAMAN_HEALING_LIGHT:
		{
			FinalDamage = 100;
			MyPlayer->GetTarget()->OnHeal(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s가 치유의빛을 사용해 %s를 %d만큼 회복했습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), 10);
			MagicSystemString = SpellMessage;
		}
		break;
		case en_SkillType::SKILL_SHAMAN_HEALING_WIND:
		{
			FinalDamage = 200;
			MyPlayer->GetTarget()->OnHeal(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s가 치유의바람을 사용해 %s를 %d만큼 회복했습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), 10);
			MagicSystemString = SpellMessage;
		}
		break;
		default:
			break;
		}

		// 공격 응답
		CMessage* ResAttackMagicPacket = G_ObjectManager->GameServer->MakePacketResAttack(
			MyPlayer->_GameObjectInfo.ObjectId,
			MyPlayer->GetTarget()->_GameObjectInfo.ObjectId,
			MyPlayer->_SkillType,
			FinalDamage,
			false);
		G_ObjectManager->GameServer->SendPacketAroundSector(MyPlayer->GetTarget()->GetCellPosition(), ResAttackMagicPacket);
		ResAttackMagicPacket->Free();

		// Idle로 상태 변경 후 주위섹터에 전송
		MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		CMessage* ResObjectStateChangePacket = MakePacketResObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
		SendPacketAroundSector(MyPlayer->GetCellPosition(), ResObjectStateChangePacket);
		ResObjectStateChangePacket->Free();

		// 시스템 메세지 전송
		CMessage* ResAttackMagicSystemMessagePacket = MakePacketResChattingMessage(MyPlayer->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, st_Color::White(), MagicSystemString);
		SendPacketAroundSector(MyPlayer->GetCellPosition(), ResAttackMagicSystemMessagePacket);
		ResAttackMagicSystemMessagePacket->Free();

		// HP 변경 전송
		CMessage* ResChangeHPPacket = MakePacketResChangeHP(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, MyPlayer->GetTarget()->_GameObjectInfo.ObjectStatInfo.HP, MyPlayer->GetTarget()->_GameObjectInfo.ObjectStatInfo.MaxHP);
		SendPacketAroundSector(MyPlayer->GetTarget()->GetCellPosition(), ResChangeHPPacket);
		ResChangeHPPacket->Free();

		// 스펠창 끝
		CMessage* ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId, false);
		SendPacketAroundSector(MyPlayer->GetCellPosition(), ResMagicPacket);
		ResMagicPacket->Free();
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
			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo.ObjectStatInfo.MinAttackDamage;
			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo.ObjectStatInfo.MaxAttackDamage;
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
	*ResCreateCharacter << CreateCharacterObjectInfo.ObjectStatInfo.MinAttackDamage;
	*ResCreateCharacter << CreateCharacterObjectInfo.ObjectStatInfo.MaxAttackDamage;
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
	*ResEnterGamePacket << ObjectInfo.ObjectStatInfo.MinAttackDamage;
	*ResEnterGamePacket << ObjectInfo.ObjectStatInfo.MaxAttackDamage;
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
CMessage* CGameServer::MakePacketResMousePositionObjectInfo(int64 AccountId, int64 PreviousChoiceObjectId, st_GameObjectInfo FindObjectInfo)
{
	CMessage* ResMousePositionObjectInfoPacket = CMessage::Alloc();
	if (ResMousePositionObjectInfoPacket == nullptr)
	{
		return nullptr;
	}

	ResMousePositionObjectInfoPacket->Clear();

	*ResMousePositionObjectInfoPacket << (WORD)en_PACKET_S2C_MOUSE_POSITION_OBJECT_INFO;
	*ResMousePositionObjectInfoPacket << AccountId;
	*ResMousePositionObjectInfoPacket << PreviousChoiceObjectId;

	// ObjectId
	*ResMousePositionObjectInfoPacket << FindObjectInfo.ObjectId;

	// EnterPlayerName
	int8 ObjectNameLen = (int8)(FindObjectInfo.ObjectName.length() * 2);
	*ResMousePositionObjectInfoPacket << ObjectNameLen;
	ResMousePositionObjectInfoPacket->InsertData(FindObjectInfo.ObjectName.c_str(), ObjectNameLen);

	// st_PositionInfo
	*ResMousePositionObjectInfoPacket << (int8)FindObjectInfo.ObjectPositionInfo.State;
	*ResMousePositionObjectInfoPacket << FindObjectInfo.ObjectPositionInfo.PositionX;
	*ResMousePositionObjectInfoPacket << FindObjectInfo.ObjectPositionInfo.PositionY;
	*ResMousePositionObjectInfoPacket << (int8)FindObjectInfo.ObjectPositionInfo.MoveDir;

	// st_StatInfo
	*ResMousePositionObjectInfoPacket << FindObjectInfo.ObjectStatInfo.Level;
	*ResMousePositionObjectInfoPacket << FindObjectInfo.ObjectStatInfo.HP;
	*ResMousePositionObjectInfoPacket << FindObjectInfo.ObjectStatInfo.MaxHP;
	*ResMousePositionObjectInfoPacket << FindObjectInfo.ObjectStatInfo.MinAttackDamage;
	*ResMousePositionObjectInfoPacket << FindObjectInfo.ObjectStatInfo.MaxAttackDamage;
	*ResMousePositionObjectInfoPacket << FindObjectInfo.ObjectStatInfo.CriticalPoint;
	*ResMousePositionObjectInfoPacket << FindObjectInfo.ObjectStatInfo.Speed;

	// ObjectType
	*ResMousePositionObjectInfoPacket << (int16)FindObjectInfo.ObjectType;

	// OwnerObjectId, OwnerObjectType
	*ResMousePositionObjectInfoPacket << FindObjectInfo.OwnerObjectId;
	*ResMousePositionObjectInfoPacket << (int16)FindObjectInfo.OwnerObjectType;

	*ResMousePositionObjectInfoPacket << (int8)FindObjectInfo.PlayerSlotIndex;

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
	*ResItemSwapMessage << SwapAItemInfo.IsQuickSlotUse;
	*ResItemSwapMessage << (int16)SwapAItemInfo.ItemType;
	*ResItemSwapMessage << (int16)SwapAItemInfo.ItemConsumableType;
	*ResItemSwapMessage << SwapAItemInfo.ItemCount;
	*ResItemSwapMessage << SwapAItemInfo.SlotIndex;
	*ResItemSwapMessage << SwapAItemInfo.IsEquipped;

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
	*ResItemSwapMessage << SwapBItemInfo.IsQuickSlotUse;
	*ResItemSwapMessage << (int16)SwapBItemInfo.ItemType;
	*ResItemSwapMessage << (int16)SwapBItemInfo.ItemConsumableType;
	*ResItemSwapMessage << SwapBItemInfo.ItemCount;
	*ResItemSwapMessage << SwapBItemInfo.SlotIndex;
	*ResItemSwapMessage << SwapBItemInfo.IsEquipped;

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

CMessage* CGameServer::MakePacketResQuickSlotBarSlotUpdate(st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo)
{
	CMessage* ResQuickSlotBarSlotMessage = CMessage::Alloc();
	if (ResQuickSlotBarSlotMessage == nullptr)
	{
		return nullptr;
	}

	ResQuickSlotBarSlotMessage->Clear();

	*ResQuickSlotBarSlotMessage << (int16)en_PACKET_S2C_QUICKSLOT_SAVE;
	*ResQuickSlotBarSlotMessage << QuickSlotBarSlotInfo.AccountDBId;
	*ResQuickSlotBarSlotMessage << QuickSlotBarSlotInfo.PlayerDBId;
	*ResQuickSlotBarSlotMessage << QuickSlotBarSlotInfo.QuickSlotBarIndex;
	*ResQuickSlotBarSlotMessage << QuickSlotBarSlotInfo.QuickSlotBarSlotIndex;

	int8 QuickSlotKeyLen = (int8)(QuickSlotBarSlotInfo.QuickSlotKey.length() * 2);
	*ResQuickSlotBarSlotMessage << QuickSlotKeyLen;
	ResQuickSlotBarSlotMessage->InsertData(QuickSlotBarSlotInfo.QuickSlotKey.c_str(), QuickSlotKeyLen);

	// 스킬타입
	*ResQuickSlotBarSlotMessage << (int16)QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillType;
	// 스킬레벨
	*ResQuickSlotBarSlotMessage << QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillLevel;

	// 스킬이름
	int8 SkillNameLen = (int8)(QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillName.length() * 2);
	*ResQuickSlotBarSlotMessage << SkillNameLen;
	ResQuickSlotBarSlotMessage->InsertData(QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillName.c_str(), SkillNameLen);

	// 스킬 쿨타임
	*ResQuickSlotBarSlotMessage << QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillCoolTime;

	int8 SkillImagePathLen = (int8)(QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillImagePath.length() * 2);
	*ResQuickSlotBarSlotMessage << SkillImagePathLen;
	ResQuickSlotBarSlotMessage->InsertData(QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillImagePath.c_str(), SkillImagePathLen);

	return ResQuickSlotBarSlotMessage;
}

CMessage* CGameServer::MakePacketQuickSlotCreate(int8 QuickSlotBarSize, int8 QuickSlotBarSlotSize, vector<st_QuickSlotBarSlotInfo> QuickslotBarSlotInfos)
{
	CMessage* ResQuickSlotCreateMessage = CMessage::Alloc();
	if (ResQuickSlotCreateMessage == nullptr)
	{
		return nullptr;
	}

	ResQuickSlotCreateMessage->Clear();

	*ResQuickSlotCreateMessage << (short)en_PACKET_S2C_QUICKSLOT_CREATE;
	*ResQuickSlotCreateMessage << QuickSlotBarSize;
	*ResQuickSlotCreateMessage << QuickSlotBarSlotSize;

	*ResQuickSlotCreateMessage << (byte)QuickslotBarSlotInfos.size();

	for (st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo : QuickslotBarSlotInfos)
	{
		*ResQuickSlotCreateMessage << QuickSlotBarSlotInfo.AccountDBId;
		*ResQuickSlotCreateMessage << QuickSlotBarSlotInfo.PlayerDBId;
		*ResQuickSlotCreateMessage << QuickSlotBarSlotInfo.QuickSlotBarIndex;
		*ResQuickSlotCreateMessage << QuickSlotBarSlotInfo.QuickSlotBarSlotIndex;
				
		int8 QuickSlotKeyLen = (int8)(QuickSlotBarSlotInfo.QuickSlotKey.length() * 2);
		*ResQuickSlotCreateMessage << QuickSlotKeyLen;
		ResQuickSlotCreateMessage->InsertData(QuickSlotBarSlotInfo.QuickSlotKey.c_str(), QuickSlotKeyLen);
		
		// 스킬타입
		*ResQuickSlotCreateMessage << (int16)QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillType;
		// 스킬레벨
		*ResQuickSlotCreateMessage << QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillLevel;

		// 스킬이름
		int8 SkillNameLen = (int8)(QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillName.length() * 2);
		*ResQuickSlotCreateMessage << SkillNameLen;
		ResQuickSlotCreateMessage->InsertData(QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillName.c_str(), SkillNameLen);

		// 스킬 쿨타임
		*ResQuickSlotCreateMessage << QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillCoolTime;

		int8 SkillImagePathLen = (int8)(QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillImagePath.length() * 2);
		*ResQuickSlotCreateMessage << SkillImagePathLen;
		ResQuickSlotCreateMessage->InsertData(QuickSlotBarSlotInfo.QuickBarSkillInfo._SkillImagePath.c_str(), SkillImagePathLen);
	}

	return ResQuickSlotCreateMessage;
}

// int64 AccountId
// int32 PlayerDBId
// char Dir
CMessage* CGameServer::MakePacketResAttack(int64 PlayerDBId, int64 TargetId, en_SkillType SkillType, int32 Damage, bool IsCritical)
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
	*ResAttackMessage << (int16)SkillType;
	*ResAttackMessage << Damage;
	*ResAttackMessage << IsCritical;

	return ResAttackMessage;
}

CMessage* CGameServer::MakePacketResMagic(int64 ObjectId, bool SpellStart, en_SkillType SkillType, float SpellTime)
{
	CMessage* ResMagicMessage = CMessage::Alloc();
	if (ResMagicMessage == nullptr)
	{
		return nullptr;
	}

	ResMagicMessage->Clear();

	*ResMagicMessage << (short)en_PACKET_S2C_MAGIC;
	*ResMagicMessage << ObjectId;
	*ResMagicMessage << SpellStart;
	*ResMagicMessage << (short)SkillType;
	*ResMagicMessage << SpellTime;

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
		*ResSpawnPacket << ObjectInfos[i].ObjectStatInfo.MinAttackDamage;
		*ResSpawnPacket << ObjectInfos[i].ObjectStatInfo.MaxAttackDamage;
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
	*ResItemToInventoryMessage << ItemInfo.IsQuickSlotUse;
	*ResItemToInventoryMessage << (int16)ItemInfo.ItemType;
	*ResItemToInventoryMessage << (int16)ItemInfo.ItemConsumableType;
	*ResItemToInventoryMessage << ItemInfo.ItemCount;
	*ResItemToInventoryMessage << ItemInfo.SlotIndex;
	*ResItemToInventoryMessage << ItemInfo.IsEquipped;

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

CMessage* CGameServer::MakePacketResSkillToSkillBox(int64 TargetObjectId, st_SkillInfo SkillInfo)
{
	CMessage* ResSkillToSkillBoxMessage = CMessage::Alloc();
	if (ResSkillToSkillBoxMessage == nullptr)
	{
		return nullptr;
	}

	ResSkillToSkillBoxMessage->Clear();

	*ResSkillToSkillBoxMessage << (int16)en_PACKET_S2C_SKILL_TO_SKILLBOX;
	*ResSkillToSkillBoxMessage << TargetObjectId;

	// 스킬타입
	*ResSkillToSkillBoxMessage << (int16)SkillInfo._SkillType;
	// 스킬레벨
	*ResSkillToSkillBoxMessage << SkillInfo._SkillLevel;
	
	// 스킬이름
	int8 SkillNameLen = (int8)(SkillInfo._SkillName.length() * 2);
	*ResSkillToSkillBoxMessage << SkillNameLen;
	ResSkillToSkillBoxMessage->InsertData(SkillInfo._SkillName.c_str(), SkillNameLen);

	// 스킬 쿨타임
	*ResSkillToSkillBoxMessage << SkillInfo._SkillCoolTime;	

	int8 SkillImagePathLen = (int8)(SkillInfo._SkillImagePath.length() * 2);
	*ResSkillToSkillBoxMessage << SkillImagePathLen;
	ResSkillToSkillBoxMessage->InsertData(SkillInfo._SkillImagePath.c_str(), SkillImagePathLen);

	return ResSkillToSkillBoxMessage;
}

void CGameServer::OnClientJoin(int64 SessionID)
{
	st_Job* ClientJoinJob = _JobMemoryPool->Alloc();
	ClientJoinJob->Type = en_JobType::AUTH_NEW_CLIENT_JOIN;
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

	NewMessageJob->Type = en_JobType::NETWORK_MESSAGE;
	NewMessageJob->SessionId = SessionID;
	NewMessageJob->Message = JobMessage;
	_GameServerNetworkThreadMessageQue.Enqueue(NewMessageJob);
	SetEvent(_NetworkThreadWakeEvent);
}

void CGameServer::OnClientLeave(st_Session* LeaveSession)
{
	st_Job* ClientLeaveJob = _JobMemoryPool->Alloc();
	ClientLeaveJob->Type = en_JobType::AUTH_DISCONNECT_CLIENT;
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

void CGameServer::SendPacketAroundSector(st_Session* Session, CMessage* Message, bool SendMe)
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