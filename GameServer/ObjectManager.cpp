#include "pch.h"
#include "ObjectManager.h"
#include "Environment.h"
#include "GameServerMessage.h"

CObjectManager::CObjectManager()
{
	_PlayerMemoryPool = new CMemoryPoolTLS<CPlayer>();
	_SlimeMemoryPool = new CMemoryPoolTLS<CSlime>();
	_BearMemoryPool = new CMemoryPoolTLS<CBear>();
	_WeaponMemoryPool = new CMemoryPoolTLS<CWeapon>();
	_MaterialMemoryPool = new CMemoryPoolTLS<CMaterial>();
	_ConsumableMemoryPool = new CMemoryPoolTLS<CConsumable>();
	_TreeMemoryPool = new CMemoryPoolTLS<CTree>();
	_StoneMemoryPool = new CMemoryPoolTLS<CStone>();

	_GameServerObjectId = 10000;	
}

CObjectManager::~CObjectManager()
{
	delete _PlayerMemoryPool;
	delete _SlimeMemoryPool;
	delete _BearMemoryPool;
	delete _WeaponMemoryPool;
	delete _MaterialMemoryPool;
	delete _ConsumableMemoryPool;
	delete _TreeMemoryPool;
	delete _StoneMemoryPool;
}

void CObjectManager::Add(CGameObject* AddObject, int32 ChannelId)
{
	bool IsEnterChannel = true;

	// 채널 찾는다.
	CChannel* EnterChannel = G_ChannelManager->Find(ChannelId);
	if (EnterChannel == nullptr)
	{
		CRASH("ObjectManager Add EnterChannel이 nullptr");
	}
	
	switch (AddObject->_GameObjectInfo.ObjectType)
	{
		case en_GameObjectType::OBJECT_MELEE_PLAYER:
		case en_GameObjectType::OBJECT_MAGIC_PLAYER:
			{
				CPlayer* Player = (CPlayer*)AddObject;
				// 채널 입장
				EnterChannel->EnterChannel(AddObject);
				_Players.insert(pair<int64, CPlayer*>(AddObject->_GameObjectInfo.ObjectId, Player));
			}
			break;
		case en_GameObjectType::OBJECT_SLIME:
		case en_GameObjectType::OBJECT_BEAR:
			{
				vector<st_GameObjectInfo> SpawnMonster;

				CMonster* Monster = (CMonster*)AddObject;
				
				EnterChannel->EnterChannel(AddObject);
				_Monsters.insert(pair<int64, CMonster*>(AddObject->_GameObjectInfo.ObjectId, Monster));

				SpawnMonster.push_back(Monster->_GameObjectInfo);

				// 몬스터 추가하면 몬스터 주위 플레이어들에게 몬스터를 소환하라고 알림
				CMessage* ResSpawnPacket = GameServer->MakePacketResObjectSpawn(1, SpawnMonster);
				GameServer->SendPacketAroundSector(Monster->GetCellPosition(), ResSpawnPacket);
				ResSpawnPacket->Free();
			} 
			break;		
		case en_GameObjectType::OBJECT_ITEM_SLIME_GEL:
		case en_GameObjectType::OBJECT_ITEM_BRONZE_COIN:
		case en_GameObjectType::OBJECT_ITEM_LEATHER:
		case en_GameObjectType::OBJECT_ITEM_SKILL_BOOK:
		case en_GameObjectType::OBJECT_ITEM_WOOD_LOG:
		case en_GameObjectType::OBJECT_ITEM_STONE:
			{
				vector<st_GameObjectInfo> SpawnItem;
				
				CItem* Item = (CItem*)AddObject;				
												
				IsEnterChannel = EnterChannel->EnterChannel(AddObject, &Item->_SpawnPosition);
				if (IsEnterChannel == true)
				{
					// 중복되지 않은 아이템 스폰
					_Items.insert(pair<int64, CItem*>(AddObject->_GameObjectInfo.ObjectId, Item));

					SpawnItem.push_back(Item->_GameObjectInfo);

					Item->SetDestoryTime(1800);
					Item->ItemSetTarget(Item->_GameObjectInfo.OwnerObjectType, Item->_GameObjectInfo.OwnerObjectId);

					CMessage* ResSpawnPacket = GameServer->MakePacketResObjectSpawn(1, SpawnItem);
					GameServer->SendPacketAroundSector(Item->GetCellPosition(), ResSpawnPacket);
					ResSpawnPacket->Free();
				}
				else
				{
					// 중복된 아이템의 경우 메모리에 반납
					ObjectReturn(Item->_GameObjectInfo.ObjectType, Item);
				}				
			}
			break;
		case en_GameObjectType::OBJECT_STONE:
		case en_GameObjectType::OBJECT_TREE:
			{
				vector<st_GameObjectInfo> SpawnEnvironment;
					
				CEnvironment* Environment = (CEnvironment*)AddObject;
				EnterChannel->EnterChannel(AddObject, &Environment->_SpawnPosition);

				_Environments.insert(pair<int64, CEnvironment*>(Environment->_GameObjectInfo.ObjectId,Environment));

				SpawnEnvironment.push_back(Environment->_GameObjectInfo);
				CMessage* ResSpawnPacket = GameServer->MakePacketResObjectSpawn(1, SpawnEnvironment);
				GameServer->SendPacketAroundSector(Environment->GetCellPosition(), ResSpawnPacket);
				ResSpawnPacket->Free();
			}
			break;
	}
}

