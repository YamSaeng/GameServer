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
#include "GameServerMessage.h"
#include <process.h>
#include <atlbase.h>

CGameServer::CGameServer()
{
	//timeBeginPeriod(1);
	_AuthThread = nullptr;
	_DataBaseThread = nullptr;
	_TimerJobThread = nullptr;
	_LogicThread = nullptr;

	// 논시그널 상태 자동리셋
	_AuthThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_DataBaseWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_TimerThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	_AuthThreadEnd = false;
	_NetworkThreadEnd = false;
	_DataBaseThreadEnd = false;
	_LogicThreadEnd = false;
	_TimerJobThreadEnd = false;

	// 타이머 잡 전용 SWRLock 초기화
	InitializeSRWLock(&_TimerJobLock);

	// 잡 메모리풀 생성
	_JobMemoryPool = new CMemoryPoolTLS<st_Job>();
	// 타이머 잡 메모리풀 생성
	_TimerJobMemoryPool = new CMemoryPoolTLS<st_TimerJob>();

	// 타이머 우선순위 큐 생성
	_TimerHeapJob = new CHeap<int64, st_TimerJob*>(2000);

	_AuthThreadWakeCount = 0;
	_AuthThreadTPS = 0;

	_NetworkThreadWakeCount = 0;
	_NetworkThreadTPS = 0;


	_TimerJobThreadWakeCount = 0;
	_TimerJobThreadTPS = 0;
}

CGameServer::~CGameServer()
{
	//timeEndPeriod(1);
}

void CGameServer::GameServerStart(const WCHAR* OpenIP, int32 Port)
{
	CNetworkLib::Start(OpenIP, Port);

	// 맵 오브젝트 스폰
	G_ObjectManager->MapObjectSpawn(1);

	G_ObjectManager->GameServer = this;

	// 인증 쓰레드 시작
	_AuthThread = (HANDLE)_beginthreadex(NULL, 0, AuthThreadProc, this, 0, NULL);	
	// 데이터베이스 쓰레드 시작
	_DataBaseThread = (HANDLE)_beginthreadex(NULL, 0, DataBaseThreadProc, this, 0, NULL);
	// 타이머 잡 쓰레드 시작
	_TimerJobThread = (HANDLE)_beginthreadex(NULL, 0, TimerJobThreadProc, this, 0, NULL);
	// 로직 쓰레드 시작
	_LogicThread = (HANDLE)_beginthreadex(NULL, 0, LogicThreadProc, this, 0, NULL);	

	CloseHandle(_AuthThread);
	CloseHandle(_DataBaseThread);
	CloseHandle(_TimerJobThread);
	CloseHandle(_LogicThread);	
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
				// 접속한 세션 대상으로 기본 정보 셋팅
			case en_JobType::AUTH_NEW_CLIENT_JOIN:
				Instance->CreateNewClient(Job->SessionId);
				break;
				// 접속 종료한 세션 대상으로 콘텐츠 정리
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
			case en_JobType::DATA_BASE_LOOTING_ITEM_INVENTORY_SAVE:
				Instance->PacketProcReqDBLootingItemToInventorySave(Job->SessionId, Job->Message);
				break;
			case en_JobType::DATA_BASE_CRAFTING_ITEM_INVENTORY_SAVE:
				Instance->PacketProcReqDBCraftingItemToInventorySave(Job->SessionId, Job->Message);
				break;
			case en_JobType::DATA_BASE_ITEM_SWAP:
				Instance->PacketProcReqDBItemSwap(Job->SessionId, Job->Message);
				break;
			case en_JobType::DATA_BASE_ITEM_USE:
				Instance->PacketProcReqDBItemUpdate(Job->SessionId, Job->Message);
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
			case en_JobType::DATA_BASE_QUICK_SWAP:
				Instance->PacketProcReqDBQuickSlotSwap(Job->SessionId, Job->Message);
				break;
			case en_JobType::DATA_BASE_QUICK_INIT:
				Instance->PacketProcReqDBQuickSlotInit(Job->SessionId, Job->Message);
				break;
			case en_JobType::DATA_BASE_LEAVE_PLAYER_INFO_SAVE:
				Instance->PacketProcReqDBLeavePlayerInfoSave(Job->SessionId, Job->Message);
				break;
			default:
				Instance->Disconnect(Job->SessionId);
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
		WaitForSingleObject(Instance->_TimerThreadWakeEvent, INFINITE);

		Instance->_TimerJobThreadWakeCount++;

		while (Instance->_TimerHeapJob->GetUseSize() != 0)
		{
			// 맨 위 데이터의 시간을 확인한다.
			st_TimerJob* TimerJob = Instance->_TimerHeapJob->Peek();

			// 현재 시간을 구한다.
			int64 NowTick = GetTickCount64();

			// TimerJob에 기록되어 있는 시간 보다 현재 시간이 클 경우
			// 즉, 시간이 경과 되었으면
			if (TimerJob->TimerJobExecTick <= NowTick)
			{
				// TimerJob을 뽑고
				AcquireSRWLockExclusive(&Instance->_TimerJobLock);
				st_TimerJob* TimerJob = Instance->_TimerHeapJob->PopHeap();
				ReleaseSRWLockExclusive(&Instance->_TimerJobLock);

				// 취소가 되지 않은 TimerJob을 대상으로 처리
				if (TimerJob->TimerJobCancel != true)
				{
					// Type에 따라 실행한다.
					switch (TimerJob->TimerJobType)
					{
					case en_TimerJobType::TIMER_MELEE_ATTACK_END:
						Instance->PacketProcTimerAttackEnd(TimerJob->SessionId, TimerJob->TimerJobMessage);
						break;
					case en_TimerJobType::TIMER_SPELL_END:
						Instance->PacketProcTimerSpellEnd(TimerJob->SessionId, TimerJob->TimerJobMessage);
						break;
					case en_TimerJobType::TIMER_SKILL_COOLTIME_END:
						Instance->PacketProcTimerCastingTimeEnd(TimerJob->SessionId, TimerJob->TimerJobMessage);
						break;
					case en_TimerJobType::TIMER_OBJECT_SPAWN:
						Instance->PacketProcTimerObjectSpawn(TimerJob->TimerJobMessage);
						break;
					case en_TimerJobType::TIMER_OBJECT_STATE_CHANGE:
						Instance->PacketProcTimerObjectStateChange(TimerJob->SessionId, TimerJob->TimerJobMessage);
						break;
					case en_TimerJobType::TIMER_OBJECT_DOT:
						Instance->PacketProcTimerDot(TimerJob->SessionId, TimerJob->TimerJobMessage);
						break;
					case en_TimerJobType::TIMER_PING:
						Instance->PacketProcTimerPing(TimerJob->SessionId);
						break;
					default:
						Instance->Disconnect(TimerJob->SessionId);
						break;
					}
				}				

				Instance->_TimerJobThreadTPS++;
				Instance->_TimerJobMemoryPool->Free(TimerJob);
			}
		}

	}
	return 0;
}

unsigned __stdcall CGameServer::LogicThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_LogicThreadEnd)
	{
		G_ChannelManager->Update();
	}

	return 0;
}

