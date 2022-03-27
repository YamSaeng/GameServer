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
	_TimerThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	_NetworkThreadEnd = false;
	_UserDataBaseThreadEnd = false;
	_WorldDataBaseThreadEnd = false;	
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

	// �� ������Ʈ ����
	G_ObjectManager->MapObjectSpawn(1);

	G_ObjectManager->GameServer = this;

	// ���� ������ ���̽� ������ ����
	for (int32 i = 0; i < 3; i++)
	{
		_UserDataBaseThread = (HANDLE)_beginthreadex(NULL, 0, UserDataBaseThreadProc, this, 0, NULL);		
		CloseHandle(_UserDataBaseThread);
	}

	// ���� ������ ���̽� ������ ����
	_WorldDataBaseThread = (HANDLE)_beginthreadex(NULL, 0, WorldDataBaseThreadProc, this, 0, NULL);
	
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

		while (!Instance->_GameServerUserDataBaseThreadMessageQue.IsEmpty())
		{
			st_GameServerJob* Job = nullptr;

			if (!Instance->_GameServerUserDataBaseThreadMessageQue.Dequeue(&Job))
			{
				break;
			}

			st_Session* Session = Instance->FindSession(Job->SessionId);

			if (Session && !Session->DBQue.IsEmpty())
			{										
				do
				{
					if (InterlockedExchange64(&Session->IsDBExecute, 1) == 0)
					{
						while (!Session->DBQue.IsEmpty())
						{
							CGameServerMessage* DBMessage = nullptr;

							if (!Session->DBQue.Dequeue(&DBMessage))
							{
								break;
							}

							int16 DBMessageType;
							*DBMessage >> DBMessageType;

							switch ((en_GameServerJobType)DBMessageType)
							{
							case en_GameServerJobType::DATA_BASE_ACCOUNT_CHECK:
								Instance->PacketProcReqDBAccountCheck(Session->SessionId, DBMessage);
								break;
							case en_GameServerJobType::DATA_BASE_CHARACTER_CHECK:
								Instance->PacketProcReqDBCreateCharacterNameCheck(Session->SessionId, DBMessage);
								break;							
							case en_GameServerJobType::DATA_BASE_LOOTING_ITEM_INVENTORY_SAVE:
								Instance->PacketProcReqDBLootingItemToInventorySave(Session->SessionId, DBMessage);
								break;
							case en_GameServerJobType::DATA_BASE_CRAFTING_ITEM_INVENTORY_SAVE:
								Instance->PacketProcReqDBCraftingItemToInventorySave(Session->SessionId, DBMessage);
								break;
							case en_GameServerJobType::DATA_BASE_PLACE_ITEM:
								Instance->PacketProcReqDBItemPlace(Session->SessionId, DBMessage);
								break;						
							case en_GameServerJobType::DATA_BASE_ITEM_USE:
								Instance->PacketProcReqDBItemUpdate(Session->SessionId, DBMessage);
								break;
							case en_GameServerJobType::DATA_BASE_GOLD_SAVE:
								Instance->PacketProcReqDBGoldSave(Session->SessionId, DBMessage);
								break;
							case en_GameServerJobType::DATA_BASE_CHARACTER_INFO_SEND:
								Instance->PacketProcReqDBCharacterInfoSend(Session->SessionId, DBMessage);
								break;							
							case en_GameServerJobType::DATA_BASE_QUICK_SLOT_SAVE:
								Instance->PacketProcReqDBQuickSlotBarSlotSave(Session->SessionId, DBMessage);
								break;
							case en_GameServerJobType::DATA_BASE_QUICK_SWAP:
								Instance->PacketProcReqDBQuickSlotSwap(Session->SessionId, DBMessage);
								break;
							case en_GameServerJobType::DATA_BASE_QUICK_INIT:
								Instance->PacketProcReqDBQuickSlotInit(Session->SessionId, DBMessage);
								break;							
							default:
								Instance->Disconnect(Session->SessionId);
								break;
							}

							DBMessage->Free();

							InterlockedDecrement64(&Session->DBReserveCount);
							InterlockedIncrement64(&Instance->_DataBaseThreadTPS);
						}					

						InterlockedExchange64(&Session->IsDBExecute, 0);
					}
				} while (0);

				Instance->ReturnSession(Session);
			}	

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

		while (!Instance->_GameServerWorldDataBaseThreadMessageQue.IsEmpty())
		{
			st_GameServerJob* Job = nullptr;

			if (!Instance->_GameServerWorldDataBaseThreadMessageQue.Dequeue(&Job))
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
					case en_TimerJobType::TIMER_MELEE_ATTACK_END:
						Instance->PacketProcTimerAttackEnd(TimerJob->SessionId, TimerJob->TimerJobMessage);
						break;
					case en_TimerJobType::TIMER_SPELL_END:
						Instance->PacketProcTimerSpellEnd(TimerJob->SessionId, TimerJob->TimerJobMessage);
						break;					
					case en_TimerJobType::TIMER_OBJECT_SPAWN:
						Instance->PacketProcTimerObjectSpawn(TimerJob->TimerJobMessage);
						break;
					case en_TimerJobType::TIMER_OBJECT_STATE_CHANGE:
						Instance->PacketProcTimerObjectStateChange(TimerJob->SessionId, TimerJob->TimerJobMessage);
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
					
			G_ChannelManager->Update();

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

		Session->MyPlayerIndex = -1;

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
	if (Session == nullptr)
	{
		return;
	}	

	if (!Session->IsDummy)
	{
		// �α��� ������ �α׾ƿ� ��û
		CGameServerMessage* LogOutPacket = MakePacketLogOut(Session->AccountId);
		SendPacketToLoginServer(LogOutPacket);
		LogOutPacket->Free();
	}	

	CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

	if (MyPlayer != nullptr)
	{
		// DB�� ���� �����ϴ� �÷��̾� ������ �����Ѵ�.		
		//PacketProcReqDBLeavePlayerInfoSave(MyPlayer);

		while (1)
		{
			if (Session->IsDBExecute == 0 && Session->DBReserveCount == 0)
			{
				// DBQue�� �����ϰ� �����ִ� DB���� �޼������� ��� �̾Ƽ� ��ȯ�Ѵ�.
				while (Session->DBQue.GetUseSize() > 0)
				{
					CGameServerMessage* ReqDBMessage = nullptr;
					if (!Session->DBQue.Dequeue(&ReqDBMessage))
					{
						break;
					}

					ReqDBMessage->Free();
				}

				break;
			}
		}			

		vector<CGameObject*> DeSpawnObject;

		for (int i = 0; i < SESSION_CHARACTER_MAX; i++)
		{
			CPlayer* SessionPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndexes[i]];
			if (SessionPlayer != nullptr 
				&& Session->SessionId == SessionPlayer->_SessionId
				&& SessionPlayer->_NetworkState == en_ObjectNetworkState::LIVE)
			{
				DeSpawnObject.push_back(SessionPlayer);

				SessionPlayer->_NetworkState = en_ObjectNetworkState::LEAVE;
				SessionPlayer->_FieldOfViewInfos.clear();
				// ������
				SessionPlayer->Init();

				// ���� �þ� ���� Ŭ��鿡�� ��ȯ���� ��Ŷ ����
				CMessage* ResDeSpawnObject = MakePacketResObjectDeSpawn(1, DeSpawnObject);
				SendPacketFieldOfView(SessionPlayer, ResDeSpawnObject);
				ResDeSpawnObject->Free();

				// ä�� ���� 
				CChannel* Channel = G_ChannelManager->Find(1);
				Channel->LeaveChannel(MyPlayer);

				DeSpawnObject.clear();
			}

			// �ε��� �ݳ�
			G_ObjectManager->PlayerIndexReturn(Session->MyPlayerIndexes[i]);
		}				
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

			int16 AccountNameLen;
			*Message >> AccountNameLen;
			
			WCHAR* AccountName = (WCHAR*)malloc(sizeof(WCHAR) * AccountNameLen);
			memset(AccountName, 0, sizeof(WCHAR) * AccountNameLen);			
			Message->GetData(AccountName, AccountNameLen);

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
			DBAccountCheckJob->SessionId = Session->SessionId;

			CGameServerMessage* DBAccountCheckMessage = CGameServerMessage::GameServerMessageAlloc();
			DBAccountCheckMessage->Clear();

			*DBAccountCheckMessage << (int16)en_GameServerJobType::DATA_BASE_ACCOUNT_CHECK;

			InterlockedIncrement64(&Session->DBReserveCount);
			Session->DBQue.Enqueue(DBAccountCheckMessage);

			_GameServerUserDataBaseThreadMessageQue.Enqueue(DBAccountCheckJob);
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
		DBCharacterCreateJob->SessionId = Session->SessionId;

		CGameServerMessage* DBReqChatacerCreateMessage = CGameServerMessage::GameServerMessageAlloc();
		DBReqChatacerCreateMessage->Clear();

		*DBReqChatacerCreateMessage << (int16)en_GameServerJobType::DATA_BASE_CHARACTER_CHECK;
		*DBReqChatacerCreateMessage << ReqGameObjectType;
		*DBReqChatacerCreateMessage << CharacterCreateSlotIndex;

		InterlockedIncrement64(&Session->DBReserveCount);
		Session->DBQue.Enqueue(DBReqChatacerCreateMessage);		

		_GameServerUserDataBaseThreadMessageQue.Enqueue(DBCharacterCreateJob);
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
					G_ObjectManager->_PlayersArray[Session->MyPlayerIndex]->_NetworkState = en_ObjectNetworkState::LIVE;
					break;
				}
			}

			if (FindName == false)
			{	
				CMessage* ResEnterGamePacket = MakePacketResEnterGame(FindName, nullptr);
				SendPacket(Session->SessionId, ResEnterGamePacket);
				ResEnterGamePacket->Free();
				break;
			}
			else
			{
				CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

				// Ŭ�� ������ ������Ʈ�� ���ӿ� ���� ��Ų��. ( ä��, ���� ���� )
				G_ObjectManager->ObjectEnterGame(MyPlayer, 1);

				// ������ �� �����϶�� �˷���
				CMessage* ResEnterGamePacket = MakePacketResEnterGame(FindName, &MyPlayer->_GameObjectInfo);
				SendPacket(Session->SessionId, ResEnterGamePacket);
				ResEnterGamePacket->Free();

				// ĳ������ ���¸� 10�� �Ŀ� IDLE ���·� ��ȯ
				//ObjectStateChangeTimerJobCreate(MyPlayer, en_CreatureState::IDLE, 10000);
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
		DBCharacterInfoSendJob->SessionId = Session->SessionId;

		CGameServerMessage* DBCharacterInfoSendMessage = CGameServerMessage::GameServerMessageAlloc();
		DBCharacterInfoSendMessage->Clear();

		*DBCharacterInfoSendMessage << (int16)en_GameServerJobType::DATA_BASE_CHARACTER_INFO_SEND;

		InterlockedIncrement64(&Session->DBReserveCount);
		Session->DBQue.Enqueue(DBCharacterInfoSendMessage);

		_GameServerUserDataBaseThreadMessageQue.Enqueue(DBCharacterInfoSendJob);
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
			
			int8 StatusAbnormalCount = MyPlayer->CheckStatusAbnormal();

			if (StatusAbnormalCount > 0)
			{
				CMessage* ErrorMessage = MakePacketStatusAbnormalMessage(en_CommonErrorType::ERROR_STATUS_ABNORMAL_MOVE, StatusAbnormalCount, MyPlayer->_StatusAbnormal);
				SendPacket(Session->SessionId, ErrorMessage);
				ErrorMessage->Free();

				CMessage* ResSyncPositionPacket = MakePacketResSyncPosition(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
				SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResSyncPositionPacket, MyPlayer);
				ResSyncPositionPacket->Free();
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

			CMessage* ResMyMoveOtherPacket = MakePacketResMove(Session->AccountId,
				MyPlayer->_GameObjectInfo.ObjectId,
				true,
				MyPlayer->_GameObjectInfo.ObjectPositionInfo);
			SendPacketFieldOfView(Session, ResMyMoveOtherPacket, true);
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

			CMessage* ResMoveStopPacket = MakePacketResMoveStop(Session->AccountId,
				MyPlayer->_GameObjectInfo.ObjectId,
				MyPlayer->_GameObjectInfo.ObjectPositionInfo);
			SendPacketFieldOfView(Session, ResMoveStopPacket, true);
			ResMoveStopPacket->Free();
		} while (0);

		ReturnSession(Session);
	}	
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

			int8 StatusAbnormalCount = MyPlayer->CheckStatusAbnormal();

			if (StatusAbnormalCount > 0)
			{
				CMessage* ErrorMessage = MakePacketStatusAbnormalMessage(en_CommonErrorType::ERROR_STATUS_ABNORMAL_MELEE, StatusAbnormalCount, MyPlayer->_StatusAbnormal);
				SendPacket(Session->SessionId, ErrorMessage);
				ErrorMessage->Free();

				CMessage* ResSyncPositionPacket = MakePacketResSyncPosition(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
				SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResSyncPositionPacket, MyPlayer);
				ResSyncPositionPacket->Free();
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

			MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir = (en_MoveDir)ReqMoveDir;
		
			vector<CGameObject*> Targets;

			CSkill* ReqMeleeSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)ReqSkillType);
			if (ReqMeleeSkill != nullptr && ReqMeleeSkill->GetSkillInfo()->CanSkillUse == true)
			{				
				MyPlayer->_SkillType = (en_SkillType)ReqSkillType;

				MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;

				CMessage* ResObjectStateChangePacket = MakePacketResChangeObjectState(MyPlayer->_GameObjectInfo.ObjectId,
					MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir,
					MyPlayer->_GameObjectInfo.ObjectType,
					MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
				SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResObjectStateChangePacket, MyPlayer);
				ResObjectStateChangePacket->Free();
				
				// Ÿ�� ��ġ Ȯ��
				switch (ReqMeleeSkill->GetSkillInfo()->SkillType)
				{
				case en_SkillType::SKILL_DEFAULT_ATTACK:
					{
						st_Vector2Int FrontCell = MyPlayer->GetFrontCellPosition(MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);
						CGameObject* Target = MyPlayer->_Channel->_Map->Find(FrontCell);
						if (Target != nullptr)
						{
							Targets.push_back(Target);
						}
					}					
					break;			
				case en_SkillType::SKILL_KNIGHT_CHOHONE:
					{
						if (MyPlayer->_SelectTarget != nullptr)
						{
							st_Vector2Int TargetPosition = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId,
								MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType)->GetCellPosition();
							st_Vector2Int MyPosition = MyPlayer->GetCellPosition();
							st_Vector2Int Direction = TargetPosition - MyPosition;

							int32 Distance = st_Vector2Int::Distance(TargetPosition, MyPosition);

							if (Distance <= 4)
							{
								CGameObject* Target = MyPlayer->_Channel->_Map->Find(TargetPosition);
								if (Target != nullptr)
								{
									st_Vector2Int MyFrontCellPotision = MyPlayer->GetFrontCellPosition(MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);

									if (MyPlayer->_Channel->_Map->ApplyMove(Target, MyFrontCellPotision))
									{										
										CSkill* NewSkill = G_ObjectManager->SkillCreate();

										st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(ReqMeleeSkill->GetSkillInfo()->SkillMediumCategory);

										*NewAttackSkillInfo = *((st_AttackSkillInfo*)ReqMeleeSkill->GetSkillInfo());

										NewSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
										NewSkill->StatusAbnormalDurationTimeStart();

										Target->AddDebuf(NewSkill);
										Target->SetStatusAbnormal(STATUS_ABNORMAL_WARRIOR_CHOHONE);

										CMessage* ResStatusAbnormalPacket = MakePacketStatusAbnormal(Target->_GameObjectInfo.ObjectId,
											Target->_GameObjectInfo.ObjectType,
											Target->_GameObjectInfo.ObjectPositionInfo.MoveDir,
											ReqMeleeSkill->GetSkillInfo()->SkillType,											
											true, STATUS_ABNORMAL_WARRIOR_CHOHONE);
										SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResStatusAbnormalPacket, MyPlayer);
										ResStatusAbnormalPacket->Free();

										CMessage* ResBufDeBufSkillPacket = MakePacketBufDeBuf(Target->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
										SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResBufDeBufSkillPacket, MyPlayer);
										ResBufDeBufSkillPacket->Free();	
									
										float EffectPrintTime = ReqMeleeSkill->GetSkillInfo()->SkillDurationTime / 1000.0f;
										
										// ����Ʈ ���
										CMessage* ResEffectPacket = MakePacketEffect(Target->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_DEBUF_STUN, EffectPrintTime);
										SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResEffectPacket, MyPlayer);
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
										SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResSyncPositionPacket, MyPlayer);
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
							st_Vector2Int TargetPosition = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType)->GetCellPosition();
							st_Vector2Int MyPosition = MyPlayer->GetCellPosition();
							st_Vector2Int Direction = TargetPosition - MyPosition;

							en_MoveDir Dir = st_Vector2Int::GetMoveDir(Direction);
							int32 Distance = st_Vector2Int::Distance(TargetPosition, MyPosition);

							if (Distance <= 4)
							{
								CGameObject* Target = MyPlayer->_Channel->_Map->Find(TargetPosition);
								st_Vector2Int MovePosition;
								if (Target != nullptr)
								{
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

									if (MyPlayer->_Channel->_Map->ApplyMove(MyPlayer, MovePosition))
									{
										CSkill* NewSkill = G_ObjectManager->SkillCreate();		

										st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(ReqMeleeSkill->GetSkillInfo()->SkillMediumCategory);

										*AttackSkillInfo = *((st_AttackSkillInfo*)ReqMeleeSkill->GetSkillInfo());

										NewSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, AttackSkillInfo);
										NewSkill->StatusAbnormalDurationTimeStart();

										Target->AddDebuf(NewSkill);
										Target->SetStatusAbnormal(STATUS_ABNORMAL_WARRIOR_SHAEHONE);

										CMessage* ResStatusAbnormalPacket = MakePacketStatusAbnormal(Target->_GameObjectInfo.ObjectId,
											Target->_GameObjectInfo.ObjectType,
											Target->_GameObjectInfo.ObjectPositionInfo.MoveDir,
											ReqMeleeSkill->GetSkillInfo()->SkillType,
											true, STATUS_ABNORMAL_WARRIOR_SHAEHONE);
										SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResStatusAbnormalPacket, MyPlayer);
										ResStatusAbnormalPacket->Free();

										CMessage* ResBufDeBufSkillPacket = MakePacketBufDeBuf(Target->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
										SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResBufDeBufSkillPacket, MyPlayer);
										ResBufDeBufSkillPacket->Free();

										float EffectPrintTime = ReqMeleeSkill->GetSkillInfo()->SkillDurationTime / 1000.0f;

										// ����Ʈ ���
										CMessage* ResEffectPacket = MakePacketEffect(Target->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_DEBUF_ROOT, EffectPrintTime);
										SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResEffectPacket, MyPlayer);
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
										SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResSyncPositionPacket, MyPlayer);
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

						// ����Ʈ ���
						CMessage* ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_SMASH_WAVE, 2.0f);
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResEffectPacket, MyPlayer);
						ResEffectPacket->Free();
					}
					break;
				}
				
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

						en_EffectType HitEffectType;

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
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResSkillSystemMessagePacket, MyPlayer);
						ResSkillSystemMessagePacket->Free();

						// ���� ���� �޼��� ����
						CMessage* ResMyAttackOtherPacket = MakePacketResAttack(MyPlayer->_GameObjectInfo.ObjectId, 
							Target->_GameObjectInfo.ObjectId, 
							(en_SkillType)ReqSkillType,
							FinalDamage, 
							IsCritical);
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResMyAttackOtherPacket, MyPlayer);
						ResMyAttackOtherPacket->Free();

						// ����Ʈ ���
						CMessage* ResEffectPacket = MakePacketEffect(Target->_GameObjectInfo.ObjectId,
							HitEffectType, 
							AttackSkillInfo->SkillTargetEffectTime);
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResEffectPacket, MyPlayer);
						ResEffectPacket->Free();

						// ���� ���� �޼��� ����
						CMessage* ResChangeObjectStat = MakePacketResChangeObjectStat(Target->_GameObjectInfo.ObjectId,							
							Target->_GameObjectInfo.ObjectStatInfo);
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResChangeObjectStat, MyPlayer);
						ResChangeObjectStat->Free();
					}
				}
				
				ReqMeleeSkill->CoolTimeStart();				

				// Ŭ�󿡰� ��Ÿ�� ǥ��
				CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(QuickSlotBarIndex,
					QuickSlotBarSlotIndex,
					1.0f, ReqMeleeSkill);					
				SendPacket(Session->SessionId, ResCoolTimeStartPacket);
				ResCoolTimeStartPacket->Free();

				SkillMotionEndTimerJobCreate(MyPlayer, 500, en_TimerJobType::TIMER_MELEE_ATTACK_END);				
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

			CGameObject* FindGameObject = nullptr;
			float SpellCastingTime = 0.0f;

			CMessage* ResEffectPacket = nullptr;
			CMessage* ResMagicPacket = nullptr;

			vector<CGameObject*> Targets;

			CSkill* ReqMagicSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)ReqSkillType);
			if (ReqMagicSkill != nullptr && ReqMagicSkill->GetSkillInfo()->CanSkillUse)
			{
				switch (ReqMagicSkill->GetSkillInfo()->SkillType)
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
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResBufDeBufSkillPacket, MyPlayer);
						ResBufDeBufSkillPacket->Free();						

						ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_CHARGE_POSE, 2.8f);
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResEffectPacket, MyPlayer);
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

						MyPlayer->_Channel->_Map->ApplyMove(MyPlayer, MovePosition);

						MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX + 0.5f;
						MyPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = MyPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY + 0.5f;

						CMessage* ResSyncPositionPacket = MakePacketResSyncPosition(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo);
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResSyncPositionPacket, MyPlayer);
						ResSyncPositionPacket->Free();						

						ResEffectPacket = MakePacketEffect(MyPlayer->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_BACK_TELEPORT, 0.5f);
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResEffectPacket, MyPlayer);
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
						SpellCastingTime = ReqMagicSkill->GetSkillInfo()->SkillCastingTime / 1000.0f;

						int16 Distance = st_Vector2Int::Distance(MyPlayer->_SelectTarget->GetCellPosition(), MyPlayer->GetCellPosition());

						if (Distance <= 6)
						{
							FindGameObject = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId,
								MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType);
							if (FindGameObject != nullptr)
							{
								Targets.push_back(FindGameObject);
							}

							// ����â ����
							ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId, true, ReqMagicSkill->GetSkillInfo()->SkillType, SpellCastingTime);
							SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResMagicPacket, MyPlayer);
							ResMagicPacket->Free();

							MyPlayer->_SkillType = ReqMagicSkill->GetSkillInfo()->SkillType;
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
						CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT, ReqMagicSkill->GetSkillInfo()->SkillName.c_str());
						SendPacket(MyPlayer->_SessionId, ResErrorPacket);
						ResErrorPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SHAMAN_ICE_WAVE:
					if (MyPlayer->_SelectTarget != nullptr)
					{
						st_Vector2 MyPosition = MyPlayer->GetPosition();
						st_Vector2 SelectTargetPosition = MyPlayer->_SelectTarget->GetPosition();
						st_Vector2 DirVector = SelectTargetPosition - MyPosition;
						st_Vector2 NormalVector = DirVector.Normalize(DirVector);
						
						st_Vector2Int IceWavePosition = MyPlayer->_SelectTarget->GetFrontCellPosition(st_Vector2::GetMoveDir(NormalVector), 2);

					}
					else
					{
						CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT, ReqMagicSkill->GetSkillInfo()->SkillName.c_str());
						SendPacket(MyPlayer->_SessionId, ResErrorPacket);
						ResErrorPacket->Free();
					}
					break;
				case en_SkillType::SKILL_TAIOIST_HEALING_LIGHT:
					SpellCastingTime = ReqMagicSkill->GetSkillInfo()->SkillCastingTime / 1000.0f;

					if (MyPlayer->_SelectTarget != nullptr)
					{
						FindGameObject = G_ObjectManager->Find(MyPlayer->_SelectTarget->_GameObjectInfo.ObjectId, MyPlayer->_SelectTarget->_GameObjectInfo.ObjectType);
						if (FindGameObject != nullptr)
						{
							Targets.push_back(FindGameObject);
						}

						// ����â ����
						ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId,
							true, ReqMagicSkill->GetSkillInfo()->SkillType, SpellCastingTime);
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResMagicPacket, MyPlayer);
						ResMagicPacket->Free();

						MyPlayer->_SkillType = ReqMagicSkill->GetSkillInfo()->SkillType;
					}
					else
					{
						Targets.push_back(MyPlayer);

						CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_HEAL_NON_SELECT_OBJECT, ReqMagicSkill->GetSkillInfo()->SkillName.c_str());
						SendPacket(MyPlayer->_SessionId, ResErrorPacket);
						ResErrorPacket->Free();

						// ����â ����
						ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId,
							true, ReqMagicSkill->GetSkillInfo()->SkillType, SpellCastingTime);
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResMagicPacket, MyPlayer);
						ResMagicPacket->Free();

						MyPlayer->_SkillType = ReqMagicSkill->GetSkillInfo()->SkillType;
					}
					break;
				case en_SkillType::SKILL_TAIOIST_HEALING_WIND:
					break;
				case en_SkillType::SKILL_SHOCK_RELEASE:
					{					
						CSkill* NewBufSkill = G_ObjectManager->SkillCreate();

						st_BufSkillInfo* NewShockReleaseSkillInfo = (st_BufSkillInfo*)G_ObjectManager->SkillInfoCreate(ReqMagicSkill->GetSkillInfo()->SkillMediumCategory);
						
						*NewShockReleaseSkillInfo = *((st_BufSkillInfo*)ReqMagicSkill->GetSkillInfo());
						NewBufSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewShockReleaseSkillInfo);
						NewBufSkill->StatusAbnormalDurationTimeStart();

						MyPlayer->AddBuf(NewBufSkill);

						CMessage* ResBufDebufSkillPacket = MakePacketBufDeBuf(MyPlayer->_GameObjectInfo.ObjectId, true, NewBufSkill->GetSkillInfo());
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResBufDebufSkillPacket, MyPlayer);
						ResBufDebufSkillPacket->Free();

						ReqMagicSkill->CoolTimeStart();						

						// Ŭ�󿡰� ��Ÿ�� ǥ��
						CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(QuickSlotBarIndex,
							QuickSlotBarSlotIndex,
							1.0f, ReqMagicSkill);
						SendPacket(Session->SessionId, ResCoolTimeStartPacket);
						ResCoolTimeStartPacket->Free();

						CMessage* ResObjectStateChangePacket = MakePacketResChangeObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
						SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResObjectStateChangePacket, MyPlayer);
						ResObjectStateChangePacket->Free();						
					}
					
					break;
				}

				if (Targets.size() >= 1)
				{
					MyPlayer->_Owner = Targets[0];

					MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPELL;

					// ���� ��ų ��� ���
					CMessage* ResObjectStateChangePacket = MakePacketResChangeObjectState(MyPlayer->_GameObjectInfo.ObjectId,
						MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType,
						MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
					SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResObjectStateChangePacket, MyPlayer);
					ResObjectStateChangePacket->Free();

					SkillCoolTimeTimerJobCreate(MyPlayer, ReqMagicSkill->GetSkillInfo()->SkillCastingTime,
						ReqMagicSkill, en_TimerJobType::TIMER_SPELL_END,
						QuickSlotBarIndex, QuickSlotBarSlotIndex);
				}
			}
			else
			{
				if (ReqMagicSkill == nullptr)
				{
					break;
				}

			    CMessage* ResErrorPacket = MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_SKILL_COOLTIME,
					ReqMagicSkill->GetSkillInfo()->SkillName.c_str());
				SendPacket(MyPlayer->_SessionId, ResErrorPacket);
				ResErrorPacket->Free();
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

			if (MyPlayer->_SkillJob != nullptr)
			{
				MyPlayer->_SkillJob->TimerJobCancel = true;
				MyPlayer->_SkillJob = nullptr;

				CMessage* ResMagicCancelPacket = MakePacketMagicCancel(MyPlayer->_AccountId, MyPlayer->_GameObjectInfo.ObjectId);
				SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResMagicCancelPacket, MyPlayer);
				ResMagicCancelPacket->Free();
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

			CGameObject* FindObject = G_ObjectManager->Find(ObjectId, (en_GameObjectType)ObjectType);
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

		CMessage* ResChattingMessage = MakePacketResChattingBoxMessage(PlayerDBId, en_MessageType::CHATTING, st_Color::White(), ChattingMessage);
		SendPacketFieldOfView(Session, ResChattingMessage, true);
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

			st_GameServerJob* DBPlaceItemJob = _GameServerJobMemoryPool->Alloc();
			DBPlaceItemJob->SessionId = Session->SessionId;

			CGameServerMessage* DBPlaceItemMessage = CGameServerMessage::GameServerMessageAlloc();
			DBPlaceItemMessage->Clear();

			*DBPlaceItemMessage << (int16)en_GameServerJobType::DATA_BASE_PLACE_ITEM;
			DBPlaceItemMessage->InsertData(Message->GetFrontBufferPtr(), Message->GetUseBufferSize());
			Message->MoveReadPosition(Message->GetUseBufferSize());

			InterlockedIncrement64(&Session->DBReserveCount);
			Session->DBQue.Enqueue(DBPlaceItemMessage);

			_GameServerUserDataBaseThreadMessageQue.Enqueue(DBPlaceItemJob);
			SetEvent(_UserDataBaseWakeEvent);			
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
			
			st_GameServerJob* DBQuickSlotSaveJob = _GameServerJobMemoryPool->Alloc();
			DBQuickSlotSaveJob->SessionId = MyPlayer->_SessionId;

			CGameServerMessage* DBQuickSlotSaveMessage = CGameServerMessage::GameServerMessageAlloc();
			DBQuickSlotSaveMessage->Clear();

			*DBQuickSlotSaveMessage << (int16)en_GameServerJobType::DATA_BASE_QUICK_SLOT_SAVE;			
			DBQuickSlotSaveMessage->InsertData(Message->GetFrontBufferPtr(), Message->GetUseBufferSize());
			Message->MoveReadPosition(Message->GetUseBufferSize());

			InterlockedIncrement64(&Session->DBReserveCount);
			Session->DBQue.Enqueue(DBQuickSlotSaveMessage);

			_GameServerUserDataBaseThreadMessageQue.Enqueue(DBQuickSlotSaveJob);
			SetEvent(_UserDataBaseWakeEvent);
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
			
			st_GameServerJob* DBQuickSlotSwapJob = _GameServerJobMemoryPool->Alloc();			
			DBQuickSlotSwapJob->SessionId = MyPlayer->_SessionId;

			CGameServerMessage* DBQuickSlotSwapMessage = CGameServerMessage::GameServerMessageAlloc();
			DBQuickSlotSwapMessage->Clear();

			*DBQuickSlotSwapMessage << (int16)en_GameServerJobType::DATA_BASE_QUICK_SWAP;
			*DBQuickSlotSwapMessage << AccountId;
			*DBQuickSlotSwapMessage << PlayerDBId;
			DBQuickSlotSwapMessage->InsertData(Message->GetFrontBufferPtr(), Message->GetUseBufferSize());
			Message->MoveReadPosition(Message->GetUseBufferSize());

			InterlockedIncrement64(&Session->DBReserveCount);
			Session->DBQue.Enqueue(DBQuickSlotSwapMessage);

			_GameServerUserDataBaseThreadMessageQue.Enqueue(DBQuickSlotSwapJob);
			SetEvent(_UserDataBaseWakeEvent);
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

			st_GameServerJob* DBQuickSlotInitJob = _GameServerJobMemoryPool->Alloc();			
			DBQuickSlotInitJob->SessionId = MyPlayer->_SessionId;

			CGameServerMessage* DBQuickSlotInitMessage = CGameServerMessage::GameServerMessageAlloc();
			DBQuickSlotInitMessage->Clear();

			*DBQuickSlotInitMessage << (int16)en_GameServerJobType::DATA_BASE_QUICK_INIT;
			*DBQuickSlotInitMessage << AccountId;
			*DBQuickSlotInitMessage << PlayerDBId;
			DBQuickSlotInitMessage->InsertData(Message->GetFrontBufferPtr(), Message->GetUseBufferSize());
			Message->MoveReadPosition(Message->GetUseBufferSize());

			InterlockedIncrement64(&Session->DBReserveCount);
			Session->DBQue.Enqueue(DBQuickSlotInitMessage);

			_GameServerUserDataBaseThreadMessageQue.Enqueue(DBQuickSlotInitJob);
			SetEvent(_UserDataBaseWakeEvent);
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
			DBInventorySaveJob->SessionId = Session->SessionId;

			CGameServerMessage* DBCraftingItemSaveMessage = CGameServerMessage::GameServerMessageAlloc();
			if (DBCraftingItemSaveMessage == nullptr)
			{
				break;
			}

			DBCraftingItemSaveMessage->Clear();

			*DBCraftingItemSaveMessage << (int16)en_GameServerJobType::DATA_BASE_CRAFTING_ITEM_INVENTORY_SAVE;
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
			
			InterlockedIncrement64(&Session->DBReserveCount);
			Session->DBQue.Enqueue(DBCraftingItemSaveMessage);

			_GameServerUserDataBaseThreadMessageQue.Enqueue(DBInventorySaveJob);
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
			DBItemEquipJob->SessionId = MyPlayer->_SessionId;

			CGameServerMessage* DBItemEquipMessage = CGameServerMessage::GameServerMessageAlloc();
			DBItemEquipMessage->Clear();

			// ������ �����۰� ���������� �������� ������ ����
			//*DBItemEquipMessage << &UseItem;

			DBItemEquipJob->Message = DBItemEquipMessage;

			_GameServerUserDataBaseThreadMessageQue.Enqueue(DBItemEquipJob);
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

			// ���� ��ġ�� �����۵��� �����´�.
			CItem** Items = G_ChannelManager->Find(1)->_Map->FindItem(ItemCellPosition);
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

					CPlayer* TargetPlayer = (CPlayer*)(G_ObjectManager->Find(TargetObjectId, en_GameObjectType::OBJECT_PLAYER));
					if (TargetPlayer == nullptr)
					{
						break;
					}

					bool IsExistItem = false;
					// �κ��丮�� ������ ������ ���� ( �ʿ� �����Ǿ� �ִ� �������� ���� )
					int16 ItemEach = Items[i]->_ItemInfo.ItemCount;					

					// �������� �κ��丮�� �ִ��� ã�´�.						
					CItem* FindItem = TargetPlayer->_InventoryManager.FindInventoryItem(0, Items[i]->_ItemInfo.ItemSmallCategory);
					if (FindItem == nullptr)
					{				
						CItem* NewItem = NewItemCrate(Items[i]->_ItemInfo);
						
						// ã�� ������ ���
						// ��� �ִ� ������ ã�Ƽ� �ش� ������ �������� �ִ´�.
						TargetPlayer->_InventoryManager.InsertItem(0, NewItem);
						
						FindItem = TargetPlayer->_InventoryManager.GetItem(0, NewItem->_ItemInfo.TileGridPositionX, NewItem->_ItemInfo.TileGridPositionY);
					}
					else
					{
						IsExistItem = true;
						// ã���� ���
						FindItem->_ItemInfo.ItemCount += Items[i]->_ItemInfo.ItemCount;
					}
					
					// DB �κ��丮�� �����ϱ� ���� Job ����
					st_GameServerJob* DBInventorySaveJob = _GameServerJobMemoryPool->Alloc();
					DBInventorySaveJob->SessionId = Session->SessionId;

					CGameServerMessage* DBInventoryItemSaveMessage = CGameServerMessage::GameServerMessageAlloc();
					DBInventoryItemSaveMessage->Clear();

					*DBInventoryItemSaveMessage << (int16)en_GameServerJobType::DATA_BASE_LOOTING_ITEM_INVENTORY_SAVE;
					// Ÿ�� ObjectId
					*DBInventoryItemSaveMessage << TargetPlayer->_GameObjectInfo.ObjectId;
					// �ߺ� ����
					*DBInventoryItemSaveMessage << IsExistItem;
					// ���� ������ ����
					*DBInventoryItemSaveMessage << ItemEach;
					// �����ؾ��� ������;
					*DBInventoryItemSaveMessage << &Items[i];
					// ������ ������ ���
					*DBInventoryItemSaveMessage << &FindItem;
					// ������ ó���� AccountId
					*DBInventoryItemSaveMessage << AccountId;

					InterlockedIncrement64(&Session->DBReserveCount);
					Session->DBQue.Enqueue(DBInventoryItemSaveMessage);

					_GameServerUserDataBaseThreadMessageQue.Enqueue(DBInventorySaveJob);
					SetEvent(_UserDataBaseWakeEvent);					
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
		if (Session->IsDummy == true || memcmp(Session->Token,OutToken,ACCOUNT_TOKEN_LEN) == 0)
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