bool CObjectManager::Remove(CGameObject* RemoveObject, int32 _ChannelId, bool IsObjectReturn)
{
	bool RemoveSuccess = false;
	
	// 타입에 따라 관리당하고 있는 자료구조에서 자신을 삭제
	// 채널에서 삭제
	switch (RemoveObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_MELEE_PLAYER:
	case en_GameObjectType::OBJECT_MAGIC_PLAYER:
		_Players.erase(RemoveObject->_GameObjectInfo.ObjectId);
		break;;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		_Monsters.erase(RemoveObject->_GameObjectInfo.ObjectId);

		RemoveObject->_Channel->LeaveChannel(RemoveObject);
		break;	
	case en_GameObjectType::OBJECT_ITEM_WEAPON:
	case en_GameObjectType::OBJECT_ITEM_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_STONE:
		_Items.erase(RemoveObject->_GameObjectInfo.ObjectId);

		RemoveObject->_Channel->LeaveChannel(RemoveObject);
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		_Environments.erase(RemoveObject->_GameObjectInfo.ObjectId);

		RemoveObject->_Channel->LeaveChannel(RemoveObject);
		break;
	}

	if (IsObjectReturn == true)
	{
		// 오브젝트 메모리 풀에 반환
		ObjectReturn(RemoveObject->_GameObjectInfo.ObjectType, RemoveObject);
	}

	return RemoveSuccess;
}

CGameObject* CObjectManager::Find(int64 ObjectId, en_GameObjectType GameObjectType)
{	
	switch (GameObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:
	case en_GameObjectType::OBJECT_MELEE_PLAYER:
	case en_GameObjectType::OBJECT_MAGIC_PLAYER:
	{
		auto FindIterator = _Players.find(ObjectId);
		if (FindIterator == _Players.end())
		{
			return nullptr;
		}

		return (*FindIterator).second;
	}
	case en_GameObjectType::OBJECT_MONSTER:	
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
	{
		auto FindIterator = _Monsters.find(ObjectId);
		if (FindIterator == _Monsters.end())
		{
			return nullptr;
		}

		return (*FindIterator).second;
	}
	case en_GameObjectType::OBJECT_ITEM:	
	case en_GameObjectType::OBJECT_ITEM_WEAPON:
	case en_GameObjectType::OBJECT_ITEM_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_SLIVER_COIN:
	case en_GameObjectType::OBJECT_ITEM_GOLD_COIN:
	case en_GameObjectType::OBJECT_ITEM_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_STONE:
	{
		auto FindIterator = _Items.find(ObjectId);
		if (FindIterator == _Items.end())
		{
			return nullptr;
		}

		return (*FindIterator).second;
	}		
	case en_GameObjectType::OBJECT_ENVIRONMENT:	
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
	{
		auto FindIterator = _Environments.find(ObjectId);
		if (FindIterator == _Environments.end())
		{
			return nullptr;
		}

		return (*FindIterator).second;
	}
		break;
	default:
		return nullptr;
	}
}

CGameObject* CObjectManager::ObjectCreate(en_GameObjectType ObjectType)
{
	CGameObject* NewObject = nullptr;

	switch (ObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:	
		NewObject = _PlayerMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_SLIME:
		NewObject = _SlimeMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_BEAR:
		NewObject = _BearMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON:
		NewObject = _WeaponMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_ITEM_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_STONE:
		NewObject = _MaterialMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_ITEM_SKILL_BOOK:
		NewObject = _ConsumableMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_STONE:
		NewObject = _StoneMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_TREE:
		NewObject = _TreeMemoryPool->Alloc();
		break;
	}

	return NewObject;
}

