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
#include "Skill.h"
#include "MapManager.h"
#include "QuickSlotBar.h"
#include "Furnace.h"
#include "Sawmill.h"
#include "Day.h"
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

	int64 MapID = 1;

	_Day = new CDay();

	// 맵 정보를 읽어옴
	G_MapManager->MapSave();

	// 맵 오브젝트 스폰
	G_ObjectManager->MapObjectSpawn(MapID);

	// 맵 타일 정보 가져오기
	G_ObjectManager->GameServer = this;

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
	// 클래스 공격 스킬 생성
	switch (NewCharacterInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	{
		for (auto PublicAttackSkillDataIter : G_Datamanager->_PublicAttackSkillDatas)
		{
			st_AttackSkillInfo* PublicAttackSkillData = PublicAttackSkillDataIter.second;

			if (PublicAttackSkillData != nullptr)
			{
				// 일반 공격 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = PublicAttackSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = PublicAttackSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = PublicAttackSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 AttackSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 AttackSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 AttackSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(AttackSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(AttackSkillMediumCategory);
				SkillToSkillBox.InSkillType(AttackSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}

		for (auto PublicBufSkillDataIter : G_Datamanager->_PublicBufSkillDatas)
		{
			st_BufSkillInfo* PublicBufSkillData = PublicBufSkillDataIter.second;

			if (PublicBufSkillData != nullptr)
			{
				// 일반 공격 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = PublicBufSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = PublicBufSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = PublicBufSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 BufSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 BufSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 BufSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(BufSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(BufSkillMediumCategory);
				SkillToSkillBox.InSkillType(BufSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}

		for (auto WarriorAttackSkillDataIter : G_Datamanager->_WarriorAttackSkillDatas)
		{
			st_AttackSkillInfo* WarriorAttackSkillData = WarriorAttackSkillDataIter.second;

			if (WarriorAttackSkillData != nullptr)
			{
				// 전사 공격 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = WarriorAttackSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = WarriorAttackSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = WarriorAttackSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 AttackSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 AttackSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 AttackSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(AttackSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(AttackSkillMediumCategory);
				SkillToSkillBox.InSkillType(AttackSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}

		for (auto WarriorTacTicSkillDataIter : G_Datamanager->_WarriorTacTicSkillDatas)
		{
			st_TacTicSkillInfo* WarriorTacTicSkillData = WarriorTacTicSkillDataIter.second;

			if (WarriorTacTicSkillData != nullptr)
			{
				// 전사 전술 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = WarriorTacTicSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = WarriorTacTicSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = WarriorTacTicSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 TacTicSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 TacTicSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 TacTicSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(TacTicSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(TacTicSkillMediumCategory);
				SkillToSkillBox.InSkillType(TacTicSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}

		for (auto WarriorBufSkillDataIter : G_Datamanager->_WarriorBufSkillDatas)
		{
			st_BufSkillInfo* WarriorBufSkillData = WarriorBufSkillDataIter.second;

			if (WarriorBufSkillData != nullptr)
			{
				// 전사 버프 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = WarriorBufSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = WarriorBufSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = WarriorBufSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 BufSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 BufSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 BufSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(BufSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(BufSkillMediumCategory);
				SkillToSkillBox.InSkillType(BufSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}
	}
	break;
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	{
		for (auto PublicAttackSkillDataIter : G_Datamanager->_PublicAttackSkillDatas)
		{
			st_AttackSkillInfo* PublicAttackSkillData = PublicAttackSkillDataIter.second;

			if (PublicAttackSkillData != nullptr)
			{
				// 일반 공격 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = PublicAttackSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = PublicAttackSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = PublicAttackSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 AttackSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 AttackSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 AttackSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(AttackSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(AttackSkillMediumCategory);
				SkillToSkillBox.InSkillType(AttackSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}

		for (auto PublicBufSkillDataIter : G_Datamanager->_PublicBufSkillDatas)
		{
			st_BufSkillInfo* PublicBufSkillData = PublicBufSkillDataIter.second;

			if (PublicBufSkillData != nullptr)
			{
				// 일반 공격 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = PublicBufSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = PublicBufSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = PublicBufSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 BufSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 BufSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 BufSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(BufSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(BufSkillMediumCategory);
				SkillToSkillBox.InSkillType(BufSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}

		for (auto ShamanAttackSkillDataIter : G_Datamanager->_ShamanAttackSkillDatas)
		{
			st_AttackSkillInfo* ShamanAttackSkillData = ShamanAttackSkillDataIter.second;

			if (ShamanAttackSkillData != nullptr)
			{
				// 주술사 공격 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = ShamanAttackSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = ShamanAttackSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = ShamanAttackSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 AttackSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 AttackSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 AttackSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(AttackSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(AttackSkillMediumCategory);
				SkillToSkillBox.InSkillType(AttackSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}

		for (auto ShamanTacTicSkillDataIter : G_Datamanager->_ShamanTacTicSkillDatas)
		{
			st_TacTicSkillInfo* ShamanTacTicSkillData = ShamanTacTicSkillDataIter.second;

			if (ShamanTacTicSkillData != nullptr)
			{
				// 주술사 전술 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = ShamanTacTicSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = ShamanTacTicSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = ShamanTacTicSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 TacTicSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 TacTicSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 TacTicSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(TacTicSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(TacTicSkillMediumCategory);
				SkillToSkillBox.InSkillType(TacTicSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}

		for (auto ShamanBufSkillDataIter : G_Datamanager->_ShamanBufSkillDatas)
		{
			st_BufSkillInfo* ShamanBufSkillData = ShamanBufSkillDataIter.second;

			if (ShamanBufSkillData != nullptr)
			{
				// 주술사 버프 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = ShamanBufSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = ShamanBufSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = ShamanBufSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 BufSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 BufSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 BufSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(BufSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(BufSkillMediumCategory);
				SkillToSkillBox.InSkillType(BufSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}
	}
	break;
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	{
		for (auto PublicAttackSkillDataIter : G_Datamanager->_PublicAttackSkillDatas)
		{
			st_AttackSkillInfo* PublicAttackSkillData = PublicAttackSkillDataIter.second;

			if (PublicAttackSkillData != nullptr)
			{
				// 일반 공격 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = PublicAttackSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = PublicAttackSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = PublicAttackSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 AttackSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 AttackSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 AttackSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(AttackSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(AttackSkillMediumCategory);
				SkillToSkillBox.InSkillType(AttackSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}

		for (auto PublicBufSkillDataIter : G_Datamanager->_PublicBufSkillDatas)
		{
			st_BufSkillInfo* PublicBufSkillData = PublicBufSkillDataIter.second;

			if (PublicBufSkillData != nullptr)
			{
				// 일반 공격 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = PublicBufSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = PublicBufSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = PublicBufSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 BufSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 BufSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 BufSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(BufSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(BufSkillMediumCategory);
				SkillToSkillBox.InSkillType(BufSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}

		for (auto TaioistAttackSkillDataIter : G_Datamanager->_TaioistAttackSkillDatas)
		{
			st_AttackSkillInfo* TaioistAttackSkillData = TaioistAttackSkillDataIter.second;

			if (TaioistAttackSkillData != nullptr)
			{
				// 도사 공격 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = TaioistAttackSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = TaioistAttackSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = TaioistAttackSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 AttackSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 AttackSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 AttackSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(AttackSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(AttackSkillMediumCategory);
				SkillToSkillBox.InSkillType(AttackSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}

		for (auto TaioistTacTicSkillDataIter : G_Datamanager->_TaioistTacTicSkillDatas)
		{
			st_TacTicSkillInfo* TaioistTacTicSkillData = TaioistTacTicSkillDataIter.second;

			if (TaioistTacTicSkillData != nullptr)
			{
				// 도사 전술 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = TaioistTacTicSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = TaioistTacTicSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = TaioistTacTicSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 TacTicSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 TacTicSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 TacTicSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(TacTicSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(TacTicSkillMediumCategory);
				SkillToSkillBox.InSkillType(TacTicSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}

		for (auto TacTicBufSkillDataIter : G_Datamanager->_TaioistBufSkillDatas)
		{
			st_BufSkillInfo* TaioistBufSkillData = TacTicBufSkillDataIter.second;

			if (TaioistBufSkillData != nullptr)
			{
				// 주술사 버프 스킬 생성 후 DB 저장
				CDBConnection* NewSkillCreateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerSkillToSkillBox SkillToSkillBox(*NewSkillCreateDBConnection);

				st_SkillInfo NewSkillInfo;
				NewSkillInfo.IsSkillLearn = false;
				NewSkillInfo.IsQuickSlotUse = false;
				NewSkillInfo.SkillLargeCategory = TaioistBufSkillData->SkillLargeCategory;
				NewSkillInfo.SkillMediumCategory = TaioistBufSkillData->SkillMediumCategory;
				NewSkillInfo.SkillType = TaioistBufSkillData->SkillType;
				NewSkillInfo.SkillLevel = 1;

				int8 BufSkillLargeCategory = (int8)NewSkillInfo.SkillLargeCategory;
				int8 BufSkillMediumCategory = (int8)NewSkillInfo.SkillMediumCategory;
				int16 BufSkillType = (int16)NewSkillInfo.SkillType;

				SkillToSkillBox.InAccountDBId(AccountId);
				SkillToSkillBox.InPlayerDBId(NewCharacterInfo.ObjectId);
				SkillToSkillBox.InIsSkillLearn(NewSkillInfo.IsSkillLearn);
				SkillToSkillBox.InItemIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
				SkillToSkillBox.InSkillLargeCategory(BufSkillLargeCategory);
				SkillToSkillBox.InSkillMediumCategory(BufSkillMediumCategory);
				SkillToSkillBox.InSkillType(BufSkillType);
				SkillToSkillBox.InSkillLevel(NewSkillInfo.SkillLevel);

				SkillToSkillBox.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, NewSkillCreateDBConnection);
			}
		}
	}
	break;
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	{

	}
	break;
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	{

	}
	break;
	default:
		break;
	}

	// 기본 공격 스킬 퀵슬롯 1번에 등록
	st_QuickSlotBarSlotInfo DefaultAttackSkillQuickSlotInfo;
	DefaultAttackSkillQuickSlotInfo.AccountDBId = AccountId;
	DefaultAttackSkillQuickSlotInfo.PlayerDBId = NewCharacterInfo.ObjectId;
	DefaultAttackSkillQuickSlotInfo.QuickSlotBarIndex = 0;
	DefaultAttackSkillQuickSlotInfo.QuickSlotBarSlotIndex = 0;
	DefaultAttackSkillQuickSlotInfo.QuickSlotKey = 49;

	int8 DefaultSkillLargeCategory = (int8)en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
	int8 DefaultSkillMediumCategory = (int8)en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK;
	int16 DefaultSkillType = (int16)en_SkillType::SKILL_DEFAULT_ATTACK;
	int8 DefaultAttackSkillLevel = 1;
	int8 ItemLargeCategory = 0;
	int8 ItemMediumCategory = 0;
	int16 ItemSmallCategory = 0;
	int16 ItemCount = 0;

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
	QuickSlotUpdate.InSkillLevel(DefaultAttackSkillLevel);
	QuickSlotUpdate.InSkillLevel(DefaultAttackSkillLevel);
	QuickSlotUpdate.InSkillLevel(DefaultAttackSkillLevel);
	QuickSlotUpdate.InItemLargeCategory(ItemLargeCategory);
	QuickSlotUpdate.InItemMediumCategory(ItemMediumCategory);
	QuickSlotUpdate.InItemSmallCategory(ItemSmallCategory);
	QuickSlotUpdate.InItemCount(ItemCount);

	QuickSlotUpdate.Execute();
		
	SP::CDBGameServerInitEquipment EquipmentInit(*DBQuickSlotSaveConnection);

	for (int8 EquipmentPart = (int8)en_EquipmentParts::EQUIPMENT_PARTS_HEAD;
		EquipmentPart <= (int8)en_EquipmentParts::EQUIPMENT_PARTS_BOOT; EquipmentPart++)
	{
		EquipmentInit.InAccountDBID(AccountId);
		EquipmentInit.InPlayerDBID(NewCharacterInfo.ObjectId);
		EquipmentInit.InEquipmentParts(EquipmentPart);

		EquipmentInit.Execute();
	}	

	// 기본 아이템 지급
	SP::CDBGameServerInventoryPlace LeavePlayerInventoryItemSave(*DBQuickSlotSaveConnection);
	LeavePlayerInventoryItemSave.InOwnerAccountId(AccountId);
	LeavePlayerInventoryItemSave.InOwnerPlayerId(NewCharacterInfo.ObjectId);

	vector<st_ItemInfo> NewPlayerDefaultItems;	

	st_ItemInfo WoodSword = *(G_Datamanager->FindItemData(en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD));
	WoodSword.ItemTileGridPositionX = 0;
	WoodSword.ItemTileGridPositionY = 0;
	WoodSword.ItemCount = 1;
	WoodSword.ItemCurrentDurability = WoodSword.ItemMaxDurability;

	st_ItemInfo WoodShield = *(G_Datamanager->FindItemData(en_SmallItemCategory::ITEM_SAMLL_CATEGORY_WEAPON_WOOD_SHIELD));	
	WoodShield.ItemTileGridPositionX = 1;
	WoodShield.ItemTileGridPositionY = 0;
	WoodShield.ItemCount = 1;
	WoodShield.ItemCurrentDurability = WoodSword.ItemMaxDurability;

	st_ItemInfo LeatherHelmet = *(G_Datamanager->FindItemData(en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER));
	LeatherHelmet.ItemTileGridPositionX = 3;
	LeatherHelmet.ItemTileGridPositionY = 0;
	LeatherHelmet.ItemCount = 1;
	LeatherHelmet.ItemCurrentDurability = LeatherHelmet.ItemMaxDurability;

	st_ItemInfo LeatherArmor = *(G_Datamanager->FindItemData(en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER));
	LeatherArmor.ItemTileGridPositionX = 5;
	LeatherArmor.ItemTileGridPositionY = 0;
	LeatherArmor.ItemCount = 1;
	LeatherArmor.ItemCurrentDurability = LeatherArmor.ItemMaxDurability;

	st_ItemInfo LeatherBoot = *(G_Datamanager->FindItemData(en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER));
	LeatherBoot.ItemTileGridPositionX = 7;
	LeatherBoot.ItemTileGridPositionY = 0;
	LeatherBoot.ItemCount = 1;
	LeatherBoot.ItemCurrentDurability = LeatherBoot.ItemMaxDurability;

	st_ItemInfo SmallHPPotion = *(G_Datamanager->FindItemData(en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEALTH_RESTORATION_POTION_SMALL));
	SmallHPPotion.ItemTileGridPositionX = 0;
	SmallHPPotion.ItemTileGridPositionY = 2;
	SmallHPPotion.ItemCount = 5;

	st_ItemInfo SmallMPPotion = *(G_Datamanager->FindItemData(en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_MANA_RESTORATION_POTION_SMALL));
	SmallMPPotion.ItemTileGridPositionX = 1;
	SmallMPPotion.ItemTileGridPositionY = 2;
	SmallMPPotion.ItemCount = 5;

	NewPlayerDefaultItems.push_back(WoodSword);
	NewPlayerDefaultItems.push_back(WoodShield);
	NewPlayerDefaultItems.push_back(LeatherHelmet);
	NewPlayerDefaultItems.push_back(LeatherArmor);
	NewPlayerDefaultItems.push_back(LeatherBoot);
	NewPlayerDefaultItems.push_back(SmallHPPotion);
	NewPlayerDefaultItems.push_back(SmallMPPotion);	

	for (st_ItemInfo InventoryItem : NewPlayerDefaultItems)
	{
		int8 InventoryItemLargeCategory = (int8)InventoryItem.ItemLargeCategory;
		int8 InventoryItemMediumCategory = (int8)InventoryItem.ItemMediumCategory;
		int16 InventoryItemSmallCategory = (int16)InventoryItem.ItemSmallCategory;

		LeavePlayerInventoryItemSave.InItemIsQuickSlotUse(InventoryItem.ItemIsQuickSlotUse);
		LeavePlayerInventoryItemSave.InIsEquipped(InventoryItem.ItemIsEquipped);
		LeavePlayerInventoryItemSave.InItemWidth(InventoryItem.ItemWidth);
		LeavePlayerInventoryItemSave.InItemHeight(InventoryItem.ItemHeight);
		LeavePlayerInventoryItemSave.InItemTileGridPositionX(InventoryItem.ItemTileGridPositionX);
		LeavePlayerInventoryItemSave.InItemTileGridPositionY(InventoryItem.ItemTileGridPositionY);
		LeavePlayerInventoryItemSave.InItemLargeCategory(InventoryItemLargeCategory);
		LeavePlayerInventoryItemSave.InItemMediumCategory(InventoryItemMediumCategory);
		LeavePlayerInventoryItemSave.InItemSmallCategory(InventoryItemSmallCategory);		
		LeavePlayerInventoryItemSave.InItemCount(InventoryItem.ItemCount);		
		LeavePlayerInventoryItemSave.InItemDurability(InventoryItem.ItemMaxDurability);
		LeavePlayerInventoryItemSave.InItemEnchantPoint(InventoryItem.ItemEnchantPoint);

		LeavePlayerInventoryItemSave.Execute();
	}	

	G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotSaveConnection);
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

		ReturnSession(Session);
	}
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
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MOVE:
		PacketProcReqMove(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MOVE_STOP:
		PacketProcReqMoveStop(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_ATTACK:
		PacketProcReqMelee(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MAGIC:
		PacketProcReqMagic(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MAGIC_CANCEL:
		PacketProcReqMagicCancel(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_GATHERING_CANCEL:
		PacketProcReqGatheringCancel(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_LEFT_MOUSE_OBJECT_INFO:
		PacketProcReqLeftMouseObjectInfo(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_LEFT_MOUSE_UI_OBJECT_INFO:
		PacketProcReqLeftMouseUIObjectInfo(SessionID, Message);
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
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_OBJECT_STATE_CHANGE:
		PacketProcReqObjectStateChange(SessionID, Message);
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
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_UI_MENU_TILE_BUY:
		PacketProcReqUIMenuTileBuy(SessionID, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_TILE_BUY:
		PacketProcReqTileBuy(SessionID, Message);
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
		// 클라에서 선택한 플레이어 오브젝트 타입
		int16 ReqGameObjectType;
		*Message >> ReqGameObjectType;

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

		st_GameServerJob* DBCharacterCreateJob = _GameServerJobMemoryPool->Alloc();
		DBCharacterCreateJob->Type = en_GameServerJobType::DATA_BASE_CHARACTER_CHECK;

		CGameServerMessage* DBReqChatacerCreateMessage = CGameServerMessage::GameServerMessageAlloc();
		DBReqChatacerCreateMessage->Clear();

		*DBReqChatacerCreateMessage << Session->SessionId;
		*DBReqChatacerCreateMessage << ReqGameObjectType;
		*DBReqChatacerCreateMessage << CharacterCreateSlotIndex;

		DBCharacterCreateJob->Message = DBReqChatacerCreateMessage;		

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
				CMessage* ResEnterGamePacket = MakePacketResEnterGame(FindName, nullptr, nullptr);
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

			if (MyPlayer->_StatusAbnormal & STATUS_ABNORMAL_WARRIOR_CHOHONE
				|| MyPlayer->_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE
				|| MyPlayer->_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ICE_WAVE
				|| MyPlayer->_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ROOT
				|| MyPlayer->_StatusAbnormal & STATUS_ABNORMAL_TAIOIST_ROOT)
			{
				break;
			}

			int8 MovePlayerCurrentState;
			*Message >> MovePlayerCurrentState;
			st_Vector2Int ClientPlayerPosition;
			*Message >> ClientPlayerPosition._X;
			*Message >> ClientPlayerPosition._Y;
			int8 MovePlayerCurrentDir;
			*Message >> MovePlayerCurrentDir;
			float PositionX;
			*Message >> PositionX;
			float PositionY;
			*Message >> PositionY;

			// 클라가 움직일 방향값을 가져온다.
			int8 ReqMoveDir;
			*Message >> ReqMoveDir;

			MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = (en_MoveDir)ReqMoveDir;

			if (MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::IDLE
				|| MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::SPAWN_IDLE
				|| MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::SPELL
				|| MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::GATHERING
				|| MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::ATTACK)
			{
				MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			}			

			CMap* Map = G_MapManager->GetMap(1);

			vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(MyPlayer, 1, false);

			CMessage* ResMyMoveOtherPacket = MakePacketResMove(
				MyPlayer->_GameObjectInfo.ObjectId,
				true,
				MyPlayer->_GameObjectInfo.ObjectPositionInfo);
			SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResMyMoveOtherPacket);
			ResMyMoveOtherPacket->Free();
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

			MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

			int8  StopPlayerCurrentState;
			*Message >> StopPlayerCurrentState;
			int32 ClientCollisionPositionX;
			*Message >> ClientCollisionPositionX;
			int32 ClientCollisionPositionY;
			*Message >> ClientCollisionPositionY;
			int8 StopDir;
			*Message >> StopDir;
			float PositionX;
			*Message >> PositionX;
			float PositionY;
			*Message >> PositionY;

			float CheckPositionX = abs(MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X - PositionX);
			float CheckPositionY = abs(MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y - PositionY);

			CMap* Map = G_MapManager->GetMap(1);

			vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(MyPlayer, 1, false);

			CMessage* ResMoveStopPacket = MakePacketResMoveStop(
				MyPlayer->_GameObjectInfo.ObjectId,
				MyPlayer->_GameObjectInfo.ObjectPositionInfo);
			SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResMoveStopPacket);
			ResMoveStopPacket->Free();
		} while (0);
	}

	ReturnSession(Session);
}

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

			if (MyPlayer->_StatusAbnormal & STATUS_ABNORMAL_WARRIOR_CHOHONE
				|| MyPlayer->_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE
				|| MyPlayer->_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ICE_WAVE)
			{
				break;
			}			

			int8 QuickSlotBarIndex;
			*Message >> QuickSlotBarIndex;
			int8 QuickSlotBarSlotIndex;
			*Message >> QuickSlotBarSlotIndex;

			int8 ReqMoveDir;
			*Message >> ReqMoveDir;

			int16 ReqSkillType;
			*Message >> ReqSkillType;

			bool SkillUseSuccess = false;
			bool GlobalSkillCooltime = true;

			vector<CGameObject*> Targets;

			if (MyPlayer->_ComboSkill != nullptr)
			{
				st_GameObjectJob* ComboAttackOffJob = MakeGameObjectJobComboSkillOff();
				MyPlayer->_GameObjectJobQue.Enqueue(ComboAttackOffJob);
			}

			CSkill* ReqMeleeSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)ReqSkillType);
			if (ReqMeleeSkill != nullptr && ReqMeleeSkill->GetSkillInfo()->CanSkillUse == true)
			{
				CMap* Map = G_MapManager->GetMap(1);

				vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(MyPlayer, 1, false);
				vector<st_FieldOfViewInfo> CurrentFieldOfViewAttackObjectIDs = Map->GetFieldOfViewAttackObjects(MyPlayer, 1);

				// 타겟 위치 확인
				switch (ReqMeleeSkill->GetSkillInfo()->SkillType)
				{
				case en_SkillType::SKILL_DEFAULT_ATTACK:
					{
						SkillUseSuccess = true;

						st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)ReqMeleeSkill->GetSkillInfo();
						
						// 전방 2미터 거리 안에 있는 공격 가능한 오브젝트들을 가져옴
						vector<CGameObject*> FindObjects = MyPlayer->GetChannel()->FindAttackChannelObjects(CurrentFieldOfViewAttackObjectIDs, MyPlayer, 2);
						for (CGameObject* FindOBJ : FindObjects)
						{
							Targets.push_back(FindOBJ);
						}
					}
					break;
				case en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK:
				case en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK:
				case en_SkillType::SKILL_KNIGHT_SHIELD_SMASH:				
					{
						SkillUseSuccess = true;

						st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)ReqMeleeSkill->GetSkillInfo();
						if (AttackSkillInfo->NextComboSkill != en_SkillType::SKILL_TYPE_NONE)
						{
							CSkill* FindNextComboSkill = MyPlayer->_SkillBox.FindSkill(AttackSkillInfo->NextComboSkill);
							if (FindNextComboSkill->GetSkillInfo()->CanSkillUse == true)
							{
								st_GameObjectJob* ComboAttackCreateJob = MakeGameObjectJobComboSkillCreate(QuickSlotBarIndex, QuickSlotBarSlotIndex, ReqMeleeSkill);
								MyPlayer->_GameObjectJobQue.Enqueue(ComboAttackCreateJob);
							}						
						}						
						
						vector<CGameObject*> FindObjects = MyPlayer->GetChannel()->FindAttackChannelObjects(CurrentFieldOfViewObjectIDs, MyPlayer, 2);
						for (CGameObject* FindObject : FindObjects)
						{
							Targets.push_back(FindObject);
						}						
					}
					break;				
				case en_SkillType::SKILL_KNIGHT_CHOHONE:
				{
					if (MyPlayer->_SelectTarget != nullptr)
					{
						st_Vector2Int TargetPosition = MyPlayer->GetChannel()->FindChannelObject(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId,
							MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType)->_GameObjectInfo.ObjectPositionInfo.CollisionPosition;
						st_Vector2Int MyPosition = MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition;
						st_Vector2Int Direction = TargetPosition - MyPosition;

						int32 Distance = st_Vector2Int::Distance(TargetPosition, MyPosition);

						if (Distance <= 4)
						{
							CGameObject* Target = MyPlayer->GetChannel()->GetMap()->Find(TargetPosition);
							if (Target != nullptr)
							{
								SkillUseSuccess = true;

								st_Vector2Int MyFrontCellPotision = MyPlayer->GetFrontCellPosition(MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);

								if (MyPlayer->GetChannel()->GetMap()->ApplyMove(Target, MyFrontCellPotision))
								{
									CSkill* NewDeBufSkill = G_ObjectManager->SkillCreate();

									st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(ReqMeleeSkill->GetSkillInfo()->SkillMediumCategory);
									*NewAttackSkillInfo = *((st_AttackSkillInfo*)ReqMeleeSkill->GetSkillInfo());

									NewDeBufSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
									NewDeBufSkill->StatusAbnormalDurationTimeStart();
												
									Target->AddDebuf(NewDeBufSkill);
									Target->SetStatusAbnormal(STATUS_ABNORMAL_WARRIOR_CHOHONE);											
									
									Target->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

									CMessage* SelectTargetMoveStopMessage = MakePacketResMoveStop(Target->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectPositionInfo);
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, SelectTargetMoveStopMessage);
									SelectTargetMoveStopMessage->Free();

									CMessage* ResStatusAbnormalPacket = MakePacketStatusAbnormal(Target->_GameObjectInfo.ObjectId,
										Target->_GameObjectInfo.ObjectType,
										Target->_GameObjectInfo.ObjectPositionInfo.MoveDir,
										ReqMeleeSkill->GetSkillInfo()->SkillType,
										true, STATUS_ABNORMAL_WARRIOR_CHOHONE);
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
									ResStatusAbnormalPacket->Free();

									CMessage* ResBufDeBufSkillPacket = MakePacketBufDeBuf(Target->_GameObjectInfo.ObjectId, false, NewDeBufSkill->GetSkillInfo());
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
									ResBufDeBufSkillPacket->Free();

									CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(Target->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_DEBUF_STUN, NewDeBufSkill->GetSkillInfo()->SkillDurationTime / 1000.0f);
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
									ResEffectPacket->Free();

									Targets.push_back(Target);

									switch (MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir)
									{
									case en_MoveDir::UP:
										Target->_GameObjectInfo.ObjectPositionInfo.Position._X = MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X;
										Target->_GameObjectInfo.ObjectPositionInfo.Position._Y = MyFrontCellPotision._Y + 0.5f;
										break;
									case en_MoveDir::DOWN:
										Target->_GameObjectInfo.ObjectPositionInfo.Position._X = MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X;
										Target->_GameObjectInfo.ObjectPositionInfo.Position._Y = MyFrontCellPotision._Y + 0.5f;
										break;
									case en_MoveDir::LEFT:
										Target->_GameObjectInfo.ObjectPositionInfo.Position._X = MyFrontCellPotision._X + 0.5f;
										Target->_GameObjectInfo.ObjectPositionInfo.Position._Y = MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y;
										break;
									case en_MoveDir::RIGHT:
										Target->_GameObjectInfo.ObjectPositionInfo.Position._X = MyFrontCellPotision._X + 0.5f;
										Target->_GameObjectInfo.ObjectPositionInfo.Position._Y = MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y;
										break;
									}

									CMessage* ResSyncPositionPacket = MakePacketResSyncPosition(Target->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectPositionInfo);
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSyncPositionPacket);
									ResSyncPositionPacket->Free();
								}
								else
								{
									CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_PLACE_BLOCK, ReqMeleeSkill->GetSkillInfo()->SkillName.c_str());
									SendPacket(MyPlayer->_SessionId, ResErrorPacket);
									ResErrorPacket->Free();
								}
							}
						}
						else
						{
							CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_PLACE_DISTANCE, ReqMeleeSkill->GetSkillInfo()->SkillName.c_str(), Distance);
							SendPacket(MyPlayer->_SessionId, ResErrorPacket);
							ResErrorPacket->Free();
						}
					}
					else
					{
						CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT, ReqMeleeSkill->GetSkillInfo()->SkillName.c_str());
						SendPacket(MyPlayer->_SessionId, ResErrorPacket);
						ResErrorPacket->Free();
					}
				}
				break;
				case en_SkillType::SKILL_KNIGHT_SHAEHONE:
				{
					if (MyPlayer->_SelectTarget != nullptr)
					{
						st_Vector2Int TargetPosition = MyPlayer->GetChannel()->FindChannelObject(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType)->_GameObjectInfo.ObjectPositionInfo.CollisionPosition;
						st_Vector2Int MyPosition = MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition;
						st_Vector2Int Direction = TargetPosition - MyPosition;

						en_MoveDir Dir = st_Vector2Int::GetMoveDir(Direction);
						int32 Distance = st_Vector2Int::Distance(TargetPosition, MyPosition);

						if (Distance <= 4)
						{
							CGameObject* Target = MyPlayer->GetChannel()->GetMap()->Find(TargetPosition);
							st_Vector2Int MovePosition;
							if (Target != nullptr)
							{
								SkillUseSuccess = true;

								switch (Dir)
								{
								case en_MoveDir::UP:
									MovePosition = Target->GetFrontCellPosition(en_MoveDir::DOWN, 1);
									break;
								case en_MoveDir::DOWN:
									MovePosition = Target->GetFrontCellPosition(en_MoveDir::UP, 1);
									break;
								case en_MoveDir::LEFT:
									MovePosition = Target->GetFrontCellPosition(en_MoveDir::RIGHT, 1);
									break;
								case en_MoveDir::RIGHT:
									MovePosition = Target->GetFrontCellPosition(en_MoveDir::LEFT, 1);
									break;
								default:
									break;
								}

								if (MyPlayer->GetChannel()->GetMap()->ApplyMove(MyPlayer, MovePosition))
								{
									CSkill* NewDeBufSkill = G_ObjectManager->SkillCreate();

									st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(ReqMeleeSkill->GetSkillInfo()->SkillMediumCategory);

									*AttackSkillInfo = *((st_AttackSkillInfo*)ReqMeleeSkill->GetSkillInfo());

									NewDeBufSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, AttackSkillInfo);
									NewDeBufSkill->StatusAbnormalDurationTimeStart();
									
									Target->AddDebuf(NewDeBufSkill);
									Target->SetStatusAbnormal(STATUS_ABNORMAL_WARRIOR_SHAEHONE);																			

									Target->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

									CMessage* SelectTargetMoveStopMessage = MakePacketResMoveStop(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectPositionInfo);
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, SelectTargetMoveStopMessage);
									SelectTargetMoveStopMessage->Free();

									CMessage* ResStatusAbnormalPacket = MakePacketStatusAbnormal(Target->_GameObjectInfo.ObjectId,
										Target->_GameObjectInfo.ObjectType,
										Target->_GameObjectInfo.ObjectPositionInfo.MoveDir,
										ReqMeleeSkill->GetSkillInfo()->SkillType,
										true, STATUS_ABNORMAL_WARRIOR_SHAEHONE);
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
									ResStatusAbnormalPacket->Free();

									CMessage* ResBufDeBufSkillPacket = MakePacketBufDeBuf(Target->_GameObjectInfo.ObjectId, false, NewDeBufSkill->GetSkillInfo());
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
									ResBufDeBufSkillPacket->Free();

									float EffectPrintTime = ReqMeleeSkill->GetSkillInfo()->SkillDurationTime / 1000.0f;

									// 이펙트 출력
									CMessage* ResEffectPacket = MakePacketEffect(Target->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_DEBUF_ROOT, EffectPrintTime);
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
									ResEffectPacket->Free();

									Targets.push_back(Target);

									switch (Dir)
									{
									case en_MoveDir::UP:
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::UP;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X = Target->_GameObjectInfo.ObjectPositionInfo.Position._X;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y = MovePosition._Y + 0.5f;										
										break;
									case en_MoveDir::DOWN:
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X = Target->_GameObjectInfo.ObjectPositionInfo.Position._X;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y = MovePosition._Y + 0.5f;
										break;
									case en_MoveDir::LEFT:
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::LEFT;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X = MovePosition._X + 0.5f;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y = Target->_GameObjectInfo.ObjectPositionInfo.Position._Y;
										break;
									case en_MoveDir::RIGHT:
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::RIGHT;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X = MovePosition._X + 0.5f;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y = Target->_GameObjectInfo.ObjectPositionInfo.Position._Y;
										break;
									}

									CMessage* ResSyncPositionPacket = MakePacketResSyncPosition(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSyncPositionPacket);
									ResSyncPositionPacket->Free();
								}
								else
								{
									CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_PLACE_BLOCK, ReqMeleeSkill->GetSkillInfo()->SkillName.c_str());
									SendPacket(MyPlayer->_SessionId, ResErrorPacket);
									ResErrorPacket->Free();
								}
							}
						}
						else
						{
							CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_PLACE_DISTANCE, ReqMeleeSkill->GetSkillInfo()->SkillName.c_str(), Distance);
							SendPacket(MyPlayer->_SessionId, ResErrorPacket);
							ResErrorPacket->Free();
						}
					}
					else
					{
						CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT, ReqMeleeSkill->GetSkillInfo()->SkillName.c_str());
						SendPacket(MyPlayer->_SessionId, ResErrorPacket);
						ResErrorPacket->Free();
					}
				}
				break;
				case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
					{
						SkillUseSuccess = true;

						vector<st_Vector2Int> TargetPositions;

						TargetPositions = MyPlayer->GetAroundCellPositions(MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, 1);
						for (st_Vector2Int TargetPosition : TargetPositions)
						{
							CGameObject* Target = MyPlayer->GetChannel()->GetMap()->Find(TargetPosition);
							if (Target != nullptr)
							{
								Targets.push_back(Target);
							}
						}

						// 이펙트 출력
						CMessage* ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_SMASH_WAVE, 2.0f);
						SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
						ResEffectPacket->Free();
					}
					break;				
				}

				if (SkillUseSuccess == true)
				{
					CMessage* AnimationPlayPacket = MakePacketResAnimationPlay(MyPlayer->_GameObjectInfo.ObjectId,
						MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir,
						(*ReqMeleeSkill->GetSkillInfo()->SkillAnimations.find(MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir)).second);
					SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, AnimationPlayPacket);
					AnimationPlayPacket->Free();

					ReqMeleeSkill->CoolTimeStart();

					for (auto QuickSlotBarPosition : MyPlayer->_QuickSlotManager.FindQuickSlotBar(ReqMeleeSkill->GetSkillInfo()->SkillType))
					{
						// 클라에게 쿨타임 표시
						CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
							QuickSlotBarPosition.QuickSlotBarSlotIndex,
							1.0f, ReqMeleeSkill);
						SendPacket(Session->SessionId, ResCoolTimeStartPacket);
						ResCoolTimeStartPacket->Free();
					}

					if (GlobalSkillCooltime)
					{
						// 전역 쿨타임 적용
						for (auto QuickSlotBarPosition : MyPlayer->_QuickSlotManager.GlobalCoolTimeFindQuickSlotBar(QuickSlotBarIndex, QuickSlotBarSlotIndex, ReqMeleeSkill->GetSkillKind()))
						{
							st_QuickSlotBarSlotInfo* QuickSlotInfo = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarPosition.QuickSlotBarIndex, QuickSlotBarPosition.QuickSlotBarSlotIndex);
							if (QuickSlotInfo->QuickBarSkill != nullptr)
							{
								QuickSlotInfo->QuickBarSkill->GlobalCoolTimeStart(MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate);

								CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
									QuickSlotBarPosition.QuickSlotBarSlotIndex,
									1.0f, nullptr, MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate);
								SendPacket(Session->SessionId, ResCoolTimeStartPacket);
								ResCoolTimeStartPacket->Free();
							}
						}
					}					

					MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = (en_MoveDir)ReqMoveDir;

					for (CGameObject* Target : Targets)
					{
						if (Target->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
						{
							st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)ReqMeleeSkill->GetSkillInfo();

							random_device Seed;
							default_random_engine Eng(Seed());

							// 크리티컬 판단
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

							int32 FinalDamage = (int32)(CriticalDamage * DefenceRate);

							st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(MyPlayer, FinalDamage);
							Target->_GameObjectJobQue.Enqueue(DamageJob);							

							en_EffectType HitEffectType = en_EffectType::EFFECT_TYPE_NONE;

							wstring SkillTypeString;
							wchar_t SkillTypeMessage[64] = L"0";
							wchar_t SkillDamageMessage[64] = L"0";

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
							case en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK:
								wsprintf(SkillTypeMessage, L"%s가 맹렬한 일격을 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
								HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
								break;
							case en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK:
								wsprintf(SkillTypeMessage, L"%s가 회심의 일격을 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
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
							case en_SkillType::SKILL_KNIGHT_SHIELD_SMASH:
								wsprintf(SkillTypeMessage, L"%s가 방패강타를 사용해 %s에게 %d의 데미지를 줬습니다.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
								break;
							default:
								break;
							}

							SkillTypeString = SkillTypeMessage;
							SkillTypeString = IsCritical ? L"치명타! " + SkillTypeString : SkillTypeString;

							// 데미지 시스템 메세지 전송
							CMessage* ResSkillSystemMessagePacket = MakePacketResChattingBoxMessage(MyPlayer->_GameObjectInfo.ObjectId,
								en_MessageType::SYSTEM,
								IsCritical ? st_Color::Red() : st_Color::White(),
								SkillTypeString);
							SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSkillSystemMessagePacket);
							ResSkillSystemMessagePacket->Free();

							CMessage* ResDamagePacket = MakePacketResDamage(MyPlayer->_GameObjectInfo.ObjectId,
								Target->_GameObjectInfo.ObjectId,
								(en_SkillType)ReqSkillType,
								FinalDamage,
								IsCritical);
							SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResDamagePacket);
							ResDamagePacket->Free();							

							// 이펙트 출력
							CMessage* ResEffectPacket = MakePacketEffect(Target->_GameObjectInfo.ObjectId,
								HitEffectType,
								AttackSkillInfo->SkillTargetEffectTime);
							SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
							ResEffectPacket->Free();

							// 스탯 변경 메세지 전송
							CMessage* ResChangeObjectStat = MakePacketResChangeObjectStat(Target->_GameObjectInfo.ObjectId,
								Target->_GameObjectInfo.ObjectStatInfo);
							SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResChangeObjectStat);
							ResChangeObjectStat->Free();
						}
					}
				}				
			}
			else
			{
				CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_SKILL_COOLTIME, ReqMeleeSkill->GetSkillInfo()->SkillName.c_str());
				SendPacket(MyPlayer->_SessionId, ResErrorPacket);
				ResErrorPacket->Free();
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

			if ((en_SkillType)ReqSkillType == en_SkillType::SKILL_SHOCK_RELEASE)
			{
				CSkill* ReqMagicSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)ReqSkillType);
				if (ReqMagicSkill != nullptr && ReqMagicSkill->GetSkillInfo()->CanSkillUse == true)
				{
					CMap* Map = G_MapManager->GetMap(1);

					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(MyPlayer, 1, false);

					st_GameObjectJob* ShockReleaseJob = MakeGameObjectJobShockRelease();
					MyPlayer->_GameObjectJobQue.Enqueue(ShockReleaseJob);

					CSkill* NewBufSkill = G_ObjectManager->SkillCreate();

					st_BufSkillInfo* NewShockReleaseSkillInfo = (st_BufSkillInfo*)G_ObjectManager->SkillInfoCreate(ReqMagicSkill->GetSkillInfo()->SkillMediumCategory);

					*NewShockReleaseSkillInfo = *((st_BufSkillInfo*)ReqMagicSkill->GetSkillInfo());
					NewBufSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewShockReleaseSkillInfo);
					NewBufSkill->StatusAbnormalDurationTimeStart();

					MyPlayer->AddBuf(NewBufSkill);

					CMessage* ResBufDebufSkillPacket = MakePacketBufDeBuf(MyPlayer->_GameObjectInfo.ObjectId, true, NewBufSkill->GetSkillInfo());
					SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDebufSkillPacket);
					ResBufDebufSkillPacket->Free();

					ReqMagicSkill->CoolTimeStart();

					// 클라에게 쿨타임 표시
					CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(QuickSlotBarIndex,
						QuickSlotBarSlotIndex,
						1.0f, ReqMagicSkill);
					SendPacket(Session->SessionId, ResCoolTimeStartPacket);
					ResCoolTimeStartPacket->Free();

					CMessage* ResObjectStateChangePacket = MakePacketResChangeObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
					SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResObjectStateChangePacket);
					ResObjectStateChangePacket->Free();
				}				
			}
			else
			{
				// 충격 해제 마법을 제외하고 상태이상값을 판단
				bool IsWarriorChohone = MyPlayer->_StatusAbnormal & STATUS_ABNORMAL_WARRIOR_CHOHONE;
				bool IsShamanLightningStrike = MyPlayer->_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE;
				bool IsShamanIceWave = MyPlayer->_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ICE_WAVE;

				if (IsWarriorChohone || IsShamanLightningStrike || IsShamanIceWave)
				{
					break;
				}				

				CGameObject* FindGameObject = nullptr;
				float SpellCastingTime = 0.0f;

				CMessage* ResEffectPacket = nullptr;
				CMessage* ResMagicPacket = nullptr;

				vector<CGameObject*> Targets;				
								
				// 요청 스킬을 스킬창에서 찾음
				CSkill* ReqMagicSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)ReqSkillType);
				if (ReqMagicSkill != nullptr && ReqMagicSkill->GetSkillInfo()->CanSkillUse == true)
				{
					if (MyPlayer->_ComboSkill != nullptr)
					{
						st_GameObjectJob* ComboAttackOffJob = MakeGameObjectJobComboSkillOff();
						MyPlayer->_GameObjectJobQue.Enqueue(ComboAttackOffJob);
					}

					CMap* Map = G_MapManager->GetMap(1);

					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(MyPlayer, 1, false);
					
					// 요청한 스킬이 퀵슬롯에 등록이 되어 있는지 확인
					st_QuickSlotBarSlotInfo* QuickSlotInfo = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarIndex, QuickSlotBarSlotIndex);
					if (QuickSlotInfo->QuickBarSkill != nullptr)
					{
						// 요청한 스킬을 사용 할 수 있는지 확인
						if (ReqMagicSkill->GetSkillInfo()->CanSkillUse)
						{
							en_SkillType ReqMagicSkillType = ReqMagicSkill->GetSkillInfo()->SkillType;
							switch (ReqMagicSkillType)
							{
							case en_SkillType::SKILL_KNIGHT_CHARGE_POSE:
							{
								st_BufSkillInfo* ChargePoseSkillInfo = (st_BufSkillInfo*)ReqMagicSkill->GetSkillInfo();

								CSkill* ChargePoseSkill = G_ObjectManager->SkillCreate();

								st_BufSkillInfo* NewChargePoseSkillInfo = (st_BufSkillInfo*)G_ObjectManager->SkillInfoCreate(ReqMagicSkill->GetSkillInfo()->SkillMediumCategory);
								*NewChargePoseSkillInfo = *((st_BufSkillInfo*)ReqMagicSkill->GetSkillInfo());
								ChargePoseSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewChargePoseSkillInfo);
								ChargePoseSkill->StatusAbnormalDurationTimeStart();

								MyPlayer->AddBuf(ChargePoseSkill);

								CMessage* ResBufDeBufSkillPacket = MakePacketBufDeBuf(MyPlayer->_GameObjectInfo.ObjectId, true, ChargePoseSkill->GetSkillInfo());
								SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
								ResBufDeBufSkillPacket->Free();

								ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_CHARGE_POSE, 2.8f);
								SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
								ResEffectPacket->Free();

								ReqMagicSkill->CoolTimeStart();

								// 클라에게 쿨타임 표시
								CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(QuickSlotBarIndex,
									QuickSlotBarSlotIndex,
									1.0f, ReqMagicSkill);
								SendPacket(Session->SessionId, ResCoolTimeStartPacket);
								ResCoolTimeStartPacket->Free();
							}
							break;
							case en_SkillType::SKILL_SHAMAN_BACK_TELEPORT:
							{								
								st_GameObjectJob* BackTelePortJob = MakeGameObjectJobBackTeleport();
								MyPlayer->_GameObjectJobQue.Enqueue(BackTelePortJob);								

								en_MoveDir TelePortDir;

								switch (MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir)
								{
								case en_MoveDir::UP:
									TelePortDir = en_MoveDir::DOWN;
									break;
								case en_MoveDir::DOWN:
									TelePortDir = en_MoveDir::UP;
									break;
								case en_MoveDir::LEFT:
									TelePortDir = en_MoveDir::RIGHT;
									break;
								case en_MoveDir::RIGHT:
									TelePortDir = en_MoveDir::LEFT;
									break;
								}

								st_Vector2Int MovePosition;
								MovePosition = MyPlayer->GetFrontCellPosition(TelePortDir, 5);

								MyPlayer->GetChannel()->GetMap()->ApplyMove(MyPlayer, MovePosition);

								MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X = MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
								MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y = MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;

								CMessage* ResSyncPositionPacket = MakePacketResSyncPosition(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
								SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSyncPositionPacket);
								ResSyncPositionPacket->Free();

								ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_BACK_TELEPORT, 0.5f);
								SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
								ResEffectPacket->Free();

								ReqMagicSkill->CoolTimeStart();

								// 클라에게 쿨타임 표시
								CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(QuickSlotBarIndex,
									QuickSlotBarSlotIndex,
									1.0f, ReqMagicSkill);
								SendPacket(Session->SessionId, ResCoolTimeStartPacket);
								ResCoolTimeStartPacket->Free();
							}
							break;
							case en_SkillType::SKILL_SHAMAN_FLAME_HARPOON:
							case en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE:
							case en_SkillType::SKILL_SHAMAN_HELL_FIRE:
							case en_SkillType::SKILL_SHAMAN_ROOT:
							case en_SkillType::SKILL_SHAMAN_ICE_CHAIN:
							case en_SkillType::SKILL_TAIOIST_DIVINE_STRIKE:
							case en_SkillType::SKILL_TAIOIST_ROOT:
								if (MyPlayer->_SelectTarget != nullptr)
								{
									if (MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId != MyPlayer->_GameObjectInfo.ObjectId)
									{
										SpellCastingTime = ReqMagicSkill->GetSkillInfo()->SkillCastingTime / 1000.0f;

										int16 Distance = st_Vector2Int::Distance(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

										if (Distance <= 6)
										{
											FindGameObject = MyPlayer->GetChannel()->FindChannelObject(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId,
												MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType);
											if (FindGameObject != nullptr)
											{
												Targets.push_back(FindGameObject);
											}

											// 스펠창 시작
											ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId, true, ReqMagicSkill->GetSkillInfo()->SkillType, SpellCastingTime);
											SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResMagicPacket);
											ResMagicPacket->Free();
										}
										else
										{
											CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_PLACE_DISTANCE, ReqMagicSkill->GetSkillInfo()->SkillName.c_str(), Distance);
											SendPacket(MyPlayer->_SessionId, ResErrorPacket);
											ResErrorPacket->Free();
										}
									}
									else
									{
										CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_MYSELF_TARGET, ReqMagicSkill->GetSkillInfo()->SkillName.c_str());
										SendPacket(MyPlayer->_SessionId, ResErrorPacket);
										ResErrorPacket->Free();
									}
								}
								else
								{
									CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT, ReqMagicSkill->GetSkillInfo()->SkillName.c_str());
									SendPacket(MyPlayer->_SessionId, ResErrorPacket);
									ResErrorPacket->Free();
								}
								break;
							case en_SkillType::SKILL_SHAMAN_ICE_WAVE:
								{
									if (MyPlayer->_SelectTarget != nullptr)
									{
										if (MyPlayer->_ComboSkill != nullptr && MyPlayer->_ComboSkill->GetSkillInfo()->SkillType == en_SkillType::SKILL_SHAMAN_ICE_WAVE)
										{
											CSkill* NewDebufSkill = G_ObjectManager->SkillCreate();

											st_AttackSkillInfo* NewDebufSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(ReqMagicSkill->GetSkillInfo()->SkillMediumCategory);
											*NewDebufSkillInfo = *((st_AttackSkillInfo*)ReqMagicSkill->GetSkillInfo());

											NewDebufSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewDebufSkillInfo);
											NewDebufSkill->StatusAbnormalDurationTimeStart();

											MyPlayer->_SelectTarget->AddDebuf(NewDebufSkill);
											MyPlayer->_SelectTarget->SetStatusAbnormal(STATUS_ABNORMAL_SHAMAN_ICE_WAVE);

											CMessage* ResStatusAbnormalPacket = MakePacketStatusAbnormal(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId,
												MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType,
												MyPlayer->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
												NewDebufSkill->GetSkillInfo()->SkillType,
												true, STATUS_ABNORMAL_SHAMAN_ICE_WAVE);
											SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
											ResStatusAbnormalPacket->Free();

											CMessage* ResBufDeBufSkillPacket = MakePacketBufDeBuf(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, false, NewDebufSkill->GetSkillInfo());
											SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
											ResBufDeBufSkillPacket->Free();

											ReqMagicSkill->CoolTimeStart();

											for (auto QuickSlotBarPosition : MyPlayer->_QuickSlotManager.FindQuickSlotBar(ReqMagicSkill->GetSkillInfo()->SkillType))
											{
												// 클라에게 쿨타임 표시
												CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
													QuickSlotBarPosition.QuickSlotBarSlotIndex,
													1.0f, ReqMagicSkill);
												SendPacket(Session->SessionId, ResCoolTimeStartPacket);
												ResCoolTimeStartPacket->Free();
											}
										}
									}
									else
									{
										CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT, ReqMagicSkill->GetSkillInfo()->SkillName.c_str());
										SendPacket(MyPlayer->_SessionId, ResErrorPacket);
										ResErrorPacket->Free();
									}																		
								}
								break;
							case en_SkillType::SKILL_TAIOIST_HEALING_LIGHT:
							case en_SkillType::SKILL_TAIOIST_HEALING_WIND:
								SpellCastingTime = ReqMagicSkill->GetSkillInfo()->SkillCastingTime / 1000.0f;

								if (MyPlayer->_SelectTarget != nullptr)
								{
									FindGameObject = MyPlayer->GetChannel()->FindChannelObject(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType);
									if (FindGameObject != nullptr)
									{
										Targets.push_back(FindGameObject);
									}

									// 스펠창 시작
									ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId,
										true, ReqMagicSkill->GetSkillInfo()->SkillType, SpellCastingTime);
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResMagicPacket);
									ResMagicPacket->Free();
								}
								else
								{
									MyPlayer->_SelectTarget = MyPlayer;

									Targets.push_back(MyPlayer);

									CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_HEAL_NON_SELECT_OBJECT, ReqMagicSkill->GetSkillInfo()->SkillName.c_str());
									SendPacket(MyPlayer->_SessionId, ResErrorPacket);
									ResErrorPacket->Free();

									// 스펠창 시작
									ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId,
										true, ReqMagicSkill->GetSkillInfo()->SkillType, SpellCastingTime);
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResMagicPacket);
									ResMagicPacket->Free();
								}
								break;
							}

							if (Targets.size() >= 1)
							{
								MyPlayer->_Owner = Targets[0];

								ReqMagicSkill->_QuickSlotBarIndex = QuickSlotBarIndex;
								ReqMagicSkill->_QuickSlotBarSlotIndex = QuickSlotBarSlotIndex;

								st_GameObjectJob* SpellStartJob = MakeGameObjectJobSpellStart(ReqMagicSkill);
								MyPlayer->_GameObjectJobQue.Enqueue(SpellStartJob);								
							}
						}
						else
						{
							CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_SKILL_COOLTIME,
								ReqMagicSkill->GetSkillInfo()->SkillName.c_str());
							SendPacket(MyPlayer->_SessionId, ResErrorPacket);
							ResErrorPacket->Free();
						}
					}
				}				
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

			st_GameObjectJob* SpellCancelJob = MakeGameObjectJobSpellCancel();
			MyPlayer->_GameObjectJobQue.Enqueue(SpellCancelJob);
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

			CGameObject* FindObject = MyPlayer->GetChannel()->FindChannelObject(ObjectId, (en_GameObjectType)ObjectType);
			if (FindObject != nullptr && FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::GATHERING)
			{
				st_Vector2 DirNormalVector = (FindObject->_GameObjectInfo.ObjectPositionInfo.Position - MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position).Normalize();

				en_MoveDir Dir = st_Vector2::GetMoveDir(DirNormalVector);

				float Distance = st_Vector2::Distance(FindObject->_GameObjectInfo.ObjectPositionInfo.Position, MyPlayer->_GameObjectInfo.ObjectPositionInfo.Position);

				if (MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir != Dir)
				{
					CMessage* DirErrorPacket = MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_DIR_DIFFERENT, FindObject->_GameObjectInfo.ObjectName.c_str());
					SendPacket(Session->SessionId, DirErrorPacket);
					DirErrorPacket->Clear();
					break;
				}

				if (Distance > 1.0f)
				{
					CMessage* DirErrorPacket = MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_GATHERING_DISTANCE, FindObject->_GameObjectInfo.ObjectName.c_str());
					SendPacket(Session->SessionId, DirErrorPacket);
					DirErrorPacket->Clear();
					break;
				}

				CMap* Map = G_MapManager->GetMap(1);

				vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(MyPlayer, 1, false);

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

			CGameObject* FindObject = MyPlayer->GetChannel()->FindChannelObject(ObjectId, (en_GameObjectType)ObjectType);
			if (FindObject != nullptr)
			{
				int64 PreviousChoiceObject = 0;

				if (MyPlayer->_SelectTarget != nullptr)
				{
					PreviousChoiceObject = MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId;
				}

				MyPlayer->_SelectTarget = FindObject;

				CMessage* ResMousePositionObjectInfo = MakePacketResLeftMousePositionObjectInfo(Session->AccountId,
					PreviousChoiceObject, FindObject->_GameObjectInfo.ObjectId,
					FindObject->_Bufs, FindObject->_DeBufs);
				SendPacket(Session->SessionId, ResMousePositionObjectInfo);
				ResMousePositionObjectInfo->Free();
			}
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
					CMap* Map = G_MapManager->GetMap(1);
					CChannel* Channel = Map->GetChannelManager()->Find(1);

					CGameObject* CraftingTableGO = Channel->FindChannelObject(OwnerObjectID, en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE);
					if (CraftingTableGO != nullptr)
					{						
						CCraftingTable* CraftingTable = (CCraftingTable*)CraftingTableGO;

						CraftingTable->_SelectCraftingItemType = (en_SmallItemCategory)LeftMouseItemCategory;
						
						CMessage* ResCraftingTableCompleteItemSelectPacket = MakePacketResCraftingTableCompleteItemSelect(CraftingTableGO->_GameObjectInfo.ObjectId,
							CraftingTable->_SelectCraftingItemType,
							CraftingTable->GetMaterialItems());
						SendPacket(Session->SessionId, ResCraftingTableCompleteItemSelectPacket);
						ResCraftingTableCompleteItemSelectPacket->Free();						
					}					
				}				
				break;		
			}
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

			CGameObject* FindObject = MyPlayer->GetChannel()->FindChannelObject(ObjectId, (en_GameObjectType)ObjectType);
			if (FindObject != nullptr)
			{
				CMap* Map = G_MapManager->GetMap(1);
				CChannel* Channel = Map->GetChannelManager()->Find(1);

				switch (FindObject->_GameObjectInfo.ObjectType)
				{
				case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
				case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
					{
						CCraftingTable* CraftingTable = (CCraftingTable*)FindObject;						

						// 사용중이 아님
						if (CraftingTable->_SelectedCraftingTable == false)
						{
							// 요청한 플레이어가 전에 선택중이었던 제작대가 있는지 확인				
							vector<CGameObject*> ChannelFindObjects = Channel->FindChannelObjects(en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE);
							for (CGameObject* ChannelFindObject : ChannelFindObjects)
							{
								CCraftingTable* FindCraftingTable = (CCraftingTable*)ChannelFindObject;

								// 선택중이었던 용광로가 있으면
								if (FindCraftingTable->_SelectedObject != nullptr)
								{
									// 선택중인 용광로 선택해제
									st_GameObjectJob* CraftingTableNonSelectJob = MakeGameObjectJobCraftingTableNonSelect(ChannelFindObject);
									ChannelFindObject->_GameObjectJobQue.Enqueue(CraftingTableNonSelectJob);
								}
							}

							st_GameObjectJob* CraftingTableSelectJob = MakeGameObjectJobCraftingTableSelect(FindObject, MyPlayer);
							FindObject->_GameObjectJobQue.Enqueue(CraftingTableSelectJob);

							CMessage* ResRightMousePositionObjectInfoPacket = MakePacketResRightMousePositionObjectInfo(MyPlayer->_GameObjectInfo.ObjectId,
								FindObject->_GameObjectInfo.ObjectId, FindObject->_GameObjectInfo.ObjectType);
							SendPacket(Session->SessionId, ResRightMousePositionObjectInfoPacket);
							ResRightMousePositionObjectInfoPacket->Free();
						}
						else
						{
							// 사용중임
							CMessage* CommonErrorPacket = MakePacketCommonError(en_PersonalMessageType::PERSONAL_MEESAGE_CRAFTING_TABLE_OVERLAP_SELECT, FindObject->_GameObjectInfo.ObjectName.c_str());
							SendPacket(Session->SessionId, CommonErrorPacket);
							CommonErrorPacket->Free();
						}
					}					
					break;				
				}
			}
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

			CGameObject* FindObject = MyPlayer->GetChannel()->FindChannelObject(CraftingTableObjectID, en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE);
			if (FindObject != nullptr)
			{
				switch (FindObject->_GameObjectInfo.ObjectType)
				{
				case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
				case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:					
					{					
						CCraftingTable* CraftingTable = (CCraftingTable*)FindObject;

						if (CraftingTable->_SelectedCraftingTable == true)
						{
							st_GameObjectJob* CraftingTableSelectJob = MakeGameObjectJobCraftingTableNonSelect(FindObject);
							FindObject->_GameObjectJobQue.Enqueue(CraftingTableSelectJob);

							CMessage* ResCraftingTableNonSelectMessage = MakePacketResCraftingTableNonSelect(CraftingTable->_GameObjectInfo.ObjectId, CraftingTable->_GameObjectInfo.ObjectType);
							SendPacket(Session->SessionId, ResCraftingTableNonSelectMessage);
							ResCraftingTableNonSelectMessage->Free();
						}
						else
						{
							// 선택한 용광로 UI에서 다른 용광로 UI를 켤 경우 이전 용광로 UI는 닫는 작업이 필요함
							CRASH("선택되어 있지 않은 대상을 선택 풀려고 함");
						}
					}					
					break;			
				}
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

			CGameObject* FindGameObject = MyPlayer->GetChannel()->FindChannelObject(ObjectId, (en_GameObjectType)ObjectType);
			if (FindGameObject != nullptr)
			{
				CMap* Map = G_MapManager->GetMap(1);

				vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(MyPlayer, 1, false);

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
						SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResObjectStatePakcet);
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
						SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResObjectStatePakcet);
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

		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(MyPlayer, 1, false);
				
		CMessage* ResChattingMessage = MakePacketResChattingBoxMessage(PlayerDBId, en_MessageType::CHATTING, st_Color::White(), ChattingMessage);
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

			CItem* SelectItem = MyPlayer->_InventoryManager.SelectItem(0, SelectItemTileGridPositionX, SelectItemTileGridPositionY);

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

			CItem* PlaceItem = MyPlayer->_InventoryManager.SwapItem(0, PlaceItemTilePositionX, PlaceItemTilePositionY);			

			CMessage* ResPlaceItemPacket = MakePacketResPlaceItem(Session->AccountId, MyPlayer->_GameObjectInfo.ObjectId, PlaceItem, MyPlayer->_InventoryManager._SelectItem);
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

			if (MyPlayer->_InventoryManager._SelectItem != nullptr)
			{
				int16 ItemWidth = MyPlayer->_InventoryManager._SelectItem->_ItemInfo.ItemWidth;
				int16 ItemHeight = MyPlayer->_InventoryManager._SelectItem->_ItemInfo.ItemHeight;

				MyPlayer->_InventoryManager._SelectItem->_ItemInfo.ItemWidth = ItemHeight;
				MyPlayer->_InventoryManager._SelectItem->_ItemInfo.ItemHeight = ItemWidth;

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
			int16 QuickSlotKey;
			*Message >> QuickSlotKey;
			bool IsSkilQuickSlot;
			*Message >> IsSkilQuickSlot;

			if (IsSkilQuickSlot == true)
			{
				// 저장할 기술 정보 파싱
				int8 QuickSlotSkillLargetCategory;
				*Message >> QuickSlotSkillLargetCategory;
				int8 QuickSlotSkillMediumCategory;
				*Message >> QuickSlotSkillMediumCategory;
				int16 QuickSlotSkillType;
				*Message >> QuickSlotSkillType;

				st_QuickSlotBarSlotInfo* FindQuickSlotInfo = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarIndex, QuickSlotBarSlotIndex);
				if (FindQuickSlotInfo != nullptr)
				{
					CSkill* FindSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)QuickSlotSkillType);
					if (FindSkill != nullptr)
					{
						FindQuickSlotInfo->QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_SKILL;
						FindQuickSlotInfo->QuickBarSkill = FindSkill;
						FindQuickSlotInfo->QuickBarItem = nullptr;
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
					CItem* FindItem = MyPlayer->_InventoryManager.FindInventoryItem(0, (en_SmallItemCategory)ItemSmallCategory);
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

			int8 QuickSlotBarSwapIndexA;
			*Message >> QuickSlotBarSwapIndexA;
			int8 QuickSlotBarSlotSwapIndexA;
			*Message >> QuickSlotBarSlotSwapIndexA;

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

			SwapAQuickSlotBarInfo.QuickSlotKey = FindAQuickslotInfo->QuickSlotKey;
			SwapBQuickSlotBarInfo.QuickSlotKey = FindBQuickslotInfo->QuickSlotKey;

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

			int8 QuickSlotBarIndex;
			*Message >> QuickSlotBarIndex;

			int8 QuickSlotBarSlotIndex;
			*Message >> QuickSlotBarSlotIndex;

			// 퀵슬롯에서 정보 찾기
			st_QuickSlotBarSlotInfo* InitQuickSlotBarSlot = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarIndex, QuickSlotBarSlotIndex);
			if (InitQuickSlotBarSlot != nullptr)
			{				
				// 퀵슬롯에서 스킬 연결 해제
				InitQuickSlotBarSlot->QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_NONE;
				InitQuickSlotBarSlot->QuickBarSkill = nullptr;
				InitQuickSlotBarSlot->QuickBarItem = nullptr;
			}
			else
			{
				CRASH("퀵슬롯정보를 찾지 못함");
			}

			CMessage* ResQuickSlotInitMessage = MakePacketResQuickSlotInit(QuickSlotBarIndex, QuickSlotBarSlotIndex);
			SendPacket(Session->SessionId, ResQuickSlotInitMessage);
			ResQuickSlotInitMessage->Free();

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
				vector<CItem*> FindMaterials = MyPlayer->_InventoryManager.FindAllInventoryItem(0, CraftingMaterialItemInfo.MaterialItemType);

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

			// 만들고자 하는 제작템을 DataManager에서 가져온다.
			auto ItemDataIterator = G_Datamanager->_Items.find((int16)ReqCraftingItemType);
			if (ItemDataIterator == G_Datamanager->_Items.end())
			{
				break;
			}

			st_ItemInfo* FindReqCompleteItemData = (*ItemDataIterator).second;

			st_ItemInfo CraftingItemInfo;
			CraftingItemInfo.ItemDBId = 0;
			CraftingItemInfo.ItemIsQuickSlotUse = FindReqCompleteItemData->ItemIsEquipped;
			CraftingItemInfo.ItemLargeCategory = FindReqCompleteItemData->ItemLargeCategory;
			CraftingItemInfo.ItemMediumCategory = FindReqCompleteItemData->ItemMediumCategory;
			CraftingItemInfo.ItemSmallCategory = FindReqCompleteItemData->ItemSmallCategory;
			CraftingItemInfo.ItemName = FindReqCompleteItemData->ItemName;
			CraftingItemInfo.ItemExplain = FindReqCompleteItemData->ItemExplain;
			CraftingItemInfo.ItemMinDamage = FindReqCompleteItemData->ItemMinDamage;
			CraftingItemInfo.ItemMaxDamage = FindReqCompleteItemData->ItemMaxDamage;
			CraftingItemInfo.ItemDefence = FindReqCompleteItemData->ItemDefence;
			CraftingItemInfo.ItemMaxCount = FindReqCompleteItemData->ItemMaxCount;
			CraftingItemInfo.ItemCount = ReqCraftingItemCount;			
			CraftingItemInfo.ItemIsEquipped = FindReqCompleteItemData->ItemIsEquipped;

			bool IsExistItem = false;
			int8 SlotIndex = 0;

			int16 FindItemGridPositionX = -1;
			int16 FindItemGridPositionY = -1;

			CItem* FindItem = MyPlayer->_InventoryManager.FindInventoryItem(0, CraftingItemInfo.ItemSmallCategory);
			if (FindItem == nullptr)
			{
				CItem* CraftingItem = G_ObjectManager->ItemCreate(FindReqCompleteItemData->ItemSmallCategory);
				CraftingItem->_ItemInfo = CraftingItemInfo;

				MyPlayer->_InventoryManager.InsertItem(0, CraftingItem);
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
			
			CItem* UseItem = MyPlayer->_InventoryManager.FindInventoryItem(0, (en_SmallItemCategory)UseItemSmallCategory);
			if (UseItem != nullptr)
			{
				switch (UseItem->_ItemInfo.ItemSmallCategory)
				{
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
				case en_SmallItemCategory::ITEM_SAMLL_CATEGORY_WEAPON_WOOD_SHIELD:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
					{
						st_GameObjectJob* DoItemEquipmentJob = MakeGameObjectJobOnEquipment(UseItem);
						MyPlayer->_GameObjectJobQue.Enqueue(DoItemEquipmentJob);
					}					
					break;									
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_SEED_POTATO:
					// 씨앗 배치 
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEALTH_RESTORATION_POTION_SMALL:
					{						
						// 체력 포션 사용
						st_GameObjectJob* HPHealJob = MakeGameObjectJobHPHeal(MyPlayer, UseItem->_ItemInfo.ItemHealPoint);
						MyPlayer->_GameObjectJobQue.Enqueue(HPHealJob);
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
						CMessage* CommonErrorPacket = MakePacketCommonError(en_PersonalMessageType::PERSONAL_FAULT_ITEM_USE, UseItem->_ItemInfo.ItemName.c_str());
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

void CGameServer::PacketProcReqUIMenuTileBuy(int64 SessionID, CMessage* Message)
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

			// 요청한 플레이어 시야범위 안에 있는 타일 정보 전송
			CMap* Map = G_MapManager->GetMap(1);
			if (Map != nullptr)
			{				
				vector<st_TileMapInfo> MapTileInfos = Map->FindMapTileInfo(MyPlayer);

				CMessage* ResMenuTileBuyPacket = MakePacketResMenuTileBuy(MapTileInfos);
				SendPacket(Session->SessionId, ResMenuTileBuyPacket);
				ResMenuTileBuyPacket->Free();
			}
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqTileBuy(int64 SessionID, CMessage* Message)
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

			int32 TilePositionX;
			*Message >> TilePositionX;
			int32 TilePositionY;
			*Message >> TilePositionY;

			st_Vector2Int TileBuyPosition;
			TileBuyPosition._X = TilePositionX;
			TileBuyPosition._Y = TilePositionY;

			CMap* Map = G_MapManager->GetMap(1);
			if (Map->ApplyTileUserAlloc(MyPlayer, TileBuyPosition) == true)
			{
				st_TileMapInfo TileMapInfo;
				TileMapInfo.TilePosition._X = TilePositionX;
				TileMapInfo.TilePosition._Y = TilePositionY;
				TileMapInfo.AccountID = AccountID;
				TileMapInfo.PlayerID = PlayerID;
				TileMapInfo.MapTileType = en_MapTileInfo::MAP_TILE_USER_ALLOC;

				CMessage* ResTileBuyMessage = MakePacketResTileBuy(TileMapInfo);
				SendPacketFieldOfView(MyPlayer, ResTileBuyMessage);
				ResTileBuyMessage->Free();
			}			
		} while (0);
	}
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
			*Message >> ItemPosition.CollisionPosition._X;
			*Message >> ItemPosition.CollisionPosition._Y;
			*Message >> ItemMoveDir;

			st_Vector2Int ItemCellPosition;
			ItemCellPosition._X = ItemPosition.CollisionPosition._X;
			ItemCellPosition._Y = ItemPosition.CollisionPosition._Y;

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

			int64 PlayerId;
			WCHAR PlayerName[100] = { 0 };
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
			float PlayerMagicHitRate;
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
				CPlayer* NewPlayerCharacter = G_ObjectManager->_PlayersArray[Session->MyPlayerIndexes[PlayerCount]];
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
				NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxSpeed = PlayerSpeed;
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
		// 요청한 게임오브젝트 타입
		int16 ReqGameObjectType;
		*Message >> ReqGameObjectType;

		// 요청한 캐릭터 슬롯 인덱스
		int8 ReqCharacterCreateSlotIndex;
		*Message >> ReqCharacterCreateSlotIndex;

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
			case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
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
					NewPlayerCharacter->_GameObjectInfo.ObjectStatInfo.MaxSpeed = NewCharacterStatus.Speed;
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

					// DB에 새로운 인벤토리 생성
					for (int16 Y = 0; Y < (int8)en_InventoryManager::INVENTORY_DEFAULT_HEIGHT_SIZE; Y++)
					{
						for (int16 X = 0; X < (int8)en_InventoryManager::INVENTORY_DEFAULT_WIDH_SIZE; X++)
						{
							CDBConnection* DBItemToInventoryConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
							SP::CDBGameServerItemCreateToInventory ItemToInventory(*DBItemToInventoryConnection);
							st_ItemInfo NewItem;

							ItemToInventory.InItemTileGridPositionX(X);
							ItemToInventory.InItemTileGridPositionY(Y);							
							ItemToInventory.InOwnerAccountId(Session->AccountId);
							ItemToInventory.InOwnerPlayerId(PlayerDBId);

							ItemToInventory.Execute();

							G_DBConnectionPool->Push(en_DBConnect::GAME, DBItemToInventoryConnection);
						}
					}

					// 골드 테이블 새로 생성
					CDBConnection* DBNewGoldTableCreateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
					SP::CDBGameServerGoldTableCreatePush GoldTableCreate(*DBNewGoldTableCreateConnection);

					GoldTableCreate.InAccountDBId(Session->AccountId);
					GoldTableCreate.InPlayerDBId(PlayerDBId);

					GoldTableCreate.Execute();

					G_DBConnectionPool->Push(en_DBConnect::GAME, DBNewGoldTableCreateConnection);					

					int16 DefaultKey = (int16)UnityEngine::Alpha1;

					// DB에 퀵슬롯바 생성
					for (int8 SlotIndex = 0; SlotIndex < (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SIZE; ++SlotIndex)
					{
						CDBConnection* DBQuickSlotCreateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
						SP::CDBGameServerQuickSlotBarSlotInit QuickSlotBarSlotCreate(*DBQuickSlotCreateConnection);

						int8 QuickSlotBarIndex = SlotIndex;
						int8 QuickSlotBarSlotIndex;
						int16 QuickSlotBarKey = 0;						

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

							QuickSlotBarSlotCreate.Execute();

							DefaultKey++;
						}

						G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotCreateConnection);
					}

					// 기본 스킬 생성
					PlayerDefaultSetting(Session->AccountId, NewPlayerCharacter->_GameObjectInfo, ReqCharacterCreateSlotIndex);								

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
			// 캐릭터가 소유하고 있는 스킬 정보를 DB로부터 읽어온다.			
			SP::CDBGameServerSkillGet CharacterSkillGet(*DBCharacterInfoGetConnection);
			CharacterSkillGet.InAccountDBId(MyPlayer->_AccountId);
			CharacterSkillGet.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);

			bool IsSkillLearn;
			bool IsQuickSlotUse;
			int8 SkillLargeCategory;
			int8 SkillMediumCategory;
			int16 SkillType;
			int8 SkillLevel;

			CharacterSkillGet.OutIsSkillLearn(IsSkillLearn);
			CharacterSkillGet.OutIsQuickSlotUse(IsQuickSlotUse);
			CharacterSkillGet.OutSkillLargeCategory(SkillLargeCategory);
			CharacterSkillGet.OutSkillMediumCategory(SkillMediumCategory);
			CharacterSkillGet.OutSkillType(SkillType);
			CharacterSkillGet.OutSkillLevel(SkillLevel);

			CharacterSkillGet.Execute();

			while (CharacterSkillGet.Fetch())
			{
				// 스킬 데이터 찾기
				st_SkillInfo* FindSkillData = G_Datamanager->FindSkillData((en_SkillMediumCategory)SkillMediumCategory, (en_SkillType)SkillType);
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
					CSkill* AttackSkill = G_ObjectManager->SkillCreate();

					st_AttackSkillInfo* FindAttackSkillData = (st_AttackSkillInfo*)FindSkillData;

					st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(FindAttackSkillData->SkillMediumCategory);

					AttackSkillInfo->IsSkillLearn = IsSkillLearn;
					AttackSkillInfo->IsQuickSlotUse = IsQuickSlotUse;
					AttackSkillInfo->SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;
					AttackSkillInfo->SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;
					AttackSkillInfo->SkillType = (en_SkillType)SkillType;
					AttackSkillInfo->SkillLevel = SkillLevel;
					AttackSkillInfo->SkillName = FindAttackSkillData->SkillName;

					if (AttackSkillInfo->SkillType != en_SkillType::SKILL_DEFAULT_ATTACK)
					{
						AttackSkillInfo->SkillCoolTime = FindAttackSkillData->SkillCoolTime;
					}
					else
					{
						AttackSkillInfo->SkillCoolTime = MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate;
					}

					AttackSkillInfo->SkillCastingTime = FindAttackSkillData->SkillCastingTime;
					AttackSkillInfo->SkillDurationTime = FindAttackSkillData->SkillDurationTime;
					AttackSkillInfo->SkillDotTime = FindAttackSkillData->SkillDotTime;
					AttackSkillInfo->SkillRemainTime = 0;
					AttackSkillInfo->SkillMinDamage = FindAttackSkillData->SkillMinDamage;
					AttackSkillInfo->SkillMaxDamage = FindAttackSkillData->SkillMaxDamage;
					AttackSkillInfo->SkillTargetEffectTime = FindAttackSkillData->SkillTargetEffectTime;
					AttackSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::UP, (*FindAttackSkillData->SkillAnimations.find(en_MoveDir::UP)).second));
					AttackSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::DOWN, (*FindAttackSkillData->SkillAnimations.find(en_MoveDir::DOWN)).second));
					AttackSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::LEFT, (*FindAttackSkillData->SkillAnimations.find(en_MoveDir::LEFT)).second));
					AttackSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::RIGHT, (*FindAttackSkillData->SkillAnimations.find(en_MoveDir::RIGHT)).second));
					AttackSkillInfo->NextComboSkill = FindAttackSkillData->NextComboSkill;
					AttackSkillInfo->SkillExplanation = FindAttackSkillData->SkillExplanation;

					TCHAR SkillExplanationMessage[256] = L"0";

					switch (AttackSkillInfo->SkillType)
					{
					case en_SkillType::SKILL_DEFAULT_ATTACK:
						wsprintf(SkillExplanationMessage, AttackSkillInfo->SkillExplanation.c_str());
						break;
					case en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK:
					case en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK:
					case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:					
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, AttackSkillInfo->SkillExplanation.c_str(), FindAttackSkillData->SkillDistance, AttackSkillInfo->SkillMinDamage, AttackSkillInfo->SkillMaxDamage);
						break;
					case en_SkillType::SKILL_KNIGHT_SHIELD_SMASH:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, AttackSkillInfo->SkillExplanation.c_str(), AttackSkillInfo->SkillMinDamage, AttackSkillInfo->SkillMaxDamage);
						break;
					case en_SkillType::SKILL_KNIGHT_SHAEHONE:
					case en_SkillType::SKILL_KNIGHT_CHOHONE:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, AttackSkillInfo->SkillExplanation.c_str(), FindAttackSkillData->SkillDistance, AttackSkillInfo->SkillDurationTime / 1000.0f, AttackSkillInfo->SkillMinDamage, AttackSkillInfo->SkillMaxDamage);						
						break;
					case en_SkillType::SKILL_SHAMAN_FLAME_HARPOON:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, AttackSkillInfo->SkillExplanation.c_str(), FindAttackSkillData->SkillDistance, AttackSkillInfo->SkillMinDamage, AttackSkillInfo->SkillMaxDamage);
						break;
					case en_SkillType::SKILL_SHAMAN_ROOT:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, AttackSkillInfo->SkillExplanation.c_str(), FindAttackSkillData->SkillDistance, AttackSkillInfo->SkillDurationTime / 1000.0f);
						break;
					case en_SkillType::SKILL_SHAMAN_ICE_CHAIN:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, AttackSkillInfo->SkillExplanation.c_str(), FindAttackSkillData->SkillDistance, AttackSkillInfo->SkillMinDamage, AttackSkillInfo->SkillMaxDamage, FindAttackSkillData->SkillDebufMovingSpeed, AttackSkillInfo->SkillDurationTime / 1000.0f);
						break;
					case en_SkillType::SKILL_SHAMAN_ICE_WAVE:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, AttackSkillInfo->SkillExplanation.c_str(), FindAttackSkillData->SkillDistance, AttackSkillInfo->SkillMinDamage, AttackSkillInfo->SkillMaxDamage);
						break;
					case en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, AttackSkillInfo->SkillExplanation.c_str(), FindAttackSkillData->SkillDistance, AttackSkillInfo->SkillMinDamage, AttackSkillInfo->SkillMaxDamage, AttackSkillInfo->SkillDurationTime / 1000.0f);
						break;
					case en_SkillType::SKILL_SHAMAN_HELL_FIRE:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, AttackSkillInfo->SkillExplanation.c_str(), FindAttackSkillData->SkillDistance, AttackSkillInfo->SkillMinDamage, AttackSkillInfo->SkillMaxDamage);
						break;
					case en_SkillType::SKILL_TAIOIST_DIVINE_STRIKE:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, AttackSkillInfo->SkillExplanation.c_str(), FindAttackSkillData->SkillDistance, AttackSkillInfo->SkillMinDamage, AttackSkillInfo->SkillMaxDamage);
						break;
					case en_SkillType::SKILL_TAIOIST_ROOT:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, AttackSkillInfo->SkillExplanation.c_str(), FindAttackSkillData->SkillDistance, AttackSkillInfo->SkillDurationTime / 1000.0f);
						break;
					}

					AttackSkillInfo->SkillExplanation = SkillExplanationMessage;
					AttackSkillInfo->SkillDebufAttackSpeed = FindAttackSkillData->SkillDebufAttackSpeed;
					AttackSkillInfo->SkillDebufMovingSpeed = FindAttackSkillData->SkillDebufMovingSpeed;
					AttackSkillInfo->StatusAbnormalityProbability = FindAttackSkillData->StatusAbnormalityProbability;

					AttackSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL_COOLTIME, AttackSkillInfo);

					MyPlayer->_SkillBox.AddAttackSkill(AttackSkill);
				}
				break;
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_TACTIC:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_TACTIC:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_TACTIC:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_TACTIC:
				{
					CSkill* TacTicSkill = G_ObjectManager->SkillCreate();

					st_TacTicSkillInfo* FindTacTicSkillData = (st_TacTicSkillInfo*)FindSkillData;

					st_TacTicSkillInfo* TacTicSkillInfo = (st_TacTicSkillInfo*)G_ObjectManager->SkillInfoCreate(FindTacTicSkillData->SkillMediumCategory);
					TacTicSkillInfo->CanSkillUse = true;
					TacTicSkillInfo->IsSkillLearn = IsSkillLearn;
					TacTicSkillInfo->IsQuickSlotUse = IsQuickSlotUse;
					TacTicSkillInfo->SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;
					TacTicSkillInfo->SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;
					TacTicSkillInfo->SkillType = (en_SkillType)SkillType;
					TacTicSkillInfo->SkillLevel = SkillLevel;
					TacTicSkillInfo->SkillName = FindTacTicSkillData->SkillName;
					TacTicSkillInfo->SkillCoolTime = FindTacTicSkillData->SkillCoolTime;
					TacTicSkillInfo->SkillCastingTime = FindTacTicSkillData->SkillCastingTime;
					TacTicSkillInfo->SkillDurationTime = FindTacTicSkillData->SkillDurationTime;
					TacTicSkillInfo->SkillDotTime = FindTacTicSkillData->SkillDotTime;
					TacTicSkillInfo->SkillRemainTime = 0;					
					TacTicSkillInfo->SkillTargetEffectTime = FindTacTicSkillData->SkillTargetEffectTime;
					TacTicSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::UP, (*FindTacTicSkillData->SkillAnimations.find(en_MoveDir::UP)).second));
					TacTicSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::DOWN, (*FindTacTicSkillData->SkillAnimations.find(en_MoveDir::DOWN)).second));
					TacTicSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::LEFT, (*FindTacTicSkillData->SkillAnimations.find(en_MoveDir::LEFT)).second));
					TacTicSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::RIGHT, (*FindTacTicSkillData->SkillAnimations.find(en_MoveDir::RIGHT)).second));

					TacTicSkillInfo->SkillExplanation = FindTacTicSkillData->SkillExplanation;

					TCHAR SkillExplanationMessage[256] = L"0";

					switch (TacTicSkillInfo->SkillType)
					{
					case en_SkillType::SKILL_SHAMAN_BACK_TELEPORT:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, TacTicSkillInfo->SkillExplanation.c_str(), FindTacTicSkillData->SkillDistance);
						break;
					}

					TacTicSkillInfo->SkillExplanation = SkillExplanationMessage;

					TacTicSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL_COOLTIME, TacTicSkillInfo);

					MyPlayer->_SkillBox.AddTacTicSkill(TacTicSkill);
				}
				break;
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_HEAL:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_HEAL:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL:
				{
					CSkill* HealSkill = G_ObjectManager->SkillCreate();

					st_HealSkillInfo* FindHealSkillData = (st_HealSkillInfo*)FindSkillData;

					st_HealSkillInfo* HealSkillInfo = (st_HealSkillInfo*)G_ObjectManager->SkillInfoCreate(FindHealSkillData->SkillMediumCategory);
					HealSkillInfo->CanSkillUse = true;
					HealSkillInfo->IsSkillLearn = IsSkillLearn;
					HealSkillInfo->IsQuickSlotUse = IsQuickSlotUse;
					HealSkillInfo->SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;
					HealSkillInfo->SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;
					HealSkillInfo->SkillType = (en_SkillType)SkillType;
					HealSkillInfo->SkillLevel = SkillLevel;
					HealSkillInfo->SkillName = FindHealSkillData->SkillName;
					HealSkillInfo->SkillCoolTime = FindHealSkillData->SkillCoolTime;
					HealSkillInfo->SkillCastingTime = FindHealSkillData->SkillCastingTime;
					HealSkillInfo->SkillDurationTime = FindHealSkillData->SkillDurationTime;
					HealSkillInfo->SkillDotTime = FindHealSkillData->SkillDotTime;
					HealSkillInfo->SkillRemainTime = 0;
					HealSkillInfo->SkillMinHealPoint = FindHealSkillData->SkillMinHealPoint;
					HealSkillInfo->SkillMaxHealPoint = FindHealSkillData->SkillMaxHealPoint;
					HealSkillInfo->SkillTargetEffectTime = FindHealSkillData->SkillTargetEffectTime;
					HealSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::UP, (*FindHealSkillData->SkillAnimations.find(en_MoveDir::UP)).second));
					HealSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::DOWN, (*FindHealSkillData->SkillAnimations.find(en_MoveDir::DOWN)).second));
					HealSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::LEFT, (*FindHealSkillData->SkillAnimations.find(en_MoveDir::LEFT)).second));
					HealSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::RIGHT, (*FindHealSkillData->SkillAnimations.find(en_MoveDir::RIGHT)).second));

					HealSkillInfo->SkillExplanation = FindHealSkillData->SkillExplanation;

					TCHAR SkillExplanationMessage[256] = L"0";

					switch (HealSkillInfo->SkillType)
					{
					case en_SkillType::SKILL_TAIOIST_HEALING_LIGHT:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, HealSkillInfo->SkillExplanation.c_str(), FindHealSkillData->SkillDistance, HealSkillInfo->SkillMinHealPoint, HealSkillInfo->SkillMaxHealPoint);
						break;
					case en_SkillType::SKILL_TAIOIST_HEALING_WIND:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, HealSkillInfo->SkillExplanation.c_str(), FindHealSkillData->SkillDistance, HealSkillInfo->SkillMinHealPoint, HealSkillInfo->SkillMaxHealPoint);
						break;
					}

					HealSkillInfo->SkillExplanation = SkillExplanationMessage;

					HealSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL_COOLTIME, HealSkillInfo);

					MyPlayer->_SkillBox.AddTacTicSkill(HealSkill);
				}
				break;
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_BUF:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_BUF:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_BUF:
				{
					CSkill* BufSkill = G_ObjectManager->SkillCreate();

					st_BufSkillInfo* FindBufSkillData = (st_BufSkillInfo*)FindSkillData;

					st_BufSkillInfo* BufSkillInfo = (st_BufSkillInfo*)G_ObjectManager->SkillInfoCreate(FindBufSkillData->SkillMediumCategory);
					BufSkillInfo->CanSkillUse = true;
					BufSkillInfo->IsSkillLearn = IsSkillLearn;
					BufSkillInfo->IsQuickSlotUse = IsQuickSlotUse;
					BufSkillInfo->SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;
					BufSkillInfo->SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;
					BufSkillInfo->SkillType = (en_SkillType)SkillType;
					BufSkillInfo->SkillLevel = SkillLevel;
					BufSkillInfo->SkillName = FindBufSkillData->SkillName;
					BufSkillInfo->SkillCoolTime = FindBufSkillData->SkillCoolTime;
					BufSkillInfo->SkillCastingTime = FindBufSkillData->SkillCastingTime;
					BufSkillInfo->SkillDurationTime = FindBufSkillData->SkillDurationTime;
					BufSkillInfo->SkillDotTime = FindBufSkillData->SkillDotTime;
					BufSkillInfo->SkillRemainTime = 0;
					BufSkillInfo->SkillTargetEffectTime = FindBufSkillData->SkillTargetEffectTime;
					BufSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::UP, (*FindBufSkillData->SkillAnimations.find(en_MoveDir::UP)).second));
					BufSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::DOWN, (*FindBufSkillData->SkillAnimations.find(en_MoveDir::DOWN)).second));
					BufSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::LEFT, (*FindBufSkillData->SkillAnimations.find(en_MoveDir::LEFT)).second));
					BufSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::RIGHT, (*FindBufSkillData->SkillAnimations.find(en_MoveDir::RIGHT)).second));

					BufSkillInfo->SkillExplanation = FindBufSkillData->SkillExplanation;
					TCHAR SkillExplanationMessage[256] = L"0";

					switch (BufSkillInfo->SkillType)
					{
					case en_SkillType::SKILL_SHOCK_RELEASE:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, BufSkillInfo->SkillExplanation.c_str(), FindBufSkillData->SkillDurationTime / 1000.0f);
						break;
					case en_SkillType::SKILL_KNIGHT_CHARGE_POSE:
						_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, BufSkillInfo->SkillExplanation.c_str(), FindBufSkillData->IncreaseMaxAttackPoint, FindBufSkillData->SkillDurationTime / 1000.0f);
						break;
					}

					BufSkillInfo->SkillExplanation = SkillExplanationMessage;

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

					BufSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL_COOLTIME, BufSkillInfo);

					MyPlayer->_SkillBox.AddBufSkill(BufSkill);
				}
				break;
				}
			}

			// 공격 스킬 개수
			*ResCharacterInfoMessage << (int8)MyPlayer->_SkillBox.GetAttackSkill().size();
			// 전술 스킬 개수
			*ResCharacterInfoMessage << (int8)MyPlayer->_SkillBox.GetTacTicSkill().size();
			// 버프 스킬 개수
			*ResCharacterInfoMessage << (int8)MyPlayer->_SkillBox.GetBufSkill().size();

			// 공격 스킬 정보 담기
			for (CSkill* AttackSkillInfo : MyPlayer->_SkillBox.GetAttackSkill())
			{
				*ResCharacterInfoMessage << *(AttackSkillInfo->GetSkillInfo());
			}

			// 전술 스킬 정보 담기
			for (CSkill* TacTicSkillInfo : MyPlayer->_SkillBox.GetTacTicSkill())
			{
				*ResCharacterInfoMessage << *(TacTicSkillInfo->GetSkillInfo());
			}

			// 버프 스킬 정보 담기
			for (CSkill* BufSkillInfo : MyPlayer->_SkillBox.GetBufSkill())
			{
				*ResCharacterInfoMessage << *(BufSkillInfo->GetSkillInfo());
			}			
