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
	_NetworkThread = nullptr;
	_DataBaseThread = nullptr;
	_TimerJobThread = nullptr;
	_LogicThread = nullptr;

	// 논시그널 상태 자동리셋
	_AuthThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_NetworkThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
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
	CloseHandle(_NetworkThreadWakeEvent);
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
	// 네트워크 쓰레드 시작
	_NetworkThread = (HANDLE)_beginthreadex(NULL, 0, NetworkThreadProc, this, 0, NULL);
	// 데이터베이스 쓰레드 시작
	_DataBaseThread = (HANDLE)_beginthreadex(NULL, 0, DataBaseThreadProc, this, 0, NULL);
	// 타이머 잡 쓰레드 시작
	_TimerJobThread = (HANDLE)_beginthreadex(NULL, 0, TimerJobThreadProc, this, 0, NULL);
	// 로직 쓰레드 시작
	_LogicThread = (HANDLE)_beginthreadex(NULL, 0, LogicThreadProc, this, 0, NULL);

	CloseHandle(_AuthThread);
	CloseHandle(_NetworkThread);
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

unsigned __stdcall CGameServer::NetworkThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_NetworkThreadEnd)
	{
		WaitForSingleObject(Instance->_NetworkThreadWakeEvent, INFINITE);

		Instance->_NetworkThreadWakeCount++;

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

				if (TimerJob->TimerJobCancel != true)
				{
					// Type에 따라 실행한다.
					switch (TimerJob->TimerJobType)
					{
					case en_TimerJobType::TIMER_ATTACK_END:
						Instance->PacketProcTimerAttackEnd(TimerJob->SessionId, TimerJob->TimerJobMessage);
						break;
					case en_TimerJobType::TIMER_SPELL_END:
						Instance->PacketProcTimerSpellEnd(TimerJob->SessionId, TimerJob->TimerJobMessage);
						break;
					case en_TimerJobType::TIMER_SKILL_COOLTIME_END:
						Instance->PacketProcTimerCoolTimeEnd(TimerJob->SessionId, TimerJob->TimerJobMessage);
						break;
					case en_TimerJobType::TIMER_OBJECT_SPAWN:
						Instance->PacketProcTimerObjectSpawn(TimerJob->TimerJobMessage);
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

unsigned __stdcall CGameServer::HeartBeatCheckThreadProc(void* Argument)
{
	return 0;
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
		Session->RecvPacketTime = timeGetTime();

		for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
		{
			Session->MyPlayers[i] = (CPlayer*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_PLAYER);
		}

		Session->MyPlayer = nullptr;

		// 게임 서버 접속 성공을 클라에게 알려준다.
		CMessage* ResClientConnectedMessage = MakePacketResClientConnected();
		SendPacket(Session->SessionId, ResClientConnectedMessage);
		ResClientConnectedMessage->Free();
	}	

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
		CMessage* ResLeaveGame = MakePacketResObjectDeSpawn(1, DeSpawnObjectIds);
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
	case en_PACKET_TYPE::en_PACKET_C2S_ITEM_TO_INVENTORY:
		PacketProcReqItemToInventory(SessionId, Message);
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

		CGameServerMessage* DBReqChatacerCreateMessage = CGameServerMessage::GameServerMessageAlloc();
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
			// DB 큐에 요청하기 전 IOCount를 증가시켜서 Session이 반납 안되도록 막음
			InterlockedIncrement64(&Session->IOBlock->IOCount);

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
			CMessage* ResSpawnPacket = MakePacketResObjectSpawn(1, SpawnObjectInfo);
			SendPacketAroundSector(Session, ResSpawnPacket);
			ResSpawnPacket->Free();

			SpawnObjectInfo.clear();

			// 나한테 다른 오브젝트들을 생성하라고 알려줌						
			st_GameObjectInfo* SpawnGameObjectInfos;

			vector<CGameObject*> AroundObjects = G_ChannelManager->Find(1)->GetAroundObjects(Session->MyPlayer, 1);

			if (AroundObjects.size() > 0)
			{
				SpawnGameObjectInfos = new st_GameObjectInfo[AroundObjects.size()];

				for (int32 i = 0; i < AroundObjects.size(); i++)
				{
					SpawnObjectInfo.push_back(AroundObjects[i]->_GameObjectInfo);
				}

				CMessage* ResOtherObjectSpawnPacket = MakePacketResObjectSpawn((int32)AroundObjects.size(), SpawnObjectInfo);
				SendPacket(Session->SessionId, ResOtherObjectSpawnPacket);
				ResOtherObjectSpawnPacket->Free();

				delete[] SpawnGameObjectInfos;
			}			

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
						
			// 요청한 스킬이 스킬창에 있는지 확인
			st_SkillInfo* FindSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)ReqSkillType);
			if (FindSkill != nullptr && FindSkill->CanSkillUse == true)
			{
				// 스킬 지정
				MyPlayer->_SkillType = (en_SkillType)ReqSkillType;
				// 공격 상태로 변경
				MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
				// 클라에게 알려줘서 공격 애니메이션 출력
				CMessage* ResObjectStateChangePacket = MakePacketResObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
				SendPacketAroundSector(MyPlayer->GetCellPosition(), ResObjectStateChangePacket);
				ResObjectStateChangePacket->Free();

				// 타겟 위치 확인
				switch ((en_SkillType)ReqSkillType)
				{
				case en_SkillType::SKILL_TYPE_NONE:
					break;
				case en_SkillType::SKILL_NORMAL:
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

								ResSyncPosition = MakePacketResSyncPosition(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
								SendPacketAroundSector(MyPlayer->GetCellPosition(), ResSyncPosition);
								ResSyncPosition->Free();
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
					CMessage* ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_SMASH_WAVE);
					SendPacketAroundSector(MyPlayer->GetCellPosition(), ResEffectPacket);
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

					en_EffectType HitEffectType;

					// 시스템 메세지 생성
					switch ((en_SkillType)ReqSkillType)
					{
					case en_SkillType::SKILL_TYPE_NONE:
						CRASH("SkillType None");
						break;
					case en_SkillType::SKILL_NORMAL:
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
					SendPacketAroundSector(MyPlayer->GetCellPosition(), ResSkillSystemMessagePacket);
					ResSkillSystemMessagePacket->Free();

					// 공격 응답 메세지 전송
					CMessage* ResMyAttackOtherPacket = MakePacketResAttack(MyPlayer->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectId, (en_SkillType)ReqSkillType, FinalDamage, IsCritical);
					SendPacketAroundSector(MyPlayer->GetCellPosition(), ResMyAttackOtherPacket);
					ResMyAttackOtherPacket->Free();

					// 이펙트 출력
					CMessage* ResEffectPacket = MakePacketEffect(Target->_GameObjectInfo.ObjectId, HitEffectType);
					SendPacketAroundSector(MyPlayer->GetCellPosition(), ResEffectPacket);
					ResEffectPacket->Free();
										
					// 스탯 변경 메세지 전송
					CMessage* ResChangeObjectStat = MakePacketChangeObjectStat(Target->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectStatInfo);
					SendPacketAroundSector(Target->GetCellPosition(), ResChangeObjectStat);
					ResChangeObjectStat->Free();
				}

				CoolTimeSkillTimerJobCreate(MyPlayer, 500, FindSkill, en_TimerJobType::TIMER_ATTACK_END, QuickSlotBarindex, QuickSlotBarSlotIndex);
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
					ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_CHARGE_POSE);
					SendPacketAroundSector(MyPlayer->GetCellPosition(), ResEffectPacket);
					ResEffectPacket->Free();
					break;
					// 불꽃 작살
				case en_SkillType::SKILL_SHAMNA_FLAME_HARPOON:
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
						SendPacketAroundSector(MyPlayer->GetCellPosition(), ResMagicPacket);
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
					// 치유의 빛
				case en_SkillType::SKILL_SHAMAN_HEALING_LIGHT:					
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

					// 이펙트 출력
					ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_HELAING_MYSELF);
					SendPacketAroundSector(MyPlayer->GetCellPosition(), ResEffectPacket);
					ResEffectPacket->Free();

					// 스펠창 시작
					ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId, true, (en_SkillType)ReqSkillType, SpellCastingTime);
					SendPacketAroundSector(MyPlayer->GetCellPosition(), ResMagicPacket);
					ResMagicPacket->Free();

					MyPlayer->_SkillType = (en_SkillType)ReqSkillType;
					break;
					// 치유의 바람
				case en_SkillType::SKILL_SHAMAN_HEALING_WIND:
					if (MyPlayer->_SelectTarget != nullptr)
					{
						MyPlayer->_SpellTick = GetTickCount64() + FindSkill->SkillCastingTime;

						SpellCastingTime = FindSkill->SkillCastingTime / 1000.0f;

						FindGameObject = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType);
						if (FindGameObject != nullptr)
						{
							Targets.push_back(FindGameObject);
						}						

						// 이펙트 출력
						ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_HELAING_MYSELF);
						SendPacketAroundSector(MyPlayer->GetCellPosition(), ResEffectPacket);
						ResEffectPacket->Free();

						// 스펠창 시작
						ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId, true, (en_SkillType)ReqSkillType, SpellCastingTime);
						SendPacketAroundSector(MyPlayer->GetCellPosition(), ResMagicPacket);
						ResMagicPacket->Free();

						MyPlayer->_SkillType = (en_SkillType)ReqSkillType;
					}
					else
					{

					}
					break;
				}								

				if (Targets.size() >= 1)
				{
					MyPlayer->SetTarget(Targets[0]);

					MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPELL;

					// 마법 스킬 모션 출력
					CMessage* ResObjectStateChangePacket = MakePacketResObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
					SendPacketAroundSector(MyPlayer->GetCellPosition(), ResObjectStateChangePacket);
					ResObjectStateChangePacket->Free();					

					CoolTimeSkillTimerJobCreate(MyPlayer, FindSkill->SkillCastingTime, FindSkill, en_TimerJobType::TIMER_SPELL_END, QuickSlotBarindex, QuickSlotBarSlotIndex);
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

			CPlayer* MagicCancelPlayer = (CPlayer*)G_ObjectManager->Find(PlayerId, en_GameObjectType::OBJECT_PLAYER);
			if (MagicCancelPlayer->_SkillJob != nullptr)
			{
				MagicCancelPlayer->_SkillJob->TimerJobCancel = true;
				MagicCancelPlayer->_SkillJob = nullptr;

				// 스킬 취소 응답 처리 주위 섹터에 전송
				CMessage* ResMagicCancelPacket = MakePacketMagicCancel(Session->MyPlayer->_AccountId,Session->MyPlayer->_GameObjectInfo.ObjectId);
				SendPacketAroundSector(Session, ResMagicCancelPacket, true);
				ResMagicCancelPacket->Free();
			}
			else
			{
				// 시전중인 스킬이 존재하지 않음
				wstring ErrorCastingSkillNotExist;

				WCHAR ErrorMessage[100] = { 0 };

				wsprintf(ErrorMessage, L"스킬 시전 중이 아닙니다.");
				ErrorCastingSkillNotExist = ErrorMessage;

				CMessage* ResErrorPacket = MakePacketError(Session->MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_SKILL_COOLTIME, ErrorCastingSkillNotExist);
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
						|| FindGameObject->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::PATROL
						|| FindGameObject->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::IDLE)
					{
						FindGameObject->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

						ResObjectStatePakcet = MakePacketResObjectState(FindGameObject->_GameObjectInfo.ObjectId, FindGameObject->_GameObjectInfo.ObjectPositionInfo.MoveDir, FindGameObject->_GameObjectInfo.ObjectType, FindGameObject->_GameObjectInfo.ObjectPositionInfo.State);
						SendPacketAroundSector(Session, ResObjectStatePakcet, true);
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

						ResObjectStatePakcet = MakePacketResObjectState(FindGameObject->_GameObjectInfo.ObjectId, FindGameObject->_GameObjectInfo.ObjectPositionInfo.MoveDir, FindGameObject->_GameObjectInfo.ObjectType, FindGameObject->_GameObjectInfo.ObjectPositionInfo.State);
						SendPacketAroundSector(Session, ResObjectStatePakcet, true);
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
			CMessage* ResChattingMessage = MakePacketResChattingBoxMessage(PlayerDBId, en_MessageType::CHATTING, st_Color::White(), ChattingMessage);
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

			// 아이템이 ObjectManager에 있는지 확인한다.
			CItem* Item = (CItem*)(G_ObjectManager->Find(ItemId, en_GameObjectType::OBJECT_ITEM));
			if (Item != nullptr)
			{
				int64 TargetObjectId;
				*Message >> TargetObjectId;				

				CPlayer* TargetPlayer = (CPlayer*)(G_ObjectManager->Find(TargetObjectId, en_GameObjectType::OBJECT_PLAYER));
				if (TargetPlayer == nullptr)
				{
					break;
				}

				bool IsExistItem = false;

				// 아이템 정보가 코인이라면
				if (Item->_ItemInfo.ItemSmallCategory == en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN
					|| Item->_ItemInfo.ItemSmallCategory == en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN
					|| Item->_ItemInfo.ItemSmallCategory == en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN)
				{
					// 코인 저장
					TargetPlayer->_Inventory.AddItem(Item);

					st_Job* DBGoldSaveJob = _JobMemoryPool->Alloc();
					DBGoldSaveJob->Type = en_JobType::DATA_BASE_GOLD_SAVE;
					DBGoldSaveJob->SessionId = TargetPlayer->_SessionId;

					CGameServerMessage* DBGoldSaveMessage = CGameServerMessage::GameServerMessageAlloc();
					DBGoldSaveMessage->Clear();

					*DBGoldSaveMessage << TargetPlayer->_AccountId;
					*DBGoldSaveMessage << TargetPlayer->_GameObjectInfo.ObjectId;
					*DBGoldSaveMessage << ItemId;
					*DBGoldSaveMessage << TargetPlayer->_Inventory._GoldCoinCount;
					*DBGoldSaveMessage << TargetPlayer->_Inventory._SliverCoinCount;
					*DBGoldSaveMessage << TargetPlayer->_Inventory._BronzeCoinCount;
					*DBGoldSaveMessage << Item->_ItemInfo.ItemCount;
					*DBGoldSaveMessage << (int16)Item->_ItemInfo.ItemSmallCategory;

					DBGoldSaveJob->Message = DBGoldSaveMessage;

					_GameServerDataBaseThreadMessageQue.Enqueue(DBGoldSaveJob);
					SetEvent(_DataBaseWakeEvent);
				}
				else
				{
					// 인벤토리에 저장할 아이템 개수 ( 맵에 스폰되어 있는 아이템의 개수 )
					int16 ItemEach = Item->_ItemInfo.ItemCount;		

					// 그 외 아이템이라면 
					int8 SlotIndex = 0;
					// 인벤토리에 저장된 아이템 개수
					int16 ItemCount = 0;

					// 아이템이 이미 존재 하는지 확인한다.
					// 존재 하면 개수를 아이템 개수 만큼 증가한다.
					IsExistItem = TargetPlayer->_Inventory.IsExistItem(Item->_ItemInfo.ItemSmallCategory, ItemEach, &SlotIndex);

					// 존재 하지 않을 경우
					if (IsExistItem == false)
					{
						// 비어 있는 슬롯인덱스 가져와서
						if (TargetPlayer->_Inventory.GetEmptySlot(&SlotIndex))
						{							
							// 인벤토리에 저장
							Item->_ItemInfo.SlotIndex = SlotIndex;
							TargetPlayer->_Inventory.AddItem(Item);	
							ItemCount = ItemEach;
						}
					}

					// DB 인벤토리에 저장하기 위해 Job 생성
					st_Job* DBInventorySaveJob = _JobMemoryPool->Alloc();
					DBInventorySaveJob->Type = en_JobType::DATA_BASE_LOOTING_ITEM_INVENTORY_SAVE;
					DBInventorySaveJob->SessionId = Session->SessionId;

					CGameServerMessage* DBSaveMessage = CGameServerMessage::GameServerMessageAlloc();

					DBSaveMessage->Clear();					

					CItem* CraftingItemCompleteItem = TargetPlayer->_Inventory.Get(SlotIndex);					

					// 타겟 ObjectId
					*DBSaveMessage << TargetPlayer->_GameObjectInfo.ObjectId;
					// 중복 여부
					*DBSaveMessage << IsExistItem;	
					// 얻은 아이템 개수
					*DBSaveMessage << ItemEach;
					// 삭제해야할 아이템 ObjectId;
					*DBSaveMessage << Item->_GameObjectInfo.ObjectId;						
					// 아이템 포인터 담기
					*DBSaveMessage << &CraftingItemCompleteItem;
					// 아이템 처리한 AccountId
					*DBSaveMessage << AccountId;

					DBInventorySaveJob->Message = DBSaveMessage;

					_GameServerDataBaseThreadMessageQue.Enqueue(DBInventorySaveJob);
					SetEvent(_DataBaseWakeEvent);
				}
				
				// 월드에 스폰되어 있었던 아이템을 반납한다.
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

			st_Job* DBQuickSlotSwapJob = _JobMemoryPool->Alloc();
			DBQuickSlotSwapJob->Type = en_JobType::DATA_BASE_QUICK_SWAP;
			DBQuickSlotSwapJob->SessionId = Session->MyPlayer->_SessionId;

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

			st_Job* DBQuickSlotInitJob = _JobMemoryPool->Alloc();
			DBQuickSlotInitJob->Type = en_JobType::DATA_BASE_QUICK_INIT;
			DBQuickSlotInitJob->SessionId = Session->MyPlayer->_SessionId;

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
			InterlockedIncrement64(&Session->IOBlock->IOCount);

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
						
			CPlayer* MyPlayer = Session->MyPlayer;
			
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
						// 만약 개수가 0개라면 해당 인벤토리 슬롯을 초기화 시킨다.
						if (FindMaterialItem->_ItemInfo.ItemCount == 0)
						{							
							MyPlayer->_Inventory.SlotInit(FindMaterialItem->_ItemInfo.SlotIndex);
						}	

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
			CraftingItemInfo.IsQuickSlotUse = FindReqCompleteItemData->IsEquipped;
			CraftingItemInfo.ItemLargeCategory = FindReqCompleteItemData->LargeItemCategory;
			CraftingItemInfo.ItemMediumCategory = FindReqCompleteItemData->MediumItemCategory;
			CraftingItemInfo.ItemSmallCategory = FindReqCompleteItemData->SmallItemCategory;
			CraftingItemInfo.ItemName = (LPWSTR)CA2W(FindReqCompleteItemData->ItemName.c_str());
			CraftingItemInfo.MinDamage = FindReqCompleteItemData->MinDamage;
			CraftingItemInfo.MaxDamage = FindReqCompleteItemData->MaxDamage;
			CraftingItemInfo.Defence = FindReqCompleteItemData->Defence;
			CraftingItemInfo.MaxCount = FindReqCompleteItemData->MaxCount;
			CraftingItemInfo.ItemCount = ReqCraftingItemCount;
			CraftingItemInfo.ThumbnailImagePath = (LPWSTR)CA2W(FindReqCompleteItemData->ThumbnailImagePath.c_str());
			CraftingItemInfo.IsEquipped = FindReqCompleteItemData->IsEquipped;

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
						CraftingWeaponItem->_ItemInfo.SlotIndex = SlotIndex;
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
						CraftingArmorItem->_ItemInfo.SlotIndex = SlotIndex;
						MyPlayer->_Inventory.AddItem(CraftingArmorItem);
					}
				}
				break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_CHOHONE:
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
						CraftingMaterialItem->_ItemInfo.SlotIndex = SlotIndex;
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
			int32 PlayerCurrentMP;
			int32 PlayerMaxMP;
			int32 PlayerCurrentDP;
			int32 PlayerMaxDP;
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
			ClientPlayersGet.OutCurrentMP(PlayerCurrentMP);
			ClientPlayersGet.OutMaxMP(PlayerMaxMP);
			ClientPlayersGet.OutCurrentDP(PlayerCurrentDP);
			ClientPlayersGet.OutMaxDP(PlayerMaxDP);
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
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.MP = PlayerCurrentMP;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.MaxMP = PlayerMaxMP;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.DP = PlayerCurrentDP;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.MaxDP = PlayerMaxDP;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.MinAttackDamage = PlayerMinAttack;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.MaxAttackDamage = PlayerMaxAttack;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.CriticalPoint = PlayerCriticalPoint;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectStatInfo.Speed = PlayerSpeed;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.ObjectType = (en_GameObjectType)PlayerObjectType;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.OwnerObjectId = 0;
				Session->MyPlayers[PlayerIndex]->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)PlayerObjectType;
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
			st_PlayerStatusData NewCharacterStatus = *(*FindStatus).second;
			int32 CurrentDP = 0;

			// 앞서 읽어온 캐릭터 정보를 토대로 DB에 저장
			// DBConnection Pool에서 DB연결을 위해서 하나를 꺼내온다.
			CDBConnection* NewCharacterPushDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			// GameServerDB에 새로운 캐릭터 저장하는 프로시저 클래스
			SP::CDBGameServerCreateCharacterPush NewCharacterPush(*NewCharacterPushDBConnection);
			NewCharacterPush.InAccountID(Session->AccountId);
			NewCharacterPush.InPlayerName(Session->CreateCharacterName);
			NewCharacterPush.InPlayerType(NewCharacterStatus.PlayerType);
			NewCharacterPush.InPlayerIndex(ReqCharacterCreateSlotIndex);
			NewCharacterPush.InLevel(NewCharacterStatus.Level);
			NewCharacterPush.InCurrentHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InMaxHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InCurrentMP(NewCharacterStatus.MaxMP);
			NewCharacterPush.InMaxMP(NewCharacterStatus.MaxMP);
			NewCharacterPush.InCurrentDP(CurrentDP);
			NewCharacterPush.InMaxDP(NewCharacterStatus.MaxDP);
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
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectStatInfo.DP = 0;
			Session->MyPlayers[ReqCharacterCreateSlotIndex]->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
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

				NewItem.ItemName = L"";
				NewItem.ThumbnailImagePath = L"";

				ItemToInventory.InItemName(NewItem.ItemName);
				ItemToInventory.InSlotIndex(SlotIndex);
				ItemToInventory.InThumbnailImagePath(NewItem.ThumbnailImagePath);
				ItemToInventory.InOwnerAccountId(Session->AccountId);
				ItemToInventory.InOwnerPlayerId(PlayerDBId);

				ItemToInventory.Execute();
			}			

			int8 DefaultKey = 1;

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
				int32 SkillCastingTime = 0;
				wstring SkillThumbnailImagePath;
				WCHAR QuickSlotBarKeyString[10] = { 0 };
								
				for (int8 i = 0; i < (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SLOT_SIZE; ++i)
				{
					QuickSlotBarSlotIndex = i;

					wsprintf(QuickSlotBarKeyString, L"%d", DefaultKey);
					DefaultKey++;

					if (DefaultKey == 10)
					{
						DefaultKey = 0;
					}

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
					QuickSlotBarSlotCreate.InSkillCastingTime(SkillCastingTime);
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
				ItemThumbnailImagePath = (LPWSTR)CA2W(DropItemData.ThumbnailImagePath.c_str());

				st_ItemData* WeaponItemData = (*G_Datamanager->_Items.find((int16)DropItemData.SmallItemCategory)).second;
				ItemMinDamage = WeaponItemData->MinDamage;
				ItemMaxDamage = WeaponItemData->MaxDamage;				
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
				ItemThumbnailImagePath = (LPWSTR)CA2W(DropItemData.ThumbnailImagePath.c_str());

				st_ItemData* ArmorItemData = (*G_Datamanager->_Items.find((int16)DropItemData.SmallItemCategory)).second;
				ItemDefence = ArmorItemData->Defence;
			}
			break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
			break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_CHOHONE:
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
				ItemThumbnailImagePath = (LPWSTR)CA2W(DropItemData.ThumbnailImagePath.c_str());

				st_ItemData* MaterialItemData = (*G_Datamanager->_Items.find((int16)DropItemData.SmallItemCategory)).second;
				ItemMaxCount = MaterialItemData->MaxCount;
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
						NewWeaponItem->_ItemInfo.IsQuickSlotUse = ItemIsQuickSlotUse;
						NewWeaponItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
						NewWeaponItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
						NewWeaponItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
						NewWeaponItem->_ItemInfo.ItemName = ItemName;
						NewWeaponItem->_ItemInfo.MinDamage = ItemMinDamage;
						NewWeaponItem->_ItemInfo.MaxDamage = ItemMaxDamage;
						NewWeaponItem->_ItemInfo.ItemCount = ItemCount;
						NewWeaponItem->_ItemInfo.ThumbnailImagePath = ItemThumbnailImagePath;
						NewWeaponItem->_ItemInfo.IsEquipped = ItemEquipped;
						NewWeaponItem->_ItemInfo.SlotIndex = ItemSlotIndex;												

						NewWeaponItem->_GameObjectInfo.ObjectType = DropItemData.ItemObjectType;
						NewWeaponItem->_GameObjectInfo.ObjectId = ItemDBId;
						NewWeaponItem->_GameObjectInfo.OwnerObjectId = KillerId;
						NewWeaponItem->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)KillerObjectType;
						NewWeaponItem->_SpawnPosition = SpawnPosition;
						NewWeaponItem->_SpawnPosition = SpawnPosition;

						G_ObjectManager->Add(NewWeaponItem, 1);
					}					
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
					{
						CArmor* NewArmorItem = (CArmor*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_ARMOR);
						NewArmorItem->_ItemInfo.ItemDBId = ItemDBId;
						NewArmorItem->_ItemInfo.IsQuickSlotUse = ItemIsQuickSlotUse;
						NewArmorItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
						NewArmorItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
						NewArmorItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
						NewArmorItem->_ItemInfo.ItemName = ItemName;
						NewArmorItem->_ItemInfo.Defence = ItemDefence;
						NewArmorItem->_ItemInfo.ItemCount = ItemCount;
						NewArmorItem->_ItemInfo.ThumbnailImagePath = ItemThumbnailImagePath;
						NewArmorItem->_ItemInfo.IsEquipped = ItemEquipped;
						NewArmorItem->_ItemInfo.SlotIndex = ItemSlotIndex;						

						NewArmorItem->_GameObjectInfo.ObjectType = DropItemData.ItemObjectType;
						NewArmorItem->_GameObjectInfo.ObjectId = ItemDBId;
						NewArmorItem->_GameObjectInfo.OwnerObjectId = KillerId;
						NewArmorItem->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)KillerObjectType;
						NewArmorItem->_SpawnPosition = SpawnPosition;

						G_ObjectManager->Add(NewArmorItem, 1);
					}					
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_CHOHONE:
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
						NewMaterialItem->_ItemInfo.IsQuickSlotUse = ItemIsQuickSlotUse;
						NewMaterialItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
						NewMaterialItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
						NewMaterialItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
						NewMaterialItem->_ItemInfo.ItemName = ItemName;
						NewMaterialItem->_ItemInfo.MaxCount = ItemMaxCount;
						NewMaterialItem->_ItemInfo.ItemCount = ItemCount;
						NewMaterialItem->_ItemInfo.ThumbnailImagePath = ItemThumbnailImagePath;
						NewMaterialItem->_ItemInfo.IsEquipped = ItemEquipped;
						NewMaterialItem->_ItemInfo.SlotIndex = ItemSlotIndex;						
						
						NewMaterialItem->_GameObjectInfo.ObjectType = DropItemData.ItemObjectType;
						NewMaterialItem->_GameObjectInfo.ObjectId = ItemDBId;
						NewMaterialItem->_GameObjectInfo.OwnerObjectId = KillerId;
						NewMaterialItem->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)KillerObjectType;
						NewMaterialItem->_SpawnPosition = SpawnPosition;						

						G_ObjectManager->Add(NewMaterialItem, 1);
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

