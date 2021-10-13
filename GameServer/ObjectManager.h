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
	// Object�� �߰��ϸ鼭 Object�� ������ ä���� ���̵� �޴´�.
	//-----------------------------------------------------------
	void Add(CGameObject* AddObject, int32 ChannelId);
	//-----------------------------------------------------------
	// Object�� �����ϸ鼭 Object�� ������ ä���� ���̵� �޴´�.
	//-----------------------------------------------------------
	bool Remove(CGameObject* RemoveObject, int32 _ChannelId, bool IsObjectReturn = true);

	//---------------
	// ������Ʈ ã��
	//---------------
	CGameObject* Find(int64 ObjectId, en_GameObjectType GameObjectType);

	//---------------
	// ������Ʈ ����
	//---------------
	CGameObject* ObjectCreate(en_GameObjectType ObjectType);
	//---------------
	// ������Ʈ �ݳ�
	//---------------
	void ObjectReturn(en_GameObjectType ObjectType, CGameObject* ReturnObject);

	//-----------------------------------------------------------------------------------
	// ���� ����
	//-----------------------------------------------------------------------------------
	void MonsterSpawn(int32 MonsterCount, int32 ChannelId, en_GameObjectType MonsterType);
	//------------------------------------------------------------------------------------------------
	// ������ ����
	//------------------------------------------------------------------------------------------------
	void ItemSpawn(int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnPosition, en_GameObjectType SpawnItemOwnerType, en_ObjectDataType MonsterDataType);		
	
	void EnvironmentSpawn(int32 ChannelId);
};