void CGameServer::PacketProcReqDBCreateCharacterNameCheck(int64 SessionID, CMessage* Message)
{
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

void CGameServer::PacketProcReqDBLootingItemToInventorySave(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		int64 TargetObjectId;
		*Message >> TargetObjectId;

		bool IsExistItem;
		*Message >> IsExistItem;

		int16 ItemEach;
		*Message >> ItemEach;

		CItem* DeleteItem;
		*Message >> &DeleteItem;

		CItem* Item = nullptr;
		*Message >> &Item;

		int64 OwnerAccountId;
		*Message >> OwnerAccountId;

		int64 ItemDBId = Item->_ItemInfo.ItemDBId;
		bool ItemIsQuickSlotUse = Item->_ItemInfo.ItemIsQuickSlotUse;
		bool ItemRotated = Item->_ItemInfo.Rotated;
		int16 ItemWidth = Item->_ItemInfo.Width;
		int16 ItemHeight = Item->_ItemInfo.Height;
		int16 ItemTileGridPositionX = Item->_ItemInfo.TileGridPositionX;
		int16 ItemTileGridPositionY = Item->_ItemInfo.TileGridPositionY;
		int8 ItemLargeCategory = (int8)Item->_ItemInfo.ItemLargeCategory;
		int8 ItemMediumCategory = (int8)Item->_ItemInfo.ItemMediumCategory;
		int16 ItemSmallCategory = (int16)Item->_ItemInfo.ItemSmallCategory;
		wstring ItemName = Item->_ItemInfo.ItemName;
		int16 ItemCount = Item->_ItemInfo.ItemCount;
		wstring ItemThumbnailImagePath = Item->_ItemInfo.ItemThumbnailImagePath;
		bool ItemEquipped = Item->_ItemInfo.ItemIsEquipped;					
		int32 ItemMinDamage = Item->_ItemInfo.ItemMinDamage;
		int32 ItemMaxDamage = Item->_ItemInfo.ItemMaxDamage;
		int32 ItemDefence = Item->_ItemInfo.ItemDefence;
		int32 ItemMaxCount = Item->_ItemInfo.ItemMaxCount;		

		//CDBConnection* ItemToInventorySaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);

		//// ������ Count ����
		//if (IsExistItem == true)
		//{
		//	SP::CDBGameServerItemRefreshPush ItemRefreshPush(*ItemToInventorySaveDBConnection);
		//	ItemRefreshPush.InAccountDBId(OwnerAccountId);
		//	ItemRefreshPush.InPlayerDBId(TargetObjectId);
		//	ItemRefreshPush.InItemType(ItemSmallCategory);
		//	ItemRefreshPush.InCount(ItemCount);
		//	ItemRefreshPush.InItemTileGridPositionX(ItemTileGridPositionX);
		//	ItemRefreshPush.InItemTileGridPositionY(ItemTileGridPositionY);

		//	ItemRefreshPush.Execute();
		//}
		//else
		//{
		//	// ���ο� ������ ���� �� Inventory DB �ֱ�			
		//	SP::CDBGameServerItemToInventoryPush ItemToInventoryPush(*ItemToInventorySaveDBConnection);
		//	ItemToInventoryPush.InIsQuickSlotUse(ItemIsQuickSlotUse);
		//	ItemToInventoryPush.InItemRotated(ItemRotated);
		//	ItemToInventoryPush.InItemWidth(ItemWidth);
		//	ItemToInventoryPush.InItemHeight(ItemHeight);
		//	ItemToInventoryPush.InItemTileGridPositionX(ItemTileGridPositionX);
		//	ItemToInventoryPush.InItemTileGridPositionY(ItemTileGridPositionY);
		//	ItemToInventoryPush.InItemLargeCategory(ItemLargeCategory);
		//	ItemToInventoryPush.InItemMediumCategory(ItemMediumCategory);
		//	ItemToInventoryPush.InItemSmallCategory(ItemSmallCategory);
		//	ItemToInventoryPush.InItemName(ItemName);
		//	ItemToInventoryPush.InItemCount(ItemCount);			
		//	ItemToInventoryPush.InIsEquipped(ItemEquipped);
		//	ItemToInventoryPush.InItemMinDamage(ItemMinDamage);
		//	ItemToInventoryPush.InItemMaxDamage(ItemMaxDamage);
		//	ItemToInventoryPush.InItemDefence(ItemDefence);
		//	ItemToInventoryPush.InItemMaxCount(ItemMaxCount);
		//	ItemToInventoryPush.InThumbnailImagePath(ItemThumbnailImagePath);
		//	ItemToInventoryPush.InOwnerAccountId(OwnerAccountId);
		//	ItemToInventoryPush.InOwnerPlayerId(TargetObjectId);

		//	ItemToInventoryPush.Execute();
		//}

		//G_DBConnectionPool->Push(en_DBConnect::GAME, ItemToInventorySaveDBConnection);

		// Ŭ�� �κ��丮���� �ش� �������� ����
		CMessage* ResItemToInventoryPacket = MakePacketResItemToInventory(TargetObjectId, Item, ItemEach, IsExistItem);
		SendPacket(Session->SessionId, ResItemToInventoryPacket);
		ResItemToInventoryPacket->Free();

		vector<CGameObject*> DeSpawnItem;
		DeSpawnItem.push_back(DeleteItem);

		// Ŭ�󿡰� �ش� ������ �����϶�� �˷���
		CMessage* ResItemDeSpawnPacket = MakePacketResObjectDeSpawn(1, DeSpawnItem);
		SendPacketFieldOfView(Session, ResItemDeSpawnPacket, true);		
		ResItemDeSpawnPacket->Free();

		bool ItemUse = true;
		// ������ DB���� Inventory�� ������ ������ ����	( ������ ���������� �ʰ� ItemUse�� 1�� �ٲ۴� )	
		CDBConnection* ItemDeleteDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
		SP::CDBGameServerItemDelete ItemDelete(*ItemDeleteDBConnection);
		ItemDelete.InItemDBId(ItemDBId);
		ItemDelete.InItemUse(ItemUse);

		ItemDelete.Execute();

		G_DBConnectionPool->Push(en_DBConnect::GAME, ItemDeleteDBConnection);

		// ���忡 �����Ǿ� �־��� �������� �ݳ��Ѵ�.
		G_ObjectManager->ObjectLeaveGame(DeleteItem, 1, IsExistItem);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBCraftingItemToInventorySave(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

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

void CGameServer::PacketProcReqDBItemPlace(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

	if (Session)
	{
		CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];			

		int16 PlaceItemTilePositionX;
		*Message >> PlaceItemTilePositionX;
		int16 PlaceITemTilePositionY;
		*Message >> PlaceITemTilePositionY;

		CItem* PlaceItem = MyPlayer->_InventoryManager.SwapItem(0, PlaceItemTilePositionX, PlaceITemTilePositionY);		

		CMessage* ResPlaceItemPacket = MakePacketResPlaceItem(Session->AccountId, MyPlayer->_GameObjectInfo.ObjectId, PlaceItem, MyPlayer->_InventoryManager._SelectItem);
		SendPacket(Session->SessionId, ResPlaceItemPacket);
		ResPlaceItemPacket->Free();			

		ReturnSession(Session);
	}
}

void CGameServer::PacketProcReqDBItemUpdate(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

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

				CGameServerMessage* ResChangeObjectStat = MakePacketResChangeObjectStat(MyPlayer->_GameObjectInfo.ObjectId,					
					MyPlayer->_GameObjectInfo.ObjectStatInfo);
				SendPacketFieldOfView(Session, ResChangeObjectStat, true);			
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

void CGameServer::PacketProcReqDBGoldSave(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

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

void CGameServer::PacketProcReqDBCharacterInfoSend(int64 SessionId, CMessage* Message)
{
	st_Session* Session = FindSession(SessionId);

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

#pragma region ��ų ���� �о����				
			// ĳ���Ͱ� �����ϰ� �ִ� ��ų ������ DB�κ��� �о�´�.
			CDBConnection* DBCharacterSkillGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerSkillGet CharacterSkillGet(*DBCharacterSkillGetConnection);
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

					AttackSkillInfo->CanSkillUse = true;
					AttackSkillInfo->IsSkillLearn = IsSkillLearn;
					AttackSkillInfo->IsQuickSlotUse = IsQuickSlotUse;
					AttackSkillInfo->SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;
					AttackSkillInfo->SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;
					AttackSkillInfo->SkillType = (en_SkillType)SkillType;
					AttackSkillInfo->SkillLevel = SkillLevel;
					AttackSkillInfo->SkillName = (LPWSTR)CA2W(FindAttackSkillData->SkillName.c_str());
					AttackSkillInfo->SkillCoolTime = FindAttackSkillData->SkillCoolTime;
					AttackSkillInfo->SkillCastingTime = FindAttackSkillData->SkillCastingTime;
					AttackSkillInfo->SkillDurationTime = FindAttackSkillData->SkillDurationTime;
					AttackSkillInfo->SkillDotTime = FindAttackSkillData->SkillDotTime;
					AttackSkillInfo->SkillRemainTime = 0;
					AttackSkillInfo->SkillImagePath = (LPWSTR)CA2W(FindAttackSkillData->SkillThumbnailImagePath.c_str());
					AttackSkillInfo->SkillMinDamage = FindAttackSkillData->SkillMinDamage;
					AttackSkillInfo->SkillMaxDamage = FindAttackSkillData->SkillMaxDamage;
					AttackSkillInfo->SkillTargetEffectTime = FindAttackSkillData->SkillTargetEffectTime;

					AttackSkillInfo->SkillDebufAttackSpeed = FindAttackSkillData->SkillDebufAttackSpeed;
					AttackSkillInfo->SkillDebufMovingSpeed = FindAttackSkillData->SkillDebufMovingSpeed;
					AttackSkillInfo->StatusAbnormalityProbability = FindAttackSkillData->StatusAbnormalityProbability;
					
					AttackSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL, AttackSkillInfo);

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

					TacTicSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL, TacTicSkillInfo);

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

					HealSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL, HealSkillInfo);

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

					BufSkill->SetSkillInfo(en_SkillCategory::QUICK_SLOT_SKILL, BufSkillInfo);

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

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterSkillGetConnection);
#pragma endregion

#pragma region ������ ���� �����ͼ� Ŭ�� ������
			// ������ ���� �ʱ�ȭ
			MyPlayer->_QuickSlotManager.Init();

			vector<st_QuickSlotBarSlotInfo> QuickSlotBarSlotInfos;

			// ������ ���̺� �����ؼ� �ش� ��ų�� ��ϵǾ� �ִ� ��� ������ ��ȣ �������
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

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotBarGetConnection);

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
			CDBConnection* DBInventoryItemInfoGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerInventoryItemGet CharacterInventoryItem(*DBInventoryItemInfoGetConnection);
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
					MaterialItem = (CMaterial*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_ITEM_MATERIAL);
					MaterialItem->_ItemInfo.ItemDBId = 0;
					MaterialItem->_ItemInfo.Rotated = ItemRotated;
					MaterialItem->_ItemInfo.Width = ItemWidth;
					MaterialItem->_ItemInfo.Height = ItemHeight;
					MaterialItem->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemLargeCategory;
					MaterialItem->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemMediumCategory;
					MaterialItem->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemSmallCategory;
					MaterialItem->_ItemInfo.ItemName = ItemName;
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

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBInventoryItemInfoGetConnection);

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
			//// ĳ���Ͱ� �����ϰ� �־��� ��� ������ GoldTable���� �о�´�.
			//CDBConnection* DBCharacterGoldGetConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			//SP::CDBGameServerGoldGet CharacterGoldGet(*DBCharacterGoldGetConnection);
			//CharacterGoldGet.InAccountDBId(MyPlayer->_AccountId);
			//CharacterGoldGet.InPlayerDBId(MyPlayer->_GameObjectInfo.ObjectId);

			//int64 GoldCoin = 0;
			//int16 SliverCoin = 0;
			//int16 BronzeCoin = 0;

			//CharacterGoldGet.OutGoldCoin(GoldCoin);
			//CharacterGoldGet.OutSliverCoin(SliverCoin);
			//CharacterGoldGet.OutBronzeCoin(BronzeCoin);

			//if (CharacterGoldGet.Execute() && CharacterGoldGet.Fetch())
			//{
			//	// DB���� �о�� Gold�� Inventory�� �����Ѵ�.
			//	MyPlayer->_InventoryManager._GoldCoinCount = GoldCoin;
			//	MyPlayer->_InventoryManager._SliverCoinCount = SliverCoin;
			//	MyPlayer->_InventoryManager._BronzeCoinCount = BronzeCoin;

			//	// DBConnection �ݳ��ϰ�
			//	G_DBConnectionPool->Push(en_DBConnect::GAME, DBCharacterGoldGetConnection);

			//	// ��� ���� ���
			//	*ResCharacterInfoMessage << GoldCoin;
			//	*ResCharacterInfoMessage << SliverCoin;
			//	*ResCharacterInfoMessage << BronzeCoin;				
			//}			
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
			
			SendPacket(Session->SessionId, ResCharacterInfoMessage);
			ResCharacterInfoMessage->Free();
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

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];
						
			st_QuickSlotBarSlotInfo SaveQuickSlotBarSlot;
			SaveQuickSlotBarSlot.AccountDBId = MyPlayer->_AccountId;
			SaveQuickSlotBarSlot.PlayerDBId = MyPlayer->_GameObjectInfo.ObjectId;
			SaveQuickSlotBarSlot.QuickSlotBarIndex = QuickSlotBarIndex;
			SaveQuickSlotBarSlot.QuickSlotBarSlotIndex = QuickSlotBarSlotIndex;
			SaveQuickSlotBarSlot.QuickSlotKey = QuickSlotKey;

			CSkill* FindSkill = MyPlayer->_SkillBox.FindSkill((en_SkillType)QuickSlotSkillType);
			if (FindSkill != nullptr)
			{
				SaveQuickSlotBarSlot.QuickBarSkill = FindSkill;
			}
			else
			{
				CRASH(L"��ų �ڽ��� ���� ��ų�� �����Կ� ����Ϸ��� �õ�");
			}

			FindSkill->GetSkillInfo()->IsQuickSlotUse = true;

			MyPlayer->_QuickSlotManager.UpdateQuickSlotBar(SaveQuickSlotBarSlot);			

			int8 SkillLargeCategory = (int8)SaveQuickSlotBarSlot.QuickBarSkill->GetSkillInfo()->SkillLargeCategory;
			int8 SkillMediumCategory = (int8)SaveQuickSlotBarSlot.QuickBarSkill->GetSkillInfo()->SkillMediumCategory;
			int16 SkillType = (int16)SaveQuickSlotBarSlot.QuickBarSkill->GetSkillInfo()->SkillType;

			// DB�� ������ ���� ����
			CDBConnection* DBQuickSlotUpdateConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotBarSlotUpdate QuickSlotUpdate(*DBQuickSlotUpdateConnection);
			QuickSlotUpdate.InAccountDBId(SaveQuickSlotBarSlot.AccountDBId);
			QuickSlotUpdate.InPlayerDBId(SaveQuickSlotBarSlot.PlayerDBId);
			QuickSlotUpdate.InQuickSlotBarIndex(SaveQuickSlotBarSlot.QuickSlotBarIndex);
			QuickSlotUpdate.InQuickSlotBarSlotIndex(SaveQuickSlotBarSlot.QuickSlotBarSlotIndex);
			QuickSlotUpdate.InQuickSlotKey(SaveQuickSlotBarSlot.QuickSlotKey);
			QuickSlotUpdate.InSkillLargeCategory(SkillLargeCategory);
			QuickSlotUpdate.InSkillMediumCategory(SkillMediumCategory);
			QuickSlotUpdate.InSkillType(SkillType);
			QuickSlotUpdate.InSkillLevel(SaveQuickSlotBarSlot.QuickBarSkill->GetSkillInfo()->SkillLevel);

			QuickSlotUpdate.Execute();			

			G_DBConnectionPool->Push(en_DBConnect::GAME, DBQuickSlotUpdateConnection);

			CMessage* ResQuickSlotUpdateMessage = MakePacketResQuickSlotBarSlotSave(SaveQuickSlotBarSlot);
			SendPacket(Session->SessionId, ResQuickSlotUpdateMessage);
			ResQuickSlotUpdateMessage->Free();
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

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];			