#pragma endregion

#pragma region 가방 아이템 정보 읽어오기			
			// 인벤토리 생성
			MyPlayer->_InventoryManager.InventoryCreate(1, (int8)en_InventoryManager::INVENTORY_DEFAULT_WIDH_SIZE, (int8)en_InventoryManager::INVENTORY_DEFAULT_HEIGHT_SIZE);

			*ResCharacterInfoMessage << (int8)en_InventoryManager::INVENTORY_DEFAULT_WIDH_SIZE;
			*ResCharacterInfoMessage << (int8)en_InventoryManager::INVENTORY_DEFAULT_HEIGHT_SIZE;

			vector<CItem*> InventoryItems;

			// DB에 기록되어 있는 인벤토리 아이템들의 정보를 모두 긁어온다.			
			SP::CDBGameServerInventoryItemGet CharacterInventoryItem(*DBCharacterInfoGetConnection);
			CharacterInventoryItem.InAccountDBId(MyPlayer->_AccountId);
			CharacterInventoryItem.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);

			bool ItemQuickSlotUse = false;
			bool ItemEquipped = false;
			int16 ItemWidth = 0;
			int16 ItemHeight = 0;
			int16 ItemTilePositionX = 0;
			int16 ItemTilePositionY = 0;
			int8 ItemLargeCategory = 0;
			int8 ItemMediumCategory = 0;
			int16 ItemSmallCategory = 0;			
			int16 ItemCount = 0;	
			int32 ItemDurability = 0;
			int8 ItemEnchantPoint = 0;

			CharacterInventoryItem.OutItemIsQuickSlotUse(ItemQuickSlotUse);
			CharacterInventoryItem.OutItemIsEquipped(ItemEquipped);
			CharacterInventoryItem.OutItemWidth(ItemWidth);
			CharacterInventoryItem.OutItemHeight(ItemHeight);						
			CharacterInventoryItem.OutItemTileGridPositionX(ItemTilePositionX);
			CharacterInventoryItem.OutItemTileGridPositionY(ItemTilePositionY);
			CharacterInventoryItem.OutItemLargeCategory(ItemLargeCategory);
			CharacterInventoryItem.OutItemMediumCategory(ItemMediumCategory);
			CharacterInventoryItem.OutItemSmallCategory(ItemSmallCategory);			
			CharacterInventoryItem.OutItemCount(ItemCount);
			CharacterInventoryItem.OutItemDurability(ItemDurability);
			CharacterInventoryItem.OutItemEnchantPoint(ItemEnchantPoint);		

			CharacterInventoryItem.Execute();

			while (CharacterInventoryItem.Fetch())
			{
				CItem* NewItem = G_ObjectManager->ItemCreate((en_SmallItemCategory)ItemSmallCategory);		

				if (NewItem != nullptr)
				{
					st_ItemInfo NewItemInfo = *(G_Datamanager->FindItemData((en_SmallItemCategory)ItemSmallCategory));
					
					NewItemInfo.ItemIsQuickSlotUse = ItemQuickSlotUse;
					NewItemInfo.ItemIsEquipped = ItemEquipped;
					NewItemInfo.ItemWidth = ItemWidth;
					NewItemInfo.ItemHeight = ItemHeight;
					NewItemInfo.ItemTileGridPositionX = ItemTilePositionX;
					NewItemInfo.ItemTileGridPositionY = ItemTilePositionY;
					NewItemInfo.ItemCount = ItemCount;
					NewItemInfo.ItemCurrentDurability = ItemDurability;
					NewItemInfo.ItemEnchantPoint = ItemEnchantPoint;
					NewItem->_ItemInfo = NewItemInfo;

					MyPlayer->_InventoryManager.DBItemInsertItem(0, NewItem);
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

#pragma region 장비 아이템 정보 읽어오기

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
			int16 QuickSlotKey;
			int8 QuickSlotSkillLargeCategory;
			int8 QuickSlotSkillMediumCategory;
			int16 QuickSlotSkillType;
			int8 QuickSlotSkillLevel;
			int8 QuickSlotItemLargeCategory;
			int8 QuickSlotItemMediumCategory;
			int16 QuickSlotItemSmallCategory;
			int16 QuickSlotItemCount;

			QuickSlotBarGet.OutQuickSlotBarIndex(QuickSlotBarIndex);
			QuickSlotBarGet.OutQuickSlotBarItemIndex(QuickSlotBarSlotIndex);
			QuickSlotBarGet.OutQuickSlotKey(QuickSlotKey);
			QuickSlotBarGet.OutQuickSlotSkillLargeCategory(QuickSlotSkillLargeCategory);
			QuickSlotBarGet.OutQuickSlotSkillMediumCategory(QuickSlotSkillMediumCategory);
			QuickSlotBarGet.OutQuickSlotSkillType(QuickSlotSkillType);
			QuickSlotBarGet.OutQuickSlotSkillLevel(QuickSlotSkillLevel);
			QuickSlotBarGet.OutQuickSlotItemLargeCategory(QuickSlotItemLargeCategory);
			QuickSlotBarGet.OutQuickSlotItemMediumCategory(QuickSlotItemMediumCategory);
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
				NewQuickSlotBarSlot.QuickSlotKey = QuickSlotKey;				

				CSkill* FindSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)QuickSlotSkillType);
				if (FindSkill != nullptr)
				{
					NewQuickSlotBarSlot.QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_SKILL;
					NewQuickSlotBarSlot.QuickBarSkill = FindSkill;
				}
				else
				{
					NewQuickSlotBarSlot.QuickBarSkill = nullptr;
				}

				CItem* FindItem = MyPlayer->_InventoryManager.FindInventoryItem(0, (en_SmallItemCategory)QuickSlotItemSmallCategory);
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