void CGameServer::PlayerLevelUpSkillCreate(int64& AccountId, st_GameObjectInfo& NewCharacterInfo, int8& CharacterCreateSlotIndex)
{	
	// 클래스 공격 스킬 생성
	switch (NewCharacterInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	{
		auto FindWarriorStatus = G_Datamanager->_WarriorStatus.find(NewCharacterInfo.ObjectStatInfo.Level);
		st_ObjectStatusData* WarriorStatusData = (*FindWarriorStatus).second;

		if (WarriorStatusData != nullptr)
		{
			for (st_SkillData NewSkillData : WarriorStatusData->LevelSkills)
			{
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);
				
				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = NewSkillData.SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = NewSkillData.SkillMediumCategory;
				NewSkillInfo.SkillType = NewSkillData.SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 DefaultAttackSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 DefaultAttackSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 DefaultAttackSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(DefaultAttackSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(DefaultAttackSkillMediumCategory);
				SkillToSkillBox.InSkillType(DefaultAttackSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}
		else
		{
			CRASH("전사 스킬 생성 데이터를 찾지 못함");
		}		
	}
	break;
	case en_GameObjectType::OBJECT_MAGIC_PLAYER:
	{
		auto FindShmanStatus = G_Datamanager->_ShamanStatus.find(NewCharacterInfo.ObjectStatInfo.Level);
		st_ObjectStatusData* ShamanStatusData = (*FindShmanStatus).second;

		if (ShamanStatusData != nullptr)
		{
			for (st_SkillData NewSkillData : ShamanStatusData->LevelSkills)
			{
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = NewSkillData.SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = NewSkillData.SkillMediumCategory;
				NewSkillInfo.SkillType = NewSkillData.SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 DefaultAttackSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 DefaultAttackSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 DefaultAttackSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(DefaultAttackSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(DefaultAttackSkillMediumCategory);
				SkillToSkillBox.InSkillType(DefaultAttackSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}
		else
		{
			CRASH("주술사 스킬 생성 데이터를 찾지 못함");
		}
	}
	break;
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	{
		auto FindTaioistStatus = G_Datamanager->_TaioistStatus.find(NewCharacterInfo.ObjectStatInfo.Level);
		st_ObjectStatusData* TaioistStatusData = (*FindTaioistStatus).second;

		if (TaioistStatusData != nullptr)
		{
			for (st_SkillData NewSkillData : TaioistStatusData->LevelSkills)
			{
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = NewSkillData.SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = NewSkillData.SkillMediumCategory;
				NewSkillInfo.SkillType = NewSkillData.SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 DefaultAttackSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 DefaultAttackSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 DefaultAttackSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(DefaultAttackSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(DefaultAttackSkillMediumCategory);
				SkillToSkillBox.InSkillType(DefaultAttackSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}
		else
		{
			CRASH("도사 스킬 생성 데이터를 찾지 못함");
		}
	}	
	break;
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	{
		auto FindThiefStatus = G_Datamanager->_ThiefStatus.find(NewCharacterInfo.ObjectStatInfo.Level);
		st_ObjectStatusData* ThiefStatusData = (*FindThiefStatus).second;

		if (ThiefStatusData != nullptr)
		{
			for (st_SkillData NewSkillData : ThiefStatusData->LevelSkills)
			{
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = NewSkillData.SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = NewSkillData.SkillMediumCategory;
				NewSkillInfo.SkillType = NewSkillData.SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 DefaultAttackSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 DefaultAttackSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 DefaultAttackSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(DefaultAttackSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(DefaultAttackSkillMediumCategory);
				SkillToSkillBox.InSkillType(DefaultAttackSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}
		else
		{
			CRASH("도적 스킬 생성 데이터를 찾지 못함");
		}
	}
	break;
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	{
		auto FindArcherStatus = G_Datamanager->_ArcherStatus.find(NewCharacterInfo.ObjectStatInfo.Level);
		st_ObjectStatusData* ArcherStatusData = (*FindArcherStatus).second;

		if (ArcherStatusData != nullptr)
		{
			for (st_SkillData NewSkillData : ArcherStatusData->LevelSkills)
			{
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = NewSkillData.SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = NewSkillData.SkillMediumCategory;
				NewSkillInfo.SkillType = NewSkillData.SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 DefaultAttackSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 DefaultAttackSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 DefaultAttackSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(DefaultAttackSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(DefaultAttackSkillMediumCategory);
				SkillToSkillBox.InSkillType(DefaultAttackSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}
		else
		{
			CRASH("궁사 스킬 생성 데이터를 찾지 못함");
		}
	}
	break;
	default:
		break;
	}	
}

void CGameServer::CreateNewClient(int64 SessionId)
{
	// 세션 찾기
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		// 기본 정보 셋팅
		Session->AccountId = 0;
		Session->IsLogin = false;
		Session->Token = 0;
		Session->PingPacketTime = GetTickCount64();

		for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
		{				
			// 플레이어 인덱스 할당
			G_ObjectManager->_PlayersArrayIndexs.Pop(&Session->MyPlayerIndexes[i]);			
		}

		Session->MyPlayerIndex = -1;

		// 게임 서버 접속 성공을 클라에게 알려준다.
		CMessage* ResClientConnectedMessage = MakePacketResClientConnected();
		SendPacket(Session->SessionId, ResClientConnectedMessage);
		ResClientConnectedMessage->Free();

		// 핑 전송 시작		
		//PingTimerCreate(Session);
	}

	ReturnSession(Session);
}

void CGameServer::DeleteClient(st_Session* Session)
{
	if (Session == nullptr)
	{
		return;
	}

	CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

	if (MyPlayer != nullptr)
	{
		// 마지막 위치 기록
		st_Job* DBLeavePlayerInfoSaveJob = _JobMemoryPool->Alloc();
		DBLeavePlayerInfoSaveJob->Type = en_JobType::DATA_BASE_LEAVE_PLAYER_INFO_SAVE;
		DBLeavePlayerInfoSaveJob->SessionId = Session->SessionId;

		CGameServerMessage* DBLeavePlayerInfoSaveMessage = CGameServerMessage::GameServerMessageAlloc();
		if (DBLeavePlayerInfoSaveMessage == nullptr)
		{
			return;
		}

		DBLeavePlayerInfoSaveMessage->Clear();

		*DBLeavePlayerInfoSaveMessage << Session->AccountId;
		*DBLeavePlayerInfoSaveMessage << MyPlayer->_GameObjectInfo.ObjectId;
		*DBLeavePlayerInfoSaveMessage << MyPlayer->_GameObjectInfo.ObjectPositionInfo;
		*DBLeavePlayerInfoSaveMessage << MyPlayer->_GameObjectInfo.ObjectStatInfo;
		*DBLeavePlayerInfoSaveMessage << MyPlayer->_Experience.CurrentExperience;
		*DBLeavePlayerInfoSaveMessage << MyPlayer->_Experience.RequireExperience;
		*DBLeavePlayerInfoSaveMessage << MyPlayer->_Experience.TotalExperience;

		DBLeavePlayerInfoSaveJob->Message = DBLeavePlayerInfoSaveMessage;

		_GameServerDataBaseThreadMessageQue.Enqueue(DBLeavePlayerInfoSaveJob);
		SetEvent(_DataBaseWakeEvent);

		CChannel* Channel = G_ChannelManager->Find(1);
		Channel->LeaveChannel(MyPlayer);

		vector<int64> DeSpawnObjectIds;
		DeSpawnObjectIds.push_back(MyPlayer->_GameObjectInfo.ObjectId);

		CMessage* ResLeaveGame = MakePacketResObjectDeSpawn(1, DeSpawnObjectIds);
		SendPacketFieldOfView(Session, ResLeaveGame, true);		
		ResLeaveGame->Free();

		for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
		{
			CPlayer* SessionPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndexes[i]];
			if (SessionPlayer != nullptr)
			{
				SessionPlayer->_NetworkState = en_ObjectNetworkState::LEAVE;				
				// 오브젝트 반납 ( = 인덱스 반납 )
				G_ObjectManager->ObjectLeaveGame(SessionPlayer, Session->MyPlayerIndexes[i], 1);
			}			
		}
	}

	Session->MyPlayerIndex = -1;
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
		PacketProcReqMelee(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_MAGIC:
		PacketProcReqMagic(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_MAGIC_CANCEL:
		PacketProcReqMagicCancel(SessionId, Message);
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
	case en_PACKET_TYPE::en_PACKET_C2S_LOOTING:
		PacketProcReqItemLooting(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_ITEM_SWAP:
		PacketProcReqItemSwap(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_QUICKSLOT_SAVE:
		PacketProcReqQuickSlotSave(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_QUICKSLOT_SWAP:
		PacketProcReqQuickSlotSwap(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_QUICKSLOT_EMPTY:
		PacketProcReqQuickSlotInit(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_CRAFTING_CONFIRM:
		PacketProcReqCraftingConfirm(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_INVENTORY_ITEM_USE:
		PacketProcReqItemUse(SessionId, Message);
		break;
	case en_PACKET_TYPE::en_PACKET_CS_GAME_REQ_HEARTBEAT:		
		break;
	case en_PACKET_TYPE::en_PACKET_C2S_PONG:
		PacketProcReqPong(SessionId, Message);
		break;
	default:
		Disconnect(SessionId);
		break;
	}

	//Message->Free();
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

	do
	{
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
					break;
				}
			}

			Session->AccountId = AccountId;

			int8 IdLen;
			*Message >> IdLen;

			WCHAR ClientId[20];
			memset(ClientId, 0, sizeof(WCHAR) * 20);
			Message->GetData(ClientId, IdLen);

			Session->LoginId = ClientId;

			bool IsDummy;
			*Message >> IsDummy;

			if (IsDummy != true)
			{
				// 토큰 셋팅
				*Message >> Token;
				Session->Token = Token;
			}			

			Session->IsDummy = IsDummy;

			if (Session->AccountId != 0 && Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
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
	} while (0);	

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

		CGameServerMessage* DBReqChatacerCreateMessage = CGameServerMessage::GameServerMessageAlloc();
		DBReqChatacerCreateMessage->Clear();

		*DBReqChatacerCreateMessage << ReqGameObjectType;
		*DBReqChatacerCreateMessage << CharacterCreateSlotIndex;

		InterlockedIncrement64(&Session->IOBlock->IOCount);

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
				if (G_ObjectManager->_PlayersArray[Session->MyPlayerIndexes[i]] != nullptr && 
					G_ObjectManager->_PlayersArray[Session->MyPlayerIndexes[i]]->_GameObjectInfo.ObjectName == EnterGameCharacterName)
				{
					FindName = true;
					Session->MyPlayerIndex = Session->MyPlayerIndexes[i];					
					G_ObjectManager->_PlayersArray[Session->MyPlayerIndex]->_NetworkState = en_ObjectNetworkState::LIVE;					
					break;
				}
			}

			if (FindName == false)
			{
				CRASH("클라가 가지고 있는 캐릭 중 패킷으로 받은 캐릭터가 없음");
				break;
			}

			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// 클라가 선택한 오브젝트를 게임에 입장 시킨다. ( 채널, 섹터 입장 )
			G_ObjectManager->ObjectEnterGame(MyPlayer, 1);

			// 나한테 나 생성하라고 알려줌
			CMessage* ResEnterGamePacket = MakePacketResEnterGame(MyPlayer->_GameObjectInfo);
			SendPacket(Session->SessionId, ResEnterGamePacket);
			ResEnterGamePacket->Free();			

			// 캐릭터의 상태를 10초 후에 IDLE 상태로 전환
			ObjectStateChangeTimerJobCreate(MyPlayer, en_CreatureState::IDLE, 10000);

			vector<st_GameObjectInfo> SpawnObjectInfo;
			SpawnObjectInfo.push_back(MyPlayer->_GameObjectInfo);

			// 다른 플레이어들한테 나를 생성하라고 알려줌
			CMessage* ResSpawnPacket = MakePacketResObjectSpawn(1, SpawnObjectInfo);
			SendPacketFieldOfView(Session, ResSpawnPacket);			
			ResSpawnPacket->Free();

			SpawnObjectInfo.clear();

			if (Session->IsDummy == false)
			{
				// 나한테 다른 오브젝트들을 생성하라고 알려줌						
				st_GameObjectInfo* SpawnGameObjectInfos;
				// 시야 범위 안에 있는 오브젝트 가져오기
				vector<CGameObject*> FieldOfViewObjects = G_ChannelManager->Find(1)->GetFieldOfViewObjects(MyPlayer, MyPlayer->_FieldOfViewDistance);
				MyPlayer->_FieldOfViewObjects = FieldOfViewObjects;				

				if (FieldOfViewObjects.size() > 0)
				{
					SpawnGameObjectInfos = new st_GameObjectInfo[FieldOfViewObjects.size()];

					for (int32 i = 0; i < FieldOfViewObjects.size(); i++)
					{
						SpawnObjectInfo.push_back(FieldOfViewObjects[i]->_GameObjectInfo);
					}

					CMessage* ResOtherObjectSpawnPacket = MakePacketResObjectSpawn((int32)FieldOfViewObjects.size(), SpawnObjectInfo);
					SendPacket(Session->SessionId, ResOtherObjectSpawnPacket);
					ResOtherObjectSpawnPacket->Free();

					delete[] SpawnGameObjectInfos;
				}				
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

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// 클라가 조종중인 캐릭터가 있는지 확인
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종중인 캐릭터가 있으면 ObjectId가 다른지 확인
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
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
			SendPacketFieldOfView(Session, ResMyMoveOtherPacket, true);			
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
void CGameServer::PacketProcReqMelee(int64 SessionID, CMessage* Message)
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

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// 조종하고 있는 플레이어가 있는지 확인
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종하고 있는 플레이어의 ObjectId와 클라가 보낸 ObjectId가 같은지 확인
				if (MyPlayer->_GameObjectInfo.ObjectId != ObjectId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}			

			int8 QuickSlotBarIndex;
			*Message >> QuickSlotBarIndex;

			int8 QuickSlotBarSlotIndex;
			*Message >> QuickSlotBarSlotIndex;

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
			CMessage* ResSyncPositionPacket = nullptr;

			// 퀵바에 등록되지 않은 스킬을 요청했을 경우
			if ((en_SkillType)ReqSkillType == en_SkillType::SKILL_TYPE_NONE)
			{
				break;
			}

			// 요청한 스킬이 스킬창에 있는지 확인
			st_SkillInfo* FindSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)ReqSkillType);
			if (FindSkill != nullptr && FindSkill->CanSkillUse == true)
			{
				// 스킬 지정
				MyPlayer->_SkillType = (en_SkillType)ReqSkillType;
				// 공격 상태로 변경
				MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
				// 클라에게 알려줘서 공격 애니메이션 출력
				CMessage* ResObjectStateChangePacket = MakePacketResChangeObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
				SendPacketFieldOfView(Session, ResObjectStateChangePacket, true);				
				ResObjectStateChangePacket->Free();

				// 타겟 위치 확인
				switch ((en_SkillType)ReqSkillType)
				{
				case en_SkillType::SKILL_TYPE_NONE:
					break;
				case en_SkillType::SKILL_DEFAULT_ATTACK:
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
								ResSyncPositionPacket = MakePacketResSyncPosition(Target->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectPositionInfo);
								SendPacketFieldOfView(Session, ResSyncPositionPacket);								
								ResSyncPositionPacket->Free();

								//Target->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::STUN;
								//CMessage* ResObjectStateChange = MakePacketResChangeObjectState(Target->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectPositionInfo.MoveDir, Target->_GameObjectInfo.ObjectType, Target->_GameObjectInfo.ObjectPositionInfo.State);
								//SendPacketFieldOfView(Session, ResObjectStateChange, true);
								//ResObjectStateChange->Free();
							}
						}
						else
						{
							wstring ErrorNonSelectObjectString;

							WCHAR ErrorMessage[100] = { 0 };

							wsprintf(ErrorMessage, L"[%s] 대상과의 거리가 너무 멉니다. [거리 : %d ]", FindSkill->SkillName.c_str(), Distance);
							ErrorNonSelectObjectString = ErrorMessage;

							CMessage* ResErrorPacket = MakePacketError(MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_DISTANCE, ErrorNonSelectObjectString);
							SendPacket(MyPlayer->_SessionId, ResErrorPacket);
							ResErrorPacket->Free();
						}
					}
					else
					{
						wstring ErrorDistance;

						WCHAR ErrorMessage[100] = { 0 };

						wsprintf(ErrorMessage, L"[%s] 대상을 선택하고 사용해야 합니다.", FindSkill->SkillName.c_str());
						ErrorDistance = ErrorMessage;

						CMessage* ResErrorPacket = MakePacketError(MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_NON_SELECT_OBJECT, ErrorDistance);
						SendPacket(MyPlayer->_SessionId, ResErrorPacket);
						ResErrorPacket->Free();
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

								ResSyncPositionPacket = MakePacketResSyncPosition(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
								SendPacketFieldOfView(Session, ResSyncPositionPacket, true);
								ResSyncPositionPacket->Free();
							}
						}
						else
						{
							wstring ErrorDistance;

							WCHAR ErrorMessage[100] = { 0 };

							wsprintf(ErrorMessage, L"[%s] 대상과의 거리가 너무 멉니다. [거리 : %d ]", FindSkill->SkillName.c_str(), Distance);
							ErrorDistance = ErrorMessage;

							CMessage* ResErrorPacket = MakePacketError(MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_DISTANCE, ErrorDistance);
							SendPacket(MyPlayer->_SessionId, ResErrorPacket);
							ResErrorPacket->Free();
						}
					}
					else
					{
						wstring ErrorNonSelectObjectString;

						WCHAR ErrorMessage[100] = { 0 };

						wsprintf(ErrorMessage, L"[%s] 대상을 선택하고 사용해야 합니다.", FindSkill->SkillName.c_str());
						ErrorNonSelectObjectString = ErrorMessage;

						CMessage* ResErrorPacket = MakePacketError(MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_NON_SELECT_OBJECT, ErrorNonSelectObjectString);
						SendPacket(MyPlayer->_SessionId, ResErrorPacket);
						ResErrorPacket->Free();
					}
				}
				break;
				case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
				{
					vector<st_Vector2Int> TargetPositions;

					TargetPositions = MyPlayer->GetAroundCellPositions(MyPlayer->GetCellPosition(), 1);
					for (st_Vector2Int TargetPosition : TargetPositions)
					{
						CGameObject* Target = MyPlayer->_Channel->_Map->Find(TargetPosition);
						if (Target != nullptr)
						{
							Targets.push_back(Target);
						}
					}

					// 이펙트 출력
					CMessage* ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_SMASH_WAVE, 2.0f);
					SendPacketFieldOfView(Session, ResEffectPacket, true);
					ResEffectPacket->Free();
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
					if (Target->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
					{
						st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)FindSkill;

						// 크리티컬 판단
						random_device Seed;
						default_random_engine Eng(Seed());

						float CriticalPoint = MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint / 1000.0f;
						bernoulli_distribution CriticalCheck(CriticalPoint);
						bool IsCritical = CriticalCheck(Eng);

						// 데미지 판단
						mt19937 Gen(Seed());
						uniform_int_distribution<int> DamageChoiceRandom(
							MyPlayer->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage + MyPlayer->_Equipment._WeaponMinDamage + AttackSkillInfo->SkillMinDamage,
							MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage + MyPlayer->_Equipment._WeaponMaxDamage + AttackSkillInfo->SkillMaxDamage);
						int32 ChoiceDamage = DamageChoiceRandom(Gen);

						int32 CriticalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

						float DefenceRate = (float)pow(((float)(200 - Target->_GameObjectInfo.ObjectStatInfo.Defence)) / 20, 2) * 0.01f;

						int32 FinalDamage = CriticalDamage * DefenceRate;

						bool TargetIsDead = Target->OnDamaged(MyPlayer, FinalDamage);
						if (TargetIsDead == true)
						{
							CMonster* TargetMonster = (CMonster*)Target;
							if (TargetMonster != nullptr)
							{
								MyPlayer->_Experience.CurrentExperience += TargetMonster->_GetExpPoint;
								MyPlayer->_Experience.CurrentExpRatio = ((float)MyPlayer->_Experience.CurrentExperience) / MyPlayer->_Experience.RequireExperience;

								if (MyPlayer->_Experience.CurrentExpRatio >= 1.0f)
								{
									// 레벨 증가
									MyPlayer->_GameObjectInfo.ObjectStatInfo.Level += 1;

									// 해당 레벨에 
									st_ObjectStatusData NewCharacterStatus;
									st_LevelData LevelData;

									switch (MyPlayer->_GameObjectInfo.ObjectType)
									{
									case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
										{
											auto FindStatus = G_Datamanager->_WarriorStatus.find(MyPlayer->_GameObjectInfo.ObjectStatInfo.Level);
											if (FindStatus == G_Datamanager->_WarriorStatus.end())
											{
												CRASH("레벨 스테이터스 찾지 못함");
											}

											NewCharacterStatus = *(*FindStatus).second;

											MyPlayer->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
										}
										break;
									case en_GameObjectType::OBJECT_MAGIC_PLAYER:
										{
											auto FindStatus = G_Datamanager->_ShamanStatus.find(MyPlayer->_GameObjectInfo.ObjectStatInfo.Level);
											if (FindStatus == G_Datamanager->_WarriorStatus.end())
											{
												CRASH("레벨 데이터 찾지 못함");
											}

											NewCharacterStatus = *(*FindStatus).second;

											MyPlayer->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
										}
										break;
									case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
										{
											auto FindStatus = G_Datamanager->_TaioistStatus.find(MyPlayer->_GameObjectInfo.ObjectStatInfo.Level);
											if (FindStatus == G_Datamanager->_TaioistStatus.end())
											{
												CRASH("레벨 데이터 찾지 못함");
											}

											NewCharacterStatus = *(*FindStatus).second;

											MyPlayer->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
										}
										break;
									case en_GameObjectType::OBJECT_THIEF_PLAYER:
										{
											auto FindStatus = G_Datamanager->_ThiefStatus.find(MyPlayer->_GameObjectInfo.ObjectStatInfo.Level);
											if (FindStatus == G_Datamanager->_ThiefStatus.end())
											{
												CRASH("레벨 데이터 찾지 못함");
											}

											NewCharacterStatus = *(*FindStatus).second;

											MyPlayer->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
										}
										break;
									case en_GameObjectType::OBJECT_ARCHER_PLAYER:
										{
											auto FindStatus = G_Datamanager->_ArcherStatus.find(MyPlayer->_GameObjectInfo.ObjectStatInfo.Level);
											if (FindStatus == G_Datamanager->_ArcherStatus.end())
											{
												CRASH("레벨 데이터 찾지 못함");
											}

											NewCharacterStatus = *(*FindStatus).second;

											MyPlayer->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
											MyPlayer->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
										}
										break;
									default:
										break;
									}

									CGameServerMessage* ResObjectStatChangeMessage = MakePacketResChangeObjectStat(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectStatInfo);
									SendPacket(Session->SessionId, ResObjectStatChangeMessage);
									ResObjectStatChangeMessage->Free();

									auto FindLevelData = G_Datamanager->_LevelDatas.find(MyPlayer->_GameObjectInfo.ObjectStatInfo.Level);
									if (FindLevelData == G_Datamanager->_LevelDatas.end())
									{
										CRASH("레벨 데이터 찾지 못함");
									}

									LevelData = *(*FindLevelData).second;

									MyPlayer->_Experience.CurrentExperience = 0;
									MyPlayer->_Experience.RequireExperience = LevelData.RequireExperience;
									MyPlayer->_Experience.TotalExperience = LevelData.TotalExperience;
								}

								CGameServerMessage* ResMonsterGetExpMessage = MakePacketExperience(MyPlayer->_AccountId, MyPlayer->_GameObjectInfo.ObjectId, TargetMonster->_GetExpPoint, MyPlayer->_Experience.CurrentExperience, MyPlayer->_Experience.RequireExperience, MyPlayer->_Experience.TotalExperience);
								SendPacket(Session->SessionId, ResMonsterGetExpMessage);
								ResMonsterGetExpMessage->Free();
							}
						}

						en_EffectType HitEffectType;

						// 시스템 메세지 생성
						switch ((en_SkillType)ReqSkillType)
						{
						case en_SkillType::SKILL_TYPE_NONE:
							CRASH("SkillType None");
							break;
						case en_SkillType::SKILL_DEFAULT_ATTACK:
							wsprintf(SkillTypeMessage, L"%s가 일반공격을 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
							HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
							break;
						case en_SkillType::SKILL_KNIGHT_CHOHONE:
							wsprintf(SkillTypeMessage, L"%s가 초혼비무를 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
							HitEffectType = en_EffectType::EFFECT_CHOHONE_TARGET_HIT;
							break;
						case en_SkillType::SKILL_KNIGHT_SHAEHONE:
							wsprintf(SkillTypeMessage, L"%s가 쇄혼비무를 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
							HitEffectType = en_EffectType::EFFECT_SHAHONE_TARGET_HIT;
							break;
						case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
							wsprintf(SkillTypeMessage, L"%s가 분쇄파동을 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
							HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
							break;
						default:
							break;
						}

						SkillTypeString = SkillTypeMessage;
						SkillTypeString = IsCritical ? L"치명타! " + SkillTypeString : SkillTypeString;

						// 데미지 시스템 메세지 전송
						CMessage* ResSkillSystemMessagePacket = MakePacketResChattingBoxMessage(MyPlayer->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, IsCritical ? st_Color::Red() : st_Color::White(), SkillTypeString);
						SendPacketFieldOfView(Session, ResSkillSystemMessagePacket, true);
						ResSkillSystemMessagePacket->Free();

						// 공격 응답 메세지 전송
						CMessage* ResMyAttackOtherPacket = MakePacketResAttack(MyPlayer->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectId, (en_SkillType)ReqSkillType, FinalDamage, IsCritical);
						SendPacketFieldOfView(Session, ResMyAttackOtherPacket, true);						
						ResMyAttackOtherPacket->Free();

						// 이펙트 출력
						CMessage* ResEffectPacket = MakePacketEffect(Target->_GameObjectInfo.ObjectId, HitEffectType, AttackSkillInfo->SkillTargetEffectTime);
						SendPacketFieldOfView(Session, ResEffectPacket, true);
						ResEffectPacket->Free();

						// 스탯 변경 메세지 전송
						CMessage* ResChangeObjectStat = MakePacketResChangeObjectStat(Target->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectStatInfo);
						SendPacketFieldOfView(Session, ResChangeObjectStat, true);
						ResChangeObjectStat->Free();
					}
				}				

				SkillCoolTimeTimerJobCreate(MyPlayer, 500, FindSkill, en_TimerJobType::TIMER_MELEE_ATTACK_END, QuickSlotBarIndex, QuickSlotBarSlotIndex);

				float SkillCoolTime = FindSkill->SkillCoolTime / 1000.0f;

				// 클라에게 쿨타임 표시
				CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(MyPlayer->_GameObjectInfo.ObjectId, QuickSlotBarIndex, QuickSlotBarSlotIndex, SkillCoolTime, 1.0f);
				SendPacket(Session->SessionId, ResCoolTimeStartPacket);
				ResCoolTimeStartPacket->Free();

				// 쿨타임 시간 동안 스킬 사용 못하게 막음
				FindSkill->CanSkillUse = false;

				// 스킬 쿨타임 얻어옴
				// 스킬 쿨타임 스킬쿨타임 잡 등록
				st_TimerJob* SkillCoolTimeTimerJob = _TimerJobMemoryPool->Alloc();
				SkillCoolTimeTimerJob->TimerJobExecTick = GetTickCount64() + FindSkill->SkillCoolTime;
				SkillCoolTimeTimerJob->SessionId = Session->SessionId;
				SkillCoolTimeTimerJob->TimerJobType = en_TimerJobType::TIMER_SKILL_COOLTIME_END;

				CGameServerMessage* ResCoolTimeEndMessage = CGameServerMessage::GameServerMessageAlloc();
				if (ResCoolTimeEndMessage == nullptr)
				{
					return;
				}

				ResCoolTimeEndMessage->Clear();

				*ResCoolTimeEndMessage << (int16)FindSkill->SkillType;
				SkillCoolTimeTimerJob->TimerJobMessage = ResCoolTimeEndMessage;

				AcquireSRWLockExclusive(&_TimerJobLock);
				_TimerHeapJob->InsertHeap(SkillCoolTimeTimerJob->TimerJobExecTick, SkillCoolTimeTimerJob);
				ReleaseSRWLockExclusive(&_TimerJobLock);

				SetEvent(_TimerThreadWakeEvent);
			}
			else
			{
				wstring ErrorSkillCoolTime;

				WCHAR ErrorMessage[100] = { 0 };

				wsprintf(ErrorMessage, L"[%s] 재사용 대기시간이 완료되지 않았습니다.", FindSkill->SkillName.c_str());
				ErrorSkillCoolTime = ErrorMessage;

				CMessage* ResErrorPacket = MakePacketError(MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_SKILL_COOLTIME, ErrorSkillCoolTime);
				SendPacket(MyPlayer->_SessionId, ResErrorPacket);
				ResErrorPacket->Free();
				break;
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
			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// 입장한 캐릭터가 있는지 확인
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종하고 있는 캐릭의 ObjectId와 클라가 전송한 ObjectId가 같은지 확인
				if (MyPlayer->_GameObjectInfo.ObjectId != ObjectId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}		

			int8 QuickSlotBarindex;
			*Message >> QuickSlotBarindex;

			int8 QuickSlotBarSlotIndex;
			*Message >> QuickSlotBarSlotIndex;

			// 공격한 방향
			int8 ReqMoveDir;
			*Message >> ReqMoveDir;

			// 스킬 종류
			int16 ReqSkillType;
			*Message >> ReqSkillType;

			vector<CGameObject*> Targets;

			CGameObject* FindGameObject = nullptr;

			float SpellCastingTime = 0.0f;

			int64 SpellEndTime = 0;

			// 요청한 스킬이 스킬창에 있는지 확인
			st_SkillInfo* FindSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)ReqSkillType);
			if (FindSkill != nullptr && FindSkill->CanSkillUse == true)
			{
				CMessage* ResEffectPacket = nullptr;
				CMessage* ResMagicPacket = nullptr;
				CMessage* ResErrorPacket = nullptr;

				// 스킬 타입 확인
				switch ((en_SkillType)ReqSkillType)
				{
					// 돌격 자세
				case en_SkillType::SKILL_KNIGHT_CHARGE_POSE:
					MyPlayer->_SpellTick = GetTickCount64() + FindSkill->SkillCastingTime;
					SpellCastingTime = FindSkill->SkillCastingTime / 1000.0f;

					Targets.push_back(MyPlayer);

					// 이펙트 출력
					ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_CHARGE_POSE, 2.8f);
					SendPacketFieldOfView(Session, ResEffectPacket, true);					
					ResEffectPacket->Free();
					break;
				case en_SkillType::SKILL_SHAMAN_FLAME_HARPOON:
				case en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE:
				case en_SkillType::SKILL_SHAMAN_HELL_FIRE:
				case en_SkillType::SKILL_SHAMAN_ROOT:
				case en_SkillType::SKILL_SHAMAN_ICE_CHAIN:
				case en_SkillType::SKILL_SHAMAN_ICE_WAVE:
				case en_SkillType::SKILL_TAIOIST_DIVINE_STRIKE:
				case en_SkillType::SKILL_TAIOIST_ROOT:
					if (MyPlayer->_SelectTarget != nullptr)
					{
						MyPlayer->_SpellTick = GetTickCount64() + FindSkill->SkillCastingTime;
						SpellCastingTime = FindSkill->SkillCastingTime / 1000.0f;

						// 타겟이 ObjectManager에 존재하는지 확인
						FindGameObject = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType);
						if (FindGameObject != nullptr)
						{
							Targets.push_back(FindGameObject);
						}

						// 스펠창 시작
						ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId, true, (en_SkillType)ReqSkillType, SpellCastingTime);
						SendPacketFieldOfView(Session, ResMagicPacket, true);						
						ResMagicPacket->Free();

						MyPlayer->_SkillType = (en_SkillType)ReqSkillType;
					}
					else
					{
						wstring ErrorNonSelectObjectString;

						WCHAR ErrorMessage[100] = { 0 };

						wsprintf(ErrorMessage, L"[%s] 대상을 선택해야합니다.", FindSkill->SkillName.c_str());
						ErrorNonSelectObjectString = ErrorMessage;

						ResErrorPacket = MakePacketError(MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_NON_SELECT_OBJECT, ErrorNonSelectObjectString);
						SendPacket(MyPlayer->_SessionId, ResErrorPacket);
						ResErrorPacket->Free();
					}
					break;
				case en_SkillType::SKILL_TAIOIST_HEALING_LIGHT: // 치유의 빛
					MyPlayer->_SpellTick = GetTickCount64() + FindSkill->SkillCastingTime;

					SpellCastingTime = FindSkill->SkillCastingTime / 1000.0f;

					if (MyPlayer->_SelectTarget != nullptr)
					{
						FindGameObject = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType);
						if (FindGameObject != nullptr)
						{
							Targets.push_back(FindGameObject);
						}
					}
					else
					{
						Targets.push_back(MyPlayer);

						wstring ErrorNonSelectObjectString;

						WCHAR ErrorMessage[100] = { 0 };

						wsprintf(ErrorMessage, L"[%s] 대상을 선택하지 않아서 자신에게 사용합니다.", FindSkill->SkillName.c_str());
						ErrorNonSelectObjectString = ErrorMessage;

						ResErrorPacket = MakePacketError(MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_NON_SELECT_OBJECT, ErrorNonSelectObjectString);
						SendPacket(MyPlayer->_SessionId, ResErrorPacket);
						ResErrorPacket->Free();
					}

					// 스펠창 시작
					ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId, true, (en_SkillType)ReqSkillType, SpellCastingTime);
					SendPacketFieldOfView(Session, ResMagicPacket, true);					
					ResMagicPacket->Free();

					MyPlayer->_SkillType = (en_SkillType)ReqSkillType;
					break;
				case en_SkillType::SKILL_TAIOIST_HEALING_WIND: // 치유의 바람
					if (MyPlayer->_SelectTarget != nullptr)
					{
						MyPlayer->_SpellTick = GetTickCount64() + FindSkill->SkillCastingTime;

						SpellCastingTime = FindSkill->SkillCastingTime / 1000.0f;

						FindGameObject = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType);
						if (FindGameObject != nullptr)
						{
							Targets.push_back(FindGameObject);
						}

						// 스펠창 시작
						ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId, true, (en_SkillType)ReqSkillType, SpellCastingTime);
						SendPacketFieldOfView(Session, ResMagicPacket, true);						
						ResMagicPacket->Free();

						MyPlayer->_SkillType = (en_SkillType)ReqSkillType;
					}
					else
					{

					}
					break;
				case en_SkillType::SKILL_SHOCK_RELEASE:
					break;
				}

				if (Targets.size() >= 1)
				{
					MyPlayer->SetTarget(Targets[0]);

					MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPELL;

					// 마법 스킬 모션 출력
					CMessage* ResObjectStateChangePacket = MakePacketResChangeObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
					SendPacketFieldOfView(Session, ResObjectStateChangePacket, true);					
					ResObjectStateChangePacket->Free();					

					SkillCoolTimeTimerJobCreate(MyPlayer, FindSkill->SkillCastingTime, FindSkill, en_TimerJobType::TIMER_SPELL_END, QuickSlotBarindex, QuickSlotBarSlotIndex);
				}
			}
			else
			{
				if (FindSkill == nullptr)
				{
					break;
				}

				wstring ErrorSkillCoolTime;

				WCHAR ErrorMessage[100] = { 0 };

				wsprintf(ErrorMessage, L"[%s] 재사용 대기시간이 완료되지 않았습니다.", FindSkill->SkillName.c_str());
				ErrorSkillCoolTime = ErrorMessage;

				CMessage* ResErrorPacket = MakePacketError(MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_SKILL_COOLTIME, ErrorSkillCoolTime);
				SendPacket(MyPlayer->_SessionId, ResErrorPacket);
				ResErrorPacket->Free();
				break;
			}
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqMagicCancel(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

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

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			CPlayer* MagicCancelPlayer = (CPlayer*)G_ObjectManager->Find(PlayerId, en_GameObjectType::OBJECT_PLAYER);
			if (MagicCancelPlayer->_SkillJob != nullptr)
			{
				MagicCancelPlayer->_SkillJob->TimerJobCancel = true;
				MagicCancelPlayer->_SkillJob = nullptr;

				// 스킬 취소 응답 처리 주위 섹터에 전송
				CMessage* ResMagicCancelPacket = MakePacketMagicCancel(MyPlayer->_AccountId, MyPlayer->_GameObjectInfo.ObjectId);
				SendPacketFieldOfView(Session, ResMagicCancelPacket, true);				
				ResMagicCancelPacket->Free();
			}
			else
			{
				// 시전중인 스킬이 존재하지 않음
				wstring ErrorCastingSkillNotExist;

				WCHAR ErrorMessage[100] = { 0 };

				wsprintf(ErrorMessage, L"스킬 시전 중이 아닙니다.");
				ErrorCastingSkillNotExist = ErrorMessage;

				CMessage* ResErrorPacket = MakePacketError(MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_SKILL_COOLTIME, ErrorCastingSkillNotExist);
				SendPacket(SessionId, ResErrorPacket);
				ResErrorPacket->Free();
			}
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqMousePositionObjectInfo(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

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

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerId)
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

				if (MyPlayer->_SelectTarget != nullptr)
				{
					PreviousChoiceObject = MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId;
				}

				MyPlayer->_SelectTarget = FindObject;

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

			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// 조종하고 있는 플레이어가 있는지 확인 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				return;
			}
			else
			{
				// 조종하고 있는 플레이어와 전송받은 PlayerId가 같은지 확인
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					return;
				}
			}

			int16 ObjectType;
			*Message >> ObjectType;

			int64 ObjectId;
			*Message >> ObjectId;

			*Message >> StateChange;

			CMessage* ResObjectStatePakcet = nullptr;

			CGameObject* FindGameObject = G_ObjectManager->Find(ObjectId, (en_GameObjectType)ObjectType);
			if (FindGameObject != nullptr)
			{
				switch ((en_StateChange)StateChange)
				{
				case en_StateChange::MOVE_TO_STOP:
					if (FindGameObject->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::MOVING
						|| FindGameObject->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::SPAWN_IDLE
						|| FindGameObject->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::IDLE
						|| FindGameObject->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::PATROL
						|| FindGameObject->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::ATTACK)
					{
						FindGameObject->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

						ResObjectStatePakcet = MakePacketResChangeObjectState(FindGameObject->_GameObjectInfo.ObjectId, FindGameObject->_GameObjectInfo.ObjectPositionInfo.MoveDir, FindGameObject->_GameObjectInfo.ObjectType, FindGameObject->_GameObjectInfo.ObjectPositionInfo.State);
						SendPacketFieldOfView(Session, ResObjectStatePakcet, true);
						ResObjectStatePakcet->Free();
					}
					else
					{
						Disconnect(Session->SessionId);
						break;
					}
					break;
				case en_StateChange::SPELL_TO_IDLE:
					if (FindGameObject->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::ATTACK
						|| FindGameObject->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::SPELL)
					{
						FindGameObject->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

						ResObjectStatePakcet = MakePacketResChangeObjectState(FindGameObject->_GameObjectInfo.ObjectId, FindGameObject->_GameObjectInfo.ObjectPositionInfo.MoveDir, FindGameObject->_GameObjectInfo.ObjectType, FindGameObject->_GameObjectInfo.ObjectPositionInfo.State);
						SendPacketFieldOfView(Session, ResObjectStatePakcet, true);						
						ResObjectStatePakcet->Free();
					}
					else
					{
						Disconnect(Session->SessionId);
						break;
					}
					break;
				}
			}
		} while (0);
	}

	ReturnSession(Session);
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

		// 게임에 입장한 캐릭터를 가져온다.
		CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

		// 조종하고 있는 플레이어가 있는지 확인 
		if (MyPlayer == nullptr)
		{
			Disconnect(Session->SessionId);
			return;
		}
		else
		{
			// 조종하고 있는 플레이어와 전송받은 PlayerId가 같은지 확인
			if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
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

		CMessage* ResChattingMessage = MakePacketResChattingBoxMessage(PlayerDBId, en_MessageType::CHATTING, st_Color::White(), ChattingMessage);
		SendPacketFieldOfView(Session, ResChattingMessage, true);
		ResChattingMessage->Free();		
	}

	// 세션 반납
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

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// 조종하고 있는 플레이어가 있는지 확인 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종하고 있는 플레이어와 전송받은 PlayerId가 같은지 확인
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			int8 SwapIndexA;
			*Message >> SwapIndexA;

			int8 SwapIndexB;
			*Message >> SwapIndexB;

			InterlockedIncrement64(&Session->IOBlock->IOCount);

			st_Job* DBItemSwapJob = _JobMemoryPool->Alloc();
			DBItemSwapJob->Type = en_JobType::DATA_BASE_ITEM_SWAP;
			DBItemSwapJob->SessionId = MyPlayer->_SessionId;

			CGameServerMessage* DBItemSwapMessage = CGameServerMessage::GameServerMessageAlloc();
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

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// 조종하고 있는 플레이어가 있는지 확인 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종하고 있는 플레이어와 전송받은 PlayerId가 같은지 확인
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			InterlockedIncrement64(&Session->IOBlock->IOCount);

			st_Job* DBQuickSlotSaveJob = _JobMemoryPool->Alloc();
			DBQuickSlotSaveJob->Type = en_JobType::DATA_BASE_QUICK_SLOT_SAVE;
			DBQuickSlotSaveJob->SessionId = MyPlayer->_SessionId;

			CGameServerMessage* DBQuickSlotSaveMessage = CGameServerMessage::GameServerMessageAlloc();
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

// int64 AccountId
// int64 ObjectId
// int8 SwapQuickSlotBarIndexA
// int8 SwapQuickSlotBarSlotIndexA
// int8 SwapQuickSlotBarIndexB
// int8 SwapQuickSlotBarSlotIndexB
void CGameServer::PacketProcReqQuickSlotSwap(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
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

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// 조종하고 있는 플레이어가 있는지 확인 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종하고 있는 플레이어와 전송받은 PlayerId가 같은지 확인
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			InterlockedIncrement64(&Session->IOBlock->IOCount);

			st_Job* DBQuickSlotSwapJob = _JobMemoryPool->Alloc();
			DBQuickSlotSwapJob->Type = en_JobType::DATA_BASE_QUICK_SWAP;
			DBQuickSlotSwapJob->SessionId = MyPlayer->_SessionId;

			CGameServerMessage* DBQuickSlotSwapMessage = CGameServerMessage::GameServerMessageAlloc();
			DBQuickSlotSwapMessage->Clear();

			*DBQuickSlotSwapMessage << AccountId;
			*DBQuickSlotSwapMessage << PlayerDBId;
			DBQuickSlotSwapMessage->InsertData(Message->GetFrontBufferPtr(), Message->GetUseBufferSize());
			Message->MoveReadPosition(Message->GetUseBufferSize());

			DBQuickSlotSwapJob->Message = DBQuickSlotSwapMessage;

			_GameServerDataBaseThreadMessageQue.Enqueue(DBQuickSlotSwapJob);
			SetEvent(_DataBaseWakeEvent);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqQuickSlotInit(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
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

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// 조종하고 있는 플레이어가 있는지 확인 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종하고 있는 플레이어와 전송받은 PlayerId가 같은지 확인
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			InterlockedIncrement64(&Session->IOBlock->IOCount);

			st_Job* DBQuickSlotInitJob = _JobMemoryPool->Alloc();
			DBQuickSlotInitJob->Type = en_JobType::DATA_BASE_QUICK_INIT;
			DBQuickSlotInitJob->SessionId = MyPlayer->_SessionId;

			CGameServerMessage* DBQuickSlotInitMessage = CGameServerMessage::GameServerMessageAlloc();
			DBQuickSlotInitMessage->Clear();

			*DBQuickSlotInitMessage << AccountId;
			*DBQuickSlotInitMessage << PlayerDBId;
			DBQuickSlotInitMessage->InsertData(Message->GetFrontBufferPtr(), Message->GetUseBufferSize());
			Message->MoveReadPosition(Message->GetUseBufferSize());

			DBQuickSlotInitJob->Message = DBQuickSlotInitMessage;

			_GameServerDataBaseThreadMessageQue.Enqueue(DBQuickSlotInitJob);
			SetEvent(_DataBaseWakeEvent);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqCraftingConfirm(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		do
		{
			int64 AccountId;
			int64 PlayerDBId;

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

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// 조종하고 있는 플레이어가 있는지 확인 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종하고 있는 플레이어와 전송받은 PlayerId가 같은지 확인
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			int8 ReqCategoryType;
			*Message >> ReqCategoryType;

			int16 ReqCraftingItemType;
			*Message >> ReqCraftingItemType;

			int16 ReqCraftingItemCount;
			*Message >> ReqCraftingItemCount;

			int8 MaterialTotalCount;
			*Message >> MaterialTotalCount;

			// 클라가 제작 요청한 제작템의 재료템과 재료템의 개수
			vector<st_CraftingMaterialItemInfo> ReqMaterials;
			for (int i = 0; i < MaterialTotalCount; i++)
			{
				st_CraftingMaterialItemInfo CraftingMaterialItemInfo;

				int16 MaterialType;
				*Message >> MaterialType;
				int16 MaterialItemCount;
				*Message >> MaterialItemCount;

				CraftingMaterialItemInfo.MaterialItemType = (en_SmallItemCategory)MaterialType;
				CraftingMaterialItemInfo.ItemCount = MaterialItemCount;

				ReqMaterials.push_back(CraftingMaterialItemInfo);
			}

			st_CraftingItemCategoryData* FindCraftingCategoryData = nullptr;
			// 제작법 카테고리 찾기
			auto FindCategoryItem = G_Datamanager->_CraftingData.find(ReqCategoryType);
			if (FindCategoryItem == G_Datamanager->_CraftingData.end())
			{
				// 카테고리를 찾지 못함
				break;
			}

			FindCraftingCategoryData = (*FindCategoryItem).second;

			// 완성템 제작에 필요한 재료 목록 찾기
			vector<st_CraftingMaterialItemData> RequireMaterialDatas;
			for (st_CraftingCompleteItemData CraftingCompleteItemData : FindCraftingCategoryData->CraftingCompleteItems)
			{
				if (CraftingCompleteItemData.CraftingCompleteItemDataId == (en_SmallItemCategory)ReqCraftingItemType)
				{
					RequireMaterialDatas = CraftingCompleteItemData.CraftingMaterials;
				}
			}			

			InterlockedIncrement64(&Session->IOBlock->IOCount);

			// 재료템 DB에 개수 업데이트와 제작한 아이템 저장하기 위한 Job
			st_Job* DBInventorySaveJob = _JobMemoryPool->Alloc();
			DBInventorySaveJob->Type = en_JobType::DATA_BASE_CRAFTING_ITEM_INVENTORY_SAVE;
			DBInventorySaveJob->SessionId = Session->SessionId;

			CGameServerMessage* DBCraftingItemSaveMessage = CGameServerMessage::GameServerMessageAlloc();
			if (DBCraftingItemSaveMessage == nullptr)
			{
				break;
			}

			DBCraftingItemSaveMessage->Clear();

			// 타겟 ObjectId
			*DBCraftingItemSaveMessage << MyPlayer->_GameObjectInfo.ObjectId;

			bool InventoryCheck = true;
			// 완성 제작템 만드는데 필요한 재료 개수
			*DBCraftingItemSaveMessage << (int8)ReqMaterials.size();

			// 인벤토리에 재료템이 있는지 확인
			for (st_CraftingMaterialItemInfo CraftingMaterialItemInfo : ReqMaterials)
			{
				// 인벤토리에 재료템 목록을 가지고 옴
				vector<CItem*> FindMaterialItem;
				vector<CItem*> FindMaterials = MyPlayer->_Inventory.Find(CraftingMaterialItemInfo.MaterialItemType);

				if (FindMaterials.size() > 0)
				{
					for (CItem* FindMaterialItem : FindMaterials)
					{
						// 제작템을 한개 만들때 필요한 재료의 개수를 얻어옴
						int16 OneReqMaterialCount = 0;
						for (st_CraftingMaterialItemData ReqMaterialCountData : RequireMaterialDatas)
						{
							if (FindMaterialItem->_ItemInfo.ItemSmallCategory == ReqMaterialCountData.MaterialDataId)
							{
								OneReqMaterialCount = ReqMaterialCountData.MaterialCount;
								break;
							}
						}

						// 클라가 요청한 개수 * 한개 만들떄 필요한 개수 만큼 인벤토리에서 차감한다.
						int16 ReqCraftingItemTotalCount = ReqCraftingItemCount * OneReqMaterialCount;
						FindMaterialItem->_ItemInfo.ItemCount -= ReqCraftingItemTotalCount;

						// 앞서 업데이트한 아이템의 정보를 DB에 업데이트 하기 위해 담는다.
						*DBCraftingItemSaveMessage << &FindMaterialItem;
					}
				}
				else
				{
					InventoryCheck = false;
					break;
				}
			}

			if (InventoryCheck == false)
			{
				break;
			}

			// 재료템 확인작업 완료
			// 아이템 생성 후 인벤토리에 넣고 클라에게도 전송

			// 만들고자 하는 제작템을 DataManager에서 가져온다.
			auto ItemDataIterator = G_Datamanager->_Items.find((int16)ReqCraftingItemType);
			if (ItemDataIterator == G_Datamanager->_Items.end())
			{
				break;
			}

			st_ItemData* FindReqCompleteItemData = (*ItemDataIterator).second;

			st_ItemInfo CraftingItemInfo;
			CraftingItemInfo.ItemDBId = 0;
			CraftingItemInfo.ItemIsQuickSlotUse = FindReqCompleteItemData->ItemIsEquipped;
			CraftingItemInfo.ItemLargeCategory = FindReqCompleteItemData->LargeItemCategory;
			CraftingItemInfo.ItemMediumCategory = FindReqCompleteItemData->MediumItemCategory;
			CraftingItemInfo.ItemSmallCategory = FindReqCompleteItemData->SmallItemCategory;
			CraftingItemInfo.ItemName = (LPWSTR)CA2W(FindReqCompleteItemData->ItemName.c_str());
			CraftingItemInfo.ItemExplain = (LPWSTR)CA2W(FindReqCompleteItemData->ItemExplain.c_str());
			CraftingItemInfo.ItemMinDamage = FindReqCompleteItemData->ItemMinDamage;
			CraftingItemInfo.ItemMaxDamage = FindReqCompleteItemData->ItemMaxDamage;
			CraftingItemInfo.ItemDefence = FindReqCompleteItemData->ItemDefence;
			CraftingItemInfo.ItemMaxCount = FindReqCompleteItemData->ItemMaxCount;
			CraftingItemInfo.ItemCount = ReqCraftingItemCount;
			CraftingItemInfo.ItemThumbnailImagePath = (LPWSTR)CA2W(FindReqCompleteItemData->ItemThumbnailImagePath.c_str());
			CraftingItemInfo.ItemIsEquipped = FindReqCompleteItemData->ItemIsEquipped;

			bool IsExistItem = false;
			int8 SlotIndex = 0;

			// 인벤토리에 제작템이 이미 있으면 개수만 늘려준다.
			IsExistItem = MyPlayer->_Inventory.IsExistItem(CraftingItemInfo.ItemSmallCategory, ReqCraftingItemCount, &SlotIndex);

			if (IsExistItem == false)
			{
				switch (FindReqCompleteItemData->SmallItemCategory)
				{
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
				{
					CWeapon* CraftingWeaponItem = (CWeapon*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_WEAPON);
					CraftingWeaponItem->_ItemInfo = CraftingItemInfo;

					if (MyPlayer->_Inventory.GetEmptySlot(&SlotIndex))
					{
						CraftingWeaponItem->_ItemInfo.ItemSlotIndex = SlotIndex;
						MyPlayer->_Inventory.AddItem(CraftingWeaponItem);
					}
				}
				break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
				{
					CArmor* CraftingArmorItem = (CArmor*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_ARMOR);
					CraftingArmorItem->_ItemInfo = CraftingItemInfo;

					if (MyPlayer->_Inventory.GetEmptySlot(&SlotIndex))
					{
						CraftingArmorItem->_ItemInfo.ItemSlotIndex = SlotIndex;
						MyPlayer->_Inventory.AddItem(CraftingArmorItem);
					}
				}
				break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_FIERCE_ATTACK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE:
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE:
				case en_SmallItemCategory::ITEM_SMALL_CATEOGRY_SKILLBOOK_SHAMAN_HEALING_LIGHT:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HEALING_WIND:
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE:
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
				{
					CMaterial* CraftingMaterialItem = (CMaterial*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_MATERIAL);
					CraftingMaterialItem->_ItemInfo = CraftingItemInfo;

					if (MyPlayer->_Inventory.GetEmptySlot(&SlotIndex))
					{
						CraftingMaterialItem->_ItemInfo.ItemSlotIndex = SlotIndex;
						MyPlayer->_Inventory.AddItem(CraftingMaterialItem);
					}
				}
				break;
				default:
					break;
				}
			}

			CItem* CraftingItemCompleteItem = MyPlayer->_Inventory.Get(SlotIndex);

			// 중복 여부
			*DBCraftingItemSaveMessage << IsExistItem;
			// 증가한 아이템 개수
			*DBCraftingItemSaveMessage << ReqCraftingItemCount;
			// 작업 완료된 아이템 정보
			*DBCraftingItemSaveMessage << &CraftingItemCompleteItem;
			*DBCraftingItemSaveMessage << AccountId;

			DBInventorySaveJob->Message = DBCraftingItemSaveMessage;

			_GameServerDataBaseThreadMessageQue.Enqueue(DBInventorySaveJob);
			SetEvent(_DataBaseWakeEvent);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqItemUse(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		do
		{
			int64 AccountId;
			int64 PlayerDBId;

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

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// 조종하고 있는 플레이어가 있는지 확인 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// 조종하고 있는 플레이어와 전송받은 PlayerId가 같은지 확인
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			int8 ItemLargeCategory;
			int8 ItemMediumCategory;
			int16 ItemSmallCategory;

			// 클라에서 요청한 사용 아이템 정보
			st_ItemInfo UseItemInfo;
			*Message >> UseItemInfo.ItemDBId;
			*Message >> UseItemInfo.ItemIsQuickSlotUse;
			*Message >> ItemLargeCategory;
			UseItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
			*Message >> ItemMediumCategory;
			UseItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
			*Message >> ItemSmallCategory;
			UseItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;

			int8 ItemNameLen;
			*Message >> ItemNameLen;
			Message->GetData(UseItemInfo.ItemName, ItemNameLen);

			*Message >> UseItemInfo.ItemMinDamage;
			*Message >> UseItemInfo.ItemMaxDamage;
			*Message >> UseItemInfo.ItemDefence;
			*Message >> UseItemInfo.ItemMaxCount;
			*Message >> UseItemInfo.ItemCount;

			int8 ItemThumbnailImagePathLen;
			*Message >> ItemThumbnailImagePathLen;
			Message->GetData(UseItemInfo.ItemThumbnailImagePath, ItemThumbnailImagePathLen);
			*Message >> UseItemInfo.ItemIsEquipped;
			*Message >> UseItemInfo.ItemSlotIndex;

			// 인벤토리에 아이템이 있는지 확인
			CItem* UseItem = MyPlayer->_Inventory.Get(UseItemInfo.ItemSlotIndex);
			if (UseItem->_ItemInfo != UseItemInfo)
			{
				CRASH("요청한 사용 아이템과 인벤토리에 보관중인 아이템이 다름");
			}

			InterlockedIncrement64(&Session->IOBlock->IOCount);

			st_Job* DBItemEquipJob = _JobMemoryPool->Alloc();
			DBItemEquipJob->Type = en_JobType::DATA_BASE_ITEM_USE;
			DBItemEquipJob->SessionId = MyPlayer->_SessionId;

			CGameServerMessage* DBItemEquipMessage = CGameServerMessage::GameServerMessageAlloc();
			DBItemEquipMessage->Clear();

			// 착용할 아이템과 착용해제한 아이템의 정보를 담음
			*DBItemEquipMessage << &UseItem;

			DBItemEquipJob->Message = DBItemEquipMessage;

			_GameServerDataBaseThreadMessageQue.Enqueue(DBItemEquipJob);
			SetEvent(_DataBaseWakeEvent);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqItemLooting(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		do
		{
			int64 AccountId;

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

			st_PositionInfo ItemPosition;
			int8 ItemState;
			int8 ItemMoveDir;

			*Message >> ItemState;
			*Message >> ItemPosition.PositionX;
			*Message >> ItemPosition.PositionY;
			*Message >> ItemMoveDir;

			st_Vector2Int ItemCellPosition;
			ItemCellPosition._X = ItemPosition.PositionX;
			ItemCellPosition._Y = ItemPosition.PositionY;

			// 루팅 위치에 아이템들을 가져온다.
			CItem** Items = G_ChannelManager->Find(1)->_Map->FindItem(ItemCellPosition);
			if (Items != nullptr)
			{
				for (int8 i = 0; i < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; i++)
				{
					if (Items[i] == nullptr)
					{
						continue;
					}

					// 게임에 입장한 캐릭터를 가져온다.
					CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

					int64 TargetObjectId = MyPlayer->_GameObjectInfo.ObjectId;

					CPlayer* TargetPlayer = (CPlayer*)(G_ObjectManager->Find(TargetObjectId, en_GameObjectType::OBJECT_PLAYER));
					if (TargetPlayer == nullptr)
					{
						break;
					}

					bool IsExistItem = false;
					CItem* CraftingItemCompleteItem;
					// 아이템 정보가 코인이라면
					if (Items[i]->_ItemInfo.ItemSmallCategory == en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN
						|| Items[i]->_ItemInfo.ItemSmallCategory == en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN
						|| Items[i]->_ItemInfo.ItemSmallCategory == en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN)
					{
						// 코인 저장
						TargetPlayer->_Inventory.AddItem(Items[i]);

						InterlockedIncrement64(&Session->IOBlock->IOCount);

						st_Job* DBGoldSaveJob = _JobMemoryPool->Alloc();
						DBGoldSaveJob->Type = en_JobType::DATA_BASE_GOLD_SAVE;
						DBGoldSaveJob->SessionId = TargetPlayer->_SessionId;

						CGameServerMessage* DBGoldSaveMessage = CGameServerMessage::GameServerMessageAlloc();
						DBGoldSaveMessage->Clear();

						*DBGoldSaveMessage << TargetPlayer->_AccountId;
						*DBGoldSaveMessage << TargetPlayer->_GameObjectInfo.ObjectId;
						*DBGoldSaveMessage << Items[i]->_ItemInfo.ItemDBId;
						*DBGoldSaveMessage << TargetPlayer->_Inventory._GoldCoinCount;
						*DBGoldSaveMessage << TargetPlayer->_Inventory._SliverCoinCount;
						*DBGoldSaveMessage << TargetPlayer->_Inventory._BronzeCoinCount;
						*DBGoldSaveMessage << Items[i]->_ItemInfo.ItemCount;
						*DBGoldSaveMessage << (int16)Items[i]->_ItemInfo.ItemSmallCategory;

						DBGoldSaveJob->Message = DBGoldSaveMessage;

						_GameServerDataBaseThreadMessageQue.Enqueue(DBGoldSaveJob);
						SetEvent(_DataBaseWakeEvent);
					}
					else
					{
						// 인벤토리에 저장할 아이템 개수 ( 맵에 스폰되어 있는 아이템의 개수 )
						int16 ItemEach = Items[i]->_ItemInfo.ItemCount;

						// 그 외 아이템이라면 
						int8 SlotIndex = 0;
						// 인벤토리에 저장된 아이템 개수
						int16 ItemCount = 0;

						// 아이템이 이미 존재 하는지 확인한다.
						// 존재 하면 개수를 아이템 개수 만큼 증가한다.
						IsExistItem = TargetPlayer->_Inventory.IsExistItem(Items[i]->_ItemInfo.ItemSmallCategory, ItemEach, &SlotIndex);

						// 존재 하지 않을 경우
						if (IsExistItem == false)
						{
							// 비어 있는 슬롯인덱스 가져와서
							if (TargetPlayer->_Inventory.GetEmptySlot(&SlotIndex))
							{
								// 인벤토리에 저장
								Items[i]->_ItemInfo.ItemSlotIndex = SlotIndex;
								TargetPlayer->_Inventory.AddItem(Items[i]);
								ItemCount = ItemEach;
							}
						}

						InterlockedIncrement64(&Session->IOBlock->IOCount);

						// DB 인벤토리에 저장하기 위해 Job 생성
						st_Job* DBInventorySaveJob = _JobMemoryPool->Alloc();
						DBInventorySaveJob->Type = en_JobType::DATA_BASE_LOOTING_ITEM_INVENTORY_SAVE;
						DBInventorySaveJob->SessionId = Session->SessionId;

						CGameServerMessage* DBSaveMessage = CGameServerMessage::GameServerMessageAlloc();

						DBSaveMessage->Clear();

						CraftingItemCompleteItem = TargetPlayer->_Inventory.Get(SlotIndex);

						// 타겟 ObjectId
						*DBSaveMessage << TargetPlayer->_GameObjectInfo.ObjectId;
						// 중복 여부
						*DBSaveMessage << IsExistItem;
						// 얻은 아이템 개수
						*DBSaveMessage << ItemEach;
						// 삭제해야할 아이템 ObjectId;
						*DBSaveMessage << Items[i]->_GameObjectInfo.ObjectId;
						// 아이템 포인터 담기
						*DBSaveMessage << &CraftingItemCompleteItem;
						// 아이템 처리한 AccountId
						*DBSaveMessage << AccountId;

						DBInventorySaveJob->Message = DBSaveMessage;

						_GameServerDataBaseThreadMessageQue.Enqueue(DBInventorySaveJob);
						SetEvent(_DataBaseWakeEvent);
					}

					// 월드에 스폰되어 있었던 아이템을 반납한다.
					G_ObjectManager->ObjectLeaveGame(Items[i], 1, IsExistItem);
				}
			}

		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqPong(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

	Session->PingPacketTime = GetTickCount64();

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
		// 더미일 경우에는 토큰은 확인하지 않는다.

		int DBToken = 0;

		if (Session->IsDummy == false)
		{
			// AccountNo와 Token으로 AccountServerDB 접근해서 데이터가 있는지 확인
			CDBConnection* TokenDBConnection = G_DBConnectionPool->Pop(en_DBConnect::TOKEN);

			SP::CDBAccountTokenGet AccountTokenGet(*TokenDBConnection);
			AccountTokenGet.InAccountID(ClientAccountId); // AccountId 입력
			
			TIMESTAMP_STRUCT LoginSuccessTime;
			TIMESTAMP_STRUCT TokenExpiredTime;

			AccountTokenGet.OutToken(DBToken); // 토큰 받아옴
			AccountTokenGet.OutLoginsuccessTime(LoginSuccessTime);
			AccountTokenGet.OutTokenExpiredTime(TokenExpiredTime);

			AccountTokenGet.Execute();

			AccountTokenGet.Fetch();

			G_DBConnectionPool->Push(en_DBConnect::TOKEN, TokenDBConnection); // 풀 반납
		}	

		// DB 토큰과 클라로부터 온 토큰이 같으면 로그인 최종성공
		if (Session->IsDummy == true || Token == DBToken)
		{
			Session->IsLogin = true;
			// 클라가 소유하고 있는 플레이어들을 DB로부터 긁어온다.
			CDBConnection* GameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

			SP::CDBGameServerPlayersGet ClientPlayersGet(*GameServerDBConnection);
			ClientPlayersGet.InAccountID(ClientAccountId);

			int64 PlayerId;
			WCHAR PlayerName[100];
			int16 PlayerObjectType;
			int8 PlayerIndex;
			int32 PlayerLevel;
			int32 PlayerCurrentHP;
			int32 PlayerMaxHP;
			int32 PlayerCurrentMP;
			int32 PlayerMaxMP;
			int32 PlayerCurrentDP;
			int32 PlayerMaxDP;
			int16 PlayerAutoRecoveyHPPercent;
			int16 PlayerAutoRecoveyMPPercent;
			int32 PlayerMinMeleeAttackDamage;
			int32 PlayerMaxMeleeAttackDamage;
			int16 PlayerMeleeAttackHitRate;
			int16 PlayerMagicDamage;
			int16 PlayerMagicHitRate;
			int32 PlayerDefence;
			int16 PlayerEvasionRate;
			int16 PlayerMeleeCriticalPoint;
			int16 PlayerMagicCriticalPoint;
			float PlayerSpeed;
			int32 PlayerLastPositionY;
			int32 PlayerLastPositionX;
			int64 PlayerCurrentExperience;
			int64 PlayerRequireExperience;
			int64 PlayerTotalExperience;

			ClientPlayersGet.OutPlayerDBID(PlayerId);
			ClientPlayersGet.OutPlayerName(PlayerName);
			ClientPlayersGet.OutPlayerObjectType(PlayerObjectType);
			ClientPlayersGet.OutPlayerIndex(PlayerIndex);
			ClientPlayersGet.OutLevel(PlayerLevel);
			ClientPlayersGet.OutCurrentHP(PlayerCurrentHP);
			ClientPlayersGet.OutMaxHP(PlayerMaxHP);
			ClientPlayersGet.OutCurrentMP(PlayerCurrentMP);
			ClientPlayersGet.OutMaxMP(PlayerMaxMP);
			ClientPlayersGet.OutCurrentDP(PlayerCurrentDP);
			ClientPlayersGet.OutMaxDP(PlayerMaxDP);
			ClientPlayersGet.OutAutoRecoveryHPPercent(PlayerAutoRecoveyHPPercent);
			ClientPlayersGet.OutAutoRecoveryMPPercent(PlayerAutoRecoveyMPPercent);
			ClientPlayersGet.OutMinMeleeAttackDamage(PlayerMinMeleeAttackDamage);
			ClientPlayersGet.OutMaxMeleeAttackDamage(PlayerMaxMeleeAttackDamage);
			ClientPlayersGet.OutMeleeAttackHitRate(PlayerMeleeAttackHitRate);
			ClientPlayersGet.OutMagicDamage(PlayerMagicDamage);
			ClientPlayersGet.OutMagicHitRate(PlayerMagicHitRate);
			ClientPlayersGet.OutDefence(PlayerDefence);
			ClientPlayersGet.OutEvasionRate(PlayerEvasionRate);
			ClientPlayersGet.OutMeleeCriticalPoint(PlayerMeleeCriticalPoint);
			ClientPlayersGet.OutMagicCriticalPointt(PlayerMagicCriticalPoint);
			ClientPlayersGet.OutSpeed(PlayerSpeed);
			ClientPlayersGet.OutLastPositionY(PlayerLastPositionY);
			ClientPlayersGet.OutLastPositionX(PlayerLastPositionX);
			ClientPlayersGet.OutCurrentExperience(PlayerCurrentExperience);
			ClientPlayersGet.OutRequireExperience(PlayerRequireExperience);
			ClientPlayersGet.OutTotalExperience(PlayerTotalExperience);

			bool FindPlyaerCharacter = ClientPlayersGet.Execute();

			int8 PlayerCount = 0;

			while (ClientPlayersGet.Fetch())
			{
				// 플레이어 정보 셋팅
				CPlayer* NewPlayerCharacter = (CPlayer*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_PLAYER);
				NewPlayerCharacter->_GameObjectInfo.ObjectId = PlayerId;
				NewPlayerCharacter->_GameObjectInfo.ObjectName = PlayerName;
				NewPlayerCharacter->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)PlayerObjectType;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Level = PlayerLevel;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.HP = PlayerCurrentHP;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxHP = PlayerMaxHP;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MP = PlayerCurrentMP;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxMP = PlayerMaxMP;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.DP = PlayerCurrentDP;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxDP = PlayerMaxDP;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = PlayerAutoRecoveyHPPercent;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = PlayerAutoRecoveyMPPercent;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = PlayerMinMeleeAttackDamage;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = PlayerMaxMeleeAttackDamage;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = PlayerMeleeAttackHitRate;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MagicDamage = PlayerMagicDamage;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MagicHitRate = PlayerMagicHitRate;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Defence = PlayerDefence;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.EvasionRate = PlayerEvasionRate;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = PlayerMeleeCriticalPoint;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = PlayerMagicCriticalPoint;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Speed = PlayerSpeed;
				NewPlayerCharacter->_SpawnPosition._Y = PlayerLastPositionY;
				NewPlayerCharacter->_SpawnPosition._X = PlayerLastPositionX;
				NewPlayerCharacter->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
				NewPlayerCharacter->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
				NewPlayerCharacter->_GameObjectInfo.ObjectType = (en_GameObjectType)PlayerObjectType;
				NewPlayerCharacter->_GameObjectInfo.OwnerObjectId = 0;
				NewPlayerCharacter->_GameObjectInfo.PlayerSlotIndex = PlayerIndex;
				NewPlayerCharacter->_SessionId = Session->SessionId;
				NewPlayerCharacter->_AccountId = Session->AccountId;
				NewPlayerCharacter->_Experience.CurrentExperience = PlayerCurrentExperience;
				NewPlayerCharacter->_Experience.RequireExperience = PlayerRequireExperience;
				NewPlayerCharacter->_Experience.TotalExperience = PlayerTotalExperience;
				
				// 플레이어 배열에 새로만들어준 캐릭터 할당
				G_ObjectManager->_PlayersArray[Session->MyPlayerIndexes[PlayerCount]] = NewPlayerCharacter;
				
				PlayerCount++;
			}

			// 클라에게 로그인 응답 패킷을 보내면서 캐릭터의 정보를 함께 보낸다.
			CMessage* ResLoginMessage = MakePacketResLogin(Status, PlayerCount, Session->MyPlayerIndexes);
			SendPacket(Session->SessionId, ResLoginMessage);
			ResLoginMessage->Free();

			G_DBConnectionPool->Push(en_DBConnect::GAME, GameServerDBConnection);
		}
		else
		{
			Session->IsLogin = false;
			//Disconnect(SessionID);
		}
	}
	else
	{
		// 클라 접속 끊겻을 경우
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBCreateCharacterNameCheck(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

	int64 PlayerDBId = 0;

	if (Session != nullptr)
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
			// 요청한 캐릭터의 1레벨에 해당하는 데이터를 읽어온다.
			st_ObjectStatusData NewCharacterStatus;
			switch ((en_GameObjectType)ReqGameObjectType)
			{
			case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
				{
					auto FindStatus = G_Datamanager->_WarriorStatus.find(1);
					NewCharacterStatus = *(*FindStatus).second;
				}
				break;
			case en_GameObjectType::OBJECT_MAGIC_PLAYER:
				{
					auto FindStatus = G_Datamanager->_ShamanStatus.find(1);
					NewCharacterStatus = *(*FindStatus).second;
				}
				break;
			case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
				{
					auto FindStatus = G_Datamanager->_TaioistStatus.find(1);
					NewCharacterStatus = *(*FindStatus).second;
				}
				break;
			case en_GameObjectType::OBJECT_THIEF_PLAYER:
				{
					auto FindStatus = G_Datamanager->_ThiefStatus.find(1);
					NewCharacterStatus = *(*FindStatus).second;
				}
				break;
			case en_GameObjectType::OBJECT_ARCHER_PLAYER:
				{
					auto FindStatus = G_Datamanager->_ArcherStatus.find(1);
					NewCharacterStatus = *(*FindStatus).second;
				}
				break;
			case en_GameObjectType::OBJECT_PLAYER_DUMMY:
				{
					auto FindStatus = G_Datamanager->_WarriorStatus.find(1);
					NewCharacterStatus = *(*FindStatus).second;
				}
				break;
			default:
				break;
			}

			int32 CurrentDP = 0;

			// 앞서 읽어온 캐릭터 정보를 토대로 DB에 저장
			// DBConnection Pool에서 DB연결을 위해서 하나를 꺼내온다.
			CDBConnection* NewCharacterPushDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			// GameServerDB에 새로운 캐릭터 저장하는 프로시저 클래스

			int16 PlayerType = ReqGameObjectType;
			int64 CurrentExperience = 0;

			auto FindLevel = G_Datamanager->_LevelDatas.find(NewCharacterStatus.Level);
			st_LevelData LevelData = *(*FindLevel).second;

			int32 NewCharacterPositionY = 26;
			int32 NewCharacterPositionX = 17;

			SP::CDBGameServerCreateCharacterPush NewCharacterPush(*NewCharacterPushDBConnection);
			NewCharacterPush.InAccountID(Session->AccountId);
			NewCharacterPush.InPlayerName(Session->CreateCharacterName);
			NewCharacterPush.InPlayerType(PlayerType);
			NewCharacterPush.InPlayerIndex(ReqCharacterCreateSlotIndex);
			NewCharacterPush.InLevel(NewCharacterStatus.Level);
			NewCharacterPush.InCurrentHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InMaxHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InCurrentMP(NewCharacterStatus.MaxMP);
			NewCharacterPush.InMaxMP(NewCharacterStatus.MaxMP);
			NewCharacterPush.InCurrentDP(CurrentDP);
			NewCharacterPush.InMaxDP(NewCharacterStatus.MaxDP);
			NewCharacterPush.InAutoRecoveryHPPercent(NewCharacterStatus.AutoRecoveryHPPercent);
			NewCharacterPush.InAutoRecoveryMPPercent(NewCharacterStatus.AutoRecoveryMPPercent);
			NewCharacterPush.InMinMeleeAttackDamage(NewCharacterStatus.MinMeleeAttackDamage);
			NewCharacterPush.InMaxMeleeAttackDamage(NewCharacterStatus.MaxMeleeAttackDamage);
			NewCharacterPush.InMeleeAttackHitRate(NewCharacterStatus.MeleeAttackHitRate);
			NewCharacterPush.InMagicDamage(NewCharacterStatus.MagicDamage);
			NewCharacterPush.InMagicHitRate(NewCharacterStatus.MagicHitRate);
			NewCharacterPush.InDefence(NewCharacterStatus.Defence);
			NewCharacterPush.InEvasionRate(NewCharacterStatus.EvasionRate);
			NewCharacterPush.InMeleeCriticalPoint(NewCharacterStatus.MeleeCriticalPoint);
			NewCharacterPush.InMagicCriticalPoint(NewCharacterStatus.MagicCriticalPoint);
			NewCharacterPush.InSpeed(NewCharacterStatus.Speed);
			NewCharacterPush.InLastPositionY(NewCharacterPositionY);
			NewCharacterPush.InLastPositionX(NewCharacterPositionX);
			NewCharacterPush.InCurrentExperence(CurrentExperience);
			NewCharacterPush.InRequireExperience(LevelData.RequireExperience);
			NewCharacterPush.InTotalExperience(LevelData.TotalExperience);

			// DB 요청 실행
			bool SaveNewCharacterQuery = NewCharacterPush.Execute();
			if (SaveNewCharacterQuery == true)
			{				
				// 앞서 저장한 캐릭터의 DBId를 AccountId를 이용해 얻어온다.
				CDBConnection* PlayerDBIDGetDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				// GameServerDB에 생성한 캐릭터의 PlayerDBId를 읽어오는 프로시저 클래스
				SP::CDBGameServerPlayerDBIDGet PlayerDBIDGet(*PlayerDBIDGetDBConnection);
				PlayerDBIDGet.InAccountID(Session->AccountId);
				PlayerDBIDGet.InPlayerSlotIndex(ReqCharacterCreateSlotIndex);

				PlayerDBIDGet.OutPlayerDBID(PlayerDBId);

				// DB 요청 실행
				bool GetNewCharacterPlayerDBId = PlayerDBIDGet.Execute();

				if (GetNewCharacterPlayerDBId == true)
				{
					PlayerDBIDGet.Fetch();					
					
					// DB에 등록한 새 캐릭터 정보
					CPlayer* NewPlayerCharacter = (CPlayer*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_PLAYER);
					NewPlayerCharacter->_GameObjectInfo.ObjectId = PlayerDBId;
					NewPlayerCharacter->_GameObjectInfo.ObjectName = Session->CreateCharacterName;
					NewPlayerCharacter->_GameObjectInfo.OwnerObjectType = en_GameObjectType::NORMAL;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Level = NewCharacterStatus.Level;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.DP = CurrentDP;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
					NewPlayerCharacter->_SpawnPosition._Y = NewCharacterPositionY;
					NewPlayerCharacter->_SpawnPosition._X = NewCharacterPositionX;
					NewPlayerCharacter->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
					NewPlayerCharacter->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
					NewPlayerCharacter->_GameObjectInfo.ObjectType = (en_GameObjectType)PlayerType;
					NewPlayerCharacter->_GameObjectInfo.OwnerObjectId = 0;
					NewPlayerCharacter->_GameObjectInfo.PlayerSlotIndex = ReqCharacterCreateSlotIndex;
					NewPlayerCharacter->_SessionId = Session->SessionId;
					NewPlayerCharacter->_AccountId = Session->AccountId;
					NewPlayerCharacter->_Experience.CurrentExperience = 0;
					NewPlayerCharacter->_Experience.RequireExperience = LevelData.RequireExperience;
					NewPlayerCharacter->_Experience.TotalExperience = LevelData.TotalExperience;					
					
					G_ObjectManager->_PlayersArray[Session->MyPlayerIndexes[ReqCharacterCreateSlotIndex]] = NewPlayerCharacter;

					// Gold Table 생성
					CDBConnection* DBGoldTableCreateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
					SP::CDBGameServerGoldTableCreatePush GoldTableCreate(*DBGoldTableCreateConnection);
					GoldTableCreate.InAccountDBId(Session->AccountId);
					GoldTableCreate.InPlayerDBId(PlayerDBId);

					GoldTableCreate.Execute();

					G_DBConnectionPool->Push(en_DBConnect::GAME, DBGoldTableCreateConnection);

					// DB에 인벤토리 생성
					for (int8 SlotIndex = 0; SlotIndex < (int8)en_Inventory::INVENTORY_SIZE; SlotIndex++)
					{
						CDBConnection* DBItemToInventoryConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
						SP::CDBGameServerItemCreateToInventory ItemToInventory(*DBItemToInventoryConnection);
						st_ItemInfo NewItem;

						NewItem.ItemName = L"";
						NewItem.ItemThumbnailImagePath = L"";

						ItemToInventory.InItemName(NewItem.ItemName);
						ItemToInventory.InSlotIndex(SlotIndex);
						ItemToInventory.InThumbnailImagePath(NewItem.ItemThumbnailImagePath);
						ItemToInventory.InOwnerAccountId(Session->AccountId);
						ItemToInventory.InOwnerPlayerId(PlayerDBId);

						ItemToInventory.Execute();

						G_DBConnectionPool->Push(en_DBConnect::GAME, DBItemToInventoryConnection);
					}

					int16 DefaultKey = (int16)UnityEngine::Alpha1;

					// DB에 퀵슬롯바 생성
					for (int8 SlotIndex = 0; SlotIndex < (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SIZE; ++SlotIndex)
					{
						CDBConnection* DBQuickSlotCreateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
						SP::CDBGameServerQuickSlotBarSlotCreate QuickSlotBarSlotCreate(*DBQuickSlotCreateConnection);
																
						int8 QuickSlotBarIndex = SlotIndex;
						int8 QuickSlotBarSlotIndex;
						int16 QuickSlotBarKey = 0;
						int8 SkillLargeCategory = (int8)(en_SkillLargeCategory::SKILL_LARGE_CATEGORY_NONE);
						int8 SkillMediumCategory = (int8)(en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_NONE);
						int16 SkillType = (int16)(en_SkillType::SKILL_TYPE_NONE);
						int8 SkillLevel = 0;
						wstring SkillName = L"";
						int32 SkillCoolTime = 0;
						int32 SkillCastingTime = 0;
						wstring SkillThumbnailImagePath;

						for (int8 i = 0; i < (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SLOT_SIZE; ++i)
						{	
							QuickSlotBarSlotIndex = i;

							if (DefaultKey == (int16)UnityEngine::Colon)
							{
								DefaultKey = (int16)UnityEngine::Alpha0;
							}

							QuickSlotBarKey = DefaultKey;

							QuickSlotBarSlotCreate.InAccountDBId(Session->AccountId);
							QuickSlotBarSlotCreate.InPlayerDBId(PlayerDBId);
							QuickSlotBarSlotCreate.InQuickSlotBarIndex(SlotIndex);
							QuickSlotBarSlotCreate.InQuickSlotBarSlotIndex(QuickSlotBarSlotIndex);
							QuickSlotBarSlotCreate.InQuickSlotKey(QuickSlotBarKey);
							QuickSlotBarSlotCreate.InSkillLargeCategory(SkillLargeCategory);
							QuickSlotBarSlotCreate.InSkillMediumCategory(SkillMediumCategory);
							QuickSlotBarSlotCreate.InSkillType(SkillType);
							QuickSlotBarSlotCreate.InSkillLevel(SkillLevel);

							QuickSlotBarSlotCreate.Execute();

							DefaultKey++;
						}

						G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotCreateConnection);
					}

					// 기본 스킬 생성
					PlayerLevelUpSkillCreate(Session->AccountId, NewPlayerCharacter->_GameObjectInfo, ReqCharacterCreateSlotIndex);
					
					// 기본 공격 스킬 퀵슬롯 1번에 등록
					st_QuickSlotBarSlotInfo DefaultAttackSkillQuickSlotInfo;
					DefaultAttackSkillQuickSlotInfo.AccountDBId = Session->AccountId;
					DefaultAttackSkillQuickSlotInfo.PlayerDBId = NewPlayerCharacter->_GameObjectInfo.ObjectId;
					DefaultAttackSkillQuickSlotInfo.QuickSlotBarIndex = 0;
					DefaultAttackSkillQuickSlotInfo.QuickSlotBarSlotIndex = 0;
					DefaultAttackSkillQuickSlotInfo.QuickSlotKey = 49;

					int8 DefaultSkillLargeCategory = (int8)en_SkillLargeCategory::SKILL_LARGE_CATEOGRY_PUBLIC;
					int8 DefaultSkillMediumCategory = (int8)en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK;
					int16 DefaultSkillType = (int16)en_SkillType::SKILL_DEFAULT_ATTACK;
					int8 DefaultAttackSkillLevel = 1;

					CDBConnection* DBQuickSlotSaveConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
					SP::CDBGameServerQuickSlotBarSlotUpdate QuickSlotUpdate(*DBQuickSlotSaveConnection);
					QuickSlotUpdate.InAccountDBId(DefaultAttackSkillQuickSlotInfo.AccountDBId);
					QuickSlotUpdate.InPlayerDBId(DefaultAttackSkillQuickSlotInfo.PlayerDBId);
					QuickSlotUpdate.InQuickSlotBarIndex(DefaultAttackSkillQuickSlotInfo.QuickSlotBarIndex);
					QuickSlotUpdate.InQuickSlotBarSlotIndex(DefaultAttackSkillQuickSlotInfo.QuickSlotBarSlotIndex);
					QuickSlotUpdate.InQuickSlotKey(DefaultAttackSkillQuickSlotInfo.QuickSlotKey);
					QuickSlotUpdate.InSkillLargeCategory(DefaultSkillLargeCategory);
					QuickSlotUpdate.InSkillMediumCategory(DefaultSkillMediumCategory);
					QuickSlotUpdate.InSkillType(DefaultSkillType);
					QuickSlotUpdate.InSkillLevel(DefaultAttackSkillLevel);

					QuickSlotUpdate.Execute();

					// 캐릭터 생성 응답 보냄
					CMessage* ResCreateCharacterMessage = MakePacketResCreateCharacter(!CharacterNameFind, NewPlayerCharacter->_GameObjectInfo);
					SendPacket(Session->SessionId, ResCreateCharacterMessage);
					ResCreateCharacterMessage->Free();
				}				

				G_DBConnectionPool->Push(en_DBConnect::GAME, PlayerDBIDGetDBConnection);
			}
			else
			{

			}

			// DBConnection 반납
			G_DBConnectionPool->Push(en_DBConnect::GAME, NewCharacterPushDBConnection);
				}
		else
		{
			// 캐릭터가 이미 DB에 생성되어 있는 경우
		}		
#pragma endregion

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

	int16 SpawnItemOwnerType;
	*Message >> SpawnItemOwnerType;

	int32 DropItemDataType;
	*Message >> DropItemDataType;

	Message->Free();

	bool Find = false;
	st_ItemData DropItemData;

	random_device RD;
	mt19937 Gen(RD());
	uniform_real_distribution<float> RandomDropPoint(0, 1); // 0.0 ~ 1.0	
	float RandomPoint = 100 * RandomDropPoint(Gen);

	int32 Sum = 0;

	switch ((en_GameObjectType)SpawnItemOwnerType)
	{
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
	{
		auto FindMonsterDropItem = G_Datamanager->_Monsters.find(DropItemDataType);
		st_MonsterData MonsterData = *(*FindMonsterDropItem).second;

		for (st_DropData DropItem : MonsterData.DropItems)
		{
			Sum += DropItem.Probability;

			if (Sum >= RandomPoint)
			{
				Find = true;
				// 드랍 확정 되면 해당 아이템 읽어오기
				auto FindDropItemInfo = G_Datamanager->_Items.find((int16)DropItem.DropItemSmallCategory);
				if (FindDropItemInfo == G_Datamanager->_Items.end())
				{
					CRASH("DropItemInfo를 찾지 못함");
				}

				DropItemData = *(*FindDropItemInfo).second;

				uniform_int_distribution<int> RandomDropItemCount(DropItem.MinCount, DropItem.MaxCount);
				DropItemData.ItemCount = RandomDropItemCount(Gen);
				DropItemData.SmallItemCategory = DropItem.DropItemSmallCategory;
				break;
			}
		}
	}
	break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
	{
		auto FindEnvironmentDropItem = G_Datamanager->_Environments.find(DropItemDataType);
		st_EnvironmentData EnvironmentData = *(*FindEnvironmentDropItem).second;

		for (st_DropData DropItem : EnvironmentData.DropItems)
		{
			Sum += DropItem.Probability;

			if (Sum >= RandomPoint)
			{
				Find = true;
				// 드랍 확정 되면 해당 아이템 읽어오기
				auto FindDropItemInfo = G_Datamanager->_Items.find((int16)DropItem.DropItemSmallCategory);
				if (FindDropItemInfo == G_Datamanager->_Items.end())
				{
					CRASH("DropItemInfo를 찾지 못함");
				}

				DropItemData = *(*FindDropItemInfo).second;

				uniform_int_distribution<int> RandomDropItemCount(DropItem.MinCount, DropItem.MaxCount);
				DropItemData.ItemCount = RandomDropItemCount(Gen);
				DropItemData.SmallItemCategory = DropItem.DropItemSmallCategory;
				break;
			}
		}
	}
	break;
	}

	if (Find == true)
	{
		bool ItemIsQuickSlotUse = false;
		int8 ItemLargeCategory = 0;
		int8 ItemMediumCategory = 0;
		int16 ItemSmallCategory = 0;
		wstring ItemName;
		int16 ItemCount = 0;
		wstring ItemThumbnailImagePath;
		bool ItemEquipped = false;
		int8 ItemSlotIndex = -1;
		int32 ItemMinDamage = 0;
		int32 ItemMaxDamage = 0;
		int32 ItemDefence = 0;
		int32 ItemMaxCount = 0;

		switch (DropItemData.SmallItemCategory)
		{
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
		{
			ItemIsQuickSlotUse = false;
			ItemLargeCategory = (int8)DropItemData.LargeItemCategory;
			ItemMediumCategory = (int8)DropItemData.MediumItemCategory;
			ItemSmallCategory = (int16)DropItemData.SmallItemCategory;
			ItemName = (LPWSTR)CA2W(DropItemData.ItemName.c_str());
			ItemCount = DropItemData.ItemCount;
			ItemEquipped = false;
			ItemThumbnailImagePath = (LPWSTR)CA2W(DropItemData.ItemThumbnailImagePath.c_str());

			st_ItemData* WeaponItemData = (*G_Datamanager->_Items.find((int16)DropItemData.SmallItemCategory)).second;
			ItemMinDamage = WeaponItemData->ItemMinDamage;
			ItemMaxDamage = WeaponItemData->ItemMaxDamage;
		}
		break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
		{
			ItemIsQuickSlotUse = false;
			ItemLargeCategory = (int8)DropItemData.LargeItemCategory;
			ItemMediumCategory = (int8)DropItemData.MediumItemCategory;
			ItemSmallCategory = (int16)DropItemData.SmallItemCategory;
			ItemName = (LPWSTR)CA2W(DropItemData.ItemName.c_str());
			ItemCount = DropItemData.ItemCount;
			ItemEquipped = false;
			ItemThumbnailImagePath = (LPWSTR)CA2W(DropItemData.ItemThumbnailImagePath.c_str());

			st_ItemData* ArmorItemData = (*G_Datamanager->_Items.find((int16)DropItemData.SmallItemCategory)).second;
			ItemDefence = ArmorItemData->ItemDefence;
		}
		break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
			break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK:
			break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
		{
			ItemIsQuickSlotUse = false;
			ItemLargeCategory = (int8)DropItemData.LargeItemCategory;
			ItemMediumCategory = (int8)DropItemData.MediumItemCategory;
			ItemSmallCategory = (int16)DropItemData.SmallItemCategory;
			ItemName = (LPWSTR)CA2W(DropItemData.ItemName.c_str());
			ItemCount = DropItemData.ItemCount;
			ItemEquipped = false;
			ItemThumbnailImagePath = (LPWSTR)CA2W(DropItemData.ItemThumbnailImagePath.c_str());

			st_ItemData* MaterialItemData = (*G_Datamanager->_Items.find((int16)DropItemData.SmallItemCategory)).second;
			ItemMaxCount = MaterialItemData->ItemMaxCount;
		}
		break;
		}

		int64 ItemDBId = 0;

		// 아이템 DB에 생성
		CDBConnection* DBCreateItemConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerCreateItem CreateItem(*DBCreateItemConnection);

		bool ItemUse = false;

		CreateItem.InItemUse(ItemUse);
		CreateItem.InIsQuickSlotUse(ItemIsQuickSlotUse);
		CreateItem.InItemLargeCategory(ItemLargeCategory);
		CreateItem.InItemMediumCategory(ItemMediumCategory);
		CreateItem.InItemSmallCategory(ItemSmallCategory);
		CreateItem.InItemName(ItemName);
		CreateItem.InItemMinDamage(ItemMinDamage);
		CreateItem.InItemMaxDamage(ItemMaxDamage);
		CreateItem.InItemDefence(ItemDefence);
		CreateItem.InItemMaxCount(ItemMaxCount);
		CreateItem.InItemCount(ItemCount);
		CreateItem.InIsEquipped(ItemEquipped);
		CreateItem.InThumbnailImagePath(ItemThumbnailImagePath);

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

				G_DBConnectionPool->Push(en_DBConnect::GAME, DBItemDBIdGetConnection);

				G_Logger->WriteStdOut(en_Color::RED, L"ItemDBId %d\n", ItemDBId);

				// 아이템이 스폰 될 위치 지정
				st_Vector2Int SpawnPosition(SpawnPositionX, SpawnPositionY);

				// 아이템 생성 후 스폰
				switch (DropItemData.SmallItemCategory)
				{
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
				{
					CWeapon* NewWeaponItem = (CWeapon*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_WEAPON);
					NewWeaponItem->_ItemInfo.ItemDBId = ItemDBId;
					NewWeaponItem->_ItemInfo.ItemIsQuickSlotUse = ItemIsQuickSlotUse;
					NewWeaponItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
					NewWeaponItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
					NewWeaponItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
					NewWeaponItem->_ItemInfo.ItemName = ItemName;
					NewWeaponItem->_ItemInfo.ItemMinDamage = ItemMinDamage;
					NewWeaponItem->_ItemInfo.ItemMaxDamage = ItemMaxDamage;
					NewWeaponItem->_ItemInfo.ItemCount = ItemCount;
					NewWeaponItem->_ItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
					NewWeaponItem->_ItemInfo.ItemIsEquipped = ItemEquipped;
					NewWeaponItem->_ItemInfo.ItemSlotIndex = ItemSlotIndex;

					NewWeaponItem->_GameObjectInfo.ObjectType = DropItemData.ItemObjectType;
					NewWeaponItem->_GameObjectInfo.ObjectId = ItemDBId;
					NewWeaponItem->_GameObjectInfo.OwnerObjectId = KillerId;
					NewWeaponItem->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)KillerObjectType;
					NewWeaponItem->_SpawnPosition = SpawnPosition;
					NewWeaponItem->_SpawnPosition = SpawnPosition;

					G_ObjectManager->ObjectEnterGame(NewWeaponItem, 1);
				}
				break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
				{
					CArmor* NewArmorItem = (CArmor*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_ARMOR);
					NewArmorItem->_ItemInfo.ItemDBId = ItemDBId;
					NewArmorItem->_ItemInfo.ItemIsQuickSlotUse = ItemIsQuickSlotUse;
					NewArmorItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
					NewArmorItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
					NewArmorItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
					NewArmorItem->_ItemInfo.ItemName = ItemName;
					NewArmorItem->_ItemInfo.ItemDefence = ItemDefence;
					NewArmorItem->_ItemInfo.ItemCount = ItemCount;
					NewArmorItem->_ItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
					NewArmorItem->_ItemInfo.ItemIsEquipped = ItemEquipped;
					NewArmorItem->_ItemInfo.ItemSlotIndex = ItemSlotIndex;

					NewArmorItem->_GameObjectInfo.ObjectType = DropItemData.ItemObjectType;
					NewArmorItem->_GameObjectInfo.ObjectId = ItemDBId;
					NewArmorItem->_GameObjectInfo.OwnerObjectId = KillerId;
					NewArmorItem->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)KillerObjectType;
					NewArmorItem->_SpawnPosition = SpawnPosition;

					G_ObjectManager->ObjectEnterGame(NewArmorItem, 1);
				}
				break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK:
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
				{
					CMaterial* NewMaterialItem = (CMaterial*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_MATERIAL);
					NewMaterialItem->_ItemInfo.ItemDBId = ItemDBId;
					NewMaterialItem->_ItemInfo.ItemIsQuickSlotUse = ItemIsQuickSlotUse;
					NewMaterialItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
					NewMaterialItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
					NewMaterialItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
					NewMaterialItem->_ItemInfo.ItemName = ItemName;
					NewMaterialItem->_ItemInfo.ItemMaxCount = ItemMaxCount;
					NewMaterialItem->_ItemInfo.ItemCount = ItemCount;
					NewMaterialItem->_ItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
					NewMaterialItem->_ItemInfo.ItemIsEquipped = ItemEquipped;
					NewMaterialItem->_ItemInfo.ItemSlotIndex = ItemSlotIndex;

					NewMaterialItem->_GameObjectInfo.ObjectType = DropItemData.ItemObjectType;
					NewMaterialItem->_GameObjectInfo.ObjectId = ItemDBId;
					NewMaterialItem->_GameObjectInfo.OwnerObjectId = KillerId;
					NewMaterialItem->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)KillerObjectType;
					NewMaterialItem->_SpawnPosition = SpawnPosition;

					G_ObjectManager->ObjectEnterGame(NewMaterialItem, 1);
				}
				break;
				}
			}
		}
	}
}

//(WORD)ItemType
//int32 Count
//int32 SlotNumber;
//int64 OwnerAccountId;
//bool IsEquipped

void CGameServer::PacketProcReqDBLootingItemToInventorySave(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		InterlockedDecrement64(&Session->IOBlock->IOCount);

		int64 TargetObjectId;
		*Message >> TargetObjectId;

		bool IsExistItem;
		*Message >> IsExistItem;

		int16 ItemEach;
		*Message >> ItemEach;

		int64 MapDeleteItemObjectId;
		*Message >> MapDeleteItemObjectId;

		CItem* Item = nullptr;
		*Message >> &Item;

		int64 OwnerAccountId;
		*Message >> OwnerAccountId;

		Message->Free();

		int64 ItemDBId = Item->_ItemInfo.ItemDBId;
		bool ItemIsQuickSlotUse = Item->_ItemInfo.ItemIsQuickSlotUse;
		int8 ItemLargeCategory = (int8)Item->_ItemInfo.ItemLargeCategory;
		int8 ItemMediumCategory = (int8)Item->_ItemInfo.ItemMediumCategory;
		int16 ItemSmallCategory = (int16)Item->_ItemInfo.ItemSmallCategory;
		wstring ItemName = Item->_ItemInfo.ItemName;
		int16 ItemCount = Item->_ItemInfo.ItemCount;
		wstring ItemThumbnailImagePath = Item->_ItemInfo.ItemThumbnailImagePath;
		bool ItemEquipped = Item->_ItemInfo.ItemIsEquipped;
		int8 ItemSlotIndex = Item->_ItemInfo.ItemSlotIndex;
		int32 ItemMinDamage = Item->_ItemInfo.ItemMinDamage;
		int32 ItemMaxDamage = Item->_ItemInfo.ItemMaxDamage;
		int32 ItemDefence = Item->_ItemInfo.ItemDefence;
		int32 ItemMaxCount = Item->_ItemInfo.ItemMaxCount;

		CDBConnection* ItemToInventorySaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

		// 아이템 Count 갱신
		if (IsExistItem == true)
		{
			SP::CDBGameServerItemRefreshPush ItemRefreshPush(*ItemToInventorySaveDBConnection);
			ItemRefreshPush.InAccountDBId(OwnerAccountId);
			ItemRefreshPush.InPlayerDBId(TargetObjectId);
			ItemRefreshPush.InItemType(ItemSmallCategory);
			ItemRefreshPush.InCount(ItemCount);
			ItemRefreshPush.InSlotIndex(ItemSlotIndex);

			ItemRefreshPush.Execute();
		}
		else
		{
			// 새로운 아이템 생성 후 Inventory DB 넣기			
			SP::CDBGameServerItemToInventoryPush ItemToInventoryPush(*ItemToInventorySaveDBConnection);
			ItemToInventoryPush.InIsQuickSlotUse(ItemIsQuickSlotUse);
			ItemToInventoryPush.InItemLargeCategory(ItemLargeCategory);
			ItemToInventoryPush.InItemMediumCategory(ItemMediumCategory);
			ItemToInventoryPush.InItemSmallCategory(ItemSmallCategory);
			ItemToInventoryPush.InItemName(ItemName);
			ItemToInventoryPush.InItemCount(ItemCount);
			ItemToInventoryPush.InSlotIndex(ItemSlotIndex);
			ItemToInventoryPush.InIsEquipped(ItemEquipped);
			ItemToInventoryPush.InItemMinDamage(ItemMinDamage);
			ItemToInventoryPush.InItemMaxDamage(ItemMaxDamage);
			ItemToInventoryPush.InItemDefence(ItemDefence);
			ItemToInventoryPush.InItemMaxCount(ItemMaxCount);
			ItemToInventoryPush.InThumbnailImagePath(ItemThumbnailImagePath);
			ItemToInventoryPush.InOwnerAccountId(OwnerAccountId);
			ItemToInventoryPush.InOwnerPlayerId(TargetObjectId);

			ItemToInventoryPush.Execute();
		}

		G_DBConnectionPool->Push(en_DBConnect::GAME, ItemToInventorySaveDBConnection);

		// 클라 인벤토리에서 해당 아이템을 저장
		CMessage* ResItemToInventoryPacket = MakePacketResItemToInventory(TargetObjectId, Item, ItemEach);
		SendPacket(Session->SessionId, ResItemToInventoryPacket);
		ResItemToInventoryPacket->Free();

		vector<int64> DeSpawnItem;
		DeSpawnItem.push_back(MapDeleteItemObjectId);

		// 클라에게 해당 아이템 삭제하라고 알려줌
		CMessage* ResItemDeSpawnPacket = MakePacketResObjectDeSpawn(1, DeSpawnItem);
		SendPacketFieldOfView(Session, ResItemDeSpawnPacket, true);		
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

void CGameServer::PacketProcReqDBCraftingItemToInventorySave(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		InterlockedDecrement64(&Session->IOBlock->IOCount);

		int64 TargetObjectId;
		*Message >> TargetObjectId;

		// 개수를 업데이트 해야할 재료의 개수
		int8 UpdateMaterialItemCount;
		*Message >> UpdateMaterialItemCount;

		for (int i = 0; i < UpdateMaterialItemCount; i++)
		{
			CItem* MaterialItem = nullptr;
			*Message >> &MaterialItem;

			int16 MaterialItemSmallCategory = (int16)MaterialItem->_ItemInfo.ItemSmallCategory;
			int16 MaterialItemCount = MaterialItem->_ItemInfo.ItemCount;
			int8 MaterialItemSlotIndex = MaterialItem->_ItemInfo.ItemSlotIndex;

			if (MaterialItemCount != 0)
			{
				// 재료템을 DB에서 개수 업데이트
				CDBConnection* CraftingItemToInventorySaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerItemRefreshPush InventoryItemCountRefreshPush(*CraftingItemToInventorySaveDBConnection);
				InventoryItemCountRefreshPush.InAccountDBId(Session->AccountId);
				InventoryItemCountRefreshPush.InPlayerDBId(TargetObjectId);
				InventoryItemCountRefreshPush.InItemType(MaterialItemSmallCategory);
				InventoryItemCountRefreshPush.InCount(MaterialItemCount);
				InventoryItemCountRefreshPush.InSlotIndex(MaterialItemSlotIndex);

				InventoryItemCountRefreshPush.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, CraftingItemToInventorySaveDBConnection);
			}
			else
			{
				// 만약 개수가 0개라면 해당 인벤토리 슬롯을 초기화 시킨다.
				wstring InitString = L"";

				CDBConnection* InventoryItemInitDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerInventorySlotInit InventoryItemInit(*InventoryItemInitDBConnection);
				InventoryItemInit.InOwnerAccountId(Session->AccountId);
				InventoryItemInit.InOwnerPlayerId(TargetObjectId);
				InventoryItemInit.InSlotIndex(MaterialItem->_ItemInfo.ItemSlotIndex);
				InventoryItemInit.InItemName(InitString);
				InventoryItemInit.InItemThumbnailImagePath(InitString);

				InventoryItemInit.Execute();

				// 게임에 입장한 캐릭터를 가져온다.
				CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

				MyPlayer->_Inventory.SlotInit(MaterialItem->_ItemInfo.ItemSlotIndex);

				G_DBConnectionPool->Push(en_DBConnect::GAME, InventoryItemInitDBConnection);
			}

			// 클라에게 업데이트 결과 전송
			CMessage* ReqInventoryItemUpdate = MakePacketInventoryItemUpdate(TargetObjectId, MaterialItem->_ItemInfo);
			SendPacket(SessionId, ReqInventoryItemUpdate);
			ReqInventoryItemUpdate->Free();
		}

		bool IsExistItem;
		*Message >> IsExistItem;

		int16 ItemEachCount;
		*Message >> ItemEachCount;

		CItem* CompleteItem = nullptr;
		*Message >> &CompleteItem;

		int64 OwnerAccountId;
		*Message >> OwnerAccountId;

		Message->Free();

		CDBConnection* CraftingItemToInventorySaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

		// 아이템 Count 갱신
		if (IsExistItem == true)
		{
			int16 CompleteItemSmallCategory = (int16)CompleteItem->_ItemInfo.ItemSmallCategory;
			int16 CompleteItemCount = CompleteItem->_ItemInfo.ItemCount;
			int8 CompleteItemSlotIndex = CompleteItem->_ItemInfo.ItemSlotIndex;

			SP::CDBGameServerItemRefreshPush ItemRefreshPush(*CraftingItemToInventorySaveDBConnection);
			ItemRefreshPush.InAccountDBId(OwnerAccountId);
			ItemRefreshPush.InPlayerDBId(TargetObjectId);
			ItemRefreshPush.InItemType(CompleteItemSmallCategory);
			ItemRefreshPush.InCount(CompleteItemCount);
			ItemRefreshPush.InSlotIndex(CompleteItemSlotIndex);

			ItemRefreshPush.Execute();
		}
		else
		{
			bool CompleteItemIsQuickSlotUse = CompleteItem->_ItemInfo.ItemIsQuickSlotUse;
			int8 CompleteItemLargeCategory = (int8)CompleteItem->_ItemInfo.ItemLargeCategory;
			int8 CompleteItemMediumCategory = (int8)CompleteItem->_ItemInfo.ItemMediumCategory;
			int16 CompleteItemSmallCategory = (int16)CompleteItem->_ItemInfo.ItemSmallCategory;
			wstring CompleteItemName = CompleteItem->_ItemInfo.ItemName;
			int32 CompleteItemMinDamage = CompleteItem->_ItemInfo.ItemMinDamage;
			int32 CompleteItemMaxDamage = CompleteItem->_ItemInfo.ItemMaxDamage;
			int32 CompleteItemDefence = CompleteItem->_ItemInfo.ItemDefence;
			int32 CompleteItemMaxCount = CompleteItem->_ItemInfo.ItemMaxCount;
			int16 CompleteItemCount = CompleteItem->_ItemInfo.ItemCount;
			wstring CompleteItemThumbnailImagePath = CompleteItem->_ItemInfo.ItemThumbnailImagePath;
			bool CompleteItemIsEquipped = CompleteItem->_ItemInfo.ItemIsEquipped;
			int8 CompleteItemSlotIndex = CompleteItem->_ItemInfo.ItemSlotIndex;

			// 새로운 아이템 생성 후 Inventory DB 넣기			
			SP::CDBGameServerItemToInventoryPush ItemToInventoryPush(*CraftingItemToInventorySaveDBConnection);
			ItemToInventoryPush.InIsQuickSlotUse(CompleteItemIsQuickSlotUse);
			ItemToInventoryPush.InItemLargeCategory(CompleteItemLargeCategory);
			ItemToInventoryPush.InItemMediumCategory(CompleteItemMediumCategory);
			ItemToInventoryPush.InItemSmallCategory(CompleteItemSmallCategory);
			ItemToInventoryPush.InItemName(CompleteItemName);
			ItemToInventoryPush.InItemMinDamage(CompleteItemMinDamage);
			ItemToInventoryPush.InItemMaxDamage(CompleteItemMaxDamage);
			ItemToInventoryPush.InItemDefence(CompleteItemDefence);
			ItemToInventoryPush.InItemMaxCount(CompleteItemMaxCount);
			ItemToInventoryPush.InItemCount(CompleteItemCount);
			ItemToInventoryPush.InSlotIndex(CompleteItemSlotIndex);
			ItemToInventoryPush.InIsEquipped(CompleteItemIsEquipped);
			ItemToInventoryPush.InThumbnailImagePath(CompleteItemThumbnailImagePath);
			ItemToInventoryPush.InOwnerAccountId(OwnerAccountId);
			ItemToInventoryPush.InOwnerPlayerId(TargetObjectId);

			ItemToInventoryPush.Execute();
		}

		G_DBConnectionPool->Push(en_DBConnect::GAME, CraftingItemToInventorySaveDBConnection);

		// 클라 인벤토리에서 해당 아이템을 저장
		CMessage* ResItemToInventoryPacket = MakePacketResItemToInventory(TargetObjectId, CompleteItem, ItemEachCount);
		SendPacket(Session->SessionId, ResItemToInventoryPacket);
		ResItemToInventoryPacket->Free();
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
		int8 ADBItemLargeCategory = 0;
		int8 ADBItemMediumCategory = 0;
		int16 ADBItemSmallCategory = -1;
		WCHAR AItemName[20] = { 0 };
		int16 AItemCount = -1;
		bool AItemEquipped = false;
		int32 AItemMinDamage = 0;
		int32 AItemMaxDamage = 0;
		int32 AItemDefence = 0;
		int32 AItemMaxCount = 0;
		WCHAR AItemThumbnailImagePath[100] = { 0 };
		ADBItemCheck.OutIsQuickSlotUse(AIsQuickSlotUse);
		ADBItemCheck.OutItemLargeCategory(ADBItemLargeCategory);
		ADBItemCheck.OutItemMediumCategory(ADBItemMediumCategory);
		ADBItemCheck.OutItemSmallCategory(ADBItemSmallCategory);
		ADBItemCheck.OutItemName(AItemName);
		ADBItemCheck.OutItemCount(AItemCount);
		ADBItemCheck.OutItemIsEquipped(AItemEquipped);
		ADBItemCheck.OutItemMinDamage(AItemMinDamage);
		ADBItemCheck.OutItemMaxDamage(AItemMaxDamage);
		ADBItemCheck.OutItemDefence(AItemDefence);
		ADBItemCheck.OutItemMaxCount(AItemMaxCount);
		ADBItemCheck.OutItemThumbnailImagePath(AItemThumbnailImagePath);

		ADBItemCheck.Execute();

		ADBItemCheck.Fetch();

		// 스왑 요청할 A 아이템 정보 셋팅
		st_ItemInfo SwapAItemInfo;
		SwapAItemInfo.ItemSlotIndex = SwapBIndex;
		SwapAItemInfo.ItemIsQuickSlotUse = AIsQuickSlotUse;
		SwapAItemInfo.ItemLargeCategory = (en_LargeItemCategory)ADBItemLargeCategory;
		SwapAItemInfo.ItemMediumCategory = (en_MediumItemCategory)ADBItemMediumCategory;
		SwapAItemInfo.ItemSmallCategory = (en_SmallItemCategory)ADBItemSmallCategory;
		SwapAItemInfo.ItemName = AItemName;
		SwapAItemInfo.ItemMinDamage = AItemMinDamage;
		SwapAItemInfo.ItemMaxDamage = AItemMaxDamage;
		SwapAItemInfo.ItemDefence = AItemDefence;
		SwapAItemInfo.ItemMaxCount = AItemMaxCount;
		SwapAItemInfo.ItemCount = AItemCount;
		SwapAItemInfo.ItemIsEquipped = AItemEquipped;
		SwapAItemInfo.ItemThumbnailImagePath = AItemThumbnailImagePath;

		G_DBConnectionPool->Push(en_DBConnect::GAME, AItemCheckDBConnection);

		// 스왑 요청한 B 아이템이 Inventory에 있는지 확인한다.
		CDBConnection* BItemCheckDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerItemCheck BDBItemCheck(*BItemCheckDBConnection);
		BDBItemCheck.InAccountDBId(AccountId);
		BDBItemCheck.InPlayerDBId(PlayerDBId);
		BDBItemCheck.InSlotIndex(SwapBIndex);

		bool BIsQuickSlotUse = false;
		int8 BDBItemLargeCategory = 0;
		int8 BDBItemMediumCategory = 0;
		int16 BDBItemSmallCategory = -1;
		WCHAR BItemName[20] = { 0 };
		int16 BItemCount = -1;
		bool BItemEquipped = false;
		int32 BItemMinDamage = 0;
		int32 BItemMaxDamage = 0;
		int32 BItemDefence = 0;
		int32 BItemMaxCount = 0;
		WCHAR BItemThumbnailImagePath[100] = { 0 };

		BDBItemCheck.OutIsQuickSlotUse(BIsQuickSlotUse);
		BDBItemCheck.OutItemLargeCategory(BDBItemLargeCategory);
		BDBItemCheck.OutItemMediumCategory(BDBItemMediumCategory);
		BDBItemCheck.OutItemSmallCategory(BDBItemSmallCategory);
		BDBItemCheck.OutItemName(BItemName);
		BDBItemCheck.OutItemCount(BItemCount);
		BDBItemCheck.OutItemIsEquipped(BItemEquipped);
		BDBItemCheck.OutItemMinDamage(BItemMinDamage);
		BDBItemCheck.OutItemMaxDamage(BItemMaxDamage);
		BDBItemCheck.OutItemDefence(BItemDefence);
		BDBItemCheck.OutItemMaxCount(BItemMaxCount);
		BDBItemCheck.OutItemThumbnailImagePath(BItemThumbnailImagePath);

		BDBItemCheck.Execute();

		BDBItemCheck.Fetch();

		// 스왑 요청할 B 아이템 정보 셋팅
		st_ItemInfo SwapBItemInfo;
		SwapBItemInfo.ItemSlotIndex = SwapAIndex;
		SwapBItemInfo.ItemIsQuickSlotUse = BIsQuickSlotUse;
		SwapBItemInfo.ItemLargeCategory = (en_LargeItemCategory)BDBItemLargeCategory;
		SwapBItemInfo.ItemMediumCategory = (en_MediumItemCategory)BDBItemMediumCategory;
		SwapBItemInfo.ItemSmallCategory = (en_SmallItemCategory)BDBItemSmallCategory;
		SwapBItemInfo.ItemName = BItemName;
		SwapBItemInfo.ItemMinDamage = BItemMinDamage;
		SwapBItemInfo.ItemMaxDamage = BItemMaxDamage;
		SwapBItemInfo.ItemDefence = BItemDefence;
		SwapBItemInfo.ItemMaxCount = BItemMaxCount;
		SwapBItemInfo.ItemCount = BItemCount;
		SwapBItemInfo.ItemIsEquipped = BItemEquipped;
		SwapBItemInfo.ItemThumbnailImagePath = BItemThumbnailImagePath;

		G_DBConnectionPool->Push(en_DBConnect::GAME, BItemCheckDBConnection);

		int16 AItemSmallCategory = (int16)SwapAItemInfo.ItemSmallCategory;
		int16 BItemSmallCategory = (int16)SwapBItemInfo.ItemSmallCategory;

		// DB에서 아이템 스왑
		CDBConnection* ItemSwapConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerItemSwap ItemSwap(*ItemSwapConnection);
		ItemSwap.InAccountDBId(AccountId);
		ItemSwap.InPlayerDBId(PlayerDBId);

		ItemSwap.InAIsQuickSlotUse(SwapBItemInfo.ItemIsQuickSlotUse);
		ItemSwap.InAItemLargeCategory(BDBItemLargeCategory);
		ItemSwap.InAItemMediumCategory(BDBItemMediumCategory);
		ItemSwap.InAItemSmallCategory(BItemSmallCategory);
		ItemSwap.InAItemName(SwapBItemInfo.ItemName);
		ItemSwap.InAItemMinDamage(SwapBItemInfo.ItemMinDamage);
		ItemSwap.InAItemMaxDamage(SwapBItemInfo.ItemMaxDamage);
		ItemSwap.InAItemDefence(SwapBItemInfo.ItemDefence);
		ItemSwap.InAItemMaxCount(SwapBItemInfo.ItemMaxCount);
		ItemSwap.InAItemCount(SwapBItemInfo.ItemCount);
		ItemSwap.InAItemIsEquipped(SwapBItemInfo.ItemIsEquipped);
		ItemSwap.InAItemThumbnailImagePath(SwapBItemInfo.ItemThumbnailImagePath);
		ItemSwap.InAItemSlotIndex(SwapBItemInfo.ItemSlotIndex);

		ItemSwap.InBIsQuickSlotUse(SwapAItemInfo.ItemIsQuickSlotUse);
		ItemSwap.InBItemLargeCategory(ADBItemLargeCategory);
		ItemSwap.InBItemMediumCategory(ADBItemMediumCategory);
		ItemSwap.InBItemSmallCategory(AItemSmallCategory);
		ItemSwap.InBItemName(SwapAItemInfo.ItemName);
		ItemSwap.InBItemMinDamage(SwapAItemInfo.ItemMinDamage);
		ItemSwap.InBItemMaxDamage(SwapAItemInfo.ItemMaxDamage);
		ItemSwap.InBItemDefence(SwapAItemInfo.ItemDefence);
		ItemSwap.InBItemMaxCount(SwapAItemInfo.ItemMaxCount);
		ItemSwap.InBItemCount(SwapAItemInfo.ItemCount);
		ItemSwap.InBItemIsEquipped(SwapAItemInfo.ItemIsEquipped);
		ItemSwap.InBItemThumbnailImagePath(SwapAItemInfo.ItemThumbnailImagePath);
		ItemSwap.InBItemSlotIndex(SwapAItemInfo.ItemSlotIndex);

		// Item Swap 성공
		bool SwapSuccess = ItemSwap.Execute();
		if (SwapSuccess == true)
		{
			CPlayer* TargetPlayer = (CPlayer*)G_ObjectManager->Find(PlayerDBId, en_GameObjectType::OBJECT_PLAYER);

			TargetPlayer->_Inventory.SwapItem(SwapAIndex, SwapBIndex);

			// Swap 요청 응답 보내기
			CMessage* ResItemSwapPacket = MakePacketResItemSwap(AccountId, PlayerDBId, SwapBItemInfo, SwapAItemInfo);
			SendPacket(Session->SessionId, ResItemSwapPacket);
			ResItemSwapPacket->Free();
		}

		G_DBConnectionPool->Push(en_DBConnect::GAME, ItemSwapConnection);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBItemUpdate(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		InterlockedDecrement64(&Session->IOBlock->IOCount);

		CItem* UseItem = nullptr;
		*Message >> &UseItem;

		Message->Free();

		// 게임에 입장한 캐릭터를 가져온다.
		CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

		switch (UseItem->_ItemInfo.ItemSmallCategory)
		{
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
		{
			MyPlayer->_Equipment.ItemEquip(UseItem, MyPlayer);

			// DB 아이템 업데이트
			CDBConnection* DBInventoryItemUpdateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerInventoryItemUpdate ItemUpdate(*DBInventoryItemUpdateConnection);
			ItemUpdate.InOwnerAccountId(Session->AccountId);
			ItemUpdate.InOwnerPlayerId(MyPlayer->_GameObjectInfo.ObjectId);
			ItemUpdate.InSlotIndex(UseItem->_ItemInfo.ItemSlotIndex);
			ItemUpdate.InItemCount(UseItem->_ItemInfo.ItemCount);
			ItemUpdate.InIsEquipped(UseItem->_ItemInfo.ItemIsEquipped);

			ItemUpdate.Execute();

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBInventoryItemUpdateConnection);

			// 클라에게 장비 착용 결과 알려줌
			CGameServerMessage* ResInventoryItemUsePacket = MakePacketEquipmentUpdate(MyPlayer->_GameObjectInfo.ObjectId, UseItem->_ItemInfo);
			SendPacket(Session->SessionId, ResInventoryItemUsePacket);
			ResInventoryItemUsePacket->Free();
		}
		break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
		{
			MyPlayer->_GameObjectInfo.ObjectStatInfo.HP += 50;

			CGameServerMessage* ResChangeObjectStat = MakePacketResChangeObjectStat(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectStatInfo);
			SendPacketFieldOfView(Session, ResChangeObjectStat, true);			
			ResChangeObjectStat->Free();
		}
		break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_FIERCE_ATTACK:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE:
		case en_SmallItemCategory::ITEM_SMALL_CATEOGRY_SKILLBOOK_SHAMAN_HEALING_LIGHT:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HEALING_WIND:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE:
		{
			st_ConsumableData* ConsumableSkillItemData = (*G_Datamanager->_Consumables.find((int16)UseItem->_ItemInfo.ItemSmallCategory)).second;
			if (ConsumableSkillItemData == nullptr)
			{
				CRASH("스킬아이템 데이터를 찾을 수 없습니다.");
			}

			// 스킬 데이터 찾기
			st_AttackSkillData* FindAttackSkillData = (st_AttackSkillData*)G_Datamanager->FindSkillData(ConsumableSkillItemData->SkillMediumCategory, ConsumableSkillItemData->SkillType);
			if (FindAttackSkillData == nullptr)
			{
				CRASH("스킬 데이터를 찾을 수 없습니다.");
			}

			// 스킬을 이미 습득했는지 확인
			st_SkillInfo* FindSkill = MyPlayer->_SkillBox.FindSkill(FindAttackSkillData->SkillType);
			if (FindSkill != nullptr)
			{
				wstring ErrorDistance;

				WCHAR ErrorMessage[100] = { 0 };

				wsprintf(ErrorMessage, L"[%s] 이미 습득한 스킬입니다.", FindSkill->SkillName.c_str());
				ErrorDistance = ErrorMessage;

				CMessage* ResErrorPacket = MakePacketError(MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_NON_SELECT_OBJECT, ErrorDistance);
				SendPacket(Session->SessionId, ResErrorPacket);
				ResErrorPacket->Free();

				break;
			}

			st_AttackSkillInfo* NewAttackSkillInfo = new st_AttackSkillInfo();
			NewAttackSkillInfo->IsQuickSlotUse = false;
			NewAttackSkillInfo->SkillLargeCategory = FindAttackSkillData->SkillLargeCategory;
			NewAttackSkillInfo->SkillMediumCategory = FindAttackSkillData->SkillMediumCategory;
			NewAttackSkillInfo->SkillType = FindAttackSkillData->SkillType;
			NewAttackSkillInfo->SkillLevel = 1;
			NewAttackSkillInfo->SkillName = (LPWSTR)CA2W(FindAttackSkillData->SkillName.c_str());
			NewAttackSkillInfo->SkillCoolTime = FindAttackSkillData->SkillCoolTime;
			NewAttackSkillInfo->SkillCastingTime = FindAttackSkillData->SkillCastingTime;
			NewAttackSkillInfo->SkillImagePath = (LPWSTR)CA2W(FindAttackSkillData->SkillThumbnailImagePath.c_str());
			NewAttackSkillInfo->SkillMinDamage = FindAttackSkillData->SkillMinDamage;
			NewAttackSkillInfo->SkillMaxDamage = FindAttackSkillData->SkillMaxDamage;

			int8 SkillLargeCategory = (int8)NewAttackSkillInfo->SkillLargeCategory;
			int8 SkillMediumCategory = (int8)NewAttackSkillInfo->SkillMediumCategory;
			int16 SkillType = (int16)NewAttackSkillInfo->SkillType;

			// 스킬 테이블에 배운 스킬 DB에 넣기
			CDBConnection* DBSkillToSkillBoxConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*DBSkillToSkillBoxConnection);
			SkillToSkillBox.InAccountDBId(Session->AccountId);
			SkillToSkillBox.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);
			SkillToSkillBox.InIsQuickSlotUse(NewAttackSkillInfo->IsQuickSlotUse);
			SkillToSkillBox.InSkillLargeCategory(SkillLargeCategory);
			SkillToSkillBox.InSkillMediumCategory(SkillMediumCategory);
			SkillToSkillBox.InSkillType(SkillType);
			SkillToSkillBox.InSkillLevel(NewAttackSkillInfo->SkillLevel);

			SkillToSkillBox.Execute();

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBSkillToSkillBoxConnection);

			MyPlayer->_SkillBox.AddSkill(NewAttackSkillInfo);

			// 새로 배운 스킬을 클라에게 전송한다.
			CMessage* ResSkillToSkillBoxPacket = MakePacketResSkillToSkillBox(MyPlayer->_GameObjectInfo.ObjectId, NewAttackSkillInfo);
			SendPacket(Session->SessionId, ResSkillToSkillBoxPacket);
			ResSkillToSkillBoxPacket->Free();

			// 사용을 다한 스킬북 인벤토리에서 제거하기	
			wstring InitString = L"";

			CDBConnection* InventoryItemInitDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerInventorySlotInit InventoryItemInit(*InventoryItemInitDBConnection);
			InventoryItemInit.InOwnerAccountId(Session->AccountId);
			InventoryItemInit.InOwnerPlayerId(MyPlayer->_GameObjectInfo.ObjectId);
			InventoryItemInit.InSlotIndex(UseItem->_ItemInfo.ItemSlotIndex);
			InventoryItemInit.InItemName(InitString);
			InventoryItemInit.InItemThumbnailImagePath(InitString);

			InventoryItemInit.Execute();

			G_DBConnectionPool->Push(en_DBConnect::GAME, InventoryItemInitDBConnection);

			MyPlayer->_Inventory.SlotInit(UseItem->_ItemInfo.ItemSlotIndex);

			// 클라에게 업데이트 결과 전송
			CMessage* ReqInventoryItemUpdate = MakePacketInventoryItemUpdate(MyPlayer->_GameObjectInfo.ObjectId, UseItem->_ItemInfo);
			SendPacket(SessionId, ReqInventoryItemUpdate);
			ReqInventoryItemUpdate->Free();
		}
		break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
		{
			wstring ErrorDistance;

			WCHAR ErrorMessage[100] = { 0 };

			wsprintf(ErrorMessage, L"[%s] 사용 할 수 없는 아이템 입니다.", UseItem->_ItemInfo.ItemName.c_str());
			ErrorDistance = ErrorMessage;

			CMessage* ResErrorPacket = MakePacketError(MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_NON_SELECT_OBJECT, ErrorDistance);
			SendPacket(Session->SessionId, ResErrorPacket);
			ResErrorPacket->Free();
		}
		break;
		default:
			CRASH("요청한 사용 아이템 타입이 올바르지 않음");
			break;
		}
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

		int16 SliverCoinCount;
		*Message >> SliverCoinCount;

		int16 BronzeCoinCount;
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
		CMessage* ResGoldSaveMeesage = MakePacketResGoldSave(AccountId, TargetId, GoldCoinCount, SliverCoinCount, BronzeCoinCount, ItemCount, ItemType);
		SendPacket(Session->SessionId, ResGoldSaveMeesage);
		ResGoldSaveMeesage->Free();

		// 돈 오브젝트 디스폰
		vector<int64> DeSpawnItem;
		DeSpawnItem.push_back(ItemDBId);

		CMessage* ResItemDeSpawnPacket = MakePacketResObjectDeSpawn(1, DeSpawnItem);
		SendPacketFieldOfView(Session, ResItemDeSpawnPacket, true);		
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

			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

#pragma region 캐릭터 경험치 정보 보내기
			CGameServerMessage* ResExperience = MakePacketExperience(MyPlayer->_AccountId, MyPlayer->_GameObjectInfo.ObjectId, 0, MyPlayer->_Experience.CurrentExperience, MyPlayer->_Experience.RequireExperience, MyPlayer->_Experience.TotalExperience);
			SendPacket(Session->SessionId, ResExperience);
			ResExperience->Free();
#pragma endregion

#pragma region 퀵슬롯 정보 가져와서 클라에 보내기
			// 퀵슬롯 정보 초기화
			MyPlayer->_QuickSlotManager.Init();

			vector<st_QuickSlotBarSlotInfo> QuickSlotBarSlotInfos;

			// 퀵슬롯 테이블 접근해서 해당 스킬이 등록되어 있는 모든 퀵슬롯 번호 가지고옴
			CDBConnection* DBQuickSlotBarGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotBarGet QuickSlotBarGet(*DBQuickSlotBarGetConnection);
			QuickSlotBarGet.InAccountDBId(MyPlayer->_AccountId);
			QuickSlotBarGet.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);

			int8 QuickSlotBarIndex;
			int8 QuickSlotBarSlotIndex;
			int16 QuickSlotKey;
			int8 QuickSlotSkillLargeCategory;
			int8 QuickSlotSkillMediumCategory;
			int16 QuickSlotSkillType;
			int8 QuickSlotSkillLevel;

			QuickSlotBarGet.OutQuickSlotBarIndex(QuickSlotBarIndex);
			QuickSlotBarGet.OutQuickSlotBarItemIndex(QuickSlotBarSlotIndex);
			QuickSlotBarGet.OutQuickSlotKey(QuickSlotKey);
			QuickSlotBarGet.OutQuickSlotSkillLargeCategory(QuickSlotSkillLargeCategory);
			QuickSlotBarGet.OutQuickSlotSkillMediumCategory(QuickSlotSkillMediumCategory);
			QuickSlotBarGet.OutQuickSlotSkillType(QuickSlotSkillType);
			QuickSlotBarGet.OutQuickSlotSkillLevel(QuickSlotSkillLevel);

			QuickSlotBarGet.Execute();

			while (QuickSlotBarGet.Fetch())
			{
				st_QuickSlotBarSlotInfo NewQuickSlotBarSlot;
				NewQuickSlotBarSlot.AccountDBId = MyPlayer->_AccountId;
				NewQuickSlotBarSlot.PlayerDBId = MyPlayer->_GameObjectInfo.ObjectId;
				NewQuickSlotBarSlot.QuickSlotBarIndex = QuickSlotBarIndex;
				NewQuickSlotBarSlot.QuickSlotBarSlotIndex = QuickSlotBarSlotIndex;
				NewQuickSlotBarSlot.QuickSlotKey = QuickSlotKey;
				NewQuickSlotBarSlot.QuickBarSkillInfo.SkillLargeCategory = (en_SkillLargeCategory)QuickSlotSkillLargeCategory;
				NewQuickSlotBarSlot.QuickBarSkillInfo.SkillMediumCategory = (en_SkillMediumCategory)QuickSlotSkillMediumCategory;
				NewQuickSlotBarSlot.QuickBarSkillInfo.SkillType = (en_SkillType)QuickSlotSkillType;
				NewQuickSlotBarSlot.QuickBarSkillInfo.SkillLevel = QuickSlotSkillLevel;

				// 퀵슬롯에 등록된 스킬 정보 찾기
				st_SkillData* FindSkillData = G_Datamanager->FindSkillData((en_SkillMediumCategory)QuickSlotSkillMediumCategory, (en_SkillType)QuickSlotSkillType);
				if (FindSkillData != nullptr)
				{
					NewQuickSlotBarSlot.QuickBarSkillInfo.SkillName = (LPWSTR)CA2W(FindSkillData->SkillName.c_str());
					NewQuickSlotBarSlot.QuickBarSkillInfo.SkillCoolTime = FindSkillData->SkillCoolTime;
					NewQuickSlotBarSlot.QuickBarSkillInfo.SkillCastingTime = FindSkillData->SkillCastingTime;
					NewQuickSlotBarSlot.QuickBarSkillInfo.SkillImagePath = (LPWSTR)CA2W(FindSkillData->SkillThumbnailImagePath.c_str());
				}

				// 퀵슬롯에 등록한다.
				MyPlayer->_QuickSlotManager.UpdateQuickSlotBar(NewQuickSlotBarSlot);
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
			MyPlayer->_Inventory.Init();

			vector<CItem*> InventoryItems;
			vector<st_ItemInfo> Equipments;

			// DB에 기록되어 있는 인벤토리 아이템들의 정보를 모두 긁어온다.
			CDBConnection* DBInventoryItemInfoGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerInventoryItemGet CharacterInventoryItem(*DBInventoryItemInfoGetConnection);
			CharacterInventoryItem.InAccountDBId(MyPlayer->_AccountId);
			CharacterInventoryItem.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);

			int8 ItemLargeCategory = 0;
			int8 ItemMediumCategory = 0;
			int16 ItemSmallCategory = 0;
			WCHAR ItemName[20] = { 0 };
			int16 ItemCount = 0;
			int8 SlotIndex = 0;
			bool IsEquipped = 0;
			int32 ItemMinDamage = 0;
			int32 ItemMaxDamage = 0;
			int32 ItemDefence = 0;
			int32 ItemMaxCount = 0;
			WCHAR ItemThumbnailImagePath[100] = { 0 };

			CharacterInventoryItem.OutItemLargeCategory(ItemLargeCategory);
			CharacterInventoryItem.OutItemMediumCategory(ItemMediumCategory);
			CharacterInventoryItem.OutItemSmallCategory(ItemSmallCategory);
			CharacterInventoryItem.OutItemName(ItemName);
			CharacterInventoryItem.OutMinDamage(ItemMinDamage);
			CharacterInventoryItem.OutMaxDamage(ItemMaxDamage);
			CharacterInventoryItem.OutDefence(ItemDefence);
			CharacterInventoryItem.OutMaxCount(ItemMaxCount);
			CharacterInventoryItem.OutItemCount(ItemCount);
			CharacterInventoryItem.OutSlotIndex(SlotIndex);
			CharacterInventoryItem.OutIsEquipped(IsEquipped);
			CharacterInventoryItem.OutItemThumbnailImagePath(ItemThumbnailImagePath);

			CharacterInventoryItem.Execute();

			while (CharacterInventoryItem.Fetch())
			{
				CWeapon* WeaponItem = nullptr;
				CArmor* ArmorItem = nullptr;
				CItem* InventoryItem = nullptr;
				CMaterial* MaterialItem = nullptr;
				CConsumable* SkillBookItem = nullptr;

				switch ((en_SmallItemCategory)ItemSmallCategory)
				{
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
					WeaponItem = (CWeapon*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_WEAPON);
					WeaponItem->_ItemInfo.ItemDBId = 0;
					WeaponItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
					WeaponItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
					WeaponItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
					WeaponItem->_ItemInfo.ItemName = ItemName;
					WeaponItem->_ItemInfo.ItemCount = ItemCount;
					WeaponItem->_ItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
					WeaponItem->_ItemInfo.ItemIsEquipped = IsEquipped;
					WeaponItem->_ItemInfo.ItemSlotIndex = SlotIndex;
					WeaponItem->_ItemInfo.ItemMinDamage = ItemMinDamage;
					WeaponItem->_ItemInfo.ItemMaxDamage = ItemMaxDamage;

					MyPlayer->_Inventory.AddItem(WeaponItem);

					// 아이템 테이블에서 읽어들인 무기 아이템이 착용중일 경우
					// 해당 무기를 착용하고 착용 아이템 정보에 담는다.
					if (WeaponItem->_ItemInfo.ItemIsEquipped == true)
					{
						MyPlayer->_Equipment.ItemEquip(WeaponItem, MyPlayer);
						Equipments.push_back(WeaponItem->_ItemInfo);
					}

					InventoryItems.push_back(WeaponItem);
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
					ArmorItem = (CArmor*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_ARMOR);
					ArmorItem->_ItemInfo.ItemDBId = 0;
					ArmorItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
					ArmorItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
					ArmorItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
					ArmorItem->_ItemInfo.ItemName = ItemName;
					ArmorItem->_ItemInfo.ItemCount = ItemCount;
					ArmorItem->_ItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
					ArmorItem->_ItemInfo.ItemIsEquipped = IsEquipped;
					ArmorItem->_ItemInfo.ItemSlotIndex = SlotIndex;
					ArmorItem->_ItemInfo.ItemDefence = ItemDefence;

					MyPlayer->_Inventory.AddItem(ArmorItem);

					// 아이템 테이블에서 읽어들인 방어구 아이템이 착용중일 경우
					// 해당 방어구를 착용하고 착용 아이템 정보에 담는다.
					if (ArmorItem->_ItemInfo.ItemIsEquipped == true)
					{
						MyPlayer->_Equipment.ItemEquip(ArmorItem, MyPlayer);
						Equipments.push_back(WeaponItem->_ItemInfo);
					}

					InventoryItems.push_back(ArmorItem);
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_FIERCE_ATTACK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE:
				case en_SmallItemCategory::ITEM_SMALL_CATEOGRY_SKILLBOOK_SHAMAN_HEALING_LIGHT:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HEALING_WIND:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE:
					SkillBookItem = (CConsumable*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_CONSUMABLE);
					SkillBookItem->_ItemInfo.ItemDBId = 0;
					SkillBookItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
					SkillBookItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
					SkillBookItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
					SkillBookItem->_ItemInfo.ItemName = ItemName;
					SkillBookItem->_ItemInfo.ItemCount = ItemCount;
					SkillBookItem->_ItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
					SkillBookItem->_ItemInfo.ItemIsEquipped = IsEquipped;
					SkillBookItem->_ItemInfo.ItemSlotIndex = SlotIndex;
					SkillBookItem->_ItemInfo.ItemMaxCount = ItemMaxCount;

					MyPlayer->_Inventory.AddItem(SkillBookItem);

					InventoryItems.push_back(SkillBookItem);
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
					MaterialItem = (CMaterial*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_MATERIAL);
					MaterialItem->_ItemInfo.ItemDBId = 0;
					MaterialItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
					MaterialItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
					MaterialItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
					MaterialItem->_ItemInfo.ItemName = ItemName;
					MaterialItem->_ItemInfo.ItemCount = ItemCount;
					MaterialItem->_ItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
					MaterialItem->_ItemInfo.ItemIsEquipped = IsEquipped;
					MaterialItem->_ItemInfo.ItemSlotIndex = SlotIndex;
					MaterialItem->_ItemInfo.ItemMaxCount = ItemMaxCount;

					MyPlayer->_Inventory.AddItem(MaterialItem);

					InventoryItems.push_back(MaterialItem);
					break;
				default:
					break;
				}
			}

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBInventoryItemInfoGetConnection);

			// 클라에게 인벤토리 생성 메세지
			CMessage* ResItemToInventoryPacket = MakePacketInventoryCreate((int8)en_Inventory::INVENTORY_SIZE, InventoryItems);
			SendPacket(Session->SessionId, ResItemToInventoryPacket);
			ResItemToInventoryPacket->Free();

#pragma endregion			

#pragma region 장비 정보 보내주기
			for (st_ItemInfo EquipmentItemInfo : Equipments)
			{
				CMessage* ResEquipmentItemInfoPacket = MakePacketEquipmentUpdate(MyPlayer->_GameObjectInfo.ObjectId, EquipmentItemInfo);
				SendPacket(Session->SessionId, ResEquipmentItemInfoPacket);
				ResEquipmentItemInfoPacket->Free();
			}
#pragma endregion


#pragma region 골드 정보 읽어오기
			// 캐릭터가 소유하고 있었던 골드 정보를 GoldTable에서 읽어온다.
			CDBConnection* DBCharacterGoldGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerGoldGet CharacterGoldGet(*DBCharacterGoldGetConnection);
			CharacterGoldGet.InAccountDBId(MyPlayer->_AccountId);
			CharacterGoldGet.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);

			int64 GoldCoin = 0;
			int16 SliverCoin = 0;
			int16 BronzeCoin = 0;

			CharacterGoldGet.OutGoldCoin(GoldCoin);
			CharacterGoldGet.OutSliverCoin(SliverCoin);
			CharacterGoldGet.OutBronzeCoin(BronzeCoin);

			if (CharacterGoldGet.Execute() && CharacterGoldGet.Fetch())
			{
				// DB에서 읽어온 Gold를 Inventory에 저장한다.
				MyPlayer->_Inventory._GoldCoinCount = GoldCoin;
				MyPlayer->_Inventory._SliverCoinCount = SliverCoin;
				MyPlayer->_Inventory._BronzeCoinCount = BronzeCoin;

				// DBConnection 반납하고
				G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterGoldGetConnection);

				// 클라에게 골드 정보를 보내준다.
				CMessage* ResGoldSaveMeesage = MakePacketResGoldSave(MyPlayer->_AccountId, MyPlayer->_GameObjectInfo.ObjectId, GoldCoin, SliverCoin, BronzeCoin, 0, 0, false);
				SendPacket(Session->SessionId, ResGoldSaveMeesage);
				ResGoldSaveMeesage->Free();
			}

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterGoldGetConnection);
#pragma endregion			

#pragma region 스킬 정보 읽어오기
			// 캐릭터가 소유하고 있는 스킬 정보를 DB로부터 읽어온다.
			CDBConnection* DBCharacterSkillGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerSkillGet CharacterSkillGet(*DBCharacterSkillGetConnection);
			CharacterSkillGet.InAccountDBId(MyPlayer->_AccountId);
			CharacterSkillGet.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);

			bool IsQuickSlotUse;
			int8 SkillLargeCategory;
			int8 SkillMediumCategory;
			int16 SkillType;
			int8 SkillLevel;

			CharacterSkillGet.OutIsQuickSlotUse(IsQuickSlotUse);
			CharacterSkillGet.OutSkillLargeCategory(SkillLargeCategory);
			CharacterSkillGet.OutSkillMediumCategory(SkillMediumCategory);
			CharacterSkillGet.OutSkillType(SkillType);
			CharacterSkillGet.OutSkillLevel(SkillLevel);

			CharacterSkillGet.Execute();

			while (CharacterSkillGet.Fetch())
			{
				// 스킬 데이터 찾기
				st_SkillData* FindSkillData = G_Datamanager->FindSkillData((en_SkillMediumCategory)SkillMediumCategory, (en_SkillType)SkillType);
				if (FindSkillData == nullptr)
				{
					CRASH("스킬 데이터를 찾을 수 없습니다.");
				}

				switch ((en_SkillMediumCategory)SkillMediumCategory)
				{
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK:
				{
					st_AttackSkillData* FindAttackSkillData = (st_AttackSkillData*)FindSkillData;

					st_AttackSkillInfo* AttackSkillInfo = new st_AttackSkillInfo();
					AttackSkillInfo->IsQuickSlotUse = IsQuickSlotUse;
					AttackSkillInfo->SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;
					AttackSkillInfo->SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;
					AttackSkillInfo->SkillType = (en_SkillType)SkillType;
					AttackSkillInfo->SkillLevel = SkillLevel;
					AttackSkillInfo->SkillName = (LPWSTR)CA2W(FindAttackSkillData->SkillName.c_str());
					AttackSkillInfo->SkillCoolTime = FindAttackSkillData->SkillCoolTime;
					AttackSkillInfo->SkillCastingTime = FindAttackSkillData->SkillCastingTime;
					AttackSkillInfo->SkillImagePath = (LPWSTR)CA2W(FindAttackSkillData->SkillThumbnailImagePath.c_str());
					AttackSkillInfo->SkillMinDamage = FindAttackSkillData->SkillMinDamage;
					AttackSkillInfo->SkillMaxDamage = FindAttackSkillData->SkillMaxDamage;
					AttackSkillInfo->SkillTargetEffectTime = FindAttackSkillData->SkillTargetEffectTime;

					AttackSkillInfo->SkillDebuf = FindAttackSkillData->SkillDebuf;
					AttackSkillInfo->SkillDebufTime = FindAttackSkillData->SkillDebufTime;
					AttackSkillInfo->SkillDebufAttackSpeed = FindAttackSkillData->SkillDebufAttackSpeed;
					AttackSkillInfo->SkillDebufMovingSpeed = FindAttackSkillData->SkillDebufMovingSpeed;
					AttackSkillInfo->SkillDebufStun = FindAttackSkillData->SkillDebufStun;
					AttackSkillInfo->SkillDebufPushAway = FindAttackSkillData->SkillDebufPushAway;
					AttackSkillInfo->SkillDebufRoot = FindAttackSkillData->SkillDebufRoot;
					AttackSkillInfo->SkillDamageOverTime = FindAttackSkillData->SkillDamageOverTime;
					AttackSkillInfo->StatusAbnormalityProbability = FindAttackSkillData->StatusAbnormalityProbability;

					MyPlayer->_SkillBox.AddSkill(AttackSkillInfo);

					// 클라가 소유하고 있는 스킬 정보를 보내준다.
					CMessage* ResSkillToSkillBoxPacket = MakePacketResSkillToSkillBox(MyPlayer->_GameObjectInfo.ObjectId, AttackSkillInfo);
					SendPacket(Session->SessionId, ResSkillToSkillBoxPacket);
					ResSkillToSkillBoxPacket->Free();
				}
				break;
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_HEAL:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_HEAL:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL:
				{
					st_HealSkillData* FindHealSkillData = (st_HealSkillData*)FindSkillData;

					st_HealSkillInfo* HealSkillInfo = new st_HealSkillInfo();
					HealSkillInfo->IsQuickSlotUse = IsQuickSlotUse;
					HealSkillInfo->SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;
					HealSkillInfo->SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;
					HealSkillInfo->SkillType = (en_SkillType)SkillType;
					HealSkillInfo->SkillLevel = SkillLevel;
					HealSkillInfo->SkillName = (LPWSTR)CA2W(FindHealSkillData->SkillName.c_str());
					HealSkillInfo->SkillCoolTime = FindHealSkillData->SkillCoolTime;
					HealSkillInfo->SkillCastingTime = FindHealSkillData->SkillCastingTime;
					HealSkillInfo->SkillImagePath = (LPWSTR)CA2W(FindHealSkillData->SkillThumbnailImagePath.c_str());
					HealSkillInfo->SkillMinHealPoint = FindHealSkillData->SkillMinHealPoint;
					HealSkillInfo->SkillMaxHealPoint = FindHealSkillData->SkillMaxHealPoint;
					HealSkillInfo->SkillTargetEffectTime = FindHealSkillData->SkillTargetEffectTime;

					MyPlayer->_SkillBox.AddSkill(HealSkillInfo);

					// 클라가 소유하고 있는 스킬 정보를 보내준다.
					CMessage* ResSkillToSkillBoxPacket = MakePacketResSkillToSkillBox(MyPlayer->_GameObjectInfo.ObjectId, HealSkillInfo);
					SendPacket(Session->SessionId, ResSkillToSkillBoxPacket);
					ResSkillToSkillBoxPacket->Free();
				}
				break;
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_BUF:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_BUF:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_BUF:
				{
					st_BufSkillData* FindBufSkillData = (st_BufSkillData*)FindSkillData;

					st_BufSkillInfo* BufSkillInfo = new st_BufSkillInfo();
					BufSkillInfo->IsQuickSlotUse = IsQuickSlotUse;
					BufSkillInfo->SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;
					BufSkillInfo->SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;
					BufSkillInfo->SkillType = (en_SkillType)SkillType;
					BufSkillInfo->SkillLevel = SkillLevel;
					BufSkillInfo->SkillName = (LPWSTR)CA2W(FindBufSkillData->SkillName.c_str());
					BufSkillInfo->SkillCoolTime = FindBufSkillData->SkillCoolTime;
					BufSkillInfo->SkillCastingTime = FindBufSkillData->SkillCastingTime;
					BufSkillInfo->SkillTargetEffectTime = FindBufSkillData->SkillTargetEffectTime;
					BufSkillInfo->SkillImagePath = (LPWSTR)CA2W(FindBufSkillData->SkillThumbnailImagePath.c_str());

					BufSkillInfo->IncreaseMinAttackPoint = FindBufSkillData->IncreaseMinAttackPoint;
					BufSkillInfo->IncreaseMaxAttackPoint = FindBufSkillData->IncreaseMaxAttackPoint;
					BufSkillInfo->IncreaseMeleeAttackSpeedPoint = FindBufSkillData->IncreaseMeleeAttackSpeedPoint;
					BufSkillInfo->IncreaseMeleeAttackHitRate = FindBufSkillData->IncreaseMeleeAttackHitRate;
					BufSkillInfo->IncreaseMagicAttackPoint = FindBufSkillData->IncreaseMagicAttackPoint;
					BufSkillInfo->IncreaseMagicCastingPoint = FindBufSkillData->IncreaseMagicCastingPoint;
					BufSkillInfo->IncreaseMagicAttackHitRate = FindBufSkillData->IncreaseMagicAttackHitRate;
					BufSkillInfo->IncreaseDefencePoint = FindBufSkillData->IncreaseDefencePoint;
					BufSkillInfo->IncreaseEvasionRate = FindBufSkillData->IncreaseEvasionRate;
					BufSkillInfo->IncreaseMeleeCriticalPoint = FindBufSkillData->IncreaseMeleeCriticalPoint;
					BufSkillInfo->IncreaseMagicCriticalPoint = FindBufSkillData->IncreaseMagicCriticalPoint;
					BufSkillInfo->IncreaseSpeedPoint = FindBufSkillData->IncreaseSpeedPoint;
					BufSkillInfo->IncreaseStatusAbnormalityResistance = FindBufSkillData->IncreaseStatusAbnormalityResistance;

					MyPlayer->_SkillBox.AddSkill(BufSkillInfo);

					// 클라가 소유하고 있는 스킬 정보를 보내준다.
					CMessage* ResSkillToSkillBoxPacket = MakePacketResSkillToSkillBox(MyPlayer->_GameObjectInfo.ObjectId, BufSkillInfo);
					SendPacket(Session->SessionId, ResSkillToSkillBoxPacket);
					ResSkillToSkillBoxPacket->Free();
				}
				break;
				}
			}

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterSkillGetConnection);
#pragma endregion