#pragma region ������ A�� DB�� �ִ��� Ȯ��
			// �ش� ������ ��ġ�� ������ �ִ��� DB���� Ȯ��
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

			// ���� ��û A ���� ����
			st_QuickSlotBarSlotInfo SwapAQuickSlotBarInfo;
			SwapAQuickSlotBarInfo.AccountDBId = AccountId;
			SwapAQuickSlotBarInfo.PlayerDBId = PlayerId;
			SwapAQuickSlotBarInfo.QuickSlotBarIndex = QuickSlotBarSwapIndexB;
			SwapAQuickSlotBarInfo.QuickSlotBarSlotIndex = QuickSlotBarSlotSwapIndexB;			
			
			st_QuickSlotBarSlotInfo* FindAQuickslotInfo = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarSwapIndexA, QuickSlotBarSlotSwapIndexA);
			if (FindAQuickslotInfo != nullptr)
			{
				if (FindAQuickslotInfo->QuickBarSkill != nullptr)
				{
					SwapAQuickSlotBarInfo.QuickBarSkill = FindAQuickslotInfo->QuickBarSkill;
				}
				else
				{
					SwapAQuickSlotBarInfo.QuickBarSkill = nullptr;
				}
			}
			else
			{
				CRASH("������ ������ ã�� �� ����");
			}		
			
