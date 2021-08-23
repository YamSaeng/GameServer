#pragma once
#include "ChannelManager.h"
#include "Player.h"
#include "Bear.h"
#include "Slime.h"
#include "GameServer.h"

class CObjectManager
{
private:
	map<int64, CPlayer*> _Players;
	map<int64, CMonster*> _Monsters;

	CMemoryPoolTLS<CPlayer>* _PlayerMemoryPool;		
	CMemoryPoolTLS<CSlime>* _SlimeMemoryPool;
	CMemoryPoolTLS<CBear>* _BearMemoryPool;

	int32 _MonsterId;
public:
	CGameServer* GameServer;
	
	CObjectManager();
	//-----------------------------------------------------------
	// Object를 추가하면서 Object가 입장할 채널의 아이디를 받는다.
	//-----------------------------------------------------------
	void Add(CGameObject* AddObject, int32 ChannelId);
	//-----------------------------------------------------------
	// Object를 삭제하면서 Object가 퇴장할 채널의 아이디를 받는다.
	//-----------------------------------------------------------
	bool Remove(CGameObject* RemoveObject, int32 _ChannelId);

	CGameObject* ObjectCreate(en_GameObjectType ObjectType);
	void ObjectReturn(en_GameObjectType ObjectType, CGameObject* ReturnObject);

	void MonsterSpawn(int32 MonsterCount, int32 ChannelId, en_GameObjectType MonsterType);
};

