#include "pch.h"
#include "GameServer.h"
#include "DBBind.h"
#include "DBConnection.h"
#include "DBConnectionPool.h"
#include "DBStoreProcedure.h"
#include "DataManager.h"
#include "ChannelManager.h"
#include "ObjectManager.h"
#include "NetworkManager.h"
#include "Inventory.h"
#include "GameServerMessage.h"
#include "Skill.h"
#include "MapManager.h"
#include "QuickSlotBar.h"
#include "Furnace.h"
#include "Sawmill.h"
#include "Day.h"
#include "Math.h"
#include "RectCollision.h"
#include <WS2tcpip.h>
#include <process.h>
#include <atlbase.h>

CGameServer::CGameServer()
{
	//timeBeginPeriod(1);
	_UserDataBaseThread = nullptr;
	_TimerJobThread = nullptr;
	_LogicThread = nullptr;

	// 논시그널 상태 자동리셋
	_UpdateThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_UserDataBaseWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_WorldDataBaseWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_ClientLeaveDBThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_TimerThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	_UpdateThreadEnd = false;
	_NetworkThreadEnd = false;
	_UserDataBaseThreadEnd = false;
	_WorldDataBaseThreadEnd = false;
	_ClientLeaveSaveDBThreadEnd = false;
	_LogicThreadEnd = false;
	_TimerJobThreadEnd = false;	

	// 타이머 잡 전용 SWRLock 초기화
	InitializeSRWLock(&_TimerJobLock);

	// 잡 메모리풀 생성
	_GameServerJobMemoryPool = new CMemoryPoolTLS<st_GameServerJob>();
	// 타이머 잡 메모리풀 생성
	_TimerJobMemoryPool = new CMemoryPoolTLS<st_TimerJob>();

	// 타이머 우선순위 큐 생성
	_TimerHeapJob = new CHeap<int64, st_TimerJob*>(2000);

	_LogicThreadFPS = 0;

	_NetworkThreadWakeCount = 0;
	_NetworkThreadTPS = 0;

	_UserDBThreadTPS = 0;
	_LeaveDBThreadTPS = 0;

	_TimerJobThreadWakeCount = 0;
	_TimerJobThreadTPS = 0;

	_Day = nullptr;
}

CGameServer::~CGameServer()
{
	//timeEndPeriod(1);
}

void CGameServer::GameServerStart(const WCHAR* OpenIP, int32 Port)
{
	CNetworkLib::Start(OpenIP, Port);

	SYSTEM_INFO SI;
	GetSystemInfo(&SI);	

	_Day = new CDay();

	// 맵 정보를 읽어옴
	G_MapManager->MapSave();	

	// 네트워크 매니저에 게임서버 할당
	G_NetworkManager->SetGameServer(this);	

	// 유저 데이터 베이스 쓰레드 시작
	for (int32 i = 0; i < 3; i++)
	{
		_UserDataBaseThread = (HANDLE)_beginthreadex(NULL, 0, UserDataBaseThreadProc, this, 0, NULL);
		CloseHandle(_UserDataBaseThread);
	}

	// 월드 데이터 베이스 쓰레드 시작
	_WorldDataBaseThread = (HANDLE)_beginthreadex(NULL, 0, WorldDataBaseThreadProc, this, 0, NULL);
	// 클라 종료 캐릭터 정보 DB 저장 쓰레드 시작
	_ClientLeaveSaveThread = (HANDLE)_beginthreadex(NULL, 0, ClientLeaveThreadProc, this, 0, NULL);
	// 타이머 잡 쓰레드 시작
	_TimerJobThread = (HANDLE)_beginthreadex(NULL, 0, TimerJobThreadProc, this, 0, NULL);
	// 로직 쓰레드 시작
	_LogicThread = (HANDLE)_beginthreadex(NULL, 0, LogicThreadProc, this, 0, NULL);

	st_GameServerJob* WorldMapTileInfoLoadJob = _GameServerJobMemoryPool->Alloc();
	WorldMapTileInfoLoadJob->Type = en_GameServerJobType::DATA_BASE_MAP_TILE_INFO_LOAD;

	_GameServerWorldDBThreadMessageQue.Enqueue(WorldMapTileInfoLoadJob);	
	SetEvent(_WorldDataBaseWakeEvent);			

	CloseHandle(_WorldDataBaseThread);
	CloseHandle(_TimerJobThread);
	CloseHandle(_LogicThread);
}

void CGameServer::LoginServerConnect(const WCHAR* ConnectIP, int32 Port)
{
	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	InetPtonW(AF_INET, ConnectIP, &ServerAddr.sin_addr);
	ServerAddr.sin_port = htons(Port);

	_LoginServerSock = socket(AF_INET, SOCK_STREAM, 0);

	while (1)
	{
		int32 LoginServerConnectRet = connect(_LoginServerSock, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
		if (LoginServerConnectRet == SOCKET_ERROR)
		{
			DWORD Error = WSAGetLastError();
			G_Logger->WriteStdOut(en_Color::RED, L"LoginServerConnect Error %d\n", Error);
		}
		else if (LoginServerConnectRet == 0)
		{
			break;
		}		
	}
}

unsigned __stdcall CGameServer::UpdateThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_UpdateThreadEnd)
	{
		WaitForSingleObject(Instance->_UpdateThreadWakeEvent, INFINITE);

		
	}
	return 0;
}

unsigned __stdcall CGameServer::UserDataBaseThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_UserDataBaseThreadEnd)
	{
		WaitForSingleObject(Instance->_UserDataBaseWakeEvent, INFINITE);

		while (!Instance->_GameServerUserDBThreadMessageQue.IsEmpty())
		{
			st_GameServerJob* Job = nullptr;

			if (!Instance->_GameServerUserDBThreadMessageQue.Dequeue(&Job))
			{
				break;
			}

			switch (Job->Type)
			{
			case en_GameServerJobType::DATA_BASE_ACCOUNT_CHECK:
				Instance->PacketProcReqDBAccountCheck(Job->Message);
				break;
			case en_GameServerJobType::DATA_BASE_CHARACTER_CHECK:
				Instance->PacketProcReqDBCreateCharacterNameCheck(Job->Message);
				break;									
			case en_GameServerJobType::DATA_BASE_CHARACTER_INFO_SEND:
				Instance->PacketProcReqDBCharacterInfoSend(Job->Message);
				break;			
			}

			Job->Message->Free();

			InterlockedIncrement64(&Instance->_UserDBThreadTPS);		

			Instance->_GameServerJobMemoryPool->Free(Job);
		}
	}
	return 0;
}

unsigned __stdcall CGameServer::WorldDataBaseThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_WorldDataBaseThreadEnd)
	{
		WaitForSingleObject(Instance->_WorldDataBaseWakeEvent, INFINITE);

		while (!Instance->_GameServerWorldDBThreadMessageQue.IsEmpty())
		{
			st_GameServerJob* Job = nullptr;

			if (!Instance->_GameServerWorldDBThreadMessageQue.Dequeue(&Job))
			{
				break;
			}

			switch (Job->Type)
			{	
			case en_GameServerJobType::DATA_BASE_MAP_TILE_INFO_LOAD:				
				Instance->WorldMapTileInfoLoad();
				break;
			default:
				break;
			}

			if (Job->Message != nullptr)
			{
				Job->Message->Free();
			}

			Instance->_GameServerJobMemoryPool->Free(Job);
		}
	}

	return 0;
}