void CGameServer::PacketProcReqDBLootingItemToInventorySave(int64 SessionId, CMessage* Message)
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
		Message->GetData(&Item, sizeof(CItem*));
		
		int64 OwnerAccountId;
		*Message >> OwnerAccountId;

		Message->Free();

		int64 ItemDBId = Item->_ItemInfo.ItemDBId;
		bool ItemIsQuickSlotUse = Item->_ItemInfo.IsQuickSlotUse;
		int8 ItemLargeCategory = (int8)Item->_ItemInfo.ItemLargeCategory;
		int8 ItemMediumCategory = (int8)Item->_ItemInfo.ItemMediumCategory;
		int16 ItemSmallCategory = (int16)Item->_ItemInfo.ItemSmallCategory;
		wstring ItemName = Item->_ItemInfo.ItemName;
		int16 ItemCount = Item->_ItemInfo.ItemCount;
		wstring ItemThumbnailImagePath = Item->_ItemInfo.ThumbnailImagePath;
		bool ItemEquipped = Item->_ItemInfo.IsEquipped;
		int8 ItemSlotIndex = Item->_ItemInfo.SlotIndex;
		int32 ItemMinDamage = Item->_ItemInfo.MinDamage;
		int32 ItemMaxDamage = Item->_ItemInfo.MaxDamage;
		int32 ItemDefence = Item->_ItemInfo.Defence;
		int32 ItemMaxCount = Item->_ItemInfo.MaxCount;		

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