#pragma region 조합템 정보 보내기			
			vector<st_CraftingItemCategory> CraftingItemCategorys;

			for (int8 Category = (int8)en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON; Category <= (int8)en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL; ++Category)
			{
				auto FindCraftingIterator = G_Datamanager->_CraftingData.find(Category);
				if (FindCraftingIterator == G_Datamanager->_CraftingData.end())
				{
					continue;
				}

				st_CraftingItemCategoryData* CraftingData = (*FindCraftingIterator).second;

				// 제작템 카테고리 추출
				st_CraftingItemCategory CraftingItemCategory;
				CraftingItemCategory.CategoryType = CraftingData->CraftingType;
				CraftingItemCategory.CategoryName = (LPWSTR)CA2W(CraftingData->CraftingTypeName.c_str());

				// 제작템 카테고리에 속한 제작템 가져오기
				for (st_CraftingCompleteItemData CraftingCompleteItemData : CraftingData->CraftingCompleteItems)
				{
					st_CraftingCompleteItem CraftingCompleteItem;
					CraftingCompleteItem.CompleteItemType = CraftingCompleteItemData.CraftingCompleteItemDataId;
					CraftingCompleteItem.CompleteItemName = (LPWSTR)CA2W(CraftingCompleteItemData.CraftingCompleteName.c_str());
					CraftingCompleteItem.CompleteItemImagePath = (LPWSTR)CA2W(CraftingCompleteItemData.CraftingCompleteThumbnailImagePath.c_str());

					for (st_CraftingMaterialItemData CraftingMaterialItemData : CraftingCompleteItemData.CraftingMaterials)
					{
						st_CraftingMaterialItemInfo CraftingMaterialItem;
						CraftingMaterialItem.AccountDBId = Session->AccountId;
						CraftingMaterialItem.PlayerDBId = MyPlayer->_GameObjectInfo.ObjectId;
						CraftingMaterialItem.MaterialItemType = CraftingMaterialItemData.MaterialDataId;
						CraftingMaterialItem.MaterialItemName = (LPWSTR)CA2W(CraftingMaterialItemData.MaterialName.c_str());
						CraftingMaterialItem.ItemCount = CraftingMaterialItemData.MaterialCount;
						CraftingMaterialItem.MaterialItemImagePath = (LPWSTR)CA2W(CraftingMaterialItemData.MaterialThumbnailImagePath.c_str());

						CraftingCompleteItem.Materials.push_back(CraftingMaterialItem);
					}

					CraftingItemCategory.CompleteItems.push_back(CraftingCompleteItem);
				}

				CraftingItemCategorys.push_back(CraftingItemCategory);
			}

			CMessage* ResCraftingItemListMessage = MakePacketCraftingList(Session->AccountId, MyPlayer->_GameObjectInfo.ObjectId, CraftingItemCategorys);
			SendPacket(Session->SessionId, ResCraftingItemListMessage);
			ResCraftingItemListMessage->Free();