#pragma endregion
#pragma region ������ B�� DB�� �ִ��� Ȯ��
			// �ش� ������ ��ġ�� ������ �ִ��� DB���� Ȯ��
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

			// ���� ��û B ���� ����
			st_QuickSlotBarSlotInfo SwapBQuickSlotBarInfo;
			SwapBQuickSlotBarInfo.AccountDBId = AccountId;
			SwapBQuickSlotBarInfo.PlayerDBId = PlayerId;
			SwapBQuickSlotBarInfo.QuickSlotBarIndex = QuickSlotBarSwapIndexA;
			SwapBQuickSlotBarInfo.QuickSlotBarSlotIndex = QuickSlotBarSlotSwapIndexA;

			st_QuickSlotBarSlotInfo* FindBQuickslotInfo = MyPlayer->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarSwapIndexB, QuickSlotBarSlotSwapIndexB);
			if (FindBQuickslotInfo != nullptr)
			{
				if (FindBQuickslotInfo->QuickBarSkill != nullptr)
				{
					SwapBQuickSlotBarInfo.QuickBarSkill = FindBQuickslotInfo->QuickBarSkill;
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

			SwapAQuickSlotBarInfo.QuickSlotKey = QuickSlotBKey;
			SwapBQuickSlotBarInfo.QuickSlotKey = QuickSlotAKey;
#pragma endregion

#pragma region DB���� ������ ����
			CDBConnection* DBQuickSlotSwapConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
			SP::CDBGameServerQuickSlotSwap QuickSlotSwap(*DBQuickSlotSwapConnection);
			QuickSlotSwap.InAccountDBId(AccountId);
			QuickSlotSwap.InPlayerDBId(PlayerId);

			QuickSlotSwap.InAQuickSlotBarIndex(SwapBQuickSlotBarInfo.QuickSlotBarIndex);
			QuickSlotSwap.InAQuickSlotBarSlotIndex(SwapBQuickSlotBarInfo.QuickSlotBarSlotIndex);
			QuickSlotSwap.InASkillLargeCategory(QuickSlotBSkillLargeCategory);
			QuickSlotSwap.InASkillMediumCategory(QuickSlotBSkillMediumCategory);
			QuickSlotSwap.InAQuickSlotSkillType(QuickSlotBSkillType);
			QuickSlotSwap.InAQuickSlotSkillLevel(QuickSlotBSkillLevel);

			QuickSlotSwap.InBQuickSlotBarIndex(SwapAQuickSlotBarInfo.QuickSlotBarIndex);
			QuickSlotSwap.InBQuickSlotBarSlotIndex(SwapAQuickSlotBarInfo.QuickSlotBarSlotIndex);
			QuickSlotSwap.InBSkillLargeCategory(QuickSlotASkillLargeCategory);
			QuickSlotSwap.InBSkillMediumCategory(QuickSlotASkillMediumCategory);
			QuickSlotSwap.InBQuickSlotSkillType(QuickSlotASkillType);
			QuickSlotSwap.InBQuickSlotSkillLevel(QuickSlotASkillLevel);

			bool QuickSlotSwapSuccess = QuickSlotSwap.Execute();
			if (QuickSlotSwapSuccess == true)
			{
				MyPlayer->_QuickSlotManager.SwapQuickSlot(SwapBQuickSlotBarInfo, SwapAQuickSlotBarInfo);

				// Ŭ�󿡰� ��� ����
				CMessage* ResQuickSlotSwapPacket = MakePacketResQuickSlotSwap(AccountId, PlayerId,
					SwapBQuickSlotBarInfo,
					SwapAQuickSlotBarInfo);
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
			int64 AccountId;
			*Message >> AccountId;

			int64 PlayerId;
			*Message >> PlayerId;

			int8 QuickSlotBarIndex;
			*Message >> QuickSlotBarIndex;

			int8 QuickSlotBarSlotIndex;
			*Message >> QuickSlotBarSlotIndex;

			// ���ӿ� ������ ĳ���͸� �����´�.
			CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

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
			
			// �ش� ������ ��ġ�� ������ �ִ��� DB���� Ȯ��
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

			// ã�� ������ ������ �ʱ�ȭ
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

			CMessage* ResQuickSlotInitMessage = MakePacketResQuickSlotInit(Session->AccountId, MyPlayer->_GameObjectInfo.ObjectId, QuickSlotBarIndex, QuickSlotBarSlotIndex, QuickSlotKey);
			SendPacket(Session->SessionId, ResQuickSlotInitMessage);
			ResQuickSlotInitMessage->Free();
		} while (0);
	}

	ReturnSession(Session);
}

