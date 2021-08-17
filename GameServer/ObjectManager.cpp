#include "pch.h"
#include "ObjectManager.h"

CObjectManager::CObjectManager()
{
	_PlayerMemoryPool = new CMemoryPoolTLS<CPlayer>(0);
	_MonsterMemoryPool = new CMemoryPoolTLS<CMonster>(0);	

	_MonsterId = 10000;
}

void CObjectManager::Add(CGameObject* AddObject, int32 ChannelId)
{		
	// ä�� ã�´�.
	CChannel* EnterChannel = G_ChannelManager->Find(ChannelId);
	if (EnterChannel == nullptr)
	{
		CRASH("ObjectManager Add EnterChannel�� nullptr");
	}

	// ä�� ����
	EnterChannel->EnterChannel(AddObject);

	switch (AddObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::PLAYER:
	{
		CPlayer* Player = (CPlayer*)AddObject;
		_Players.insert(pair<int64, CPlayer*>(AddObject->_GameObjectInfo.ObjectId, Player));					
	}
	break;
	case en_GameObjectType::MONSTER:
	{
		vector<st_GameObjectInfo> SpawnMonster;

		CMonster* Monster = (CMonster*)AddObject;
		_Monsters.insert(pair<int64, CMonster*>(AddObject->_GameObjectInfo.ObjectId, Monster));

		SpawnMonster.push_back(Monster->_GameObjectInfo);

		// ���� �߰��ϸ� ���� ���� �÷��̾�鿡�� ���͸� ��ȯ�϶�� �˸�
		CMessage* ResSpawnPacket = GameServer->MakePacketResSpawn(1, SpawnMonster);
		GameServer->SendPacketAroundSector(Monster->GetCellPosition(),ResSpawnPacket);
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
	case en_GameObjectType::MONSTER:
		_Monsters.erase(RemoveObject->_GameObjectInfo.ObjectId);
		break;
	}
}

CGameObject* CObjectManager::ObjectCreate(en_GameObjectType ObjectType)
{
	CGameObject* NewObject = nullptr;

	switch (ObjectType)
	{	
	case PLAYER:		
		NewObject = _PlayerMemoryPool->Alloc();
		break;
	case MONSTER:
		NewObject = _MonsterMemoryPool->Alloc();
		break;	
	}

	return NewObject;
}

void CObjectManager::ObjectReturn(en_GameObjectType ObjectType, CGameObject* ReturnObject)
{
	switch (ObjectType)
	{	
	case PLAYER:
		_PlayerMemoryPool->Free((CPlayer*)ReturnObject);
		break;
	case MONSTER:
		_MonsterMemoryPool->Free((CMonster*)ReturnObject);
		break;	
	}
}

void CObjectManager::MonsterSpawn(int32 MonsterCount, int32 ChannelId)
{
	for (int32 i = 0; i < MonsterCount; i++)
	{
		CMonster* NewMonster = (CMonster*)ObjectCreate(en_GameObjectType::MONSTER);		
		NewMonster->_GameObjectInfo.ObjectId = _MonsterId++;		
		Add(NewMonster, ChannelId);
	}	
}