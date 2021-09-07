#include "pch.h"
#include "ObjectManager.h"

CObjectManager::CObjectManager()
{
	_PlayerMemoryPool = new CMemoryPoolTLS<CPlayer>();
	_SlimeMemoryPool = new CMemoryPoolTLS<CSlime>();
	_BearMemoryPool = new CMemoryPoolTLS<CBear>();
	_WeaponMemoryPool = new CMemoryPoolTLS<CWeapon>();
	_MaterialMemoryPool = new CMemoryPoolTLS<CMaterial>();
	_ConsumableMemoryPool = new CMemoryPoolTLS<CConsumable>();

	_MonsterObjectId = 10000;	
}

void CObjectManager::Add(CGameObject* AddObject, int32 ChannelId)
{
	// 채널 찾는다.
	CChannel* EnterChannel = G_ChannelManager->Find(ChannelId);
	if (EnterChannel == nullptr)
	{
		CRASH("ObjectManager Add EnterChannel이 nullptr");
	}
	
	switch (AddObject->_GameObjectInfo.ObjectType)
	{
		case en_GameObjectType::MELEE_PLAYER:
		case en_GameObjectType::MAGIC_PLAYER:
			{
				CPlayer* Player = (CPlayer*)AddObject;
				// 채널 입장
				EnterChannel->EnterChannel(AddObject);
				_Players.insert(pair<int64, CPlayer*>(AddObject->_GameObjectInfo.ObjectId, Player));
			}
			break;
		case en_GameObjectType::SLIME:
		case en_GameObjectType::BEAR:
			{
				vector<st_GameObjectInfo> SpawnMonster;

				CMonster* Monster = (CMonster*)AddObject;
				
				EnterChannel->EnterChannel(AddObject);
				_Monsters.insert(pair<int64, CMonster*>(AddObject->_GameObjectInfo.ObjectId, Monster));

				SpawnMonster.push_back(Monster->_GameObjectInfo);

				// 몬스터 추가하면 몬스터 주위 플레이어들에게 몬스터를 소환하라고 알림
				CMessage* ResSpawnPacket = GameServer->MakePacketResSpawn(1, SpawnMonster);
				GameServer->SendPacketAroundSector(Monster->GetCellPosition(), ResSpawnPacket);
				ResSpawnPacket->Free();
			} 
			break;		
		case en_GameObjectType::SLIME_GEL:
		case en_GameObjectType::BRONZE_COIN:
		case en_GameObjectType::LEATHER:
		case en_GameObjectType::SKILL_BOOK:
			{
				vector<st_GameObjectInfo> SpawnItem;
				
				CItem* Item = (CItem*)AddObject;								
				EnterChannel->EnterChannel(AddObject, &Item->_SpawnPosition);

				_Items.insert(pair<int64, CItem*>(AddObject->_GameObjectInfo.ObjectId,Item));

				SpawnItem.push_back(Item->_GameObjectInfo);

				Item->SetDestoryTime(1800);
				Item->ItemSetTarget(Item->_GameObjectInfo.OwnerObjectType, Item->_GameObjectInfo.OwnerObjectId);

				CMessage* ResSpawnPacket = GameServer->MakePacketResSpawn(1, SpawnItem);
				GameServer->SendPacketAroundSector(Item->GetCellPosition(), ResSpawnPacket);
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
	case en_GameObjectType::MELEE_PLAYER:
	case en_GameObjectType::MAGIC_PLAYER:
		_Players.erase(RemoveObject->_GameObjectInfo.ObjectId);
		break;;
	case en_GameObjectType::SLIME:
	case en_GameObjectType::BEAR:
		_Monsters.erase(RemoveObject->_GameObjectInfo.ObjectId);

		RemoveObject->_Channel->LeaveChannel(RemoveObject);
		break;	
	case en_GameObjectType::WEAPON:
	case en_GameObjectType::SLIME_GEL:
	case en_GameObjectType::BRONZE_COIN:
	case en_GameObjectType::LEATHER:
	case en_GameObjectType::SKILL_BOOK:
		_Items.erase(RemoveObject->_GameObjectInfo.ObjectId);

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
	case en_GameObjectType::MELEE_PLAYER:
	case en_GameObjectType::MAGIC_PLAYER:
	{
		auto FindIterator = _Players.find(ObjectId);
		if (FindIterator == _Players.end())
		{
			return nullptr;
		}

		return (*FindIterator).second;
	}
	case en_GameObjectType::SLIME:
	case en_GameObjectType::BEAR:
	{
		auto FindIterator = _Monsters.find(ObjectId);
		if (FindIterator == _Monsters.end())
		{
			return nullptr;
		}

		return (*FindIterator).second;
	}
	case en_GameObjectType::WEAPON:
	case en_GameObjectType::SLIME_GEL:
	case en_GameObjectType::LEATHER:
	case en_GameObjectType::BRONZE_COIN:
	case en_GameObjectType::SKILL_BOOK:
	{
		auto FindIterator = _Items.find(ObjectId);
		if (FindIterator == _Items.end())
		{
			return nullptr;
		}

		return (*FindIterator).second;
	}	
	default:
		return nullptr;
	}
}

CGameObject* CObjectManager::ObjectCreate(en_GameObjectType ObjectType)
{
	CGameObject* NewObject = nullptr;

	switch (ObjectType)
	{
	case en_GameObjectType::MELEE_PLAYER:
	case en_GameObjectType::MAGIC_PLAYER:
		NewObject = _PlayerMemoryPool->Alloc();
		break;
	case en_GameObjectType::SLIME:
		NewObject = _SlimeMemoryPool->Alloc();
		break;
	case en_GameObjectType::BEAR:
		NewObject = _BearMemoryPool->Alloc();
		break;
	case en_GameObjectType::WEAPON:
		NewObject = _WeaponMemoryPool->Alloc();
		break;
	case en_GameObjectType::SLIME_GEL:
	case en_GameObjectType::BRONZE_COIN:
	case en_GameObjectType::LEATHER:
		NewObject = _MaterialMemoryPool->Alloc();
		break;
	case en_GameObjectType::SKILL_BOOK:
		NewObject = _ConsumableMemoryPool->Alloc();
		break;
	}

	return NewObject;
}

void CObjectManager::ObjectReturn(en_GameObjectType ObjectType, CGameObject* ReturnObject)
{
	switch (ObjectType)
	{
	case en_GameObjectType::MELEE_PLAYER:
	case en_GameObjectType::MAGIC_PLAYER:
		_PlayerMemoryPool->Free((CPlayer*)ReturnObject);
		break;
	case en_GameObjectType::SLIME:
		_SlimeMemoryPool->Free((CSlime*)ReturnObject);
		break;
	case en_GameObjectType::BEAR:
		_BearMemoryPool->Free((CBear*)ReturnObject);
		break;
	case en_GameObjectType::WEAPON:
		_WeaponMemoryPool->Free((CWeapon*)ReturnObject);
		break;
	case en_GameObjectType::SLIME_GEL:
	case en_GameObjectType::BRONZE_COIN:
	case en_GameObjectType::LEATHER:
		_MaterialMemoryPool->Free((CMaterial*)ReturnObject);
		break;
	case en_GameObjectType::SKILL_BOOK:
		_ConsumableMemoryPool->Free((CConsumable*)ReturnObject);
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
		case en_GameObjectType::SLIME:
			NewMonster = (CSlime*)ObjectCreate(en_GameObjectType::SLIME);
			break;
		case en_GameObjectType::BEAR:
			NewMonster = (CBear*)ObjectCreate(en_GameObjectType::BEAR);
			break;		
		}

		NewMonster->_GameObjectInfo.ObjectId = _MonsterObjectId++;
		Add(NewMonster, ChannelId);
	}
}

void CObjectManager::ItemSpawn(int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnPosition, en_MonsterDataType MonsterDataType)
{
	// 아이템 생성 메세지 생성
	CMessage* ReqItemCreateMessage = CMessage::Alloc();

	ReqItemCreateMessage->Clear();

	*ReqItemCreateMessage << KillerId;
	*ReqItemCreateMessage << (int16)KillerObjectType;
	*ReqItemCreateMessage << SpawnPosition._X;
	*ReqItemCreateMessage << SpawnPosition._Y;
	*ReqItemCreateMessage << (int32)MonsterDataType;

	st_Job* ReqDBaseItemCreateJob = GameServer->_JobMemoryPool->Alloc();
	ReqDBaseItemCreateJob->Type = en_MESSAGE_TYPE::DATA_BASE_ITEM_CREATE;
	ReqDBaseItemCreateJob->SessionId = -1;
	ReqDBaseItemCreateJob->Session = nullptr;
	ReqDBaseItemCreateJob->Message = ReqItemCreateMessage;

	GameServer->_GameServerDataBaseThreadMessageQue.Enqueue(ReqDBaseItemCreateJob);
	SetEvent(GameServer->_DataBaseWakeEvent);	
}