#pragma endregion
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBQuickSlotBarSlotSave(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		do
		{
			InterlockedDecrement64(&Session->IOBlock->IOCount);

			st_QuickSlotBarSlotInfo SaveQuickSlotInfo;
			*Message >> SaveQuickSlotInfo;

			Message->Free();

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			st_SkillInfo* FindSkill = MyPlayer->_SkillBox.FindSkill(SaveQuickSlotInfo.QuickBarSkillInfo.SkillType);
			// 캐릭터가 해당 스킬을 가지고 있는지 확인
			if (FindSkill != nullptr)
			{
				FindSkill->IsQuickSlotUse = true;

				MyPlayer->_QuickSlotManager.UpdateQuickSlotBar(SaveQuickSlotInfo);

				int8 SkillLargeCategory = (int8)SaveQuickSlotInfo.QuickBarSkillInfo.SkillLargeCategory;
				int8 SkillMediumCategory = (int8)SaveQuickSlotInfo.QuickBarSkillInfo.SkillMediumCategory;
				int16 SkillType = (int16)SaveQuickSlotInfo.QuickBarSkillInfo.SkillType;

				// DB에 퀵슬롯 정보 저장
				CDBConnection* DBQuickSlotUpdateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerQuickSlotBarSlotUpdate QuickSlotUpdate(*DBQuickSlotUpdateConnection);
				QuickSlotUpdate.InAccountDBId(SaveQuickSlotInfo.AccountDBId);
				QuickSlotUpdate.InPlayerDBId(SaveQuickSlotInfo.PlayerDBId);
				QuickSlotUpdate.InQuickSlotBarIndex(SaveQuickSlotInfo.QuickSlotBarIndex);
				QuickSlotUpdate.InQuickSlotBarSlotIndex(SaveQuickSlotInfo.QuickSlotBarSlotIndex);
				QuickSlotUpdate.InQuickSlotKey(SaveQuickSlotInfo.QuickSlotKey);
				QuickSlotUpdate.InSkillLargeCategory(SkillLargeCategory);
				QuickSlotUpdate.InSkillMediumCategory(SkillMediumCategory);
				QuickSlotUpdate.InSkillType(SkillType);
				QuickSlotUpdate.InSkillLevel(SaveQuickSlotInfo.QuickBarSkillInfo.SkillLevel);

				QuickSlotUpdate.Execute();

				// 스킬 데이터 찾기
				st_AttackSkillData* FindAttackSkillData = (st_AttackSkillData*)G_Datamanager->FindSkillData((en_SkillMediumCategory)SkillMediumCategory, (en_SkillType)SkillType);
				if (FindAttackSkillData == nullptr)
				{
					CRASH("스킬 데이터를 찾을 수 없습니다.");
				}

				SaveQuickSlotInfo.QuickBarSkillInfo.SkillName = (LPWSTR)CA2W(FindAttackSkillData->SkillName.c_str());
				SaveQuickSlotInfo.QuickBarSkillInfo.SkillCoolTime = FindAttackSkillData->SkillCoolTime;
				SaveQuickSlotInfo.QuickBarSkillInfo.SkillCastingTime = FindAttackSkillData->SkillCastingTime;
				SaveQuickSlotInfo.QuickBarSkillInfo.SkillImagePath = (LPWSTR)CA2W(FindAttackSkillData->SkillThumbnailImagePath.c_str());

				G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotUpdateConnection);

				CMessage* ResQuickSlotUpdateMessage = MakePacketResQuickSlotBarSlotSave(SaveQuickSlotInfo);
				SendPacket(Session->SessionId, ResQuickSlotUpdateMessage);
				ResQuickSlotUpdateMessage->Free();
			}
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBQuickSlotSwap(int64 SessionId, CMessage* Message)
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

			int8 QuickSlotBarSwapIndexA;
			*Message >> QuickSlotBarSwapIndexA;
			int8 QuickSlotBarSlotSwapIndexA;
			*Message >> QuickSlotBarSlotSwapIndexA;

			int8 QuickSlotBarSwapIndexB;
			*Message >> QuickSlotBarSwapIndexB;
			int8 QuickSlotBarSlotSwapIndexB;
			*Message >> QuickSlotBarSlotSwapIndexB;

			Message->Free();

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];			