void CGameServer::PacketProcReqDBLeavePlayerInfoSave(CMessage* Message)
{
	int64 AccountID;
	*Message >> AccountID;
	int64 ObjectID;
	*Message >> ObjectID;
	int32 Level;
	*Message >> Level;
	int32 MaxHP;
	*Message >> MaxHP;
	int32 MaxMP;
	*Message >> MaxMP;
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
	int16 MagicDamage;
	int16 MAgicHitRate;
	int32 Defence;
	int16 EvasionRate;
	int16 MeleeCriticalPoint;
	int16 MagicCriticalPoint;
	float Speed;
	int32 CollisionPositionX;
	int32 CollisionPositionY;
	int64 CurrentExperience;
	int64 RequireExperience;
	int64 TotalExperience;

	CDBConnection* PlayerInfoSaveDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
	SP::CDBGameServerLeavePlayerStatInfoSave LeavePlayerStatInfoSave(*PlayerInfoSaveDBConnection);

	LeavePlayerStatInfoSave.InAccountDBId(AccountID);
	LeavePlayerStatInfoSave.InPlayerDBId(ObjectID);
	LeavePlayerStatInfoSave.InLevel(Level);
	LeavePlayerStatInfoSave.InMaxHP(MaxHP);
	LeavePlayerStatInfoSave.InMaxMP(MaxMP);
	LeavePlayerStatInfoSave.InMaxDP(MaxDP);
	LeavePlayerStatInfoSave.InAutoRecoveryHPPercent(AutoRecoveryHPPercent);
	LeavePlayerStatInfoSave.InAutoRecoveryMPPercent(AutoRecoveryMPPercent);
	LeavePlayerStatInfoSave.InMinMeleeAttackDamage(MinMeleeAttackDamage);
	LeavePlayerStatInfoSave.InMaxMeleeAttackDamage(MaxMeleeAttackDamage);
	LeavePlayerStatInfoSave.InMeleeAttackHitRate(MeleeAttackHitRate);
	LeavePlayerStatInfoSave.InMagicDamage(MagicDamage);
	LeavePlayerStatInfoSave.InMagicHitRate(MAgicHitRate);
	LeavePlayerStatInfoSave.InDefence(Defence);
	LeavePlayerStatInfoSave.InEvasionRate(EvasionRate);
	LeavePlayerStatInfoSave.InMeleeCriticalPoint(MeleeCriticalPoint);
	LeavePlayerStatInfoSave.InMagicCriticalPoint(MagicCriticalPoint);
	LeavePlayerStatInfoSave.InSpeed(Speed);
	LeavePlayerStatInfoSave.InLastPositionY(CollisionPositionX);
	LeavePlayerStatInfoSave.InLastPositionX(CollisionPositionY);
	LeavePlayerStatInfoSave.InCurrentExperience(CurrentExperience);
	LeavePlayerStatInfoSave.InRequireExperience(RequireExperience);
	LeavePlayerStatInfoSave.InTotalExperience(TotalExperience);

	LeavePlayerStatInfoSave.Execute();

	G_DBConnectionPool->Push(en_DBConnect::GAME, PlayerInfoSaveDBConnection);

	//// �κ��丮�� ������ ��
	//if (MyPlayer->_InventoryManager._Inventorys[0] != nullptr)
	//{
	//	SP::CDBGameServerInventoryPlace LeavePlayerInventoryItemSave(*PlayerInfoSaveDBConnection);
	//	LeavePlayerInventoryItemSave.InOwnerAccountId(MyPlayer->_AccountId);
	//	LeavePlayerInventoryItemSave.InOwnerPlayerId(MyPlayer->_GameObjectInfo.ObjectId);

	//	vector<st_ItemInfo> PlayerInventoryItems = MyPlayer->_InventoryManager._Inventorys[0]->DBInventorySaveReturnItems();

	//	for (st_ItemInfo InventoryItem : PlayerInventoryItems)
	//	{
	//		int8 InventoryItemLargeCategory = (int8)InventoryItem.ItemLargeCategory;
	//		int8 InventoryItemMediumCategory = (int8)InventoryItem.ItemMediumCategory;
	//		int16 InventoryItemSmallCategory = (int16)InventoryItem.ItemSmallCategory;

	//		LeavePlayerInventoryItemSave.InIsQuickSlotUse(InventoryItem.ItemIsQuickSlotUse);
	//		LeavePlayerInventoryItemSave.InItemRotated(InventoryItem.ItemIsQuickSlotUse);
	//		LeavePlayerInventoryItemSave.InItemWidth(InventoryItem.Width);
	//		LeavePlayerInventoryItemSave.InItemHeight(InventoryItem.Height);
	//		LeavePlayerInventoryItemSave.InItemTileGridPositionX(InventoryItem.TileGridPositionX);
	//		LeavePlayerInventoryItemSave.InItemTileGridPositionY(InventoryItem.TileGridPositionY);
	//		LeavePlayerInventoryItemSave.InItemLargeCategory(InventoryItemLargeCategory);
	//		LeavePlayerInventoryItemSave.InItemMediumCategory(InventoryItemMediumCategory);
	//		LeavePlayerInventoryItemSave.InItemSmallCategory(InventoryItemSmallCategory);
	//		LeavePlayerInventoryItemSave.InItemName(InventoryItem.ItemName);
	//		LeavePlayerInventoryItemSave.InItemCount(InventoryItem.ItemCount);
	//		LeavePlayerInventoryItemSave.InIsEquipped(InventoryItem.ItemIsEquipped);
	//		LeavePlayerInventoryItemSave.InItemMinDamage(InventoryItem.ItemMinDamage);
	//		LeavePlayerInventoryItemSave.InItemMaxDamage(InventoryItem.ItemMaxDamage);
	//		LeavePlayerInventoryItemSave.InItemDefence(InventoryItem.ItemDefence);
	//		LeavePlayerInventoryItemSave.InItemMaxCount(InventoryItem.ItemMaxCount);
	//		LeavePlayerInventoryItemSave.InItemThumbnailImagePath(InventoryItem.ItemThumbnailImagePath);

	//		LeavePlayerInventoryItemSave.Execute();
	//	}
	//}	

	//G_DBConnectionPool->Push(en_DBConnect::GAME, PlayerInfoSaveDBConnection);
}

