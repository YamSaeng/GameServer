#pragma once
#include "ChannelManager.h"
#include "Player.h"
#include "Monster.h"
#include "GameServer.h"

class CObjectManager
{
private:
	map<int64, CPlayer*> _Players;
	map<int64, CMonster*> _Monsters;

	CMemoryPoolTLS<CPlayer>* _PlayerMemoryPool;
	CMemoryPoolTLS<CMonster>* _MonsterMemoryPool;	

	int32 _MonsterId;
public:
	CGameServer* GameServer;
	
	CObjectManager();
	//-----------------------------------------------------------
	// Object�� �߰��ϸ鼭 Object�� ������ ä���� ���̵� �޴´�.
	//-----------------------------------------------------------
	void Add(CGameObject* AddObject, int32 ChannelId);
	//-----------------------------------------------------------
	// Object�� �����ϸ鼭 Object�� ������ ä���� ���̵� �޴´�.
	//-----------------------------------------------------------
	bool Remove(CGameObject* RemoveObject, int32 _ChannelId);

	CGameObject* ObjectCreate(en_GameObjectType ObjectType);
	void ObjectReturn(en_GameObjectType ObjectType, CGameObject* ReturnObject);

	void MonsterSpawn(int32 MonsterCount, int32 ChannelId);
};