#pragma region 퀵슬롯 A가 DB에 있는지 확인
			// 해당 퀵슬롯 위치에 정보가 있는지 DB에서 확인
			CDBConnection* DBQuickSlotACheckConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotCheck QuickSlotACheck(*DBQuickSlotACheckConnection);
			QuickSlotACheck.InAccountDBId(AccountId);
			QuickSlotACheck.InPlayerDBId(PlayerId);
			QuickSlotACheck.InQuickSlotBarIndex(QuickSlotBarSwapIndexA);
			QuickSlotACheck.InQuickSlotBarSlotIndex(QuickSlotBarSlotSwapIndexA);

			int16 QuickSlotAKey;
			int8 QuickSlotASkillLargeCategory;
			int8 QuickSlotASkillMediumCategory;
			int16 QuickSlotASkillType;
			int8 QuickSlotASkillLevel;

			QuickSlotACheck.OutQuickSlotKey(QuickSlotAKey);
			QuickSlotACheck.OutQuickSlotSkillLargeCategory(QuickSlotASkillLargeCategory);
			QuickSlotACheck.OutQuickSlotSkillMediumCategory(QuickSlotASkillMediumCategory);
			QuickSlotACheck.OutQuickSlotSkillType(QuickSlotASkillType);
			QuickSlotACheck.OutQuickSlotSkillLevel(QuickSlotASkillLevel);

			QuickSlotACheck.Execute();

			bool QuickSlotAFind = QuickSlotACheck.Fetch();

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotACheckConnection);

			// 스왑 요청 A 정보 셋팅
			st_QuickSlotBarSlotInfo SwapAQuickSlotBarInfo;
			SwapAQuickSlotBarInfo.AccountDBId = AccountId;
			SwapAQuickSlotBarInfo.PlayerDBId = PlayerId;
			SwapAQuickSlotBarInfo.QuickSlotBarIndex = QuickSlotBarSwapIndexB;
			SwapAQuickSlotBarInfo.QuickSlotBarSlotIndex = QuickSlotBarSlotSwapIndexB;
			SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillLargeCategory = (en_SkillLargeCategory)QuickSlotASkillLargeCategory;
			SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillMediumCategory = (en_SkillMediumCategory)QuickSlotASkillMediumCategory;
			SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillType = (en_SkillType)QuickSlotASkillType;
			SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillLevel = QuickSlotASkillLevel;