void CGameServer::PacketProcTimerAttackEnd(int64 SessionId, CGameServerMessage* Message)
{
	st_Session* Session = FindSession(SessionId);	

	if (Session)
	{
		// ���ӿ� ������ ĳ���͸� �����´�.
		CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];		

		MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		MyPlayer->_SkillType = en_SkillType::SKILL_TYPE_NONE;

		CMessage* ResObjectStateMessage = MakePacketResChangeObjectState(MyPlayer->_GameObjectInfo.ObjectId,
			MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, 
			MyPlayer->_GameObjectInfo.ObjectType,
			MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
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

	CSkill* SpellEndSkill = nullptr;
	*Message >> &SpellEndSkill;

	Message->Free();

	CGameObject* FindObject = G_ObjectManager->Find(TargetObjectId,(en_GameObjectType)TargetObjectType);

	if (Session)
	{
		// ���ӿ� ������ ĳ���͸� �����´�.
		CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];		

		// ���� ���� �Ϸ� ������ ���� ���·� ������
		MyPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

		CMessage* ResObjectStateChangePacket = MakePacketResChangeObjectState(MyPlayer->_GameObjectInfo.ObjectId, MyPlayer->_GameObjectInfo.ObjectPositionInfo.MoveDir, MyPlayer->_GameObjectInfo.ObjectType, MyPlayer->_GameObjectInfo.ObjectPositionInfo.State);
		SendPacketFieldOfView(Session, ResObjectStateChangePacket, true);		
		ResObjectStateChangePacket->Free();				

		SpellEndSkill->CoolTimeStart();

		// Ŭ�󿡰� ��Ÿ�� ǥ��
		CMessage* ResCoolTimeStartPacket = MakePacketCoolTime(QuickSlotBarIndex,
			QuickSlotBarSlotIndex,
			1.0f, SpellEndSkill);
		SendPacket(Session->SessionId, ResCoolTimeStartPacket);
		ResCoolTimeStartPacket->Free();

		en_EffectType HitEffectType = en_EffectType::EFFECT_TYPE_NONE;

		wstring MagicSystemString;

		wchar_t SpellMessage[64] = L"0";

		// ũ��Ƽ�� �Ǵ�
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

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkill->GetSkillInfo();

				uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
				int32 ChoiceDamage = DamageChoiceRandom(Gen);
				FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

				// ������ ó��
				MyPlayer->GetTarget()->OnDamaged(MyPlayer, FinalDamage);

				wsprintf(SpellMessage, L"%s�� %s�� ����� %s���� %d�� �������� ����ϴ�.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), SpellEndSkill->GetSkillInfo()->SkillName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

				MagicSystemString = SpellMessage;
			}
		break;
		case en_SkillType::SKILL_SHAMAN_ROOT:
			{
				HitEffectType = en_EffectType::EFFECT_DEBUF_ROOT;			

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkill->GetSkillInfo();

				CSkill* NewSkill = G_ObjectManager->SkillCreate();
				
				st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(SpellEndSkill->GetSkillInfo()->SkillMediumCategory);
				*NewAttackSkillInfo = *((st_AttackSkillInfo*)SpellEndSkill->GetSkillInfo());
				NewSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
				NewSkill->StatusAbnormalDurationTimeStart();

				MyPlayer->GetTarget()->AddDebuf(NewSkill);
				MyPlayer->GetTarget()->SetStatusAbnormal(STATUS_ABNORMAL_SHAMAN_ROOT);

				CMessage* ResStatusAbnormalPacket = MakePacketStatusAbnormal(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId,
					MyPlayer->GetTarget()->_GameObjectInfo.ObjectType,
					MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.MoveDir,
					SpellEndSkill->GetSkillInfo()->SkillType,
					true, STATUS_ABNORMAL_SHAMAN_ROOT);
				SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResStatusAbnormalPacket, MyPlayer);
				ResStatusAbnormalPacket->Free();

				CMessage* ResBufDeBufSkillPacket = MakePacketBufDeBuf(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
				SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResBufDeBufSkillPacket, MyPlayer);
				ResBufDeBufSkillPacket->Free();												
			}
			break;
		case en_SkillType::SKILL_SHAMAN_ICE_CHAIN:
			{
				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkill->GetSkillInfo();

				MyPlayer->GetTarget()->_GameObjectInfo.ObjectStatInfo.Speed -= AttackSkillInfo->SkillDebufMovingSpeed;

				CMessage* ResObjectStatChange = MakePacketResChangeObjectStat(MyPlayer->_GameObjectInfo.ObjectId,				
					MyPlayer->_GameObjectInfo.ObjectStatInfo);
				SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResObjectStatChange, MyPlayer);
				ResObjectStatChange->Free();

				CSkill* NewSkill = G_ObjectManager->SkillCreate();
				
				st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(SpellEndSkill->GetSkillInfo()->SkillMediumCategory);
				*NewAttackSkillInfo = *((st_AttackSkillInfo*)SpellEndSkill->GetSkillInfo());
				NewSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
				NewSkill->StatusAbnormalDurationTimeStart();

				MyPlayer->GetTarget()->AddDebuf(NewSkill);
				MyPlayer->GetTarget()->SetStatusAbnormal(STATUS_ABNORMAL_SHAMAN_ICE_CHAIN);

				CMessage* ResStatusAbnormalPacket = MakePacketStatusAbnormal(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId,
					MyPlayer->GetTarget()->_GameObjectInfo.ObjectType,
					MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.MoveDir,
					SpellEndSkill->GetSkillInfo()->SkillType, true, STATUS_ABNORMAL_SHAMAN_ICE_CHAIN);
				SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResStatusAbnormalPacket, MyPlayer);
				ResStatusAbnormalPacket->Free();

				CMessage* ResBufDeBufSkillPacket = MakePacketBufDeBuf(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
				SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResBufDeBufSkillPacket, MyPlayer);
				ResBufDeBufSkillPacket->Free();
			}
			break;		
		case en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE:
		{
			HitEffectType = en_EffectType::EFFECT_LIGHTNING;						

			int32 MagicDamage = MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6;

			st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkill->GetSkillInfo();

			uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
			int32 ChoiceDamage = DamageChoiceRandom(Gen);
			FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

			// ������ ó��
			MyPlayer->GetTarget()->OnDamaged(MyPlayer, FinalDamage);

			CSkill* NewSkill = G_ObjectManager->SkillCreate();

			st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(SpellEndSkill->GetSkillInfo()->SkillMediumCategory);
			*NewAttackSkillInfo = *((st_AttackSkillInfo*)SpellEndSkill->GetSkillInfo());
			NewSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
			NewSkill->StatusAbnormalDurationTimeStart();

			MyPlayer->GetTarget()->AddDebuf(NewSkill);
			MyPlayer->GetTarget()->SetStatusAbnormal(STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE);

			CMessage* ResStatusAbnormalPacket = MakePacketStatusAbnormal(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId,
				MyPlayer->GetTarget()->_GameObjectInfo.ObjectType,
				MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.MoveDir,
				SpellEndSkill->GetSkillInfo()->SkillType, 
				true, STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE);
			SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResStatusAbnormalPacket, MyPlayer);
			ResStatusAbnormalPacket->Free();

			CMessage* ResBufDeBufSkillPacket = MakePacketBufDeBuf(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
			SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResBufDeBufSkillPacket, MyPlayer);
			ResBufDeBufSkillPacket->Free();

			float EffectPrintTime = SpellEndSkill->GetSkillInfo()->SkillDurationTime / 1000.0f;

			// ����Ʈ ���
			CMessage* ResEffectPacket = MakePacketEffect(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_DEBUF_STUN, EffectPrintTime);
			SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResEffectPacket, MyPlayer);
			ResEffectPacket->Free();
		}
		break;
		case en_SkillType::SKILL_SHAMAN_HELL_FIRE:
		{
			HitEffectType = en_EffectType::EFFECT_FLAME_HARPOON_TARGET;

			int32 MagicDamage = MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6;

			st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkill->GetSkillInfo();

			uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
			int32 ChoiceDamage = DamageChoiceRandom(Gen);
			FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

			// ������ ó��
			MyPlayer->GetTarget()->OnDamaged(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s�� %s�� ����� %s���� %d�� �������� ����ϴ�.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), SpellEndSkill->GetSkillInfo()->SkillName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

			MagicSystemString = SpellMessage;
		}
		break;
		case en_SkillType::SKILL_TAIOIST_DIVINE_STRIKE:
		{
			HitEffectType = en_EffectType::EFFECT_FLAME_HARPOON_TARGET;

			int32 MagicDamage = (int32)(MyPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6);

			st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkill->GetSkillInfo();

			uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
			int32 ChoiceDamage = DamageChoiceRandom(Gen);
			FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

			// ������ ó��
			MyPlayer->GetTarget()->OnDamaged(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s�� %s�� ����� %s���� %d�� �������� ����ϴ�.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), SpellEndSkill->GetSkillInfo()->SkillName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

			MagicSystemString = SpellMessage;
		}
		break;
		case en_SkillType::SKILL_TAIOIST_ROOT:
		{
			HitEffectType = en_EffectType::EFFECT_DEBUF_ROOT;

			st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)SpellEndSkill->GetSkillInfo();

			CSkill* NewSkill = G_ObjectManager->SkillCreate();

			st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(SpellEndSkill->GetSkillInfo()->SkillMediumCategory);
			*NewAttackSkillInfo = *((st_AttackSkillInfo*)SpellEndSkill->GetSkillInfo());
			NewSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
			NewSkill->StatusAbnormalDurationTimeStart();

			MyPlayer->GetTarget()->AddDebuf(NewSkill);
			MyPlayer->GetTarget()->SetStatusAbnormal(STATUS_ABNORMAL_TAIOIST_ROOT);

			CMessage* ResStatusAbnormalPacket = MakePacketStatusAbnormal(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId,
				MyPlayer->GetTarget()->_GameObjectInfo.ObjectType,
				MyPlayer->GetTarget()->_GameObjectInfo.ObjectPositionInfo.MoveDir,
				SpellEndSkill->GetSkillInfo()->SkillType,
				true, STATUS_ABNORMAL_TAIOIST_ROOT);
			SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResStatusAbnormalPacket, MyPlayer);
			ResStatusAbnormalPacket->Free();

			CMessage* ResBufDeBufSkillPacket = MakePacketBufDeBuf(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
			SendPacketFieldOfView(MyPlayer->_FieldOfViewInfos, ResBufDeBufSkillPacket, MyPlayer);
			ResBufDeBufSkillPacket->Free();			
		}
		break;
		case en_SkillType::SKILL_TAIOIST_HEALING_LIGHT:
		{
			HitEffectType = en_EffectType::EFFECT_HEALING_LIGHT_TARGET;

			st_HealSkillInfo* HealSkillInfo = (st_HealSkillInfo*)SpellEndSkill->GetSkillInfo();

			uniform_int_distribution<int> HealChoiceRandom(HealSkillInfo->SkillMinHealPoint, HealSkillInfo->SkillMaxHealPoint);
			FinalDamage = HealChoiceRandom(Gen);

			MyPlayer->GetTarget()->OnHeal(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s�� ġ���Ǻ��� ����� %s�� %d��ŭ ȸ���߽��ϴ�.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
			MagicSystemString = SpellMessage;
		}
		break;
		case en_SkillType::SKILL_TAIOIST_HEALING_WIND:
		{
			HitEffectType = en_EffectType::EFFECT_HEALING_WIND_TARGET;

			st_HealSkillInfo* HealSkillInfo = (st_HealSkillInfo*)SpellEndSkill->GetSkillInfo();

			uniform_int_distribution<int> HealChoiceRandom(HealSkillInfo->SkillMinHealPoint, HealSkillInfo->SkillMaxHealPoint);
			FinalDamage = HealChoiceRandom(Gen);

			MyPlayer->GetTarget()->OnHeal(MyPlayer, FinalDamage);

			wsprintf(SpellMessage, L"%s�� ġ���ǹٶ��� ����� %s�� %d��ŭ ȸ���߽��ϴ�.", MyPlayer->_GameObjectInfo.ObjectName.c_str(), MyPlayer->GetTarget()->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
			MagicSystemString = SpellMessage;
		}
		break;
		default:
			break;
		}

		// ���� ����
		CMessage* ResAttackMagicPacket = MakePacketResAttack(
			MyPlayer->_GameObjectInfo.ObjectId,
			MyPlayer->GetTarget()->_GameObjectInfo.ObjectId,
			MyPlayer->_SkillType,
			FinalDamage,
			false);
		SendPacketFieldOfView(Session, ResAttackMagicPacket, true);
		ResAttackMagicPacket->Free();


		// �ý��� �޼��� ����
		CMessage* ResAttackMagicSystemMessagePacket = MakePacketResChattingBoxMessage(MyPlayer->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, st_Color::White(), MagicSystemString);
		SendPacketFieldOfView(Session, ResAttackMagicSystemMessagePacket, true);		
		ResAttackMagicSystemMessagePacket->Free();

		// HP ���� ����
		CMessage* ResChangeObjectStat = MakePacketResChangeObjectStat(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId,			
			MyPlayer->GetTarget()->_GameObjectInfo.ObjectStatInfo);
		SendPacketFieldOfView(Session, ResChangeObjectStat, true);
		ResChangeObjectStat->Free();

		// ����â ��
		CMessage* ResMagicPacket = MakePacketResMagic(MyPlayer->_GameObjectInfo.ObjectId, false);
		SendPacketFieldOfView(Session, ResMagicPacket, true);		
		ResMagicPacket->Free();

		// ����Ʈ ���
		CMessage* ResEffectPacket = MakePacketEffect(MyPlayer->GetTarget()->_GameObjectInfo.ObjectId, HitEffectType, SpellEndSkill->GetSkillInfo()->SkillTargetEffectTime);
		SendPacketFieldOfView(Session, ResEffectPacket, true);		
		ResEffectPacket->Free();		
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
		// ������ ��ġ�� �̹� �ٸ� ������Ʈ�� ���� ��� ���� ���� �ʰ� �ٽ� �����Ѵ�. 		
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
		case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
		case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
		case en_GameObjectType::OBJECT_THIEF_PLAYER:
		case en_GameObjectType::OBJECT_ARCHER_PLAYER:
		case en_GameObjectType::OBJECT_PLAYER_DUMMY:
			{
				st_Session* Session = FindSession(SessionId);

				if (Session != nullptr)
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
							bool MapApplyMoveSuccess = Channel->_Map->ApplyMove(ChangeStateObject, ChangeStateObjectPosition);
							if (MapApplyMoveSuccess == true)
							{
								ChangeStatePlayerObject->_GameObjectInfo.ObjectPositionInfo.State = (en_CreatureState)ChangeState;

								CGameServerMessage* ResObjectStateChangeMessage = MakePacketResChangeObjectState(TargetObjectId, ChangeStateObject->_GameObjectInfo.ObjectPositionInfo.MoveDir, ChangeStateObject->_GameObjectInfo.ObjectType, (en_CreatureState)ChangeState);
								SendPacketFieldOfView(Session, ResObjectStateChangeMessage);
								ResObjectStateChangeMessage->Free();
							}
						}
					}
					break;
					default:
					{
						ChangeStatePlayerObject->_GameObjectInfo.ObjectPositionInfo.State = (en_CreatureState)ChangeState;

						CGameServerMessage* ResObjectStateChangeMessage = MakePacketResChangeObjectState(TargetObjectId, ChangeStateObject->_GameObjectInfo.ObjectPositionInfo.MoveDir, ChangeStateObject->_GameObjectInfo.ObjectType, (en_CreatureState)ChangeState);
						SendPacketFieldOfView(Session, ResObjectStateChangeMessage);
						ResObjectStateChangeMessage->Free();
					}
					break;
					}
				
					ReturnSession(Session);
				}								
			}
			break;
		default:
			break;
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

