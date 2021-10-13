#pragma once
#include "ChannelManager.h"
#include "Player.h"
#include "Bear.h"
#include "Slime.h"
#include "Item.h"
#include "GameServer.h"
#include "Environment.h"

class CObjectManager
{
private:
	map<int64, CItem*> _Items;
	map<int64, CPlayer*> _Players;
	map<int64, CMonster*> _Monsters;
	map<int64, CEnvironment*> _Environments;
		
	CMemoryPoolTLS<CPlayer>* _PlayerMemoryPool;		
	CMemoryPoolTLS<CSlime>* _SlimeMemoryPool;
	CMemoryPoolTLS<CBear>* _BearMemoryPool;
	CMemoryPoolTLS<CWeapon>* _WeaponMemoryPool;
	CMemoryPoolTLS<CMaterial>* _MaterialMemoryPool;
	CMemoryPoolTLS<CConsumable>* _ConsumableMemoryPool;

	CMemoryPoolTLS<CTree>* _TreeMemoryPool;
	CMemoryPoolTLS<CStone>* _StoneMemoryPool;
	
	int64 _GameServerObjectId;	
public:
	CGameServer* GameServer;
	
	CObjectManager();
	~CObjectManager();
	//-----------------------------------------------------------
	// Object를 추가하면서 Object가 입장할 채널의 아이디를 받는다.
	//-----------------------------------------------------------
	void Add(CGameObject* AddObject, int32 ChannelId);
	//-----------------------------------------------------------
	// Object를 삭제하면서 Object가 퇴장할 채널의 아이디를 받는다.
	//-----------------------------------------------------------
	bool Remove(CGameObject* RemoveObject, int32 _ChannelId, bool IsObjectReturn = true);

	//---------------
	// 오브젝트 찾기
	//---------------
	CGameObject* Find(int64 ObjectId, en_GameObjectType GameObjectType);

	//---------------
	// 오브젝트 생성
	//---------------
	CGameObject* ObjectCreate(en_GameObjectType ObjectType);
	//---------------
	// 오브젝트 반납
	//---------------
	void ObjectReturn(en_GameObjectType ObjectType, CGameObject* ReturnObject);

	//-----------------------------------------------------------------------------------
	// 몬스터 스폰
	//-----------------------------------------------------------------------------------
	void MonsterSpawn(int32 MonsterCount, int32 ChannelId, en_GameObjectType MonsterType);
	//------------------------------------------------------------------------------------------------
	// 아이템 스폰
	//------------------------------------------------------------------------------------------------
	void ItemSpawn(int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnPosition, en_GameObjectType SpawnItemOwnerType, en_ObjectDataType MonsterDataType);		
	
	void EnvironmentSpawn(int32 ChannelId);
};