void CGameServer::PacketProcReqDBCraftingItemToInventorySave(int64 SessionId, CMessage* Message)
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
			Message->GetData(&MaterialItem, sizeof(CItem*));			
						
			int16 MaterialItemSmallCategory = (int16)MaterialItem->_ItemInfo.ItemSmallCategory;
			int16 MaterialItemCount = MaterialItem->_ItemInfo.ItemCount;
			int8 MaterialItemSlotIndex = MaterialItem->_ItemInfo.SlotIndex;

			// 재료템을 DB에서 개수 업데이트
			CDBConnection* CraftingItemToInventorySaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerItemRefreshPush ItemRefreshPush(*CraftingItemToInventorySaveDBConnection);
			ItemRefreshPush.InAccountDBId(Session->AccountId);
			ItemRefreshPush.InPlayerDBId(TargetObjectId);
			ItemRefreshPush.InItemType(MaterialItemSmallCategory);
			ItemRefreshPush.InCount(MaterialItemCount);
			ItemRefreshPush.InSlotIndex(MaterialItemSlotIndex);

			ItemRefreshPush.Execute();

			G_DBConnectionPool->Push(en_DBConnect::GAME, CraftingItemToInventorySaveDBConnection);
			
			// 클라에게 업데이트 결과 전송
			CMessage* ReqInventoryItemUpdate = MakePacketInventoryItemUpdate(TargetObjectId, MaterialItem->_ItemInfo);
			SendPacket(SessionId, ReqInventoryItemUpdate);
			ReqInventoryItemUpdate->Free();
		}			

		bool IsExistItem;
		*Message >> IsExistItem;

		int16 ItemEachCount;
		*Message >> ItemEachCount;

		CItem* CompleteItem;		
		Message->GetData(&CompleteItem, sizeof(CItem*));

		int64 OwnerAccountId;
		*Message >> OwnerAccountId;

		Message->Free();

		CDBConnection* CraftingItemToInventorySaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

		// 아이템 Count 갱신
		if (IsExistItem == true)
		{
			int16 CompleteItemSmallCategory = (int16)CompleteItem->_ItemInfo.ItemSmallCategory;
			int16 CompleteItemCount = CompleteItem->_ItemInfo.ItemCount;
			int8 CompleteItemSlotIndex = CompleteItem->_ItemInfo.SlotIndex;

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
			bool CompleteItemIsQuickSlotUse = CompleteItem->_ItemInfo.IsQuickSlotUse;
			int8 CompleteItemLargeCategory = (int8)CompleteItem->_ItemInfo.ItemLargeCategory;
			int8 CompleteItemMediumCategory = (int8)CompleteItem->_ItemInfo.ItemMediumCategory;
			int16 CompleteItemSmallCategory = (int16)CompleteItem->_ItemInfo.ItemSmallCategory;
			wstring CompleteItemName = CompleteItem->_ItemInfo.ItemName;
			int32 CompleteItemMinDamage = CompleteItem->_ItemInfo.MinDamage;
			int32 CompleteItemMaxDamage = CompleteItem->_ItemInfo.MaxDamage;
			int32 CompleteItemDefence = CompleteItem->_ItemInfo.Defence;
			int32 CompleteItemMaxCount = CompleteItem->_ItemInfo.MaxCount;						
			int16 CompleteItemCount = CompleteItem->_ItemInfo.ItemCount;
			wstring CompleteItemThumbnailImagePath = CompleteItem->_ItemInfo.ThumbnailImagePath;
			bool CompleteItemIsEquipped = CompleteItem->_ItemInfo.IsEquipped;
			int8 CompleteItemSlotIndex = CompleteItem->_ItemInfo.SlotIndex;

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
		SwapAItemInfo.SlotIndex = SwapBIndex;
		SwapAItemInfo.IsQuickSlotUse = AIsQuickSlotUse;
		SwapAItemInfo.ItemLargeCategory = (en_LargeItemCategory)ADBItemLargeCategory;
		SwapAItemInfo.ItemMediumCategory = (en_MediumItemCategory)ADBItemMediumCategory;
		SwapAItemInfo.ItemSmallCategory = (en_SmallItemCategory)ADBItemSmallCategory;		
		SwapAItemInfo.ItemName = AItemName;
		SwapAItemInfo.MinDamage = AItemMinDamage;
		SwapAItemInfo.MaxDamage = AItemMaxDamage;
		SwapAItemInfo.Defence = AItemDefence;
		SwapAItemInfo.MaxCount = AItemMaxCount;
		SwapAItemInfo.ItemCount = AItemCount;
		SwapAItemInfo.IsEquipped = AItemEquipped;		
		SwapAItemInfo.ThumbnailImagePath = AItemThumbnailImagePath;

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
		SwapBItemInfo.SlotIndex = SwapAIndex;
		SwapBItemInfo.IsQuickSlotUse = BIsQuickSlotUse;
		SwapBItemInfo.ItemLargeCategory = (en_LargeItemCategory)BDBItemLargeCategory;		
		SwapBItemInfo.ItemMediumCategory = (en_MediumItemCategory)BDBItemMediumCategory;		
		SwapBItemInfo.ItemSmallCategory = (en_SmallItemCategory)BDBItemSmallCategory;
		SwapBItemInfo.ItemName = BItemName;
		SwapBItemInfo.MinDamage = BItemMinDamage;
		SwapBItemInfo.MaxDamage = BItemMaxDamage;
		SwapBItemInfo.Defence = BItemDefence;
		SwapBItemInfo.MaxCount = BItemMaxCount;
		SwapBItemInfo.ItemCount = BItemCount;
		SwapBItemInfo.IsEquipped = BItemEquipped;
		SwapBItemInfo.ThumbnailImagePath = BItemThumbnailImagePath;

		G_DBConnectionPool->Push(en_DBConnect::GAME, BItemCheckDBConnection);	

		int16 AItemSmallCategory = (int16)SwapAItemInfo.ItemSmallCategory;
		int16 BItemSmallCategory = (int16)SwapBItemInfo.ItemSmallCategory;		

		// DB에서 아이템 스왑
		CDBConnection* ItemSwapConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerItemSwap ItemSwap(*ItemSwapConnection);
		ItemSwap.InAccountDBId(AccountId);
		ItemSwap.InPlayerDBId(PlayerDBId);

		ItemSwap.InAIsQuickSlotUse(SwapBItemInfo.IsQuickSlotUse);
		ItemSwap.InAItemLargeCategory(BDBItemLargeCategory);
		ItemSwap.InAItemMediumCategory(BDBItemMediumCategory);
		ItemSwap.InAItemSmallCategory(BItemSmallCategory);		
		ItemSwap.InAItemName(SwapBItemInfo.ItemName);
		ItemSwap.InAItemMinDamage(SwapBItemInfo.MinDamage);
		ItemSwap.InAItemMaxDamage(SwapBItemInfo.MaxDamage);
		ItemSwap.InAItemDefence(SwapBItemInfo.Defence);
		ItemSwap.InAItemMaxCount(SwapBItemInfo.MaxCount);
		ItemSwap.InAItemCount(SwapBItemInfo.ItemCount);
		ItemSwap.InAItemIsEquipped(SwapBItemInfo.IsEquipped);
		ItemSwap.InAItemThumbnailImagePath(SwapBItemInfo.ThumbnailImagePath);		
		ItemSwap.InAItemSlotIndex(SwapBItemInfo.SlotIndex);

		ItemSwap.InBIsQuickSlotUse(SwapAItemInfo.IsQuickSlotUse);
		ItemSwap.InBItemLargeCategory(ADBItemLargeCategory);
		ItemSwap.InBItemMediumCategory(ADBItemMediumCategory);
		ItemSwap.InBItemSmallCategory(AItemSmallCategory);
		ItemSwap.InBItemName(SwapAItemInfo.ItemName);
		ItemSwap.InBItemMinDamage(SwapAItemInfo.MinDamage);
		ItemSwap.InBItemMaxDamage(SwapAItemInfo.MaxDamage);
		ItemSwap.InBItemDefence(SwapAItemInfo.Defence);
		ItemSwap.InBItemMaxCount(SwapAItemInfo.MaxCount);
		ItemSwap.InBItemCount(SwapAItemInfo.ItemCount);
		ItemSwap.InBItemIsEquipped(SwapAItemInfo.IsEquipped);		
		ItemSwap.InBItemThumbnailImagePath(SwapAItemInfo.ThumbnailImagePath);
		ItemSwap.InBItemSlotIndex(SwapAItemInfo.SlotIndex);
			
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
			int32 QuickSlotSkillCastingTime;
			WCHAR QuickSlotSkillThumbnailImagePath[100] = { 0 };

			QuickSlotBarGet.OutQuickSlotBarIndex(QuickSlotBarIndex);
			QuickSlotBarGet.OutQuickSlotBarItemIndex(QuickSlotBarSlotIndex);
			QuickSlotBarGet.OutQuickSlotKey(QuickSlotKey);
			QuickSlotBarGet.OutQuickSlotSkillType(QuickSlotSkillType);
			QuickSlotBarGet.OutQuickSlotSkillLevel(QuickSlotSkillLevel);
			QuickSlotBarGet.OutQuickSlotSkillCoolTime(QuickSlotSkillCoolTime);
			QuickSlotBarGet.OutQuickSlotSkillCastingTime(QuickSlotSkillCastingTime);
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
				NewQuickSlotBarSlot.QuickBarSkillInfo.SkillType = (en_SkillType)QuickSlotSkillType;
				NewQuickSlotBarSlot.QuickBarSkillInfo.SkillLevel = QuickSlotSkillLevel;
				NewQuickSlotBarSlot.QuickBarSkillInfo.SkillCoolTime = QuickSlotSkillCoolTime;
				NewQuickSlotBarSlot.QuickBarSkillInfo.SkillCastingTime = QuickSlotSkillCastingTime;
				NewQuickSlotBarSlot.QuickBarSkillInfo.SkillImagePath = QuickSlotSkillThumbnailImagePath;

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

			vector<CItem*> InventoryItems;			

			// DB에 기록되어 있는 인벤토리 아이템들의 정보를 모두 긁어온다.
			CDBConnection* DBInventoryItemInfoGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerInventoryItemGet CharacterInventoryItem(*DBInventoryItemInfoGetConnection);
			CharacterInventoryItem.InAccountDBId(Session->MyPlayer->_AccountId);
			CharacterInventoryItem.InPlayerDBId(Session->MyPlayer->_GameObjectInfo.ObjectId);

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
			CharacterInventoryItem.OutMaxCount(ItemMaxDamage);
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
					WeaponItem->_ItemInfo.ThumbnailImagePath = ItemThumbnailImagePath;
					WeaponItem->_ItemInfo.IsEquipped = IsEquipped;
					WeaponItem->_ItemInfo.SlotIndex = SlotIndex;
					WeaponItem->_ItemInfo.MinDamage = ItemMinDamage;
					WeaponItem->_ItemInfo.MaxDamage = ItemMaxDamage;				

					Session->MyPlayer->_Inventory.AddItem(WeaponItem);

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
					ArmorItem->_ItemInfo.ThumbnailImagePath = ItemThumbnailImagePath;
					ArmorItem->_ItemInfo.IsEquipped = IsEquipped;
					ArmorItem->_ItemInfo.SlotIndex = SlotIndex;
					ArmorItem->_ItemInfo.Defence = ItemDefence;				

					Session->MyPlayer->_Inventory.AddItem(ArmorItem);

					InventoryItems.push_back(ArmorItem);
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_CHOHONE:
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
					MaterialItem->_ItemInfo.ThumbnailImagePath = ItemThumbnailImagePath;
					MaterialItem->_ItemInfo.IsEquipped = IsEquipped;
					MaterialItem->_ItemInfo.SlotIndex = SlotIndex;
					MaterialItem->_ItemInfo.MaxCount = ItemMaxCount;					

					Session->MyPlayer->_Inventory.AddItem(MaterialItem);

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

#pragma region 골드 정보 읽어오기
			// 캐릭터가 소유하고 있었던 골드 정보를 GoldTable에서 읽어온다.
			CDBConnection* DBCharacterGoldGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerGoldGet CharacterGoldGet(*DBCharacterGoldGetConnection);
			CharacterGoldGet.InAccountDBId(Session->MyPlayer->_AccountId);
			CharacterGoldGet.InPlayerDBId(Session->MyPlayer->_GameObjectInfo.ObjectId);
			
			int64 GoldCoin = 0;
			int16 SliverCoin = 0;
			int16 BronzeCoin = 0;

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
				CMessage* ResGoldSaveMeesage = MakePacketResGoldSave(Session->MyPlayer->_AccountId, Session->MyPlayer->_GameObjectInfo.ObjectId, GoldCoin, SliverCoin, BronzeCoin, 0, 0, false);
				SendPacket(Session->SessionId, ResGoldSaveMeesage);
				ResGoldSaveMeesage->Free();
			}

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterGoldGetConnection);
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
			int32 SkillCastingTime;
			WCHAR SkillThumbnailImagePath[100] = { 0 };

			CharacterSkillGet.OutIsQuickSlotUse(IsQuickSlotUse);
			CharacterSkillGet.OutSkillType(SkillType);
			CharacterSkillGet.OutSkillLevel(SkillLevel);
			CharacterSkillGet.OutSkillName(SkillName);
			CharacterSkillGet.OutSkillCoolTime(SkillCoolTime);
			CharacterSkillGet.OutSkillCastingTime(SkillCastingTime);
			CharacterSkillGet.OutSkillThumbnailImagePath(SkillThumbnailImagePath);

			CharacterSkillGet.Execute();

			while (CharacterSkillGet.Fetch())
			{
				st_SkillInfo SkillInfo;
				SkillInfo.IsQuickSlotUse = IsQuickSlotUse;
				SkillInfo.SkillType = (en_SkillType)SkillType;
				SkillInfo.SkillLevel = SkillLevel;
				SkillInfo.SkillName = SkillName;
				SkillInfo.SkillCoolTime = SkillCoolTime;
				SkillInfo.SkillCastingTime = SkillCastingTime;
				SkillInfo.SkillImagePath = SkillThumbnailImagePath;

				Session->MyPlayer->_SkillBox.AddSkill(SkillInfo);

				// 클라가 소유하고 있는 스킬 정보를 보내준다.
				CMessage* ResSkillToSkillBoxPacket = MakePacketResSkillToSkillBox(Session->MyPlayer->_GameObjectInfo.ObjectId, SkillInfo);
				SendPacket(Session->SessionId, ResSkillToSkillBoxPacket);
				ResSkillToSkillBoxPacket->Free();				
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
						CraftingMaterialItem.PlayerDBId = Session->MyPlayer->_GameObjectInfo.ObjectId;
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

			CMessage* ResCraftingItemListMessage = MakePacketCraftingList(Session->AccountId, Session->MyPlayer->_GameObjectInfo.ObjectId, CraftingItemCategorys);
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

			st_SkillInfo* FindSkill = Session->MyPlayer->_SkillBox.FindSkill(SaveQuickSlotInfo.QuickBarSkillInfo.SkillType);
			// 캐릭터가 해당 스킬을 가지고 있는지 확인
			if (FindSkill != nullptr)
			{
				FindSkill->IsQuickSlotUse = true;				

				Session->MyPlayer->_QuickSlotManager.UpdateQuickSlotBar(SaveQuickSlotInfo);
				int16 SkillType = (int16)SaveQuickSlotInfo.QuickBarSkillInfo.SkillType;
				// DB에 퀵슬롯 정보 저장
				CDBConnection* DBQuickSlotUpdateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerQuickSlotBarSlotUpdate QuickSlotUpdate(*DBQuickSlotUpdateConnection);
				QuickSlotUpdate.InAccountDBId(SaveQuickSlotInfo.AccountDBId);
				QuickSlotUpdate.InPlayerDBId(SaveQuickSlotInfo.PlayerDBId);
				QuickSlotUpdate.InQuickSlotBarIndex(SaveQuickSlotInfo.QuickSlotBarIndex);
				QuickSlotUpdate.InQuickSlotBarSlotIndex(SaveQuickSlotInfo.QuickSlotBarSlotIndex);
				QuickSlotUpdate.InQuickSlotKey(SaveQuickSlotInfo.QuickSlotKey);
				QuickSlotUpdate.InSkillType(SkillType);
				QuickSlotUpdate.InSkillLevel(SaveQuickSlotInfo.QuickBarSkillInfo.SkillLevel);
				QuickSlotUpdate.InSkillName(SaveQuickSlotInfo.QuickBarSkillInfo.SkillName);
				QuickSlotUpdate.InSkillCoolTime(SaveQuickSlotInfo.QuickBarSkillInfo.SkillCoolTime);
				QuickSlotUpdate.InSkillCastingTime(SaveQuickSlotInfo.QuickBarSkillInfo.SkillCastingTime);
				QuickSlotUpdate.InSkillThumbnailImagePath(SaveQuickSlotInfo.QuickBarSkillInfo.SkillImagePath);

				QuickSlotUpdate.Execute();

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

			CPlayer* MyPlayer = Session->MyPlayer;

#pragma region 퀵슬롯 A가 DB에 있는지 확인
			// 해당 퀵슬롯 위치에 정보가 있는지 DB에서 확인
			CDBConnection* DBQuickSlotACheckConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotCheck QuickSlotACheck(*DBQuickSlotACheckConnection);
			QuickSlotACheck.InAccountDBId(AccountId);
			QuickSlotACheck.InPlayerDBId(PlayerId);
			QuickSlotACheck.InQuickSlotBarIndex(QuickSlotBarSwapIndexA);
			QuickSlotACheck.InQuickSlotBarSlotIndex(QuickSlotBarSlotSwapIndexA);
						
			WCHAR QuickSlotAKey[20] = { 0 };
			int16 QuickSlotASkillType;
			int8 QuickSlotASkillLevel;			
			WCHAR QuickSlotASkillName[20] = { 0 };
			int32 QuickSlotASkillCoolTime;
			int32 QuickSlotASkillCastingTime;
			WCHAR QuickSlotASkillImagePath[100] = { 0 };
			
			QuickSlotACheck.OutQuickSlotKey(QuickSlotAKey);
			QuickSlotACheck.OutQuickSlotSkillType(QuickSlotASkillType);
			QuickSlotACheck.OutQuickSlotSkillLevel(QuickSlotASkillLevel);
			QuickSlotACheck.OutQuickSlotSkillName(QuickSlotASkillName);
			QuickSlotACheck.OutQuickSlotSkillCoolTime(QuickSlotASkillCoolTime);
			QuickSlotACheck.OutQuickSlotSkillCastingTime(QuickSlotASkillCastingTime);
			QuickSlotACheck.OutQuickSlotSkillThumbnailImagePath(QuickSlotASkillImagePath);

			QuickSlotACheck.Execute();

			bool QuickSlotAFind = QuickSlotACheck.Fetch();

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotACheckConnection);
			
			// 스왑 요청 A 정보 셋팅
			st_QuickSlotBarSlotInfo SwapAQuickSlotBarInfo;
			SwapAQuickSlotBarInfo.AccountDBId = AccountId;
			SwapAQuickSlotBarInfo.PlayerDBId = PlayerId;
			SwapAQuickSlotBarInfo.QuickSlotBarIndex = QuickSlotBarSwapIndexB;
			SwapAQuickSlotBarInfo.QuickSlotBarSlotIndex = QuickSlotBarSlotSwapIndexB;			
			SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillType = (en_SkillType)QuickSlotASkillType;
			SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillLevel = QuickSlotASkillLevel;
			SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillName = QuickSlotASkillName;
			SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillCoolTime = QuickSlotASkillCoolTime;
			SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillCastingTime = QuickSlotASkillCastingTime;
			SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillImagePath = QuickSlotASkillImagePath;
#pragma endregion
#pragma region 퀵슬롯 B가 DB에 있는지 확인
			// 해당 퀵슬롯 위치에 정보가 있는지 DB에서 확인
			CDBConnection* DBQuickSlotBCheckConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotCheck QuickSlotBCheck(*DBQuickSlotBCheckConnection);
			QuickSlotBCheck.InAccountDBId(AccountId);
			QuickSlotBCheck.InPlayerDBId(PlayerId);
			QuickSlotBCheck.InQuickSlotBarIndex(QuickSlotBarSwapIndexB);
			QuickSlotBCheck.InQuickSlotBarSlotIndex(QuickSlotBarSlotSwapIndexB);

			WCHAR QuickSlotBKey[20] = { 0 };
			int16 QuickSlotBSkillType;
			int8 QuickSlotBSkillLevel;
			WCHAR QuickSlotBSkillName[20] = { 0 };
			int32 QuickSlotBSkillCoolTime;
			int32 QuickSlotBSkillCastingTime;
			WCHAR QuickSlotBSkillImagePath[100] = { 0 };

			QuickSlotBCheck.OutQuickSlotKey(QuickSlotBKey);
			QuickSlotBCheck.OutQuickSlotSkillType(QuickSlotBSkillType);
			QuickSlotBCheck.OutQuickSlotSkillLevel(QuickSlotBSkillLevel);
			QuickSlotBCheck.OutQuickSlotSkillName(QuickSlotBSkillName);
			QuickSlotBCheck.OutQuickSlotSkillCoolTime(QuickSlotBSkillCoolTime);
			QuickSlotBCheck.OutQuickSlotSkillCastingTime(QuickSlotBSkillCastingTime);
			QuickSlotBCheck.OutQuickSlotSkillThumbnailImagePath(QuickSlotBSkillImagePath);

			QuickSlotBCheck.Execute();

			bool QuickSlotBFind = QuickSlotBCheck.Fetch();

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotBCheckConnection);
			
			// 스왑 요청 B 정보 셋팅
			st_QuickSlotBarSlotInfo SwapBQuickSlotBarInfo;
			SwapBQuickSlotBarInfo.AccountDBId = AccountId;
			SwapBQuickSlotBarInfo.PlayerDBId = PlayerId;
			SwapBQuickSlotBarInfo.QuickSlotBarIndex = QuickSlotBarSwapIndexA;
			SwapBQuickSlotBarInfo.QuickSlotBarSlotIndex = QuickSlotBarSlotSwapIndexA;			
			SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillType = (en_SkillType)QuickSlotBSkillType;
			SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillLevel = QuickSlotBSkillLevel;
			SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillName = QuickSlotBSkillName;
			SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillCoolTime = QuickSlotBSkillCoolTime;
			SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillCastingTime = QuickSlotBSkillCastingTime;
			SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillImagePath = QuickSlotBSkillImagePath;

			SwapAQuickSlotBarInfo.QuickSlotKey = QuickSlotBKey;
			SwapBQuickSlotBarInfo.QuickSlotKey = QuickSlotAKey;
