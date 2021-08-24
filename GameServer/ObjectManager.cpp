#include "pch.h"
#include "ObjectManager.h"

CObjectManager::CObjectManager()
{
	_PlayerMemoryPool = new CMemoryPoolTLS<CPlayer>(0);
	_SlimeMemoryPool = new CMemoryPoolTLS<CSlime>(0);
	_BearMemoryPool = new CMemoryPoolTLS<CBear>(0);
	_WeaponMemoryPool = new CMemoryPoolTLS<CWeapon>(0);
	_MaterialMemoryPool = new CMemoryPoolTLS<CMaterial>(0);

	_MonsterId = 10000;
	_ItemId = 100000;
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
		case en_GameObjectType::PLAYER:
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
			{
				vector<st_GameObjectInfo> SpawnItem;
				
				CItem* Item = (CItem*)AddObject;

				EnterChannel->EnterChannel(AddObject, &Item->_OwnerPosition);

				_Items.insert(pair<int64, CItem*>(AddObject->_GameObjectInfo.ObjectId,Item));

				SpawnItem.push_back(Item->_GameObjectInfo);

				CMessage* ResSpawnPacket = GameServer->MakePacketResSpawn(1, SpawnItem);
				GameServer->SendPacketAroundSector(Item->_OwnerPosition, ResSpawnPacket);
				ResSpawnPacket->Free();
			}
			break;
	}
}

bool CObjectManager::Remove(CGameObject* RemoveObject, int32 _ChannelId)
{
	bool RemoveSuccess = false;

	switch (RemoveObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::PLAYER:
		_Players.erase(RemoveObject->_GameObjectInfo.ObjectId);
		break;;
	case en_GameObjectType::SLIME:
	case en_GameObjectType::BEAR:
		_Monsters.erase(RemoveObject->_GameObjectInfo.ObjectId);			
		break;	
	case en_GameObjectType::WEAPON:
	case en_GameObjectType::SLIME_GEL:
	case en_GameObjectType::BRONZE_COIN:
		_Items.erase(RemoveObject->_GameObjectInfo.ObjectId);
		break;
	}

	return RemoveSuccess;
}

CGameObject* CObjectManager::ObjectCreate(en_GameObjectType ObjectType)
{
	CGameObject* NewObject = nullptr;

	switch (ObjectType)
	{
	case en_GameObjectType::PLAYER:
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
		NewObject = _MaterialMemoryPool->Alloc();
		break;
	}

	return NewObject;
}

void CObjectManager::ObjectReturn(en_GameObjectType ObjectType, CGameObject* ReturnObject)
{
	switch (ObjectType)
	{
	case en_GameObjectType::PLAYER:
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
		_MaterialMemoryPool->Free((CMaterial*)ReturnObject);
		break;
	}

	Remove(ReturnObject, 1);
}

void CObjectManager::MonsterSpawn(int32 MonsterCount, int32 ChannelId, en_GameObjectType MonsterType)
{
	for (int32 i = 0; i < MonsterCount; i++)
	{
		CMonster* NewMonster = nullptr;
		switch (MonsterType)
		{
		case SLIME:
			NewMonster = (CSlime*)ObjectCreate(en_GameObjectType::SLIME);
			break;
		case BEAR:
			NewMonster = (CBear*)ObjectCreate(en_GameObjectType::BEAR);
			break;		
		}

		NewMonster->_GameObjectInfo.ObjectId = _MonsterId++;
		Add(NewMonster, ChannelId);
	}
}

void CObjectManager::ItemSpawn(int32 ChannelId, st_Vector2Int OwnerPosition, int64 KillerId, st_ItemInfo ItemInfo, en_GameObjectType ItemType)
{
	for (int32 i = 0; i < ItemInfo.Count; i++)
	{
		CItem* NewItem = nullptr;
		switch (ItemType)
		{		
		case en_GameObjectType::WEAPON:
			break;
		case en_GameObjectType::SLIME_GEL:
			NewItem = (CItem*)ObjectCreate(en_GameObjectType::SLIME_GEL);
			break;		
		case en_GameObjectType::BRONZE_COIN:
			NewItem = (CItem*)ObjectCreate(en_GameObjectType::BRONZE_COIN);
			break;
		}
			
		NewItem->_ItemInfo = ItemInfo;		
		NewItem->_GameObjectInfo.ObjectType = ItemType;		
		NewItem->_GameObjectInfo.ObjectId = _ItemId++;		
		NewItem->_GameObjectInfo.ObjectName = ItemInfo.ItemName;
		NewItem->_GameObjectInfo.OwnerObjectId = KillerId;

		NewItem->_OwnerPosition = OwnerPosition;
		Add(NewItem, ChannelId);
	}
}