unsigned __stdcall CGameServer::ClientLeaveThreadProc(void* Argument)
{
	CGameServer* Instance = (CGameServer*)Argument;

	while (!Instance->_ClientLeaveSaveDBThreadEnd)
	{
		WaitForSingleObject(Instance->_ClientLeaveDBThreadWakeEvent, INFINITE);

		while (!Instance->_GameServerClientLeaveDBThreadMessageQue.IsEmpty())
		{
			st_GameServerJob* Job = nullptr;

			if (!Instance->_GameServerClientLeaveDBThreadMessageQue.Dequeue(&Job))
			{
				break;
			}

			switch (Job->Type)
			{
			case en_GameServerJobType::DATA_BASE_CHATACTER_INFO_SAVE:
				Instance->PacketProcReqDBLeavePlayerInfoSave(Job->Message);
				break;
			default:
				break;
			}

			if (Job->Message != nullptr)
			{
				Job->Message->Free();
			}

			Instance->_LeaveDBThreadTPS++;

			Instance->_GameServerJobMemoryPool->Free(Job);
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

	int64 LogicUpdatePreviousTime = GetTickCount64();
	int64 LogicUpdateCurrentTime = 0;
	int64 DeltaTime = 0;

	while (!Instance->_LogicThreadEnd)
	{
		LogicUpdateCurrentTime = GetTickCount64();
		DeltaTime = LogicUpdateCurrentTime - LogicUpdatePreviousTime;

		if (DeltaTime >= 20)
		{
			LogicUpdatePreviousTime = LogicUpdateCurrentTime - (DeltaTime - 20);

			if (Instance->_Day != nullptr)
			{
				Instance->_Day->Update();
				G_MapManager->Update();
			}			

			Instance->_LogicThreadFPS++;
		}
	}

	return 0;
}

void CGameServer::PlayerDefaultSetting(int64& AccountId, st_GameObjectInfo& NewCharacterInfo, int8& CharacterCreateSlotIndex)
{
	CDBConnection* DBPlayerDefaultInformationCreateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
	SP::CDBGameCharacterDefaultInfoCreate GameCharacterDefaultInfoCreate(*DBPlayerDefaultInformationCreateConnection);
	GameCharacterDefaultInfoCreate.InAccountDBId(AccountId);
	GameCharacterDefaultInfoCreate.InPlayerDBId(NewCharacterInfo.ObjectId);

	GameCharacterDefaultInfoCreate.Execute();

	G_DBConnectionPool->Push(en_DBConnect::GAME, DBPlayerDefaultInformationCreateConnection);
}

void CGameServer::CreateNewClient(int64 SessionId)
{
	// 세션 찾기
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		Session->PingPacketTime = GetTickCount64();

		for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
		{
			// 플레이어 인덱스 할당
			G_ObjectManager->_PlayersArrayIndexs.Pop(&Session->MyPlayerIndexes[i]);
		}

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
	// 캐릭터 정보를 DB에 기록하기 위해 DB_WORKING 중이라고 기록
	if (Session->IsDummy == false)
	{
		CGameServerMessage* LogOutPacket = MakePacketLoginServerLoginStateChange(Session->AccountId, en_LoginState::LOGIN_OUT);
		SendPacketToLoginServer(LogOutPacket);
		LogOutPacket->Free();
	}
		
	if (Session->MyPlayerIndex >= 0)
	{
		// Session이 게임 입장 이후 접속이 끊겻을 경우
		st_GameServerJob* CharacterInfoSaveJob = _GameServerJobMemoryPool->Alloc();
		CharacterInfoSaveJob->Type = en_GameServerJobType::DATA_BASE_CHATACTER_INFO_SAVE;

		CGameServerMessage* CharacterInfoSaveMessage = CGameServerMessage::GameServerMessageAlloc();
		CharacterInfoSaveMessage->Clear();

		*CharacterInfoSaveMessage << &Session;

		CharacterInfoSaveJob->Message = CharacterInfoSaveMessage;

		_GameServerClientLeaveDBThreadMessageQue.Enqueue(CharacterInfoSaveJob);
		SetEvent(_ClientLeaveDBThreadWakeEvent);
	}
	else
	{
		// Session이 게임 입장 전에 접속이 끊겻을 경우

		for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
		{
			G_ObjectManager->PlayerIndexReturn(Session->MyPlayerIndexes[i]);
		}

		memset(Session->Token, 0, sizeof(Session->Token));

		Session->IsLogin = false;
		Session->IsDummy = false;

		Session->MyPlayerIndex = -1;
		Session->AccountId = 0;

		Session->ClientSock = INVALID_SOCKET;
		closesocket(Session->CloseSock);

		InterlockedDecrement64(&_SessionCount);
		// 세션 인덱스 반납	
		_SessionArrayIndexs.Push(GET_SESSIONINDEX(Session->SessionId));		
	}	
}

void CGameServer::SendPacketToLoginServer(CGameServerMessage* Message)
{
	Message->Encode();

	int32 SendRet = send(_LoginServerSock, Message->GetBufferPtr(), Message->GetUseBufferSize(), 0);
	if (SendRet == SOCKET_ERROR)
	{
		DWORD Error = WSAGetLastError();
		G_Logger->WriteStdOut(en_Color::RED, L"Send LoginServer Error %d \n", Error);
	}
}

void CGameServer::PacketProc(int64 SessionID, CMessage* Message)
{
	WORD MessageType;
	*Message >> MessageType;

	switch (MessageType)
	{
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_GAME_REQ_LOGIN:
		PacketProcReqLogin(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_GAME_CREATE_CHARACTER:
		PacketProcReqCreateCharacter(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_GAME_ENTER:
		PacketProcReqEnterGame(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_CHARACTER_INFO:
		PacketProcReqCharacterInfo(SessionID, Message);
		break;	
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_FACE_DIRECTION:
		PacketProcReqLookAtDirection(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MOVE:
		PacketProcReqMove(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MOVE_STOP:
		PacketProcReqMoveStop(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_SKILL_PROCESS:
		PacketProcReqSkillProcess(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_SKILL_CASTING_CANCEL:
		PacketProcReqSkillCastingCancel(SessionID, Message);
		break;	
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_LEFT_MOUSE_OBJECT_INFO:
		PacketProcReqLeftMouseObjectInfo(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_LEFT_MOUSE_UI_OBJECT_INFO:
		PacketProcReqLeftMouseUIObjectInfo(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_LEFT_MOUSE_DRAG_OBJECTS_SELECT:
		PacketProcReqLeftMouseDragObjectsSelect(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_RIGHT_MOUSE_OBJECT_INFO:
		PacketProcReqRightMouseObjectInfo(SessionID, Message);
		break;	
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_CRAFTING_TABLE_NON_SELECT:
		PacketProcReqCraftingTableNonSelect(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_GATHERING:
		PacketProcReqGathering(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_GATHERING_CANCEL:
		PacketProcReqGatheringCancel(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MESSAGE:
		PacketProcReqChattingMessage(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_LOOTING:
		PacketProcReqItemLooting(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_ITEM_DROP:
		PacketProcReqItemDrop(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_CRAFTING_TABLE_ITEM_ADD:
		PacketProcReqCraftingTableItemAdd(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_CRAFTING_TABLE_MATERIAL_ITEM_SUBTRACT:
		PacketProcReqCraftingTableMaterialItemSubtract(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_CRAFTING_TABLE_COMPLETE_ITEM_SUBTRACT:
		PacketProcReqCraftingTableCompleteItemSubtract(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_CRAFTING_TABLE_CRAFTING_START:
		PacketProcReqCraftingTableCraftingStart(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_CRAFTING_TABLE_CRAFTING_STOP:
		PacketProcReqCraftingTableCraftingStop(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_ITEM_SELECT:
		PacketProcReqItemSelect(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_ITEM_PLACE:
		PacketProcReqItemPlace(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_ITEM_ROTATE:
		PacketProcReqItemRotate(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_SELECT_SKILL_CHARACTERISTIC:
		PacketProcReqSelectSkillCharacteristic(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_LEARN_SKILL:
		PacketProcReqLearnSkill(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_QUICKSLOT_SAVE:
		PacketProcReqQuickSlotSave(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_QUICKSLOT_SWAP:
		PacketProcReqQuickSlotSwap(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_QUICKSLOT_EMPTY:
		PacketProcReqQuickSlotInit(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_CRAFTING_CONFIRM:
		PacketProcReqCraftingConfirm(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_INVENTORY_ITEM_USE:
		PacketProcReqItemUse(SessionID, Message);
		break;	
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_OFF_EQUIPMENT:
		PacketProcReqOffEquipment(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_BUILDING:
		PacketProcReqBuildingInstall(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_SEED_FARMING:
		PacketProcReqSeedFarming(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_PLANT_GROWTH_CHECK:
		PacketProcReqPlantGrowthCheck(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_PARTY_INVITE:
		PacketProcReqPartyInvite(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_PARTY_INVITE_ACCEPT:
		PacketProcReqPartyAccept(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_PARTY_INVITE_REJECT:
		PacketProcReqPartyReject(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_PARTY_QUIT:
		PacketProcReqPartyQuit(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_PARTY_BANISH:
		PacketProcReqPartyBanish(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MENU:
		PacketProcReqMenu(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_INTERACTION:
		PacketProcReqInteraction(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_CS_GAME_REQ_HEARTBEAT:
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_PONG:
		PacketProcReqPong(SessionID, Message);
		break;
	default:
		Disconnect(SessionID);
		break;
	}

	//Message->Free();
}

void CGameServer::PacketProcReqLogin(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

	int64 AccountId;

	do
	{
		if (Session != nullptr)
		{
			// AccountID 셋팅
			*Message >> AccountId;

			Session->AccountId = AccountId;

			int16 IdLen;
			*Message >> IdLen;

			WCHAR* AccountName = (WCHAR*)malloc(sizeof(WCHAR) * IdLen);
			memset(AccountName, 0, sizeof(WCHAR) * IdLen);
			Message->GetData(AccountName, IdLen);

			Session->LoginId = AccountName;

			bool IsDummy;
			*Message >> IsDummy;

			Session->IsDummy = IsDummy;

			if (!IsDummy)
			{
				int8 TokenLen;
				*Message >> TokenLen;

				byte Token[ACCOUNT_TOKEN_LEN];
				Message->GetData((char*)Token, TokenLen);

				// 토큰 저장
				memcpy(Session->Token, Token, TokenLen);
			}

			if (Session->AccountId != 0 && Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			st_GameServerJob* DBAccountCheckJob = _GameServerJobMemoryPool->Alloc();
			DBAccountCheckJob->Type = en_GameServerJobType::DATA_BASE_ACCOUNT_CHECK;

			CGameServerMessage* DBAccountCheckMessage = CGameServerMessage::GameServerMessageAlloc();
			DBAccountCheckMessage->Clear();			

			*DBAccountCheckMessage << Session->SessionId;

			DBAccountCheckJob->Message = DBAccountCheckMessage;

			_GameServerUserDBThreadMessageQue.Enqueue(DBAccountCheckJob);
			SetEvent(_UserDataBaseWakeEvent);

			free(AccountName);
			AccountName = nullptr;
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
		// 캐릭터 이름 길이
		int16 CharacterNameLen;
		*Message >> CharacterNameLen;

		WCHAR* CharacterName = (WCHAR*)malloc(sizeof(WCHAR) * CharacterNameLen);
		memset(CharacterName, 0, sizeof(WCHAR) * CharacterNameLen);
		Message->GetData(CharacterName, CharacterNameLen);

		// 캐릭터 이름 셋팅
		Session->CreateCharacterName = CharacterName;
		free(CharacterName);
		CharacterName = nullptr;

		// 클라에서 선택한 플레이어 인덱스
		byte CharacterCreateSlotIndex;
		*Message >> CharacterCreateSlotIndex;		

		int8 SkillChracteristicType;
		*Message >> SkillChracteristicType;

		st_GameServerJob* DBCharacterCreateJob = _GameServerJobMemoryPool->Alloc();
		DBCharacterCreateJob->Type = en_GameServerJobType::DATA_BASE_CHARACTER_CHECK;

		CGameServerMessage* DBReqCharaterCreateMessage = CGameServerMessage::GameServerMessageAlloc();
		DBReqCharaterCreateMessage->Clear();

		*DBReqCharaterCreateMessage << Session->SessionId;		
		*DBReqCharaterCreateMessage << CharacterCreateSlotIndex;	
		*DBReqCharaterCreateMessage << SkillChracteristicType;		

		DBCharacterCreateJob->Message = DBReqCharaterCreateMessage;		

		_GameServerUserDBThreadMessageQue.Enqueue(DBCharacterCreateJob);
		SetEvent(_UserDataBaseWakeEvent);		
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

			int16 EnterGameCharacterNameLen;
			*Message >> EnterGameCharacterNameLen;

			wstring EnterGameCharacterName;
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
					G_ObjectManager->_PlayersArray[Session->MyPlayerIndex]->_ObjectManagerArrayIndex = Session->MyPlayerIndex;
					break;
				}
			}

			if (FindName == false)
			{
				CMessage* ResEnterGamePacket = MakePacketResEnterGame(FindName, nullptr, nullptr, nullptr);
				SendPacket(Session->SessionId, ResEnterGamePacket);
				ResEnterGamePacket->Free();
				break;
			}
			else
			{	
				CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];
				st_GameObjectJob* EnterGameJob = MakeGameObjectJobPlayerEnterChannel(MyPlayer);

				CMap* EnterMap = G_MapManager->GetMap(1);
				CChannel* EnterChannel = EnterMap->GetChannelManager()->Find(1);
				EnterChannel->_ChannelJobQue.Enqueue(EnterGameJob);				
			}
		}
		else
		{
			break;
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqCharacterInfo(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

	if (Session)
	{
		if (!Session->IsLogin)
		{
			Disconnect(Session->SessionId);
		}

		int64 AccountId;
		*Message >> AccountId;

		if (Session->AccountId != AccountId)
		{
			Disconnect(Session->SessionId);
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
		}
		else
		{
			// 조종중인 캐릭터가 있으면 ObjectId가 다른지 확인
			if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
			{
				Disconnect(Session->SessionId);
			}
		}

		st_GameServerJob* DBCharacterInfoSendJob = _GameServerJobMemoryPool->Alloc();
		DBCharacterInfoSendJob->Type = en_GameServerJobType::DATA_BASE_CHARACTER_INFO_SEND;

		CGameServerMessage* DBCharacterInfoSendMessage = CGameServerMessage::GameServerMessageAlloc();
		DBCharacterInfoSendMessage->Clear();

		*DBCharacterInfoSendMessage << Session->SessionId;

		DBCharacterInfoSendJob->Message = DBCharacterInfoSendMessage;		

		_GameServerUserDBThreadMessageQue.Enqueue(DBCharacterInfoSendJob);
		SetEvent(_UserDataBaseWakeEvent);

		ReturnSession(Session);
	}
}

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

			if (MyPlayer->CheckCantControlStatusAbnormal())
			{
				break;
			}

			float MoveDirectionX;
			*Message >> MoveDirectionX;
			float MoveDirectionY;
			*Message >> MoveDirectionY;

			float GameObjectWorldPositionX;
			*Message >> GameObjectWorldPositionX;
			float GameObjectWorldPositionY;
			*Message >> GameObjectWorldPositionY;

			int8 GameObjectState;
			*Message >> GameObjectState;
			
			st_GameObjectJob* MoveJob = MakeGameObjectJobMove(MoveDirectionX, MoveDirectionY, GameObjectWorldPositionX, GameObjectWorldPositionY, GameObjectState);
			MyPlayer->_GameObjectJobQue.Enqueue(MoveJob);			
		}
		else
		{

		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqMoveStop(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

	if (Session)
	{
		do
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

			float PositionX;
			*Message >> PositionX;
			float PositionY;
			*Message >> PositionY;
			int8 ObjectState;
			*Message >> ObjectState;
		
			st_GameObjectJob* MoveStopJob = MakeGameObjectJobMoveStop(PositionX, PositionY, ObjectState);
			MyPlayer->_GameObjectJobQue.Enqueue(MoveStopJob);			
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqLookAtDirection(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

	if (Session)
	{
		do
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

			float DirectionX;
			*Message >> DirectionX;
			float DirectionY;
			*Message >> DirectionY;			

			st_GameObjectJob* LookAtDirectionJob = MakeGameObjectJobLookAtDirection(DirectionX, DirectionY);
			MyPlayer->_GameObjectJobQue.Enqueue(LookAtDirectionJob);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqSkillProcess(int64 SessionID, CMessage* Message)
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

			if (MyPlayer->CheckCantControlStatusAbnormal())
			{
				break;
			}
			
			int8 QuickSlotBarIndex;
			*Message >> QuickSlotBarIndex;
			int8 QuickSlotBarSlotIndex;
			*Message >> QuickSlotBarSlotIndex;						

			int8 ReqCharacteristicType;
			*Message >> ReqCharacteristicType;			

			int16 ReqSkillType;
			*Message >> ReqSkillType;			

			float SkillDirectionX;
			*Message >> SkillDirectionX;

			float SkillDirecitonY;
			*Message >> SkillDirecitonY;

			st_GameObjectJob* SkillProcessJob = MakeGameObjectJobSkillProcess(QuickSlotBarIndex, QuickSlotBarSlotIndex, ReqCharacteristicType, ReqSkillType, SkillDirectionX, SkillDirecitonY);
			MyPlayer->_GameObjectJobQue.Enqueue(SkillProcessJob);
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqSkillCastingCancel(int64 SessionId, CMessage* Message)
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

			st_GameObjectJob* SkillCastingCancelJob = MakeGameObjectJobSkillCastingCancel();
			MyPlayer->_GameObjectJobQue.Enqueue(SkillCastingCancelJob);
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqGathering(int64 SessionID, CMessage* Message)
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

			switch ((en_GameObjectType)ObjectType)
			{
			case en_GameObjectType::OBJECT_CROP_CORN:
			case en_GameObjectType::OBJECT_CROP_POTATO:
				break;			
			case en_GameObjectType::OBJECT_TREE:
			case en_GameObjectType::OBJECT_STONE:
				{
					CGameObject* FindObject = MyPlayer->GetChannel()->FindChannelObject(ObjectId, (en_GameObjectType)ObjectType);
	
					st_GameObjectJob* GatheringStartJob = MakeGameObjectJobGatheringStart(FindObject);
					MyPlayer->_GameObjectJobQue.Enqueue(GatheringStartJob);
				}				
				break;
			}

			CGameObject* FindObject = MyPlayer->GetChannel()->FindChannelObject(ObjectId, (en_GameObjectType)ObjectType);
			if (FindObject != nullptr && FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::GATHERING)
			{
				st_GameObjectJob* GatheringStartJob = MakeGameObjectJobGatheringStart(FindObject);
				MyPlayer->_GameObjectJobQue.Enqueue(GatheringStartJob);								
			}
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqGatheringCancel(int64 SessionID, CMessage* Message)
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

			st_GameObjectJob* GatheringCancelJob = MakeGameObjectJobGatheringCancel();
			MyPlayer->_GameObjectJobQue.Enqueue(GatheringCancelJob);
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqLeftMouseObjectInfo(int64 SessionId, CMessage* Message)
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

			st_GameObjectJob* DeSpawnMonsterChannelJob = MakeGameObjectJobFindObjectChannel(MyPlayer, ObjectId, ObjectType);
			MyPlayer->GetChannel()->_ChannelJobQue.Enqueue(DeSpawnMonsterChannelJob);					
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqLeftMouseUIObjectInfo(int64 SessionID, CMessage* Message)
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

			int64 OwnerObjectID;
			*Message >> OwnerObjectID;

			int16 OwnerObjectType;
			*Message >> OwnerObjectType;

			int16 UIObjectInfo;
			*Message >> UIObjectInfo;

			int16 LeftMouseItemCategory;
			*Message >> LeftMouseItemCategory;

			switch ((en_UIObjectInfo)UIObjectInfo)
			{
			case en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_FURNACE:
			case en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_SAWMILL:
				{	
					st_GameObjectJob* FindCraftingTableJob = MakeGameObjectJobFindCraftingTableSelectItem(MyPlayer, OwnerObjectID, OwnerObjectType, LeftMouseItemCategory);
					MyPlayer->GetChannel()->_ChannelJobQue.Enqueue(FindCraftingTableJob);					
				}				
				break;		
			}
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqLeftMouseDragObjectsSelect(int64 SessionID, CMessage* Message)
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

			float MouseDragStartPositionX;
			*Message >> MouseDragStartPositionX;

			float MouseDragStartPositionY;
			*Message >> MouseDragStartPositionY;

			float MouseDragEndPositionX;
			*Message >> MouseDragEndPositionX;

			float MouseDragEndPositionY;
			*Message >> MouseDragEndPositionY;

			st_GameObjectJob* MouseDragObjectsSelectJob = MakeGameObjectLeftMouseDragObjectsSelect(PlayerId, MouseDragStartPositionX, MouseDragStartPositionY, MouseDragEndPositionX, MouseDragEndPositionY);
			MyPlayer->GetChannel()->_ChannelJobQue.Enqueue(MouseDragObjectsSelectJob);			
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqRightMouseObjectInfo(int64 SessionId, CMessage* Message)
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
						
			st_GameObjectJob* RightMouseObjectInfoJob = MakeGameObjectJobRightMouseObjectInfo(MyPlayer, ObjectId, ObjectType);
			MyPlayer->GetChannel()->_ChannelJobQue.Enqueue(RightMouseObjectInfoJob);			
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqCraftingTableNonSelect(int64 SessionID, CMessage* Message)
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

			int64 CraftingTableObjectID;
			*Message >> CraftingTableObjectID;

			int16 CraftingTableObjectType;
			*Message >> CraftingTableObjectType;

			st_GameObjectJob* CraftingTableNonSelectJob = MakeGameObjectJobCraftingTableNonSelect(MyPlayer, CraftingTableObjectID, CraftingTableObjectType);
			MyPlayer->GetChannel()->_ChannelJobQue.Enqueue(CraftingTableNonSelectJob);			
		}
	} while (0);

	ReturnSession(Session);
}

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
		int16 ChattingMessageLen;
		*Message >> ChattingMessageLen;

		// 채팅 메세지 얻어오기
		wstring ChattingMessage;
		Message->GetData(ChattingMessage, ChattingMessageLen);
		
		CMap* Map = G_MapManager->GetMap(1);

		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldAroundPlayers(MyPlayer, false);
				
		CMessage* ResChattingMessage = MakePacketResChattingBoxMessage(en_MessageType::MESSAGE_TYPE_CHATTING, st_Color::White(), ChattingMessage);
		SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResChattingMessage);
		ResChattingMessage->Free();
	}

	// 세션 반납
	ReturnSession(Session);
}

void CGameServer::PacketProcReqItemSelect(int64 SessionId, CMessage* Message)
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

			int16 SelectItemTileGridPositionX;
			int16 SelectItemTileGridPositionY;
			*Message >> SelectItemTileGridPositionX;
			*Message >> SelectItemTileGridPositionY;
						
			CItem* SelectItem = MyPlayer->GetInventoryManager()->SelectItem(0, SelectItemTileGridPositionX, SelectItemTileGridPositionY);

			if (SelectItem != nullptr)
			{
				CMessage* ResItemSelectPacket = MakePacketResSelectItem(Session->AccountId, MyPlayer->_GameObjectInfo.ObjectId, SelectItem);
				SendPacket(Session->SessionId, ResItemSelectPacket);
				ResItemSelectPacket->Free();
			}
		} while (0);

		ReturnSession(Session);
	}
}

void CGameServer::PacketProcReqItemPlace(int64 SessionId, CMessage* Message)
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

			int16 PlaceItemTilePositionX;
			*Message >> PlaceItemTilePositionX;
			int16 PlaceItemTilePositionY;
			*Message >> PlaceItemTilePositionY;

			CItem* PlaceItem = MyPlayer->GetInventoryManager()->SwapItem(0, PlaceItemTilePositionX, PlaceItemTilePositionY);

			CMessage* ResPlaceItemPacket = MakePacketResPlaceItem(Session->AccountId, MyPlayer->_GameObjectInfo.ObjectId, PlaceItem, MyPlayer->GetInventoryManager()->_SelectItem);
			SendPacket(Session->SessionId, ResPlaceItemPacket);
			ResPlaceItemPacket->Free();
		} while (0);

		ReturnSession(Session);
	}
}

void CGameServer::PacketProcReqItemRotate(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

	if (Session)
	{
		int64 AccountID;
		int64 PlayerID;

		do
		{
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> AccountID;

			// AccountId가 맞는지 확인
			if (Session->AccountId != AccountID)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> PlayerID;

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
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerID)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			if (MyPlayer->GetInventoryManager()->_SelectItem != nullptr)
			{
				int16 ItemWidth = MyPlayer->GetInventoryManager()->_SelectItem->_ItemInfo.ItemWidth;
				int16 ItemHeight = MyPlayer->GetInventoryManager()->_SelectItem->_ItemInfo.ItemHeight;

				MyPlayer->GetInventoryManager()->_SelectItem->_ItemInfo.ItemWidth = ItemHeight;
				MyPlayer->GetInventoryManager()->_SelectItem->_ItemInfo.ItemHeight = ItemWidth;

				CMessage* ResItemRotatePacket = MakePacketResItemRotate(MyPlayer->_AccountId, MyPlayer->_GameObjectInfo.ObjectId);
				SendPacket(Session->SessionId, ResItemRotatePacket);
			}
			else
			{
				CRASH("선택 중인 아이템이 없는데 아이템 회전 요청");
			}
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqSelectSkillCharacteristic(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

	if (Session)
	{
		int64 AccountID;
		int64 PlayerID;

		do
		{
			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> AccountID;

			// AccountId가 맞는지 확인
			if (Session->AccountId != AccountID)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> PlayerID;

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
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerID)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}			

			int8 SelectCharacteristicType;
			*Message >> SelectCharacteristicType;
		
			st_GameObjectJob* SkillCharacteristicJob = MakeGameObjectJobSelectSkillCharacteristic(SelectCharacteristicType);
			MyPlayer->_GameObjectJobQue.Enqueue(SkillCharacteristicJob);	

		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqLearnSkill(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

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

			bool IsSkillLearn;
			*Message >> IsSkillLearn;			

			int8 LearnSkillCharacteristicType;
			*Message >> LearnSkillCharacteristicType;

			int16 LearnSkillType;
			*Message >> LearnSkillType;

			st_GameObjectJob* SkillLearnJob = MakeGameObjectJobSkillLearn(IsSkillLearn, LearnSkillCharacteristicType, LearnSkillType);
			MyPlayer->_GameObjectJobQue.Enqueue(SkillLearnJob);
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

			int8 QuickSlotBarIndex;
			*Message >> QuickSlotBarIndex;
			int8 QuickSlotBarSlotIndex;
			*Message >> QuickSlotBarSlotIndex;			
			bool IsSkilQuickSlot;
			*Message >> IsSkilQuickSlot;

			if (IsSkilQuickSlot == true)
			{
				// 저장할 기술 정보 파싱				
				int8 QuickSlotSkillLargetCategory;
				*Message >> QuickSlotSkillLargetCategory;
				int8 QuickSlotSkillMediumCategory;
				*Message >> QuickSlotSkillMediumCategory;
				int8 QuickSlotSkillCharacteristicType;
				*Message >> QuickSlotSkillCharacteristicType;
				int16 QuickSlotSkillType;
				*Message >> QuickSlotSkillType;

				st_QuickSlotBarSlotInfo* FindQuickSlotInfo = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarIndex, QuickSlotBarSlotIndex);
				if (FindQuickSlotInfo != nullptr)
				{
					CSkill* FindSkill = MyPlayer->_SkillBox.FindSkill((en_SkillCharacteristic)QuickSlotSkillCharacteristicType, (en_SkillType)QuickSlotSkillType);
					if (FindSkill != nullptr)
					{
						st_QuickSlotBarPosition QuickSlotPosition;
						
						QuickSlotPosition.QuickSlotBarIndex = QuickSlotBarIndex;
						QuickSlotPosition.QuickSlotBarSlotIndex = QuickSlotBarSlotIndex;

						FindQuickSlotInfo->QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_SKILL;
						FindQuickSlotInfo->QuickBarSkill = FindSkill;
						FindQuickSlotInfo->QuickBarItem = nullptr;

						FindSkill->_QuickSlotBarPosition.push_back(QuickSlotPosition);
					}
					else
					{
						CRASH(L"스킬 박스에 없는 스킬을 퀵슬롯에 등록하려고 시도");
					}
				}

				CMessage* ResQuickSlotUpdateMessage = MakePacketResQuickSlotBarSlotSave(*FindQuickSlotInfo);
				SendPacket(Session->SessionId, ResQuickSlotUpdateMessage);
				ResQuickSlotUpdateMessage->Free();
			}

			bool IsItemQuickSlot;
			*Message >> IsItemQuickSlot;

			if (IsItemQuickSlot == true)
			{
				// 저장할 아이템 정보 파싱
				int8 ItemLargeCategory;
				*Message >> ItemLargeCategory;
				int8 ItemMediumCategory;
				*Message >> ItemMediumCategory;
				int16 ItemSmallCategory;
				*Message >> ItemSmallCategory;

				st_QuickSlotBarSlotInfo* FindQuickSlotInfo = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarIndex, QuickSlotBarSlotIndex);
				if (FindQuickSlotInfo != nullptr)
				{
					CItem* FindItem = MyPlayer->GetInventoryManager()->FindInventoryItem(0, (en_SmallItemCategory)ItemSmallCategory);
					if (FindItem != nullptr)
					{
						FindQuickSlotInfo->QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_ITEM;
						FindQuickSlotInfo->QuickBarItem = FindItem;
						FindQuickSlotInfo->QuickBarSkill = nullptr;
					}	
					else
					{
						CRASH(L"스킬 박스에 없는 스킬을 퀵슬롯에 등록하려고 시도");
					}
				}

				CMessage* ResQuickSlotUpdateMessage = MakePacketResQuickSlotBarSlotSave(*FindQuickSlotInfo);
				SendPacket(Session->SessionId, ResQuickSlotUpdateMessage);
				ResQuickSlotUpdateMessage->Free();
			}					
		} while (0);
	}

	ReturnSession(Session);
}

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

			// 스왑 A QuickSlotPosition;
			int8 QuickSlotBarSwapIndexA;
			*Message >> QuickSlotBarSwapIndexA;
			int8 QuickSlotBarSlotSwapIndexA;
			*Message >> QuickSlotBarSlotSwapIndexA;

			// 스왑 B QuickSlotPosition;
			int8 QuickSlotBarSwapIndexB;
			*Message >> QuickSlotBarSwapIndexB;
			int8 QuickSlotBarSlotSwapIndexB;
			*Message >> QuickSlotBarSlotSwapIndexB;

			// 스왑 요청 A 정보 셋팅
			st_QuickSlotBarSlotInfo SwapAQuickSlotBarInfo;
			SwapAQuickSlotBarInfo.AccountDBId = AccountId;
			SwapAQuickSlotBarInfo.PlayerDBId = PlayerDBId;

			// 스왑 하기 위해서 B 퀵슬롯 위치 정보를 넣는다.
			SwapAQuickSlotBarInfo.QuickSlotBarIndex = QuickSlotBarSwapIndexA;
			SwapAQuickSlotBarInfo.QuickSlotBarSlotIndex = QuickSlotBarSlotSwapIndexA;

			// B 퀵슬롯 정보를 가져온다.
			st_QuickSlotBarSlotInfo* FindBQuickslotInfo = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarSwapIndexB, QuickSlotBarSlotSwapIndexB);
			if (FindBQuickslotInfo != nullptr)
			{
				st_QuickSlotBarPosition QuickSlotPosition;
				QuickSlotPosition.QuickSlotBarIndex= QuickSlotBarSwapIndexA;
				QuickSlotPosition.QuickSlotBarSlotIndex = QuickSlotBarSlotSwapIndexA;

				CSkill* BSkill = nullptr;

				if (FindBQuickslotInfo->QuickBarSkill != nullptr)
				{
					BSkill = MyPlayer->_SkillBox.FindSkill(FindBQuickslotInfo->QuickBarSkill->GetSkillInfo()->SkillCharacteristic, FindBQuickslotInfo->QuickBarSkill->GetSkillInfo()->SkillType);

					for (auto QuickSlotPositionIter = BSkill->_QuickSlotBarPosition.begin();
						QuickSlotPositionIter != BSkill->_QuickSlotBarPosition.end();
						++QuickSlotPositionIter)
					{
						st_QuickSlotBarPosition QuickSlotErasePosition = *QuickSlotPositionIter;

						if (QuickSlotErasePosition.QuickSlotBarIndex == QuickSlotBarSwapIndexB && QuickSlotErasePosition.QuickSlotBarSlotIndex == QuickSlotBarSlotSwapIndexB)
						{
							BSkill->_QuickSlotBarPosition.erase(QuickSlotPositionIter);
							break;
						}
					}

					BSkill->_QuickSlotBarPosition.push_back(QuickSlotPosition);
				}				

				switch (FindBQuickslotInfo->QuickSlotBarType)
				{
				case en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_NONE:
					SwapAQuickSlotBarInfo.QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_NONE;
					SwapAQuickSlotBarInfo.QuickBarSkill = nullptr;
					SwapAQuickSlotBarInfo.QuickBarItem = nullptr;
					break;
				case en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_SKILL:
					SwapAQuickSlotBarInfo.QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_SKILL;
					SwapAQuickSlotBarInfo.QuickBarSkill = FindBQuickslotInfo->QuickBarSkill;										
					break;
				case en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_ITEM:
					SwapAQuickSlotBarInfo.QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_ITEM;
					SwapAQuickSlotBarInfo.QuickBarItem = FindBQuickslotInfo->QuickBarItem;					
					break;
				}					
			}
			else
			{
				CRASH("퀵슬롯 정보를 찾을 수 없음");
			}

			// 스왑 요청 B 정보 셋팅
			st_QuickSlotBarSlotInfo SwapBQuickSlotBarInfo;
			SwapBQuickSlotBarInfo.AccountDBId = AccountId;
			SwapBQuickSlotBarInfo.PlayerDBId = PlayerDBId;
			SwapBQuickSlotBarInfo.QuickSlotBarIndex = QuickSlotBarSwapIndexB;
			SwapBQuickSlotBarInfo.QuickSlotBarSlotIndex = QuickSlotBarSlotSwapIndexB;

			st_QuickSlotBarSlotInfo* FindAQuickslotInfo = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarSwapIndexA, QuickSlotBarSlotSwapIndexA);
			if (FindAQuickslotInfo != nullptr)
			{
				st_QuickSlotBarPosition QuickSlotPosition;
				QuickSlotPosition.QuickSlotBarIndex = QuickSlotBarSwapIndexB;
				QuickSlotPosition.QuickSlotBarSlotIndex = QuickSlotBarSlotSwapIndexB;

				if (FindAQuickslotInfo->QuickBarSkill != nullptr)
				{
					CSkill* ASkill = MyPlayer->_SkillBox.FindSkill(FindAQuickslotInfo->QuickBarSkill->GetSkillInfo()->SkillCharacteristic, FindAQuickslotInfo->QuickBarSkill->GetSkillInfo()->SkillType);

					for (auto QuickSlotPositionIter = ASkill->_QuickSlotBarPosition.begin();
						QuickSlotPositionIter != ASkill->_QuickSlotBarPosition.end();
						++QuickSlotPositionIter)
					{
						st_QuickSlotBarPosition QuickSlotErasePosition = *QuickSlotPositionIter;

						if (QuickSlotErasePosition.QuickSlotBarIndex == QuickSlotBarSwapIndexA && QuickSlotErasePosition.QuickSlotBarSlotIndex == QuickSlotBarSlotSwapIndexA)
						{
							ASkill->_QuickSlotBarPosition.erase(QuickSlotPositionIter);
							break;
						}
					}

					ASkill->_QuickSlotBarPosition.push_back(QuickSlotPosition);
				}				

				switch (FindAQuickslotInfo->QuickSlotBarType)
				{
				case en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_NONE:
					SwapBQuickSlotBarInfo.QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_NONE;
					SwapBQuickSlotBarInfo.QuickBarSkill = nullptr;
					SwapBQuickSlotBarInfo.QuickBarItem = nullptr;					
					break;
				case en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_SKILL:
					SwapBQuickSlotBarInfo.QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_SKILL;
					SwapBQuickSlotBarInfo.QuickBarSkill = FindAQuickslotInfo->QuickBarSkill;				
					break;
				case en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_ITEM:
					SwapBQuickSlotBarInfo.QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_ITEM;
					SwapBQuickSlotBarInfo.QuickBarItem = FindAQuickslotInfo->QuickBarItem;					
					break;
				}
			}
			else
			{
				CRASH("퀵슬롯 정보를 찾을 수 없음");
			}						

			MyPlayer->_QuickSlotManager.SwapQuickSlot(SwapBQuickSlotBarInfo, SwapAQuickSlotBarInfo);

			// 클라에게 결과 전송
			CMessage* ResQuickSlotSwapPacket = MakePacketResQuickSlotSwap(
				SwapBQuickSlotBarInfo,
				SwapAQuickSlotBarInfo);
			SendPacket(MyPlayer->_SessionId, ResQuickSlotSwapPacket);
			ResQuickSlotSwapPacket->Free();
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

			int8 InitSkillCharacteristicType;
			*Message >> InitSkillCharacteristicType;

			int16 InitSkillType;
			*Message >> InitSkillType;

			int8 QuickSlotBarIndex;
			*Message >> QuickSlotBarIndex;

			int8 QuickSlotBarSlotIndex;
			*Message >> QuickSlotBarSlotIndex;			

			CSkill* QuickSlotInitSkill = MyPlayer->_SkillBox.FindSkill((en_SkillCharacteristic)InitSkillCharacteristicType, (en_SkillType)InitSkillType);
			if (QuickSlotInitSkill != nullptr)
			{
				// 퀵슬롯에서 정보 찾기
				st_QuickSlotBarSlotInfo* InitQuickSlotBarSlot = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarIndex, QuickSlotBarSlotIndex);
				if (InitQuickSlotBarSlot != nullptr)
				{
					// 퀵슬롯에서 스킬 연결 해제
					InitQuickSlotBarSlot->QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_NONE;
					InitQuickSlotBarSlot->QuickBarSkill = nullptr;
					InitQuickSlotBarSlot->QuickBarItem = nullptr;

					for (auto QuickSlotPositionIter = QuickSlotInitSkill->_QuickSlotBarPosition.begin();
						QuickSlotPositionIter != QuickSlotInitSkill->_QuickSlotBarPosition.end();
						++QuickSlotPositionIter)
					{
						st_QuickSlotBarPosition QuickSlotPosition = *QuickSlotPositionIter;

						if (QuickSlotPosition.QuickSlotBarIndex == QuickSlotBarIndex && QuickSlotPosition.QuickSlotBarSlotIndex == QuickSlotBarSlotIndex)
						{
							QuickSlotInitSkill->_QuickSlotBarPosition.erase(QuickSlotPositionIter);
							break;
						}
					}				
				}
				else
				{
					CRASH("퀵슬롯정보를 찾지 못함");
				}

				CMessage* ResQuickSlotInitMessage = MakePacketResQuickSlotInit(QuickSlotBarIndex, QuickSlotBarSlotIndex);
				SendPacket(Session->SessionId, ResQuickSlotInitMessage);
				ResQuickSlotInitMessage->Free();
			}
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

			st_CraftingItemCategory* FindCraftingCategoryData = nullptr;
			// 제작법 카테고리 찾기
			auto FindCategoryItem = G_Datamanager->_CraftingData.find(ReqCategoryType);
			if (FindCategoryItem == G_Datamanager->_CraftingData.end())
			{
				// 카테고리를 찾지 못함
				break;
			}

			FindCraftingCategoryData = (*FindCategoryItem).second;

			// 완성템 제작에 필요한 재료 목록 찾기
			vector<st_CraftingMaterialItemInfo> RequireMaterialDatas;
			for (CItem* CraftingCompleteItem : FindCraftingCategoryData->CommonCraftingCompleteItems)
			{
				if (CraftingCompleteItem->_ItemInfo.ItemSmallCategory == (en_SmallItemCategory)ReqCraftingItemType)
				{
					RequireMaterialDatas = CraftingCompleteItem->_ItemInfo.Materials;
				}
			}
								
			bool InventoryCheck = true;

			// 인벤토리에 재료템이 있는지 확인
			for (st_CraftingMaterialItemInfo CraftingMaterialItemInfo : ReqMaterials)
			{
				// 인벤토리에 재료템 목록을 가지고 옴
				vector<CItem*> FindMaterialItem;
				vector<CItem*> FindMaterials = MyPlayer->GetInventoryManager()->FindAllInventoryItem(0, CraftingMaterialItemInfo.MaterialItemType);

				if (FindMaterials.size() > 0)
				{
					for (CItem* FindMaterialItem : FindMaterials)
					{
						// 제작템을 한개 만들때 필요한 재료의 개수를 얻어온다.
						int16 OneReqMaterialCount = 0;
						for (st_CraftingMaterialItemInfo ReqMaterialCountData : RequireMaterialDatas)
						{
							if (FindMaterialItem->_ItemInfo.ItemSmallCategory == ReqMaterialCountData.MaterialItemType)
							{
								OneReqMaterialCount = ReqMaterialCountData.ItemCount;
								break;
							}
						}

						// 클라가 요청한 개수 * 한개 만들때 필요한 개수 만큼 인벤토리에서 차감한다.
						int16 ReqCraftingItemTotalCount = ReqCraftingItemCount * OneReqMaterialCount;
						FindMaterialItem->_ItemInfo.ItemCount -= ReqCraftingItemTotalCount;
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

			bool IsExistItem = false;
			int8 SlotIndex = 0;

			int16 FindItemGridPositionX = -1;
			int16 FindItemGridPositionY = -1;

			CItem* FindItem = MyPlayer->GetInventoryManager()->FindInventoryItem(0, (en_SmallItemCategory)ReqCraftingItemType);
			if (FindItem == nullptr)
			{
				// 가방에 완성한 제작템이 없을 경우 새로 생성해준다.
				CItem* CraftingItem = G_ObjectManager->ItemCreate((en_SmallItemCategory)ReqCraftingItemType);

				MyPlayer->GetInventoryManager()->InsertItem(0, CraftingItem);

				FindItemGridPositionX = CraftingItem->_ItemInfo.ItemTileGridPositionX;
				FindItemGridPositionY = CraftingItem->_ItemInfo.ItemTileGridPositionY;				
			}
			else
			{
				// 인벤토리에 제작템이 이미 있으면 개수만 늘려준다.

				IsExistItem = true;
				FindItem->_ItemInfo.ItemCount += ReqCraftingItemCount;

				FindItemGridPositionX = FindItem->_ItemInfo.ItemTileGridPositionX;
				FindItemGridPositionY = FindItem->_ItemInfo.ItemTileGridPositionY;
			}			
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
						
			int16 UseItemSmallCategory;
			*Message >> UseItemSmallCategory;
			int16 UseItemTileGridPositonX;
			*Message >> UseItemTileGridPositonX;
			int16 UseItemTileGridPositionY;
			*Message >> UseItemTileGridPositionY;
			
			CItem* UseItem = MyPlayer->GetInventoryManager()->FindInventoryItem(0, (en_SmallItemCategory)UseItemSmallCategory);
			if (UseItem != nullptr)
			{
				switch (UseItem->_ItemInfo.ItemSmallCategory)
				{
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_LONG_SWORD_WOOD:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_DAGGER_WOOD:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
					{
						st_GameObjectJob* DoItemEquipmentJob = MakeGameObjectJobOnEquipment(UseItem);
						MyPlayer->_GameObjectJobQue.Enqueue(DoItemEquipmentJob);
					}					
					break;												
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEALTH_RESTORATION_POTION_SMALL:
					{						
						// 체력 포션 사용
						st_GameObjectJob* ItemHPHealJob = MakeGameObjectJobItemHPHeal(UseItem->_ItemInfo.ItemSmallCategory);
						MyPlayer->_GameObjectJobQue.Enqueue(ItemHPHealJob);
					}
					break;		
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_MANA_RESTORATION_POTION_SMALL:
					{
						st_GameObjectJob* MPHealJob = MakeGameObjectJobMPHeal(MyPlayer, UseItem->_ItemInfo.ItemHealPoint);
						MyPlayer->_GameObjectJobQue.Enqueue(MPHealJob);
					}
					break;
				default:
					{
						CMessage* CommonErrorPacket = MakePacketCommonError(en_GlobalMessageType::GLOBAL_FAULT_ITEM_USE, UseItem->_ItemInfo.ItemName.c_str());
						SendPacket(Session->SessionId, CommonErrorPacket);
						CommonErrorPacket->Free();
					}
					break;
				}
			}
			
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqOffEquipment(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

	if (Session)
	{
		do
		{
			int64 AccountID;
			int64 PlayerID;

			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> AccountID;

			// AccountId가 맞는지 확인
			if (Session->AccountId != AccountID)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> PlayerID;

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
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerID)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			int8 EquipmentParts;
			*Message >> EquipmentParts;

			st_GameObjectJob* OffEquipmentParts = MakeGameObjectJobOffEquipment(EquipmentParts);
			MyPlayer->_GameObjectJobQue.Enqueue(OffEquipmentParts);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqBuildingInstall(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

	if (Session)
	{
		do
		{
			int64 AccountID;
			int64 PlayerID;

			if (!Session->IsLogin)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> AccountID;

			// AccountId가 맞는지 확인
			if (Session->AccountId != AccountID)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> PlayerID;

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
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerID)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			short BuildingTileCount;
			*Message >> BuildingTileCount;

			vector<st_TileInfo> TileInfos;			
			
			for (short i = 0; i < BuildingTileCount; i++)
			{
				st_TileInfo TileInfo;
				
				*Message >> TileInfo.IsOccupation;
				*Message >> TileInfo.OwnerObjectID;
				*Message >> TileInfo.Position.X;
				*Message >> TileInfo.Position.Y;

				TileInfos.push_back(TileInfo);
			}

			if (TileInfos.size() > 0)
			{
				CMap* MainMap = G_MapManager->GetMap(1);
				if (MainMap != nullptr)
				{
					bool BuildingInstallSuccess = MainMap->BuildingInstall(TileInfos);
					if (!BuildingInstallSuccess)
					{
						CMessage* ResBuildingInstallPacket = MakePacketResBuildingInstall(!BuildingInstallSuccess, TileInfos);
						SendPacket(Session->SessionId, ResBuildingInstallPacket);
						ResBuildingInstallPacket->Free();
					}
				}
			}
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqSeedFarming(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

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

			int16 ReqSeedItemSmallCategory;
			*Message >> ReqSeedItemSmallCategory;

			st_GameObjectJob* ChannelReqSeedFarming = MakeGameObjectJobSeedFarming(MyPlayer, ReqSeedItemSmallCategory);
			MyPlayer->GetChannel()->_ChannelJobQue.Enqueue(ChannelReqSeedFarming);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqPlantGrowthCheck(int64 SessionID, CMessage* Message)
{	
	st_Session* Session = FindSession(SessionID);

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

			// AccountId가 맞는지 확인DD
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

			int64 PlantObjectID;
			*Message >> PlantObjectID;

			int16 PlantObjectType;
			*Message >> PlantObjectType;

			CChannel* Channel = MyPlayer->GetChannel();
			if (Channel != nullptr)
			{
				st_GameObjectJob* PlantGrowthCheckJob = MakeGameObjectJobPlantGrowthCheck(MyPlayer, PlantObjectID, PlantObjectType);
				Channel->_ChannelJobQue.Enqueue(PlantGrowthCheckJob);
			}
			
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqPartyInvite(int64 SessionID, CMessage* Message)
{
	// 그룹 초대 요청 처리
	st_Session* Session = FindSession(SessionID);

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

			int64 PartyPlayerID;
			*Message >> PartyPlayerID;			

			st_GameObjectJob* PartyInviteJob = MakeGameObjectJobPartyInvite(MyPlayer, PartyPlayerID);
			MyPlayer->GetChannel()->_ChannelJobQue.Enqueue(PartyInviteJob);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqPartyAccept(int64 SessionID, CMessage* Message)
{
	// 그룹 초대 수락 요청 처리
	st_Session* Session = FindSession(SessionID);

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

			int64 ReqPartyPlayerID;
			*Message >> ReqPartyPlayerID;

			st_GameObjectJob* PartyAcceptJob = MakeGameObjectJobPartyAccept(ReqPartyPlayerID, MyPlayer->_GameObjectInfo.ObjectId);
			MyPlayer->GetChannel()->_ChannelJobQue.Enqueue(PartyAcceptJob);
		} while (0);
	}

	ReturnSession(Session);	
}

void CGameServer::PacketProcReqPartyReject(int64 SessionID, CMessage* Message)
{
	// 그룹 초대 수락 요청 처리
	st_Session* Session = FindSession(SessionID);

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

			int64 ReqPartyInvitePlayerID;
			*Message >> ReqPartyInvitePlayerID;

			st_GameObjectJob* PartyAcceptJob = MakeGameObjectJobPartyReject(MyPlayer->_GameObjectInfo.ObjectId, ReqPartyInvitePlayerID);
			MyPlayer->GetChannel()->_ChannelJobQue.Enqueue(PartyAcceptJob);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqPartyQuit(int64 SessionID, CMessage* Message)
{
	// 그룹 탈퇴 요청 처리
	st_Session* Session = FindSession(SessionID);

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

			st_GameObjectJob* PartyInviteJob = MakeGameObjectJobPartyQuit(MyPlayer->_GameObjectInfo.ObjectId);
			MyPlayer->GetChannel()->_ChannelJobQue.Enqueue(PartyInviteJob);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqPartyBanish(int64 SessionID, CMessage* Message)
{
	// 그룹 추방 요청 처리
	st_Session* Session = FindSession(SessionID);

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

			int64 PartyBanishPlayerID;
			*Message >> PartyBanishPlayerID;

			st_GameObjectJob* PartyBanishJob = MakeGameObjectJobPartyBanish(MyPlayer, PartyBanishPlayerID);
			MyPlayer->GetChannel()->_ChannelJobQue.Enqueue(PartyBanishJob);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqMenu(int64 SessionID, CMessage* Message)
{
	// 그룹 추방 요청 처리
	st_Session* Session = FindSession(SessionID);

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

			int8 MenuType;
			*Message >> MenuType;

			st_GameObjectJob* MenuJob = MakeGameObjectMenu(MenuType, MyPlayer);
			MyPlayer->GetChannel()->_ChannelJobQue.Enqueue(MenuJob);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqInteraction(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

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

			int8 InteractionType;
			*Message >> InteractionType;

			int16 ObjectType;
			*Message >> ObjectType;

			int64 ObjectID;
			*Message >> ObjectID;

			st_GameObjectJob* InteractionJob = MakeGameObjectJobInteraction(InteractionType, ObjectType, ObjectID);
			MyPlayer->_GameObjectJobQue.Enqueue(InteractionJob);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqItemLooting(int64 SessionId, CMessage* Message)
{
	// 아이템 줍기 처리
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

			// AccountId가 맞는지 확인DD
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			st_PositionInfo ItemPosition;
			int8 ItemState;			

			*Message >> ItemState;
			*Message >> ItemPosition.CollisionPosition.X;
			*Message >> ItemPosition.CollisionPosition.Y;			

			Vector2Int ItemCellPosition;
			ItemCellPosition.X = ItemPosition.CollisionPosition.X;
			ItemCellPosition.Y = ItemPosition.CollisionPosition.Y;

			int64 MapID = 1;
			// 루팅 위치에 아이템들을 가져온다.
			CItem** Items = G_MapManager->GetMap(MapID)->FindItem(ItemCellPosition);
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

					st_GameObjectJob* ItemSaveJob = MakeGameObjectJobItemSave(Items[i]);
					MyPlayer->_GameObjectJobQue.Enqueue(ItemSaveJob);					
				}
			}

		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqItemDrop(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

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

			int16 DropItemType;
			*Message >> DropItemType;

			int32 DropItemCount;
			*Message >> DropItemCount;

			st_GameObjectJob* DropItemJob = MakeGameObjectJobItemDrop(DropItemType, DropItemCount);
			MyPlayer->_GameObjectJobQue.Enqueue(DropItemJob);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqCraftingTableItemAdd(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

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

			int64 CraftingTableObjectID;
			*Message >> CraftingTableObjectID;			

			int16 InputItemSmallCategory;
			*Message >> InputItemSmallCategory;

			int16 InputItemCount;
			*Message >> InputItemCount;

			CMap* Map = G_MapManager->GetMap(1);
			CChannel* Channel = Map->GetChannelManager()->Find(1);

			// 제작대 찾음
			CGameObject* CraftingTable = Channel->FindChannelObject(CraftingTableObjectID, en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE);
			if (CraftingTable != nullptr)
			{	
				st_GameObjectJob* CraftingTableItemAddJob = MakeGameObjectJobCraftingTableItemAdd(MyPlayer, InputItemSmallCategory, InputItemCount);
				CraftingTable->_GameObjectJobQue.Enqueue(CraftingTableItemAddJob);
			}
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqCraftingTableMaterialItemSubtract(int64 SessionID, CMessage* Message)
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
			
			int64 OwnerCraftingTableObjectID;
			*Message >> OwnerCraftingTableObjectID;

			int16 OwnerGameObjectType;
			*Message >> OwnerGameObjectType;

			int16 SubtractItemSmallCategory;
			*Message >> SubtractItemSmallCategory;

			int16 SubtractItemCount;
			*Message >> SubtractItemCount;

			SubtractItemCount = 1;

			CMap* Map = G_MapManager->GetMap(1);
			CChannel* Channel = Map->GetChannelManager()->Find(1);

			CGameObject* CraftingTable = Channel->FindChannelObject(OwnerCraftingTableObjectID, en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE);
			if (CraftingTable != nullptr)
			{
				st_GameObjectJob* CraftingTableItemSubtractJob = MakeGameObjectJobCraftingTableMaterialItemSubtract(MyPlayer, SubtractItemSmallCategory, SubtractItemCount);
				CraftingTable->_GameObjectJobQue.Enqueue(CraftingTableItemSubtractJob);
			}
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqCraftingTableCompleteItemSubtract(int64 SessionID, CMessage* Message)
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

			int64 OwnerCraftingTableObjectID;
			*Message >> OwnerCraftingTableObjectID;

			int16 OwnerGameObjectType;
			*Message >> OwnerGameObjectType;

			int16 SubtractItemSmallCategory;
			*Message >> SubtractItemSmallCategory;

			int16 SubtractItemCount;
			*Message >> SubtractItemCount;

			SubtractItemCount = 1;

			CMap* Map = G_MapManager->GetMap(1);
			CChannel* Channel = Map->GetChannelManager()->Find(1);

			CGameObject* CraftingTable = Channel->FindChannelObject(OwnerCraftingTableObjectID, en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE);
			if (CraftingTable != nullptr)
			{
				st_GameObjectJob* CraftingTableItemSubtractJob = MakeGameObjectJobCraftingTableCompleteItemSubtract(MyPlayer, SubtractItemSmallCategory, SubtractItemCount);
				CraftingTable->_GameObjectJobQue.Enqueue(CraftingTableItemSubtractJob);
			}
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqCraftingTableCraftingStart(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

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

			int64 CraftingTableObjectID;
			*Message >> CraftingTableObjectID;			

			int16 CraftingCompleteItemType;
			*Message >> CraftingCompleteItemType;

			int16 CraftingCount;
			*Message >> CraftingCount;

			CMap* Map = G_MapManager->GetMap(1);
			CChannel* Channel = Map->GetChannelManager()->Find(1);

			CGameObject* CraftingTable = Channel->FindChannelObject(CraftingTableObjectID, en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE);
			if (CraftingTable != nullptr)
			{
				switch (CraftingTable->_GameObjectInfo.ObjectType)
				{
				case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
				case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
					{
						st_GameObjectJob* CraftingTableCraftStratJob = MakeGameObjectJobCraftingTableStart(MyPlayer, (en_SmallItemCategory)CraftingCompleteItemType, CraftingCount);
						CraftingTable->_GameObjectJobQue.Enqueue(CraftingTableCraftStratJob);
					}
					break;
				}
			}
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqCraftingTableCraftingStop(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

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

			int64 CraftingTableObjectID;
			*Message >> CraftingTableObjectID;		

			CMap* Map = G_MapManager->GetMap(1);
			CChannel* Channel = Map->GetChannelManager()->Find(1);

			CGameObject* CraftingTable = Channel->FindChannelObject(CraftingTableObjectID, en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE);
			if (CraftingTable != nullptr)
			{
				switch (CraftingTable->_GameObjectInfo.ObjectType)
				{
				case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
				case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
					{
						st_GameObjectJob* CraftingTableCraftStratJob = MakeGameObjectJobCraftingTableCancel(MyPlayer);
						CraftingTable->_GameObjectJobQue.Enqueue(CraftingTableCraftStratJob);					
					}
					break;
				}
			}
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqPong(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

	if (Session)
	{
		Session->PingPacketTime = GetTickCount64();		
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBAccountCheck(CMessage* Message)
{
	int64 SessionID;
	*Message >> SessionID;

	st_Session* Session = FindSession(SessionID);

	bool Status = LOGIN_SUCCESS;

	if (Session)
	{
		int64 ClientAccountId = Session->AccountId;

		// AccountServer에 입력받은 AccountID가 있는지 확인한다.
		// 더미일 경우에는 토큰은 확인하지 않는다.

		int DBToken = 0;
		BYTE OutToken[ACCOUNT_TOKEN_LEN] = { 0 };

		if (Session->IsDummy == false)
		{
			// AccountNo와 Token으로 AccountServerDB 접근해서 데이터가 있는지 확인
			CDBConnection* AccountDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);

			SP::CDBAccountTokenGet AccountTokenGet(*AccountDBConnection);
			AccountTokenGet.InAccountID(ClientAccountId); // AccountId 입력

			TIMESTAMP_STRUCT TokenCreateTime;

			AccountTokenGet.OutTokenTime(TokenCreateTime);
			AccountTokenGet.OutToken(OutToken);

			AccountTokenGet.Execute();

			AccountTokenGet.Fetch();

			G_DBConnectionPool->Push(en_DBConnect::ACCOUNT, AccountDBConnection); // 풀 반납
		}

		// DB 토큰과 클라로부터 온 토큰이 같으면 로그인 최종성공
		if (Session->IsDummy == true || memcmp(Session->Token, OutToken, ACCOUNT_TOKEN_LEN) == 0)
		{
			Session->IsLogin = true;
			// 클라가 소유하고 있는 플레이어들을 DB로부터 긁어온다.
			CDBConnection* GameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

			SP::CDBGameServerPlayersGet ClientPlayersGet(*GameServerDBConnection);
			ClientPlayersGet.InAccountID(ClientAccountId);

			int64 PlayerId = 0;
			WCHAR PlayerName[100] = { 0 };
			int8 PlayerIndex = 0;
			int32 PlayerLevel = 0;
			int32 PlayerStr = 0;
			int32 PlayerDex = 0;
			int32 PlayerInt = 0;
			int32 PlayerLuck = 0;
			int32 PlayerCurrentHP = 0;
			int32 PlayerMaxHP = 0;
			int32 PlayerCurrentMP = 0;
			int32 PlayerMaxMP = 0;
			int16 PlayerAutoRecoveyHPPercent = 0;
			int16 PlayerAutoRecoveyMPPercent = 0;
			int32 PlayerMinAttackPoint = 0;
			int32 PlayerMaxAttackPoint = 0;			
			int16 PlayerSpiritPoint = 0;
			int16 PlayerAttackHitRate = 0;
			float PlayerSpellCastingRate = 0;
			int32 PlayerDefence = 0;
			int16 PlayerEvasionRate = 0;
			int16 PlayerAttackCriticalPoint = 0;
			int16 PlayerSpellCriticalPoint = 0;
			int16 PlayerAttackCriticalResistance = 0;
			int16 PlayerStatusCriticalResistance = 0;
			float PlayerSpeed = 0;
			int32 PlayerLastPositionY = 0;
			int32 PlayerLastPositionX = 0;
			int64 PlayerCurrentExperience = 0;
			int64 PlayerRequireExperience = 0;
			int64 PlayerTotalExperience = 0;
			int8 PlayerSkillMaxPoint = 0;

			ClientPlayersGet.OutPlayerDBID(PlayerId);
			ClientPlayersGet.OutPlayerName(PlayerName);			
			ClientPlayersGet.OutPlayerIndex(PlayerIndex);
			ClientPlayersGet.OutLevel(PlayerLevel);
			ClientPlayersGet.OutStr(PlayerStr);
			ClientPlayersGet.OutDex(PlayerDex);
			ClientPlayersGet.OutInt(PlayerInt);
			ClientPlayersGet.OutLuck(PlayerLuck);
			ClientPlayersGet.OutCurrentHP(PlayerCurrentHP);
			ClientPlayersGet.OutMaxHP(PlayerMaxHP);
			ClientPlayersGet.OutCurrentMP(PlayerCurrentMP);
			ClientPlayersGet.OutMaxMP(PlayerMaxMP);
			ClientPlayersGet.OutAutoRecoveryHPPercent(PlayerAutoRecoveyHPPercent);
			ClientPlayersGet.OutAutoRecoveryMPPercent(PlayerAutoRecoveyMPPercent);
			ClientPlayersGet.OutMinAttackPoint(PlayerMinAttackPoint);
			ClientPlayersGet.OutMaxAttackPoint(PlayerMaxAttackPoint);
			ClientPlayersGet.OutAttackHitRate(PlayerAttackHitRate);
			ClientPlayersGet.OutSpiritPoint(PlayerSpiritPoint);
			ClientPlayersGet.OutSpellCastingRate(PlayerSpellCastingRate);
			ClientPlayersGet.OutDefence(PlayerDefence);
			ClientPlayersGet.OutEvasionRate(PlayerEvasionRate);
			ClientPlayersGet.OutAttackCriticalPoint(PlayerAttackCriticalPoint);
			ClientPlayersGet.OutSpellCriticalPoint(PlayerSpellCriticalPoint);
			ClientPlayersGet.OutAttackCriticalResistance(PlayerAttackCriticalResistance);
			ClientPlayersGet.OutStatusAbnormalResistance(PlayerStatusCriticalResistance);
			ClientPlayersGet.OutSpeed(PlayerSpeed);
			ClientPlayersGet.OutLastPositionY(PlayerLastPositionY);
			ClientPlayersGet.OutLastPositionX(PlayerLastPositionX);
			ClientPlayersGet.OutCurrentExperience(PlayerCurrentExperience);
			ClientPlayersGet.OutRequireExperience(PlayerRequireExperience);
			ClientPlayersGet.OutTotalExperience(PlayerTotalExperience);						
			ClientPlayersGet.OutSkillMaxPoint(PlayerSkillMaxPoint);

			bool FindPlyaerCharacter = ClientPlayersGet.Execute();

			int8 PlayerCount = 0;

			while (ClientPlayersGet.Fetch())
			{
				// 플레이어 정보 셋팅			
				CPlayer* NewPlayerCharacter = G_ObjectManager->_PlayersArray[Session->MyPlayerIndexes[PlayerCount]];
				NewPlayerCharacter->_GameObjectInfo.ObjectId = PlayerId;
				NewPlayerCharacter->_GameObjectInfo.ObjectName = PlayerName;				
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Level = PlayerLevel;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.HP = PlayerCurrentHP;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxHP = PlayerMaxHP;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MP = PlayerCurrentMP;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxMP = PlayerMaxMP;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = PlayerAutoRecoveyHPPercent;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = PlayerAutoRecoveyMPPercent;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MinAttackPoint = PlayerMinAttackPoint;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxAttackPoint = PlayerMaxAttackPoint;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.SpiritPoint = PlayerSpiritPoint;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.AttackHitRate = PlayerAttackHitRate;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.SpellCastingRate = PlayerSpellCastingRate;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Defence = PlayerDefence;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.EvasionRate = PlayerEvasionRate;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.AttackCriticalPoint = PlayerAttackCriticalPoint;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.SpellCriticalPoint = PlayerSpellCriticalPoint;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Speed = PlayerSpeed;
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxSpeed = PlayerSpeed;
				NewPlayerCharacter->_GameObjectInfo.ObjectSkillMaxPoint = PlayerSkillMaxPoint;
				NewPlayerCharacter->_GameObjectInfo.ObjectSkillPoint = PlayerSkillMaxPoint;
				NewPlayerCharacter->_SpawnPosition.Y = PlayerLastPositionY;
				NewPlayerCharacter->_SpawnPosition.X = PlayerLastPositionX;
				NewPlayerCharacter->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;							
				NewPlayerCharacter->_GameObjectInfo.OwnerObjectId = 0;
				NewPlayerCharacter->_GameObjectInfo.PlayerSlotIndex = PlayerIndex;
				NewPlayerCharacter->_SessionId = Session->SessionId;
				NewPlayerCharacter->_AccountId = Session->AccountId;
				NewPlayerCharacter->_Experience.CurrentExperience = PlayerCurrentExperience;
				NewPlayerCharacter->_Experience.RequireExperience = PlayerRequireExperience;
				NewPlayerCharacter->_Experience.TotalExperience = PlayerTotalExperience;				

				PlayerCount++;
			}

			G_DBConnectionPool->Push(en_DBConnect::GAME, GameServerDBConnection);

			// 클라에게 로그인 응답 패킷을 보내면서 캐릭터의 정보를 함께 보낸다.
			CMessage* ResLoginMessage = MakePacketResLogin(Status, PlayerCount, Session->MyPlayerIndexes);
			SendPacket(Session->SessionId, ResLoginMessage);
			ResLoginMessage->Free();
		}
		else
		{
			Session->IsLogin = false;
		}
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBCreateCharacterNameCheck(CMessage* Message)
{
	int64 SessionID;
	*Message >> SessionID;

	st_Session* Session = FindSession(SessionID);

	int64 PlayerDBId = 0;

	if (Session != nullptr)
	{
		// 요청한 캐릭터 슬롯 인덱스
		int8 ReqCharacterCreateSlotIndex;
		*Message >> ReqCharacterCreateSlotIndex;

		int8 ReqSkillCharacteristic;
		*Message >> ReqSkillCharacteristic;

#pragma region 생성할 캐릭터가 DB에 있는지 확인
		// 클라에서 생성할 유저의 이름을 가져온다.
		wstring CreateCharacterName = Session->CreateCharacterName;

		// GameServerDB에 해당 유저가 이미 있는지 확인한다.
		CDBConnection* UserNameCreateGameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		// GameServerDB에 캐릭터 이름이 있는지 확인하는 프로시저 클래스
		SP::CDBGameServerCharacterNameGet CharacterNameGet(*UserNameCreateGameServerDBConnection);
		CharacterNameGet.InCharacterName(CreateCharacterName);

		// DB 요청 실행
		CharacterNameGet.Execute();

		// 결과 받기
		bool CharacterNameFind = CharacterNameGet.Fetch();		
		
#pragma endregion		
#pragma region 캐릭터 생성 후 클라에 캐릭터 생성 응답 보내기
		// 캐릭터가 존재하지 않을 경우
		if (!CharacterNameFind)
		{
			// 요청한 캐릭터의 1레벨에 해당하는 데이터를 읽어온다.
			st_StatInfo NewCharacterStatus;
			auto FindStatus = G_Datamanager->_WarriorStatus.find(1);
			NewCharacterStatus = *(*FindStatus).second;
			
			// GameServerDB에 새로운 캐릭터 저장하는 프로시저 클래스
			
			int64 CurrentExperience = 0;

			auto FindLevel = G_Datamanager->_LevelDatas.find(NewCharacterStatus.Level);
			st_LevelData LevelData = *(*FindLevel).second;

			int32 NewCharacterPositionY = 50;
			int32 NewCharacterPositionX = 25;
			int8 NewCharacterSkillPoint = 100;
						
			NewCharacterStatus.MaxHP = NewCharacterStatus.HP + (NewCharacterStatus.Con * 10);
			NewCharacterStatus.MaxMP = NewCharacterStatus.MP + (NewCharacterStatus.Wis * 15);
			NewCharacterStatus.AutoRecoveryHPPercent = 2;
			NewCharacterStatus.AutoRecoveryMPPercent = 1;

			NewCharacterStatus.MinAttackPoint = NewCharacterStatus.Str;
			NewCharacterStatus.MaxAttackPoint = NewCharacterStatus.Str;
			NewCharacterStatus.SpiritPoint = NewCharacterStatus.Int * 2;
			NewCharacterStatus.AttackHitRate = 1000;
			NewCharacterStatus.SpellCastingRate = 1.0f;
			NewCharacterStatus.EvasionRate = 0;
			NewCharacterStatus.AttackCriticalPoint = NewCharacterStatus.Dex * 0.05f * 100;
			NewCharacterStatus.SpellCriticalPoint = NewCharacterStatus.Int * 0.01f * 100;	
			NewCharacterStatus.MaxSpeed = NewCharacterStatus.Speed;

			SP::CDBGameServerCreateCharacterPush NewCharacterPush(*UserNameCreateGameServerDBConnection);
			NewCharacterPush.InAccountID(Session->AccountId);
			NewCharacterPush.InPlayerName(Session->CreateCharacterName);			
			NewCharacterPush.InPlayerIndex(ReqCharacterCreateSlotIndex);
			NewCharacterPush.InLevel(NewCharacterStatus.Level);
			NewCharacterPush.InStr(NewCharacterStatus.Str);
			NewCharacterPush.InDex(NewCharacterStatus.Dex);
			NewCharacterPush.InInt(NewCharacterStatus.Int);
			NewCharacterPush.InLuck(NewCharacterStatus.Luck);
			NewCharacterPush.InCurrentHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InMaxHP(NewCharacterStatus.MaxHP);
			NewCharacterPush.InCurrentMP(NewCharacterStatus.MaxMP);
			NewCharacterPush.InMaxMP(NewCharacterStatus.MaxMP);
			NewCharacterPush.InAutoRecoveryHPPercent(NewCharacterStatus.AutoRecoveryHPPercent);
			NewCharacterPush.InAutoRecoveryMPPercent(NewCharacterStatus.AutoRecoveryMPPercent);
			NewCharacterPush.InMinAttackPoint(NewCharacterStatus.MinAttackPoint);
			NewCharacterPush.InMaxAttackPoint(NewCharacterStatus.MaxAttackPoint);
			NewCharacterPush.InSpiritPoint(NewCharacterStatus.SpiritPoint);
			NewCharacterPush.InAttackHitRate(NewCharacterStatus.AttackHitRate);			
			NewCharacterPush.InSpellCastingRate(NewCharacterStatus.SpellCastingRate);
			NewCharacterPush.InDefence(NewCharacterStatus.Defence);
			NewCharacterPush.InEvasionRate(NewCharacterStatus.EvasionRate);
			NewCharacterPush.InAttackCriticalPoint(NewCharacterStatus.AttackCriticalPoint);
			NewCharacterPush.InSpellCriticalPoint(NewCharacterStatus.SpellCriticalPoint);
			NewCharacterPush.InAttackCriticalResistance(NewCharacterStatus.AttackCriticalResistance);
			NewCharacterPush.InSpellCriticalResistance(NewCharacterStatus.AttackCriticalResistance);
			NewCharacterPush.InSpeed(NewCharacterStatus.Speed);
			NewCharacterPush.InLastPositionY(NewCharacterPositionY);
			NewCharacterPush.InLastPositionX(NewCharacterPositionX);
			NewCharacterPush.InCurrentExperence(CurrentExperience);
			NewCharacterPush.InRequireExperience(LevelData.RequireExperience);
			NewCharacterPush.InTotalExperience(LevelData.TotalExperience);			
			NewCharacterPush.InSkillMaxPoint(NewCharacterSkillPoint);

			// DB 요청 실행
			bool SaveNewCharacterQuery = NewCharacterPush.Execute();
			if (SaveNewCharacterQuery == true)
			{				
				// GameServerDB에 생성한 캐릭터의 PlayerDBId를 읽어오는 프로시저 클래스
				SP::CDBGameServerPlayerDBIDGet PlayerDBIDGet(*UserNameCreateGameServerDBConnection);
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
					NewPlayerCharacter->_GameObjectInfo.OwnerObjectType = en_GameObjectType::OBJECT_NON_TYPE;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Level = NewCharacterStatus.Level;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Str = NewCharacterStatus.Str;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Dex = NewCharacterStatus.Dex;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Int = NewCharacterStatus.Int;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Luck = NewCharacterStatus.Luck;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MinAttackPoint = NewCharacterStatus.MinAttackPoint;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxAttackPoint = NewCharacterStatus.MaxAttackPoint;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.SpiritPoint = NewCharacterStatus.SpiritPoint;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.AttackHitRate = NewCharacterStatus.AttackHitRate;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.SpellCastingRate = NewCharacterStatus.SpellCastingRate;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.AttackCriticalPoint = NewCharacterStatus.AttackCriticalPoint;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.SpellCriticalPoint = NewCharacterStatus.SpellCriticalPoint;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxSpeed = NewCharacterStatus.Speed;
					NewPlayerCharacter->_GameObjectInfo.ObjectSkillPoint = NewCharacterSkillPoint;
					NewPlayerCharacter->_GameObjectInfo.ObjectSkillMaxPoint = NewCharacterSkillPoint;
					NewPlayerCharacter->_SpawnPosition.Y = NewCharacterPositionY;
					NewPlayerCharacter->_SpawnPosition.X = NewCharacterPositionX;
					NewPlayerCharacter->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;					
					NewPlayerCharacter->_GameObjectInfo.OwnerObjectId = 0;
					NewPlayerCharacter->_GameObjectInfo.PlayerSlotIndex = ReqCharacterCreateSlotIndex;
					NewPlayerCharacter->_SessionId = Session->SessionId;
					NewPlayerCharacter->_AccountId = Session->AccountId;
					NewPlayerCharacter->_Experience.CurrentExperience = 0;
					NewPlayerCharacter->_Experience.RequireExperience = LevelData.RequireExperience;
					NewPlayerCharacter->_Experience.TotalExperience = LevelData.TotalExperience;

					NewPlayerCharacter->_SkillBox.CreateChracteristic(ReqSkillCharacteristic);

					G_ObjectManager->_PlayersArray[Session->MyPlayerIndexes[ReqCharacterCreateSlotIndex]] = NewPlayerCharacter;

					// DB에 새로운 인벤토리 생성
					for (int16 Y = 0; Y < (int8)en_InventoryManager::INVENTORY_DEFAULT_HEIGHT_SIZE; Y++)
					{
						for (int16 X = 0; X < (int8)en_InventoryManager::INVENTORY_DEFAULT_WIDH_SIZE; X++)
						{							
							SP::CDBGameServerItemCreateToInventory ItemToInventory(*UserNameCreateGameServerDBConnection);

							ItemToInventory.InItemTileGridPositionX(X);
							ItemToInventory.InItemTileGridPositionY(Y);							
							ItemToInventory.InOwnerAccountId(Session->AccountId);
							ItemToInventory.InOwnerPlayerId(PlayerDBId);

							ItemToInventory.Execute();							
						}
					}

					// 골드 테이블 새로 생성					
					SP::CDBGameServerGoldTableCreatePush GoldTableCreate(*UserNameCreateGameServerDBConnection);

					GoldTableCreate.InAccountDBId(Session->AccountId);
					GoldTableCreate.InPlayerDBId(PlayerDBId);

					GoldTableCreate.Execute();					

					// 플레이어 기본 정보 설정
					PlayerDefaultSetting(Session->AccountId, NewPlayerCharacter->_GameObjectInfo, ReqCharacterCreateSlotIndex);								

					// 캐릭터 생성 응답 보냄
					CMessage* ResCreateCharacterMessage = MakePacketResCreateCharacter(!CharacterNameFind, NewPlayerCharacter->_GameObjectInfo);
					SendPacket(Session->SessionId, ResCreateCharacterMessage);
					ResCreateCharacterMessage->Free();
				}				
			}
			else
			{

			}			
		}
		else
		{
			// 캐릭터가 이미 DB에 생성되어 있는 경우
		}
#pragma endregion

		G_DBConnectionPool->Push(en_DBConnect::GAME, UserNameCreateGameServerDBConnection);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBCharacterInfoSend(CMessage* Message)
{
	int64 SessionID;
	*Message >> SessionID;

	st_Session* Session = FindSession(SessionID);

	if (Session)
	{
		do
		{
			CGameServerMessage* ResCharacterInfoMessage = CGameServerMessage::GameServerMessageAlloc();
			ResCharacterInfoMessage->Clear();

			*ResCharacterInfoMessage << (int16)en_PACKET_S2C_CHARACTER_INFO;
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			*ResCharacterInfoMessage << MyPlayer->_AccountId;
			*ResCharacterInfoMessage << MyPlayer->_GameObjectInfo.ObjectId;
#pragma region 캐릭터 경험치 정보 담기
			*ResCharacterInfoMessage << MyPlayer->_Experience.CurrentExperience;
			*ResCharacterInfoMessage << MyPlayer->_Experience.RequireExperience;
			*ResCharacterInfoMessage << MyPlayer->_Experience.TotalExperience;
#pragma endregion

			CDBConnection* DBCharacterInfoGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

#pragma region 스킬 정보 읽어오기				
			MyPlayer->_SkillBox.Init();

			// 공용 스킬 만들기
			CSkillCharacteristic* PublicCharacteristic = MyPlayer->_SkillBox.GetSkillCharacteristicPublic();
			PublicCharacteristic->SkillCharacteristicInit(en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC);
			
			// 캐릭터가 선택한 특성 정보를 DB로부터 가져온다.
			// 선택한 특성에 따라 특성을 생성한다.
			SP::CDBGameServerSkillCharacteristicGet CharacterSkillCharacteristicGet(*DBCharacterInfoGetConnection);
			CharacterSkillCharacteristicGet.InAccountDBId(MyPlayer->_AccountId);
			CharacterSkillCharacteristicGet.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);
			
			int8 CharacteristicType = 0;
			
			CharacterSkillCharacteristicGet.OutSKillCharacteristicType(CharacteristicType);

			CharacterSkillCharacteristicGet.Execute();

			while (CharacterSkillCharacteristicGet.Fetch())
			{
				// 특성 생성
				if (CharacteristicType != (int8)en_SkillCharacteristic::SKILL_CATEGORY_NONE)
				{
					MyPlayer->_SkillBox.CreateChracteristic(CharacteristicType);

					CSkillCharacteristic* Characteristic = MyPlayer->_SkillBox.FindCharacteristic(CharacteristicType);
					if (Characteristic != nullptr)
					{
						// 스킬 특성 테이블에서 찾은 특성의 스킬이 있는지 확인
						SP::CDBGameServerSkillGet SkillGet(*DBCharacterInfoGetConnection);
						SkillGet.InAccountDBId(MyPlayer->_AccountId);
						SkillGet.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);
						SkillGet.InCharacteristicType(CharacteristicType);

						bool IsSkillLearn = false;
						int16 SkillType = 0;
						int8 SkillLevel = 0;

						SkillGet.OutSkillLearn(IsSkillLearn);
						SkillGet.OutSkillType(SkillType);
						SkillGet.OutSkillLevel(SkillLevel);

						SkillGet.Execute();

						while (SkillGet.Fetch())
						{
							// 캐릭터 특성에서 스킬 활성화
							if (IsSkillLearn == true)
							{													
								Characteristic->SkillCharacteristicActive(IsSkillLearn,
									(en_SkillType)SkillType, SkillLevel);
								MyPlayer->_GameObjectInfo.ObjectSkillPoint--;								
							}			
						}
					}
				}				
			}			

			*ResCharacterInfoMessage << MyPlayer->_GameObjectInfo.ObjectSkillPoint;			

			CSkillCharacteristic* SkillCharacteristicPublic = MyPlayer->_SkillBox.GetSkillCharacteristicPublic();			

			// Public 패시브 스킬 가져오기
			*ResCharacterInfoMessage << (int8)SkillCharacteristicPublic->GetPassiveSkill().size();
			for (CSkill* PassiveSkill : SkillCharacteristicPublic->GetPassiveSkill())
			{
				*ResCharacterInfoMessage << *PassiveSkill->GetSkillInfo();
			}

			// Public 액티브 스킬 가져오기
			*ResCharacterInfoMessage << (int8)SkillCharacteristicPublic->GetActiveSkill().size();
			for (CSkill* ActiveSkill : SkillCharacteristicPublic->GetActiveSkill())
			{
				*ResCharacterInfoMessage << *ActiveSkill->GetSkillInfo();
			}

			// Public을 제외한 특성 스킬 가져오기
			CSkillCharacteristic* SkillCharacteristics = MyPlayer->_SkillBox.GetSkillCharacteristics();			

			*ResCharacterInfoMessage << (int8)SkillCharacteristics->_SkillCharacteristic;

			if (SkillCharacteristics->_SkillCharacteristic != en_SkillCharacteristic::SKILL_CATEGORY_NONE)
			{
				*ResCharacterInfoMessage << (int8)SkillCharacteristics->GetPassiveSkill().size();
				for (CSkill* PassiveSkill : SkillCharacteristics->GetPassiveSkill())
				{
					*ResCharacterInfoMessage << *PassiveSkill->GetSkillInfo();
				}

				*ResCharacterInfoMessage << (int8)SkillCharacteristics->GetActiveSkill().size();
				for (CSkill* ActiveSkill : SkillCharacteristics->GetActiveSkill())
				{
					*ResCharacterInfoMessage << *ActiveSkill->GetSkillInfo();
				}
			}
#pragma endregion

#pragma region 가방 아이템 정보 읽어오기			
			// 인벤토리 생성
			MyPlayer->GetInventoryManager()->InventoryCreate(1, (int8)en_InventoryManager::INVENTORY_DEFAULT_WIDH_SIZE, (int8)en_InventoryManager::INVENTORY_DEFAULT_HEIGHT_SIZE);

			*ResCharacterInfoMessage << (int8)en_InventoryManager::INVENTORY_DEFAULT_WIDH_SIZE;
			*ResCharacterInfoMessage << (int8)en_InventoryManager::INVENTORY_DEFAULT_HEIGHT_SIZE;

			vector<CItem*> InventoryItems;

			// DB에 기록되어 있는 인벤토리 아이템들의 정보를 모두 긁어온다.			
			SP::CDBGameServerInventoryItemGet CharacterInventoryItem(*DBCharacterInfoGetConnection);
			CharacterInventoryItem.InAccountDBId(MyPlayer->_AccountId);
			CharacterInventoryItem.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);
						
			bool ItemEquipped = false;			
			int16 ItemTilePositionX = 0;
			int16 ItemTilePositionY = 0;			
			int16 ItemSmallCategory = 0;			
			int16 ItemCount = 0;	
			bool ItemIsSearching = false;
			int32 ItemDurability = 0;
			int8 ItemEnchantPoint = 0;
			
			CharacterInventoryItem.OutItemIsEquipped(ItemEquipped);			
			CharacterInventoryItem.OutItemTileGridPositionX(ItemTilePositionX);
			CharacterInventoryItem.OutItemTileGridPositionY(ItemTilePositionY);			
			CharacterInventoryItem.OutItemSmallCategory(ItemSmallCategory);			
			CharacterInventoryItem.OutItemCount(ItemCount);
			CharacterInventoryItem.OutItemIsSearching(ItemIsSearching);
			CharacterInventoryItem.OutItemDurability(ItemDurability);
			CharacterInventoryItem.OutItemEnchantPoint(ItemEnchantPoint);		

			CharacterInventoryItem.Execute();

			while (CharacterInventoryItem.Fetch())
			{
				// DB에서 읽어온 데이터로 아이템 생성
				CItem* NewItem = G_ObjectManager->ItemCreate((en_SmallItemCategory)ItemSmallCategory);		

				if (NewItem != nullptr)
				{					
					NewItem->_ItemInfo.ItemIsEquipped = ItemEquipped;					
					NewItem->_ItemInfo.ItemTileGridPositionX = ItemTilePositionX;
					NewItem->_ItemInfo.ItemTileGridPositionY = ItemTilePositionY;
					NewItem->_ItemInfo.ItemCount = ItemCount;
					NewItem->_ItemInfo.ItemIsSearching = ItemIsSearching;
					NewItem->_ItemInfo.ItemCurrentDurability = ItemDurability;
					NewItem->_ItemInfo.ItemEnchantPoint = ItemEnchantPoint;					

					MyPlayer->GetInventoryManager()->DBItemInsertItem(0, NewItem);
					InventoryItems.push_back(NewItem);
				}
			}

			// 클라 인벤토리 정보 담기			
			*ResCharacterInfoMessage << (int8)InventoryItems.size();

			for (CItem* Item : InventoryItems)
			{
				*ResCharacterInfoMessage << Item;
			}
#pragma endregion		

#pragma region 골드 정보 읽어오기
			// 캐릭터가 소유하고 있었던 골드 정보를 GoldTable에서 읽어온다.			
			SP::CDBGameServerGoldGet CharacterGoldGet(*DBCharacterInfoGetConnection);
			CharacterGoldGet.InAccountDBId(MyPlayer->_AccountId);
			CharacterGoldGet.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);

			int64 Coin = 0;			

			CharacterGoldGet.OutCoin(Coin);			

			if (CharacterGoldGet.Execute() && CharacterGoldGet.Fetch())
			{
				// DB에서 읽어온 Gold를 Inventory에 저장한다.
				MyPlayer->GetInventoryManager()->DBMoneyInsert(Coin);

				// 골드 정보 담기
				*ResCharacterInfoMessage << Coin;				
			}
#pragma endregion	

#pragma region 퀵슬롯 정보 가져와서 클라에 보내기
			// 퀵슬롯 정보 초기화
			MyPlayer->_QuickSlotManager.Init();

			vector<st_QuickSlotBarSlotInfo> QuickSlotBarSlotInfos;

			// 퀵슬롯 테이블 접근해서 해당 스킬이 등록되어 있는 모든 퀵슬롯 번호 가지고옴			
			SP::CDBGameServerQuickSlotBarGet QuickSlotBarGet(*DBCharacterInfoGetConnection);
			QuickSlotBarGet.InAccountDBId(MyPlayer->_AccountId);
			QuickSlotBarGet.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);

			int8 QuickSlotBarIndex;
			int8 QuickSlotBarSlotIndex;						
			int8 QuickSlotCharacteristicType;
			int16 QuickSlotSkillType;
			int8 QuickSlotSkillLevel;			
			int16 QuickSlotItemSmallCategory;
			int16 QuickSlotItemCount;

			QuickSlotBarGet.OutQuickSlotBarIndex(QuickSlotBarIndex);
			QuickSlotBarGet.OutQuickSlotBarItemIndex(QuickSlotBarSlotIndex);
			QuickSlotBarGet.OutQuickSlotCharacteristicType(QuickSlotCharacteristicType);
			QuickSlotBarGet.OutQuickSlotSkillType(QuickSlotSkillType);
			QuickSlotBarGet.OutQuickSlotSkillLevel(QuickSlotSkillLevel);			
			QuickSlotBarGet.OutQuickSlotItemSmallCategory(QuickSlotItemSmallCategory);
			QuickSlotBarGet.OutQuickSlotItemCount(QuickSlotItemCount);

			QuickSlotBarGet.Execute();

			while (QuickSlotBarGet.Fetch())
			{
				st_QuickSlotBarSlotInfo NewQuickSlotBarSlot;
				NewQuickSlotBarSlot.QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_NONE;
				NewQuickSlotBarSlot.AccountDBId = MyPlayer->_AccountId;
				NewQuickSlotBarSlot.PlayerDBId = MyPlayer->_GameObjectInfo.ObjectId;
				NewQuickSlotBarSlot.QuickSlotBarIndex = QuickSlotBarIndex;
				NewQuickSlotBarSlot.QuickSlotBarSlotIndex = QuickSlotBarSlotIndex;

				CSkill* FindSkill = MyPlayer->_SkillBox.FindSkill((en_SkillCharacteristic)QuickSlotCharacteristicType,(en_SkillType)QuickSlotSkillType);
				if (FindSkill != nullptr)
				{
					st_QuickSlotBarPosition SkillQuickslotPosition;
					SkillQuickslotPosition.QuickSlotBarIndex = QuickSlotBarIndex;
					SkillQuickslotPosition.QuickSlotBarSlotIndex = QuickSlotBarSlotIndex;

					NewQuickSlotBarSlot.QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_SKILL;
					NewQuickSlotBarSlot.QuickBarSkill = FindSkill;

					FindSkill->_QuickSlotBarPosition.push_back(SkillQuickslotPosition);
				}
				else
				{
					NewQuickSlotBarSlot.QuickBarSkill = nullptr;
				}

				CItem* FindItem = MyPlayer->GetInventoryManager()->FindInventoryItem(0, (en_SmallItemCategory)QuickSlotItemSmallCategory);
				if (FindItem != nullptr)
				{
					NewQuickSlotBarSlot.QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_ITEM;
					NewQuickSlotBarSlot.QuickBarItem = FindItem;
				}
				else
				{
					NewQuickSlotBarSlot.QuickBarItem = nullptr;
				}

				// 퀵슬롯에 등록한다.
				MyPlayer->_QuickSlotManager.UpdateQuickSlotBar(NewQuickSlotBarSlot);
				QuickSlotBarSlotInfos.push_back(NewQuickSlotBarSlot);
			}			

			// 캐릭터 퀵슬롯 정보 담기
			*ResCharacterInfoMessage << (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SIZE;
			*ResCharacterInfoMessage << (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SLOT_SIZE;
			*ResCharacterInfoMessage << (int8)QuickSlotBarSlotInfos.size();

			for (st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo : QuickSlotBarSlotInfos)
			{
				*ResCharacterInfoMessage << QuickSlotBarSlotInfo;
			}

			SP::CDBGameSerGetQuickSlotKey QuickSlotKeyGet(*DBCharacterInfoGetConnection);
			QuickSlotKeyGet.InAccountDBId(MyPlayer->_AccountId);
			QuickSlotKeyGet.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);

			int16 UserQuickSlotKey;
			int16 QuickSlotKeyCode;

			QuickSlotKeyGet.OutQuickSlotKey(UserQuickSlotKey);
			QuickSlotKeyGet.OutQuickSlotKeyCode(QuickSlotKeyCode);

			QuickSlotKeyGet.Execute();

			*ResCharacterInfoMessage << (int16)en_UserQuickSlot::USER_KEY_QUICK_SLOT_INTERACTION;

			vector<st_BindingKey> QuickSlotBindingKeys;

			while (QuickSlotKeyGet.Fetch())
			{
				*ResCharacterInfoMessage << UserQuickSlotKey;
				*ResCharacterInfoMessage << QuickSlotKeyCode;

				st_BindingKey QuickSlotBindingKey;
				QuickSlotBindingKey.UserQuickSlot = (en_UserQuickSlot)UserQuickSlotKey;
				QuickSlotBindingKey.KeyCode = (en_KeyCode)QuickSlotKeyCode;

				QuickSlotBindingKeys.push_back(QuickSlotBindingKey);
			}

			MyPlayer->_QuickSlotKey.QuickSlotKeyInit(QuickSlotBindingKeys);

#pragma endregion

#pragma region 장비 아이템 정보 읽어오기
			vector<st_ItemInfo> Equipments;
			SP::CDBGameServerGetEquipment EquipmentGet(*DBCharacterInfoGetConnection);
			EquipmentGet.InAccountDBID(MyPlayer->_AccountId);
			EquipmentGet.InPlayerDBID(MyPlayer->_GameObjectInfo.ObjectId);

			int8 EquipmentParts;			
			int16 EquipmentSmallCategory = 0;
			int32 EquipmentDurability = 0;
			int8 EquipmentEnchantPoint = 0;

			EquipmentGet.OutEquipmentParts(EquipmentParts);			
			EquipmentGet.OutEquipmentSmallCategory(EquipmentSmallCategory);
			EquipmentGet.OutEquipmentDurability(EquipmentDurability);
			EquipmentGet.OutEquipmentEnchantPoint(EquipmentEnchantPoint);

			EquipmentGet.Execute();

			while (EquipmentGet.Fetch())
			{
				if ((en_SmallItemCategory)EquipmentSmallCategory != en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE)
				{
					CItem* NewEquipmentItem = G_ObjectManager->ItemCreate((en_SmallItemCategory)EquipmentSmallCategory);
					NewEquipmentItem->_ItemInfo.ItemCurrentDurability = EquipmentDurability;
					NewEquipmentItem->_ItemInfo.ItemEnchantPoint = EquipmentEnchantPoint;					

					MyPlayer->GetEquipment()->ItemOnEquipment(NewEquipmentItem);

					Equipments.push_back(NewEquipmentItem->_ItemInfo);
				}				
			}						

			// 장비 정보 담기
			*ResCharacterInfoMessage << (int8)Equipments.size();
			for (st_ItemInfo EquipmentItemInfo : Equipments)
			{
				*ResCharacterInfoMessage << EquipmentItemInfo;
			}
#pragma endregion

//#pragma region 조합템 정보 보내기			
//			vector<st_CraftingItemCategory> CraftingItemCategorys;
//
//			for (int8 Category = (int8)en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARCHITECTURE;
//				Category <= (int8)en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL; ++Category)
//			{
//				auto FindCraftingIterator = G_Datamanager->_CraftingData.find(Category);
//				if (FindCraftingIterator == G_Datamanager->_CraftingData.end())
//				{
//					continue;
//				}
//
//				st_CraftingItemCategory* CraftingCategory = (*FindCraftingIterator).second;				
//				CraftingItemCategorys.push_back(*CraftingCategory);
//			}
//
//			*ResCharacterInfoMessage << (int8)CraftingItemCategorys.size();
//
//			for (st_CraftingItemCategory CraftingItemCategory : CraftingItemCategorys)
//			{
//				*ResCharacterInfoMessage << CraftingItemCategory;				
//			}						
//#pragma endregion
//
//			G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterInfoGetConnection);
//
//#pragma region 제작대 조합템 정보 보내기
//			vector<st_CraftingTableRecipe*> CraftingTables;
//
//			for (int16 CraftingTableType = (int16)en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE;
//				CraftingTableType <= (int16)en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL; CraftingTableType++)
//			{
//				auto FindFurnaceCraftingTable = G_Datamanager->_CraftingTableData.find(CraftingTableType);
//				st_CraftingTableRecipe* FurnaceCraftingTable = (*FindFurnaceCraftingTable).second;
//
//				CraftingTables.push_back(FurnaceCraftingTable);
//			}			
//
//			*ResCharacterInfoMessage << (int8)CraftingTables.size();
//
//			for (st_CraftingTableRecipe* CraftingTable : CraftingTables)
//			{
//				*ResCharacterInfoMessage << *CraftingTable;
//			}
//#pragma endregion
						
#pragma region Day 시간 보내기			
			st_Day DayInfo = _Day->GetDayInfo();
			*ResCharacterInfoMessage << DayInfo.DayTimeCycle;
			*ResCharacterInfoMessage << DayInfo.DayTimeCheck;
			*ResCharacterInfoMessage << DayInfo.DayRatio;
			*ResCharacterInfoMessage << (int8)DayInfo.DayType;
#pragma endregion					

			SendPacket(MyPlayer->_SessionId, ResCharacterInfoMessage);			

			MyPlayer->_NetworkState = en_ObjectNetworkState::OBJECT_NETWORK_STATE_LIVE;	

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterInfoGetConnection);

		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBLeavePlayerInfoSave(CGameServerMessage* Message)
{
	st_Session* LeaveSession;
	*Message >> &LeaveSession;
		
	CPlayer* LeavePlayer = G_ObjectManager->_PlayersArray[LeaveSession->MyPlayerIndex];	
	
	if (LeavePlayer->_AccountId == 0)
	{
		return;
	}

	// 캐릭터 정보 DB에 저장
	CDBConnection* PlayerInfoSaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
	SP::CDBGameServerLeavePlayerStatInfoSave LeavePlayerStatInfoSave(*PlayerInfoSaveDBConnection);

	LeavePlayerStatInfoSave.InAccountDBId(LeavePlayer->_AccountId);
	LeavePlayerStatInfoSave.InPlayerDBId(LeavePlayer->_GameObjectInfo.ObjectId);
	LeavePlayerStatInfoSave.InLevel(LeavePlayer->_GameObjectInfo.ObjectStatInfo.Level);
	LeavePlayerStatInfoSave.InStr(LeavePlayer->_GameObjectInfo.ObjectStatInfo.Str);
	LeavePlayerStatInfoSave.InDex(LeavePlayer->_GameObjectInfo.ObjectStatInfo.Dex);
	LeavePlayerStatInfoSave.InInt(LeavePlayer->_GameObjectInfo.ObjectStatInfo.Int);
	LeavePlayerStatInfoSave.InLuck(LeavePlayer->_GameObjectInfo.ObjectStatInfo.Luck);
	LeavePlayerStatInfoSave.InMaxHP(LeavePlayer->_GameObjectInfo.ObjectStatInfo.MaxHP);
	LeavePlayerStatInfoSave.InMaxMP(LeavePlayer->_GameObjectInfo.ObjectStatInfo.MaxMP);
	LeavePlayerStatInfoSave.InAutoRecoveryHPPercent(LeavePlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent);
	LeavePlayerStatInfoSave.InAutoRecoveryMPPercent(LeavePlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent);
	LeavePlayerStatInfoSave.InMinAttackPoint(LeavePlayer->_GameObjectInfo.ObjectStatInfo.MinAttackPoint);
	LeavePlayerStatInfoSave.InMaxAttackPoint(LeavePlayer->_GameObjectInfo.ObjectStatInfo.MaxAttackPoint);
	LeavePlayerStatInfoSave.InSpiritPoint(LeavePlayer->_GameObjectInfo.ObjectStatInfo.SpiritPoint);
	LeavePlayerStatInfoSave.InAttackHitRate(LeavePlayer->_GameObjectInfo.ObjectStatInfo.AttackHitRate);
	LeavePlayerStatInfoSave.InSpellCastingRate(LeavePlayer->_GameObjectInfo.ObjectStatInfo.SpellCastingRate);
	LeavePlayerStatInfoSave.InDefence(LeavePlayer->_GameObjectInfo.ObjectStatInfo.Defence);
	LeavePlayerStatInfoSave.InEvasionRate(LeavePlayer->_GameObjectInfo.ObjectStatInfo.EvasionRate);
	LeavePlayerStatInfoSave.InAttackCriticalPoint(LeavePlayer->_GameObjectInfo.ObjectStatInfo.AttackCriticalPoint);
	LeavePlayerStatInfoSave.InSpellCriticalPoint(LeavePlayer->_GameObjectInfo.ObjectStatInfo.SpellCriticalPoint);
	LeavePlayerStatInfoSave.InAttackCriticalResistance(LeavePlayer->_GameObjectInfo.ObjectStatInfo.AttackCriticalResistance);
	LeavePlayerStatInfoSave.InStatusAbnormalResistance(LeavePlayer->_GameObjectInfo.ObjectStatInfo.StatusAbnormalResistance);
	LeavePlayerStatInfoSave.InSpeed(LeavePlayer->_GameObjectInfo.ObjectStatInfo.Speed);
	LeavePlayerStatInfoSave.InLastPositionY(LeavePlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y);
	LeavePlayerStatInfoSave.InLastPositionX(LeavePlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X);
	LeavePlayerStatInfoSave.InCurrentExperience(LeavePlayer->_Experience.CurrentExperience);
	LeavePlayerStatInfoSave.InRequireExperience(LeavePlayer->_Experience.RequireExperience);
	LeavePlayerStatInfoSave.InTotalExperience(LeavePlayer->_Experience.TotalExperience);	
	LeavePlayerStatInfoSave.InSkillMaxPoint(LeavePlayer->_GameObjectInfo.ObjectSkillMaxPoint);

	LeavePlayerStatInfoSave.Execute();		

	// 스킬 특성 정보 DB에 저장
	SP::CDBGameServerSkillCharacteristicUpdate SkillCharacteristicUpdate(*PlayerInfoSaveDBConnection);
	// 스킬 정보 DB에 저장
	SP::CDBGameServerSkillToSkillBox SkillIntoSkillBox(*PlayerInfoSaveDBConnection);

	SkillCharacteristicUpdate.InAccountDBId(LeavePlayer->_AccountId);
	SkillCharacteristicUpdate.InPlayerDBId(LeavePlayer->_GameObjectInfo.ObjectId);

	SkillIntoSkillBox.InAccountDBId(LeavePlayer->_AccountId);
	SkillIntoSkillBox.InPlayerDBId(LeavePlayer->_GameObjectInfo.ObjectId);

	CSkillCharacteristic* SkillCharacteristic = LeavePlayer->_SkillBox.GetSkillCharacteristics();
	int8 SkillCharacterType = (int8)SkillCharacteristic->_SkillCharacteristic;

	SkillCharacteristicUpdate.InSkillCharacteristicType(SkillCharacterType);
	SkillCharacteristicUpdate.Execute();

	SkillIntoSkillBox.InCharacteristicType(SkillCharacterType);

	vector<CSkill*> ActiveSkills = SkillCharacteristic->GetActiveSkill();
	for (CSkill* ActiveSkill : ActiveSkills)
	{
		int16 ActiveSkillType = (int16)ActiveSkill->GetSkillInfo()->SkillType;
		int8 ActiveSkillLevel = ActiveSkill->GetSkillInfo()->SkillLevel;

		SkillIntoSkillBox.InSkillLearn(ActiveSkill->GetSkillInfo()->IsSkillLearn);
		SkillIntoSkillBox.InSkillType(ActiveSkillType);
		SkillIntoSkillBox.InSkillLevel(ActiveSkillLevel);

		SkillIntoSkillBox.Execute();
	}	

	// 퀵슬롯 정보 업데이트
	for (auto QuickSlotIterator : LeavePlayer->_QuickSlotManager.GetQuickSlotBar())
	{
		for (auto QuickSlotBarSlotIterator : QuickSlotIterator.second->GetQuickSlotBarInfo())
		{
			st_QuickSlotBarSlotInfo* SaveQuickSlotInfo = QuickSlotBarSlotIterator.second;

			switch (SaveQuickSlotInfo->QuickSlotBarType)
			{
			case en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_NONE:
				{
					SP::CDBGameServerQuickSlotInit QuickSlotInit(*PlayerInfoSaveDBConnection);

					QuickSlotInit.InAccountDBId(LeavePlayer->_AccountId);
					QuickSlotInit.InPlayerDBId(LeavePlayer->_GameObjectInfo.ObjectId);
					QuickSlotInit.InQuickSlotBarIndex(SaveQuickSlotInfo->QuickSlotBarIndex);
					QuickSlotInit.InQuickSlotBarSlotIndex(SaveQuickSlotInfo->QuickSlotBarSlotIndex);

					QuickSlotInit.Execute();
				}
				break;
			case en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_SKILL:
				{
					SP::CDBGameServerQuickSlotBarSlotUpdate QuickSlotDBUpdate(*PlayerInfoSaveDBConnection);
										
					int8 SaveQuickSlotInfoSkillChracteristicType = (int8)SaveQuickSlotInfo->QuickBarSkill->GetSkillInfo()->SkillCharacteristic;
					int16 SaveQuickSlotInfoSkillSkillType = (int16)SaveQuickSlotInfo->QuickBarSkill->GetSkillInfo()->SkillType;
										
					int16 SaveItemSmallCategory = 0;
					int16 SaveItemCount = 0;

					QuickSlotDBUpdate.InAccountDBId(LeavePlayer->_AccountId);
					QuickSlotDBUpdate.InPlayerDBId(LeavePlayer->_GameObjectInfo.ObjectId);
					QuickSlotDBUpdate.InQuickSlotBarIndex(SaveQuickSlotInfo->QuickSlotBarIndex);
					QuickSlotDBUpdate.InQuickSlotBarSlotIndex(SaveQuickSlotInfo->QuickSlotBarSlotIndex);
					QuickSlotDBUpdate.InCharacteristicType(SaveQuickSlotInfoSkillChracteristicType);
					QuickSlotDBUpdate.InSkillType(SaveQuickSlotInfoSkillSkillType);
					QuickSlotDBUpdate.InSkillLevel(SaveQuickSlotInfo->QuickBarSkill->GetSkillInfo()->SkillLevel);					
					QuickSlotDBUpdate.InItemSmallCategory(SaveItemSmallCategory);
					QuickSlotDBUpdate.InItemCount(SaveItemCount);

					QuickSlotDBUpdate.Execute();
				}
				break;
			case en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_ITEM:
				{
					SP::CDBGameServerQuickSlotBarSlotUpdate QuickSlotDBUpdate(*PlayerInfoSaveDBConnection);
					
					int16 SaveQuickSlotInfoSkillSkillType = 0;
					int8 SkillLevel = 0;
					
					int16 SaveItemSmallCategory = (int16)SaveQuickSlotInfo->QuickBarItem->_ItemInfo.ItemSmallCategory;
					int16 SaveItemCount = SaveQuickSlotInfo->QuickBarItem->_ItemInfo.ItemCount;

					QuickSlotDBUpdate.InAccountDBId(LeavePlayer->_AccountId);
					QuickSlotDBUpdate.InPlayerDBId(LeavePlayer->_GameObjectInfo.ObjectId);
					QuickSlotDBUpdate.InQuickSlotBarIndex(SaveQuickSlotInfo->QuickSlotBarIndex);
					QuickSlotDBUpdate.InQuickSlotBarSlotIndex(SaveQuickSlotInfo->QuickSlotBarSlotIndex);									
					QuickSlotDBUpdate.InSkillType(SaveQuickSlotInfoSkillSkillType);
					QuickSlotDBUpdate.InSkillLevel(SkillLevel);
					
					QuickSlotDBUpdate.InItemSmallCategory(SaveItemSmallCategory);
					QuickSlotDBUpdate.InItemCount(SaveItemCount);

					QuickSlotDBUpdate.Execute();
				}
				break;			
			}			
		}
	}

	// 장비 정보 DB에 저장
	map<en_EquipmentParts, CItem*> Equipments = LeavePlayer->GetEquipment()->GetEquipments();
	for (auto Equipment : Equipments)
	{
		if (Equipment.second != nullptr)
		{
			SP::CDBGameServerOnEquipment SaveEquipmentInfo(*PlayerInfoSaveDBConnection);

			int8 EquipmentParts = (int8)Equipment.second->_ItemInfo.ItemEquipmentPart;
			int16 EquipmentSmallCategory = (int16)Equipment.second->_ItemInfo.ItemSmallCategory;

			SaveEquipmentInfo.InAccountDBID(LeavePlayer->_AccountId);
			SaveEquipmentInfo.InPlayerDBID(LeavePlayer->_GameObjectInfo.ObjectId);
			SaveEquipmentInfo.InEquipmentParts(EquipmentParts);
			SaveEquipmentInfo.InEquipmentSmallCategory(EquipmentSmallCategory);
			SaveEquipmentInfo.InEquipmentDurability(Equipment.second->_ItemInfo.ItemCurrentDurability);
			SaveEquipmentInfo.InEquipmentEnchantPoint(Equipment.second->_ItemInfo.ItemEnchantPoint);

			SaveEquipmentInfo.Execute();
		}
		else
		{
			SP::CDBGameServerOffEquipment OffEquipment(*PlayerInfoSaveDBConnection);

			int8 EquipmentParts = (int8)Equipment.first;

			OffEquipment.InAccountDBID(LeavePlayer->_AccountId);
			OffEquipment.InPlayerDBID(LeavePlayer->_GameObjectInfo.ObjectId);
			OffEquipment.InEquipmentParts(EquipmentParts);

			OffEquipment.Execute();
		}
	}		

	// 가방 정보 DB에 저장	
	vector<CInventory*> LeavePlayerInventorys = LeavePlayer->GetInventoryManager()->GetInventoryManager();	

	// 가방 DB 청소 후 새로 저장
	for (int i = 0; i < LeavePlayer->GetInventoryManager()->GetInventoryCount(); i++)
	{
		SP::CDBGameServerInventoryAllSlotInit InventoryAllSlotInit(*PlayerInfoSaveDBConnection);
		InventoryAllSlotInit.InOwnerAccountID(LeavePlayer->_AccountId);
		InventoryAllSlotInit.InOwnerPlayerID(LeavePlayer->_GameObjectInfo.ObjectId);
		InventoryAllSlotInit.InInventoryWidth(LeavePlayerInventorys[i]->_InventoryWidth);
		InventoryAllSlotInit.InInventoryHeight(LeavePlayerInventorys[i]->_InventoryHeight);

		InventoryAllSlotInit.Execute();

		if (LeavePlayerInventorys[i] != nullptr)
		{
			SP::CDBGameServerInventoryPlace LeavePlayerInventoryItemSave(*PlayerInfoSaveDBConnection);
			LeavePlayerInventoryItemSave.InOwnerAccountId(LeavePlayer->_AccountId);
			LeavePlayerInventoryItemSave.InOwnerPlayerId(LeavePlayer->_GameObjectInfo.ObjectId);

			vector<st_ItemInfo> PlayerInventoryItems = LeavePlayerInventorys[i]->DBInventorySaveReturnItems();

			for (st_ItemInfo InventoryItem : PlayerInventoryItems)
			{					
				int16 InventoryItemSmallCategory = (int16)InventoryItem.ItemSmallCategory;

				LeavePlayerInventoryItemSave.InIsEquipped(InventoryItem.ItemIsEquipped);				
				LeavePlayerInventoryItemSave.InItemTileGridPositionX(InventoryItem.ItemTileGridPositionX);
				LeavePlayerInventoryItemSave.InItemTileGridPositionY(InventoryItem.ItemTileGridPositionY);				
				LeavePlayerInventoryItemSave.InItemSmallCategory(InventoryItemSmallCategory);				
				LeavePlayerInventoryItemSave.InItemCount(InventoryItem.ItemCount);	
				LeavePlayerInventoryItemSave.InItemIsSearching(InventoryItem.ItemIsSearching);
				LeavePlayerInventoryItemSave.InItemDurability(InventoryItem.ItemCurrentDurability);
				LeavePlayerInventoryItemSave.InItemEnchantPoint(InventoryItem.ItemEnchantPoint);

				LeavePlayerInventoryItemSave.Execute();
			}
		}
	}			

	G_DBConnectionPool->Push(en_DBConnect::GAME, PlayerInfoSaveDBConnection);			

	if (LeavePlayer->GetChannel() != nullptr)
	{
		CMap* LeaveMap = G_MapManager->GetMap(1);
		CChannel* LeaveChannel = LeaveMap->GetChannelManager()->Find(1);

		if (LeavePlayer->_PartyManager._IsParty == true)
		{
			st_GameObjectJob* PartyQuitJob = MakeGameObjectJobPartyQuit(LeavePlayer->_GameObjectInfo.ObjectId);
			LeaveChannel->_ChannelJobQue.Enqueue(PartyQuitJob);
		}		

		st_GameObjectJob* DeSpawnMonsterChannelJob = MakeGameObjectJobObjectDeSpawnObjectChannel(LeavePlayer);
		LeaveChannel->_ChannelJobQue.Enqueue(DeSpawnMonsterChannelJob);

		st_GameObjectJob* LeaveGameJob = MakeGameObjectJobLeaveChannelPlayer(LeavePlayer, LeaveSession->MyPlayerIndexes);
		LeaveChannel->_ChannelJobQue.Enqueue(LeaveGameJob);				
	}	
	
	// GameServer와 관련된 Session 정보 초기화
	memset(LeaveSession->Token, 0, sizeof(LeaveSession->Token));

	LeaveSession->IsLogin = false;
	LeaveSession->IsDummy = false;

	LeaveSession->MyPlayerIndex = -1;
	LeaveSession->AccountId = 0;

	LeaveSession->ClientSock = INVALID_SOCKET;
	closesocket(LeaveSession->CloseSock);

	InterlockedDecrement64(&_SessionCount);
	// 세션 인덱스 반납	
	_SessionArrayIndexs.Push(GET_SESSIONINDEX(LeaveSession->SessionId));	
}

void CGameServer::WorldMapTileInfoLoad()
{
	CDBConnection* DBCharacterInfoGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

	// 월드 타일 정보 가져오기
	SP::CDBGameServerGetTileInfos GetTileInfos(*DBCharacterInfoGetConnection);

	bool IsTileOccupation;
	int64 TileOwnerObjectID;
	int16 TilePositionY;
	int16 TilePositionX;

	GetTileInfos.OutIsTileOccupation(IsTileOccupation);
	GetTileInfos.OutIsTileOwnerObjectID(TileOwnerObjectID);
	GetTileInfos.OutTilePositionY(TilePositionY);
	GetTileInfos.OutTilePositionX(TilePositionX);

	GetTileInfos.Execute();

	vector<st_TileInfo> TileInfos;

	while (GetTileInfos.Fetch())
	{
		st_TileInfo TileInfo;
		TileInfo.IsOccupation = IsTileOccupation;
		TileInfo.OwnerObjectID = TileOwnerObjectID;
		TileInfo.Position.Y = TilePositionY;
		TileInfo.Position.X = TilePositionX;

		TileInfos.push_back(TileInfo);
	}	

	CMap* Map = G_MapManager->GetMap(1);
	if (Map != nullptr)
	{
		Map->SetTileInfos(TileInfos);
	}

	G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterInfoGetConnection);
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

st_GameObjectJob* CGameServer::MakeGameObjectJobLeaveChannelPlayer(CGameObject* LeavePlayerObject, int32* PlayerIndexes)
{
	st_GameObjectJob* LeaveChannelJob = G_ObjectManager->GameObjectJobCreate();
	LeaveChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PLAYER_LEAVE;

	CGameServerMessage* LeaveChannelMessage = CGameServerMessage::GameServerMessageAlloc();
	LeaveChannelMessage->Clear();

	*LeaveChannelMessage << &LeavePlayerObject;

	if (PlayerIndexes != nullptr)
	{
		for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
		{
			*LeaveChannelMessage << PlayerIndexes[i];
		}
	}

	LeaveChannelJob->GameObjectJobMessage = LeaveChannelMessage;

	return LeaveChannelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobMove(float DirectionX, float DirectionY, float PositionX, float PositionY, int8 GameObjectState)
{
	st_GameObjectJob* MoveJob = G_ObjectManager->GameObjectJobCreate();
	MoveJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_MOVE;

	CGameServerMessage* MoveMessage = CGameServerMessage::GameServerMessageAlloc();
	MoveMessage->Clear();

	*MoveMessage << DirectionX;
	*MoveMessage << DirectionY;
	*MoveMessage << PositionX;
	*MoveMessage << PositionY;
	*MoveMessage << GameObjectState;

	MoveJob->GameObjectJobMessage = MoveMessage;

	return MoveJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobMoveStop(float PositionX, float PositionY, int8 GameObjectState)
{
	st_GameObjectJob* MoveStopJob = G_ObjectManager->GameObjectJobCreate();
	MoveStopJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_MOVE_STOP;

	CGameServerMessage* MoveStopMessage = CGameServerMessage::GameServerMessageAlloc();
	MoveStopMessage->Clear();

	*MoveStopMessage << PositionX;
	*MoveStopMessage << PositionY;
	*MoveStopMessage << GameObjectState;

	MoveStopJob->GameObjectJobMessage = MoveStopMessage;

	return MoveStopJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobLookAtDirection(float DirectionX, float DirectionY)
{
	st_GameObjectJob* LookAtDirectionJob = G_ObjectManager->GameObjectJobCreate();
	LookAtDirectionJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_LOOK_AT_DIRECTION;

	CGameServerMessage* LookAtDirectionMessage = CGameServerMessage::GameServerMessageAlloc();
	LookAtDirectionMessage->Clear();

	*LookAtDirectionMessage << DirectionX;
	*LookAtDirectionMessage << DirectionY;	

	LookAtDirectionJob->GameObjectJobMessage = LookAtDirectionMessage;

	return LookAtDirectionJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobSelectSkillCharacteristic(int8 SelectChracteristicType)
{
	st_GameObjectJob* SelectSkillCharacteristicJob = G_ObjectManager->GameObjectJobCreate();
	SelectSkillCharacteristicJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SELECT_SKILL_CHARACTERISTIC;

	CGameServerMessage* SelectSkillCharacteristicJobMessage = CGameServerMessage::GameServerMessageAlloc();
	SelectSkillCharacteristicJobMessage->Clear();
	
	*SelectSkillCharacteristicJobMessage << SelectChracteristicType;

	SelectSkillCharacteristicJob->GameObjectJobMessage = SelectSkillCharacteristicJobMessage;

	return SelectSkillCharacteristicJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobSkillLearn(bool IsSkillLearn, int8 LearnSkillCharacteristicType, int16 LearnSkillType)
{
	st_GameObjectJob* SkillLearnJob = G_ObjectManager->GameObjectJobCreate();
	SkillLearnJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SKILL_LEARN;

	CGameServerMessage* SkillLearnJobMessage = CGameServerMessage::GameServerMessageAlloc();
	SkillLearnJobMessage->Clear();

	*SkillLearnJobMessage << IsSkillLearn;	
	*SkillLearnJobMessage << LearnSkillCharacteristicType;
	*SkillLearnJobMessage << LearnSkillType;

	SkillLearnJob->GameObjectJobMessage = SkillLearnJobMessage;

	return SkillLearnJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobSkillProcess(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, int8 SkillCharacteristicType, int16 SkillType, float SkillDirectionX, float SkillDirectionY)
{
	st_GameObjectJob* MeleeAttackJob = G_ObjectManager->GameObjectJobCreate();
	MeleeAttackJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SKILL_PROCESS;

	CGameServerMessage* MeleeAttackJobMessage = CGameServerMessage::GameServerMessageAlloc();
	MeleeAttackJobMessage->Clear();

	*MeleeAttackJobMessage << QuickSlotBarIndex;
	*MeleeAttackJobMessage << QuickSlotBarSlotIndex;
	*MeleeAttackJobMessage << SkillCharacteristicType;
	*MeleeAttackJobMessage << SkillType;
	*MeleeAttackJobMessage << SkillDirectionX;
	*MeleeAttackJobMessage << SkillDirectionY;

	MeleeAttackJob->GameObjectJobMessage = MeleeAttackJobMessage;

	return MeleeAttackJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobSkillCastingCancel()
{
	st_GameObjectJob* SkillCastingCancelJob = G_ObjectManager->GameObjectJobCreate();
	SkillCastingCancelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SKILL_CASTING_CANCEL;

	SkillCastingCancelJob->GameObjectJobMessage = nullptr;

	return SkillCastingCancelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobGatheringStart(CGameObject* GatheringObject)
{
	st_GameObjectJob* GatheringStartJob = G_ObjectManager->GameObjectJobCreate();
	GatheringStartJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_GATHERING_START;

	CGameServerMessage* GatheringMessage = CGameServerMessage::GameServerMessageAlloc();
	GatheringMessage->Clear();

	*GatheringMessage << &GatheringObject;

	GatheringStartJob->GameObjectJobMessage = GatheringMessage;

	return GatheringStartJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobGatheringCancel()
{
	st_GameObjectJob* SpellCancelJob = G_ObjectManager->GameObjectJobCreate();
	SpellCancelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_GATHERING_CANCEL;

	SpellCancelJob->GameObjectJobMessage = nullptr;

	return SpellCancelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobItemDrop(int16 DropItemType, int32 DropItemCount)
{
	st_GameObjectJob* ItemDropJob = G_ObjectManager->GameObjectJobCreate();
	ItemDropJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_ITEM_DROP;

	CGameServerMessage* ItemDropJobMessage = CGameServerMessage::GameServerMessageAlloc();
	ItemDropJobMessage->Clear();

	*ItemDropJobMessage << DropItemType;
	*ItemDropJobMessage << DropItemCount;

	ItemDropJob->GameObjectJobMessage = ItemDropJobMessage;

	return ItemDropJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobCraftingTableStart(CGameObject* CraftingStartObject, en_SmallItemCategory CraftingCompleteItemType, int16 CraftingCount)
{
	st_GameObjectJob* CraftingTableStartJob = G_ObjectManager->GameObjectJobCreate();
	CraftingTableStartJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_CRAFTING_START;

	CGameServerMessage* CraftingTableStartJobMessage = CGameServerMessage::GameServerMessageAlloc();
	CraftingTableStartJobMessage->Clear();

	*CraftingTableStartJobMessage << &CraftingStartObject;
	*CraftingTableStartJobMessage << (int16)CraftingCompleteItemType;
	*CraftingTableStartJobMessage << CraftingCount;

	CraftingTableStartJob->GameObjectJobMessage = CraftingTableStartJobMessage;

	return CraftingTableStartJob;	
}

st_GameObjectJob* CGameServer::MakeGameObjectJobCraftingTableItemAdd(CGameObject* CraftingTableItemAddObject, int16 AddItemSmallCategory, int16 AddItemCount)
{
	st_GameObjectJob* CraftingTableItemInputJob = G_ObjectManager->GameObjectJobCreate();
	CraftingTableItemInputJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_ITEM_ADD;

	CGameServerMessage* CraftingTableItemInputJobMessage = CGameServerMessage::GameServerMessageAlloc();
	CraftingTableItemInputJobMessage->Clear();

	*CraftingTableItemInputJobMessage << &CraftingTableItemAddObject;
	*CraftingTableItemInputJobMessage << AddItemSmallCategory;
	*CraftingTableItemInputJobMessage << AddItemCount;

	CraftingTableItemInputJob->GameObjectJobMessage = CraftingTableItemInputJobMessage;

	return CraftingTableItemInputJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobCraftingTableMaterialItemSubtract(CGameObject* CraftingTableItemSubtractObject, int16 SubtractItemSmallCategory, int16 SubtractItemCount)
{
	st_GameObjectJob* CraftingTableMaterialItemInputJob = G_ObjectManager->GameObjectJobCreate();
	CraftingTableMaterialItemInputJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_MATERIAL_ITEM_SUBTRACT;

	CGameServerMessage* CraftingTableItemInputJobMessage = CGameServerMessage::GameServerMessageAlloc();
	CraftingTableItemInputJobMessage->Clear();

	*CraftingTableItemInputJobMessage << &CraftingTableItemSubtractObject;
	*CraftingTableItemInputJobMessage << SubtractItemSmallCategory;
	*CraftingTableItemInputJobMessage << SubtractItemCount;

	CraftingTableMaterialItemInputJob->GameObjectJobMessage = CraftingTableItemInputJobMessage;

	return CraftingTableMaterialItemInputJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobCraftingTableCompleteItemSubtract(CGameObject* CraftingTableItemSubtractObject, int16 SubtractItemSmallCategory, int16 SubtractItemCount)
{
	st_GameObjectJob* CraftingTableCompleteItemInputJob = G_ObjectManager->GameObjectJobCreate();
	CraftingTableCompleteItemInputJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_COMPLETE_ITEM_SUBTRACT;

	CGameServerMessage* CraftingTableItemInputJobMessage = CGameServerMessage::GameServerMessageAlloc();
	CraftingTableItemInputJobMessage->Clear();

	*CraftingTableItemInputJobMessage << &CraftingTableItemSubtractObject;
	*CraftingTableItemInputJobMessage << SubtractItemSmallCategory;
	*CraftingTableItemInputJobMessage << SubtractItemCount;

	CraftingTableCompleteItemInputJob->GameObjectJobMessage = CraftingTableItemInputJobMessage;

	return CraftingTableCompleteItemInputJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobCraftingTableCancel(CGameObject* CraftingStopObject)
{
	st_GameObjectJob* CraftingTableStopJob = G_ObjectManager->GameObjectJobCreate();
	CraftingTableStopJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_CRAFTING_STOP;

	CGameServerMessage* CraftingTableStopJobMessage = CGameServerMessage::GameServerMessageAlloc();
	CraftingTableStopJobMessage->Clear();

	*CraftingTableStopJobMessage << &CraftingStopObject;

	CraftingTableStopJob->GameObjectJobMessage = CraftingTableStopJobMessage;

	return CraftingTableStopJob;
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

CGameServerMessage* CGameServer::MakePacketMyCharacterInfos(int8& CharacterCount, int32* MyPlayerIndexes)
{
	CGameServerMessage* CharacterInfoMessage = CGameServerMessage::GameServerMessageAlloc();
	if (CharacterInfoMessage == nullptr)
	{
		return nullptr;
	}

	CharacterInfoMessage->Clear();

	*CharacterInfoMessage << (int16)en_PACKET_S2C_GAME_CHARACTER_INFOS;
	*CharacterInfoMessage << CharacterCount;

	for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
	{
		if (G_ObjectManager->_PlayersArray[MyPlayerIndexes[i]] != nullptr)
		{
			*CharacterInfoMessage << G_ObjectManager->_PlayersArray[MyPlayerIndexes[i]]->_GameObjectInfo;
		}
	}

	return CharacterInfoMessage;
}

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

CGameServerMessage* CGameServer::MakePacketResSelectItem(int64 AccountId, int64 ObjectId, CItem* SelectItem)
{
	CGameServerMessage* ResSelectItemMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResSelectItemMessage == nullptr)
	{
		return nullptr;
	}

	ResSelectItemMessage->Clear();

	*ResSelectItemMessage << (int16)en_PACKET_S2C_ITEM_SELECT;
	*ResSelectItemMessage << AccountId;
	*ResSelectItemMessage << ObjectId;
	*ResSelectItemMessage << SelectItem;

	return ResSelectItemMessage;
}

CGameServerMessage* CGameServer::MakePacketResItemRotate(int64 AccountID, int64 PlayerID)
{
	CGameServerMessage* ResItemRotateMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResItemRotateMessage == nullptr)
	{
		return nullptr;
	}

	ResItemRotateMessage->Clear();

	*ResItemRotateMessage << (int16)en_PACKET_S2C_ITEM_ROTATE;
	*ResItemRotateMessage << AccountID;
	*ResItemRotateMessage << PlayerID;

	return ResItemRotateMessage;
}

CGameServerMessage* CGameServer::MakePacketResPlaceItem(int64 AccountId, int64 ObjectId, CItem* PlaceItem, CItem* OverlapItem)
{
	CGameServerMessage* ResPlaceItemMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResPlaceItemMessage == nullptr)
	{
		return nullptr;
	}

	ResPlaceItemMessage->Clear();

	*ResPlaceItemMessage << (int16)en_PACKET_S2C_ITEM_PLACE;
	*ResPlaceItemMessage << AccountId;
	*ResPlaceItemMessage << ObjectId;
	*ResPlaceItemMessage << PlaceItem;

	if (OverlapItem != nullptr)
	{
		*ResPlaceItemMessage << OverlapItem;
	}

	return ResPlaceItemMessage;
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

CGameServerMessage* CGameServer::MakePacketResQuickSlotSwap(st_QuickSlotBarSlotInfo SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo SwapBQuickSlotInfo)
{
	CGameServerMessage* ResQuickSlotSwapMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResQuickSlotSwapMessage == nullptr)
	{
		return nullptr;
	}

	ResQuickSlotSwapMessage->Clear();

	*ResQuickSlotSwapMessage << (int16)en_PACKET_S2C_QUICKSLOT_SWAP;

	*ResQuickSlotSwapMessage << SwapAQuickSlotInfo;
	*ResQuickSlotSwapMessage << SwapBQuickSlotInfo;

	return ResQuickSlotSwapMessage;
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
		int16 CraftingItemCategoryNameLen = (int16)(CraftingItemCategory.CategoryName.length() * 2);
		*ResCraftingListMessage << CraftingItemCategoryNameLen;
		ResCraftingListMessage->InsertData(CraftingItemCategory.CategoryName.c_str(), CraftingItemCategoryNameLen);

		*ResCraftingListMessage << (int8)CraftingItemCategory.CommonCraftingCompleteItems.size();

		for (CItem* CraftingCompleteItem : CraftingItemCategory.CommonCraftingCompleteItems)
		{
			*ResCraftingListMessage << CraftingCompleteItem->_ItemInfo;			
		}
	}

	return ResCraftingListMessage;
}

CGameServerMessage* CGameServer::MakePacketResBuildingInstall(bool BuildingSuccess, vector<st_TileInfo> BuildingTileInfos)
{
	CGameServerMessage* ResBuildingInstallPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResBuildingInstallPacket == nullptr)
	{
		return nullptr;
	}

	ResBuildingInstallPacket->Clear();

	*ResBuildingInstallPacket << (int16)en_PACKET_S2C_BUILDING;
	*ResBuildingInstallPacket << BuildingSuccess;

	if (BuildingSuccess == true)
	{
		int16 BuildingTileCount = (int16)BuildingTileInfos.size();
		*ResBuildingInstallPacket << BuildingTileCount;

		for (st_TileInfo BuildingTileInfo : BuildingTileInfos)
		{
			*ResBuildingInstallPacket << BuildingTileInfo;
		}
	}

	return nullptr;
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

CItem* CGameServer::NewItemCrate(st_ItemInfo& NewItemInfo)
{
	CItem* NewItem = G_ObjectManager->ItemCreate(NewItemInfo.ItemSmallCategory);
	if (NewItem != nullptr)
	{
		NewItem->_ItemInfo = NewItemInfo;		
	}
	
	return NewItem;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobObjectDeSpawnObjectChannel(CGameObject* DeSpawnChannelObject)
{
	st_GameObjectJob* DeSpawnObjectChannelJob = G_ObjectManager->GameObjectJobCreate();
	DeSpawnObjectChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_OBJECT_DESPAWN;

	CGameServerMessage* DeSpawnObjectChannelGameMessage = CGameServerMessage::GameServerMessageAlloc();
	DeSpawnObjectChannelGameMessage->Clear();

	*DeSpawnObjectChannelGameMessage << &DeSpawnChannelObject;
	DeSpawnObjectChannelJob->GameObjectJobMessage = DeSpawnObjectChannelGameMessage;

	return DeSpawnObjectChannelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobPlayerEnterChannel(CGameObject* EnterChannelObject)
{
	st_GameObjectJob* EnterChannelJob = G_ObjectManager->GameObjectJobCreate();
	EnterChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PLAYER_ENTER;

	CGameServerMessage* EnterChannelGameMessage = CGameServerMessage::GameServerMessageAlloc();
	EnterChannelGameMessage->Clear();

	*EnterChannelGameMessage << &EnterChannelObject;
	EnterChannelJob->GameObjectJobMessage = EnterChannelGameMessage;

	return EnterChannelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobObjectEnterChannel(CGameObject* EnterChannelObject)
{
	st_GameObjectJob* EnterChannelJob = G_ObjectManager->GameObjectJobCreate();
	EnterChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_OBJECT_ENTER;

	CGameServerMessage* EnterChannelGameMessage = CGameServerMessage::GameServerMessageAlloc();
	EnterChannelGameMessage->Clear();

	*EnterChannelGameMessage << &EnterChannelObject;	

	EnterChannelJob->GameObjectJobMessage = EnterChannelGameMessage;

	return EnterChannelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobFindObjectChannel(CGameObject* ReqPlayer, int64& FindObjectID, int16& FindObjectType)
{
	st_GameObjectJob* FindChannelObjectJob = G_ObjectManager->GameObjectJobCreate();
	FindChannelObjectJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_FIND_OBJECT;

	CGameServerMessage* FindChannObjectMessage = CGameServerMessage::GameServerMessageAlloc();
	FindChannObjectMessage->Clear();

	*FindChannObjectMessage << &ReqPlayer;
	*FindChannObjectMessage << FindObjectID;
	*FindChannObjectMessage << FindObjectType;

	FindChannelObjectJob->GameObjectJobMessage = FindChannObjectMessage;

	return FindChannelObjectJob;
}

CGameServerMessage* CGameServer::MakePacketResCraftingTableNonSelect(int64 CraftingTableObjectID, en_GameObjectType CraftingTableObjectType)
{
	CGameServerMessage* ResCraftingTableNonSelectMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCraftingTableNonSelectMessage == nullptr)
	{
		return nullptr;
	}

	ResCraftingTableNonSelectMessage->Clear();

	*ResCraftingTableNonSelectMessage << (int16)en_PACKET_S2C_CRAFTING_TABLE_NON_SELECT;
	*ResCraftingTableNonSelectMessage << CraftingTableObjectID;
	*ResCraftingTableNonSelectMessage << (int16)CraftingTableObjectType;

	return ResCraftingTableNonSelectMessage;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobFindCraftingTableSelectItem(CGameObject* ReqPlayer, int64& FindCraftingTableObjectID, int16& FindCraftingTableObjectType, int16& LeftMouseItemCategory)
{
	st_GameObjectJob* FindCraftingTableJob = G_ObjectManager->GameObjectJobCreate();
	FindCraftingTableJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_CRAFTING_TABLE_SELECT_ITEM;

	CGameServerMessage* FindCraftingTableMessage = CGameServerMessage::GameServerMessageAlloc();
	FindCraftingTableMessage->Clear();

	*FindCraftingTableMessage << &ReqPlayer;
	*FindCraftingTableMessage << FindCraftingTableObjectID;
	*FindCraftingTableMessage << FindCraftingTableObjectType;
	*FindCraftingTableMessage << LeftMouseItemCategory;

	FindCraftingTableJob->GameObjectJobMessage = FindCraftingTableMessage;

	return FindCraftingTableJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobRightMouseObjectInfo(CGameObject* ReqPlayer, int64& FindObjectID, int16& FindObjectType)
{
	st_GameObjectJob* RightMouseObjectInfoJob = G_ObjectManager->GameObjectJobCreate();
	RightMouseObjectInfoJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_RIGHT_MOUSE_OBJECT_INFO;

	CGameServerMessage* RightMouseObjectInfoMessage = CGameServerMessage::GameServerMessageAlloc();
	RightMouseObjectInfoMessage->Clear();

	*RightMouseObjectInfoMessage << &ReqPlayer;
	*RightMouseObjectInfoMessage << FindObjectID;
	*RightMouseObjectInfoMessage << FindObjectType;

	RightMouseObjectInfoJob->GameObjectJobMessage = RightMouseObjectInfoMessage;

	return RightMouseObjectInfoJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectLeftMouseDragObjectsSelect(int64 TargetID, float MouseDragStartPositionX, float MouseDragStartPositionY, float MouseDragEndPositionX, float MouseDragEndPositionY)
{
	st_GameObjectJob* LeftDragObjectsSelectJob = G_ObjectManager->GameObjectJobCreate();
	LeftDragObjectsSelectJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_LEFT_MOUSE_DRAG_OBJECTS_SELECT;

	CGameServerMessage* LeftDragObjectsSelectMessage = CGameServerMessage::GameServerMessageAlloc();
	LeftDragObjectsSelectMessage->Clear();

	*LeftDragObjectsSelectMessage << TargetID;
	*LeftDragObjectsSelectMessage << MouseDragStartPositionX;
	*LeftDragObjectsSelectMessage << MouseDragStartPositionY;
	*LeftDragObjectsSelectMessage << MouseDragEndPositionX;
	*LeftDragObjectsSelectMessage << MouseDragEndPositionY;

	LeftDragObjectsSelectJob->GameObjectJobMessage = LeftDragObjectsSelectMessage;

	return LeftDragObjectsSelectJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobLeaveChannel(CGameObject* LeaveChannelObject)
{
	st_GameObjectJob* LeaveChannelJob = G_ObjectManager->GameObjectJobCreate();
	LeaveChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_OBJECT_LEAVE;

	CGameServerMessage* LeaveChannelMessage = CGameServerMessage::GameServerMessageAlloc();
	LeaveChannelMessage->Clear();

	*LeaveChannelMessage << &LeaveChannelObject;		

	LeaveChannelJob->GameObjectJobMessage = LeaveChannelMessage;

	return LeaveChannelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectDamage(int64& AttackerID, en_GameObjectType AttackerType, en_SkillKinds SkillKind, int32& SkillMinDamage, int32& SkillMaxDamage, bool IsBackAttack)
{
	st_GameObjectJob* DamageJob = G_ObjectManager->GameObjectJobCreate();
	DamageJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_DAMAGE;

	CGameServerMessage* DamageMessage = CGameServerMessage::GameServerMessageAlloc();
	DamageMessage->Clear();
	
	*DamageMessage << AttackerID;
	*DamageMessage << (int16)AttackerType;	
	*DamageMessage << (int8)SkillKind;
	*DamageMessage << SkillMinDamage; 
	*DamageMessage << SkillMaxDamage;
	*DamageMessage << IsBackAttack;

	DamageJob->GameObjectJobMessage = DamageMessage;

	return DamageJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobHPHeal(CGameObject* Healer, bool IsCritical, int32 HealPoint, en_SkillType SkillType)
{
	st_GameObjectJob* SkillHealJob = G_ObjectManager->GameObjectJobCreate();
	SkillHealJob->GameObjectJobType = en_GameObjectJobType::GAMEOJBECT_JOB_TYPE_SKILL_HP_HEAL;

	CGameServerMessage* SkillHealMessage = CGameServerMessage::GameServerMessageAlloc();
	SkillHealMessage->Clear();

	*SkillHealMessage << &Healer;
	*SkillHealMessage << IsCritical;
	*SkillHealMessage << HealPoint;
	*SkillHealMessage << (int16)SkillType;

	SkillHealJob->GameObjectJobMessage = SkillHealMessage;

	return SkillHealJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobItemHPHeal(en_SmallItemCategory HealItemCategory)
{
	st_GameObjectJob* ItemHealJob = G_ObjectManager->GameObjectJobCreate();
	ItemHealJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_ITEM_HP_HEAL;

	CGameServerMessage* ItemHealMessage = CGameServerMessage::GameServerMessageAlloc();
	ItemHealMessage->Clear();

	*ItemHealMessage << (int16)HealItemCategory;	

	ItemHealJob->GameObjectJobMessage = ItemHealMessage;

	return ItemHealJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobMPHeal(CGameObject* Healer, int32 MPHealPoint)
{
	st_GameObjectJob* MPHealJob = G_ObjectManager->GameObjectJobCreate();
	MPHealJob->GameObjectJobType = en_GameObjectJobType::GAMEOJBECT_JOB_TYPE_SKILL_MP_HEAL;

	CGameServerMessage* MPHealMessage = CGameServerMessage::GameServerMessageAlloc();
	MPHealMessage->Clear();

	*MPHealMessage << &Healer;
	*MPHealMessage << MPHealPoint;

	MPHealJob->GameObjectJobMessage = MPHealMessage;

	return MPHealJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobOnEquipment(CItem* EquipmentItem)
{
	st_GameObjectJob* DoEquipmentJob = G_ObjectManager->GameObjectJobCreate();
	DoEquipmentJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_ON_EQUIPMENT;

	CGameServerMessage* DoEquipmentMessage = CGameServerMessage::GameServerMessageAlloc();
	DoEquipmentMessage->Clear();

	*DoEquipmentMessage << &EquipmentItem;

	DoEquipmentJob->GameObjectJobMessage = DoEquipmentMessage;

	return DoEquipmentJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobOffEquipment(int8& EquipmentParts)
{
	st_GameObjectJob* UnDoEquipmentJob = G_ObjectManager->GameObjectJobCreate();
	UnDoEquipmentJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_OFF_EQUIPMENT;

	CGameServerMessage* UnDoEquipmentMessage = CGameServerMessage::GameServerMessageAlloc();
	UnDoEquipmentMessage->Clear();

	*UnDoEquipmentMessage << EquipmentParts;

	UnDoEquipmentJob->GameObjectJobMessage = UnDoEquipmentMessage;

	return UnDoEquipmentJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobSeedFarming(CGameObject* ReqSeedFarmingObject, int16 SeedItemCategory)
{
	st_GameObjectJob* SeedFarmingJob = G_ObjectManager->GameObjectJobCreate();
	SeedFarmingJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_SEED_FARMING;

	CGameServerMessage* SeedFarmingMessage = CGameServerMessage::GameServerMessageAlloc();
	SeedFarmingMessage->Clear();

	*SeedFarmingMessage << &ReqSeedFarmingObject;
	*SeedFarmingMessage << SeedItemCategory;

	SeedFarmingJob->GameObjectJobMessage = SeedFarmingMessage;

	return SeedFarmingJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobPlantGrowthCheck(CGameObject* ReqPlantGrowthCheckObject, int64 PlantObjectID, int16 PlantObjectType)
{
	st_GameObjectJob* PlantGrowthCheckJob = G_ObjectManager->GameObjectJobCreate();
	PlantGrowthCheckJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PLANT_GROWTH_CHECK;

	CGameServerMessage* PlantGrowthCheckMessage = CGameServerMessage::GameServerMessageAlloc();
	PlantGrowthCheckMessage->Clear();

	*PlantGrowthCheckMessage << &ReqPlantGrowthCheckObject;
	*PlantGrowthCheckMessage << PlantObjectID;
	*PlantGrowthCheckMessage << PlantObjectType;

	PlantGrowthCheckJob->GameObjectJobMessage = PlantGrowthCheckMessage;

	return PlantGrowthCheckJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobCraftingTableSelect(CGameObject* CraftingTableObject, CGameObject* OwnerObject)
{
	st_GameObjectJob* CraftingTableSelectJob = G_ObjectManager->GameObjectJobCreate();
	CraftingTableSelectJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_CRAFTING_TABLE_SELECT_ITEM;

	CGameServerMessage* CraftingTableSelectJobMessage = CGameServerMessage::GameServerMessageAlloc();
	CraftingTableSelectJobMessage->Clear();

	*CraftingTableSelectJobMessage << &CraftingTableObject;
	*CraftingTableSelectJobMessage << &OwnerObject;

	CraftingTableSelectJob->GameObjectJobMessage = CraftingTableSelectJobMessage;

	return CraftingTableSelectJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobCraftingTableNonSelect(CGameObject* CraftingTableObject, int64& CraftingTableObjectID, int16& CraftingTableObjectType)
{
	st_GameObjectJob* CraftingTableSelectJob = G_ObjectManager->GameObjectJobCreate();
	CraftingTableSelectJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_CRAFTING_TABLE_NON_SELECT;

	CGameServerMessage* CraftingTableSelectJobMessage = CGameServerMessage::GameServerMessageAlloc();
	CraftingTableSelectJobMessage->Clear();

	*CraftingTableSelectJobMessage << &CraftingTableObject;
	*CraftingTableSelectJobMessage << CraftingTableObjectID;
	*CraftingTableSelectJobMessage << CraftingTableObjectType;

	CraftingTableSelectJob->GameObjectJobMessage = CraftingTableSelectJobMessage;

	return CraftingTableSelectJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobItemSave(CGameObject* Item)
{
	st_GameObjectJob* ItemSaveJob = G_ObjectManager->GameObjectJobCreate();
	ItemSaveJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_ITEM_INVENTORY_SAVE;

	CGameServerMessage* ItemSaveMessage = CGameServerMessage::GameServerMessageAlloc();
	ItemSaveMessage->Clear();

	*ItemSaveMessage << &Item;

	ItemSaveJob->GameObjectJobMessage = ItemSaveMessage;

	return ItemSaveJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobPartyInvite(CGameObject* ReqPartyPlayer, int64 InvitePlayerID)
{
	st_GameObjectJob* PartyInviteJob = G_ObjectManager->GameObjectJobCreate();
	PartyInviteJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_INVITE;

	CGameServerMessage* PartyInviteMessage = CGameServerMessage::GameServerMessageAlloc();
	PartyInviteMessage->Clear();

	*PartyInviteMessage << &ReqPartyPlayer;
	*PartyInviteMessage << InvitePlayerID;

	PartyInviteJob->GameObjectJobMessage = PartyInviteMessage;

	return PartyInviteJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobPartyAccept(int64 PartyReqPlayerID, int64 PartyAcceptPlayerID)
{
	st_GameObjectJob* PartyAcceptJob = G_ObjectManager->GameObjectJobCreate();
	PartyAcceptJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_INVITE_ACCEPT;

	CGameServerMessage* PartyAcceptMessage = CGameServerMessage::GameServerMessageAlloc();
	PartyAcceptMessage->Clear();

	*PartyAcceptMessage << PartyReqPlayerID;
	*PartyAcceptMessage << PartyAcceptPlayerID;

	PartyAcceptJob->GameObjectJobMessage = PartyAcceptMessage;

	return PartyAcceptJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobPartyReject(int64 PartyRejectPlayerID, int64 ReqPartyInvitePlayerID)
{
	st_GameObjectJob* PartyRejectJob = G_ObjectManager->GameObjectJobCreate();
	PartyRejectJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_INVITE_REJECT;

	CGameServerMessage* PartyRejectMessage = CGameServerMessage::GameServerMessageAlloc();
	PartyRejectMessage->Clear();

	*PartyRejectMessage << PartyRejectPlayerID;
	*PartyRejectMessage << ReqPartyInvitePlayerID;

	PartyRejectJob->GameObjectJobMessage = PartyRejectMessage;

	return PartyRejectJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobPartyQuit(int64 PartyQuitPlayerID)
{
	st_GameObjectJob* PartyInviteJob = G_ObjectManager->GameObjectJobCreate();
	PartyInviteJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_QUIT;

	CGameServerMessage* PartyInviteMessage = CGameServerMessage::GameServerMessageAlloc();
	PartyInviteMessage->Clear();

	*PartyInviteMessage << PartyQuitPlayerID;

	PartyInviteJob->GameObjectJobMessage = PartyInviteMessage;

	return PartyInviteJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobPartyBanish(CGameObject* ReqPartyBanishPlayer, int64 PartyBanishPlayerID)
{
	st_GameObjectJob* PartyBanishJob = G_ObjectManager->GameObjectJobCreate();
	PartyBanishJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_BANISH;

	CGameServerMessage* PartyInviteMessage = CGameServerMessage::GameServerMessageAlloc();
	PartyInviteMessage->Clear();

	*PartyInviteMessage << &ReqPartyBanishPlayer;
	*PartyInviteMessage << PartyBanishPlayerID;

	PartyBanishJob->GameObjectJobMessage = PartyInviteMessage;

	return PartyBanishJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectMenu(int8 MenuType, CGameObject* MenuReqGameObject)
{
	st_GameObjectJob* MenuJob = G_ObjectManager->GameObjectJobCreate();
	MenuJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_MENU;

	CGameServerMessage* MenuJobMessage = CGameServerMessage::GameServerMessageAlloc();
	MenuJobMessage->Clear();

	*MenuJobMessage << MenuType;	
	*MenuJobMessage << &MenuReqGameObject;

	MenuJob->GameObjectJobMessage = MenuJobMessage;

	return MenuJob;	
}

st_GameObjectJob* CGameServer::MakeGameObjectJobInteraction(int16 InteractionType, int16 ObjectType, int64 ObjectID)
{
	st_GameObjectJob* InteractionJob = G_ObjectManager->GameObjectJobCreate();
	InteractionJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_INTERACTION;

	CGameServerMessage* InteractionJobMessage = CGameServerMessage::GameServerMessageAlloc();
	InteractionJobMessage->Clear();

	*InteractionJobMessage << InteractionType;
	*InteractionJobMessage << ObjectType;
	*InteractionJobMessage << ObjectID;

	InteractionJob->GameObjectJobMessage = InteractionJobMessage;

	return InteractionJob;
}

CGameServerMessage* CGameServer::MakePacketResEnterGame(bool EnterGameSuccess, st_GameObjectInfo* ObjectInfo, Vector2Int* SpawnPosition, vector<st_TileInfo>* AroundTileInfos)
{
	CGameServerMessage* ResEnterGamePacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResEnterGamePacket == nullptr)
	{
		return nullptr;
	}

	ResEnterGamePacket->Clear();

	*ResEnterGamePacket << (int16)en_PACKET_S2C_GAME_ENTER;
	*ResEnterGamePacket << EnterGameSuccess;

	if (ObjectInfo != nullptr)
	{
		*ResEnterGamePacket << *ObjectInfo;
	}

	if (SpawnPosition != nullptr)
	{
		*ResEnterGamePacket << *SpawnPosition;
	}

	if (AroundTileInfos->size() > 0)
	{
		int16 AroundTileInfosCount = (int16)AroundTileInfos->size();
		*ResEnterGamePacket << AroundTileInfosCount;

		for (st_TileInfo AroundTileInfo : *AroundTileInfos)
		{
			*ResEnterGamePacket << AroundTileInfo;
		}
	}	

	return ResEnterGamePacket;
}

CGameServerMessage* CGameServer::MakePacketResDamage(int64 ObjectID, int64 TargetID, int8 SkillKind, en_ResourceName EffectType, int32 Damage, bool IsCritical)
{
	CGameServerMessage* ResDamageMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResDamageMessage == nullptr)
	{
		return nullptr;
	}

	ResDamageMessage->Clear();

	*ResDamageMessage << (int16)en_PACKET_S2C_COMMON_DAMAGE;
	*ResDamageMessage << ObjectID;
	*ResDamageMessage << TargetID;
	*ResDamageMessage << SkillKind;
	*ResDamageMessage << (int16)EffectType;
	*ResDamageMessage << Damage;
	*ResDamageMessage << IsCritical;

	return ResDamageMessage;
}

CGameServerMessage* CGameServer::MakePacketResGatheringDamage(int64 TargetID)
{
	CGameServerMessage* ResGatheringDamage = CGameServerMessage::GameServerMessageAlloc();
	if (ResGatheringDamage == nullptr)
	{
		return nullptr;
	}

	ResGatheringDamage->Clear();

	*ResGatheringDamage << (int16)en_PACKET_S2C_GATHERING_DAMAGE;
	*ResGatheringDamage << TargetID;

	return ResGatheringDamage;
}

CGameServerMessage* CGameServer::MakePacketResAttack(int64 ObjectID)
{
	CGameServerMessage* ResAttackMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResAttackMessage == nullptr)
	{
		return nullptr;
	}

	ResAttackMessage->Clear();

	*ResAttackMessage << (int16)en_PACKET_S2C_ATTACK;
	*ResAttackMessage << ObjectID;

	return ResAttackMessage;
}

CGameServerMessage* CGameServer::MakePacketSkillCastingStart(int64 ObjectId, bool SpellStart, en_SkillType SkillType, float SpellTime)
{
	CGameServerMessage* ResSkillCastingStartMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResSkillCastingStartMessage == nullptr)
	{
		return nullptr;
	}

	ResSkillCastingStartMessage->Clear();

	*ResSkillCastingStartMessage << (int16)en_PACKET_S2C_SKILL_CASTING_START;
	*ResSkillCastingStartMessage << ObjectId;
	*ResSkillCastingStartMessage << SpellStart;
	*ResSkillCastingStartMessage << (int16)SkillType;
	*ResSkillCastingStartMessage << SpellTime;

	return ResSkillCastingStartMessage;
}

CGameServerMessage* CGameServer::MakePacketResGathering(int64 ObjectID, bool GatheringStart, wstring GatheringName)
{
	CGameServerMessage* ResGatheringMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResGatheringMessage == nullptr)
	{
		return nullptr;
	}

	ResGatheringMessage->Clear();

	*ResGatheringMessage << (int16)en_PACKET_S2C_GATHERING;
	*ResGatheringMessage << ObjectID;
	*ResGatheringMessage << GatheringStart;

	if (GatheringStart == true)
	{
		int16 GatheringNameLen = (int16)(GatheringName.length() * 2);
		*ResGatheringMessage << GatheringNameLen;
		ResGatheringMessage->InsertData(GatheringName.c_str(), GatheringNameLen);
	}

	return ResGatheringMessage;
}

CGameServerMessage* CGameServer::MakePacketResRightMousePositionObjectInfo(int64 ReqPlayerID, int64 FindObjectID, en_GameObjectType FindObjectType)
{
	CGameServerMessage* ResRightMousePositionObjectInfoMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResRightMousePositionObjectInfoMessage == nullptr)
	{
		return nullptr;
	}

	ResRightMousePositionObjectInfoMessage->Clear();

	*ResRightMousePositionObjectInfoMessage << (int16)en_PACKET_S2C_RIGHT_MOUSE_OBJECT_INFO;
	*ResRightMousePositionObjectInfoMessage << ReqPlayerID;
	*ResRightMousePositionObjectInfoMessage << FindObjectID;
	*ResRightMousePositionObjectInfoMessage << (int16)FindObjectType;

	return ResRightMousePositionObjectInfoMessage;
}

CGameServerMessage* CGameServer::MakePacketResLeftMousePositionObjectInfo(int64 AccountId, int64 PreviousChoiceObjectId, int64 FindObjectId,
	map<en_SkillType, CSkill*> BufSkillInfo, map<en_SkillType, CSkill*> DeBufSkillInfo)
{
	CGameServerMessage* ResMousePositionObjectInfoPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResMousePositionObjectInfoPacket == nullptr)
	{
		return nullptr;
	}

	ResMousePositionObjectInfoPacket->Clear();

	*ResMousePositionObjectInfoPacket << (int16)en_PACKET_S2C_LEFT_MOUSE_OBJECT_INFO;
	*ResMousePositionObjectInfoPacket << AccountId;
	*ResMousePositionObjectInfoPacket << PreviousChoiceObjectId;
	*ResMousePositionObjectInfoPacket << FindObjectId;

	int16 BufSize = (int16)BufSkillInfo.size();
	*ResMousePositionObjectInfoPacket << BufSize;

	for (auto BufSkillIterator : BufSkillInfo)
	{
		*ResMousePositionObjectInfoPacket << *(BufSkillIterator.second->GetSkillInfo());
	}

	int16 DeBufSize = (int16)DeBufSkillInfo.size();
	*ResMousePositionObjectInfoPacket << DeBufSize;

	for (auto DeBufSkillIterator : DeBufSkillInfo)
	{
		*ResMousePositionObjectInfoPacket << *(DeBufSkillIterator.second->GetSkillInfo());
	}

	return ResMousePositionObjectInfoPacket;
}

CGameServerMessage* CGameServer::MakePacketResLeftMouseDragObjectsSelect(vector<int64> DragObjects)
{
	CGameServerMessage* ResLeftMouseDragObjectsSelectMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResLeftMouseDragObjectsSelectMessage == nullptr)
	{
		return nullptr;
	}

	ResLeftMouseDragObjectsSelectMessage->Clear();

	*ResLeftMouseDragObjectsSelectMessage << (int16)en_PACKET_S2C_LEFT_MOUSE_DRAG_OBJECTS_SELECT;
	
	int8 DragObjectsCount = (int8)DragObjects.size();
	*ResLeftMouseDragObjectsSelectMessage << DragObjectsCount;

	for (auto DragObjectID : DragObjects)
	{
		*ResLeftMouseDragObjectsSelectMessage << DragObjectID;
	}

	return ResLeftMouseDragObjectsSelectMessage;
}

CGameServerMessage* CGameServer::MakePacketResCraftingTableCompleteItemSelect(int64 CraftingTableObjectID, en_SmallItemCategory SelectCompleteType, map<en_SmallItemCategory, CItem*> MaterialItems)
{
	CGameServerMessage* ResCraftingTableCompleteItemSelectMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCraftingTableCompleteItemSelectMessage == nullptr)
	{
		return nullptr;
	}

	ResCraftingTableCompleteItemSelectMessage->Clear();

	*ResCraftingTableCompleteItemSelectMessage << (int16)en_PACKET_S2C_CRAFTING_TABLE_COMPLETE_ITEM_SELECT;
	*ResCraftingTableCompleteItemSelectMessage << CraftingTableObjectID;
	*ResCraftingTableCompleteItemSelectMessage << (int16)SelectCompleteType;

	int16 MaterialItemCount = (int16)MaterialItems.size();
	*ResCraftingTableCompleteItemSelectMessage << MaterialItemCount;

	for (auto MaterialItemIter : MaterialItems)
	{
		*ResCraftingTableCompleteItemSelectMessage << MaterialItemIter.second->_ItemInfo;
	}

	return ResCraftingTableCompleteItemSelectMessage;
}

CGameServerMessage* CGameServer::MakePacketResAnimationPlay(int64 ObjectId, en_GameObjectType ObjectType, en_AnimationType AnimationType)
{
	CGameServerMessage* ResAnimationPlayMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResAnimationPlayMessage == nullptr)
	{
		return nullptr;
	}

	ResAnimationPlayMessage->Clear();

	*ResAnimationPlayMessage << (int16)en_PACKET_S2C_ANIMATION_PLAY;
	*ResAnimationPlayMessage << ObjectId;	
	*ResAnimationPlayMessage << (int16)ObjectType;
	*ResAnimationPlayMessage << (int8)AnimationType;	

	return ResAnimationPlayMessage;
}

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

CGameServerMessage* CGameServer::MakePacketResChangeObjectState(int64 ObjectId, en_CreatureState ObjectState)
{
	CGameServerMessage* ResObjectStatePacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResObjectStatePacket == nullptr)
	{
		return nullptr;
	}

	ResObjectStatePacket->Clear();

	*ResObjectStatePacket << (int16)en_PACKET_S2C_OBJECT_STATE_CHANGE;
	*ResObjectStatePacket << ObjectId;		
	*ResObjectStatePacket << (int8)ObjectState;

	return ResObjectStatePacket;
}

CGameServerMessage* CGameServer::MakePacketResFaceDirection(int64 ObjectID, float FaceDirectionX, float FaceDirectionY)
{
	CGameServerMessage* ResFaceDirectionPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResFaceDirectionPacket == nullptr)
	{
		return nullptr;
	}

	ResFaceDirectionPacket->Clear();

	*ResFaceDirectionPacket << (int16)en_PACKET_S2C_FACE_DIRECTION;
	*ResFaceDirectionPacket << ObjectID;
	*ResFaceDirectionPacket << FaceDirectionX;
	*ResFaceDirectionPacket << FaceDirectionY;

	return ResFaceDirectionPacket;
}

CGameServerMessage* CGameServer::MakePacketResMove(int64& ObjectID, Vector2& LookAtDirection, Vector2& MoveDirection, Vector2& Position, en_CreatureState State, int64 TargetID)
{
	CGameServerMessage* ResMoveMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResMoveMessage == nullptr)
	{
		return nullptr;
	}

	ResMoveMessage->Clear();

	*ResMoveMessage << (int16)en_PACKET_S2C_MOVE;	
	*ResMoveMessage << ObjectID;	
	*ResMoveMessage << LookAtDirection.X;
	*ResMoveMessage << LookAtDirection.Y;
	*ResMoveMessage << MoveDirection.X;
	*ResMoveMessage << MoveDirection.Y;		
	*ResMoveMessage << Position.X;
	*ResMoveMessage << Position.Y;
	*ResMoveMessage << (int8)State;
	*ResMoveMessage << TargetID;

	return ResMoveMessage;
}

CGameServerMessage* CGameServer::MakePacketResMoveStop(int64 ObjectId, float StopPositionX, float StopPositionY)
{
	CGameServerMessage* ResMoveStopPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResMoveStopPacket == nullptr)
	{
		return nullptr;
	}

	ResMoveStopPacket->Clear();

	*ResMoveStopPacket << (int16)en_PACKET_S2C_MOVE_STOP;	
	*ResMoveStopPacket << ObjectId;
	*ResMoveStopPacket << StopPositionX;
	*ResMoveStopPacket << StopPositionY;	

	return ResMoveStopPacket;
}

CGameServerMessage* CGameServer::MakePacketResToAttack(int64 ObjectID, int64 TargetID,  float StopPositionX, float StopPositionY, en_CreatureState State)
{
	CGameServerMessage* ResToAttackPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResToAttackPacket == nullptr)
	{
		return nullptr;
	}

	ResToAttackPacket->Clear();

	*ResToAttackPacket << (int16)en_PACKET_S2C_TO_ATTACK;
	*ResToAttackPacket << ObjectID;
	*ResToAttackPacket << TargetID;
	*ResToAttackPacket << StopPositionX;
	*ResToAttackPacket << StopPositionY;
	*ResToAttackPacket << (byte)State;

	return ResToAttackPacket;
}

CGameServerMessage* CGameServer::MakePacketItemMove(st_GameObjectInfo ItemMoveObjectInfo)
{
	CGameServerMessage* ResItemMovePacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResItemMovePacket == nullptr)
	{
		return nullptr;
	}

	ResItemMovePacket->Clear();

	*ResItemMovePacket << (int16)en_PACKET_S2C_ITEM_MOVE_START;
	*ResItemMovePacket << ItemMoveObjectInfo;	

	return ResItemMovePacket;
}

CGameServerMessage* CGameServer::MakePacketRectCollisionSpawn(CRectCollision* RectCollision)
{
	if (RectCollision == nullptr)
	{
		CRASH("RectCollision null")
	}

	CGameServerMessage* RectCollisionSpawnPacket = CGameServerMessage::GameServerMessageAlloc();
	if (RectCollisionSpawnPacket == nullptr)
	{
		return nullptr;
	}

	RectCollisionSpawnPacket->Clear();

	*RectCollisionSpawnPacket << (int16)en_PACKET_S2C_COLLISION;

	*RectCollisionSpawnPacket << (byte)RectCollision->_CollisionPosition;
	*RectCollisionSpawnPacket << RectCollision->_Position.X;
	*RectCollisionSpawnPacket << RectCollision->_Position.Y;
	*RectCollisionSpawnPacket << RectCollision->_Direction.X;
	*RectCollisionSpawnPacket << RectCollision->_Direction.Y;		
	*RectCollisionSpawnPacket << RectCollision->_Size.X;
	*RectCollisionSpawnPacket << RectCollision->_Size.Y;

	return RectCollisionSpawnPacket;
}

CGameServerMessage* CGameServer::MakePacketResObjectSpawn(CGameObject* SpawnObject)
{
	CGameServerMessage* ResSpawnPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResSpawnPacket == nullptr)
	{
		return nullptr;
	}

	ResSpawnPacket->Clear();

	*ResSpawnPacket << (int16)en_PACKET_S2C_SPAWN;

	*ResSpawnPacket << 1;
	*ResSpawnPacket << SpawnObject->_GameObjectInfo;

	return ResSpawnPacket;
}

CGameServerMessage* CGameServer::MakePacketResObjectSpawn(int32 ObjectInfosCount, vector<CGameObject*> ObjectInfos)
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
		if (ObjectInfos[i] != nullptr)
		{			
			*ResSpawnPacket << ObjectInfos[i]->_GameObjectInfo;

			CCreature* SpawnCreature = dynamic_cast<CCreature*>(ObjectInfos[i]);
			if (SpawnCreature != nullptr)
			{
				CEquipmentBox* SpawnCreatureEquipmentBox = SpawnCreature->GetEquipment();
				if (SpawnCreatureEquipmentBox != nullptr)
				{
					*ResSpawnPacket << SpawnCreatureEquipmentBox->_EquipmentCount;

					for (auto EquipemtPart : SpawnCreatureEquipmentBox->GetEquipments())
					{		
						if (EquipemtPart.second != nullptr)
						{
							*ResSpawnPacket << EquipemtPart.second;
						}						
					}
				}
			}
		}		
	}

	return ResSpawnPacket;
}

CGameServerMessage* CGameServer::MakePacketResObjectDeSpawn(int64 DeSpawnObjectID)
{
	CGameServerMessage* ResDeSpawnPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResDeSpawnPacket == nullptr)
	{
		return nullptr;
	}

	ResDeSpawnPacket->Clear();

	*ResDeSpawnPacket << (int16)en_PACKET_S2C_DESPAWN;
	*ResDeSpawnPacket << (int32)1;

	*ResDeSpawnPacket << DeSpawnObjectID;

	return ResDeSpawnPacket;
}

CGameServerMessage* CGameServer::MakePacketResObjectDeSpawn(int32 DeSpawnObjectCount, vector<CGameObject*> DeSpawnObjects)
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
		if (DeSpawnObjects[i] != nullptr)
		{
			*ResDeSpawnPacket << DeSpawnObjects[i]->_GameObjectInfo.ObjectId;
		}		
	}

	return ResDeSpawnPacket;
}

CGameServerMessage* CGameServer::MakePacketObjectDie(int64 DieObjectId, en_CreatureState DieObjectState)
{
	CGameServerMessage* ResDiePacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResDiePacket == nullptr)
	{
		return nullptr;
	}

	ResDiePacket->Clear();

	*ResDiePacket << (int16)en_PACKET_S2C_DIE;
	*ResDiePacket << DieObjectId;
	*ResDiePacket << (int8)DieObjectState;

	return ResDiePacket;
}

CGameServerMessage* CGameServer::MakePacketResChattingBoxMessage(en_MessageType MessageType, st_Color Color, wstring ChattingMessage)
{
	CGameServerMessage* ResChattingMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResChattingMessage == nullptr)
	{
		return nullptr;
	}

	ResChattingMessage->Clear();

	*ResChattingMessage << (int16)en_PACKET_S2C_MESSAGE;
	*ResChattingMessage << (int8)MessageType;

	*ResChattingMessage << Color;

	int16 PlayerNameLen = (int16)(ChattingMessage.length() * 2);
	*ResChattingMessage << PlayerNameLen;
	ResChattingMessage->InsertData(ChattingMessage.c_str(), PlayerNameLen);

	return ResChattingMessage;
}

CGameServerMessage* CGameServer::MakePacketResDamageChattingBoxMessage(en_MessageType MessageType, wstring AttackerName, wstring TargetName, en_SkillType DamageSkilltype, int32 Damage)
{
	CGameServerMessage* ResDamageChattingMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResDamageChattingMessage == nullptr)
	{
		return nullptr;
	}

	ResDamageChattingMessage->Clear();

	*ResDamageChattingMessage << (int16)en_PACKET_S2C_MESSAGE;
	*ResDamageChattingMessage << (int8)MessageType;	

	int16 AttackerNameLen = (int16)(AttackerName.length() * 2);
	*ResDamageChattingMessage << AttackerNameLen;
	ResDamageChattingMessage->InsertData(AttackerName.c_str(), AttackerNameLen);

	int16 TargetNameLen = (int16)(TargetName.length() * 2);
	*ResDamageChattingMessage << TargetNameLen;
	ResDamageChattingMessage->InsertData(TargetName.c_str(), TargetNameLen);

	*ResDamageChattingMessage << (int16)DamageSkilltype;

	*ResDamageChattingMessage << Damage;

	return ResDamageChattingMessage;
}

CGameServerMessage* CGameServer::MakePacketResSyncPosition(int64 TargetObjectId, st_PositionInfo SyncPosition)
{
	CGameServerMessage* ResSyncPositionMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResSyncPositionMessage == nullptr)
	{
		return nullptr;
	}

	ResSyncPositionMessage->Clear();

	*ResSyncPositionMessage << (int16)en_PACKET_S2C_SYNC_OBJECT_POSITION;
	*ResSyncPositionMessage << TargetObjectId;

	*ResSyncPositionMessage << SyncPosition;

	return ResSyncPositionMessage;
}

CGameServerMessage* CGameServer::MakePacketResQuickSlotInit(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex)
{
	CGameServerMessage* ResQuickSlotInitMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResQuickSlotInitMessage == nullptr)
	{
		return nullptr;
	}

	ResQuickSlotInitMessage->Clear();

	*ResQuickSlotInitMessage << (int16)en_PACKET_S2C_QUICKSLOT_EMPTY;
	*ResQuickSlotInitMessage << QuickSlotBarIndex;
	*ResQuickSlotInitMessage << QuickSlotBarSlotIndex;

	return ResQuickSlotInitMessage;
}

CGameServerMessage* CGameServer::MakePacketResSelectSkillCharacteristic(bool IsSuccess, int8 SkillCharacteristicType, vector<CSkill*> PassiveSkills, vector<CSkill*> ActiveSkills)
{
	CGameServerMessage* ResSelectSkillCharacteristicMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResSelectSkillCharacteristicMessage == nullptr)
	{
		return nullptr;
	}

	ResSelectSkillCharacteristicMessage->Clear();

	*ResSelectSkillCharacteristicMessage << (int16)en_PACKET_S2C_SELECT_SKILL_CHARACTERISTIC;	
	*ResSelectSkillCharacteristicMessage << IsSuccess;	
	*ResSelectSkillCharacteristicMessage << SkillCharacteristicType;

	int8 PassiveSkillCount = (int8)PassiveSkills.size();
	*ResSelectSkillCharacteristicMessage << PassiveSkillCount;

	for (CSkill* PassiveSkill : PassiveSkills)
	{
		*ResSelectSkillCharacteristicMessage << *PassiveSkill->GetSkillInfo();
	}

	int8 ActiveSkillCount = (int8)ActiveSkills.size();
	*ResSelectSkillCharacteristicMessage << ActiveSkillCount;

	for (CSkill* ActiveSkill : ActiveSkills)
	{
		*ResSelectSkillCharacteristicMessage << *ActiveSkill->GetSkillInfo();
	}

	return ResSelectSkillCharacteristicMessage;
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

CGameServerMessage* CGameServer::MakePacketResSkillLearn(bool IsSkillLearn, vector<en_SkillType> LearnSkillTypes, int8 SkillMaxPoint, int8 SkillPoint)
{
	CGameServerMessage* ResSkillLearnMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResSkillLearnMessage == nullptr)
	{
		return nullptr;
	}

	ResSkillLearnMessage->Clear();

	*ResSkillLearnMessage << (int16)en_PACKET_S2C_LEARN_SKILL;
	*ResSkillLearnMessage << IsSkillLearn;

	int8 LearnSkillTypesSize = LearnSkillTypes.size();
	*ResSkillLearnMessage << LearnSkillTypesSize;

	for (en_SkillType LeanSkillType : LearnSkillTypes)
	{
		*ResSkillLearnMessage << (int16)LeanSkillType;
	}

	*ResSkillLearnMessage << SkillMaxPoint;
	*ResSkillLearnMessage << SkillPoint;

	return ResSkillLearnMessage;
}

CGameServerMessage* CGameServer::MakePacketBufDeBuf(int64 TargetObjectId, bool BufDeBuf, st_SkillInfo* SkillInfo)
{
	CGameServerMessage* ResBufDeBufMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResBufDeBufMessage == nullptr)
	{
		return nullptr;
	}

	ResBufDeBufMessage->Clear();

	*ResBufDeBufMessage << (int16)en_PACKET_S2C_BUF_DEBUF;
	*ResBufDeBufMessage << TargetObjectId;
	*ResBufDeBufMessage << BufDeBuf;
	*ResBufDeBufMessage << *SkillInfo;

	return ResBufDeBufMessage;
}

CGameServerMessage* CGameServer::MakePacketBufDeBufOff(int64 TargetObjectId, bool BufDeBuf, en_SkillType OffSkillType)
{
	CGameServerMessage* ResBufDeBufOffMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResBufDeBufOffMessage == nullptr)
	{
		return nullptr;
	}

	ResBufDeBufOffMessage->Clear();

	*ResBufDeBufOffMessage << (int16)en_PACKET_S2C_BUF_DEBUF_OFF;
	*ResBufDeBufOffMessage << TargetObjectId;
	*ResBufDeBufOffMessage << BufDeBuf;
	*ResBufDeBufOffMessage << (int16)OffSkillType;

	return ResBufDeBufOffMessage;
}

CGameServerMessage* CGameServer::MakePacketComboSkillOn(vector<st_QuickSlotBarPosition> ComboSkillQuickSlotPositions, en_SkillType ComboSkilltype)
{
	CGameServerMessage* ResComboSkillMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResComboSkillMessage == nullptr)
	{
		return nullptr;
	}

	ResComboSkillMessage->Clear();

	*ResComboSkillMessage << (int16)en_PACKET_S2C_COMBO_SKILL_ON;

	*ResComboSkillMessage << (int16)ComboSkilltype;

	int8 ComboSkillQuickSlotBarIndexSize = (int8)ComboSkillQuickSlotPositions.size();
	*ResComboSkillMessage << ComboSkillQuickSlotBarIndexSize;

	for (st_QuickSlotBarPosition ComboSkillQuickSlotPosition : ComboSkillQuickSlotPositions)
	{
		*ResComboSkillMessage << ComboSkillQuickSlotPosition.QuickSlotBarIndex;
		*ResComboSkillMessage << ComboSkillQuickSlotPosition.QuickSlotBarSlotIndex;
	}

	return ResComboSkillMessage;
}

CGameServerMessage* CGameServer::MakePacketComboSkillOff(vector<st_QuickSlotBarPosition> ComboSkillQuickSlotPositions, en_SkillType OffComboSkillType, en_SkillType RollBackSkillType)
{
	CGameServerMessage* ResComboSkillOffMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResComboSkillOffMessage == nullptr)
	{
		return nullptr;
	}

	ResComboSkillOffMessage->Clear();

	*ResComboSkillOffMessage << (int16)en_PACKET_S2C_COMBO_SKILL_OFF;

	*ResComboSkillOffMessage << (int8)1;

	*ResComboSkillOffMessage << (int16)OffComboSkillType;	
	*ResComboSkillOffMessage << (int16)RollBackSkillType;	

	int8 ComboSkillQuickSlotBarOffIndexSize = (int8)ComboSkillQuickSlotPositions.size();
	*ResComboSkillOffMessage << ComboSkillQuickSlotBarOffIndexSize;

	for (st_QuickSlotBarPosition ComboSkillQuickSlotPosition : ComboSkillQuickSlotPositions)
	{
		*ResComboSkillOffMessage << ComboSkillQuickSlotPosition.QuickSlotBarIndex;
		*ResComboSkillOffMessage << ComboSkillQuickSlotPosition.QuickSlotBarSlotIndex;
	}	

	return ResComboSkillOffMessage;
}

CGameServerMessage* CGameServer::MakePacketComboSkillOff(en_SkillType OffSkillType, vector<st_QuickSlotOffInfo> ComboSkillQuickSlotPositions)
{
	CGameServerMessage* ResComboSkillOffMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResComboSkillOffMessage == nullptr)
	{
		return nullptr;
	}

	ResComboSkillOffMessage->Clear();

	*ResComboSkillOffMessage << (int16)en_PACKET_S2C_COMBO_SKILL_OFF;

	*ResComboSkillOffMessage << (int8)2;

	*ResComboSkillOffMessage << (int16)OffSkillType;

	if (ComboSkillQuickSlotPositions.size() > 0)
	{
		int8 ComboSkillQuickSlotOffPositionsSize = (int8)ComboSkillQuickSlotPositions.size();
		*ResComboSkillOffMessage << ComboSkillQuickSlotOffPositionsSize;

		for (st_QuickSlotOffInfo ComboSkillQuickSlotOffInfo : ComboSkillQuickSlotPositions)
		{
			*ResComboSkillOffMessage << ComboSkillQuickSlotOffInfo.QuickSlotBarIndex;
			*ResComboSkillOffMessage << ComboSkillQuickSlotOffInfo.QuickSlotBarSlotIndex;
			*ResComboSkillOffMessage << (int16)ComboSkillQuickSlotOffInfo.RollBackSkillType;
		}
	}	

	return ResComboSkillOffMessage;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobComboSkillCreate(en_SkillCharacteristic PreviousSkillCharacteristic, en_SkillType PreviousSkillType, en_SkillCharacteristic ComboSkillCharacteristic, en_SkillType ComboSkillType, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex)
{
	st_GameObjectJob* ComboSkillCreateJob = G_ObjectManager->GameObjectJobCreate();
	ComboSkillCreateJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_COMBO_SKILL_CREATE;

	CGameServerMessage* ComboSkillCreateMessage = CGameServerMessage::GameServerMessageAlloc();
	ComboSkillCreateMessage->Clear();

	*ComboSkillCreateMessage << (int8)PreviousSkillCharacteristic;
	*ComboSkillCreateMessage << (int16)PreviousSkillType;
	*ComboSkillCreateMessage << (int8)ComboSkillCharacteristic;
	*ComboSkillCreateMessage << (int16)ComboSkillType;
	*ComboSkillCreateMessage << QuickSlotBarIndex;
	*ComboSkillCreateMessage << QuickSlotBarSlotIndex;

	ComboSkillCreateJob->GameObjectJobMessage = ComboSkillCreateMessage;

	return ComboSkillCreateJob;
}

CGameServerMessage* CGameServer::MakePacketExperience(int64 GainExp, int64 CurrentExp, int64 RequireExp, int64 TotalExp)
{
	CGameServerMessage* ResExperienceMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResExperienceMessage == nullptr)
	{
		return nullptr;
	}

	ResExperienceMessage->Clear();

	*ResExperienceMessage << (int16)en_PACKET_S2C_EXPERIENCE;		
	*ResExperienceMessage << GainExp;
	*ResExperienceMessage << CurrentExp;
	*ResExperienceMessage << RequireExp;
	*ResExperienceMessage << TotalExp;

	return ResExperienceMessage;
}

CGameServerMessage* CGameServer::MakePacketSkillCastingCancel(int64 PlayerId)
{
	CGameServerMessage* ResSkillCastingCancelMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResSkillCastingCancelMessage == nullptr)
	{
		return nullptr;
	}

	ResSkillCastingCancelMessage->Clear();

	*ResSkillCastingCancelMessage << (int16)en_PACKET_S2C_SKILL_CASTING_CANCEL;	
	*ResSkillCastingCancelMessage << PlayerId;

	return ResSkillCastingCancelMessage;
}

CGameServerMessage* CGameServer::MakePacketGatheringCancel(int64 ObjectID)
{
	CGameServerMessage* ResMagicCancelMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResMagicCancelMessage == nullptr)
	{
		return nullptr;
	}

	ResMagicCancelMessage->Clear();

	*ResMagicCancelMessage << (int16)en_PACKET_S2C_GATHERING_CANCEL;
	*ResMagicCancelMessage << ObjectID;

	return ResMagicCancelMessage;
}

CGameServerMessage* CGameServer::MakePacketCoolTime(vector<st_QuickSlotBarPosition> QuickSlotBarPositions, float SkillCoolTimeSpeed, CSkill* QuickSlotSkill, int32 CoolTime)
{
	CGameServerMessage* ResCoolTimeMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCoolTimeMessage == nullptr)
	{
		return nullptr;
	}

	ResCoolTimeMessage->Clear();

	*ResCoolTimeMessage << (int16)en_PACKET_S2C_COOLTIME_START;

	*ResCoolTimeMessage << SkillCoolTimeSpeed;

	bool EmptySkill;

	if (QuickSlotSkill != nullptr)
	{
		if (QuickSlotSkill->GetSkillInfo())
		{
			EmptySkill = false;

			*ResCoolTimeMessage << EmptySkill;
			*ResCoolTimeMessage << *QuickSlotSkill->GetSkillInfo();
		}
	}
	else
	{
		EmptySkill = true;

		*ResCoolTimeMessage << EmptySkill;
		*ResCoolTimeMessage << CoolTime;
	}

	int8 QuickSlotBarPositionSize = (int8)QuickSlotBarPositions.size();
	*ResCoolTimeMessage << QuickSlotBarPositionSize;

	for (st_QuickSlotBarPosition QuickSlotBarPosition : QuickSlotBarPositions)
	{
		*ResCoolTimeMessage << QuickSlotBarPosition.QuickSlotBarIndex;
		*ResCoolTimeMessage << QuickSlotBarPosition.QuickSlotBarSlotIndex;
	}		

	return ResCoolTimeMessage;
}

CGameServerMessage* CGameServer::MakePacketSkillError(en_GlobalMessageType PersonalMessageType, const WCHAR* SkillName, int16 SkillDistance)
{
	CGameServerMessage* ResErrorMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResErrorMessage == nullptr)
	{
		return nullptr;
	}

	ResErrorMessage->Clear();

	*ResErrorMessage << (int16)en_PACKET_S2C_GLOBAL_MESSAGE;
	*ResErrorMessage << (int8)1;

	WCHAR ErrorMessage[100] = { 0 };

	*ResErrorMessage << (int8)PersonalMessageType;

	switch (PersonalMessageType)
	{
	case en_GlobalMessageType::GLOBAL_MESSAGE_SKILL_COOLTIME:
		wsprintf(ErrorMessage, L"[%s]의 재사용 대기시간이 완료되지 않았습니다.", SkillName);
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_GLOBAL_SKILL_COOLTIME:
		wsprintf(ErrorMessage, L"전역 재사용 대기시간이 완료되지 않았습니다.", SkillName);
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_NON_SELECT_OBJECT:
		wsprintf(ErrorMessage, L"대상을 선택하고 [%s]을/를 사용해야 합니다.", SkillName);
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_HEAL_NON_SELECT_OBJECT:
		wsprintf(ErrorMessage, L"[%s] 대상을 선택하지 않아서 자신에게 사용합니다.", SkillName);
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_PLACE_BLOCK:
		wsprintf(ErrorMessage, L"이동할 위치가 막혀 있어서 [%s]을/를 사용할 수 없습니다.", SkillName);
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_PLACE_DISTANCE:
		wsprintf(ErrorMessage, L"[%s] 대상과의 거리가 너무 멉니다. [거리 : %d ]", SkillName, SkillDistance);
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_MYSELF_TARGET:
		wsprintf(ErrorMessage, L"[%s]을/를 자신에게 사용 할 수 없습니다.", SkillName, SkillDistance);
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_DIR_DIFFERENT:
		wsprintf(ErrorMessage, L"대상을 바라보아야 합니다.");
		break;
	}

	wstring ErrorMessageString = ErrorMessage;

	// 에러 메세지
	int16 ErrorMessageLen = (int16)(ErrorMessageString.length() * 2);
	*ResErrorMessage << ErrorMessageLen;
	ResErrorMessage->InsertData(ErrorMessageString.c_str(), ErrorMessageLen);

	return ResErrorMessage;
}

CGameServerMessage* CGameServer::MakePacketCommonError(en_GlobalMessageType PersonalMessageType, const WCHAR* Name)
{
	CGameServerMessage* ResCommonErrorMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCommonErrorMessage == nullptr)
	{
		return nullptr;
	}

	ResCommonErrorMessage->Clear();

	*ResCommonErrorMessage << (int16)en_PACKET_S2C_GLOBAL_MESSAGE;
	*ResCommonErrorMessage << (int8)1;

	WCHAR ErrorMessage[100] = { 0 };

	*ResCommonErrorMessage << (int8)PersonalMessageType;

	switch (PersonalMessageType)
	{	
	case en_GlobalMessageType::GLOBAL_MESSAGE_STATUS_ABNORMAL:
		wsprintf(ErrorMessage, L"상태이상에 걸려 [%s]를 사용 할 수 없습니다.", Name);
		break;	
	case en_GlobalMessageType::GLOBAL_MESSAGE_SKILL_CANCEL_FAIL_COOLTIME:
		wsprintf(ErrorMessage, L"[%s]이 재사용 대기시간 중이라 취소 할 수 없습니다.", Name);
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_NON_SELECT_OBJECT:
		wsprintf(ErrorMessage, L"대상을 선택하고 사용해야 합니다.");
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_FAR_DISTANCE:
		wsprintf(ErrorMessage, L"대상과의 거리가 너무 멉니다.");
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_ATTACK_ANGLE:
		wsprintf(ErrorMessage, L"무기의 방향을 선택한 대상으로 두어야합니다.");
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_DIR_DIFFERENT:
		wsprintf(ErrorMessage, L"[%s]을 바라보아야 합니다.", Name);
		break;	
	case en_GlobalMessageType::GLOBAL_MESSAGE_GATHERING_DISTANCE:
		wsprintf(ErrorMessage, L"[%s]을 채집하려면 좀 더 가까이 다가가야합니다.", Name);
		break;
	case en_GlobalMessageType::GLOBAL_MEESAGE_CRAFTING_TABLE_OVERLAP_SELECT:
		wsprintf(ErrorMessage, L"[%s]를 사용중입니다.", Name);
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_CRAFTING_TABLE_OVERLAP_CRAFTING_START:
		wsprintf(ErrorMessage, L"제작 중인 아이템을 모두 제작하거나, 제작 멈춤을 누르고 제작을 다시 시작해야 합니다.");
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_CRAFTING_TABLE_MATERIAL_COUNT_NOT_ENOUGH:
		wsprintf(ErrorMessage, L"재료가 부족합니다.");
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_CRAFTING_TABLE_MATERIAL_WRONG_ITEM_ADD:
		wsprintf(ErrorMessage, L"선택된 제작법에 넣을 수 없는 재료입니다.");
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_SEED_FARMING_EXIST:
		wsprintf(ErrorMessage, L"[%s]가 심어져 있어서 심을 수 없습니다.", Name);
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_EXIST_PARTY_PLAYER:
		wsprintf(ErrorMessage, L"[%s]이 그룹 중이라서 초대 할 수 없습니다.", Name);
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_PARTY_INVITE_REJECT:
		wsprintf(ErrorMessage, L"[%s]가 그룹 초대를 거절 했습니다.", Name);
		break;
	case en_GlobalMessageType::GLOBAL_MESSAGE_PARTY_MAX:
		wsprintf(ErrorMessage, L"그룹에 빈 자리가 없습니다.", Name);
		break;
	case en_GlobalMessageType::GLOBAL_FAULT_ITEM_USE:
		wsprintf(ErrorMessage, L"[%s]를 사용 할 수 없습니다.", Name);
		break;			
	}

	wstring ErrorMessageString = ErrorMessage;

	// 에러 메세지
	int16 ErrorMessageLen = (int16)(ErrorMessageString.length() * 2);
	*ResCommonErrorMessage << ErrorMessageLen;
	ResCommonErrorMessage->InsertData(ErrorMessageString.c_str(), ErrorMessageLen);

	return ResCommonErrorMessage;
}

CGameServerMessage* CGameServer::MakePacketStatusAbnormal(int64 TargetId, float PositionX, float PositionY, st_SkillInfo* SatusAbnormalSkillInfo, bool SetStatusAbnormal, int64 StatusAbnormal)
{
	CGameServerMessage* ResStatusAbnormal = CGameServerMessage::GameServerMessageAlloc();
	if (ResStatusAbnormal == nullptr)
	{
		return nullptr;
	}

	ResStatusAbnormal->Clear();

	*ResStatusAbnormal << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_STATUS_ABNORMAL;
	*ResStatusAbnormal << TargetId;
	*ResStatusAbnormal << PositionX;
	*ResStatusAbnormal << PositionY;
	*ResStatusAbnormal << SatusAbnormalSkillInfo;
	*ResStatusAbnormal << SetStatusAbnormal;
	*ResStatusAbnormal << StatusAbnormal;

	return ResStatusAbnormal;
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

CGameServerMessage* CGameServer::MakePacketResItemToInventory(int64 TargetObjectId, st_ItemInfo InventoryItem, bool IsExist, int16 ItemEach, bool ItemGainPrint)
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

	*ResItemToInventoryMessage << false;
	// 인벤토리 아이템 정보 담기
	*ResItemToInventoryMessage << InventoryItem;
	// 아이템 중복 여부
	*ResItemToInventoryMessage << IsExist;
	// 아이템 낱개 개수
	*ResItemToInventoryMessage << ItemEach;
	// 아이템 얻는 UI 출력 할 것인지 말것인지
	*ResItemToInventoryMessage << ItemGainPrint;

	return ResItemToInventoryMessage;
}

CGameServerMessage* CGameServer::MakePacketResMoneyToInventory(int64 TargetObjectID, int64 Coin, st_ItemInfo ItemInfo, int16 ItemEach)
{
	CGameServerMessage* ResMoneyToInventoryMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResMoneyToInventoryMessage == nullptr)
	{
		return nullptr;
	}

	ResMoneyToInventoryMessage->Clear();

	*ResMoneyToInventoryMessage << (int16)en_PACKET_S2C_LOOTING;

	*ResMoneyToInventoryMessage << TargetObjectID;
	*ResMoneyToInventoryMessage << true;
	*ResMoneyToInventoryMessage << Coin;	
	*ResMoneyToInventoryMessage << ItemInfo;
	*ResMoneyToInventoryMessage << ItemEach;
	*ResMoneyToInventoryMessage << true;

	return ResMoneyToInventoryMessage;
}

CGameServerMessage* CGameServer::MakePacketResCraftingTableMaterialItemList(int64 CraftingTableObjectID, en_GameObjectType CraftingTableObjectType, en_SmallItemCategory SelectCompleteItemType, map<en_SmallItemCategory, CItem*> MaterialItems)
{
	CGameServerMessage* ResCraftingTableMaterialItemListMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCraftingTableMaterialItemListMessage == nullptr)
	{
		return nullptr;
	}

	ResCraftingTableMaterialItemListMessage->Clear();

	*ResCraftingTableMaterialItemListMessage << (int16)en_PACKET_S2C_CRAFTING_TABLE_MATERIAL_ITEM_LIST;
	*ResCraftingTableMaterialItemListMessage << CraftingTableObjectID;
	*ResCraftingTableMaterialItemListMessage << (int16)CraftingTableObjectType;
	*ResCraftingTableMaterialItemListMessage << (int16)SelectCompleteItemType;

	int16 MaterialItemCount = (int16)MaterialItems.size();
	*ResCraftingTableMaterialItemListMessage << MaterialItemCount;

	for (auto MaterialItemIter : MaterialItems)
	{
		*ResCraftingTableMaterialItemListMessage << MaterialItemIter.second->_ItemInfo;
	}

	return ResCraftingTableMaterialItemListMessage;
}

CGameServerMessage* CGameServer::MakePacketResCraftingTableInput(int64 CraftingTableObjectID, map<en_SmallItemCategory, CItem*> MaterialItems)
{
	CGameServerMessage* ResCraftingTableMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCraftingTableMessage == nullptr)
	{
		return nullptr;
	}

	ResCraftingTableMessage->Clear();

	*ResCraftingTableMessage << (int16)en_PACKET_S2C_CRAFTING_TABLE_ITEM_ADD;

	*ResCraftingTableMessage << CraftingTableObjectID;

	int16 MaterialItemCount = (int16)MaterialItems.size();
	*ResCraftingTableMessage << MaterialItemCount;

	for (auto MaterialItemIter : MaterialItems)
	{
		*ResCraftingTableMessage << MaterialItemIter.second->_ItemInfo;
	}

	return ResCraftingTableMessage;
}

CGameServerMessage* CGameServer::MakePacketResCraftingStart(int64 CraftingTableObjectID, st_ItemInfo CraftingItemInfo)
{
	CGameServerMessage* ResCraftingStartMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCraftingStartMessage == nullptr)
	{
		return nullptr;
	}

	ResCraftingStartMessage->Clear();

	*ResCraftingStartMessage << (int16)en_PACKET_S2C_CRAFTING_TABLE_CRAFTING_START;
	*ResCraftingStartMessage << CraftingTableObjectID;
	*ResCraftingStartMessage << CraftingItemInfo;

	return ResCraftingStartMessage;
}

CGameServerMessage* CGameServer::MakePacketResCraftingStop(int64 CraftingTableObjectID, st_ItemInfo CraftingStopItemInfo)
{
	CGameServerMessage* ResCraftingStartMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCraftingStartMessage == nullptr)
	{
		return nullptr;
	}

	ResCraftingStartMessage->Clear();

	*ResCraftingStartMessage << (int16)en_PACKET_S2C_CRAFTING_TABLE_CRAFTING_STOP;
	*ResCraftingStartMessage << CraftingTableObjectID;
	*ResCraftingStartMessage << CraftingStopItemInfo;

	return ResCraftingStartMessage;
}

CGameServerMessage* CGameServer::MakePacketResCraftingTableCraftRemainTime(int64 CraftingTableObjectID, st_ItemInfo CraftingItemInfo)
{
	CGameServerMessage* ResCraftingTableSelectMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCraftingTableSelectMessage == nullptr)
	{
		return nullptr;
	}

	ResCraftingTableSelectMessage->Clear();

	*ResCraftingTableSelectMessage << (int16)en_PACKET_S2C_CRAFTING_TABLE_CRAFT_REMAIN_TIME;
	*ResCraftingTableSelectMessage << CraftingTableObjectID;
	*ResCraftingTableSelectMessage << CraftingItemInfo;

	return ResCraftingTableSelectMessage;
}

CGameServerMessage* CGameServer::MakePacketResCraftingTableCompleteItemList(int64 CraftingTableObjectID, en_GameObjectType CraftingTableObjectType, map<en_SmallItemCategory, CItem*> CompleteItems)
{
	CGameServerMessage* ResCraftingTableCompleteItemListMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCraftingTableCompleteItemListMessage == nullptr)
	{
		return nullptr;
	}

	ResCraftingTableCompleteItemListMessage->Clear();

	*ResCraftingTableCompleteItemListMessage << (int16)en_PACKET_S2C_CRAFTING_TABLE_COMPLETE_ITEM_LIST;
	*ResCraftingTableCompleteItemListMessage << CraftingTableObjectID;
	*ResCraftingTableCompleteItemListMessage << (int16)CraftingTableObjectType;	

	int16 CompleteItemCount = (int16)CompleteItems.size();
	*ResCraftingTableCompleteItemListMessage << CompleteItemCount;

	for (auto CompleteItemIter : CompleteItems)
	{
		*ResCraftingTableCompleteItemListMessage << CompleteItemIter.second->_ItemInfo;
	}

	return ResCraftingTableCompleteItemListMessage;
}

CGameServerMessage* CGameServer::MakePacketOnEquipment(int64 PlayerId, st_ItemInfo& EquipmentItemInfo)
{
	CGameServerMessage* ResOnEquipmentMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResOnEquipmentMessage == nullptr)
	{
		return nullptr;
	}

	ResOnEquipmentMessage->Clear();

	*ResOnEquipmentMessage << (int16)en_PACKET_S2C_ON_EQUIPMENT;
	*ResOnEquipmentMessage << PlayerId;	
	*ResOnEquipmentMessage << EquipmentItemInfo;

	return ResOnEquipmentMessage;
}

CGameServerMessage* CGameServer::MakePacketOffEquipment(int64 PlayerID, en_EquipmentParts OffEquipmentParts)
{
	CGameServerMessage* ResOffEquipmentMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResOffEquipmentMessage == nullptr)
	{
		return nullptr;
	}

	ResOffEquipmentMessage->Clear();

	*ResOffEquipmentMessage << (int16)en_PACKET_S2C_OFF_EQUIPMENT;
	*ResOffEquipmentMessage << PlayerID;
	*ResOffEquipmentMessage << (int8)OffEquipmentParts;

	return ResOffEquipmentMessage;
}

CGameServerMessage* CGameServer::MakePacketSeedFarming(st_ItemInfo SeedItem, int64 SeedObjectID)
{
	CGameServerMessage* ResSeedFarmingMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResSeedFarmingMessage == nullptr)
	{
		return nullptr;
	}

	ResSeedFarmingMessage->Clear();

	*ResSeedFarmingMessage << (int16)en_PACKET_S2C_SEED_FARMING;
	*ResSeedFarmingMessage << SeedItem;
	*ResSeedFarmingMessage << SeedObjectID;

	return ResSeedFarmingMessage;
}

CGameServerMessage* CGameServer::MakePacketPlantGrowthStep(int64 PlantObjectID, int8 PlantGrowthStep, float PlantGrowthRatio)
{
	CGameServerMessage* ResPlantGrowthStepMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResPlantGrowthStepMessage == nullptr)
	{
		return nullptr;
	}

	ResPlantGrowthStepMessage->Clear();

	*ResPlantGrowthStepMessage << (int16)en_PACKET_S2C_PLANT_GROWTH_CHECK;
	*ResPlantGrowthStepMessage << PlantObjectID;
	*ResPlantGrowthStepMessage << PlantGrowthStep;
	*ResPlantGrowthStepMessage << PlantGrowthRatio;

	return ResPlantGrowthStepMessage;
}

CGameServerMessage* CGameServer::MakePacketResPartyInvite(int64 ReqPartyPlayerObjectID, wstring ReqPartyPlayerName)
{
	CGameServerMessage* ResPartyInviteMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResPartyInviteMessage == nullptr)
	{
		return nullptr;
	}

	ResPartyInviteMessage->Clear();

	*ResPartyInviteMessage << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_PARTY_INVITE;
	*ResPartyInviteMessage << ReqPartyPlayerObjectID;

	int16 GameObjectInfoNameLen = (int16)ReqPartyPlayerName.length() * 2;
	*ResPartyInviteMessage << GameObjectInfoNameLen;
	ResPartyInviteMessage->InsertData(ReqPartyPlayerName.c_str(), GameObjectInfoNameLen);

	return ResPartyInviteMessage;	
}

CGameServerMessage* CGameServer::MakePacketResPartyAccept(vector<CPlayer*> PartyPlayerInfos)
{
	CGameServerMessage* ResPartyAcceptMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResPartyAcceptMessage == nullptr)
	{
		return nullptr;
	}

	ResPartyAcceptMessage->Clear();

	*ResPartyAcceptMessage << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_PARTY_INVITE_ACCEPT;

	int8 PartyPlayerInfosSize = (int8)PartyPlayerInfos.size();
	*ResPartyAcceptMessage << PartyPlayerInfosSize;

	for (CPlayer* PartyPlayerInfo : PartyPlayerInfos)
	{
		*ResPartyAcceptMessage << PartyPlayerInfo->_GameObjectInfo;
	}

	return ResPartyAcceptMessage;
}

CGameServerMessage* CGameServer::MakePacketResPartyReject(wstring PartyInviteRejectName)
{
	CGameServerMessage* ResPartyRejectMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResPartyRejectMessage == nullptr)
	{
		return nullptr;
	}

	ResPartyRejectMessage->Clear();

	*ResPartyRejectMessage << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_PARTY_INVITE_REJECT;

	int16 GameObjectInfoNameLen = (int16)PartyInviteRejectName.length() * 2;
	*ResPartyRejectMessage << GameObjectInfoNameLen;
	ResPartyRejectMessage->InsertData(PartyInviteRejectName.c_str(), GameObjectInfoNameLen);

	return ResPartyRejectMessage;
}

CGameServerMessage* CGameServer::MakePacketResPartyQuit(bool IsAllQuit, int64 QuitPartyPlayerID)
{
	CGameServerMessage* ResPartyInviteMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResPartyInviteMessage == nullptr)
	{
		return nullptr;
	}

	ResPartyInviteMessage->Clear();

	*ResPartyInviteMessage << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_PARTY_QUIT;
	*ResPartyInviteMessage << IsAllQuit;
	*ResPartyInviteMessage << QuitPartyPlayerID;

	return ResPartyInviteMessage;
}

CGameServerMessage* CGameServer::MakePacketResPartyBanish(int64 BanishPlayerID)
{
	CGameServerMessage* ResBanishMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResBanishMessage == nullptr)
	{
		return nullptr;
	}

	ResBanishMessage->Clear();

	*ResBanishMessage << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_PARTY_BANISH;
	*ResBanishMessage << BanishPlayerID;	

	return ResBanishMessage;
}

CGameServerMessage* CGameServer::MakePacketResPartyLeaderMandate(int64 PerviousPartyLeaderObjectID, int64 NewPartyLeaderObjectID)
{
	CGameServerMessage* ResPartyLeaderMandateMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResPartyLeaderMandateMessage == nullptr)
	{
		return nullptr;
	}

	ResPartyLeaderMandateMessage->Clear();

	*ResPartyLeaderMandateMessage << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_PARTY_LEADER_MANDATE;
	*ResPartyLeaderMandateMessage << PerviousPartyLeaderObjectID;
	*ResPartyLeaderMandateMessage << NewPartyLeaderObjectID;

	return ResPartyLeaderMandateMessage;
}

CGameServerMessage* CGameServer::MakePacketResMenu(int8 MenuType)
{
	CGameServerMessage* ResMenuMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResMenuMessage == nullptr)
	{
		return nullptr;
	}

	ResMenuMessage->Clear();

	*ResMenuMessage << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_MENU;
	*ResMenuMessage << MenuType;	

	return ResMenuMessage;
}

CGameServerMessage* CGameServer::MakePacketResInteractionRooting(int64 RootingObjectID, en_InteractionType InteractionType, vector<st_ItemInfo> RootingItems, int16 Coin)
{
	CGameServerMessage* ResInteractionRootingMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResInteractionRootingMessage == nullptr)
	{
		return nullptr;
	}

	ResInteractionRootingMessage->Clear();

	*ResInteractionRootingMessage << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_INTERACTION;
	*ResInteractionRootingMessage << RootingObjectID;

	*ResInteractionRootingMessage << (int8)InteractionType;

	int8 RootingItemsCount = (int8)RootingItems.size();
	*ResInteractionRootingMessage << RootingItemsCount;
	
	for (st_ItemInfo RootingItem : RootingItems)
	{
		*ResInteractionRootingMessage << RootingItem;
	}

	*ResInteractionRootingMessage << Coin;

	return ResInteractionRootingMessage;
}

CGameServerMessage* CGameServer::MakePacketResRayCasting(int64 ObjectID, vector<st_RayCatingPosition>& RayCastingPositions)
{
	CGameServerMessage* ResRayCastingMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResRayCastingMessage == nullptr)
	{
		return nullptr;
	}

	ResRayCastingMessage->Clear();

	*ResRayCastingMessage << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_RAY_CASTING;
	*ResRayCastingMessage << ObjectID;

	int8 RayCastingPositionSize = (int8)RayCastingPositions.size();
	*ResRayCastingMessage << RayCastingPositionSize;

	for (auto RayCastingPosition : RayCastingPositions)
	{
		*ResRayCastingMessage << RayCastingPosition.StartPosition.X;
		*ResRayCastingMessage << RayCastingPosition.StartPosition.Y;
		*ResRayCastingMessage << RayCastingPosition.EndPosition.X;
		*ResRayCastingMessage << RayCastingPosition.EndPosition.Y;
	}

	return ResRayCastingMessage;
}

CGameServerMessage* CGameServer::MakePacketResTileInfo(vector<st_TileInfo> Tiles)
{
	CGameServerMessage* ResTileInfoMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResTileInfoMessage == nullptr)
	{
		return nullptr;
	}

	ResTileInfoMessage->Clear();

	*ResTileInfoMessage << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_MAP_TILE_INFO;

	int8 TileCount = (int8)Tiles.size();
	*ResTileInfoMessage << TileCount;
	
	for (auto Tile : Tiles)
	{
		*ResTileInfoMessage << Tile;
	}	

	return ResTileInfoMessage;
}

CGameServerMessage* CGameServer::MakePacketReqCancel(en_GAME_SERVER_PACKET_TYPE PacketType)
{
	CGameServerMessage* ResPlantGrowthStepMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResPlantGrowthStepMessage == nullptr)
	{
		return nullptr;
	}

	ResPlantGrowthStepMessage->Clear();

	*ResPlantGrowthStepMessage << (int16)PacketType;
	*ResPlantGrowthStepMessage << false;

	return ResPlantGrowthStepMessage;
}

CGameServerMessage* CGameServer::MakePacketLoginServerLogOut(int64 AccountID)
{
	CGameServerMessage* ResLogOutMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResLogOutMessage == nullptr)
	{
		return nullptr;
	}

	ResLogOutMessage->Clear();

	*ResLogOutMessage << (int16)en_LOGIN_SERVER_PACKET_TYPE::en_LOGIN_SERVER_C2S_ACCOUNT_LOGOUT;
	*ResLogOutMessage << AccountID;

	return ResLogOutMessage;
}

CGameServerMessage* CGameServer::MakePacketLoginServerLoginStateChange(int64 AccountID, en_LoginState ChangeLoginState)
{
	CGameServerMessage* ResLogInStateChange = CGameServerMessage::GameServerMessageAlloc();
	if (ResLogInStateChange == nullptr)
	{
		return nullptr;
	}

	ResLogInStateChange->Clear();

	*ResLogInStateChange << (int16)en_LOGIN_SERVER_PACKET_TYPE::en_LOGIN_SERVER_C2S_LOGIN_STATE_CHANGE;
	*ResLogInStateChange << AccountID;
	*ResLogInStateChange << (int8)ChangeLoginState;

	return ResLogInStateChange;
}

void CGameServer::OnClientJoin(int64 SessionID)
{
	CreateNewClient(SessionID);
}

void CGameServer::OnRecv(int64 SessionID, CMessage* Packet)
{
	PacketProc(SessionID, Packet);
}

void CGameServer::OnClientLeave(st_Session* LeaveSession)
{
	DeleteClient(LeaveSession);
}

bool CGameServer::OnConnectionRequest(const wchar_t ClientIP, int32 Port)
{
	return false;
}

void CGameServer::SendPacketFieldOfView(vector<st_FieldOfViewInfo> FieldOfViewObject, CMessage* Message)
{
	for (st_FieldOfViewInfo ObjectInfo : FieldOfViewObject)
	{
		SendPacket(ObjectInfo.SessionID, Message);		
	}	
}

void CGameServer::SendPacketFieldOfView(CGameObject* Object, CMessage* Message)
{
	// 섹터 얻어오기
	vector<CSector*> AroundSectors = Object->GetChannel()->GetMap()->GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

	for (CSector* AroundSector : AroundSectors)
	{
		for (CPlayer* Player : AroundSector->GetPlayers())
		{
			int16 Distance = Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

			if (Distance <= Object->_FieldOfViewDistance)
			{
				SendPacket(Player->_SessionId, Message);
			}
		}
	}
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