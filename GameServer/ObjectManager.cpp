#include "pch.h"
#include "ObjectManager.h"


CObjectManager::CObjectManager()
{
	_PlayerMemoryPool = new CMemoryPoolTLS<CPlayer>(0);
	_SlimeMemoryPool = new CMemoryPoolTLS<CSlime>(0);
	_BearMemoryPool = new CMemoryPoolTLS<CBear>(0);

	_MonsterId = 10000;
}

void CObjectManager::Add(CGameObject* AddObject, int32 ChannelId)
{
	// 채널 찾는다.
	CChannel* EnterChannel = G_ChannelManager->Find(ChannelId);
	if (EnterChannel == nullptr)
	{
		CRASH("ObjectManager Add EnterChannel이 nullptr");
	}

	// 채널 입장
	EnterChannel->EnterChannel(AddObject);

	switch (AddObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::PLAYER:
	{
		CPlayer* Player = (CPlayer*)AddObject;
		_Players.insert(pair<int64, CPlayer*>(AddObject->_GameObjectInfo.ObjectId, Player));
	}
	break;
	case en_GameObjectType::SLIME:
	{
		vector<st_GameObjectInfo> SpawnMonster;

		CMonster* Monster = (CMonster*)AddObject;
		_Monsters.insert(pair<int64, CMonster*>(AddObject->_GameObjectInfo.ObjectId, Monster));

		SpawnMonster.push_back(Monster->_GameObjectInfo);

		// 몬스터 추가하면 몬스터 주위 플레이어들에게 몬스터를 소환하라고 알림
		CMessage* ResSpawnPacket = GameServer->MakePacketResSpawn(1, SpawnMonster);
		GameServer->SendPacketAroundSector(Monster->GetCellPosition(), ResSpawnPacket);
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
		_Monsters.erase(RemoveObject->_GameObjectInfo.ObjectId);
		break;
	case en_GameObjectType::BEAR:
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
		Remove(ReturnObject, 1);
		break;
	case en_GameObjectType::SLIME:
		_SlimeMemoryPool->Free((CSlime*)ReturnObject);
		Remove(ReturnObject, 1);
		break;
	case en_GameObjectType::BEAR:
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
		case SLIME:
			NewMonster = (CSlime*)ObjectCreate(en_GameObjectType::SLIME);
			break;
		case BEAR:
			break;		
		}

		NewMonster->_GameObjectInfo.ObjectId = _MonsterId++;
		Add(NewMonster, ChannelId);
	}
}