void CObjectManager::ObjectReturn(en_GameObjectType ObjectType, CGameObject* ReturnObject)
{
	switch (ObjectType)
	{
	case en_GameObjectType::OBJECT_MELEE_PLAYER:
	case en_GameObjectType::OBJECT_MAGIC_PLAYER:
		_PlayerMemoryPool->Free((CPlayer*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_SLIME:
		_SlimeMemoryPool->Free((CSlime*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_BEAR:
		_BearMemoryPool->Free((CBear*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON:
		_WeaponMemoryPool->Free((CWeapon*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_ITEM_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_STONE:
		_MaterialMemoryPool->Free((CMaterial*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_ITEM_SKILL_BOOK:
		_ConsumableMemoryPool->Free((CConsumable*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_STONE:
		_StoneMemoryPool->Free((CStone*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_TREE:
		_TreeMemoryPool->Free((CTree*)ReturnObject);
		break;
	}		
}

void CObjectManager::MonsterSpawn(int32 MonsterCount, int32 ChannelId, en_GameObjectType MonsterType)
{
	for (int32 i = 0; i < MonsterCount; i++)
	{
		CMonster* NewMonster = nullptr;
		switch (MonsterType)
		{
		case en_GameObjectType::OBJECT_SLIME:
			NewMonster = (CSlime*)ObjectCreate(en_GameObjectType::OBJECT_SLIME);
			break;
		case en_GameObjectType::OBJECT_BEAR:
			NewMonster = (CBear*)ObjectCreate(en_GameObjectType::OBJECT_BEAR);
			break;		
		}

		NewMonster->_GameObjectInfo.ObjectId = _GameServerObjectId++;
		Add(NewMonster, ChannelId);
	}
}

void CObjectManager::ItemSpawn(int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnPosition, en_GameObjectType SpawnItemOwnerType, en_ObjectDataType MonsterDataType)
{
	// 아이템 생성 메세지 생성
	CGameServerMessage* ReqItemCreateMessage = CGameServerMessage::GameServerMessageAlloc();

	ReqItemCreateMessage->Clear();

	*ReqItemCreateMessage << KillerId;
	*ReqItemCreateMessage << (int16)KillerObjectType;
	*ReqItemCreateMessage << SpawnPosition._X;
	*ReqItemCreateMessage << SpawnPosition._Y;
	*ReqItemCreateMessage << (int16)SpawnItemOwnerType;
	*ReqItemCreateMessage << (int32)MonsterDataType;

	st_Job* ReqDBaseItemCreateJob = GameServer->_JobMemoryPool->Alloc();
	ReqDBaseItemCreateJob->Type = en_JobType::DATA_BASE_ITEM_CREATE;
	ReqDBaseItemCreateJob->SessionId = -1;
	ReqDBaseItemCreateJob->Session = nullptr;
	ReqDBaseItemCreateJob->Message = ReqItemCreateMessage;

	GameServer->_GameServerDataBaseThreadMessageQue.Enqueue(ReqDBaseItemCreateJob);
	SetEvent(GameServer->_DataBaseWakeEvent);	
}

void CObjectManager::EnvironmentSpawn(int32 ChannelId)
{
	CChannel* Channel = G_ChannelManager->Find(ChannelId);
	if (Channel == nullptr)
	{
		return;
	}

	int32 SizeX = Channel->_Map->_SizeX;
	int32 SizeY = Channel->_Map->_SizeY;

	for (int Y = 0; Y < SizeY; Y++)
	{
		for (int X = 0; X < SizeX; X++)
		{
			CEnvironment* NewEnvironment = nullptr;

			switch (Channel->_Map->_CollisionMapInfos[Y][X])
			{
			case en_TileMapEnvironment::TILE_MAP_TREE:
				NewEnvironment = (CTree*)ObjectCreate(en_GameObjectType::OBJECT_TREE);
				break;
			case en_TileMapEnvironment::TILE_MAP_STONE:
				NewEnvironment = (CTree*)ObjectCreate(en_GameObjectType::OBJECT_STONE);
				break;			
			}	

			if (NewEnvironment != nullptr)
			{
				int SpawnPositionX = X + Channel->_Map->_Left;
				int SpawnPositionY = Channel->_Map->_Down - Y;

				st_Vector2Int NewPosition;				
				NewPosition._Y = SpawnPositionY;
				NewPosition._X = SpawnPositionX;

				NewEnvironment->_GameObjectInfo.ObjectId = _GameServerObjectId++;
				NewEnvironment->_SpawnPosition = NewPosition;
				Add(NewEnvironment, ChannelId);
			}
		}
	}	
}