#pragma endregion

#pragma region 장비 정보 보내주기
			vector<st_ItemInfo> Equipments;

			// 장비 정보 담기
			*ResCharacterInfoMessage << (int8)Equipments.size();
			for (st_ItemInfo EquipmentItemInfo : Equipments)
			{
				*ResCharacterInfoMessage << EquipmentItemInfo;
			}
#pragma endregion


#pragma region 골드 정보 읽어오기
			// 캐릭터가 소유하고 있었던 골드 정보를 GoldTable에서 읽어온다.			
			SP::CDBGameServerGoldGet CharacterGoldGet(*DBCharacterInfoGetConnection);
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
				MyPlayer->_InventoryManager._GoldCoinCount = GoldCoin;
				MyPlayer->_InventoryManager._SliverCoinCount = SliverCoin;
				MyPlayer->_InventoryManager._BronzeCoinCount = BronzeCoin;				

				// 골드 정보 담기
				*ResCharacterInfoMessage << GoldCoin;
				*ResCharacterInfoMessage << SliverCoin;
				*ResCharacterInfoMessage << BronzeCoin;				
			}			
#pragma endregion	

#pragma region 조합템 정보 보내기			
			vector<st_CraftingItemCategory> CraftingItemCategorys;

			for (int8 Category = (int8)en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARCHITECTURE;
				Category <= (int8)en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL; ++Category)
			{
				auto FindCraftingIterator = G_Datamanager->_CraftingData.find(Category);
				if (FindCraftingIterator == G_Datamanager->_CraftingData.end())
				{
					continue;
				}

				st_CraftingItemCategory* CraftingCategory = (*FindCraftingIterator).second;				
				CraftingItemCategorys.push_back(*CraftingCategory);
			}

			*ResCharacterInfoMessage << (int8)CraftingItemCategorys.size();

			for (st_CraftingItemCategory CraftingItemCategory : CraftingItemCategorys)
			{
				*ResCharacterInfoMessage << CraftingItemCategory;				
			}						