//---------------------------------------------------------------
//�α��� ��û ���� ��Ŷ ����� �Լ�
//WORD Type
//BYTE Status  //0 : ����  1 : ����
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

CGameServerMessage* CGameServer::MakePacketResEnterGame(bool EnterGameSuccess, st_GameObjectInfo* ObjectInfo)
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

	return ResEnterGamePacket;
}

// int64 AccountId
// int32 PlayerDBId
// st_GameObjectInfo ObjectInfo
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

CGameServerMessage* CGameServer::MakePacketResItemToInventory(int64 TargetObjectId, CItem* InventoryItem, int16 ItemEach, bool IsExist, bool ItemGainPrint)
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
	// �κ��丮 ������ ���� ���
	*ResItemToInventoryMessage << InventoryItem;
	// ������ ���� ����
	*ResItemToInventoryMessage << ItemEach;
	// ������ �ߺ� ����
	*ResItemToInventoryMessage << IsExist;
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

CGameServerMessage* CGameServer::MakePacketResMoveStop(int64 AccountId, int64 ObjectId, st_PositionInfo PositionInto)
{
	CGameServerMessage* ResMoveStopPacket = CGameServerMessage::GameServerMessageAlloc();
	if (ResMoveStopPacket == nullptr)
	{
		return nullptr;
	}

	ResMoveStopPacket->Clear();

	*ResMoveStopPacket << (int16)en_PACKET_S2C_MOVE_STOP;
	*ResMoveStopPacket << AccountId;
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

// int64 AccountId
// int32 PlayerDBId
// st_GameObjectInfo GameObjectInfo
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
		// SpawnObjectId
		*ResSpawnPacket << ObjectInfos[i]->_GameObjectInfo;
	}

	return ResSpawnPacket;
}