#pragma endregion
#pragma region 퀵슬롯 B가 DB에 있는지 확인
			// 해당 퀵슬롯 위치에 정보가 있는지 DB에서 확인
			CDBConnection* DBQuickSlotBCheckConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotCheck QuickSlotBCheck(*DBQuickSlotBCheckConnection);
			QuickSlotBCheck.InAccountDBId(AccountId);
			QuickSlotBCheck.InPlayerDBId(PlayerId);
			QuickSlotBCheck.InQuickSlotBarIndex(QuickSlotBarSwapIndexB);
			QuickSlotBCheck.InQuickSlotBarSlotIndex(QuickSlotBarSlotSwapIndexB);

			int16 QuickSlotBKey;
			int8 QuickSlotBSkillLargeCategory;
			int8 QuickSlotBSkillMediumCategory;
			int16 QuickSlotBSkillType;
			int8 QuickSlotBSkillLevel;

			QuickSlotBCheck.OutQuickSlotKey(QuickSlotBKey);
			QuickSlotBCheck.OutQuickSlotSkillLargeCategory(QuickSlotBSkillLargeCategory);
			QuickSlotBCheck.OutQuickSlotSkillMediumCategory(QuickSlotBSkillMediumCategory);
			QuickSlotBCheck.OutQuickSlotSkillType(QuickSlotBSkillType);
			QuickSlotBCheck.OutQuickSlotSkillLevel(QuickSlotBSkillLevel);

			QuickSlotBCheck.Execute();

			bool QuickSlotBFind = QuickSlotBCheck.Fetch();

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotBCheckConnection);

			// 스왑 요청 B 정보 셋팅
			st_QuickSlotBarSlotInfo SwapBQuickSlotBarInfo;
			SwapBQuickSlotBarInfo.AccountDBId = AccountId;
			SwapBQuickSlotBarInfo.PlayerDBId = PlayerId;
			SwapBQuickSlotBarInfo.QuickSlotBarIndex = QuickSlotBarSwapIndexA;
			SwapBQuickSlotBarInfo.QuickSlotBarSlotIndex = QuickSlotBarSlotSwapIndexA;
			SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillLargeCategory = (en_SkillLargeCategory)QuickSlotBSkillLargeCategory;
			SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillMediumCategory = (en_SkillMediumCategory)QuickSlotBSkillMediumCategory;
			SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillType = (en_SkillType)QuickSlotBSkillType;
			SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillLevel = QuickSlotBSkillLevel;

			SwapAQuickSlotBarInfo.QuickSlotKey = QuickSlotBKey;
			SwapBQuickSlotBarInfo.QuickSlotKey = QuickSlotAKey;
#pragma endregion

#pragma region DB에서 퀵슬롯 스왑
			int8 ASkillLargeCategory = (int8)SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillLargeCategory;
			int8 ASkillMediumCategory = (int8)SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillMediumCategory;
			int16 ASkillType = (int16)SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillType;

			int8 BSkillLargeCategory = (int8)SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillLargeCategory;
			int8 BSkillMediumCategory = (int8)SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillMediumCategory;
			int16 BSkillType = (int16)SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillType;

			CDBConnection* DBQuickSlotSwapConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotSwap QuickSlotSwap(*DBQuickSlotSwapConnection);
			QuickSlotSwap.InAccountDBId(AccountId);
			QuickSlotSwap.InPlayerDBId(PlayerId);

			QuickSlotSwap.InAQuickSlotBarIndex(SwapBQuickSlotBarInfo.QuickSlotBarIndex);
			QuickSlotSwap.InAQuickSlotBarSlotIndex(SwapBQuickSlotBarInfo.QuickSlotBarSlotIndex);
			QuickSlotSwap.InASkillLargeCategory(BSkillLargeCategory);
			QuickSlotSwap.InASkillMediumCategory(BSkillMediumCategory);
			QuickSlotSwap.InAQuickSlotSkillType(BSkillType);
			QuickSlotSwap.InAQuickSlotSkillLevel(SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillLevel);

			QuickSlotSwap.InBQuickSlotBarIndex(SwapAQuickSlotBarInfo.QuickSlotBarIndex);
			QuickSlotSwap.InBQuickSlotBarSlotIndex(SwapAQuickSlotBarInfo.QuickSlotBarSlotIndex);
			QuickSlotSwap.InBSkillLargeCategory(ASkillLargeCategory);
			QuickSlotSwap.InBSkillMediumCategory(ASkillMediumCategory);
			QuickSlotSwap.InBQuickSlotSkillType(ASkillType);
			QuickSlotSwap.InBQuickSlotSkillLevel(SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillLevel);

			bool QuickSlotSwapSuccess = QuickSlotSwap.Execute();
			if (QuickSlotSwapSuccess == true)
			{
				// 스킬 데이터 찾기
				st_SkillData* FindASkillData = G_Datamanager->FindSkillData(SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillMediumCategory, SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillType);
				if (FindASkillData != nullptr)
				{
					SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillName = (LPWSTR)CA2W(FindASkillData->SkillName.c_str());
					SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillCoolTime = FindASkillData->SkillCoolTime;
					SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillCastingTime = FindASkillData->SkillCastingTime;
					SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillImagePath = (LPWSTR)CA2W(FindASkillData->SkillThumbnailImagePath.c_str());
				}

				// 스킬 데이터 찾기
				st_SkillData* FindBSkillData = G_Datamanager->FindSkillData(SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillMediumCategory, SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillType);
				if (FindBSkillData != nullptr)
				{
					SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillName = (LPWSTR)CA2W(FindBSkillData->SkillName.c_str());
					SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillCoolTime = FindBSkillData->SkillCoolTime;
					SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillCastingTime = FindBSkillData->SkillCastingTime;
					SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillImagePath = (LPWSTR)CA2W(FindBSkillData->SkillThumbnailImagePath.c_str());
				}

				// 캐릭터에서 퀵슬롯 스왑
				MyPlayer->_QuickSlotManager.SwapQuickSlot(SwapBQuickSlotBarInfo, SwapAQuickSlotBarInfo);

				// 클라에게 결과 전송
				CMessage* ResQuickSlotSwapPacket = MakePacketResQuickSlotSwap(AccountId, PlayerId, SwapBQuickSlotBarInfo, SwapAQuickSlotBarInfo);
				SendPacket(MyPlayer->_SessionId, ResQuickSlotSwapPacket);
				ResQuickSlotSwapPacket->Free();
			}
#pragma endregion			
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBQuickSlotInit(int64 SessionId, CMessage* Message)
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

			Message->Free();

			// 해당 퀵슬롯 위치에 정보가 있는지 DB에서 확인
			CDBConnection* DBQuickSlotCheckConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotCheck QuickSlotACheck(*DBQuickSlotCheckConnection);
			QuickSlotACheck.InAccountDBId(AccountId);
			QuickSlotACheck.InPlayerDBId(PlayerId);
			QuickSlotACheck.InQuickSlotBarIndex(QuickSlotBarIndex);
			QuickSlotACheck.InQuickSlotBarSlotIndex(QuickSlotBarSlotIndex);

			int16 QuickSlotKey;
			int8 QuickSlotSkillLargeCategory;
			int8 QuickSlotSkillMediumCategory;
			int16 QuickSlotSkillType;
			int8 QuickSlotSkillLevel;

			QuickSlotACheck.OutQuickSlotKey(QuickSlotKey);
			QuickSlotACheck.OutQuickSlotSkillLargeCategory(QuickSlotSkillLargeCategory);
			QuickSlotACheck.OutQuickSlotSkillMediumCategory(QuickSlotSkillMediumCategory);
			QuickSlotACheck.OutQuickSlotSkillType(QuickSlotSkillType);
			QuickSlotACheck.OutQuickSlotSkillLevel(QuickSlotSkillLevel);

			QuickSlotACheck.Execute();

			bool QuickSlotAFind = QuickSlotACheck.Fetch();

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotCheckConnection);

			// 찾은 퀵슬롯 정보를 초기화
			CDBConnection* DBQuickSlotInitConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotInit QuickSlotInit(*DBQuickSlotInitConnection);

			wstring InitString = L"";

			QuickSlotInit.InAccountDBId(AccountId);
			QuickSlotInit.InPlayerDBId(PlayerId);
			QuickSlotInit.InQuickSlotBarIndex(QuickSlotBarIndex);
			QuickSlotInit.InQuickSlotBarSlotIndex(QuickSlotBarSlotIndex);
			QuickSlotInit.InQuickSlotBarSkillName(InitString);
			QuickSlotInit.InQuickSlotBarSkillThumbnailImagePath(InitString);

			QuickSlotInit.Execute();

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotInitConnection);

			// 게임에 입장한 캐릭터를 가져온다.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			CMessage* ResQuickSlotInitMessage = MakePacketResQuickSlotInit(Session->AccountId, MyPlayer->_GameObjectInfo.ObjectId, QuickSlotBarIndex, QuickSlotBarSlotIndex, QuickSlotKey);
			SendPacket(Session->SessionId, ResQuickSlotInitMessage);
			ResQuickSlotInitMessage->Free();
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBLeavePlayerInfoSave(int64 SessionId, CMessage* Message)
{
	int64 AccountId;
	*Message >> AccountId;
	int64 PlayerId;
	*Message >> PlayerId;

	// 저장할 상태 위치 방향 정보
	int8 State;
	*Message >> State;
	int32 LastPositionX;
	*Message >> LastPositionX;
	int32 LastPositionY;
	*Message >> LastPositionY;
	int8 Dir;
	*Message >> Dir;

	// 저장할 스탯 정보
	int32 Level;
	*Message >> Level;
	int32 CurrentHP;
	*Message >> CurrentHP;
	int32 MaxHP;
	*Message >> MaxHP;
	int32 CurrentMP;
	*Message >> CurrentMP;
	int32 MaxMP;
	*Message >> MaxMP;
	int32 CurrentDP;
	*Message >> CurrentDP;
	int32 MaxDP;
	*Message >> MaxDP;
	int16 AutoRecoveryHPPercent;
	*Message >> AutoRecoveryHPPercent;
	int16 AutoRecoveryMPPercent;
	*Message >> AutoRecoveryMPPercent;
	int32 MinMeleeAttackDamage;
	*Message >> MinMeleeAttackDamage;
	int32 MaxMeleeAttackDamage;
	*Message >> MaxMeleeAttackDamage;
	int16 MeleeAttackHitRate;
	*Message >> MeleeAttackHitRate;
	int16 MagicDamage;
	*Message >> MagicDamage;
	int16 MagicHitRate;
	*Message >> MagicHitRate;
	int32 Defence;
	*Message >> Defence;
	int16 EvasionRate;
	*Message >> EvasionRate;
	int16 MeleeCriticalPoint;
	*Message >> MeleeCriticalPoint;
	int16 MagicCriticalPoint;
	*Message >> MagicCriticalPoint;
	float Speed = 0;
	*Message >> Speed;

	// 경험치 정보
	int64 CurrentExperience;
	*Message >> CurrentExperience;
	int64 RequireExperience;
	*Message >> RequireExperience;
	int64 TotalExperience;
	*Message >> TotalExperience;

	Message->Free();

	CDBConnection* PlayerInfoSaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
	SP::CDBGameServerPlayerLeaveInfoSave PlayerLeaveInfoSave(*PlayerInfoSaveDBConnection);
	PlayerLeaveInfoSave.InAccountDBId(AccountId);
	PlayerLeaveInfoSave.InPlayerDBId(PlayerId);
	PlayerLeaveInfoSave.InLevel(Level);
	PlayerLeaveInfoSave.InMaxHP(MaxHP);
	PlayerLeaveInfoSave.InMaxMP(MaxMP);
	PlayerLeaveInfoSave.InMaxDP(MaxDP);
	PlayerLeaveInfoSave.InAutoRecoveryHPPercent(AutoRecoveryHPPercent);
	PlayerLeaveInfoSave.InAutoRecoveryMPPercent(AutoRecoveryMPPercent);
	PlayerLeaveInfoSave.InMinMeleeAttackDamage(MinMeleeAttackDamage);
	PlayerLeaveInfoSave.InMaxMeleeAttackDamage(MaxMeleeAttackDamage);
	PlayerLeaveInfoSave.InMeleeAttackHitRate(MeleeAttackHitRate);
	PlayerLeaveInfoSave.InMagicDamage(MagicDamage);
	PlayerLeaveInfoSave.InMagicHitRate(MagicHitRate);
	PlayerLeaveInfoSave.InDefence(Defence);
	PlayerLeaveInfoSave.InEvasionRate(EvasionRate);
	PlayerLeaveInfoSave.InMeleeCriticalPoint(MeleeCriticalPoint);
	PlayerLeaveInfoSave.InMagicCriticalPoint(MagicCriticalPoint);
	PlayerLeaveInfoSave.InSpeed(Speed);
	PlayerLeaveInfoSave.InLastPositionY(LastPositionY);
	PlayerLeaveInfoSave.InLastPositionX(LastPositionX);
	PlayerLeaveInfoSave.InCurrentExperience(CurrentExperience);
	PlayerLeaveInfoSave.InRequireExperience(RequireExperience);
	PlayerLeaveInfoSave.InTotalExperience(TotalExperience);

	bool IsPlayerLeaveInfoSave = PlayerLeaveInfoSave.Execute();
	if (IsPlayerLeaveInfoSave == false)
	{
		CRASH("PlayerInfoSave 실패");
	}

	G_DBConnectionPool->Push(en_DBConnect::GAME, PlayerInfoSaveDBConnection);
}

