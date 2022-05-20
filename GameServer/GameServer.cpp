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
#include <WS2tcpip.h>
#include <process.h>
#include <atlbase.h>

CGameServer::CGameServer()
{
	//timeBeginPeriod(1);
	_UserDataBaseThread = nullptr;
	_TimerJobThread = nullptr;
	_LogicThread = nullptr;

	// ��ñ׳� ���� �ڵ�����
	_UserDataBaseWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_WorldDataBaseWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_ClientLeaveDBThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_TimerThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	_NetworkThreadEnd = false;
	_UserDataBaseThreadEnd = false;
	_WorldDataBaseThreadEnd = false;
	_ClientLeaveSaveDBThreadEnd = false;
	_LogicThreadEnd = false;
	_TimerJobThreadEnd = false;

	// Ÿ�̸� �� ���� SWRLock �ʱ�ȭ
	InitializeSRWLock(&_TimerJobLock);

	// �� �޸�Ǯ ����
	_GameServerJobMemoryPool = new CMemoryPoolTLS<st_GameServerJob>();
	// Ÿ�̸� �� �޸�Ǯ ����
	_TimerJobMemoryPool = new CMemoryPoolTLS<st_TimerJob>();

	// Ÿ�̸� �켱���� ť ����
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

	G_MapManager->MapSave();

	// �� ������Ʈ ����
	G_ObjectManager->MapObjectSpawn(MapID);

	G_ObjectManager->GameServer = this;

	// ���� ������ ���̽� ������ ����
	for (int32 i = 0; i < 3; i++)
	{
		_UserDataBaseThread = (HANDLE)_beginthreadex(NULL, 0, UserDataBaseThreadProc, this, 0, NULL);
		CloseHandle(_UserDataBaseThread);
	}

	// ���� ������ ���̽� ������ ����
	_WorldDataBaseThread = (HANDLE)_beginthreadex(NULL, 0, WorldDataBaseThreadProc, this, 0, NULL);
	// Ŭ�� ���� ĳ���� ���� DB ���� ������ ����
	_ClientLeaveSaveThread = (HANDLE)_beginthreadex(NULL, 0, ClientLeaveThreadProc, this, 0, NULL);
	// Ÿ�̸� �� ������ ����
	_TimerJobThread = (HANDLE)_beginthreadex(NULL, 0, TimerJobThreadProc, this, 0, NULL);
	// ���� ������ ����
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
			case en_GameServerJobType::DATA_BASE_CRAFTING_ITEM_INVENTORY_SAVE:
				Instance->PacketProcReqDBCraftingItemToInventorySave(Job->Message);
				break;			
			case en_GameServerJobType::DATA_BASE_ITEM_USE:
				Instance->PacketProcReqDBItemUpdate(Job->Message);
				break;
			case en_GameServerJobType::DATA_BASE_GOLD_SAVE:
				Instance->PacketProcReqDBGoldSave(Job->Message);
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
			case en_GameServerJobType::DATA_BASE_ITEM_CREATE:
				Instance->PacketProcReqDBItemCreate(Job->Message);
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
			// �� �� �������� �ð��� Ȯ���Ѵ�.
			st_TimerJob* TimerJob = Instance->_TimerHeapJob->Peek();

			// ���� �ð��� ���Ѵ�.
			int64 NowTick = GetTickCount64();

			// TimerJob�� ��ϵǾ� �ִ� �ð� ���� ���� �ð��� Ŭ ���
			// ��, �ð��� ��� �Ǿ�����
			if (TimerJob->TimerJobExecTick <= NowTick)
			{
				// TimerJob�� �̰�
				AcquireSRWLockExclusive(&Instance->_TimerJobLock);
				st_TimerJob* TimerJob = Instance->_TimerHeapJob->PopHeap();
				ReleaseSRWLockExclusive(&Instance->_TimerJobLock);

				// ��Ұ� ���� ���� TimerJob�� ������� ó��
				if (TimerJob->TimerJobCancel != true)
				{
					// Type�� ���� �����Ѵ�.
					switch (TimerJob->TimerJobType)
					{
					case en_TimerJobType::TIMER_OBJECT_SPAWN:
						Instance->PacketProcTimerObjectSpawn(TimerJob->TimerJobMessage);
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

			G_MapManager->Update();

			Instance->_LogicThreadFPS++;
		}
	}

	return 0;
}

void CGameServer::PlayerSkillCreate(int64& AccountId, st_GameObjectInfo& NewCharacterInfo, int8& CharacterCreateSlotIndex)
{
	// Ŭ���� ���� ��ų ����
	switch (NewCharacterInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	{
		for (auto PublicAttackSkillDataIter : G_Datamanager->_PublicAttackSkillDatas)
		{
			st_AttackSkillData* PublicAttackSkillData = PublicAttackSkillDataIter.second;

			if (PublicAttackSkillData != nullptr)
			{
				// �Ϲ� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_BufSkillData* PublicBufSkillData = PublicBufSkillDataIter.second;

			if (PublicBufSkillData != nullptr)
			{
				// �Ϲ� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_AttackSkillData* WarriorAttackSkillData = WarriorAttackSkillDataIter.second;

			if (WarriorAttackSkillData != nullptr)
			{
				// ���� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_TacTicSkillData* WarriorTacTicSkillData = WarriorTacTicSkillDataIter.second;

			if (WarriorTacTicSkillData != nullptr)
			{
				// ���� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_BufSkillData* WarriorBufSkillData = WarriorBufSkillDataIter.second;

			if (WarriorBufSkillData != nullptr)
			{
				// ���� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_AttackSkillData* PublicAttackSkillData = PublicAttackSkillDataIter.second;

			if (PublicAttackSkillData != nullptr)
			{
				// �Ϲ� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_BufSkillData* PublicBufSkillData = PublicBufSkillDataIter.second;

			if (PublicBufSkillData != nullptr)
			{
				// �Ϲ� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_AttackSkillData* ShamanAttackSkillData = ShamanAttackSkillDataIter.second;

			if (ShamanAttackSkillData != nullptr)
			{
				// �ּ��� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_TacTicSkillData* ShamanTacTicSkillData = ShamanTacTicSkillDataIter.second;

			if (ShamanTacTicSkillData != nullptr)
			{
				// �ּ��� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_BufSkillData* ShamanBufSkillData = ShamanBufSkillDataIter.second;

			if (ShamanBufSkillData != nullptr)
			{
				// �ּ��� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_AttackSkillData* PublicAttackSkillData = PublicAttackSkillDataIter.second;

			if (PublicAttackSkillData != nullptr)
			{
				// �Ϲ� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_BufSkillData* PublicBufSkillData = PublicBufSkillDataIter.second;

			if (PublicBufSkillData != nullptr)
			{
				// �Ϲ� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_AttackSkillData* TaioistAttackSkillData = TaioistAttackSkillDataIter.second;

			if (TaioistAttackSkillData != nullptr)
			{
				// ���� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_TacTicSkillData* TaioistTacTicSkillData = TaioistTacTicSkillDataIter.second;

			if (TaioistTacTicSkillData != nullptr)
			{
				// ���� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
			st_BufSkillData* TaioistBufSkillData = TacTicBufSkillDataIter.second;

			if (TaioistBufSkillData != nullptr)
			{
				// �ּ��� ���� ��ų ���� �� DB ����
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
				SkillToSkillBox.InIsQuickSlotUse(NewSkillInfo.IsQuickSlotUse);
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
}

void CGameServer::CreateNewClient(int64 SessionId)
{
	// ���� ã��
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		Session->PingPacketTime = GetTickCount64();

		for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
		{
			// �÷��̾� �ε��� �Ҵ�
			G_ObjectManager->_PlayersArrayIndexs.Pop(&Session->MyPlayerIndexes[i]);
		}

		// ���� ���� ���� ������ Ŭ�󿡰� �˷��ش�.
		CMessage* ResClientConnectedMessage = MakePacketResClientConnected();
		SendPacket(Session->SessionId, ResClientConnectedMessage);
		ResClientConnectedMessage->Free();

		// �� ���� ����		
		//PingTimerCreate(Session);

		ReturnSession(Session);
	}
}

void CGameServer::DeleteClient(st_Session* Session)
{
	// ĳ���� ������ DB�� ����ϱ� ���� DB_WORKING ���̶�� ���
	if (Session->IsDummy == false)
	{
		CGameServerMessage* LogOutPacket = MakePacketLoginServerLoginStateChange(Session->AccountId, en_LoginState::LOGIN_OUT);
		SendPacketToLoginServer(LogOutPacket);
		LogOutPacket->Free();
	}
		
	if (Session->MyPlayerIndex >= 0)
	{
		// Session�� ���� ���� ���� ������ ������ ���
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
		// Session�� ���� ���� ���� ������ ������ ���

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
		// ���� �ε��� �ݳ�	
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

void CGameServer::PacketProc(int64 SessionId, CMessage* Message)
{
	WORD MessageType;
	*Message >> MessageType;

	switch (MessageType)
	{
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_GAME_REQ_LOGIN:
		PacketProcReqLogin(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_GAME_CREATE_CHARACTER:
		PacketProcReqCreateCharacter(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_GAME_ENTER:
		PacketProcReqEnterGame(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_CHARACTER_INFO:
		PacketProcReqCharacterInfo(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MOVE:
		PacketProcReqMove(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MOVE_STOP:
		PacketProcReqMoveStop(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_ATTACK:
		PacketProcReqMelee(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MAGIC:
		PacketProcReqMagic(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MAGIC_CANCEL:
		PacketProcReqMagicCancel(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MOUSE_POSITION_OBJECT_INFO:
		PacketProcReqMousePositionObjectInfo(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_OBJECT_STATE_CHANGE:
		PacketProcReqObjectStateChange(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_MESSAGE:
		PacketProcReqChattingMessage(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_LOOTING:
		PacketProcReqItemLooting(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_ITEM_SELECT:
		PacketProcReqItemSelect(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_ITEM_PLACE:
		PacketProcReqItemPlace(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_QUICKSLOT_SAVE:
		PacketProcReqQuickSlotSave(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_QUICKSLOT_SWAP:
		PacketProcReqQuickSlotSwap(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_QUICKSLOT_EMPTY:
		PacketProcReqQuickSlotInit(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_CRAFTING_CONFIRM:
		PacketProcReqCraftingConfirm(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_INVENTORY_ITEM_USE:
		PacketProcReqItemUse(SessionId, Message);
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_CS_GAME_REQ_HEARTBEAT:
		break;
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_C2S_PONG:
		PacketProcReqPong(SessionId, Message);
		break;
	default:
		Disconnect(SessionId);
		break;
	}

	//Message->Free();
}

void CGameServer::PacketProcReqLogin(int64 SessionID, CMessage* Message)
{
	st_Session* Session = FindSession(SessionID);

	int64 AccountId;
	int32 Token;

	do
	{
		if (Session != nullptr)
		{
			// AccountID ����
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

				// ��ū ����
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
			// �ش� Ŭ�� ����
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
		// Ŭ�󿡼� ������ �÷��̾� ������Ʈ Ÿ��
		int16 ReqGameObjectType;
		*Message >> ReqGameObjectType;

		// ĳ���� �̸� ����
		int16 CharacterNameLen;
		*Message >> CharacterNameLen;

		WCHAR* CharacterName = (WCHAR*)malloc(sizeof(WCHAR) * CharacterNameLen);
		memset(CharacterName, 0, sizeof(WCHAR) * CharacterNameLen);
		Message->GetData(CharacterName, CharacterNameLen);

		// ĳ���� �̸� ����
		Session->CreateCharacterName = CharacterName;
		free(CharacterName);
		CharacterName = nullptr;

		// Ŭ�󿡼� ������ �÷��̾� �ε���
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
			// �α��� ���� �ƴϸ� ������.
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
			// Ŭ�� ������ �ִ� ĳ�� �߿� ��Ŷ���� ���� ĳ���Ͱ� �ִ��� Ȯ���Ѵ�.
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
				st_GameObjectJob* EnterGameJob = MakeGameObjectJobEnterChannel(MyPlayer);

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

		// PlayerDBId�� �̴´�.
		int64 PlayerDBId;
		*Message >> PlayerDBId;

		// ���ӿ� ������ ĳ���͸� �����´�.
		CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

		// Ŭ�� �������� ĳ���Ͱ� �ִ��� Ȯ��
		if (MyPlayer == nullptr)
		{
			Disconnect(Session->SessionId);
		}
		else
		{
			// �������� ĳ���Ͱ� ������ ObjectId�� �ٸ��� Ȯ��
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
			int64 PlayerDBId;
			*Message >> PlayerDBId;

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// Ŭ�� �������� ĳ���Ͱ� �ִ��� Ȯ��
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �������� ĳ���Ͱ� ������ ObjectId�� �ٸ��� Ȯ��
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

			// Ŭ�� ������ ���Ⱚ�� �����´�.
			int8 ReqMoveDir;
			*Message >> ReqMoveDir;

			MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = (en_MoveDir)ReqMoveDir;

			if (MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::IDLE
				|| MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::SPAWN_IDLE
				|| MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::SPELL
				|| MyPlayer->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::ATTACK)
			{
				MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			}			

			CMap* Map = G_MapManager->GetMap(1);

			vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(MyPlayer, 1, false);

			CMessage* ResMyMoveOtherPacket = MakePacketResMove(Session->AccountId,
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
			int64 PlayerDBId;
			*Message >> PlayerDBId;

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// Ŭ�� �������� ĳ���Ͱ� �ִ��� Ȯ��
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �������� ĳ���Ͱ� ������ ObjectId�� �ٸ��� Ȯ��
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

			float CheckPositionX = abs(MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX - PositionX);
			float CheckPositionY = abs(MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY - PositionY);

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

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// �����ϰ� �ִ� �÷��̾ �ִ��� Ȯ��
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �����ϰ� �ִ� �÷��̾��� ObjectId�� Ŭ�� ���� ObjectId�� ������ Ȯ��
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

				// Ÿ�� ��ġ Ȯ��
				switch (ReqMeleeSkill->GetSkillInfo()->SkillType)
				{
				case en_SkillType::SKILL_DEFAULT_ATTACK:
					{
						SkillUseSuccess = true;

						st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)ReqMeleeSkill->GetSkillInfo();

						st_Vector2Int FrontCell = MyPlayer->GetFrontCellPosition(MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);
						CGameObject* Target = MyPlayer->GetChannel()->GetMap()->Find(FrontCell);

						if (Target != nullptr && Target->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
						{
							Targets.push_back(Target);
						}
					}
					break;
				case en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK:
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

						st_Vector2Int FrontCell = MyPlayer->GetFrontCellPosition(MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);
						CGameObject* Target = MyPlayer->GetChannel()->GetMap()->Find(FrontCell);

						if (Target != nullptr && Target->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
						{
							Targets.push_back(Target);
						}
					}
					break;
				case en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK:
					{
						if (MyPlayer->_ComboSkill != nullptr && MyPlayer->_ComboSkill->GetSkillInfo()->SkillType == en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK)
						{
							SkillUseSuccess = true;
							GlobalSkillCooltime = false;							

							st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)ReqMeleeSkill->GetSkillInfo();

							st_Vector2Int FrontCell = MyPlayer->GetFrontCellPosition(MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);
							CGameObject* Target = MyPlayer->GetChannel()->GetMap()->Find(FrontCell);

							if (Target != nullptr && Target->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
							{
								Targets.push_back(Target);
							}
						}						
					}
					break;
				case en_SkillType::SKILL_KNIGHT_CHOHONE:
				{
					if (MyPlayer->_SelectTarget != nullptr)
					{
						st_Vector2Int TargetPosition = MyPlayer->GetChannel()->FindChannelObject(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId,
							MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType)->GetCellPosition();
						st_Vector2Int MyPosition = MyPlayer->GetCellPosition();
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
										Target->_GameObjectInfo.ObjectPositionInfo.PositionX = MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX;
										Target->_GameObjectInfo.ObjectPositionInfo.PositionY = MyFrontCellPotision._Y + 0.5f;
										break;
									case en_MoveDir::DOWN:
										Target->_GameObjectInfo.ObjectPositionInfo.PositionX = MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX;
										Target->_GameObjectInfo.ObjectPositionInfo.PositionY = MyFrontCellPotision._Y + 0.5f;
										break;
									case en_MoveDir::LEFT:
										Target->_GameObjectInfo.ObjectPositionInfo.PositionX = MyFrontCellPotision._X + 0.5f;
										Target->_GameObjectInfo.ObjectPositionInfo.PositionY = MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY;
										break;
									case en_MoveDir::RIGHT:
										Target->_GameObjectInfo.ObjectPositionInfo.PositionX = MyFrontCellPotision._X + 0.5f;
										Target->_GameObjectInfo.ObjectPositionInfo.PositionY = MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY;
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
						st_Vector2Int TargetPosition = MyPlayer->GetChannel()->FindChannelObject(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType)->GetCellPosition();
						st_Vector2Int MyPosition = MyPlayer->GetCellPosition();
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

									// ����Ʈ ���
									CMessage* ResEffectPacket = MakePacketEffect(Target->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_DEBUF_ROOT, EffectPrintTime);
									SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
									ResEffectPacket->Free();

									Targets.push_back(Target);

									switch (Dir)
									{
									case en_MoveDir::UP:
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = Target->_GameObjectInfo.ObjectPositionInfo.PositionX;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = MovePosition._Y + 0.5f;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::UP;
										break;
									case en_MoveDir::DOWN:
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = Target->_GameObjectInfo.ObjectPositionInfo.PositionX;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = MovePosition._Y + 0.5f;
										break;
									case en_MoveDir::LEFT:
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::LEFT;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = MovePosition._X + 0.5f;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = Target->_GameObjectInfo.ObjectPositionInfo.PositionY;
										break;
									case en_MoveDir::RIGHT:
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::RIGHT;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = MovePosition._X + 0.5f;
										MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = Target->_GameObjectInfo.ObjectPositionInfo.PositionY;
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

					TargetPositions = MyPlayer->GetAroundCellPositions(MyPlayer->GetCellPosition(), 1);
					for (st_Vector2Int TargetPosition : TargetPositions)
					{
						CGameObject* Target = MyPlayer->GetChannel()->GetMap()->Find(TargetPosition);
						if (Target != nullptr)
						{
							Targets.push_back(Target);
						}
					}

					// ����Ʈ ���
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
						// Ŭ�󿡰� ��Ÿ�� ǥ��
						CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
							QuickSlotBarPosition.QuickSlotBarSlotIndex,
							1.0f, ReqMeleeSkill);
						SendPacket(Session->SessionId, ResCoolTimeStartPacket);
						ResCoolTimeStartPacket->Free();
					}

					if (GlobalSkillCooltime)
					{
						// ���� ��Ÿ�� ����
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

							// ũ��Ƽ�� �Ǵ�
							float CriticalPoint = MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint / 1000.0f;
							bernoulli_distribution CriticalCheck(CriticalPoint);
							bool IsCritical = CriticalCheck(Eng);

							// ������ �Ǵ�
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
								ExperienceCalculate(MyPlayer, Target);
							}

							en_EffectType HitEffectType = en_EffectType::EFFECT_TYPE_NONE;

							wstring SkillTypeString;
							wchar_t SkillTypeMessage[64] = L"0";
							wchar_t SkillDamageMessage[64] = L"0";

							// �ý��� �޼��� ����
							switch ((en_SkillType)ReqSkillType)
							{
							case en_SkillType::SKILL_TYPE_NONE:
								CRASH("SkillType None");
								break;
							case en_SkillType::SKILL_DEFAULT_ATTACK:
								wsprintf(SkillTypeMessage, L"%s�� �Ϲݰ����� ����� %s���� %d�� �������� ����ϴ�.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
								HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
								break;
							case en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK:
								wsprintf(SkillTypeMessage, L"%s�� �ͷ��� �ϰ��� ����� %s���� %d�� �������� ����ϴ�.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
								HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
								break;
							case en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK:
								wsprintf(SkillTypeMessage, L"%s�� ȸ���� �ϰ��� ����� %s���� %d�� �������� ����ϴ�.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
								HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
								break;
							case en_SkillType::SKILL_KNIGHT_CHOHONE:
								wsprintf(SkillTypeMessage, L"%s�� ��ȥ�񹫸� ����� %s���� %d�� �������� ����ϴ�.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
								HitEffectType = en_EffectType::EFFECT_CHOHONE_TARGET_HIT;
								break;
							case en_SkillType::SKILL_KNIGHT_SHAEHONE:
								wsprintf(SkillTypeMessage, L"%s�� ��ȥ�񹫸� ����� %s���� %d�� �������� ����ϴ�.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
								HitEffectType = en_EffectType::EFFECT_SHAHONE_TARGET_HIT;
								break;
							case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
								wsprintf(SkillTypeMessage, L"%s�� �м��ĵ��� ����� %s���� %d�� �������� ����ϴ�.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
								HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
								break;
							default:
								break;
							}

							SkillTypeString = SkillTypeMessage;
							SkillTypeString = IsCritical ? L"ġ��Ÿ! " + SkillTypeString : SkillTypeString;

							// ������ �ý��� �޼��� ����
							CMessage* ResSkillSystemMessagePacket = MakePacketResChattingBoxMessage(MyPlayer->_GameObjectInfo.ObjectId,
								en_MessageType::SYSTEM,
								IsCritical ? st_Color::Red() : st_Color::White(),
								SkillTypeString);
							SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSkillSystemMessagePacket);
							ResSkillSystemMessagePacket->Free();

							// ���� ���� �޼��� ����
							CMessage* ResMyAttackOtherPacket = MakePacketResAttack(MyPlayer->_GameObjectInfo.ObjectId,
								Target->_GameObjectInfo.ObjectId,
								(en_SkillType)ReqSkillType,
								FinalDamage,
								IsCritical);
							SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResMyAttackOtherPacket);
							ResMyAttackOtherPacket->Free();

							// ����Ʈ ���
							CMessage* ResEffectPacket = MakePacketEffect(Target->_GameObjectInfo.ObjectId,
								HitEffectType,
								AttackSkillInfo->SkillTargetEffectTime);
							SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
							ResEffectPacket->Free();

							// ���� ���� �޼��� ����
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

			// �α��������� Ȯ��
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

			*Message >> ObjectId;
			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// ������ ĳ���Ͱ� �ִ��� Ȯ��
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �����ϰ� �ִ� ĳ���� ObjectId�� Ŭ�� ������ ObjectId�� ������ Ȯ��
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

			// ������ ����
			int8 ReqMoveDir;
			*Message >> ReqMoveDir;

			// ��ų ����
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

					// Ŭ�󿡰� ��Ÿ�� ǥ��
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
				// ��� ���� ������ �����ϰ� �����̻��� �Ǵ�
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

				if (MyPlayer->_ComboSkill != nullptr)
				{
					st_GameObjectJob* ComboAttackOffJob = MakeGameObjectJobComboSkillOff();
					MyPlayer->_GameObjectJobQue.Enqueue(ComboAttackOffJob);
				}

				// ��û ��ų�� ��ųâ���� ã��
				CSkill* ReqMagicSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)ReqSkillType);
				if (ReqMagicSkill != nullptr && ReqMagicSkill->GetSkillInfo()->CanSkillUse == true)
				{
					CMap* Map = G_MapManager->GetMap(1);

					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(MyPlayer, 1, false);
					
					// ��û�� ��ų�� �����Կ� ����� �Ǿ� �ִ��� Ȯ��
					st_QuickSlotBarSlotInfo* QuickSlotInfo = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarIndex, QuickSlotBarSlotIndex);
					if (QuickSlotInfo->QuickBarSkill != nullptr)
					{
						// ��û�� ��ų�� ��� �� �� �ִ��� Ȯ��
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

								// Ŭ�󿡰� ��Ÿ�� ǥ��
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

								st_Vector2 MyPosition = MyPlayer->GetPosition();

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

								MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX + 0.5f;
								MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY + 0.5f;

								CMessage* ResSyncPositionPacket = MakePacketResSyncPosition(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
								SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSyncPositionPacket);
								ResSyncPositionPacket->Free();

								ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_BACK_TELEPORT, 0.5f);
								SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
								ResEffectPacket->Free();

								ReqMagicSkill->CoolTimeStart();

								// Ŭ�󿡰� ��Ÿ�� ǥ��
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

										int16 Distance = st_Vector2Int::Distance(MyPlayer->_SelectTarget->GetCellPosition(), MyPlayer->GetCellPosition());

										if (Distance <= 6)
										{
											FindGameObject = MyPlayer->GetChannel()->FindChannelObject(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId,
												MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType);
											if (FindGameObject != nullptr)
											{
												Targets.push_back(FindGameObject);
											}

											// ����â ����
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
											// Ŭ�󿡰� ��Ÿ�� ǥ��
											CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
												QuickSlotBarPosition.QuickSlotBarSlotIndex,
												1.0f, ReqMagicSkill);
											SendPacket(Session->SessionId, ResCoolTimeStartPacket);
											ResCoolTimeStartPacket->Free();
										}
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

									// ����â ����
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

									// ����â ����
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

			// ���ӿ� ������ ĳ���͸� �����´�.
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

			// ���ӿ� ������ ĳ���͸� �����´�.
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

				CMessage* ResMousePositionObjectInfo = MakePacketResMousePositionObjectInfo(Session->AccountId,
					PreviousChoiceObject, FindObject->_GameObjectInfo.ObjectId,
					FindObject->_Bufs, FindObject->_DeBufs);
				SendPacket(Session->SessionId, ResMousePositionObjectInfo);
				ResMousePositionObjectInfo->Free();
			}
		}
	} while (0);

	ReturnSession(Session);
}

void CGameServer::PacketProcReqObjectStateChange(int64 SessionId, CMessage* Message)
{
	// ���� ���
	st_Session* Session = FindSession(SessionId);

	int64 AccountId;
	int64 PlayerDBId;
	int16 StateChange;

	if (Session)
	{
		do
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

			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// �����ϰ� �ִ� �÷��̾ �ִ��� Ȯ�� 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				return;
			}
			else
			{
				// �����ϰ� �ִ� �÷��̾�� ���۹��� PlayerId�� ������ Ȯ��
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
	// ���� ���
	st_Session* Session = FindSession(SessionId);

	int64 AccountId;
	int64 PlayerDBId;

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

		// ���ӿ� ������ ĳ���͸� �����´�.
		CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

		// �����ϰ� �ִ� �÷��̾ �ִ��� Ȯ�� 
		if (MyPlayer == nullptr)
		{
			Disconnect(Session->SessionId);
			return;
		}
		else
		{
			// �����ϰ� �ִ� �÷��̾�� ���۹��� PlayerId�� ������ Ȯ��
			if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
			{
				Disconnect(Session->SessionId);
				return;
			}
		}

		// ä�� �޼��� ���� 
		int16 ChattingMessageLen;
		*Message >> ChattingMessageLen;

		// ä�� �޼��� ������
		wstring ChattingMessage;
		Message->GetData(ChattingMessage, ChattingMessageLen);
		
		CMap* Map = G_MapManager->GetMap(1);

		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(MyPlayer, 1, false);
				
		CMessage* ResChattingMessage = MakePacketResChattingBoxMessage(PlayerDBId, en_MessageType::CHATTING, st_Color::White(), ChattingMessage);
		SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResChattingMessage);
		ResChattingMessage->Free();
	}

	// ���� �ݳ�
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

			// AccountId�� �´��� Ȯ��
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> PlayerDBId;

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// �����ϰ� �ִ� �÷��̾ �ִ��� Ȯ�� 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �����ϰ� �ִ� �÷��̾�� ���۹��� PlayerId�� ������ Ȯ��
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

			// AccountId�� �´��� Ȯ��
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> PlayerDBId;

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// �����ϰ� �ִ� �÷��̾ �ִ��� Ȯ�� 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �����ϰ� �ִ� �÷��̾�� ���۹��� PlayerId�� ������ Ȯ��
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			int16 PlaceItemTilePositionX;
			*Message >> PlaceItemTilePositionX;
			int16 PlaceITemTilePositionY;
			*Message >> PlaceITemTilePositionY;

			CItem* PlaceItem = MyPlayer->_InventoryManager.SwapItem(0, PlaceItemTilePositionX, PlaceITemTilePositionY);

			CMessage* ResPlaceItemPacket = MakePacketResPlaceItem(Session->AccountId, MyPlayer->_GameObjectInfo.ObjectId, PlaceItem, MyPlayer->_InventoryManager._SelectItem);
			SendPacket(Session->SessionId, ResPlaceItemPacket);
			ResPlaceItemPacket->Free();
		} while (0);

		ReturnSession(Session);
	}
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

			// AccountId�� �´��� Ȯ��
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> PlayerDBId;

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// �����ϰ� �ִ� �÷��̾ �ִ��� Ȯ�� 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �����ϰ� �ִ� �÷��̾�� ���۹��� PlayerId�� ������ Ȯ��
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
					FindQuickSlotInfo->QuickBarSkill = FindSkill;
				}
				else
				{
					CRASH(L"��ų �ڽ��� ���� ��ų�� �����Կ� ����Ϸ��� �õ�");
				}
			}

			CMessage* ResQuickSlotUpdateMessage = MakePacketResQuickSlotBarSlotSave(*FindQuickSlotInfo);
			SendPacket(Session->SessionId, ResQuickSlotUpdateMessage);
			ResQuickSlotUpdateMessage->Free();
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

			// AccountId�� �´��� Ȯ��
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> PlayerDBId;

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// �����ϰ� �ִ� �÷��̾ �ִ��� Ȯ�� 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �����ϰ� �ִ� �÷��̾�� ���۹��� PlayerId�� ������ Ȯ��
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

			// ���� ��û A ���� ����
			st_QuickSlotBarSlotInfo SwapAQuickSlotBarInfo;
			SwapAQuickSlotBarInfo.AccountDBId = AccountId;
			SwapAQuickSlotBarInfo.PlayerDBId = PlayerDBId;

			// ���� �ϱ� ���ؼ� B ������ ��ġ ������ �ִ´�.
			SwapAQuickSlotBarInfo.QuickSlotBarIndex = QuickSlotBarSwapIndexA;
			SwapAQuickSlotBarInfo.QuickSlotBarSlotIndex = QuickSlotBarSlotSwapIndexA;

			// B ������ ������ �����´�.
			st_QuickSlotBarSlotInfo* FindBQuickslotInfo = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarSwapIndexB, QuickSlotBarSlotSwapIndexB);
			if (FindBQuickslotInfo != nullptr)
			{
				// ��� ���� �ʴٸ� ��ų ������ �ٲ۴�.
				if (FindBQuickslotInfo->QuickBarSkill != nullptr)
				{
					SwapAQuickSlotBarInfo.QuickBarSkill = FindBQuickslotInfo->QuickBarSkill;
					SwapAQuickSlotBarInfo.QuickSlotKey = FindBQuickslotInfo->QuickSlotKey;
				}
				else // ��� ���� ��쿣 ��ų ������ �ʱ�ȭ�Ѵ�.
				{
					SwapAQuickSlotBarInfo.QuickBarSkill = nullptr;
				}
			}
			else
			{
				CRASH("������ ������ ã�� �� ����");
			}

			// ���� ��û B ���� ����
			st_QuickSlotBarSlotInfo SwapBQuickSlotBarInfo;
			SwapBQuickSlotBarInfo.AccountDBId = AccountId;
			SwapBQuickSlotBarInfo.PlayerDBId = PlayerDBId;
			SwapBQuickSlotBarInfo.QuickSlotBarIndex = QuickSlotBarSwapIndexB;
			SwapBQuickSlotBarInfo.QuickSlotBarSlotIndex = QuickSlotBarSlotSwapIndexB;

			st_QuickSlotBarSlotInfo* FindAQuickslotInfo = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarSwapIndexA, QuickSlotBarSlotSwapIndexA);
			if (FindAQuickslotInfo != nullptr)
			{
				if (FindAQuickslotInfo->QuickBarSkill != nullptr)
				{
					SwapBQuickSlotBarInfo.QuickBarSkill = FindAQuickslotInfo->QuickBarSkill;
					SwapAQuickSlotBarInfo.QuickSlotKey = FindAQuickslotInfo->QuickSlotKey;
				}
				else
				{
					SwapBQuickSlotBarInfo.QuickBarSkill = nullptr;
				}
			}
			else
			{
				CRASH("������ ������ ã�� �� ����");
			}

			SwapAQuickSlotBarInfo.QuickSlotKey = FindAQuickslotInfo->QuickSlotKey;
			SwapBQuickSlotBarInfo.QuickSlotKey = FindBQuickslotInfo->QuickSlotKey;

			MyPlayer->_QuickSlotManager.SwapQuickSlot(SwapBQuickSlotBarInfo, SwapAQuickSlotBarInfo);

			// Ŭ�󿡰� ��� ����
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

			// AccountId�� �´��� Ȯ��
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> PlayerDBId;

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// �����ϰ� �ִ� �÷��̾ �ִ��� Ȯ�� 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �����ϰ� �ִ� �÷��̾�� ���۹��� PlayerId�� ������ Ȯ��
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

			// �����Կ��� ���� ã��
			st_QuickSlotBarSlotInfo* InitQuickSlotBarSlot = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarIndex, QuickSlotBarSlotIndex);
			if (InitQuickSlotBarSlot != nullptr)
			{
				// �����Կ��� ��ų ���� ����
				InitQuickSlotBarSlot->QuickBarSkill = nullptr;
			}
			else
			{
				CRASH("������������ ã�� ����");
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

			// AccountId�� �´��� Ȯ��
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> PlayerDBId;

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// �����ϰ� �ִ� �÷��̾ �ִ��� Ȯ�� 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �����ϰ� �ִ� �÷��̾�� ���۹��� PlayerId�� ������ Ȯ��
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

			// Ŭ�� ���� ��û�� �������� ����۰� ������� ����
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
			// ���۹� ī�װ� ã��
			auto FindCategoryItem = G_Datamanager->_CraftingData.find(ReqCategoryType);
			if (FindCategoryItem == G_Datamanager->_CraftingData.end())
			{
				// ī�װ��� ã�� ����
				break;
			}

			FindCraftingCategoryData = (*FindCategoryItem).second;

			// �ϼ��� ���ۿ� �ʿ��� ��� ��� ã��
			vector<st_CraftingMaterialItemData> RequireMaterialDatas;
			for (st_CraftingCompleteItemData CraftingCompleteItemData : FindCraftingCategoryData->CraftingCompleteItems)
			{
				if (CraftingCompleteItemData.CraftingCompleteItemDataId == (en_SmallItemCategory)ReqCraftingItemType)
				{
					RequireMaterialDatas = CraftingCompleteItemData.CraftingMaterials;
				}
			}

			// ����� DB�� ���� ������Ʈ�� ������ ������ �����ϱ� ���� Job
			st_GameServerJob* DBInventorySaveJob = _GameServerJobMemoryPool->Alloc();
			DBInventorySaveJob->Type = en_GameServerJobType::DATA_BASE_CRAFTING_ITEM_INVENTORY_SAVE;			

			CGameServerMessage* DBCraftingItemSaveMessage = CGameServerMessage::GameServerMessageAlloc();
			if (DBCraftingItemSaveMessage == nullptr)
			{
				break;
			}

			DBCraftingItemSaveMessage->Clear();

			*DBCraftingItemSaveMessage << Session->SessionId;			
			// Ÿ�� ObjectId
			*DBCraftingItemSaveMessage << MyPlayer->_GameObjectInfo.ObjectId;

			bool InventoryCheck = true;
			// �ϼ� ������ ����µ� �ʿ��� ��� ����
			*DBCraftingItemSaveMessage << (int8)ReqMaterials.size();

			// �κ��丮�� ������� �ִ��� Ȯ��
			for (st_CraftingMaterialItemInfo CraftingMaterialItemInfo : ReqMaterials)
			{
				// �κ��丮�� ����� ����� ������ ��
				vector<CItem*> FindMaterialItem;
				vector<CItem*> FindMaterials = MyPlayer->_InventoryManager.FindAllInventoryItem(0, CraftingMaterialItemInfo.MaterialItemType);

				if (FindMaterials.size() > 0)
				{
					for (CItem* FindMaterialItem : FindMaterials)
					{
						// �������� �Ѱ� ���鶧 �ʿ��� ����� ������ ���´�.
						int16 OneReqMaterialCount = 0;
						for (st_CraftingMaterialItemData ReqMaterialCountData : RequireMaterialDatas)
						{
							if (FindMaterialItem->_ItemInfo.ItemSmallCategory == ReqMaterialCountData.MaterialDataId)
							{
								OneReqMaterialCount = ReqMaterialCountData.MaterialCount;
								break;
							}
						}

						// Ŭ�� ��û�� ���� * �Ѱ� ���鶧 �ʿ��� ���� ��ŭ �κ��丮���� �����Ѵ�.
						int16 ReqCraftingItemTotalCount = ReqCraftingItemCount * OneReqMaterialCount;
						FindMaterialItem->_ItemInfo.ItemCount -= ReqCraftingItemTotalCount;

						// �ռ� ������Ʈ�� �������� ������ DB�� ������Ʈ �ϱ� ���� ��´�.
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

			// ����� Ȯ���۾� �Ϸ�
			// ������ ���� �� �κ��丮�� �ְ� Ŭ�󿡰Ե� ����

			// ������� �ϴ� �������� DataManager���� �����´�.
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

			int16 FindItemGridPositionX = -1;
			int16 FindItemGridPositionY = -1;

			CItem* FindItem = MyPlayer->_InventoryManager.FindInventoryItem(0, CraftingItemInfo.ItemSmallCategory);
			if (FindItem == nullptr)
			{
				CWeapon* CraftingWeaponItem = nullptr;
				CArmor* CraftingArmorItem = nullptr;
				CMaterial* CraftingMaterialItem = nullptr;

				switch (FindReqCompleteItemData->SmallItemCategory)
				{
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
					CraftingWeaponItem = (CWeapon*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_WEAPON);
					CraftingWeaponItem->_ItemInfo = CraftingItemInfo;

					MyPlayer->_InventoryManager.InsertItem(0, CraftingWeaponItem);

					FindItemGridPositionX = CraftingWeaponItem->_ItemInfo.TileGridPositionX;
					FindItemGridPositionY = CraftingWeaponItem->_ItemInfo.TileGridPositionY;
					break;
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
					CraftingArmorItem = (CArmor*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_ARMOR);
					CraftingArmorItem->_ItemInfo = CraftingItemInfo;

					MyPlayer->_InventoryManager.InsertItem(0, CraftingArmorItem);

					FindItemGridPositionX = CraftingArmorItem->_ItemInfo.TileGridPositionX;
					FindItemGridPositionY = CraftingArmorItem->_ItemInfo.TileGridPositionY;
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
					CraftingMaterialItem = (CMaterial*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_MATERIAL);
					CraftingMaterialItem->_ItemInfo = CraftingItemInfo;

					MyPlayer->_InventoryManager.InsertItem(0, CraftingMaterialItem);

					FindItemGridPositionX = CraftingMaterialItem->_ItemInfo.TileGridPositionX;
					FindItemGridPositionY = CraftingMaterialItem->_ItemInfo.TileGridPositionY;
					break;
				default:
					break;
				}
			}
			else
			{
				// �κ��丮�� �������� �̹� ������ ������ �÷��ش�.

				IsExistItem = true;
				FindItem->_ItemInfo.ItemCount += ReqCraftingItemCount;

				FindItemGridPositionX = FindItem->_ItemInfo.TileGridPositionX;
				FindItemGridPositionY = FindItem->_ItemInfo.TileGridPositionY;
			}

			CItem* CraftingItemCompleteItem = MyPlayer->_InventoryManager.GetItem(0, FindItemGridPositionX, FindItemGridPositionY);

			// �ߺ� ����
			*DBCraftingItemSaveMessage << IsExistItem;
			// ������ ������ ����
			*DBCraftingItemSaveMessage << ReqCraftingItemCount;
			// �۾� �Ϸ�� ������ ����
			*DBCraftingItemSaveMessage << &CraftingItemCompleteItem;
			*DBCraftingItemSaveMessage << AccountId;

			DBInventorySaveJob->Message = DBCraftingItemSaveMessage;			

			_GameServerUserDBThreadMessageQue.Enqueue(DBInventorySaveJob);
			SetEvent(_UserDataBaseWakeEvent);
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

			// AccountId�� �´��� Ȯ��
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			*Message >> PlayerDBId;

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

			// �����ϰ� �ִ� �÷��̾ �ִ��� Ȯ�� 
			if (MyPlayer == nullptr)
			{
				Disconnect(Session->SessionId);
				break;
			}
			else
			{
				// �����ϰ� �ִ� �÷��̾�� ���۹��� PlayerId�� ������ Ȯ��
				if (MyPlayer->_GameObjectInfo.ObjectId != PlayerDBId)
				{
					Disconnect(Session->SessionId);
					break;
				}
			}

			int8 ItemLargeCategory;
			int8 ItemMediumCategory;
			int16 ItemSmallCategory;

			// Ŭ�󿡼� ��û�� ��� ������ ����
			st_ItemInfo UseItemInfo;
			*Message >> UseItemInfo.ItemDBId;
			*Message >> UseItemInfo.ItemIsQuickSlotUse;
			*Message >> ItemLargeCategory;
			UseItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
			*Message >> ItemMediumCategory;
			UseItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
			*Message >> ItemSmallCategory;
			UseItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;

			int16 ItemNameLen;
			*Message >> ItemNameLen;
			Message->GetData(UseItemInfo.ItemName, ItemNameLen);

			*Message >> UseItemInfo.ItemMinDamage;
			*Message >> UseItemInfo.ItemMaxDamage;
			*Message >> UseItemInfo.ItemDefence;
			*Message >> UseItemInfo.ItemMaxCount;
			*Message >> UseItemInfo.ItemCount;

			int16 ItemThumbnailImagePathLen;
			*Message >> ItemThumbnailImagePathLen;
			Message->GetData(UseItemInfo.ItemThumbnailImagePath, ItemThumbnailImagePathLen);
			*Message >> UseItemInfo.ItemIsEquipped;

			// �κ��丮�� �������� �ִ��� Ȯ��
			/*CItem* UseItem = MyPlayer->_Inventory.Get(UseItemInfo.ItemSlotIndex);
			if (UseItem->_ItemInfo != UseItemInfo)
			{
				CRASH("��û�� ��� �����۰� �κ��丮�� �������� �������� �ٸ�");
			}*/

			InterlockedIncrement64(&Session->IOBlock->IOCount);

			st_GameServerJob* DBItemEquipJob = _GameServerJobMemoryPool->Alloc();
			DBItemEquipJob->Type = en_GameServerJobType::DATA_BASE_ITEM_USE;			

			CGameServerMessage* DBItemEquipMessage = CGameServerMessage::GameServerMessageAlloc();
			DBItemEquipMessage->Clear();

			// ������ �����۰� ���������� �������� ������ ����
			//*DBItemEquipMessage << &UseItem;

			DBItemEquipJob->Message = DBItemEquipMessage;

			_GameServerUserDBThreadMessageQue.Enqueue(DBItemEquipJob);
			SetEvent(_UserDataBaseWakeEvent);
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqItemLooting(int64 SessionId, CMessage* Message)
{
	// ������ �ݱ� ó��
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

			// AccountId�� �´��� Ȯ��
			if (Session->AccountId != AccountId)
			{
				Disconnect(Session->SessionId);
				break;
			}

			st_PositionInfo ItemPosition;
			int8 ItemState;
			int8 ItemMoveDir;

			*Message >> ItemState;
			*Message >> ItemPosition.CollisionPositionX;
			*Message >> ItemPosition.CollisionPositionY;
			*Message >> ItemMoveDir;

			st_Vector2Int ItemCellPosition;
			ItemCellPosition._X = ItemPosition.CollisionPositionX;
			ItemCellPosition._Y = ItemPosition.CollisionPositionY;

			int64 MapID = 1;
			// ���� ��ġ�� �����۵��� �����´�.
			CItem** Items = G_MapManager->GetMap(MapID)->FindItem(ItemCellPosition);
			if (Items != nullptr)
			{
				for (int8 i = 0; i < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; i++)
				{
					if (Items[i] == nullptr)
					{
						continue;
					}

					// ���ӿ� ������ ĳ���͸� �����´�.
					CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

					int64 TargetObjectId = MyPlayer->_GameObjectInfo.ObjectId;

					CPlayer* TargetPlayer = (CPlayer*)(MyPlayer->GetChannel()->FindChannelObject(TargetObjectId, en_GameObjectType::OBJECT_PLAYER));
					if (TargetPlayer == nullptr)
					{
						break;
					}

					// ���� �������� ���濡 �ִ� ���������� �ƴ��� Ȯ��
					bool IsExistItem = false;
					// ���濡 ������ ������ ���� ( �ʿ� �����Ǿ� �ִ� �������� ���� )
					int16 ItemEach = Items[i]->_ItemInfo.ItemCount;

					// ���� �������� ������ Ÿ���� Ȯ��
					switch (Items[i]->_ItemInfo.ItemSmallCategory)
					{
					case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
					case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
					case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:
						TargetPlayer->_InventoryManager.InsertMoney(0, Items[i]);

						{
							CMessage* ResMoneyToInventoryPacket = MakePacketResMoneyToInventory(TargetObjectId,								
								TargetPlayer->_InventoryManager._GoldCoinCount,
								TargetPlayer->_InventoryManager._SliverCoinCount,
								TargetPlayer->_InventoryManager._BronzeCoinCount,
								Items[i]->_ItemInfo.ItemSmallCategory,
								ItemEach);
							SendPacket(Session->SessionId, ResMoneyToInventoryPacket);
							ResMoneyToInventoryPacket->Free();
						}
						break;						
					default:
						{
							// �������� ���濡 �ִ��� ã�´�.						
							CItem* FindItem = TargetPlayer->_InventoryManager.FindInventoryItem(0, Items[i]->_ItemInfo.ItemSmallCategory);
							if (FindItem == nullptr)
							{
								// ã�� ������ ���
								// ��� �ִ� ������ ã�Ƽ� �ش� ������ �������� �ִ´�.
								CItem* NewItem = NewItemCrate(Items[i]->_ItemInfo);
								TargetPlayer->_InventoryManager.InsertItem(0, NewItem);

								FindItem = TargetPlayer->_InventoryManager.GetItem(0, NewItem->_ItemInfo.TileGridPositionX, NewItem->_ItemInfo.TileGridPositionY);
							}
							else
							{
								IsExistItem = true;
								// ã���� ���
								FindItem->_ItemInfo.ItemCount += Items[i]->_ItemInfo.ItemCount;
							}

							CMessage* ResItemToInventoryPacket = MakePacketResItemToInventory(TargetObjectId, FindItem, IsExistItem, ItemEach);
							SendPacket(Session->SessionId, ResItemToInventoryPacket);
							ResItemToInventoryPacket->Free();
						}
						break;
					}				

					// �ֿ� �������� ä�ο��� �����Ŵ
					st_GameObjectJob* LeaveChannelItemJob = MakeGameObjectJobLeaveChannel(Items[i]);

					CMap* LeaveMap = G_MapManager->GetMap(1);
					CChannel* LeaveChannel = LeaveMap->GetChannelManager()->Find(1);
					LeaveChannel->_ChannelJobQue.Enqueue(LeaveChannelItemJob);
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

		// AccountServer�� �Է¹��� AccountID�� �ִ��� Ȯ���Ѵ�.
		// ������ ��쿡�� ��ū�� Ȯ������ �ʴ´�.

		int DBToken = 0;
		BYTE OutToken[ACCOUNT_TOKEN_LEN] = { 0 };

		if (Session->IsDummy == false)
		{
			// AccountNo�� Token���� AccountServerDB �����ؼ� �����Ͱ� �ִ��� Ȯ��
			CDBConnection* AccountDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);

			SP::CDBAccountTokenGet AccountTokenGet(*AccountDBConnection);
			AccountTokenGet.InAccountID(ClientAccountId); // AccountId �Է�

			TIMESTAMP_STRUCT TokenCreateTime;

			AccountTokenGet.OutTokenTime(TokenCreateTime);
			AccountTokenGet.OutToken(OutToken);

			AccountTokenGet.Execute();

			AccountTokenGet.Fetch();

			G_DBConnectionPool->Push(en_DBConnect::ACCOUNT, AccountDBConnection); // Ǯ �ݳ�
		}

		// DB ��ū�� Ŭ��κ��� �� ��ū�� ������ �α��� ��������
		if (Session->IsDummy == true || memcmp(Session->Token, OutToken, ACCOUNT_TOKEN_LEN) == 0)
		{
			Session->IsLogin = true;
			// Ŭ�� �����ϰ� �ִ� �÷��̾���� DB�κ��� �ܾ�´�.
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
				// �÷��̾� ���� ����			
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

			// Ŭ�󿡰� �α��� ���� ��Ŷ�� �����鼭 ĳ������ ������ �Բ� ������.
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
		// ��û�� ���ӿ�����Ʈ Ÿ��
		int16 ReqGameObjectType;
		*Message >> ReqGameObjectType;

		// ��û�� ĳ���� ���� �ε���
		int8 ReqCharacterCreateSlotIndex;
		*Message >> ReqCharacterCreateSlotIndex;

#pragma region ������ ĳ���Ͱ� DB�� �ִ��� Ȯ��
		// ��û�� Ŭ���� ������ ĳ������ �̸��� �����´�.
		wstring CreateCharacterName = Session->CreateCharacterName;

		// GameServerDB�� �ش� ĳ���Ͱ� �̹� �ִ��� Ȯ���Ѵ�.
		CDBConnection* FindCharacterNameGameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		// GameServerDB�� ĳ���� �̸��� �ִ��� Ȯ���ϴ� ���ν��� Ŭ����
		SP::CDBGameServerCharacterNameGet CharacterNameGet(*FindCharacterNameGameServerDBConnection);
		CharacterNameGet.InCharacterName(CreateCharacterName);

		// DB ��û ����
		CharacterNameGet.Execute();

		// ��� �ޱ�
		bool CharacterNameFind = CharacterNameGet.Fetch();

		// DBConnection �ݳ�
		G_DBConnectionPool->Push(en_DBConnect::GAME, FindCharacterNameGameServerDBConnection);
#pragma endregion		
#pragma region ĳ���� ���� �� Ŭ�� ĳ���� ���� ���� ������
		// ĳ���Ͱ� �������� ���� ���
		if (!CharacterNameFind)
		{
			// ��û�� ĳ������ 1������ �ش��ϴ� �����͸� �о�´�.
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

			// �ռ� �о�� ĳ���� ������ ���� DB�� ����
			// DBConnection Pool���� DB������ ���ؼ� �ϳ��� �����´�.
			CDBConnection* NewCharacterPushDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			// GameServerDB�� ���ο� ĳ���� �����ϴ� ���ν��� Ŭ����

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

			// DB ��û ����
			bool SaveNewCharacterQuery = NewCharacterPush.Execute();
			if (SaveNewCharacterQuery == true)
			{
				// �ռ� ������ ĳ������ DBId�� AccountId�� �̿��� ���´�.
				CDBConnection* PlayerDBIDGetDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				// GameServerDB�� ������ ĳ������ PlayerDBId�� �о���� ���ν��� Ŭ����
				SP::CDBGameServerPlayerDBIDGet PlayerDBIDGet(*PlayerDBIDGetDBConnection);
				PlayerDBIDGet.InAccountID(Session->AccountId);
				PlayerDBIDGet.InPlayerSlotIndex(ReqCharacterCreateSlotIndex);

				PlayerDBIDGet.OutPlayerDBID(PlayerDBId);

				// DB ��û ����
				bool GetNewCharacterPlayerDBId = PlayerDBIDGet.Execute();

				if (GetNewCharacterPlayerDBId == true)
				{
					PlayerDBIDGet.Fetch();

					// DB�� ����� �� ĳ���� ����
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

					// DB�� ���ο� �κ��丮 ����
					for (int16 Y = 0; Y < (int8)en_InventoryManager::INVENTORY_DEFAULT_HEIGHT_SIZE; Y++)
					{
						for (int16 X = 0; X < (int8)en_InventoryManager::INVENTORY_DEFAULT_WIDH_SIZE; X++)
						{
							CDBConnection* DBItemToInventoryConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
							SP::CDBGameServerItemCreateToInventory ItemToInventory(*DBItemToInventoryConnection);
							st_ItemInfo NewItem;

							NewItem.ItemName = L"";
							NewItem.ItemThumbnailImagePath = L"";

							ItemToInventory.InItemName(NewItem.ItemName);
							ItemToInventory.InItemTileGridPositionX(X);
							ItemToInventory.InItemTileGridPositionY(Y);
							ItemToInventory.InThumbnailImagePath(NewItem.ItemThumbnailImagePath);
							ItemToInventory.InOwnerAccountId(Session->AccountId);
							ItemToInventory.InOwnerPlayerId(PlayerDBId);

							ItemToInventory.Execute();

							G_DBConnectionPool->Push(en_DBConnect::GAME, DBItemToInventoryConnection);
						}
					}

					// ��� ���̺� ���� ����
					CDBConnection* DBNewGoldTableCreateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
					SP::CDBGameServerGoldTableCreatePush GoldTableCreate(*DBNewGoldTableCreateConnection);

					GoldTableCreate.InAccountDBId(Session->AccountId);
					GoldTableCreate.InPlayerDBId(PlayerDBId);

					GoldTableCreate.Execute();

					G_DBConnectionPool->Push(en_DBConnect::GAME, DBNewGoldTableCreateConnection);					

					int16 DefaultKey = (int16)UnityEngine::Alpha1;

					// DB�� �����Թ� ����
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

					// �⺻ ��ų ����
					PlayerSkillCreate(Session->AccountId, NewPlayerCharacter->_GameObjectInfo, ReqCharacterCreateSlotIndex);

					// �⺻ ���� ��ų ������ 1���� ���
					st_QuickSlotBarSlotInfo DefaultAttackSkillQuickSlotInfo;
					DefaultAttackSkillQuickSlotInfo.AccountDBId = Session->AccountId;
					DefaultAttackSkillQuickSlotInfo.PlayerDBId = NewPlayerCharacter->_GameObjectInfo.ObjectId;
					DefaultAttackSkillQuickSlotInfo.QuickSlotBarIndex = 0;
					DefaultAttackSkillQuickSlotInfo.QuickSlotBarSlotIndex = 0;
					DefaultAttackSkillQuickSlotInfo.QuickSlotKey = 49;

					int8 DefaultSkillLargeCategory = (int8)en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
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

					// ĳ���� ���� ���� ����
					CMessage* ResCreateCharacterMessage = MakePacketResCreateCharacter(!CharacterNameFind, NewPlayerCharacter->_GameObjectInfo);
					SendPacket(Session->SessionId, ResCreateCharacterMessage);
					ResCreateCharacterMessage->Free();
				}

				G_DBConnectionPool->Push(en_DBConnect::GAME, PlayerDBIDGetDBConnection);
			}
			else
			{

			}

			// DBConnection �ݳ�
			G_DBConnectionPool->Push(en_DBConnect::GAME, NewCharacterPushDBConnection);
		}
		else
		{
			// ĳ���Ͱ� �̹� DB�� �����Ǿ� �ִ� ���
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
				// ��� Ȯ�� �Ǹ� �ش� ������ �о����
				auto FindDropItemInfo = G_Datamanager->_Items.find((int16)DropItem.DropItemSmallCategory);
				if (FindDropItemInfo == G_Datamanager->_Items.end())
				{
					CRASH("DropItemInfo�� ã�� ����");
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
				// ��� Ȯ�� �Ǹ� �ش� ������ �о����
				auto FindDropItemInfo = G_Datamanager->_Items.find((int16)DropItem.DropItemSmallCategory);
				if (FindDropItemInfo == G_Datamanager->_Items.end())
				{
					CRASH("DropItemInfo�� ã�� ����");
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
		bool ItemRotated = false;
		int16 ItemWidth = 0;
		int16 ItemHeight = 0;
		int8 ItemLargeCategory = 0;
		int8 ItemMediumCategory = 0;
		int16 ItemSmallCategory = 0;
		wstring ItemName;
		int16 ItemCount = 0;
		wstring ItemThumbnailImagePath;
		bool ItemEquipped = false;
		int16 ItemTilePositionX = 0;
		int16 ItemTilePositionY = 0;
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
			ItemWidth = WeaponItemData->ItemWidth;
			ItemHeight = WeaponItemData->ItemHeight;
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
			ItemWidth = ArmorItemData->ItemWidth;
			ItemHeight = ArmorItemData->ItemHeight;
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
			ItemWidth = MaterialItemData->ItemWidth;
			ItemHeight = MaterialItemData->ItemHeight;
			ItemMaxCount = MaterialItemData->ItemMaxCount;
		}
		break;
		}

		int64 ItemDBId = 0;

		// ������ DB�� ����
		CDBConnection* DBCreateItemConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerCreateItem CreateItem(*DBCreateItemConnection);

		bool ItemUse = false;

		CreateItem.InItemUse(ItemUse);
		CreateItem.InIsQuickSlotUse(ItemIsQuickSlotUse);
		CreateItem.InItemRotated(ItemRotated);
		CreateItem.InItemWidth(ItemWidth);
		CreateItem.InItemHeight(ItemHeight);
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

				// �������� ���� �� ��ġ ����
				st_Vector2Int SpawnPosition(SpawnPositionX, SpawnPositionY);

				// ������ ���� ����
				st_ItemInfo NewItemInfo;
				NewItemInfo.ItemDBId = ItemDBId;
				NewItemInfo.ItemIsQuickSlotUse = ItemIsQuickSlotUse;
				NewItemInfo.Width = ItemWidth;
				NewItemInfo.Height = ItemHeight;
				NewItemInfo.TileGridPositionX = ItemTilePositionX;
				NewItemInfo.TileGridPositionY = ItemTilePositionY;
				NewItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
				NewItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
				NewItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
				NewItemInfo.ItemName = ItemName;
				NewItemInfo.ItemMaxCount = ItemMaxCount;
				NewItemInfo.ItemCount = ItemCount;
				NewItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
				NewItemInfo.ItemIsEquipped = ItemEquipped;

				// ������ ����
				CItem* NewItem = NewItemCrate(NewItemInfo);
				NewItem->_GameObjectInfo.ObjectType = DropItemData.ItemObjectType;
				NewItem->_GameObjectInfo.ObjectId = ItemDBId;
				NewItem->_GameObjectInfo.OwnerObjectId = KillerId;
				NewItem->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)KillerObjectType;
				NewItem->_SpawnPosition = SpawnPosition;

				// ������ ���忡 ����
				G_ObjectManager->ObjectEnterGame(NewItem, 1);
			}
		}
	}
}

void CGameServer::PacketProcReqDBCraftingItemToInventorySave(CGameServerMessage* Message)
{
	int64 SessionID;
	*Message >> SessionID;

	st_Session* Session = FindSession(SessionID);

	if (Session)
	{
		int64 TargetObjectId;
		*Message >> TargetObjectId;

		// ������ ������Ʈ �ؾ��� ����� ����
		int8 UpdateMaterialItemCount;
		*Message >> UpdateMaterialItemCount;

		for (int i = 0; i < UpdateMaterialItemCount; i++)
		{
			CItem* MaterialItem = nullptr;
			*Message >> &MaterialItem;

			int16 MaterialItemSmallCategory = (int16)MaterialItem->_ItemInfo.ItemSmallCategory;
			int16 MaterialItemTileGridPositonX = MaterialItem->_ItemInfo.TileGridPositionX;
			int16 MaterialItemTileGridPositonY = MaterialItem->_ItemInfo.TileGridPositionY;
			int16 MaterialItemCount = MaterialItem->_ItemInfo.ItemCount;

			if (MaterialItemCount != 0)
			{
				// ������� DB���� ���� ������Ʈ
				CDBConnection* CraftingItemToInventorySaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerItemRefreshPush InventoryItemCountRefreshPush(*CraftingItemToInventorySaveDBConnection);
				InventoryItemCountRefreshPush.InAccountDBId(Session->AccountId);
				InventoryItemCountRefreshPush.InPlayerDBId(TargetObjectId);
				InventoryItemCountRefreshPush.InItemType(MaterialItemSmallCategory);
				InventoryItemCountRefreshPush.InCount(MaterialItemCount);
				InventoryItemCountRefreshPush.InItemTileGridPositionX(MaterialItemTileGridPositonX);
				InventoryItemCountRefreshPush.InItemTileGridPositionY(MaterialItemTileGridPositonY);

				InventoryItemCountRefreshPush.Execute();

				G_DBConnectionPool->Push(en_DBConnect::GAME, CraftingItemToInventorySaveDBConnection);
			}
			else
			{
				// ���� ������ 0����� �ش� �κ��丮 ������ �ʱ�ȭ ��Ų��.
				wstring InitString = L"";

				CDBConnection* InventoryItemInitDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
				SP::CDBGameServerInventorySlotInit InventoryItemInit(*InventoryItemInitDBConnection);
				InventoryItemInit.InOwnerAccountId(Session->AccountId);
				InventoryItemInit.InOwnerPlayerId(TargetObjectId);
				InventoryItemInit.InItemTileGridPositionX(MaterialItemTileGridPositonX);
				InventoryItemInit.InItemTileGridPositionY(MaterialItemTileGridPositonY);
				InventoryItemInit.InItemName(InitString);
				InventoryItemInit.InItemThumbnailImagePath(InitString);

				InventoryItemInit.Execute();

				// ���ӿ� ������ ĳ���͸� �����´�.
				CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

				// Grid �κ��丮 �ش� �������� �������� ���̸�ŭ ����ִ� �۾� �ʿ�
				//MyPlayer->_Inventory.SlotInit(MaterialItem->_ItemInfo.ItemSlotIndex);

				G_DBConnectionPool->Push(en_DBConnect::GAME, InventoryItemInitDBConnection);
			}

			// Ŭ�󿡰� ������Ʈ ��� ����
			CMessage* ReqInventoryItemUpdate = MakePacketInventoryItemUpdate(TargetObjectId, MaterialItem->_ItemInfo);
			SendPacket(Session->SessionId, ReqInventoryItemUpdate);
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

		CDBConnection* CraftingItemToInventorySaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

		// ������ Count ����
		if (IsExistItem == true)
		{
			int16 CompleteItemSmallCategory = (int16)CompleteItem->_ItemInfo.ItemSmallCategory;
			int16 CompleteItemTileGridPositionX = CompleteItem->_ItemInfo.TileGridPositionX;
			int16 CompleteItemTileGridPositionY = CompleteItem->_ItemInfo.TileGridPositionY;
			int16 CompleteItemCount = CompleteItem->_ItemInfo.ItemCount;

			SP::CDBGameServerItemRefreshPush ItemRefreshPush(*CraftingItemToInventorySaveDBConnection);
			ItemRefreshPush.InAccountDBId(OwnerAccountId);
			ItemRefreshPush.InPlayerDBId(TargetObjectId);
			ItemRefreshPush.InItemType(CompleteItemSmallCategory);
			ItemRefreshPush.InCount(CompleteItemCount);
			ItemRefreshPush.InItemTileGridPositionX(CompleteItemTileGridPositionX);
			ItemRefreshPush.InItemTileGridPositionY(CompleteItemTileGridPositionY);

			ItemRefreshPush.Execute();
		}
		else
		{
			bool CompleteItemIsQuickSlotUse = CompleteItem->_ItemInfo.ItemIsQuickSlotUse;
			bool CompleteItemRotated = CompleteItem->_ItemInfo.Rotated;
			int16 CompleteItemWidth = CompleteItem->_ItemInfo.Width;
			int16 CompleteItemHeight = CompleteItem->_ItemInfo.Height;
			int16 CompleteItemTileGridPositionX = CompleteItem->_ItemInfo.TileGridPositionX;
			int16 CompleteItemTileGridPositionY = CompleteItem->_ItemInfo.TileGridPositionY;
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

			// ���ο� ������ ���� �� Inventory DB �ֱ�			
			SP::CDBGameServerItemToInventoryPush ItemToInventoryPush(*CraftingItemToInventorySaveDBConnection);
			ItemToInventoryPush.InIsQuickSlotUse(CompleteItemIsQuickSlotUse);
			ItemToInventoryPush.InItemRotated(CompleteItemRotated);
			ItemToInventoryPush.InItemWidth(CompleteItemWidth);
			ItemToInventoryPush.InItemHeight(CompleteItemHeight);
			ItemToInventoryPush.InItemTileGridPositionX(CompleteItemTileGridPositionX);
			ItemToInventoryPush.InItemTileGridPositionY(CompleteItemTileGridPositionY);
			ItemToInventoryPush.InItemLargeCategory(CompleteItemLargeCategory);
			ItemToInventoryPush.InItemMediumCategory(CompleteItemMediumCategory);
			ItemToInventoryPush.InItemSmallCategory(CompleteItemSmallCategory);
			ItemToInventoryPush.InItemName(CompleteItemName);
			ItemToInventoryPush.InItemMinDamage(CompleteItemMinDamage);
			ItemToInventoryPush.InItemMaxDamage(CompleteItemMaxDamage);
			ItemToInventoryPush.InItemDefence(CompleteItemDefence);
			ItemToInventoryPush.InItemMaxCount(CompleteItemMaxCount);
			ItemToInventoryPush.InItemCount(CompleteItemCount);
			ItemToInventoryPush.InIsEquipped(CompleteItemIsEquipped);
			ItemToInventoryPush.InThumbnailImagePath(CompleteItemThumbnailImagePath);
			ItemToInventoryPush.InOwnerAccountId(OwnerAccountId);
			ItemToInventoryPush.InOwnerPlayerId(TargetObjectId);

			ItemToInventoryPush.Execute();
		}

		G_DBConnectionPool->Push(en_DBConnect::GAME, CraftingItemToInventorySaveDBConnection);

		// Ŭ�� �κ��丮���� �ش� �������� ����
		CMessage* ResItemToInventoryPacket = MakePacketResItemToInventory(TargetObjectId, CompleteItem, ItemEachCount, IsExistItem);
		SendPacket(Session->SessionId, ResItemToInventoryPacket);
		ResItemToInventoryPacket->Free();
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBItemUpdate(CGameServerMessage* Message)
{
	int64 SessionID;
	*Message >> SessionID;

	st_Session* Session = FindSession(SessionID);

	if (Session)
	{
		CItem* UseItem = nullptr;
		*Message >> &UseItem;

		// ���ӿ� ������ ĳ���͸� �����´�.
		CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

		switch (UseItem->_ItemInfo.ItemSmallCategory)
		{
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
		{
			MyPlayer->_Equipment.ItemEquip(UseItem, MyPlayer);

			// DB ������ ������Ʈ
			CDBConnection* DBInventoryItemUpdateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerInventoryItemUpdate ItemUpdate(*DBInventoryItemUpdateConnection);
			ItemUpdate.InOwnerAccountId(Session->AccountId);
			ItemUpdate.InOwnerPlayerId(MyPlayer->_GameObjectInfo.ObjectId);
			ItemUpdate.InItemRotated(UseItem->_ItemInfo.Rotated);
			ItemUpdate.InItemTileGridPositionX(UseItem->_ItemInfo.TileGridPositionX);
			ItemUpdate.InItemTileGridPositionY(UseItem->_ItemInfo.TileGridPositionY);
			ItemUpdate.InItemCount(UseItem->_ItemInfo.ItemCount);
			ItemUpdate.InIsEquipped(UseItem->_ItemInfo.ItemIsEquipped);

			ItemUpdate.Execute();

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBInventoryItemUpdateConnection);

			// Ŭ�󿡰� ��� ���� ��� �˷���
			CGameServerMessage* ResInventoryItemUsePacket = MakePacketEquipmentUpdate(MyPlayer->_GameObjectInfo.ObjectId, UseItem->_ItemInfo);
			SendPacket(Session->SessionId, ResInventoryItemUsePacket);
			ResInventoryItemUsePacket->Free();
		}
		break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
		{
			MyPlayer->_GameObjectInfo.ObjectStatInfo.HP += 50;

			CMap* Map = G_MapManager->GetMap(1);

			vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(MyPlayer, 1, false);

			CGameServerMessage* ResChangeObjectStat = MakePacketResChangeObjectStat(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectStatInfo);
			SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResChangeObjectStat);
			ResChangeObjectStat->Free();
		}
		break;
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
		case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
		{
			/*wstring ErrorDistance;

			WCHAR ErrorMessage[100] = { 0 };

			wsprintf(ErrorMessage, L"[%s] ��� �� �� ���� ������ �Դϴ�.", UseItem->_ItemInfo.ItemName.c_str());
			ErrorDistance = ErrorMessage;

			CMessage* ResErrorPacket = MakePacketSkillError(MyPlayer->_GameObjectInfo.ObjectId, en_ErrorType::ERROR_NON_SELECT_OBJECT, ErrorDistance);
			SendPacket(Session->SessionId, ResErrorPacket);
			ResErrorPacket->Free();*/
		}
		break;
		default:
			CRASH("��û�� ��� ������ Ÿ���� �ùٸ��� ����");
			break;
		}
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBGoldSave(CMessage* Message)
{
	int64 SessionID;
	*Message >> SessionID;

	st_Session* Session = FindSession(SessionID);

	if (Session)
	{
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

#pragma region ��� ���̺� ��� ����
		CDBConnection* GoldSaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerGoldPush GoldSavePush(*GoldSaveDBConnection);
		GoldSavePush.InAccoountId(AccountId);
		GoldSavePush.InPlayerDBId(TargetId);
		GoldSavePush.InGoldCoin(GoldCoinCount);
		GoldSavePush.InSliverCoin(SliverCoinCount);
		GoldSavePush.InBronzeCoin(BronzeCoinCount);

		GoldSavePush.Execute();

		G_DBConnectionPool->Push(en_DBConnect::GAME, GoldSaveDBConnection);

		// Ŭ�󿡰� �� ���� ��� �˷���
		CMessage* ResGoldSaveMeesage = MakePacketResGoldSave(AccountId, TargetId, GoldCoinCount, SliverCoinCount, BronzeCoinCount, ItemCount, ItemType);
		SendPacket(Session->SessionId, ResGoldSaveMeesage);
		ResGoldSaveMeesage->Free();

		//// �� ������Ʈ ����
		//vector<CGameObject*> DeSpawnItem;
		//DeSpawnItem.push_back(ItemDBId);

		//CMessage* ResItemDeSpawnPacket = MakePacketResObjectDeSpawn(1, DeSpawnItem);
		//SendPacketFieldOfView(Session, ResItemDeSpawnPacket, true);		
		//ResItemDeSpawnPacket->Free();

		bool ItemUse = true;
		// ������ DB���� Inventory�� ������ ������ ���� ( �����۰� ���������� ItemUse�� 1�� �ٲ۴� )
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
#pragma region ĳ���� ����ġ ���� ���
			*ResCharacterInfoMessage << MyPlayer->_Experience.CurrentExperience;
			*ResCharacterInfoMessage << MyPlayer->_Experience.RequireExperience;
			*ResCharacterInfoMessage << MyPlayer->_Experience.TotalExperience;
#pragma endregion

			CDBConnection* DBCharacterInfoGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

#pragma region ��ų ���� �о����
			// ĳ���Ͱ� �����ϰ� �ִ� ��ų ������ DB�κ��� �о�´�.			
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
				// ��ų ������ ã��
				st_SkillData* FindSkillData = G_Datamanager->FindSkillData((en_SkillMediumCategory)SkillMediumCategory, (en_SkillType)SkillType);
				if (FindSkillData == nullptr)
				{
					CRASH("��ų �����͸� ã�� �� �����ϴ�.");
				}

				switch ((en_SkillMediumCategory)SkillMediumCategory)
				{
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK:
				case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK:
				{
					CSkill* AttackSkill = G_ObjectManager->SkillCreate();

					st_AttackSkillData* FindAttackSkillData = (st_AttackSkillData*)FindSkillData;

					st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(FindAttackSkillData->SkillMediumCategory);

					AttackSkillInfo->IsSkillLearn = IsSkillLearn;
					AttackSkillInfo->IsQuickSlotUse = IsQuickSlotUse;
					AttackSkillInfo->SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;
					AttackSkillInfo->SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;
					AttackSkillInfo->SkillType = (en_SkillType)SkillType;
					AttackSkillInfo->SkillLevel = SkillLevel;
					AttackSkillInfo->SkillName = (LPWSTR)CA2W(FindAttackSkillData->SkillName.c_str());

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
					AttackSkillInfo->SkillImagePath = (LPWSTR)CA2W(FindAttackSkillData->SkillThumbnailImagePath.c_str());
					AttackSkillInfo->SkillMinDamage = FindAttackSkillData->SkillMinDamage;
					AttackSkillInfo->SkillMaxDamage = FindAttackSkillData->SkillMaxDamage;
					AttackSkillInfo->SkillTargetEffectTime = FindAttackSkillData->SkillTargetEffectTime;
					AttackSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::UP, (LPWSTR)CA2W((*FindAttackSkillData->SkillAnimations.find(en_MoveDir::UP)).second.c_str())));
					AttackSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::DOWN, (LPWSTR)CA2W((*FindAttackSkillData->SkillAnimations.find(en_MoveDir::DOWN)).second.c_str())));
					AttackSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::LEFT, (LPWSTR)CA2W((*FindAttackSkillData->SkillAnimations.find(en_MoveDir::LEFT)).second.c_str())));
					AttackSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::RIGHT, (LPWSTR)CA2W((*FindAttackSkillData->SkillAnimations.find(en_MoveDir::RIGHT)).second.c_str())));
					AttackSkillInfo->NextComboSkill = FindAttackSkillData->NextComboSkill;
					AttackSkillInfo->SkillExplanation = (LPWSTR)CA2W(FindAttackSkillData->SkillExplanation.c_str());

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

					st_TacTicSkillData* FindTacTicSkillData = (st_TacTicSkillData*)FindSkillData;

					st_TacTicSkillInfo* TacTicSkillInfo = (st_TacTicSkillInfo*)G_ObjectManager->SkillInfoCreate(FindTacTicSkillData->SkillMediumCategory);
					TacTicSkillInfo->CanSkillUse = true;
					TacTicSkillInfo->IsSkillLearn = IsSkillLearn;
					TacTicSkillInfo->IsQuickSlotUse = IsQuickSlotUse;
					TacTicSkillInfo->SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;
					TacTicSkillInfo->SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;
					TacTicSkillInfo->SkillType = (en_SkillType)SkillType;
					TacTicSkillInfo->SkillLevel = SkillLevel;
					TacTicSkillInfo->SkillName = (LPWSTR)CA2W(FindTacTicSkillData->SkillName.c_str());
					TacTicSkillInfo->SkillCoolTime = FindTacTicSkillData->SkillCoolTime;
					TacTicSkillInfo->SkillCastingTime = FindTacTicSkillData->SkillCastingTime;
					TacTicSkillInfo->SkillDurationTime = FindTacTicSkillData->SkillDurationTime;
					TacTicSkillInfo->SkillDotTime = FindTacTicSkillData->SkillDotTime;
					TacTicSkillInfo->SkillRemainTime = 0;
					TacTicSkillInfo->SkillImagePath = (LPWSTR)CA2W(FindTacTicSkillData->SkillThumbnailImagePath.c_str());
					TacTicSkillInfo->SkillTargetEffectTime = FindTacTicSkillData->SkillTargetEffectTime;
					TacTicSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::UP, (LPWSTR)CA2W((*FindTacTicSkillData->SkillAnimations.find(en_MoveDir::UP)).second.c_str())));
					TacTicSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::DOWN, (LPWSTR)CA2W((*FindTacTicSkillData->SkillAnimations.find(en_MoveDir::DOWN)).second.c_str())));
					TacTicSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::LEFT, (LPWSTR)CA2W((*FindTacTicSkillData->SkillAnimations.find(en_MoveDir::LEFT)).second.c_str())));
					TacTicSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::RIGHT, (LPWSTR)CA2W((*FindTacTicSkillData->SkillAnimations.find(en_MoveDir::RIGHT)).second.c_str())));

					TacTicSkillInfo->SkillExplanation = (LPWSTR)CA2W(FindTacTicSkillData->SkillExplanation.c_str());

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

					st_HealSkillData* FindHealSkillData = (st_HealSkillData*)FindSkillData;

					st_HealSkillInfo* HealSkillInfo = (st_HealSkillInfo*)G_ObjectManager->SkillInfoCreate(FindHealSkillData->SkillMediumCategory);
					HealSkillInfo->CanSkillUse = true;
					HealSkillInfo->IsSkillLearn = IsSkillLearn;
					HealSkillInfo->IsQuickSlotUse = IsQuickSlotUse;
					HealSkillInfo->SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;
					HealSkillInfo->SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;
					HealSkillInfo->SkillType = (en_SkillType)SkillType;
					HealSkillInfo->SkillLevel = SkillLevel;
					HealSkillInfo->SkillName = (LPWSTR)CA2W(FindHealSkillData->SkillName.c_str());
					HealSkillInfo->SkillCoolTime = FindHealSkillData->SkillCoolTime;
					HealSkillInfo->SkillCastingTime = FindHealSkillData->SkillCastingTime;
					HealSkillInfo->SkillDurationTime = FindHealSkillData->SkillDurationTime;
					HealSkillInfo->SkillDotTime = FindHealSkillData->SkillDotTime;
					HealSkillInfo->SkillRemainTime = 0;
					HealSkillInfo->SkillImagePath = (LPWSTR)CA2W(FindHealSkillData->SkillThumbnailImagePath.c_str());
					HealSkillInfo->SkillMinHealPoint = FindHealSkillData->SkillMinHealPoint;
					HealSkillInfo->SkillMaxHealPoint = FindHealSkillData->SkillMaxHealPoint;
					HealSkillInfo->SkillTargetEffectTime = FindHealSkillData->SkillTargetEffectTime;
					HealSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::UP, (LPWSTR)CA2W((*FindHealSkillData->SkillAnimations.find(en_MoveDir::UP)).second.c_str())));
					HealSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::DOWN, (LPWSTR)CA2W((*FindHealSkillData->SkillAnimations.find(en_MoveDir::DOWN)).second.c_str())));
					HealSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::LEFT, (LPWSTR)CA2W((*FindHealSkillData->SkillAnimations.find(en_MoveDir::LEFT)).second.c_str())));
					HealSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::RIGHT, (LPWSTR)CA2W((*FindHealSkillData->SkillAnimations.find(en_MoveDir::RIGHT)).second.c_str())));

					HealSkillInfo->SkillExplanation = (LPWSTR)CA2W(FindHealSkillData->SkillExplanation.c_str());

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

					st_BufSkillData* FindBufSkillData = (st_BufSkillData*)FindSkillData;

					st_BufSkillInfo* BufSkillInfo = (st_BufSkillInfo*)G_ObjectManager->SkillInfoCreate(FindBufSkillData->SkillMediumCategory);
					BufSkillInfo->CanSkillUse = true;
					BufSkillInfo->IsSkillLearn = IsSkillLearn;
					BufSkillInfo->IsQuickSlotUse = IsQuickSlotUse;
					BufSkillInfo->SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;
					BufSkillInfo->SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;
					BufSkillInfo->SkillType = (en_SkillType)SkillType;
					BufSkillInfo->SkillLevel = SkillLevel;
					BufSkillInfo->SkillName = (LPWSTR)CA2W(FindBufSkillData->SkillName.c_str());
					BufSkillInfo->SkillCoolTime = FindBufSkillData->SkillCoolTime;
					BufSkillInfo->SkillCastingTime = FindBufSkillData->SkillCastingTime;
					BufSkillInfo->SkillDurationTime = FindBufSkillData->SkillDurationTime;
					BufSkillInfo->SkillDotTime = FindBufSkillData->SkillDotTime;
					BufSkillInfo->SkillRemainTime = 0;
					BufSkillInfo->SkillTargetEffectTime = FindBufSkillData->SkillTargetEffectTime;
					BufSkillInfo->SkillImagePath = (LPWSTR)CA2W(FindBufSkillData->SkillThumbnailImagePath.c_str());
					BufSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::UP, (LPWSTR)CA2W((*FindBufSkillData->SkillAnimations.find(en_MoveDir::UP)).second.c_str())));
					BufSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::DOWN, (LPWSTR)CA2W((*FindBufSkillData->SkillAnimations.find(en_MoveDir::DOWN)).second.c_str())));
					BufSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::LEFT, (LPWSTR)CA2W((*FindBufSkillData->SkillAnimations.find(en_MoveDir::LEFT)).second.c_str())));
					BufSkillInfo->SkillAnimations.insert(pair<en_MoveDir, wstring>(
						en_MoveDir::RIGHT, (LPWSTR)CA2W((*FindBufSkillData->SkillAnimations.find(en_MoveDir::RIGHT)).second.c_str())));

					BufSkillInfo->SkillExplanation = (LPWSTR)CA2W(FindBufSkillData->SkillExplanation.c_str());
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

			// ���� ��ų ����
			*ResCharacterInfoMessage << (int8)MyPlayer->_SkillBox.GetAttackSkill().size();
			// ���� ��ų ����
			*ResCharacterInfoMessage << (int8)MyPlayer->_SkillBox.GetTacTicSkill().size();
			// ���� ��ų ����
			*ResCharacterInfoMessage << (int8)MyPlayer->_SkillBox.GetBufSkill().size();

			// ���� ��ų ���� ���
			for (CSkill* AttackSkillInfo : MyPlayer->_SkillBox.GetAttackSkill())
			{
				*ResCharacterInfoMessage << *(AttackSkillInfo->GetSkillInfo());
			}

			// ���� ��ų ���� ���
			for (CSkill* TacTicSkillInfo : MyPlayer->_SkillBox.GetTacTicSkill())
			{
				*ResCharacterInfoMessage << *(TacTicSkillInfo->GetSkillInfo());
			}

			// ���� ��ų ���� ���
			for (CSkill* BufSkillInfo : MyPlayer->_SkillBox.GetBufSkill())
			{
				*ResCharacterInfoMessage << *(BufSkillInfo->GetSkillInfo());
			}			
#pragma endregion

#pragma region ������ ���� �����ͼ� Ŭ�� ������
			// ������ ���� �ʱ�ȭ
			MyPlayer->_QuickSlotManager.Init();

			vector<st_QuickSlotBarSlotInfo> QuickSlotBarSlotInfos;

			// ������ ���̺� �����ؼ� �ش� ��ų�� ��ϵǾ� �ִ� ��� ������ ��ȣ �������			
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

				CSkill* FindSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)QuickSlotSkillType);
				if (FindSkill != nullptr)
				{
					NewQuickSlotBarSlot.QuickBarSkill = FindSkill;
				}
				else
				{
					NewQuickSlotBarSlot.QuickBarSkill = nullptr;
				}

				// �����Կ� ����Ѵ�.
				MyPlayer->_QuickSlotManager.UpdateQuickSlotBar(NewQuickSlotBarSlot);
				QuickSlotBarSlotInfos.push_back(NewQuickSlotBarSlot);
			}			

			// ĳ���� ������ ���� ���
			*ResCharacterInfoMessage << (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SIZE;
			*ResCharacterInfoMessage << (int8)en_QuickSlotBar::QUICK_SLOT_BAR_SLOT_SIZE;
			*ResCharacterInfoMessage << (int8)QuickSlotBarSlotInfos.size();

			for (st_QuickSlotBarSlotInfo QuickSlotBarSlotInfo : QuickSlotBarSlotInfos)
			{
				*ResCharacterInfoMessage << QuickSlotBarSlotInfo;
			}

#pragma endregion


#pragma region ���� ������ ���� �о����
			vector<st_ItemInfo> Equipments;
			// �κ��丮 ����
			MyPlayer->_InventoryManager.InventoryCreate((int8)en_InventoryManager::INVENTORY_DEFAULT_WIDH_SIZE, (int8)en_InventoryManager::INVENTORY_DEFAULT_HEIGHT_SIZE);

			*ResCharacterInfoMessage << (int8)en_InventoryManager::INVENTORY_DEFAULT_WIDH_SIZE;
			*ResCharacterInfoMessage << (int8)en_InventoryManager::INVENTORY_DEFAULT_HEIGHT_SIZE;

			vector<CItem*> InventoryItems;

			// DB�� ��ϵǾ� �ִ� �κ��丮 �����۵��� ������ ��� �ܾ�´�.			
			SP::CDBGameServerInventoryItemGet CharacterInventoryItem(*DBCharacterInfoGetConnection);
			CharacterInventoryItem.InAccountDBId(MyPlayer->_AccountId);
			CharacterInventoryItem.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);

			bool ItemRotated;
			int16 ItemWidth;
			int16 ItemHeight;
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
			int16 ItemTilePositionX = 0;
			int16 ItemTilePositionY = 0;
			WCHAR ItemThumbnailImagePath[100] = { 0 };

			CharacterInventoryItem.OutIsRotated(ItemRotated);
			CharacterInventoryItem.OutItemWidth(ItemWidth);
			CharacterInventoryItem.OutItemHeight(ItemHeight);
			CharacterInventoryItem.OutItemLargeCategory(ItemLargeCategory);
			CharacterInventoryItem.OutItemMediumCategory(ItemMediumCategory);
			CharacterInventoryItem.OutItemSmallCategory(ItemSmallCategory);
			CharacterInventoryItem.OutItemName(ItemName);
			CharacterInventoryItem.OutMinDamage(ItemMinDamage);
			CharacterInventoryItem.OutMaxDamage(ItemMaxDamage);
			CharacterInventoryItem.OutDefence(ItemDefence);
			CharacterInventoryItem.OutMaxCount(ItemMaxCount);
			CharacterInventoryItem.OutItemCount(ItemCount);
			CharacterInventoryItem.OutItemTileGridPositionX(ItemTilePositionX);
			CharacterInventoryItem.OutItemTileGridPositionY(ItemTilePositionY);
			CharacterInventoryItem.OutIsEquipped(IsEquipped);
			CharacterInventoryItem.OutItemThumbnailImagePath(ItemThumbnailImagePath);

			CharacterInventoryItem.Execute();

			while (CharacterInventoryItem.Fetch())
			{
				CWeapon* WeaponItem = nullptr;
				CArmor* ArmorItem = nullptr;
				CItem* InventoryItem = nullptr;
				CMaterial* MaterialItem = nullptr;
				st_ItemData* ItemData = nullptr;
				CConsumable* SkillBookItem = nullptr;

				switch ((en_SmallItemCategory)ItemSmallCategory)
				{
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
					WeaponItem = (CWeapon*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_WEAPON);
					WeaponItem->_ItemInfo.ItemDBId = 0;
					WeaponItem->_ItemInfo.Rotated = ItemRotated;
					WeaponItem->_ItemInfo.Width = ItemWidth;
					WeaponItem->_ItemInfo.Height = ItemHeight;
					WeaponItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
					WeaponItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
					WeaponItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
					WeaponItem->_ItemInfo.ItemName = ItemName;
					WeaponItem->_ItemInfo.ItemCount = ItemCount;
					WeaponItem->_ItemInfo.TileGridPositionX = ItemTilePositionX;
					WeaponItem->_ItemInfo.TileGridPositionY = ItemTilePositionY;
					WeaponItem->_ItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
					WeaponItem->_ItemInfo.ItemIsEquipped = IsEquipped;
					WeaponItem->_ItemInfo.ItemMinDamage = ItemMinDamage;
					WeaponItem->_ItemInfo.ItemMaxDamage = ItemMaxDamage;

					MyPlayer->_InventoryManager.DBItemInsertItem(0, WeaponItem);

					// ������ ���̺��� �о���� ���� �������� �������� ���
					// �ش� ���⸦ �����ϰ� ���� ������ ������ ��´�.
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
					ArmorItem->_ItemInfo.Rotated = ItemRotated;
					ArmorItem->_ItemInfo.Width = ItemWidth;
					ArmorItem->_ItemInfo.Height = ItemHeight;
					ArmorItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
					ArmorItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
					ArmorItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
					ArmorItem->_ItemInfo.ItemName = ItemName;
					ArmorItem->_ItemInfo.ItemCount = ItemCount;
					ArmorItem->_ItemInfo.TileGridPositionX = ItemTilePositionX;
					ArmorItem->_ItemInfo.TileGridPositionY = ItemTilePositionY;
					ArmorItem->_ItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
					ArmorItem->_ItemInfo.ItemIsEquipped = IsEquipped;
					ArmorItem->_ItemInfo.ItemDefence = ItemDefence;

					MyPlayer->_InventoryManager.DBItemInsertItem(0, ArmorItem);

					// ������ ���̺��� �о���� �� �������� �������� ���
					// �ش� ���� �����ϰ� ���� ������ ������ ��´�.
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
					SkillBookItem->_ItemInfo.Rotated = ItemRotated;
					SkillBookItem->_ItemInfo.Width = ItemWidth;
					SkillBookItem->_ItemInfo.Height = ItemHeight;
					SkillBookItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
					SkillBookItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
					SkillBookItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
					SkillBookItem->_ItemInfo.ItemName = ItemName;
					SkillBookItem->_ItemInfo.ItemCount = ItemCount;
					SkillBookItem->_ItemInfo.TileGridPositionX = ItemTilePositionX;
					SkillBookItem->_ItemInfo.TileGridPositionY = ItemTilePositionY;
					SkillBookItem->_ItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
					SkillBookItem->_ItemInfo.ItemIsEquipped = IsEquipped;
					SkillBookItem->_ItemInfo.ItemMaxCount = ItemMaxCount;

					MyPlayer->_InventoryManager.DBItemInsertItem(0, SkillBookItem);

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
					ItemData = G_Datamanager->FindItemData((en_SmallItemCategory)ItemSmallCategory);

					MaterialItem = (CMaterial*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_MATERIAL);
					MaterialItem->_ItemInfo.ItemDBId = 0;
					MaterialItem->_ItemInfo.Rotated = ItemRotated;
					MaterialItem->_ItemInfo.Width = ItemWidth;
					MaterialItem->_ItemInfo.Height = ItemHeight;
					MaterialItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
					MaterialItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
					MaterialItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
					MaterialItem->_ItemInfo.ItemName = ItemName;
					MaterialItem->_ItemInfo.ItemExplain = (LPWSTR)CA2W(ItemData->ItemExplain.c_str());
					MaterialItem->_ItemInfo.ItemCount = ItemCount;
					MaterialItem->_ItemInfo.TileGridPositionX = ItemTilePositionX;
					MaterialItem->_ItemInfo.TileGridPositionY = ItemTilePositionY;
					MaterialItem->_ItemInfo.ItemThumbnailImagePath = ItemThumbnailImagePath;
					MaterialItem->_ItemInfo.ItemIsEquipped = IsEquipped;
					MaterialItem->_ItemInfo.ItemMaxCount = ItemMaxCount;

					MyPlayer->_InventoryManager.DBItemInsertItem(0, MaterialItem);

					InventoryItems.push_back(MaterialItem);
					break;
				default:
					break;
				}
			}			

			// Ŭ�� �κ��丮 ���� ���			
			*ResCharacterInfoMessage << (int8)InventoryItems.size();

			for (CItem* Item : InventoryItems)
			{
				*ResCharacterInfoMessage << Item;
			}

#pragma endregion			

#pragma region ��� ���� �����ֱ�
			// ��� ���� ���
			*ResCharacterInfoMessage << (int8)Equipments.size();
			for (st_ItemInfo EquipmentItemInfo : Equipments)
			{
				*ResCharacterInfoMessage << EquipmentItemInfo;
			}
#pragma endregion


#pragma region ��� ���� �о����
			// ĳ���Ͱ� �����ϰ� �־��� ��� ������ GoldTable���� �о�´�.			
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
				// DB���� �о�� Gold�� Inventory�� �����Ѵ�.
				MyPlayer->_InventoryManager._GoldCoinCount = GoldCoin;
				MyPlayer->_InventoryManager._SliverCoinCount = SliverCoin;
				MyPlayer->_InventoryManager._BronzeCoinCount = BronzeCoin;				

				// ��� ���� ���
				*ResCharacterInfoMessage << GoldCoin;
				*ResCharacterInfoMessage << SliverCoin;
				*ResCharacterInfoMessage << BronzeCoin;				
			}			
#pragma endregion	

#pragma region ������ ���� ������			
			vector<st_CraftingItemCategory> CraftingItemCategorys;

			for (int8 Category = (int8)en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON; Category <= (int8)en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL; ++Category)
			{
				auto FindCraftingIterator = G_Datamanager->_CraftingData.find(Category);
				if (FindCraftingIterator == G_Datamanager->_CraftingData.end())
				{
					continue;
				}

				st_CraftingItemCategoryData* CraftingData = (*FindCraftingIterator).second;

				// ������ ī�װ� ����
				st_CraftingItemCategory CraftingItemCategory;
				CraftingItemCategory.CategoryType = CraftingData->CraftingType;
				CraftingItemCategory.CategoryName = (LPWSTR)CA2W(CraftingData->CraftingTypeName.c_str());

				// ������ ī�װ��� ���� ������ ��������
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

			*ResCharacterInfoMessage << (int8)CraftingItemCategorys.size();

			for (st_CraftingItemCategory CraftingItemCategory : CraftingItemCategorys)
			{
				*ResCharacterInfoMessage << (int8)CraftingItemCategory.CategoryType;

				int16 CraftingItemCategoryNameLen = (int16)(CraftingItemCategory.CategoryName.length() * 2);
				*ResCharacterInfoMessage << CraftingItemCategoryNameLen;
				ResCharacterInfoMessage->InsertData(CraftingItemCategory.CategoryName.c_str(), CraftingItemCategoryNameLen);

				*ResCharacterInfoMessage << (int8)CraftingItemCategory.CompleteItems.size();

				for (st_CraftingCompleteItem CraftingCompleteItem : CraftingItemCategory.CompleteItems)
				{
					*ResCharacterInfoMessage << (int16)CraftingCompleteItem.CompleteItemType;

					// ���� �ϼ��� �̸�
					int16 CraftingCompleteItemNameLen = (int16)(CraftingCompleteItem.CompleteItemName.length() * 2);
					*ResCharacterInfoMessage << CraftingCompleteItemNameLen;
					ResCharacterInfoMessage->InsertData(CraftingCompleteItem.CompleteItemName.c_str(), CraftingCompleteItemNameLen);

					// ���� �ϼ��� �̹��� ���
					int16 CraftingCompleteItemImagePathLen = (int16)(CraftingCompleteItem.CompleteItemImagePath.length() * 2);
					*ResCharacterInfoMessage << CraftingCompleteItemImagePathLen;
					ResCharacterInfoMessage->InsertData(CraftingCompleteItem.CompleteItemImagePath.c_str(), CraftingCompleteItemImagePathLen);

					*ResCharacterInfoMessage << (int8)CraftingCompleteItem.Materials.size();

					for (st_CraftingMaterialItemInfo CraftingMaterialItem : CraftingCompleteItem.Materials)
					{
						*ResCharacterInfoMessage << MyPlayer->_AccountId;
						*ResCharacterInfoMessage << MyPlayer->_GameObjectInfo.ObjectId;
						*ResCharacterInfoMessage << (int16)CraftingMaterialItem.MaterialItemType;

						// ����� �̸�
						int16 CraftingMaterialItemNameLen = (int16)(CraftingMaterialItem.MaterialItemName.length() * 2);
						*ResCharacterInfoMessage << CraftingMaterialItemNameLen;
						ResCharacterInfoMessage->InsertData(CraftingMaterialItem.MaterialItemName.c_str(), CraftingMaterialItemNameLen);

						*ResCharacterInfoMessage << CraftingMaterialItem.ItemCount;

						// ����� �̹��� ���
						int16 MaterialImagePathLen = (int16)(CraftingMaterialItem.MaterialItemImagePath.length() * 2);
						*ResCharacterInfoMessage << MaterialImagePathLen;
						ResCharacterInfoMessage->InsertData(CraftingMaterialItem.MaterialItemImagePath.c_str(), MaterialImagePathLen);
					}
				}
			}						

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterInfoGetConnection);

			SendPacket(MyPlayer->_SessionId, ResCharacterInfoMessage);
			
			MyPlayer->_NetworkState = en_ObjectNetworkState::LIVE;			
#pragma endregion
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
	LeavePlayerStatInfoSave.InLastPositionY(MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY);
	LeavePlayerStatInfoSave.InLastPositionX(MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX);
	LeavePlayerStatInfoSave.InCurrentExperience(MyPlayer->_Experience.CurrentExperience);
	LeavePlayerStatInfoSave.InRequireExperience(MyPlayer->_Experience.RequireExperience);
	LeavePlayerStatInfoSave.InTotalExperience(MyPlayer->_Experience.TotalExperience);

	LeavePlayerStatInfoSave.Execute();	

	SP::CDBGameServerQuickSlotBarSlotUpdate QuickSlotDBupdate(*PlayerInfoSaveDBConnection);

	for (auto QuickSlotIterator : MyPlayer->_QuickSlotManager.GetQuickSlotBar())
	{
		for (auto QuickSlotBarSlotIterator : QuickSlotIterator.second->_QuickSlotBarSlotInfos)
		{
			st_QuickSlotBarSlotInfo* SaveQuickSlotInfo = QuickSlotBarSlotIterator.second;

			if (SaveQuickSlotInfo->QuickBarSkill != nullptr)
			{
				int8 SaveQuickSlotInfoSkillLargeCategory = (int8)SaveQuickSlotInfo->QuickBarSkill->GetSkillInfo()->SkillLargeCategory;
				int8 SaveQuickSlotInfoSkillMediumCategory = (int8)SaveQuickSlotInfo->QuickBarSkill->GetSkillInfo()->SkillMediumCategory;
				int16 SaveQuickSlotInfoSkillSkillType = (int8)SaveQuickSlotInfo->QuickBarSkill->GetSkillInfo()->SkillType;


;				QuickSlotDBupdate.InAccountDBId(MyPlayer->_AccountId);
				QuickSlotDBupdate.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);
				QuickSlotDBupdate.InQuickSlotBarIndex(SaveQuickSlotInfo->QuickSlotBarIndex);
				QuickSlotDBupdate.InQuickSlotBarSlotIndex(SaveQuickSlotInfo->QuickSlotBarSlotIndex);
				QuickSlotDBupdate.InQuickSlotKey(SaveQuickSlotInfo->QuickSlotKey);
				QuickSlotDBupdate.InSkillLargeCategory(SaveQuickSlotInfoSkillLargeCategory);
				QuickSlotDBupdate.InSkillMediumCategory(SaveQuickSlotInfoSkillMediumCategory);
				QuickSlotDBupdate.InSkillType(SaveQuickSlotInfoSkillSkillType);
				QuickSlotDBupdate.InSkillLevel(SaveQuickSlotInfo->QuickBarSkill->GetSkillInfo()->SkillLevel);

				QuickSlotDBupdate.Execute();
			}
		}
	}

	// ���� ���� DB�� ����
	if (MyPlayer->_InventoryManager._Inventorys[0] != nullptr)
	{
		SP::CDBGameServerInventoryPlace LeavePlayerInventoryItemSave(*PlayerInfoSaveDBConnection);
		LeavePlayerInventoryItemSave.InOwnerAccountId(MyPlayer->_AccountId);
		LeavePlayerInventoryItemSave.InOwnerPlayerId(MyPlayer->_GameObjectInfo.ObjectId);

		vector<st_ItemInfo> PlayerInventoryItems = MyPlayer->_InventoryManager._Inventorys[0]->DBInventorySaveReturnItems();

		for (st_ItemInfo InventoryItem : PlayerInventoryItems)
		{
			int8 InventoryItemLargeCategory = (int8)InventoryItem.ItemLargeCategory;
			int8 InventoryItemMediumCategory = (int8)InventoryItem.ItemMediumCategory;
			int16 InventoryItemSmallCategory = (int16)InventoryItem.ItemSmallCategory;

			LeavePlayerInventoryItemSave.InIsQuickSlotUse(InventoryItem.ItemIsQuickSlotUse);
			LeavePlayerInventoryItemSave.InItemRotated(InventoryItem.ItemIsQuickSlotUse);
			LeavePlayerInventoryItemSave.InItemWidth(InventoryItem.Width);
			LeavePlayerInventoryItemSave.InItemHeight(InventoryItem.Height);
			LeavePlayerInventoryItemSave.InItemTileGridPositionX(InventoryItem.TileGridPositionX);
			LeavePlayerInventoryItemSave.InItemTileGridPositionY(InventoryItem.TileGridPositionY);
			LeavePlayerInventoryItemSave.InItemLargeCategory(InventoryItemLargeCategory);
			LeavePlayerInventoryItemSave.InItemMediumCategory(InventoryItemMediumCategory);
			LeavePlayerInventoryItemSave.InItemSmallCategory(InventoryItemSmallCategory);
			LeavePlayerInventoryItemSave.InItemName(InventoryItem.ItemName);
			LeavePlayerInventoryItemSave.InItemCount(InventoryItem.ItemCount);
			LeavePlayerInventoryItemSave.InIsEquipped(InventoryItem.ItemIsEquipped);
			LeavePlayerInventoryItemSave.InItemMinDamage(InventoryItem.ItemMinDamage);
			LeavePlayerInventoryItemSave.InItemMaxDamage(InventoryItem.ItemMaxDamage);
			LeavePlayerInventoryItemSave.InItemDefence(InventoryItem.ItemDefence);
			LeavePlayerInventoryItemSave.InItemMaxCount(InventoryItem.ItemMaxCount);
			LeavePlayerInventoryItemSave.InItemThumbnailImagePath(InventoryItem.ItemThumbnailImagePath);

			LeavePlayerInventoryItemSave.Execute();
		}
	}

	G_DBConnectionPool->Push(en_DBConnect::GAME, PlayerInfoSaveDBConnection);			

	if (MyPlayer->GetChannel() != nullptr)
	{
		st_GameObjectJob* LeaveGameJob = MakeGameObjectJobLeaveChannelPlayer(MyPlayer, LeaveSession->MyPlayerIndexes);

		CMap* LeaveMap = G_MapManager->GetMap(1);
		CChannel* LeaveChannel = LeaveMap->GetChannelManager()->Find(1);
		LeaveChannel->_ChannelJobQue.Enqueue(LeaveGameJob);
	}	
	
	// GameServer�� ���õ� Session ���� �ʱ�ȭ
	memset(LeaveSession->Token, 0, sizeof(LeaveSession->Token));

	LeaveSession->IsLogin = false;
	LeaveSession->IsDummy = false;

	LeaveSession->MyPlayerIndex = -1;
	LeaveSession->AccountId = 0;

	LeaveSession->ClientSock = INVALID_SOCKET;
	closesocket(LeaveSession->CloseSock);

	InterlockedDecrement64(&_SessionCount);
	// ���� �ε��� �ݳ�	
	_SessionArrayIndexs.Push(GET_SESSIONINDEX(LeaveSession->SessionId));	
}

void CGameServer::PacketProcTimerObjectSpawn(CGameServerMessage* Message)
{
	int16 SpawnObjectType;
	*Message >> SpawnObjectType;

	st_Vector2Int SpawnPosition;
	*Message >> SpawnPosition._X;
	*Message >> SpawnPosition._Y;

	Message->Free();

	CMap* ObjectEnterMap = G_MapManager->GetMap(1);
	if (ObjectEnterMap != nullptr)
	{
		CGameObject* FindObject = ObjectEnterMap->Find(SpawnPosition);
		if (FindObject != nullptr)
		{
			SpawnObjectTimeTimerJobCreate(SpawnObjectType, SpawnPosition, 10000);
		}
		else
		{
			G_ObjectManager->ObjectSpawn((en_GameObjectType)SpawnObjectType, SpawnPosition);
		}
	}	
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
				// �� ��Ŷ ����
				CGameServerMessage* PingMessage = MakePacketPing();
				SendPacket(Session->SessionId, PingMessage);
				PingMessage->Free();

				// �� ��Ŷ ����
				PingTimerCreate(Session);
			}

			ReturnSession(Session);
		}
	} while (0);
}

st_GameObjectJob* CGameServer::MakeGameObjectJobEnterChannel(CGameObject* EnterChannelObject)
{
	st_GameObjectJob* EnterChannelJob = G_ObjectManager->GameObjectJobCreate();
	EnterChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_ENTER_CHANNEL;

	CGameServerMessage* EnterChannelGameMessage = CGameServerMessage::GameServerMessageAlloc();
	EnterChannelGameMessage->Clear();

	*EnterChannelGameMessage << &EnterChannelObject;
	EnterChannelJob->GameObjectJobMessage = EnterChannelGameMessage;

	return EnterChannelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobLeaveChannel(CGameObject* LeaveChannelObject)
{
	st_GameObjectJob* LeaveChannelJob = G_ObjectManager->GameObjectJobCreate();
	LeaveChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_LEAVE_CHANNEL;

	CGameServerMessage* LeaveChannelMessage = CGameServerMessage::GameServerMessageAlloc();
	*LeaveChannelMessage << &LeaveChannelObject;	

	LeaveChannelJob->GameObjectJobMessage = LeaveChannelMessage;

	return LeaveChannelJob;
}

st_GameObjectJob* CGameServer::MakeGameObjectJobLeaveChannelPlayer(CGameObject* LeavePlayerObject, int32* PlayerIndexes)
{
	st_GameObjectJob* LeaveChannelJob = G_ObjectManager->GameObjectJobCreate();
	LeaveChannelJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_PLAYER_LEAVE_CHANNEL;

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

CGameServerMessage* CGameServer::MakePacketResClientConnected()
{
	CGameServerMessage* ClientConnetedMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ClientConnetedMessage == nullptr)
	{
		CRASH("ClientConnectdMessage�� nullptr");
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

CGameServerMessage* CGameServer::MakePacketResMousePositionObjectInfo(int64 AccountId, int64 PreviousChoiceObjectId, int64 FindObjectId,
	map<en_SkillType, CSkill*> BufSkillInfo, map<en_SkillType, CSkill*> DeBufSkillInfo)
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

CGameServerMessage* CGameServer::MakePacketResMoneyToInventory(int64 TargetObjectID, int64 GoldCoinCount, int16 SliverCoinCount, int16 BronzeCoinCount, en_SmallItemCategory ItemCategory, int16 ItemEach)
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
	*ResMoneyToInventoryMessage << (int16)ItemCategory;
	*ResMoneyToInventoryMessage << ItemEach;
	*ResMoneyToInventoryMessage << true;

	return ResMoneyToInventoryMessage;
}

CGameServerMessage* CGameServer::MakePacketResItemToInventory(int64 TargetObjectId, CItem* InventoryItem, bool IsExist, int16 ItemEach, bool ItemGainPrint)
{
	CGameServerMessage* ResItemToInventoryMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResItemToInventoryMessage == nullptr)
	{
		return nullptr;
	}

	ResItemToInventoryMessage->Clear();

	*ResItemToInventoryMessage << (int16)en_PACKET_S2C_LOOTING;
	// Ÿ�� ���̵�
	*ResItemToInventoryMessage << TargetObjectId;
	
	*ResItemToInventoryMessage << false;
	// �κ��丮 ������ ���� ���
	*ResItemToInventoryMessage << InventoryItem;
	// ������ �ߺ� ����
	*ResItemToInventoryMessage << IsExist;
	// ������ ���� ����
	*ResItemToInventoryMessage << ItemEach;	
	// ������ ��� UI ��� �� ������ ��������
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

		// ī�װ� �̸� 
		int16 CraftingItemCategoryNameLen = (int16)(CraftingItemCategory.CategoryName.length() * 2);
		*ResCraftingListMessage << CraftingItemCategoryNameLen;
		ResCraftingListMessage->InsertData(CraftingItemCategory.CategoryName.c_str(), CraftingItemCategoryNameLen);

		*ResCraftingListMessage << (int8)CraftingItemCategory.CompleteItems.size();

		for (st_CraftingCompleteItem CraftingCompleteItem : CraftingItemCategory.CompleteItems)
		{
			*ResCraftingListMessage << (int16)CraftingCompleteItem.CompleteItemType;

			// ���� �ϼ��� �̸�
			int16 CraftingCompleteItemNameLen = (int16)(CraftingCompleteItem.CompleteItemName.length() * 2);
			*ResCraftingListMessage << CraftingCompleteItemNameLen;
			ResCraftingListMessage->InsertData(CraftingCompleteItem.CompleteItemName.c_str(), CraftingCompleteItemNameLen);

			// ���� �ϼ��� �̹��� ���
			int16 CraftingCompleteItemImagePathLen = (int16)(CraftingCompleteItem.CompleteItemImagePath.length() * 2);
			*ResCraftingListMessage << CraftingCompleteItemImagePathLen;
			ResCraftingListMessage->InsertData(CraftingCompleteItem.CompleteItemImagePath.c_str(), CraftingCompleteItemImagePathLen);

			*ResCraftingListMessage << (int8)CraftingCompleteItem.Materials.size();

			for (st_CraftingMaterialItemInfo CraftingMaterialItem : CraftingCompleteItem.Materials)
			{
				*ResCraftingListMessage << AccountId;
				*ResCraftingListMessage << PlayerId;
				*ResCraftingListMessage << (int16)CraftingMaterialItem.MaterialItemType;

				// ����� �̸�
				int16 CraftingMaterialItemNameLen = (int16)(CraftingMaterialItem.MaterialItemName.length() * 2);
				*ResCraftingListMessage << CraftingMaterialItemNameLen;
				ResCraftingListMessage->InsertData(CraftingMaterialItem.MaterialItemName.c_str(), CraftingMaterialItemNameLen);

				*ResCraftingListMessage << CraftingMaterialItem.ItemCount;

				// ����� �̹��� ���
				int16 MaterialImagePathLen = (int16)(CraftingMaterialItem.MaterialItemImagePath.length() * 2);
				*ResCraftingListMessage << MaterialImagePathLen;
				ResCraftingListMessage->InsertData(CraftingMaterialItem.MaterialItemImagePath.c_str(), MaterialImagePathLen);
			}
		}
	}

	return ResCraftingListMessage;
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
	CWeapon* NewWeaponItem = nullptr;
	CArmor* NewArmorItem = nullptr;
	CMaterial* NewMaterialItem = nullptr;

	switch (NewItemInfo.ItemSmallCategory)
	{
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
		NewWeaponItem = (CWeapon*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_WEAPON);
		NewWeaponItem->_ItemInfo = NewItemInfo;

		return NewWeaponItem;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
		NewArmorItem = (CArmor*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_ARMOR);
		NewArmorItem->_ItemInfo = NewItemInfo;

		return NewArmorItem;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL:
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
		NewMaterialItem = (CMaterial*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_MATERIAL);
		NewMaterialItem->_ItemInfo = NewItemInfo;

		return NewMaterialItem;
	}
}

void CGameServer::ExperienceCalculate(CPlayer* Player, CGameObject* Target)
{
	CMonster* TargetMonster = nullptr;

	switch (Target->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_SLIME:
		TargetMonster = (CMonster*)Target;

		Player->_Experience.CurrentExperience += TargetMonster->_GetExpPoint;
		Player->_Experience.CurrentExpRatio = ((float)Player->_Experience.CurrentExperience) / Player->_Experience.RequireExperience;

		if (Player->_Experience.CurrentExpRatio >= 1.0f)
		{
			// ���� ����
			Player->_GameObjectInfo.ObjectStatInfo.Level += 1;

			// ������ ������ �ش��ϴ� �ɷ�ġ ������ �о�� �� �����Ѵ�.
			st_ObjectStatusData NewCharacterStatus;
			st_LevelData LevelData;

			switch (Player->_GameObjectInfo.ObjectType)
			{
			case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
			{
				auto FindStatus = G_Datamanager->_WarriorStatus.find(Player->_GameObjectInfo.ObjectStatInfo.Level);
				if (FindStatus == G_Datamanager->_WarriorStatus.end())
				{
					CRASH("���� �������ͽ� ã�� ����");
				}

				NewCharacterStatus = *(*FindStatus).second;

				Player->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
				Player->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
				Player->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
				Player->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
				Player->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
				Player->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
				Player->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
				Player->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
				Player->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
				Player->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
				Player->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
				Player->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
			}
			break;
			case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
			{
				auto FindStatus = G_Datamanager->_ShamanStatus.find(Player->_GameObjectInfo.ObjectStatInfo.Level);
				if (FindStatus == G_Datamanager->_WarriorStatus.end())
				{
					CRASH("���� ������ ã�� ����");
				}

				NewCharacterStatus = *(*FindStatus).second;

				Player->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
				Player->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
				Player->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
				Player->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
				Player->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
				Player->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
				Player->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
				Player->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
				Player->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
				Player->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
				Player->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
				Player->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
			}
			break;
			case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
			{
				auto FindStatus = G_Datamanager->_TaioistStatus.find(Player->_GameObjectInfo.ObjectStatInfo.Level);
				if (FindStatus == G_Datamanager->_TaioistStatus.end())
				{
					CRASH("���� ������ ã�� ����");
				}

				NewCharacterStatus = *(*FindStatus).second;

				Player->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
				Player->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
				Player->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
				Player->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
				Player->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
				Player->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
				Player->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
				Player->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
				Player->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
				Player->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
				Player->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
				Player->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
			}
			break;
			case en_GameObjectType::OBJECT_THIEF_PLAYER:
			{
				auto FindStatus = G_Datamanager->_ThiefStatus.find(Player->_GameObjectInfo.ObjectStatInfo.Level);
				if (FindStatus == G_Datamanager->_ThiefStatus.end())
				{
					CRASH("���� ������ ã�� ����");
				}

				NewCharacterStatus = *(*FindStatus).second;

				Player->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
				Player->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
				Player->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
				Player->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
				Player->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
				Player->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
				Player->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
				Player->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
				Player->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
				Player->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
				Player->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
				Player->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
			}
			break;
			case en_GameObjectType::OBJECT_ARCHER_PLAYER:
			{
				auto FindStatus = G_Datamanager->_ArcherStatus.find(Player->_GameObjectInfo.ObjectStatInfo.Level);
				if (FindStatus == G_Datamanager->_ArcherStatus.end())
				{
					CRASH("���� ������ ã�� ����");
				}

				NewCharacterStatus = *(*FindStatus).second;

				Player->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
				Player->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
				Player->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
				Player->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
				Player->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
				Player->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
				Player->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
				Player->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
				Player->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
				Player->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
				Player->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
				Player->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
				Player->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
				Player->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
			}
			break;
			}

			CGameServerMessage* ResObjectStatChangeMessage = MakePacketResChangeObjectStat(Player->_GameObjectInfo.ObjectId, Player->_GameObjectInfo.ObjectStatInfo);
			SendPacket(Player->_SessionId, ResObjectStatChangeMessage);
			ResObjectStatChangeMessage->Free();

			auto FindLevelData = G_Datamanager->_LevelDatas.find(Player->_GameObjectInfo.ObjectStatInfo.Level);
			if (FindLevelData == G_Datamanager->_LevelDatas.end())
			{
				CRASH("���� ������ ã�� ����");
			}

			LevelData = *(*FindLevelData).second;

			Player->_Experience.CurrentExperience = 0;
			Player->_Experience.RequireExperience = LevelData.RequireExperience;
			Player->_Experience.TotalExperience = LevelData.TotalExperience;
		}

		{
			CGameServerMessage* ResMonsterGetExpMessage = MakePacketExperience(Player->_AccountId, Player->_GameObjectInfo.ObjectId, TargetMonster->_GetExpPoint,
				Player->_Experience.CurrentExperience,
				Player->_Experience.RequireExperience,
				Player->_Experience.TotalExperience);
			SendPacket(Player->_SessionId, ResMonsterGetExpMessage);
			ResMonsterGetExpMessage->Free();
		}
		break;
	case en_GameObjectType::OBJECT_TREE:
		break;
	case en_GameObjectType::OBJECT_STONE:
		break;
	}
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

CGameServerMessage* CGameServer::MakePacketResMove(int64 AccountId, int64 ObjectId, bool CanMove, st_PositionInfo PositionInfo)
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

	// Spawn ������Ʈ ����
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
		wsprintf(ErrorMessage, L"[%s]�� ���� ���ð��� �Ϸ���� �ʾҽ��ϴ�.", SkillName);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT:
		wsprintf(ErrorMessage, L"����� �����ϰ� [%s]��/�� ����ؾ� �մϴ�.", SkillName);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_HEAL_NON_SELECT_OBJECT:
		wsprintf(ErrorMessage, L"[%s] ����� �������� �ʾƼ� �ڽſ��� ����մϴ�.", SkillName);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_PLACE_BLOCK:
		wsprintf(ErrorMessage, L"�̵��� ��ġ�� ���� �־ [%s]��/�� ����� �� �����ϴ�.", SkillName);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_PLACE_DISTANCE:
		wsprintf(ErrorMessage, L"[%s] ������ �Ÿ��� �ʹ� �ٴϴ�. [�Ÿ� : %d ]", SkillName, SkillDistance);
		break;
	case en_PersonalMessageType::PERSONAL_MESSAGE_MYSELF_TARGET:
		wsprintf(ErrorMessage, L"[%s]��/�� �ڽſ��� ��� �� �� �����ϴ�.", SkillName, SkillDistance);
		break;
	}

	wstring ErrorMessageString = ErrorMessage;

	// ���� �޼���
	int16 ErrorMessageLen = (int16)(ErrorMessageString.length() * 2);
	*ResErrorMessage << ErrorMessageLen;
	ResErrorMessage->InsertData(ErrorMessageString.c_str(), ErrorMessageLen);

	return ResErrorMessage;
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
	// ���� ������
	vector<CSector*> AroundSectors = Object->GetChannel()->GetMap()->GetAroundSectors(Object->GetCellPosition(), 1);

	for (CSector* AroundSector : AroundSectors)
	{
		for (CPlayer* Player : AroundSector->GetPlayers())
		{
			int16 Distance = st_Vector2Int::Distance(Object->GetCellPosition(), Player->GetCellPosition());

			if (Distance <= Object->_FieldOfViewDistance)
			{
				SendPacket(Player->_SessionId, Message);
			}
		}
	}
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