#pragma endregion

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterInfoGetConnection);

#pragma region 제작대 조합템 정보 보내기
			vector<st_CraftingTableRecipe*> CraftingTables;

			for (int16 CraftingTableType = (int16)en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE;
				CraftingTableType <= (int16)en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL; CraftingTableType++)
			{
				auto FindFurnaceCraftingTable = G_Datamanager->_CraftingTableData.find(CraftingTableType);
				st_CraftingTableRecipe* FurnaceCraftingTable = (*FindFurnaceCraftingTable).second;

				CraftingTables.push_back(FurnaceCraftingTable);
			}			

			*ResCharacterInfoMessage << (int8)CraftingTables.size();

			for (st_CraftingTableRecipe* CraftingTable : CraftingTables)
			{
				*ResCharacterInfoMessage << *CraftingTable;
			}
#pragma endregion
						
#pragma region Day 시간 보내기			
			st_Day DayInfo = _Day->GetDayInfo();
			*ResCharacterInfoMessage << DayInfo.DayTimeCycle;
			*ResCharacterInfoMessage << DayInfo.DayTimeCheck;
			*ResCharacterInfoMessage << DayInfo.DayRatio;
			*ResCharacterInfoMessage << (int8)DayInfo.DayType;

#pragma endregion

			SendPacket(MyPlayer->_SessionId, ResCharacterInfoMessage);

			MyPlayer->_NetworkState = en_ObjectNetworkState::LIVE;			

		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBLeavePlayerInfoSave(CGameServerMessage* Message)
{
	st_Session* LeaveSession;
	*Message >> &LeaveSession;
		
	CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[LeaveSession->MyPlayerIndex];	

	CDBConnection* PlayerInfoSaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
	SP::CDBGameServerLeavePlayerStatInfoSave LeavePlayerStatInfoSave(*PlayerInfoSaveDBConnection);

	LeavePlayerStatInfoSave.InAccountDBId(MyPlayer->_AccountId);
	LeavePlayerStatInfoSave.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);
	LeavePlayerStatInfoSave.InLevel(MyPlayer->_GameObjectInfo.ObjectStatInfo.Level);
	LeavePlayerStatInfoSave.InMaxHP(MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxHP);
	LeavePlayerStatInfoSave.InMaxMP(MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMP);
	LeavePlayerStatInfoSave.InMaxDP(MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxDP);
	LeavePlayerStatInfoSave.InAutoRecoveryHPPercent(MyPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent);
	LeavePlayerStatInfoSave.InAutoRecoveryMPPercent(MyPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent);
	LeavePlayerStatInfoSave.InMinMeleeAttackDamage(MyPlayer->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage);
	LeavePlayerStatInfoSave.InMaxMeleeAttackDamage(MyPlayer->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage);
	LeavePlayerStatInfoSave.InMeleeAttackHitRate(MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate);
	LeavePlayerStatInfoSave.InMagicDamage(MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage);
	LeavePlayerStatInfoSave.InMagicHitRate(MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicHitRate);
	LeavePlayerStatInfoSave.InDefence(MyPlayer->_GameObjectInfo.ObjectStatInfo.Defence);
	LeavePlayerStatInfoSave.InEvasionRate(MyPlayer->_GameObjectInfo.ObjectStatInfo.EvasionRate);
	LeavePlayerStatInfoSave.InMeleeCriticalPoint(MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint);
	LeavePlayerStatInfoSave.InMagicCriticalPoint(MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint);
	LeavePlayerStatInfoSave.InSpeed(MyPlayer->_GameObjectInfo.ObjectStatInfo.Speed);
	LeavePlayerStatInfoSave.InLastPositionY(MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y);
	LeavePlayerStatInfoSave.InLastPositionX(MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X);
	LeavePlayerStatInfoSave.InCurrentExperience(MyPlayer->_Experience.CurrentExperience);
	LeavePlayerStatInfoSave.InRequireExperience(MyPlayer->_Experience.RequireExperience);
	LeavePlayerStatInfoSave.InTotalExperience(MyPlayer->_Experience.TotalExperience);

	LeavePlayerStatInfoSave.Execute();		

	// 퀵슬롯 정보 업데이트
	for (auto QuickSlotIterator : MyPlayer->_QuickSlotManager.GetQuickSlotBar())
	{
		for (auto QuickSlotBarSlotIterator : QuickSlotIterator.second->_QuickSlotBarSlotInfos)
		{
			st_QuickSlotBarSlotInfo* SaveQuickSlotInfo = QuickSlotBarSlotIterator.second;

			switch (SaveQuickSlotInfo->QuickSlotBarType)
			{
			case en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_NONE:
				{
					SP::CDBGameServerQuickSlotInit QuickSlotInit(*PlayerInfoSaveDBConnection);

					QuickSlotInit.InAccountDBId(MyPlayer->_AccountId);
					QuickSlotInit.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);
					QuickSlotInit.InQuickSlotBarIndex(SaveQuickSlotInfo->QuickSlotBarIndex);
					QuickSlotInit.InQuickSlotBarSlotIndex(SaveQuickSlotInfo->QuickSlotBarSlotIndex);

					QuickSlotInit.Execute();
				}
				break;
			case en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_SKILL:
				{
					SP::CDBGameServerQuickSlotBarSlotUpdate QuickSlotDBUpdate(*PlayerInfoSaveDBConnection);

					int8 SaveQuickSlotInfoSkillLargeCategory = (int8)SaveQuickSlotInfo->QuickBarSkill->GetSkillInfo()->SkillLargeCategory;
					int8 SaveQuickSlotInfoSkillMediumCategory = (int8)SaveQuickSlotInfo->QuickBarSkill->GetSkillInfo()->SkillMediumCategory;
					int16 SaveQuickSlotInfoSkillSkillType = (int8)SaveQuickSlotInfo->QuickBarSkill->GetSkillInfo()->SkillType;

					int8 SaveItemLargeCategory = 0;
					int8 SaveItemMediumCategory = 0;
					int16 SaveItemSmallCategory = 0;
					int16 SaveItemCount = 0;

					QuickSlotDBUpdate.InAccountDBId(MyPlayer->_AccountId);
					QuickSlotDBUpdate.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);
					QuickSlotDBUpdate.InQuickSlotBarIndex(SaveQuickSlotInfo->QuickSlotBarIndex);
					QuickSlotDBUpdate.InQuickSlotBarSlotIndex(SaveQuickSlotInfo->QuickSlotBarSlotIndex);
					QuickSlotDBUpdate.InQuickSlotKey(SaveQuickSlotInfo->QuickSlotKey);
					QuickSlotDBUpdate.InSkillLargeCategory(SaveQuickSlotInfoSkillLargeCategory);
					QuickSlotDBUpdate.InSkillMediumCategory(SaveQuickSlotInfoSkillMediumCategory);
					QuickSlotDBUpdate.InSkillType(SaveQuickSlotInfoSkillSkillType);
					QuickSlotDBUpdate.InSkillLevel(SaveQuickSlotInfo->QuickBarSkill->GetSkillInfo()->SkillLevel);
					QuickSlotDBUpdate.InItemLargeCategory(SaveItemLargeCategory);
					QuickSlotDBUpdate.InItemMediumCategory(SaveItemMediumCategory);
					QuickSlotDBUpdate.InItemSmallCategory(SaveItemSmallCategory);
					QuickSlotDBUpdate.InItemCount(SaveItemCount);

					QuickSlotDBUpdate.Execute();
				}
				break;
			case en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_ITEM:
				{
					SP::CDBGameServerQuickSlotBarSlotUpdate QuickSlotDBUpdate(*PlayerInfoSaveDBConnection);

					int8 SaveQuickSlotInfoSkillLargeCategory = 0;
					int8 SaveQuickSlotInfoSkillMediumCategory = 0;
					int16 SaveQuickSlotInfoSkillSkillType = 0;
					int8 SkillLevel = 0;

					int8 SaveItemLargeCategory = (int8)SaveQuickSlotInfo->QuickBarItem->_ItemInfo.ItemLargeCategory;
					int8 SaveItemMediumCategory = (int8)SaveQuickSlotInfo->QuickBarItem->_ItemInfo.ItemMediumCategory;
					int16 SaveItemSmallCategory = (int16)SaveQuickSlotInfo->QuickBarItem->_ItemInfo.ItemSmallCategory;
					int16 SaveItemCount = SaveQuickSlotInfo->QuickBarItem->_ItemInfo.ItemCount;

					QuickSlotDBUpdate.InAccountDBId(MyPlayer->_AccountId);
					QuickSlotDBUpdate.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);
					QuickSlotDBUpdate.InQuickSlotBarIndex(SaveQuickSlotInfo->QuickSlotBarIndex);
					QuickSlotDBUpdate.InQuickSlotBarSlotIndex(SaveQuickSlotInfo->QuickSlotBarSlotIndex);
					QuickSlotDBUpdate.InQuickSlotKey(SaveQuickSlotInfo->QuickSlotKey);
					QuickSlotDBUpdate.InSkillLargeCategory(SaveQuickSlotInfoSkillLargeCategory);
					QuickSlotDBUpdate.InSkillMediumCategory(SaveQuickSlotInfoSkillMediumCategory);
					QuickSlotDBUpdate.InSkillType(SaveQuickSlotInfoSkillSkillType);
					QuickSlotDBUpdate.InSkillLevel(SkillLevel);
					QuickSlotDBUpdate.InItemLargeCategory(SaveItemLargeCategory);
					QuickSlotDBUpdate.InItemMediumCategory(SaveItemMediumCategory);
					QuickSlotDBUpdate.InItemSmallCategory(SaveItemSmallCategory);
					QuickSlotDBUpdate.InItemCount(SaveItemCount);

					QuickSlotDBUpdate.Execute();
				}
				break;			
			}			
		}
	}

	// 장비 정보 DB에 저장
	CItem** EquipmentPartsItem = MyPlayer->_Equipment.GetEquipmentParts();
	for (int8 i = 1; i <= (int8)en_EquipmentParts::EQUIPMENT_PARTS_BOOT; i++)
	{
		if (EquipmentPartsItem[i] != nullptr)
		{
			SP::CDBGameServerOnEquipment SaveEquipmentInfo(*PlayerInfoSaveDBConnection);
			
			int8 EquipmentParts = (int8)EquipmentPartsItem[i]->_ItemInfo.ItemEquipmentPart;
			int8 EquipmentLargeCategory = (int8)EquipmentPartsItem[i]->_ItemInfo.ItemLargeCategory;
			int8 EquipmentMediumCategory = (int8)EquipmentPartsItem[i]->_ItemInfo.ItemMediumCategory;
			int16 EquipmentSmallCategory = (int16)EquipmentPartsItem[i]->_ItemInfo.ItemSmallCategory;

			SaveEquipmentInfo.InAccountDBID(MyPlayer->_AccountId);
			SaveEquipmentInfo.InPlayerDBID(MyPlayer->_GameObjectInfo.ObjectId);
			SaveEquipmentInfo.InEquipmentParts(EquipmentParts);
			SaveEquipmentInfo.InEquipmentLargeCategory(EquipmentLargeCategory);
			SaveEquipmentInfo.InEquipmentMediumCategory(EquipmentMediumCategory);
			SaveEquipmentInfo.InEquipmentSmallCategory(EquipmentSmallCategory);
			SaveEquipmentInfo.InEquipmentDurability(EquipmentPartsItem[i]->_ItemInfo.ItemCurrentDurability);
			SaveEquipmentInfo.InEquipmentEnchantPoint(EquipmentPartsItem[i]->_ItemInfo.ItemEnchantPoint);

			SaveEquipmentInfo.Execute();
		}
		else
		{
			SP::CDBGameServerOffEquipment OffEquipment(*PlayerInfoSaveDBConnection);

			int8 EquipmentParts = i;

			OffEquipment.InAccountDBID(MyPlayer->_AccountId);
			OffEquipment.InPlayerDBID(MyPlayer->_GameObjectInfo.ObjectId);
			OffEquipment.InEquipmentParts(EquipmentParts);
		}
	}


	// 가방 정보 DB에 저장	
	CInventory** MyPlayerInventorys = MyPlayer->_InventoryManager.GetInventory();	

	// 가방 DB 청소 후 새로 저장
	for (int i = 0; i < MyPlayer->_InventoryManager.GetInventoryCount(); i++)
	{
		SP::CDBGameServerInventoryAllSlotInit InventoryAllSlotInit(*PlayerInfoSaveDBConnection);
		InventoryAllSlotInit.InOwnerAccountID(MyPlayer->_AccountId);
		InventoryAllSlotInit.InOwnerPlayerID(MyPlayer->_GameObjectInfo.ObjectId);
		InventoryAllSlotInit.InInventoryWidth(MyPlayerInventorys[i]->_InventoryWidth);
		InventoryAllSlotInit.InInventoryHeight(MyPlayerInventorys[i]->_InventoryHeight);

		InventoryAllSlotInit.Execute();

		if (MyPlayerInventorys[i] != nullptr)
		{
			SP::CDBGameServerInventoryPlace LeavePlayerInventoryItemSave(*PlayerInfoSaveDBConnection);
			LeavePlayerInventoryItemSave.InOwnerAccountId(MyPlayer->_AccountId);
			LeavePlayerInventoryItemSave.InOwnerPlayerId(MyPlayer->_GameObjectInfo.ObjectId);

			vector<st_ItemInfo> PlayerInventoryItems = MyPlayerInventorys[i]->DBInventorySaveReturnItems();

			for (st_ItemInfo InventoryItem : PlayerInventoryItems)
			{				
				int8 InventoryItemLargeCategory = (int8)InventoryItem.ItemLargeCategory;
				int8 InventoryItemMediumCategory = (int8)InventoryItem.ItemMediumCategory;
				int16 InventoryItemSmallCategory = (int16)InventoryItem.ItemSmallCategory;

				LeavePlayerInventoryItemSave.InItemIsQuickSlotUse(InventoryItem.ItemIsQuickSlotUse);
				LeavePlayerInventoryItemSave.InIsEquipped(InventoryItem.ItemIsEquipped);
				LeavePlayerInventoryItemSave.InItemWidth(InventoryItem.ItemWidth);
				LeavePlayerInventoryItemSave.InItemHeight(InventoryItem.ItemHeight);
				LeavePlayerInventoryItemSave.InItemTileGridPositionX(InventoryItem.ItemTileGridPositionX);
				LeavePlayerInventoryItemSave.InItemTileGridPositionY(InventoryItem.ItemTileGridPositionY);
				LeavePlayerInventoryItemSave.InItemLargeCategory(InventoryItemLargeCategory);
				LeavePlayerInventoryItemSave.InItemMediumCategory(InventoryItemMediumCategory);
				LeavePlayerInventoryItemSave.InItemSmallCategory(InventoryItemSmallCategory);				
				LeavePlayerInventoryItemSave.InItemCount(InventoryItem.ItemCount);				
				LeavePlayerInventoryItemSave.InItemDurability(InventoryItem.ItemCurrentDurability);
				LeavePlayerInventoryItemSave.InItemEnchantPoint(InventoryItem.ItemEnchantPoint);

				LeavePlayerInventoryItemSave.Execute();
			}
		}
	}		

	G_DBConnectionPool->Push(en_DBConnect::GAME, PlayerInfoSaveDBConnection);			

	if (MyPlayer->GetChannel() != nullptr)
	{
		CMap* LeaveMap = G_MapManager->GetMap(1);
		CChannel* LeaveChannel = LeaveMap->GetChannelManager()->Find(1);

		st_GameObjectJob* DeSpawnMonsterChannelJob = MakeGameObjectJobObjectDeSpawnObjectChannel(MyPlayer);
		LeaveChannel->_ChannelJobQue.Enqueue(DeSpawnMonsterChannelJob);

		st_GameObjectJob* LeaveGameJob = MakeGameObjectJobLeaveChannelPlayer(MyPlayer, LeaveSession->MyPlayerIndexes);
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
	LeaveChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_PLAYER_LEAVE_CHANNEL;

	CGameServerMessage* LeaveChannelMessage = CGameServerMessage::GameServerMessageAlloc();
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

st_GameObjectJob* CGameServer::MakeGameObjectJobComboSkillCreate(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, CSkill* ComboSkill)
{
	st_GameObjectJob* ComboAttackCreateJob = G_ObjectManager->GameObjectJobCreate();
	ComboAttackCreateJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_COMBO_ATTACK_CREATE;

	CGameServerMessage* ComboAttackCreateMessage = CGameServerMessage::GameServerMessageAlloc();
	ComboAttackCreateMessage->Clear();

	*ComboAttackCreateMessage << QuickSlotBarIndex;
	*ComboAttackCreateMessage << QuickSlotBarSlotIndex;
	*ComboAttackCreateMessage << &ComboSkill;

	ComboAttackCreateJob->GameObjectJobMessage = ComboAttackCreateMessage;

	return ComboAttackCreateJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobComboSkillOff()
{
	st_GameObjectJob* ComboAttackOffJob = G_ObjectManager->GameObjectJobCreate();
	ComboAttackOffJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_COMBO_ATTACK_OFF;

	ComboAttackOffJob->GameObjectJobMessage = nullptr;

	return ComboAttackOffJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobSpellStart(CSkill* StartSpellSkill)
{
	st_GameObjectJob* SpellStartJob = G_ObjectManager->GameObjectJobCreate();
	SpellStartJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SPELL_START;

	CGameServerMessage* SpellStartMessage = CGameServerMessage::GameServerMessageAlloc();
	SpellStartMessage->Clear();

	*SpellStartMessage << &StartSpellSkill;

	SpellStartJob->GameObjectJobMessage = SpellStartMessage;

	return SpellStartJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobSpellCancel()
{
	st_GameObjectJob* SpellCancelJob = G_ObjectManager->GameObjectJobCreate();
	SpellCancelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SPELL_CANCEL;

	SpellCancelJob->GameObjectJobMessage = nullptr;

	return SpellCancelJob;
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

st_GameObjectJob* CGameServer::MakeGameObjectJobShockRelease()
{
	st_GameObjectJob* ShockReleaseJob = G_ObjectManager->GameObjectJobCreate();
	ShockReleaseJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SHOCK_RELEASE;

	ShockReleaseJob->GameObjectJobMessage = nullptr;

	return ShockReleaseJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobBackTeleport()
{
	st_GameObjectJob* BackTeleportJob = G_ObjectManager->GameObjectJobCreate();
	BackTeleportJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_BACK_TELEPORT;

	BackTeleportJob->GameObjectJobMessage = nullptr;

	return BackTeleportJob;
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

st_GameObjectJob* CGameServer::MakeGameObjectJobCraftingTableSelect(CGameObject* CraftingTableObject, CGameObject* OwnerObject)
{
	st_GameObjectJob* CraftingTableSelectJob = G_ObjectManager->GameObjectJobCreate();
	CraftingTableSelectJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_SELECT;

	CGameServerMessage* CraftingTableSelectJobMessage = CGameServerMessage::GameServerMessageAlloc();
	CraftingTableSelectJobMessage->Clear();

	*CraftingTableSelectJobMessage << &CraftingTableObject;
	*CraftingTableSelectJobMessage << &OwnerObject;

	CraftingTableSelectJob->GameObjectJobMessage = CraftingTableSelectJobMessage;

	return CraftingTableSelectJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobCraftingTableNonSelect(CGameObject* CraftingTableObject)
{
	st_GameObjectJob* CraftingTableSelectJob = G_ObjectManager->GameObjectJobCreate();
	CraftingTableSelectJob->GameObjectJobType = en_GameObjectJobType::GAMEOJBECT_JOB_TYPE_CRAFTING_TABLE_NON_SELECT;

	CGameServerMessage* CraftingTableSelectJobMessage = CGameServerMessage::GameServerMessageAlloc();
	CraftingTableSelectJobMessage->Clear();

	*CraftingTableSelectJobMessage << &CraftingTableObject;

	CraftingTableSelectJob->GameObjectJobMessage = CraftingTableSelectJobMessage;

	return CraftingTableSelectJob;
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

	int16 BufSize = BufSkillInfo.size();
	*ResMousePositionObjectInfoPacket << BufSize;

	for (auto BufSkillIterator : BufSkillInfo)
	{
		*ResMousePositionObjectInfoPacket << *(BufSkillIterator.second->GetSkillInfo());
	}

	int16 DeBufSize = DeBufSkillInfo.size();
	*ResMousePositionObjectInfoPacket << DeBufSize;

	for (auto DeBufSkillIterator : DeBufSkillInfo)
	{
		*ResMousePositionObjectInfoPacket << *(DeBufSkillIterator.second->GetSkillInfo());
	}

	return ResMousePositionObjectInfoPacket;
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

	int16 MaterialItemCount = MaterialItems.size();
	*ResCraftingTableCompleteItemSelectMessage << MaterialItemCount;

	for (auto MaterialItemIter : MaterialItems)
	{
		*ResCraftingTableCompleteItemSelectMessage << MaterialItemIter.second->_ItemInfo;
	}

	return ResCraftingTableCompleteItemSelectMessage;
}

CGameServerMessage* CGameServer::MakePacketResMenuTileBuy(vector<st_TileMapInfo> AroundMapTile)
{
	CGameServerMessage* ResMenuTileBuyMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResMenuTileBuyMessage == nullptr)
	{
		return nullptr;
	}

	ResMenuTileBuyMessage->Clear();

	*ResMenuTileBuyMessage << (int16)en_PACKET_S2C_UI_MENU_TILE_BUY;
	
	int16 AroundMapTileCount = AroundMapTile.size();
	*ResMenuTileBuyMessage << AroundMapTileCount;
	
	for (auto MapTileInfo : AroundMapTile)
	{
		*ResMenuTileBuyMessage << MapTileInfo;
	}

	return ResMenuTileBuyMessage;
}

CGameServerMessage* CGameServer::MakePacketResTileBuy(st_TileMapInfo TileMapInfo)
{
	CGameServerMessage* TileBuyMessage = CGameServerMessage::GameServerMessageAlloc();
	if (TileBuyMessage == nullptr)
	{
		return nullptr;
	}

	TileBuyMessage->Clear();

	*TileBuyMessage << (int16)en_PACKET_S2C_TILE_BUY;
	*TileBuyMessage << (int8)TileMapInfo.MapTileType;
	*TileBuyMessage << TileMapInfo.AccountID;
	*TileBuyMessage << TileMapInfo.PlayerID;
	*TileBuyMessage << TileMapInfo.TilePosition._X;
	*TileBuyMessage << TileMapInfo.TilePosition._Y;			

	return TileBuyMessage;
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
	DeSpawnObjectChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_OBJECT_DESPAWN_CHANNEL;

	CGameServerMessage* DeSpawnObjectChannelGameMessage = CGameServerMessage::GameServerMessageAlloc();
	DeSpawnObjectChannelGameMessage->Clear();

	*DeSpawnObjectChannelGameMessage << &DeSpawnChannelObject;
	DeSpawnObjectChannelJob->GameObjectJobMessage = DeSpawnObjectChannelGameMessage;

	return DeSpawnObjectChannelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobPlayerEnterChannel(CGameObject* EnterChannelObject)
{
	st_GameObjectJob* EnterChannelJob = G_ObjectManager->GameObjectJobCreate();
	EnterChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_PLAYER_ENTER_CHANNEL;

	CGameServerMessage* EnterChannelGameMessage = CGameServerMessage::GameServerMessageAlloc();
	EnterChannelGameMessage->Clear();

	*EnterChannelGameMessage << &EnterChannelObject;
	EnterChannelJob->GameObjectJobMessage = EnterChannelGameMessage;

	return EnterChannelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobObjectEnterChannel(CGameObject* EnterChannelObject)
{
	st_GameObjectJob* EnterChannelJob = G_ObjectManager->GameObjectJobCreate();
	EnterChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_OBJECT_ENTER_CHANNEL;

	CGameServerMessage* EnterChannelGameMessage = CGameServerMessage::GameServerMessageAlloc();
	EnterChannelGameMessage->Clear();

	*EnterChannelGameMessage << &EnterChannelObject;	

	EnterChannelJob->GameObjectJobMessage = EnterChannelGameMessage;

	return EnterChannelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobLeaveChannel(CGameObject* LeaveChannelObject)
{
	st_GameObjectJob* LeaveChannelJob = G_ObjectManager->GameObjectJobCreate();
	LeaveChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_LEAVE_CHANNEL;

	CGameServerMessage* LeaveChannelMessage = CGameServerMessage::GameServerMessageAlloc();
	LeaveChannelMessage->Clear();

	*LeaveChannelMessage << &LeaveChannelObject;		

	LeaveChannelJob->GameObjectJobMessage = LeaveChannelMessage;

	return LeaveChannelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectDamage(CGameObject* Attacker, int32 Damage)
{
	st_GameObjectJob* DamageJob = G_ObjectManager->GameObjectJobCreate();
	DamageJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_DAMAGE;

	CGameServerMessage* DamageMessage = CGameServerMessage::GameServerMessageAlloc();
	DamageMessage->Clear();
	
	*DamageMessage << &Attacker;
	*DamageMessage << Damage;

	DamageJob->GameObjectJobMessage = DamageMessage;

	return DamageJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobHPHeal(CGameObject* Healer, int32 HPHealPoint)
{
	st_GameObjectJob* HPHealJob = G_ObjectManager->GameObjectJobCreate();
	HPHealJob->GameObjectJobType = en_GameObjectJobType::GAMEOJBECT_JOB_TYPE_HP_HEAL;

	CGameServerMessage* HPHealMessage = CGameServerMessage::GameServerMessageAlloc();
	HPHealMessage->Clear();

	*HPHealMessage << &Healer;
	*HPHealMessage << HPHealPoint;

	HPHealJob->GameObjectJobMessage = HPHealMessage;

	return HPHealJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobMPHeal(CGameObject* Healer, int32 MPHealPoint)
{
	st_GameObjectJob* MPHealJob = G_ObjectManager->GameObjectJobCreate();
	MPHealJob->GameObjectJobType = en_GameObjectJobType::GAMEOJBECT_JOB_TYPE_MP_HEAL;

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

CGameServerMessage* CGameServer::MakePacketResEnterGame(bool EnterGameSuccess, st_GameObjectInfo* ObjectInfo, st_Vector2Int* SpawnPosition)
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

	return ResEnterGamePacket;
}

CGameServerMessage* CGameServer::MakePacketResDamage(int64 ObjectID, int64 TargetID, en_SkillType SkillType, int32 Damage, bool IsCritical)
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
	*ResDamageMessage << (int16)SkillType;
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

CGameServerMessage* CGameServer::MakePacketResAnimationPlay(int64 ObjectId, en_MoveDir Dir, wstring AnimationName)
{
	CGameServerMessage* ResAnimationPlayMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResAnimationPlayMessage == nullptr)
	{
		return nullptr;
	}

	ResAnimationPlayMessage->Clear();

	*ResAnimationPlayMessage << (int16)en_PACKET_S2C_ANIMATION_PLAY;
	*ResAnimationPlayMessage << ObjectId;
	*ResAnimationPlayMessage << (int8)Dir;

	int16 AnimationNameLen = (int16)(AnimationName.length() * 2);
	*ResAnimationPlayMessage << AnimationNameLen;
	ResAnimationPlayMessage->InsertData(AnimationName.c_str(), AnimationNameLen);

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
	*ResObjectStatePacket << (int8)ObjectState;

	return ResObjectStatePacket;
}

CGameServerMessage* CGameServer::MakePacketResChangeMonsterObjectState(int64 ObjectId, en_MoveDir Direction, en_GameObjectType ObjectType, en_CreatureState ObjectState, en_MonsterState MonsterState)
{
	CGameServerMessage* ResObjectStatePacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResObjectStatePacket == nullptr)
	{
		return nullptr;
	}

	ResObjectStatePacket->Clear();

	*ResObjectStatePacket << (int16)en_PACKET_S2C_MONSTER_OBJECT_STATE_CHANGE;
	*ResObjectStatePacket << ObjectId;
	*ResObjectStatePacket << (int8)Direction;
	*ResObjectStatePacket << (int16)ObjectType;
	*ResObjectStatePacket << (int8)ObjectState;
	*ResObjectStatePacket << (int8)MonsterState;

	return ResObjectStatePacket;
}

CGameServerMessage* CGameServer::MakePacketResMove(int64 ObjectId, bool CanMove, st_PositionInfo PositionInfo)
{
	CGameServerMessage* ResMoveMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResMoveMessage == nullptr)
	{
		return nullptr;
	}

	ResMoveMessage->Clear();

	*ResMoveMessage << (int16)en_PACKET_S2C_MOVE;	
	*ResMoveMessage << ObjectId;
	*ResMoveMessage << CanMove;

	*ResMoveMessage << PositionInfo;

	return ResMoveMessage;
}

CGameServerMessage* CGameServer::MakePacketResMonsterMove(int64 ObjectId, en_GameObjectType ObjectType, bool CanMove, st_PositionInfo PositionInfo, en_MonsterState MonsterState)
{
	CGameServerMessage* ResMonsterMoveMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResMonsterMoveMessage == nullptr)
	{
		return nullptr;
	}

	ResMonsterMoveMessage->Clear();

	*ResMonsterMoveMessage << (int16)en_PACKET_S2C_MONSTER_MOVE;
	*ResMonsterMoveMessage << ObjectId;
	*ResMonsterMoveMessage << (int16)ObjectType;
	*ResMonsterMoveMessage << CanMove;
	*ResMonsterMoveMessage << PositionInfo;
	*ResMonsterMoveMessage << (int8)MonsterState;

	return ResMonsterMoveMessage;
}

CGameServerMessage* CGameServer::MakePacketResMoveStop(int64 ObjectId, st_PositionInfo PositionInto)
{
	CGameServerMessage* ResMoveStopPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResMoveStopPacket == nullptr)
	{
		return nullptr;
	}

	ResMoveStopPacket->Clear();

	*ResMoveStopPacket << (int16)en_PACKET_S2C_MOVE_STOP;	
	*ResMoveStopPacket << ObjectId;
	*ResMoveStopPacket << PositionInto;

	return ResMoveStopPacket;
}

CGameServerMessage* CGameServer::MakePacketPatrol(int64 ObjectId, en_GameObjectType ObjectType, bool CanMove, st_PositionInfo PositionInfo, en_MonsterState MonsterState)
{
	CGameServerMessage* ResPatrolPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResPatrolPacket == nullptr)
	{
		return nullptr;
	}

	ResPatrolPacket->Clear();

	*ResPatrolPacket << (int16)en_PACKET_S2C_MONSTER_PATROL;
	*ResPatrolPacket << ObjectId;
	*ResPatrolPacket << (int16)ObjectType;
	*ResPatrolPacket << CanMove;
	*ResPatrolPacket << PositionInfo;
	*ResPatrolPacket << (int8)MonsterState;

	return ResPatrolPacket;
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

	int16 PlayerNameLen = (int16)(ChattingMessage.length() * 2);
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

	*ResSyncPositionMessage << (int16)en_PACKET_S2C_SYNC_OBJECT_POSITION;
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

CGameServerMessage* CGameServer::MakePacketComboSkillOn(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, st_SkillInfo ComboSkillInfo)
{
	CGameServerMessage* ResComboSkillMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResComboSkillMessage == nullptr)
	{
		return nullptr;
	}

	ResComboSkillMessage->Clear();

	*ResComboSkillMessage << (int16)en_PACKET_S2C_COMBO_SKILL_ON;
	*ResComboSkillMessage << (int8)QuickSlotBarIndex;
	*ResComboSkillMessage << (int8)QuickSlotBarSlotIndex;
	*ResComboSkillMessage << ComboSkillInfo;

	return ResComboSkillMessage;
}

CGameServerMessage* CGameServer::MakePacketComboSkillOff(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, st_SkillInfo ComboSkillInfo, en_SkillType OffComboSkillType)
{
	CGameServerMessage* ResComboSkillMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResComboSkillMessage == nullptr)
	{
		return nullptr;
	}

	ResComboSkillMessage->Clear();

	*ResComboSkillMessage << (int16)en_PACKET_S2C_COMBO_SKILL_OFF;
	*ResComboSkillMessage << QuickSlotBarIndex;
	*ResComboSkillMessage << QuickSlotBarSlotIndex;
	*ResComboSkillMessage << ComboSkillInfo;
	*ResComboSkillMessage << (int16)OffComboSkillType;

	return ResComboSkillMessage;
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

CGameServerMessage* CGameServer::MakePacketMagicCancel(int64 PlayerId)
{
	CGameServerMessage* ResMagicCancelMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResMagicCancelMessage == nullptr)
	{
		return nullptr;
	}

	ResMagicCancelMessage->Clear();

	*ResMagicCancelMessage << (int16)en_PACKET_S2C_MAGIC_CANCEL;	
	*ResMagicCancelMessage << PlayerId;

	return ResMagicCancelMessage;
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

CGameServerMessage* CGameServer::MakePacketCoolTime(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, float SkillCoolTimeSpeed, CSkill* QuickSlotSkill, int32 CoolTime)
{
	CGameServerMessage* ResCoolTimeMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCoolTimeMessage == nullptr)
	{
		return nullptr;
	}

	ResCoolTimeMessage->Clear();

	*ResCoolTimeMessage << (int16)en_PACKET_S2C_COOLTIME_START;
	*ResCoolTimeMessage << QuickSlotBarIndex;
	*ResCoolTimeMessage << QuickSlotBarSlotIndex;
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

	return ResCoolTimeMessage;
}

CGameServerMessage* CGameServer::MakePacketSkillError(en_PersonalMessageType PersonalMessageType, const WCHAR* SkillName, int16 SkillDistance)
{
	CGameServerMessage* ResErrorMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResErrorMessage == nullptr)
	{
		return nullptr;
	}

	ResErrorMessage->Clear();

	*ResErrorMessage << (int16)en_PACKET_S2C_PERSONAL_MESSAGE;
	*ResErrorMessage << (int8)1;

	WCHAR ErrorMessage[100] = { 0 };

	*ResErrorMessage << (int8)PersonalMessageType;

	switch (PersonalMessageType)
	{
	case en_PersonalMessageType::PERSONAL_MESSAGE_SKILL_COOLTIME:
		wsprintf(ErrorMessage, L"[%s]의 재사용 대기시간이 완료되지 않았습니다.", SkillName);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT:
		wsprintf(ErrorMessage, L"대상을 선택하고 [%s]을/를 사용해야 합니다.", SkillName);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_HEAL_NON_SELECT_OBJECT:
		wsprintf(ErrorMessage, L"[%s] 대상을 선택하지 않아서 자신에게 사용합니다.", SkillName);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_PLACE_BLOCK:
		wsprintf(ErrorMessage, L"이동할 위치가 막혀 있어서 [%s]을/를 사용할 수 없습니다.", SkillName);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_PLACE_DISTANCE:
		wsprintf(ErrorMessage, L"[%s] 대상과의 거리가 너무 멉니다. [거리 : %d ]", SkillName, SkillDistance);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_MYSELF_TARGET:
		wsprintf(ErrorMessage, L"[%s]을/를 자신에게 사용 할 수 없습니다.", SkillName, SkillDistance);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_DIR_DIFFERENT:
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

CGameServerMessage* CGameServer::MakePacketCommonError(en_PersonalMessageType PersonalMessageType, const WCHAR* Name)
{
	CGameServerMessage* ResCommonErrorMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResCommonErrorMessage == nullptr)
	{
		return nullptr;
	}

	ResCommonErrorMessage->Clear();

	*ResCommonErrorMessage << (int16)en_PACKET_S2C_PERSONAL_MESSAGE;
	*ResCommonErrorMessage << (int8)1;

	WCHAR ErrorMessage[100] = { 0 };

	*ResCommonErrorMessage << (int8)PersonalMessageType;

	switch (PersonalMessageType)
	{	
	case en_PersonalMessageType::PERSONAL_MESSAGE_DIR_DIFFERENT:
		wsprintf(ErrorMessage, L"[%s]을 바라보아야 합니다.", Name);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_GATHERING_DISTANCE:
		wsprintf(ErrorMessage, L"[%s]을 채집하려면 좀 더 가까이 다가가야합니다.", Name);
		break;
	case en_PersonalMessageType::PERSONAL_MEESAGE_CRAFTING_TABLE_OVERLAP_SELECT:
		wsprintf(ErrorMessage, L"[%s]를 사용중입니다.", Name);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_CRAFTING_TABLE_OVERLAP_CRAFTING_START:
		wsprintf(ErrorMessage, L"제작 중인 아이템을 모두 제작하거나, 제작 멈춤을 누르고 제작을 다시 시작해야 합니다.");
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_CRAFTING_TABLE_MATERIAL_COUNT_NOT_ENOUGH:
		wsprintf(ErrorMessage, L"재료가 부족합니다.");
		break;
	case en_PersonalMessageType::PERSOANL_MESSAGE_CRAFTING_TABLE_MATERIAL_WRONG_ITEM_ADD:
		wsprintf(ErrorMessage, L"선택된 제작법에 넣을 수 없는 재료입니다.");
		break;
	case en_PersonalMessageType::PERSONAL_FAULT_ITEM_USE:
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

CGameServerMessage* CGameServer::MakePacketStatusAbnormal(int64 TargetId, en_GameObjectType ObjectType, en_MoveDir Dir, en_SkillType SkillType, bool SetStatusAbnormal, int8 StatusAbnormal)
{
	CGameServerMessage* ResStatusAbnormal = CGameServerMessage::GameServerMessageAlloc();
	if (ResStatusAbnormal == nullptr)
	{
		return nullptr;
	}

	ResStatusAbnormal->Clear();

	*ResStatusAbnormal << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_STATUS_ABNORMAL;
	*ResStatusAbnormal << TargetId;
	*ResStatusAbnormal << (int16)ObjectType;
	*ResStatusAbnormal << (int8)Dir;
	*ResStatusAbnormal << (int16)SkillType;
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

CGameServerMessage* CGameServer::MakePacketResMoneyToInventory(int64 TargetObjectID, int64 GoldCoinCount, int16 SliverCoinCount, int16 BronzeCoinCount, st_ItemInfo ItemInfo, int16 ItemEach)
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
	*ResMoneyToInventoryMessage << GoldCoinCount;
	*ResMoneyToInventoryMessage << SliverCoinCount;
	*ResMoneyToInventoryMessage << BronzeCoinCount;
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

	int16 MaterialItemCount = MaterialItems.size();
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

	int16 MaterialItemCount = MaterialItems.size();
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

	int16 CompleteItemCount = CompleteItems.size();
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
	vector<CSector*> AroundSectors = Object->GetChannel()->GetMap()->GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, 1);

	for (CSector* AroundSector : AroundSectors)
	{
		for (CPlayer* Player : AroundSector->GetPlayers())
		{
			int16 Distance = st_Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

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