void CGameServer::PacketProcTimerAttackEnd(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	int64 TargetObjectId;
	*Message >> TargetObjectId;

	int16 TargetObjectType;
	*Message >> TargetObjectType;

	int8 QuickSlotBarIndex;
	*Message >> QuickSlotBarIndex;

	int8 QuickSlotBarSlotIndex;
	*Message >> QuickSlotBarSlotIndex;

	int8 SkillMediumCategory;
	*Message >> SkillMediumCategory;

	st_SkillInfo* SpellEndSkillInfo = nullptr;
	*Message >> &SpellEndSkillInfo;

	Message->Free();

	if (Session)
	{
		// 게임에 입장한 캐릭터를 가져온다.
		CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];		

		MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		MyPlayer->_SkillType = en_SkillType::SKILL_TYPE_NONE;

		CMessage* ResObjectStateMessage = MakePacketResChangeObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
		SendPacketFieldOfView(Session, ResObjectStateMessage, true);		
		ResObjectStateMessage->Free();
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcTimerSpellEnd(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	int64 TargetObjectId;
	*Message >> TargetObjectId;

	int16 TargetObjectType;
	*Message >> TargetObjectType;

	int8 QuickSlotBarIndex;
	*Message >> QuickSlotBarIndex;

	int8 QuickSlotBarSlotIndex;
	*Message >> QuickSlotBarSlotIndex;

	int8 SkillMediumCategory;
	*Message >> SkillMediumCategory;

	st_SkillInfo* SpellEndSkillInfo = nullptr;
	*Message >> &SpellEndSkillInfo;

	Message->Free();

	CGameObject* FindObject = G_ObjectManager->Find(TargetObjectId,(en_GameObjectType)TargetObjectType);

	if (Session)
	{
		// 게임에 입장한 캐릭터를 가져온다.
		CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];		

		// 마법 시전 완료 했으니 원래 상태로 돌려줌
		MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

		CMessage* ResObjectStateChangePacket = MakePacketResChangeObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
		SendPacketFieldOfView(Session, ResObjectStateChangePacket, true);		
		ResObjectStateChangePacket->Free();

		float SkillCoolTime = SpellEndSkillInfo->SkillCoolTime / 1000.0f;

		// 클라에게 쿨타임 표시
		CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(MyPlayer->_GameObjectInfo.ObjectId, QuickSlotBarIndex, QuickSlotBarSlotIndex, SkillCoolTime, 1.0f);
		SendPacket(Session->SessionId, ResCoolTimeStartPacket);
		ResCoolTimeStartPacket->Free();

		// 쿨타임 시간 동안 스킬 사용 못하게 막음
		SpellEndSkillInfo->CanSkillUse = false;

		// 스킬 쿨타임 얻어옴
		// 스킬 쿨타임 스킬쿨타임 잡 등록
		st_TimerJob* SkillCoolTimeTimerJob = _TimerJobMemoryPool->Alloc();
		SkillCoolTimeTimerJob->TimerJobExecTick = GetTickCount64() + SpellEndSkillInfo->SkillCoolTime;
		SkillCoolTimeTimerJob->SessionId = Session->SessionId;
		SkillCoolTimeTimerJob->TimerJobType = en_TimerJobType::TIMER_SKILL_COOLTIME_END;

		CGameServerMessage* ResCoolTimeEndMessage = CGameServerMessage::GameServerMessageAlloc();
		if (ResCoolTimeEndMessage == nullptr)
		{
			return;
		}

		ResCoolTimeEndMessage->Clear();

		*ResCoolTimeEndMessage << (int16)SpellEndSkillInfo->SkillType;
		SkillCoolTimeTimerJob->TimerJobMessage = ResCoolTimeEndMessage;		

		AcquireSRWLockExclusive(&_TimerJobLock);
		_TimerHeapJob->InsertHeap(SkillCoolTimeTimerJob->TimerJobExecTick, SkillCoolTimeTimerJob);
		ReleaseSRWLockExclusive(&_TimerJobLock);

		SetEvent(_TimerThreadWakeEvent);

		en_EffectType HitEffectType = en_EffectType::EFFECT_TYPE_NONE;

		wstring MagicSystemString;

		wchar_t SpellMessage[64] = L"0";

		// 크리티컬 판단
		random_device Seed;
		default_random_engine Eng(Seed());

		float CriticalPoint = MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint / 1000.0f;
		bernoulli_distribution CriticalCheck(CriticalPoint);
		bool IsCritical = CriticalCheck(Eng);

		int32 FinalDamage = 0;

		mt19937 Gen(Seed());

		switch (MyPlayer->_SkillType)
		{
		case en_SkillType::SKILL_SHAMAN_FLAME_HARPOON:
		{
			HitEffectType = en_EffectType::EFFECT_FLAME_HARPOON_TARGET;

			int32 MagicDamage = MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6;

			st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkillInfo;

			uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
			int32 ChoiceDamage = DamageChoiceRandom(Gen);
			FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

			// 데미지 처리
			MyPlayer->GetTarget()->OnDamaged(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s가 %s을 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), SpellEndSkillInfo->SkillName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

			MagicSystemString = SpellMessage;
		}
		break;
		case en_SkillType::SKILL_SHAMAN_ROOT:
		{
			HitEffectType = en_EffectType::EFFECT_DEBUF_ROOT;

			MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ROOT;

			CMessage* ResChangeObjectStatMessage = MakePacketResChangeObjectState(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->GetTarget()->_GameObjectInfo.ObjectType, MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.State);
			SendPacketFieldOfView(Session, ResChangeObjectStatMessage, true);			
			ResChangeObjectStatMessage->Free();

			CMessage* ResBufMessage = MakePacketBuf(MyPlayer->_GameObjectInfo.ObjectId, SpellEndSkillInfo->SkillCoolTime, 1.0f, SpellEndSkillInfo);
			SendPacketFieldOfView(Session, ResBufMessage, true);
			ResBufMessage->Free();

			st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkillInfo;

			ObjectStateChangeTimerJobCreate(MyPlayer->GetTarget(), en_CreatureState::IDLE, AttackSkillInfo->SkillDebufTime);
		}
		break;
		case en_SkillType::SKILL_SHAMAN_ICE_CHAIN:
		{
			st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkillInfo;

			MyPlayer->GetTarget()->_GameObjectInfo.ObjectStatInfo.Speed -= AttackSkillInfo->SkillDebufMovingSpeed;
		}
		break;
		case en_SkillType::SKILL_SHAMAN_ICE_WAVE:
			break;
		case en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE:
		{
			HitEffectType = en_EffectType::EFFECT_DEBUF_STUN;

			MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::STUN;

			// 스턴 상태로 변경
			CMessage* ResChangeObjectStat = MakePacketResChangeObjectState(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->GetTarget()->_GameObjectInfo.ObjectType, MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.State);
			SendPacketFieldOfView(Session, ResChangeObjectStat, true);			
			ResChangeObjectStat->Free();

			int32 MagicDamage = MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6;

			st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkillInfo;

			uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
			int32 ChoiceDamage = DamageChoiceRandom(Gen);
			FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

			// 데미지 처리
			MyPlayer->GetTarget()->OnDamaged(MyPlayer, FinalDamage);

			ObjectStateChangeTimerJobCreate(MyPlayer->GetTarget(), en_CreatureState::IDLE, AttackSkillInfo->SkillDebufTime);
		}
		break;
		case en_SkillType::SKILL_SHAMAN_HELL_FIRE:
		{
			HitEffectType = en_EffectType::EFFECT_FLAME_HARPOON_TARGET;

			int32 MagicDamage = MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6;

			st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkillInfo;

			uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
			int32 ChoiceDamage = DamageChoiceRandom(Gen);
			FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

			// 데미지 처리
			MyPlayer->GetTarget()->OnDamaged(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s가 %s을 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), SpellEndSkillInfo->SkillName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

			MagicSystemString = SpellMessage;
		}
		break;
		case en_SkillType::SKILL_TAIOIST_DIVINE_STRIKE:
		{
			HitEffectType = en_EffectType::EFFECT_FLAME_HARPOON_TARGET;

			int32 MagicDamage = MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6;

			st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkillInfo;

			uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
			int32 ChoiceDamage = DamageChoiceRandom(Gen);
			FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

			// 데미지 처리
			MyPlayer->GetTarget()->OnDamaged(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s가 %s을 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), SpellEndSkillInfo->SkillName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

			MagicSystemString = SpellMessage;
		}
		break;
		case en_SkillType::SKILL_TAIOIST_ROOT:
		{
			HitEffectType = en_EffectType::EFFECT_DEBUF_ROOT;

			MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ROOT;

			CMessage* ResChangeObjectStat = MakePacketResChangeObjectState(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->GetTarget()->_GameObjectInfo.ObjectType, MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.State);
			SendPacketFieldOfView(Session, ResChangeObjectStat, true);
			ResChangeObjectStat->Free();

			st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkillInfo;

			ObjectStateChangeTimerJobCreate(MyPlayer->GetTarget(), en_CreatureState::IDLE, AttackSkillInfo->SkillDebufTime);
		}
		break;
		case en_SkillType::SKILL_TAIOIST_HEALING_LIGHT:
		{
			HitEffectType = en_EffectType::EFFECT_HEALING_LIGHT_TARGET;

			st_HealSkillInfo* HealSkillInfo = (st_HealSkillInfo*)SpellEndSkillInfo;

			uniform_int_distribution<int> HealChoiceRandom(HealSkillInfo->SkillMinHealPoint, HealSkillInfo->SkillMaxHealPoint);
			FinalDamage = HealChoiceRandom(Gen);

			MyPlayer->GetTarget()->OnHeal(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s가 치유의빛을 사용해 %s를 %d만큼 회복했습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
			MagicSystemString = SpellMessage;
		}
		break;
		case en_SkillType::SKILL_TAIOIST_HEALING_WIND:
		{
			HitEffectType = en_EffectType::EFFECT_HEALING_WIND_TARGET;

			st_HealSkillInfo* HealSkillInfo = (st_HealSkillInfo*)SpellEndSkillInfo;

			uniform_int_distribution<int> HealChoiceRandom(HealSkillInfo->SkillMinHealPoint, HealSkillInfo->SkillMaxHealPoint);
			FinalDamage = HealChoiceRandom(Gen);

			MyPlayer->GetTarget()->OnHeal(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s가 치유의바람을 사용해 %s를 %d만큼 회복했습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
			MagicSystemString = SpellMessage;
		}
		break;
		default:
			break;
		}

		// 공격 응답
		CMessage* ResAttackMagicPacket = MakePacketResAttack(
			MyPlayer->_GameObjectInfo.ObjectId,
			MyPlayer->GetTarget()->_GameObjectInfo.ObjectId,
			MyPlayer->_SkillType,
			FinalDamage,
			false);
		SendPacketFieldOfView(Session, ResAttackMagicPacket, true);
		ResAttackMagicPacket->Free();


		// 시스템 메세지 전송
		CMessage* ResAttackMagicSystemMessagePacket = MakePacketResChattingBoxMessage(MyPlayer->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, st_Color::White(), MagicSystemString);
		SendPacketFieldOfView(Session, ResAttackMagicSystemMessagePacket, true);		
		ResAttackMagicSystemMessagePacket->Free();

		// HP 변경 전송
		CMessage* ResChangeObjectStat = MakePacketResChangeObjectStat(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, MyPlayer->GetTarget()->_GameObjectInfo.ObjectStatInfo);
		SendPacketFieldOfView(Session, ResChangeObjectStat, true);
		ResChangeObjectStat->Free();

		// 스펠창 끝
		CMessage* ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId, false);
		SendPacketFieldOfView(Session, ResMagicPacket, true);		
		ResMagicPacket->Free();

		// 이펙트 출력
		CMessage* ResEffectPacket = MakePacketEffect(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, HitEffectType, SpellEndSkillInfo->SkillTargetEffectTime);
		SendPacketFieldOfView(Session, ResEffectPacket, true);		
		ResEffectPacket->Free();
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcTimerCastingTimeEnd(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		int16 SkillType;
		*Message >> SkillType;

		Message->Free();

		// 게임에 입장한 캐릭터를 가져온다.
		CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

		st_SkillInfo* SkillInfo = MyPlayer->_SkillBox.FindSkill((en_SkillType)SkillType);
		SkillInfo->CanSkillUse = true;
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcTimerObjectSpawn(CGameServerMessage* Message)
{
	int16 SpawnObjectType;
	*Message >> SpawnObjectType;

	st_Vector2Int SpawnPosition;
	*Message >> SpawnPosition._X;
	*Message >> SpawnPosition._Y;

	Message->Free();

	CChannel* Channel = G_ChannelManager->Find(1);
	if (Channel != nullptr)
	{
		// 스폰할 위치에 이미 다른 오브젝트가 있을 경우 스폰 하지 않고 다시 예약한다. 		
		CGameObject* FindGameObject = Channel->_Map->Find(SpawnPosition);
		if (FindGameObject != nullptr)
		{
			SpawnObjectTimeTimerJobCreate(SpawnObjectType, SpawnPosition, 10000);
		}
		else
		{
			G_ObjectManager->ObjectSpawn((en_GameObjectType)SpawnObjectType, SpawnPosition);
		}
	}
}

void CGameServer::PacketProcTimerObjectStateChange(int64 SessionId, CGameServerMessage* Message)
{
	int64 TargetObjectId;
	*Message >> TargetObjectId;
	int16 TargetObjectType;
	*Message >> TargetObjectType;
	int16 ChangeState;
	*Message >> ChangeState;	

	CGameObject* ChangeStateObject = G_ObjectManager->Find(TargetObjectId, (en_GameObjectType)TargetObjectType);
	if (ChangeStateObject != nullptr)
	{
		switch ((en_GameObjectType)TargetObjectType)
		{
		case en_GameObjectType::OBJECT_BEAR:
		case en_GameObjectType::OBJECT_SLIME:
			{
				CMonster* ChangeStateMonsterObject = (CMonster*)ChangeStateObject;
				ChangeStateMonsterObject->_GameObjectInfo.ObjectPositionInfo.State = (en_CreatureState)ChangeState;

				CGameServerMessage* ResObjectStateChangeMessage = MakePacketResChangeObjectState(TargetObjectId, ChangeStateObject->_GameObjectInfo.ObjectPositionInfo.MoveDir, ChangeStateObject->_GameObjectInfo.ObjectType, (en_CreatureState)ChangeState);
				SendPacketFieldOfView(ChangeStateMonsterObject, ResObjectStateChangeMessage);
				ResObjectStateChangeMessage->Free();
			}
			break;
		case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
		case en_GameObjectType::OBJECT_MAGIC_PLAYER:
		case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
		case en_GameObjectType::OBJECT_THIEF_PLAYER:
		case en_GameObjectType::OBJECT_ARCHER_PLAYER:
		case en_GameObjectType::OBJECT_PLAYER_DUMMY:
			{
				CPlayer* ChangeStatePlayerObject = (CPlayer*)ChangeStateObject;
				switch (ChangeStatePlayerObject->_GameObjectInfo.ObjectPositionInfo.State)
				{
				case en_CreatureState::SPAWN_IDLE:
					{
						CChannel* Channel = G_ChannelManager->Find(1);
						if (Channel != nullptr)
						{
							st_Vector2Int ChangeStateObjectPosition = ChangeStateObject->GetCellPosition();
							Channel->_Map->ApplyMove(ChangeStateObject, ChangeStateObjectPosition);

							ChangeStatePlayerObject->_GameObjectInfo.ObjectPositionInfo.State = (en_CreatureState)ChangeState;

							CGameServerMessage* ResObjectStateChangeMessage = MakePacketResChangeObjectState(TargetObjectId, ChangeStateObject->_GameObjectInfo.ObjectPositionInfo.MoveDir, ChangeStateObject->_GameObjectInfo.ObjectType, (en_CreatureState)ChangeState);
							SendPacketFieldOfView(ChangeStateObject, ResObjectStateChangeMessage);
							ResObjectStateChangeMessage->Free();
						}
					}
					break;				
				default:
					{
						ChangeStatePlayerObject->_GameObjectInfo.ObjectPositionInfo.State = (en_CreatureState)ChangeState;

						CGameServerMessage* ResObjectStateChangeMessage = MakePacketResChangeObjectState(TargetObjectId, ChangeStateObject->_GameObjectInfo.ObjectPositionInfo.MoveDir, ChangeStateObject->_GameObjectInfo.ObjectType, (en_CreatureState)ChangeState);
						SendPacketFieldOfView(ChangeStateObject, ResObjectStateChangeMessage);
						ResObjectStateChangeMessage->Free();
					}
					break;
				}
			}
			break;
		default:
			break;
		}
	}
}

void CGameServer::PacketProcTimerDot(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	int64 ObjectId;
	*Message >> ObjectId;
	int16 ObjectType;
	*Message >> ObjectType;
	int8 DotType;
	*Message >> DotType;
	int64 DotTotalTime;
	*Message >> DotTotalTime;
	int64 DotTime;
	*Message >> DotTime;
	int32 HPPoint;
	*Message >> HPPoint;
	int32 MPPoint;
	*Message >> MPPoint;

	CGameObject* FindObject = G_ObjectManager->Find(ObjectId, (en_GameObjectType)ObjectType);

	// 세션이 있을 경우 스킬에 대한 도트를 의미
	if (Session != nullptr && FindObject != nullptr)
	{
		switch ((en_DotType)DotType)
		{
		case en_DotType::DOT_TYPE_HEAL:
			FindObject->_GameObjectInfo.ObjectStatInfo.HP += HPPoint;

			if (FindObject->_GameObjectInfo.ObjectStatInfo.HP >= FindObject->_GameObjectInfo.ObjectStatInfo.MaxHP)
			{
				FindObject->_GameObjectInfo.ObjectStatInfo.HP = FindObject->_GameObjectInfo.ObjectStatInfo.MaxHP;
			}

			DotTotalTime -= DotTime;

			// 힐 도트 시간이 아직 남아 있을 경우 예약
			if (DotTotalTime > 0)
			{				
				ObjectDotTimerCreate(FindObject, (en_DotType)DotType, DotTime, DotTotalTime, Session->SessionId);
			}
			break;
		case en_DotType::DOT_TYPE_POISON:
		case en_DotType::DOT_TYPE_BLEEDING:
		case en_DotType::DOT_TYPE_BURN:
			FindObject->_GameObjectInfo.ObjectStatInfo.HP -= HPPoint;

			if (FindObject->_GameObjectInfo.ObjectStatInfo.HP <= 0)
			{
				FindObject->_GameObjectInfo.ObjectStatInfo.HP = 0;
			}

			DotTotalTime -= DotTime;

			// 데미지 도트 시간이 아직 남아 있고 대상의 HP가 0 보다 클경우 도트 데미지 예약
			if (DotTotalTime > 0 && FindObject->_GameObjectInfo.ObjectStatInfo.HP > 0)
			{
				ObjectDotTimerCreate(FindObject, (en_DotType)DotType, DotTime, DotTotalTime, Session->SessionId);
			}
			break;
		}

		ReturnSession(Session);

		// 변경된 HP 전송
		CGameServerMessage* ResObjectStatChangePacket = MakePacketResChangeObjectStat(FindObject->_GameObjectInfo.ObjectId, FindObject->_GameObjectInfo.ObjectStatInfo);
		SendPacketFieldOfView(Session, ResObjectStatChangePacket, true);		
		ResObjectStatChangePacket->Free();
	}
	// 세션이 없을 경우에는 자연 회복 도트를 의미
	else if (Session == nullptr && FindObject != nullptr && FindObject->_NetworkState == en_ObjectNetworkState::LIVE)
	{
		if (FindObject->_GameObjectInfo.ObjectStatInfo.HP != FindObject->_GameObjectInfo.ObjectStatInfo.MaxHP
			|| FindObject->_GameObjectInfo.ObjectStatInfo.MP != FindObject->_GameObjectInfo.ObjectStatInfo.MaxMP)
		{
			// 자연 회복
			FindObject->_GameObjectInfo.ObjectStatInfo.HP += HPPoint;
			FindObject->_GameObjectInfo.ObjectStatInfo.MP += MPPoint;

			if (FindObject->_GameObjectInfo.ObjectStatInfo.HP >= FindObject->_GameObjectInfo.ObjectStatInfo.MaxHP)
			{
				FindObject->_GameObjectInfo.ObjectStatInfo.HP = FindObject->_GameObjectInfo.ObjectStatInfo.MaxHP;
			}

			if (FindObject->_GameObjectInfo.ObjectStatInfo.MP >= FindObject->_GameObjectInfo.ObjectStatInfo.MaxMP)
			{
				FindObject->_GameObjectInfo.ObjectStatInfo.MP = FindObject->_GameObjectInfo.ObjectStatInfo.MaxMP;
			}

			ObjectDotTimerCreate(FindObject, (en_DotType)DotType, DotTime, HPPoint, MPPoint);

			// 변경된 HP 전송
			CGameServerMessage* ResObjectStatChangePacket = MakePacketResChangeObjectStat(FindObject->_GameObjectInfo.ObjectId, FindObject->_GameObjectInfo.ObjectStatInfo);
			SendPacketFieldOfView(Session, ResObjectStatChangePacket, true);			
			ResObjectStatChangePacket->Free();
		}		
	}
	else if (Session == nullptr && FindObject->_NetworkState == en_ObjectNetworkState::LIVE)
	{
		// 네트워크 연결 끊긴 대상
	}

	Message->Free();
}

void CGameServer::PacketProcTimerPing(int64 SessionId)
{
	st_Session* Session = FindSession(SessionId);

	do
	{
		if (Session)
		{
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);				
			}

			int64 DeltaTime = GetTickCount64() - Session->PingPacketTime;

			if (DeltaTime > 30 * 1000)
			{
				Disconnect(Session->SessionId);				
			}
			else
			{
				// 핑 패킷 전송
				CGameServerMessage* PingMessage = MakePacketPing();
				SendPacket(Session->SessionId, PingMessage);
				PingMessage->Free();

				// 핑 패킷 예약
				PingTimerCreate(Session);
			}	

			ReturnSession(Session);
		}
	} while (0);	
}

CGameServerMessage* CGameServer::MakePacketResClientConnected()
{
	CGameServerMessage* ClientConnetedMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ClientConnetedMessage == nullptr)
	{
		CRASH("ClientConnectdMessage가 nullptr");
	}

	ClientConnetedMessage->Clear();

	*ClientConnetedMessage << (int16)en_PACKET_S2C_GAME_CLIENT_CONNECTED;

	return ClientConnetedMessage;
}

//---------------------------------------------------------------
//로그인 요청 응답 패킷 만들기 함수
//WORD Type
//BYTE Status  //0 : 실패  1 : 성공
//CPlayer Players
//---------------------------------------------------------------
CGameServerMessage* CGameServer::MakePacketResLogin(bool& Status, int8& PlayerCount, int32* MyPlayerIndexes)
{
	CGameServerMessage* LoginMessage = CGameServerMessage::GameServerMessageAlloc();
	if (LoginMessage == nullptr)
	{
		return nullptr;
	}

	LoginMessage->Clear();

	*LoginMessage << (int16)en_PACKET_S2C_GAME_RES_LOGIN;
	*LoginMessage << Status;
	*LoginMessage << PlayerCount;

	for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
	{
		if (G_ObjectManager->_PlayersArray[MyPlayerIndexes[i]] != nullptr)
		{
			*LoginMessage << G_ObjectManager->_PlayersArray[MyPlayerIndexes[i]]->_GameObjectInfo;
		}		
	}

	return LoginMessage;
}

// int32 PlayerDBId
// bool IsSuccess
// wstring PlayerName
CGameServerMessage* CGameServer::MakePacketResCreateCharacter(bool IsSuccess, st_GameObjectInfo& CreateCharacterObjectInfo)
{
	CGameServerMessage* ResCreateCharacter = CGameServerMessage::GameServerMessageAlloc();
	if (ResCreateCharacter == nullptr)
	{
		return nullptr;
	}

	ResCreateCharacter->Clear();

	*ResCreateCharacter << (int16)en_PACKET_S2C_GAME_CREATE_CHARACTER;
	*ResCreateCharacter << IsSuccess;

	*ResCreateCharacter << CreateCharacterObjectInfo;

	return ResCreateCharacter;
}

CGameServerMessage* CGameServer::MakePacketResEnterGame(st_GameObjectInfo ObjectInfo)
{
	CGameServerMessage* ResEnterGamePacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResEnterGamePacket == nullptr)
	{
		return nullptr;
	}

	ResEnterGamePacket->Clear();

	*ResEnterGamePacket << (int16)en_PACKET_S2C_GAME_ENTER;
	*ResEnterGamePacket << ObjectInfo;

	return ResEnterGamePacket;
}

// int64 AccountId
// int32 PlayerDBId
// st_GameObjectInfo ObjectInfo
CGameServerMessage* CGameServer::MakePacketResMousePositionObjectInfo(int64 AccountId, int64 PreviousChoiceObjectId, st_GameObjectInfo FindObjectInfo)
{
	CGameServerMessage* ResMousePositionObjectInfoPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResMousePositionObjectInfoPacket == nullptr)
	{
		return nullptr;
	}

	ResMousePositionObjectInfoPacket->Clear();

	*ResMousePositionObjectInfoPacket << (int16)en_PACKET_S2C_MOUSE_POSITION_OBJECT_INFO;
	*ResMousePositionObjectInfoPacket << AccountId;
	*ResMousePositionObjectInfoPacket << PreviousChoiceObjectId;

	// ObjectId
	*ResMousePositionObjectInfoPacket << FindObjectInfo;

	return ResMousePositionObjectInfoPacket;
}

CGameServerMessage* CGameServer::MakePacketResGoldSave(int64 AccountId, int64 ObjectId, int64 GoldCount, int16 SliverCount, int16 BronzeCount, int16 ItemCount, int16 ItemType, bool ItemGainPrint)
{
	CGameServerMessage* ResGoldSaveMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResGoldSaveMessage == nullptr)
	{
		return nullptr;
	}

	ResGoldSaveMessage->Clear();

	*ResGoldSaveMessage << (int16)en_PACKET_S2C_GOLD_SAVE;
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

CGameServerMessage* CGameServer::MakePacketInventoryCreate(int8 InventorySize, vector<CItem*> InventoryItems)
{
	CGameServerMessage* ResInventoryCreateMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResInventoryCreateMessage == nullptr)
	{
		return nullptr;
	}

	ResInventoryCreateMessage->Clear();

	*ResInventoryCreateMessage << (int16)en_PACKET_S2C_INVENTORY_CREATE;
	*ResInventoryCreateMessage << InventorySize;
	*ResInventoryCreateMessage << (int8)InventoryItems.size();

	for (CItem* Item : InventoryItems)
	{
		*ResInventoryCreateMessage << Item;
	}

	return ResInventoryCreateMessage;
}

CGameServerMessage* CGameServer::MakePacketResItemSwap(int64 AccountId, int64 ObjectId, st_ItemInfo SwapAItemInfo, st_ItemInfo SwapBItemInfo)
{
	CGameServerMessage* ResItemSwapMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResItemSwapMessage == nullptr)
	{
		return nullptr;
	}

	ResItemSwapMessage->Clear();

	*ResItemSwapMessage << (int16)en_PACKET_S2C_ITEM_SWAP;
	*ResItemSwapMessage << AccountId;
	*ResItemSwapMessage << ObjectId;

	// AItemInfo	
	*ResItemSwapMessage << SwapAItemInfo;

	// BItemInfo		
	*ResItemSwapMessage << SwapBItemInfo;

	return ResItemSwapMessage;
}

CGameServerMessage* CGameServer::MakePacketResItemToInventory(int64 TargetObjectId, CItem* InventoryItem, int16 ItemEach, bool ItemGainPrint)
{
	CGameServerMessage* ResItemToInventoryMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResItemToInventoryMessage == nullptr)
	{
		return nullptr;
	}

	ResItemToInventoryMessage->Clear();

	*ResItemToInventoryMessage << (int16)en_PACKET_S2C_LOOTING;
	// 타겟 아이디
	*ResItemToInventoryMessage << TargetObjectId;
	// 인벤토리 아이템의 소분류 값
	*ResItemToInventoryMessage << (int16)InventoryItem->_ItemInfo.ItemSmallCategory;
	// 인벤토리 아이템 정보 담기
	*ResItemToInventoryMessage << InventoryItem;
	// 아이템 낱개 개수
	*ResItemToInventoryMessage << ItemEach;
	// 아이템 얻는 UI 출력 할 것인지 말것인지
	*ResItemToInventoryMessage << ItemGainPrint;

	return ResItemToInventoryMessage;
}