#pragma endregion

#pragma region DB에서 퀵슬롯 스왑
			int16 ASkillType = (int16)SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillType;
			int16 BSkillType = (int16)SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillType;

			CDBConnection* DBQuickSlotSwapConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotSwap QuickSlotSwap(*DBQuickSlotSwapConnection);
			QuickSlotSwap.InAccountDBId(AccountId);
			QuickSlotSwap.InPlayerDBId(PlayerId);
			
			QuickSlotSwap.InAQuickSlotBarIndex(SwapBQuickSlotBarInfo.QuickSlotBarIndex);
			QuickSlotSwap.InAQuickSlotBarSlotIndex(SwapBQuickSlotBarInfo.QuickSlotBarSlotIndex);
			QuickSlotSwap.InAQuickSlotSkillType(BSkillType);
			QuickSlotSwap.InAQuickSlotSkillLevel(SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillLevel);
			QuickSlotSwap.InAQuickSlotSKillName(SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillName);
			QuickSlotSwap.InAQuickSlotSkillCoolTime(SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillCoolTime);
			QuickSlotSwap.InAQuickSlotSkillCastingTime(SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillCastingTime);
			QuickSlotSwap.InAQuickSlotSKillImagePath(SwapBQuickSlotBarInfo.QuickBarSkillInfo.SkillImagePath);

			QuickSlotSwap.InBQuickSlotBarIndex(SwapAQuickSlotBarInfo.QuickSlotBarIndex);
			QuickSlotSwap.InBQuickSlotBarSlotIndex(SwapAQuickSlotBarInfo.QuickSlotBarSlotIndex);
			QuickSlotSwap.InBQuickSlotSkillType(ASkillType);
			QuickSlotSwap.InBQuickSlotSkillLevel(SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillLevel);
			QuickSlotSwap.InBQuickSlotSKillName(SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillName);
			QuickSlotSwap.InBQuickSlotSkillCoolTime(SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillCoolTime);
			QuickSlotSwap.InBQuickSlotSkillCastingTime(SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillCastingTime);
			QuickSlotSwap.InBQuickSlotSKillImagePath(SwapAQuickSlotBarInfo.QuickBarSkillInfo.SkillImagePath);

			bool QuickSlotSwapSuccess = QuickSlotSwap.Execute();
			if (QuickSlotSwapSuccess == true)
			{
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

			WCHAR QuickSlotKey[20] = { 0 };
			int16 QuickSlotSkillType;
			int8 QuickSlotSkillLevel;
			WCHAR QuickSlotSkillName[20] = { 0 };
			int32 QuickSlotSkillCoolTime;
			int32 QuickSlotSkillCastingTime;
			WCHAR QuickSlotSkillImagePath[100] = { 0 };

			QuickSlotACheck.OutQuickSlotKey(QuickSlotKey);
			QuickSlotACheck.OutQuickSlotSkillType(QuickSlotSkillType);
			QuickSlotACheck.OutQuickSlotSkillLevel(QuickSlotSkillLevel);
			QuickSlotACheck.OutQuickSlotSkillName(QuickSlotSkillName);
			QuickSlotACheck.OutQuickSlotSkillCoolTime(QuickSlotSkillCoolTime);
			QuickSlotACheck.OutQuickSlotSkillCastingTime(QuickSlotSkillCastingTime);
			QuickSlotACheck.OutQuickSlotSkillThumbnailImagePath(QuickSlotSkillImagePath);

			QuickSlotACheck.Execute();

			bool QuickSlotAFind = QuickSlotACheck.Fetch();

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotCheckConnection);

			// 찾은 퀵슬롯 정보를 초기화
			CDBConnection* DBQuickSlotInitConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotInit QuickSlotInit(*DBQuickSlotInitConnection);
			QuickSlotInit.InAccountDBId(AccountId);
			QuickSlotInit.InPlayerDBId(PlayerId);
			QuickSlotInit.InQuickSlotBarIndex(QuickSlotBarIndex);
			QuickSlotInit.InQuickSlotBarSlotIndex(QuickSlotBarSlotIndex);

			QuickSlotInit.Execute();

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotInitConnection);

			CMessage* ResQuickSlotInitMessage = MakePacketResQuickSlotInit(Session->AccountId, Session->MyPlayer->_GameObjectInfo.ObjectId, QuickSlotBarIndex, QuickSlotBarSlotIndex, QuickSlotKey);
			SendPacket(Session->SessionId, ResQuickSlotInitMessage);
			ResQuickSlotInitMessage->Free();
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
		en_EffectType HitEffectType;

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
			HitEffectType = en_EffectType::EFFECT_FLAME_HARPOON_TARGET;

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
			HitEffectType = en_EffectType::EFFECT_HEALING_LIGHT_TARGET;

			FinalDamage = 100;
			MyPlayer->GetTarget()->OnHeal(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s가 치유의빛을 사용해 %s를 %d만큼 회복했습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), 10);
			MagicSystemString = SpellMessage;
		}
		break;
		case en_SkillType::SKILL_SHAMAN_HEALING_WIND:
		{
			HitEffectType = en_EffectType::EFFECT_HEALING_WIND_TARGET;

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
		CMessage* ResAttackMagicPacket = MakePacketResAttack(
			MyPlayer->_GameObjectInfo.ObjectId,
			MyPlayer->GetTarget()->_GameObjectInfo.ObjectId,
			MyPlayer->_SkillType,
			FinalDamage,
			false);
		SendPacketAroundSector(MyPlayer->GetTarget()->GetCellPosition(), ResAttackMagicPacket);
		ResAttackMagicPacket->Free();

		// Idle로 상태 변경 후 주위섹터에 전송
		MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

		CMessage* ResObjectStateChangePacket = MakePacketResObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
		SendPacketAroundSector(MyPlayer->GetCellPosition(), ResObjectStateChangePacket);
		ResObjectStateChangePacket->Free();

		// 시스템 메세지 전송
		CMessage* ResAttackMagicSystemMessagePacket = MakePacketResChattingBoxMessage(MyPlayer->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, st_Color::White(), MagicSystemString);
		SendPacketAroundSector(MyPlayer->GetCellPosition(), ResAttackMagicSystemMessagePacket);
		ResAttackMagicSystemMessagePacket->Free();

		// HP 변경 전송
		CMessage* ResChangeObjectStat = MakePacketChangeObjectStat(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, MyPlayer->GetTarget()->_GameObjectInfo.ObjectStatInfo);
		SendPacketAroundSector(MyPlayer->GetTarget()->GetCellPosition(), ResChangeObjectStat);
		ResChangeObjectStat->Free();

		// 스펠창 끝
		CMessage* ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId, false);
		SendPacketAroundSector(MyPlayer->GetCellPosition(), ResMagicPacket);
		ResMagicPacket->Free();

		// 이펙트 출력
		CMessage* ResEffectPacket = MakePacketEffect(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, HitEffectType);
		SendPacketAroundSector(MyPlayer->GetCellPosition(), ResEffectPacket);
		ResEffectPacket->Free();
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcTimerCoolTimeEnd(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		int16 SkillType;
		*Message >> SkillType;

		Message->Free();

		st_SkillInfo* SkillInfo = Session->MyPlayer->_SkillBox.FindSkill((en_SkillType)SkillType);
		SkillInfo->CanSkillUse = true;
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcTimerObjectSpawn(CMessage* Message)
{
	int16 SpawnObjectType;
	*Message >> SpawnObjectType;
		
	st_Vector2Int SpawnPosition;
	*Message >> SpawnPosition._X;
	*Message >> SpawnPosition._Y;

	Message->Free();	

	G_ObjectManager->ObjectSpawn((en_GameObjectType)SpawnObjectType, SpawnPosition);
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
CGameServerMessage* CGameServer::MakePacketResLogin(bool Status, int8 PlayerCount, CGameObject** MyPlayersInfo)
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

	if (PlayerCount > 0)
	{
		for (int32 i = 0; i < PlayerCount; i++)
		{
			*LoginMessage << MyPlayersInfo[i]->_GameObjectInfo;
		}
	}

	return LoginMessage;
}

// int32 PlayerDBId
// bool IsSuccess
// wstring PlayerName
CGameServerMessage* CGameServer::MakePacketResCreateCharacter(bool IsSuccess, st_GameObjectInfo CreateCharacterObjectInfo)
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

	*ResItemToInventoryMessage << (int16)en_PACKET_S2C_ITEM_TO_INVENTORY;
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

CGameServerMessage* CGameServer::MakePacketResQuickSlotInit(int64 AccountId, int64 PlayerId, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, wstring QuickSlotKey)
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

	// 퀵슬롯 키
	int8 QuickSlotKeyLen = (int8)(QuickSlotKey.length() * 2);
	*ResQuickSlotInitMessage << QuickSlotKeyLen;
	ResQuickSlotInitMessage->InsertData(QuickSlotKey.c_str(), QuickSlotKeyLen);

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
CGameServerMessage* CGameServer::MakePacketChangeObjectStat(int64 ObjectId, st_StatInfo ChangeObjectStatInfo)
{
	CGameServerMessage* ResChangeObjectStatPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResChangeObjectStatPacket == nullptr)
	{
		return nullptr;
	}

	ResChangeObjectStatPacket->Clear();

	*ResChangeObjectStatPacket << (int16)en_PACKET_S2C_CHANGE_OBJECT_STAT;
	*ResChangeObjectStatPacket << ObjectId;

	*ResChangeObjectStatPacket << ChangeObjectStatInfo;	

	return ResChangeObjectStatPacket;
}