// int64 AccountId
// int32 PlayerDBId
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
		*ResDeSpawnPacket << DeSpawnObjects[i]->_GameObjectInfo.ObjectId;
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

CGameServerMessage* CGameServer::MakePacketCoolTime(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, float SkillCoolTimeSpeed, CSkill* QuickSlotSkill)
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

	if (QuickSlotSkill->GetSkillInfo())
	{
		*ResCoolTimeMessage << *QuickSlotSkill->GetSkillInfo();
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

	*ResErrorMessage << (int16)en_PACKET_PERSONAL_MESSAGE;
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
	}

	wstring ErrorMessageString = ErrorMessage;

	// ���� �޼���
	int16 ErrorMessageLen = (int16)(ErrorMessageString.length() * 2);
	*ResErrorMessage << ErrorMessageLen;
	ResErrorMessage->InsertData(ErrorMessageString.c_str(), ErrorMessageLen);

	return ResErrorMessage;
}

CGameServerMessage* CGameServer::MakePacketStatusAbnormalMessage(en_CommonErrorType ErrorType, int8 StatusAbnormalCount, int8 StatusAbnormal)
{
	CGameServerMessage* ResErrorMessage = CGameServerMessage::GameServerMessageAlloc();
	if (ResErrorMessage == nullptr)
	{
		return nullptr;
	}

	ResErrorMessage->Clear();	

	*ResErrorMessage << (int16)en_PACKET_PERSONAL_MESSAGE;
	*ResErrorMessage << StatusAbnormalCount;

	WCHAR ErrorMessage[100] = { 0 };
	
	switch (ErrorType)
	{
	case en_CommonErrorType::ERROR_STATUS_ABNORMAL_MOVE:
		{
			for (int8 i = 0; i < StatusAbnormalCount; i++)
			{
				wstring ErrorMessageString;

				if (StatusAbnormal & STATUS_ABNORMAL_WARRIOR_CHOHONE)
				{
					*ResErrorMessage << (int8)en_PersonalMessageType::PERSONAL_MESSAGE_STATUS_ABNORMAL_WARRIOR_CHOHONE;
					StatusAbnormal &= STATUS_ABNORMAL_WARRIOR_CHOHONE_MASK;

					wsprintf(ErrorMessage, L"��ȥ�� �����̻� �ɷ� ������ �� �����ϴ�.");					

					ErrorMessageString = ErrorMessage;

					int16 ErrorMessageLen = (int16)(ErrorMessageString.length() * 2);
					*ResErrorMessage << ErrorMessageLen;
					ResErrorMessage->InsertData(ErrorMessageString.c_str(), ErrorMessageLen);
				}
				else if (StatusAbnormal & STATUS_ABNORMAL_WARRIOR_SHAEHONE)
				{
					*ResErrorMessage << (int8)en_PersonalMessageType::PERSONAL_MESSAGE_STATUS_ABNORMAL_WARRIOR_SHAEHONE;

					StatusAbnormal &= STATUS_ABNORMAL_WARRIOR_SHAEHONE_MASK;

					wsprintf(ErrorMessage, L"��ȥ�� �����̻� �ɷ� ������ �� �����ϴ�.");

					ErrorMessageString = ErrorMessage;

					int16 ErrorMessageLen = (int16)(ErrorMessageString.length() * 2);
					*ResErrorMessage << ErrorMessageLen;
					ResErrorMessage->InsertData(ErrorMessageString.c_str(), ErrorMessageLen);
				}
				else if (StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ROOT || StatusAbnormal & STATUS_ABNORMAL_TAIOIST_ROOT)
				{
					*ResErrorMessage << (int8)en_PersonalMessageType::PERSONAL_MESSAGE_STATUS_ABNORMAL_WARRIOR_SHAEHONE;

					wsprintf(ErrorMessage, L"�ӹ� �����̻� �ɷ� ������ �� �����ϴ�.");

					ErrorMessageString = ErrorMessage;

					int16 ErrorMessageLen = (int16)(ErrorMessageString.length() * 2);
					*ResErrorMessage << ErrorMessageLen;
					ResErrorMessage->InsertData(ErrorMessageString.c_str(), ErrorMessageLen);
				}
				else if (StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ICE_WAVE)
				{
					*ResErrorMessage << (int8)en_PersonalMessageType::PERSONAL_MESSAGE_STATUS_ABNORMAL_SHAMAN_ICE_WAVE;
					wsprintf(ErrorMessage, L"�ñ� �ĵ� �����̻� �ɷ� ������ �� �����ϴ�.");
					
					ErrorMessageString = ErrorMessage;

					int16 ErrorMessageLen = (int16)(ErrorMessageString.length() * 2);
					*ResErrorMessage << ErrorMessageLen;
					ResErrorMessage->InsertData(ErrorMessageString.c_str(), ErrorMessageLen);
				}
				else if (StatusAbnormal & STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE)
				{
					*ResErrorMessage << (int8)en_PersonalMessageType::PERSONAL_MESSAGE_STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE;

					wsprintf(ErrorMessage, L"���� �����̻� �ɷ� ������ �� �����ϴ�.");

					ErrorMessageString = ErrorMessage;

					int16 ErrorMessageLen = (int16)(ErrorMessageString.length() * 2);
					*ResErrorMessage << ErrorMessageLen;
					ResErrorMessage->InsertData(ErrorMessageString.c_str(), ErrorMessageLen);
				}
			}		
		}		
		break;	
	case en_CommonErrorType::ERROR_STATUS_ABNORMAL_MELEE:
		wsprintf(ErrorMessage, L"�����̻� �ɷ� ���������� �� �� �����ϴ�.");
		break;
	case en_CommonErrorType::ERROR_STATUS_ABNORMAL_MAGIC:
		wsprintf(ErrorMessage, L"�����̻� �ɷ� ������ ���� �� �� �����ϴ�.");
		break;
	}	

	return ResErrorMessage;
}

CGameServerMessage* CGameServer::MakePacketStatusAbnormal(int64 PlayerId, en_GameObjectType ObjectType, en_MoveDir Dir, en_SkillType SkillType,  bool SetStatusAbnormal, int8 StatusAbnormal)
{
	CGameServerMessage* ResStatusAbnormal = CGameServerMessage::GameServerMessageAlloc();
	if (ResStatusAbnormal == nullptr)
	{
		return nullptr;
	}

	ResStatusAbnormal->Clear();

	*ResStatusAbnormal << (int16)en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_STATUS_ABNORMAL;
	*ResStatusAbnormal << PlayerId;
	*ResStatusAbnormal << (int16)ObjectType;
	*ResStatusAbnormal << (int8)Dir;
	*ResStatusAbnormal << (int16)SkillType;	
	*ResStatusAbnormal << SetStatusAbnormal;
	*ResStatusAbnormal << StatusAbnormal;

	return ResStatusAbnormal;
}

CGameServerMessage* CGameServer::MakePacketLogOut(int64 AccountID)
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

void CGameServer::SendPacketFieldOfView(vector<st_FieldOfViewInfo> FieldOfViewObject, CMessage* Message, CGameObject* Self)
{
	for (st_FieldOfViewInfo ObjectInfo : FieldOfViewObject)
	{
		CGameObject* FindObject = G_ObjectManager->Find(ObjectInfo.ObjectId, ObjectInfo.ObjectType);
		
		if (FindObject != nullptr && FindObject->_IsSendPacketTarget)
		{
			SendPacket(((CPlayer*)FindObject)->_SessionId, Message);
		}		
	}

	if (Self != nullptr)
	{
		SendPacket(((CPlayer*)Self)->_SessionId, Message);
	}
}

void CGameServer::SendPacketFieldOfView(CGameObject* Object, CMessage* Message)
{
	CChannel* Channel = G_ChannelManager->Find(1);

	// ���� ������
	vector<CSector*> AroundSectors = Object->_Channel->GetAroundSectors(Object->GetCellPosition(), 1);	

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

void CGameServer::SendPacketFieldOfView(st_Session* Session, CMessage* Message, bool SendMe)
{
	CPlayer* MyPlayer = G_ObjectManager->_PlayersArray[Session->MyPlayerIndex];

	CChannel* Channel = G_ChannelManager->Find(1);
	
	// ���� ���͵��� �����´�.
	vector<CSector*> Sectors = Channel->GetAroundSectors(MyPlayer->GetCellPosition(), 1);

	for (CSector* Sector : Sectors)
	{	
		for (CPlayer* Player : Sector->GetPlayers())
		{
			// �þ� ���� �ȿ� �ִ� �÷��̾ ã�´�
			int16 Distance = st_Vector2Int::Distance(MyPlayer->GetCellPosition(), Player->GetCellPosition());

			if (SendMe == true && Distance <= MyPlayer->_FieldOfViewDistance)
			{				
				SendPacket(Player->_SessionId, Message);
			}
			else if (SendMe == false && Distance <= MyPlayer->_FieldOfViewDistance)
			{
				if (Session->SessionId != Player->_SessionId)
				{
					SendPacket(Player->_SessionId, Message);
				}
			}
		}				
	}
}

void CGameServer::SkillMotionEndTimerJobCreate(CPlayer* Player, int64 SkillMotionEndTime, en_TimerJobType TimerJobType)
{
	// ��ų ��� �� �Ǵ�
	// ��ų ��� �� �ð� �Ŀ� Idle���·� �ٲٱ� ���� TimerJob ���
	st_TimerJob* SkillMotionEndTimerJob = _TimerJobMemoryPool->Alloc();
	SkillMotionEndTimerJob->TimerJobExecTick = GetTickCount64() + SkillMotionEndTime;
	SkillMotionEndTimerJob->SessionId = Player->_SessionId;
	SkillMotionEndTimerJob->TimerJobType = TimerJobType;
	SkillMotionEndTimerJob->TimerJobCancel = false;

	Player->_SkillJob = SkillMotionEndTimerJob;

	SkillMotionEndTimerJob->TimerJobMessage = nullptr;

	AcquireSRWLockExclusive(&_TimerJobLock);
	_TimerHeapJob->InsertHeap(SkillMotionEndTimerJob->TimerJobExecTick, SkillMotionEndTimerJob);
	ReleaseSRWLockExclusive(&_TimerJobLock);

	SetEvent(_TimerThreadWakeEvent);
}

void CGameServer::SkillCoolTimeTimerJobCreate(CPlayer* Player, int64 CastingTime, CSkill* CoolTimeSkill, en_TimerJobType TimerJobType, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex)
{
	// ��ų ��� �� �Ǵ�
	// 0.5�� �Ŀ� Idle���·� �ٲٱ� ���� TimerJob ���
	st_TimerJob* SkillEndTimerJob = _TimerJobMemoryPool->Alloc();
	SkillEndTimerJob->TimerJobExecTick = GetTickCount64() + CastingTime;
	SkillEndTimerJob->SessionId = Player->_SessionId;
	SkillEndTimerJob->TimerJobType = TimerJobType;
	// ���� ��� ���� 
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
	*SkillEndMessage << (int8)CoolTimeSkill->GetSkillInfo()->SkillMediumCategory;
	*SkillEndMessage << &CoolTimeSkill;

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