CGameServerMessage* CGameServer::MakePacketInventoryItemUpdate(int64 PlayerId, st_ItemInfo UpdateItemInfo)
{
	CGameServerMessage* ResInventoryItemUpdateMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResInventoryItemUpdateMessage == nullptr)
	{
		return nullptr;
	}

	ResInventoryItemUpdateMessage->Clear();

	*ResInventoryItemUpdateMessage << (int16)en_PACKET_S2C_INVENTORY_ITEM_UPDATE;
	*ResInventoryItemUpdateMessage << PlayerId;
	*ResInventoryItemUpdateMessage << UpdateItemInfo;

	return ResInventoryItemUpdateMessage;
}

CGameServerMessage* CGameServer::MakePacketInventoryItemUse(int64 PlayerId, st_ItemInfo& UseItemInfo)
{
	CGameServerMessage* ResInventoryItemUseMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResInventoryItemUseMessage == nullptr)
	{
		return nullptr;
	}

	ResInventoryItemUseMessage->Clear();

	*ResInventoryItemUseMessage << (int16)en_PACKET_S2C_INVENTORY_ITEM_USE;
	*ResInventoryItemUseMessage << PlayerId;
	*ResInventoryItemUseMessage << UseItemInfo;

	return ResInventoryItemUseMessage;
}

CGameServerMessage* CGameServer::MakePacketEquipmentUpdate(int64 PlayerId, st_ItemInfo& EquipmentItemInfo)
{
	CGameServerMessage* ResEquipmentUpdateMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResEquipmentUpdateMessage == nullptr)
	{
		return nullptr;
	}

	ResEquipmentUpdateMessage->Clear();

	*ResEquipmentUpdateMessage << (int16)en_PACKET_S2C_EQUIPMENT_UPDATE;
	*ResEquipmentUpdateMessage << PlayerId;
	*ResEquipmentUpdateMessage << EquipmentItemInfo;

	return ResEquipmentUpdateMessage;
}

CGameServerMessage* CGameServer::MakePacketResQuickSlotBarSlotSave(st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo)
{
	CGameServerMessage* ResQuickSlotBarSlotMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResQuickSlotBarSlotMessage == nullptr)
	{
		return nullptr;
	}

	ResQuickSlotBarSlotMessage->Clear();

	*ResQuickSlotBarSlotMessage << (int16)en_PACKET_S2C_QUICKSLOT_SAVE;
	*ResQuickSlotBarSlotMessage << QuickSlotBarSlotInfo;

	return ResQuickSlotBarSlotMessage;
}

CGameServerMessage* CGameServer::MakePacketQuickSlotCreate(int8 QuickSlotBarSize, int8 QuickSlotBarSlotSize, vector<st_QuickSlotBarSlotInfo> QuickslotBarSlotInfos)
{
	CGameServerMessage* ResQuickSlotCreateMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResQuickSlotCreateMessage == nullptr)
	{
		return nullptr;
	}

	ResQuickSlotCreateMessage->Clear();

	*ResQuickSlotCreateMessage << (int16)en_PACKET_S2C_QUICKSLOT_CREATE;
	*ResQuickSlotCreateMessage << QuickSlotBarSize;
	*ResQuickSlotCreateMessage << QuickSlotBarSlotSize;

	*ResQuickSlotCreateMessage << (int8)QuickslotBarSlotInfos.size();

	for (st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo : QuickslotBarSlotInfos)
	{
		*ResQuickSlotCreateMessage << QuickSlotBarSlotInfo;
	}

	return ResQuickSlotCreateMessage;
}

CGameServerMessage* CGameServer::MakePacketResQuickSlotSwap(int64 AccountId, int64 PlayerId, st_QuickSlotBarSlotInfo SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo SwapBQuickSlotInfo)
{
	CGameServerMessage* ResQuickSlotSwapMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResQuickSlotSwapMessage == nullptr)
	{
		return nullptr;
	}

	ResQuickSlotSwapMessage->Clear();

	*ResQuickSlotSwapMessage << (int16)en_PACKET_S2C_QUICKSLOT_SWAP;
	*ResQuickSlotSwapMessage << AccountId;
	*ResQuickSlotSwapMessage << PlayerId;

	*ResQuickSlotSwapMessage << SwapAQuickSlotInfo;
	*ResQuickSlotSwapMessage << SwapBQuickSlotInfo;

	return ResQuickSlotSwapMessage;
}

CGameServerMessage* CGameServer::MakePacketResQuickSlotInit(int64 AccountId, int64 PlayerId, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, int16 QuickSlotKey)
{
	CGameServerMessage* ResQuickSlotInitMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResQuickSlotInitMessage == nullptr)
	{
		return nullptr;
	}

	ResQuickSlotInitMessage->Clear();

	*ResQuickSlotInitMessage << (int16)en_PACKET_S2C_QUICKSLOT_EMPTY;
	*ResQuickSlotInitMessage << AccountId;
	*ResQuickSlotInitMessage << PlayerId;
	*ResQuickSlotInitMessage << QuickSlotBarIndex;
	*ResQuickSlotInitMessage << QuickSlotBarSlotIndex;
	*ResQuickSlotInitMessage << QuickSlotKey;

	return ResQuickSlotInitMessage;
}

CGameServerMessage* CGameServer::MakePacketError(int64 PlayerId, en_ErrorType ErrorType, wstring ErrorMessage)
{
	CGameServerMessage* ResErrorMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResErrorMessage == nullptr)
	{
		return nullptr;
	}

	ResErrorMessage->Clear();

	*ResErrorMessage << (int16)en_PACKET_S2C_ERROR;
	*ResErrorMessage << PlayerId;
	*ResErrorMessage << (int16)ErrorType;

	// 에러 메세지
	int8 ErrorMessageLen = (int8)(ErrorMessage.length() * 2);
	*ResErrorMessage << ErrorMessageLen;
	ResErrorMessage->InsertData(ErrorMessage.c_str(), ErrorMessageLen);

	return ResErrorMessage;
}

CGameServerMessage* CGameServer::MakePacketCoolTime(int64 PlayerId, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, float SkillCoolTime, float SkillCoolTimeSpeed)
{
	CGameServerMessage* ResCoolTimeMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCoolTimeMessage == nullptr)
	{
		return nullptr;
	}

	ResCoolTimeMessage->Clear();

	*ResCoolTimeMessage << (int16)en_PACKET_S2C_COOLTIME_START;
	*ResCoolTimeMessage << PlayerId;
	*ResCoolTimeMessage << QuickSlotBarIndex;
	*ResCoolTimeMessage << QuickSlotBarSlotIndex;
	*ResCoolTimeMessage << SkillCoolTime;
	*ResCoolTimeMessage << SkillCoolTimeSpeed;

	return ResCoolTimeMessage;
}

CGameServerMessage* CGameServer::MakePacketCraftingList(int64 AccountId, int64 PlayerId, vector<st_CraftingItemCategory> CraftingItemList)
{
	CGameServerMessage* ResCraftingListMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCraftingListMessage == nullptr)
	{
		return nullptr;
	}

	ResCraftingListMessage->Clear();

	*ResCraftingListMessage << (int16)en_PACKET_S2C_CRAFTING_LIST;
	*ResCraftingListMessage << (int8)CraftingItemList.size();

	for (st_CraftingItemCategory CraftingItemCategory : CraftingItemList)
	{
		*ResCraftingListMessage << (int8)CraftingItemCategory.CategoryType;

		// 카테고리 이름 
		int8 CraftingItemCategoryNameLen = (int8)(CraftingItemCategory.CategoryName.length() * 2);
		*ResCraftingListMessage << CraftingItemCategoryNameLen;
		ResCraftingListMessage->InsertData(CraftingItemCategory.CategoryName.c_str(), CraftingItemCategoryNameLen);

		*ResCraftingListMessage << (int8)CraftingItemCategory.CompleteItems.size();

		for (st_CraftingCompleteItem CraftingCompleteItem : CraftingItemCategory.CompleteItems)
		{
			*ResCraftingListMessage << (int16)CraftingCompleteItem.CompleteItemType;

			// 제작 완성템 이름
			int8 CraftingCompleteItemNameLen = (int8)(CraftingCompleteItem.CompleteItemName.length() * 2);
			*ResCraftingListMessage << CraftingCompleteItemNameLen;
			ResCraftingListMessage->InsertData(CraftingCompleteItem.CompleteItemName.c_str(), CraftingCompleteItemNameLen);

			// 제작 완성템 이미지 경로
			int8 CraftingCompleteItemImagePathLen = (int8)(CraftingCompleteItem.CompleteItemImagePath.length() * 2);
			*ResCraftingListMessage << CraftingCompleteItemImagePathLen;
			ResCraftingListMessage->InsertData(CraftingCompleteItem.CompleteItemImagePath.c_str(), CraftingCompleteItemImagePathLen);

			*ResCraftingListMessage << (int8)CraftingCompleteItem.Materials.size();

			for (st_CraftingMaterialItemInfo CraftingMaterialItem : CraftingCompleteItem.Materials)
			{
				*ResCraftingListMessage << AccountId;
				*ResCraftingListMessage << PlayerId;
				*ResCraftingListMessage << (int16)CraftingMaterialItem.MaterialItemType;

				// 재료템 이름
				int8 CraftingMaterialItemNameLen = (int8)(CraftingMaterialItem.MaterialItemName.length() * 2);
				*ResCraftingListMessage << CraftingMaterialItemNameLen;
				ResCraftingListMessage->InsertData(CraftingMaterialItem.MaterialItemName.c_str(), CraftingMaterialItemNameLen);

				*ResCraftingListMessage << CraftingMaterialItem.ItemCount;

				// 재료템 이미지 경로
				int8 MaterialImagePathLen = (int8)(CraftingMaterialItem.MaterialItemImagePath.length() * 2);
				*ResCraftingListMessage << MaterialImagePathLen;
				ResCraftingListMessage->InsertData(CraftingMaterialItem.MaterialItemImagePath.c_str(), MaterialImagePathLen);
			}
		}
	}

	return ResCraftingListMessage;
}

CGameServerMessage* CGameServer::MakePacketMagicCancel(int64 AccountId, int64 PlayerId)
{
	CGameServerMessage* ResMagicCancelMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResMagicCancelMessage == nullptr)
	{
		return nullptr;
	}

	ResMagicCancelMessage->Clear();

	*ResMagicCancelMessage << (int16)en_PACKET_S2C_MAGIC_CANCEL;
	*ResMagicCancelMessage << AccountId;
	*ResMagicCancelMessage << PlayerId;

	return ResMagicCancelMessage;
}

CGameServerMessage* CGameServer::MakePacketExperience(int64 AccountId, int64 PlayerId, int64 GainExp, int64 CurrentExp, int64 RequireExp, int64 TotalExp)
{
	CGameServerMessage* ResExperienceMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResExperienceMessage == nullptr)
	{
		return nullptr;
	}

	ResExperienceMessage->Clear();

	*ResExperienceMessage << (int16)en_PACKET_S2C_EXPERIENCE;
	*ResExperienceMessage << AccountId;
	*ResExperienceMessage << PlayerId;
	*ResExperienceMessage << GainExp;
	*ResExperienceMessage << CurrentExp;
	*ResExperienceMessage << RequireExp;
	*ResExperienceMessage << TotalExp;

	return ResExperienceMessage;
}

CGameServerMessage* CGameServer::MakePacketPing()
{
	CGameServerMessage* PingMessage = CGameServerMessage::GameServerMessageAlloc();
	if (PingMessage == nullptr)
	{
		return nullptr;
	}

	PingMessage->Clear();

	*PingMessage << (int16)en_PACKET_S2C_PING;

	return PingMessage;
}

CGameServerMessage* CGameServer::MakePacketResAttack(int64 PlayerDBId, int64 TargetId, en_SkillType SkillType, int32 Damage, bool IsCritical)
{
	CGameServerMessage* ResAttackMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResAttackMessage == nullptr)
	{
		return nullptr;
	}

	ResAttackMessage->Clear();

	*ResAttackMessage << (int16)en_PACKET_S2C_ATTACK;
	*ResAttackMessage << PlayerDBId;
	*ResAttackMessage << TargetId;
	*ResAttackMessage << (int16)SkillType;
	*ResAttackMessage << Damage;
	*ResAttackMessage << IsCritical;

	return ResAttackMessage;
}

CGameServerMessage* CGameServer::MakePacketResMagic(int64 ObjectId, bool SpellStart, en_SkillType SkillType, float SpellTime)
{
	CGameServerMessage* ResMagicMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResMagicMessage == nullptr)
	{
		return nullptr;
	}

	ResMagicMessage->Clear();

	*ResMagicMessage << (int16)en_PACKET_S2C_MAGIC;
	*ResMagicMessage << ObjectId;
	*ResMagicMessage << SpellStart;
	*ResMagicMessage << (int16)SkillType;
	*ResMagicMessage << SpellTime;

	return ResMagicMessage;
}

// int64 AccountId
// int32 PlayerDBId
// int32 HP
CGameServerMessage* CGameServer::MakePacketResChangeObjectStat(int64 ObjectId, st_StatInfo ChangeObjectStatInfo)
{
	CGameServerMessage* ResChangeObjectStatPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResChangeObjectStatPacket == nullptr)
	{
		return nullptr;
	}

	ResChangeObjectStatPacket->Clear();

	*ResChangeObjectStatPacket << (int16)en_PACKET_S2C_OBJECT_STAT_CHANGE;
	*ResChangeObjectStatPacket << ObjectId;

	*ResChangeObjectStatPacket << ChangeObjectStatInfo;

	return ResChangeObjectStatPacket;
}

CGameServerMessage* CGameServer::MakePacketResChangeObjectState(int64 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState)
{
	CGameServerMessage* ResObjectStatePacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResObjectStatePacket == nullptr)
	{
		return nullptr;
	}

	ResObjectStatePacket->Clear();

	*ResObjectStatePacket << (int16)en_PACKET_S2C_OBJECT_STATE_CHANGE;
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
CGameServerMessage* CGameServer::MakePacketResMove(int64 AccountId, int64 ObjectId, en_GameObjectType ObjectType, st_PositionInfo PositionInfo)
{
	CGameServerMessage* ResMoveMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResMoveMessage == nullptr)
	{
		return nullptr;
	}

	ResMoveMessage->Clear();

	*ResMoveMessage << (int16)en_PACKET_S2C_MOVE;
	*ResMoveMessage << AccountId;
	*ResMoveMessage << ObjectId;

	// ObjectType
	*ResMoveMessage << (int16)ObjectType;

	*ResMoveMessage << PositionInfo;

	return ResMoveMessage;
}

CGameServerMessage* CGameServer::MakePacketPatrol(int64 ObjectId, en_GameObjectType ObjectType, st_PositionInfo PositionInfo)
{
	CGameServerMessage* ResPatrolPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResPatrolPacket == nullptr)
	{
		return nullptr;
	}

	ResPatrolPacket->Clear();

	*ResPatrolPacket << (int16)en_PACKET_S2C_PATROL;
	*ResPatrolPacket << ObjectId;
	*ResPatrolPacket << (int16)ObjectType;
	*ResPatrolPacket << PositionInfo;

	return ResPatrolPacket;
}

// int64 AccountId
// int32 PlayerDBId
// st_GameObjectInfo GameObjectInfo
CGameServerMessage* CGameServer::MakePacketResObjectSpawn(int32 ObjectInfosCount, vector<st_GameObjectInfo> ObjectInfos)
{
	CGameServerMessage* ResSpawnPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResSpawnPacket == nullptr)
	{
		return nullptr;
	}

	ResSpawnPacket->Clear();

	*ResSpawnPacket << (int16)en_PACKET_S2C_SPAWN;

	// Spawn 오브젝트 개수
	*ResSpawnPacket << ObjectInfosCount;

	for (int i = 0; i < ObjectInfosCount; i++)
	{
		// SpawnObjectId
		*ResSpawnPacket << ObjectInfos[i];
	}

	return ResSpawnPacket;
}

// int64 AccountId
// int32 PlayerDBId
CGameServerMessage* CGameServer::MakePacketResObjectDeSpawn(int32 DeSpawnObjectCount, vector<int64> DeSpawnObjectIds)
{
	CGameServerMessage* ResDeSpawnPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResDeSpawnPacket == nullptr)
	{
		return nullptr;
	}

	ResDeSpawnPacket->Clear();

	*ResDeSpawnPacket << (int16)en_PACKET_S2C_DESPAWN;
	*ResDeSpawnPacket << DeSpawnObjectCount;

	for (int32 i = 0; i < DeSpawnObjectCount; i++)
	{
		*ResDeSpawnPacket << DeSpawnObjectIds[i];
	}

	return ResDeSpawnPacket;
}

CGameServerMessage* CGameServer::MakePacketObjectDie(int64 DieObjectId)
{
	CGameServerMessage* ResDiePacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResDiePacket == nullptr)
	{
		return nullptr;
	}

	ResDiePacket->Clear();

	*ResDiePacket << (int16)en_PACKET_S2C_DIE;
	*ResDiePacket << DieObjectId;

	return ResDiePacket;
}

// int32 PlayerDBId
// wstring ChattingMessage
CGameServerMessage* CGameServer::MakePacketResChattingBoxMessage(int64 PlayerDBId, en_MessageType MessageType, st_Color Color, wstring ChattingMessage)
{
	CGameServerMessage* ResChattingMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResChattingMessage == nullptr)
	{
		return nullptr;
	}

	ResChattingMessage->Clear();

	*ResChattingMessage << (WORD)en_PACKET_S2C_MESSAGE;
	*ResChattingMessage << PlayerDBId;
	*ResChattingMessage << (int8)MessageType;

	*ResChattingMessage << Color;

	int8 PlayerNameLen = (int8)(ChattingMessage.length() * 2);
	*ResChattingMessage << PlayerNameLen;
	ResChattingMessage->InsertData(ChattingMessage.c_str(), PlayerNameLen);

	return ResChattingMessage;
}

CGameServerMessage* CGameServer::MakePacketResSyncPosition(int64 TargetObjectId, st_PositionInfo SyncPosition)
{
	CGameServerMessage* ResSyncPositionMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResSyncPositionMessage == nullptr)
	{
		return nullptr;
	}

	ResSyncPositionMessage->Clear();

	*ResSyncPositionMessage << (short)en_PACKET_S2C_SYNC_OBJECT_POSITION;
	*ResSyncPositionMessage << TargetObjectId;

	*ResSyncPositionMessage << SyncPosition;

	return ResSyncPositionMessage;
}

CGameServerMessage* CGameServer::MakePacketResSkillToSkillBox(int64 TargetObjectId, st_SkillInfo* SkillInfo)
{
	CGameServerMessage* ResSkillToSkillBoxMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResSkillToSkillBoxMessage == nullptr)
	{
		return nullptr;
	}

	ResSkillToSkillBoxMessage->Clear();

	*ResSkillToSkillBoxMessage << (int16)en_PACKET_S2C_SKILL_TO_SKILLBOX;
	*ResSkillToSkillBoxMessage << TargetObjectId;

	*ResSkillToSkillBoxMessage << *SkillInfo;

	return ResSkillToSkillBoxMessage;
}

CGameServerMessage* CGameServer::MakePacketEffect(int64 TargetObjectId, en_EffectType EffectType, float PrintEffectTime)
{
	CGameServerMessage* ResEffectMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResEffectMessage == nullptr)
	{
		return nullptr;
	}

	ResEffectMessage->Clear();

	*ResEffectMessage << (int16)en_PACKET_S2C_EFFECT;
	*ResEffectMessage << TargetObjectId;
	*ResEffectMessage << (int16)EffectType;
	*ResEffectMessage << PrintEffectTime;

	return ResEffectMessage;
}

CGameServerMessage* CGameServer::MakePacketBuf(int64 TargetObjectId, float SkillCoolTime, float SkillCoolTimeSpeed, st_SkillInfo* SkillInfo)
{
	CGameServerMessage* ResBufMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResBufMessage == nullptr)
	{
		return nullptr;
	}

	ResBufMessage->Clear();

	*ResBufMessage << (int16)en_PACKET_S2C_BUF;
	*ResBufMessage << TargetObjectId;
	*ResBufMessage << SkillCoolTime;
	*ResBufMessage << SkillCoolTimeSpeed;
	*ResBufMessage << *SkillInfo;

	return ResBufMessage;
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
	PacketProc(SessionID, Packet);
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

void CGameServer::SendPacketAroundSector(st_Vector2Int CellPosition, CMessage* Message)
{
	CChannel* Channel = G_ChannelManager->Find(1);
	vector<CSector*> Sectors = Channel->GetAroundSectors(CellPosition, 1);

	for (CSector* Sector : Sectors)
	{
		for (CPlayer* Player : Sector->GetPlayers())
		{
			if (Player->_GameObjectInfo.ObjectType == en_GameObjectType::OBJECT_PLAYER_DUMMY)
			{
				continue;
			}

			SendPacket(Player->_SessionId, Message);
		}
	}
}

void CGameServer::SendPacketAroundSector(st_Session* Session, CMessage* Message, bool SendMe)
{
	CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

	CChannel* Channel = G_ChannelManager->Find(1);
	vector<CSector*> Sectors = Channel->GetAroundSectors(MyPlayer->GetCellPosition(), 1);

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

void CGameServer::SendPacketFieldOfView(CGameObject* Object, CMessage* Message)
{
	CChannel* Channel = G_ChannelManager->Find(1);

	vector<CPlayer*> FieldOfViewPlayers = Channel->GetFieldOfViewPlayer(Object, Object->_FieldOfViewDistance);

	for (CPlayer* FieldOfViewPlayer : FieldOfViewPlayers)
	{
		SendPacket(FieldOfViewPlayer->_SessionId, Message);
	}
}

void CGameServer::SendPacketFieldOfView(st_Session* Session, CMessage* Message, bool SendMe)
{
	CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

	CChannel* Channel = G_ChannelManager->Find(1);

	// 주위 섹터들을 가져온다.
	vector<CSector*> Sectors = Channel->GetAroundSectors(MyPlayer->GetCellPosition(), 1);

	for (CSector* Sector : Sectors)
	{
		for (CPlayer* Player : Sector->GetPlayers())
		{
			// 시야 범위 안에 있는 플레이어를 찾는다
			int16 Distance = st_Vector2Int::Distance(MyPlayer->GetCellPosition(),Player->GetCellPosition());
			
			if (SendMe == true && Distance <= MyPlayer->_FieldOfViewDistance)
			{
				SendPacket(Player->_SessionId, Message);
			}
			else if(SendMe == false && Distance <= MyPlayer->_FieldOfViewDistance)
			{
				if (Session->SessionId != Player->_SessionId)
				{
					SendPacket(Player->_SessionId, Message);
				}
			}
		}
	}
}

void CGameServer::SkillCoolTimeTimerJobCreate(CPlayer* Player, int64 CastingTime, st_SkillInfo* CoolTimeSkillInfo, en_TimerJobType TimerJobType, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex)
{
	// 스킬 모션 끝 판단
	// 0.5초 후에 Idle상태로 바꾸기 위해 TimerJob 등록
	st_TimerJob* SkillEndTimerJob = _TimerJobMemoryPool->Alloc();
	SkillEndTimerJob->TimerJobExecTick = GetTickCount64() + CastingTime;
	SkillEndTimerJob->SessionId = Player->_SessionId;
	SkillEndTimerJob->TimerJobType = TimerJobType;
	// 스펠 취소 여부 
	SkillEndTimerJob->TimerJobCancel = false;

	Player->_SkillJob = SkillEndTimerJob;

	CGameServerMessage* SkillEndMessage = CGameServerMessage::GameServerMessageAlloc();
	if (SkillEndMessage == nullptr)
	{
		return;
	}

	SkillEndMessage->Clear();

	*SkillEndMessage << Player->_GameObjectInfo.ObjectId;
	*SkillEndMessage << (int16)Player->_GameObjectInfo.ObjectType;
	*SkillEndMessage << QuickSlotBarIndex;
	*SkillEndMessage << QuickSlotBarSlotIndex;
	*SkillEndMessage << (int8)CoolTimeSkillInfo->SkillMediumCategory;
	*SkillEndMessage << &CoolTimeSkillInfo;

	SkillEndTimerJob->TimerJobMessage = SkillEndMessage;

	AcquireSRWLockExclusive(&_TimerJobLock);
	_TimerHeapJob->InsertHeap(SkillEndTimerJob->TimerJobExecTick, SkillEndTimerJob);
	ReleaseSRWLockExclusive(&_TimerJobLock);

	SetEvent(_TimerThreadWakeEvent);
}

void CGameServer::SpawnObjectTimeTimerJobCreate(int16 SpawnObjectType, st_Vector2Int SpawnPosition, int64 SpawnTime)
{
	st_TimerJob* SpawnObjectTimerJob = _TimerJobMemoryPool->Alloc();
	SpawnObjectTimerJob->TimerJobExecTick = GetTickCount64() + SpawnTime;
	SpawnObjectTimerJob->SessionId = 0;
	SpawnObjectTimerJob->TimerJobType = en_TimerJobType::TIMER_OBJECT_SPAWN;

	CGameServerMessage* ResObjectSpawnMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResObjectSpawnMessage == nullptr)
	{
		return;
	}

	ResObjectSpawnMessage->Clear();

	*ResObjectSpawnMessage << SpawnObjectType;

	*ResObjectSpawnMessage << SpawnPosition;

	SpawnObjectTimerJob->TimerJobMessage = ResObjectSpawnMessage;

	AcquireSRWLockExclusive(&_TimerJobLock);
	_TimerHeapJob->InsertHeap(SpawnObjectTimerJob->TimerJobExecTick, SpawnObjectTimerJob);
	ReleaseSRWLockExclusive(&_TimerJobLock);

	SetEvent(_TimerThreadWakeEvent);
}

void CGameServer::ObjectStateChangeTimerJobCreate(CGameObject* Target, en_CreatureState ChangeState, int64 ChangeTime)
{
	st_TimerJob* ObjectStateChangeTimerJob = _TimerJobMemoryPool->Alloc();
	ObjectStateChangeTimerJob->TimerJobExecTick = GetTickCount64() + ChangeTime;
	ObjectStateChangeTimerJob->SessionId = 0;
	ObjectStateChangeTimerJob->TimerJobType = en_TimerJobType::TIMER_OBJECT_STATE_CHANGE;

	CGameServerMessage* ResObjectStateChangeMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResObjectStateChangeMessage == nullptr)
	{
		return;
	}

	ResObjectStateChangeMessage->Clear();
	*ResObjectStateChangeMessage << Target->_GameObjectInfo.ObjectId;
	*ResObjectStateChangeMessage << (int16)Target->_GameObjectInfo.ObjectType;
	*ResObjectStateChangeMessage << (int16)ChangeState;

	ObjectStateChangeTimerJob->TimerJobMessage = ResObjectStateChangeMessage;

	AcquireSRWLockExclusive(&_TimerJobLock);
	_TimerHeapJob->InsertHeap(ObjectStateChangeTimerJob->TimerJobExecTick, ObjectStateChangeTimerJob);
	ReleaseSRWLockExclusive(&_TimerJobLock);

	SetEvent(_TimerThreadWakeEvent);
}

st_TimerJob* CGameServer::ObjectDotTimerCreate(CGameObject* Target, en_DotType DotType, int64 DotTime, int32 HPPoint, int32 MPPoint, int64 DotTotalTime, int64 SessionId)
{
	st_TimerJob* ObjectDotTimerJob = _TimerJobMemoryPool->Alloc();
	ObjectDotTimerJob->TimerJobExecTick = GetTickCount64() + DotTime;
	ObjectDotTimerJob->SessionId = SessionId;
	ObjectDotTimerJob->TimerJobType = en_TimerJobType::TIMER_OBJECT_DOT;

	CGameServerMessage* ResObjectDotMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResObjectDotMessage == nullptr)
	{
		return nullptr;
	}

	ResObjectDotMessage->Clear();
	*ResObjectDotMessage << Target->_GameObjectInfo.ObjectId;
	*ResObjectDotMessage << (int16)Target->_GameObjectInfo.ObjectType;
	*ResObjectDotMessage << (int8)DotType;
	*ResObjectDotMessage << DotTotalTime;
	*ResObjectDotMessage << DotTime;
	*ResObjectDotMessage << HPPoint;
	*ResObjectDotMessage << MPPoint;

	ObjectDotTimerJob->TimerJobMessage = ResObjectDotMessage;

	AcquireSRWLockExclusive(&_TimerJobLock);
	_TimerHeapJob->InsertHeap(ObjectDotTimerJob->TimerJobExecTick, ObjectDotTimerJob);
	ReleaseSRWLockExclusive(&_TimerJobLock);

	SetEvent(_TimerThreadWakeEvent);

	return ObjectDotTimerJob;
}

void CGameServer::PingTimerCreate(st_Session* PingSession)
{
	st_TimerJob* PingTimerJob = _TimerJobMemoryPool->Alloc();
	PingTimerJob->TimerJobExecTick = GetTickCount64() + 2000;
	PingTimerJob->SessionId = PingSession->SessionId;
	PingTimerJob->TimerJobType = en_TimerJobType::TIMER_PING;

	AcquireSRWLockExclusive(&_TimerJobLock);
	_TimerHeapJob->InsertHeap(PingTimerJob->TimerJobExecTick, PingTimerJob);
	ReleaseSRWLockExclusive(&_TimerJobLock);

	SetEvent(_TimerThreadWakeEvent);
}