CGameServerMessage* CGameServer::MakePacketResObjectState(int64 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState)
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

CGameServerMessage* CGameServer::MakePacketResSkillToSkillBox(int64 TargetObjectId, st_SkillInfo SkillInfo)
{
	CGameServerMessage* ResSkillToSkillBoxMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResSkillToSkillBoxMessage == nullptr)
	{
		return nullptr;
	}

	ResSkillToSkillBoxMessage->Clear();

	*ResSkillToSkillBoxMessage << (int16)en_PACKET_S2C_SKILL_TO_SKILLBOX;
	*ResSkillToSkillBoxMessage << TargetObjectId;

	*ResSkillToSkillBoxMessage << SkillInfo;	

	return ResSkillToSkillBoxMessage;
}

CGameServerMessage* CGameServer::MakePacketEffect(int64 TargetObjectId, en_EffectType EffectType)
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

	return ResEffectMessage;
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
	CGameServerMessage* JobMessage = CGameServerMessage::GameServerMessageAlloc();
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

void CGameServer::SendPacketAroundSector(st_Vector2Int CellPosition, CMessage* Message)
{
	CChannel* Channel = G_ChannelManager->Find(1);
	vector<CSector*> Sectors = Channel->GetAroundSectors(CellPosition, 1);

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
	vector<CSector*> Sectors = Channel->GetAroundSectors(Session->MyPlayer->GetCellPosition(), 1);

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

void CGameServer::CoolTimeSkillTimerJobCreate(CPlayer* Player, int64 CastingTime, st_SkillInfo* CoolTimeSkillInfo, en_TimerJobType TimerJobType, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex)
{	
	CoolTimeSkillInfo->CanSkillUse = false;

	// 스킬 모션 끝 판단
	// 0.5초 후에 Idle상태로 바꾸기 위해 TimerJob 등록
	st_TimerJob* TimerJob = _TimerJobMemoryPool->Alloc();
	TimerJob->TimerJobExecTick = GetTickCount64() + CastingTime;
	TimerJob->SessionId = Player->_SessionId;
	TimerJob->TimerJobType = TimerJobType;
	TimerJob->TimerJobCancel = false;

	Player->_SkillJob = TimerJob;

	AcquireSRWLockExclusive(&_TimerJobLock);
	_TimerHeapJob->InsertHeap(TimerJob->TimerJobExecTick, TimerJob);
	ReleaseSRWLockExclusive(&_TimerJobLock);

	SetEvent(_TimerThreadWakeEvent);

	float SkillCoolTime = CoolTimeSkillInfo->SkillCoolTime / 1000.0f;

	// 클라에게 쿨타임 표시
	CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(Player->_GameObjectInfo.ObjectId, QuickSlotBarIndex, QuickSlotBarSlotIndex, SkillCoolTime, 1.0f);
	SendPacket(Player->_SessionId, ResCoolTimeStartPacket);
	ResCoolTimeStartPacket->Free();

	// 쿨타임 시간 동안 스킬 사용 못하게 막음
	CoolTimeSkillInfo->CanSkillUse = false;

	// 스킬 쿨타임 얻어옴
	auto FindSkilliterator = G_Datamanager->_Skills.find((int32)CoolTimeSkillInfo->SkillType);
	st_SkillData* ReqSkillData = (*FindSkilliterator).second;

	// 스킬 쿨타임 스킬쿨타임 잡 등록
	st_TimerJob* SkillCoolTimeTimerJob = _TimerJobMemoryPool->Alloc();
	SkillCoolTimeTimerJob->TimerJobExecTick = GetTickCount64() + ReqSkillData->SkillCoolTime;
	SkillCoolTimeTimerJob->SessionId = Player->_SessionId;
	SkillCoolTimeTimerJob->TimerJobType = en_TimerJobType::TIMER_SKILL_COOLTIME_END;

	CGameServerMessage* ResCoolTimeEndMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCoolTimeEndMessage == nullptr)
	{
		return;
	}

	ResCoolTimeEndMessage->Clear();

	*ResCoolTimeEndMessage << (int16)CoolTimeSkillInfo->SkillType;
	SkillCoolTimeTimerJob->TimerJobMessage = ResCoolTimeEndMessage;

	AcquireSRWLockExclusive(&_TimerJobLock);
	_TimerHeapJob->InsertHeap(SkillCoolTimeTimerJob->TimerJobExecTick, SkillCoolTimeTimerJob);
	ReleaseSRWLockExclusive(&_TimerJobLock);

	SetEvent(_TimerThreadWakeEvent);
}

void CGameServer::SpawnObjectTime(CGameObject* SpawnObject, int64 SpawnTime)
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

	*ResObjectSpawnMessage << (int16)SpawnObject->_GameObjectInfo.ObjectType;		
	
	*ResObjectSpawnMessage << SpawnObject->_SpawnPosition;

	SpawnObjectTimerJob->TimerJobMessage = ResObjectSpawnMessage;

	AcquireSRWLockExclusive(&_TimerJobLock);
	_TimerHeapJob->InsertHeap(SpawnObjectTimerJob->TimerJobExecTick, SpawnObjectTimerJob);
	ReleaseSRWLockExclusive(&_TimerJobLock);

	SetEvent(_TimerThreadWakeEvent);
}
