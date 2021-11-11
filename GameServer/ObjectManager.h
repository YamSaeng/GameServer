#pragma once
#include "ChannelManager.h"
#include "Player.h"
#include "Bear.h"
#include "Slime.h"
#include "GameServer.h"
#include "Environment.h"

class CItem;
class CWeapon;
class CArmor;
class CConsumable;
class CMaterial;

class CObjectManager
{
private:
	enum en_ObjectCount
	{
		PLAYER_MAX = 5000,
		MONSTER_MAX = 5000,
		ITEM_MAX = 5000,
		ENVIRONMENT_MAX = 5000
	};

	map<int64, CItem*> _Items;
	map<int64, CPlayer*> _Players;
	map<int64, CMonster*> _Monsters;
	map<int64, CEnvironment*> _Environments;
		
	CMemoryPoolTLS<CPlayer>* _PlayerMemoryPool;		
	CMemoryPoolTLS<CSlime>* _SlimeMemoryPool;
	CMemoryPoolTLS<CBear>* _BearMemoryPool;
	
	CMemoryPoolTLS<CItem>* _ItemMemoryPool;
	CMemoryPoolTLS<CWeapon>* _WeaponMemoryPool;
	CMemoryPoolTLS<CArmor>* _ArmorMemoryPool;
	CMemoryPoolTLS<CConsumable>* _ConsumableMemoryPool;
	CMemoryPoolTLS<CMaterial>* _MaterialMemoryPool;	

	CMemoryPoolTLS<CTree>* _TreeMemoryPool;
	CMemoryPoolTLS<CStone>* _StoneMemoryPool;
	
	int64 _GameServerObjectId;		
public:
	CGameServer* GameServer;

	CPlayer* _PlayersArray[PLAYER_MAX];
	CMonster* _MonstersArray[MONSTER_MAX];
	CItem* _ItemsArray[ITEM_MAX];
	CEnvironment* _EnvironmentsArray[ENVIRONMENT_MAX];

	CLockFreeStack<int32> _PlayersArrayIndexs;
	CLockFreeStack<int32> _MonstersArrayIndexs;
	CLockFreeStack<int32> _ItemsArrayIndexs;
	CLockFreeStack<int32> _EnvironmentsArrayIndexs;

	CObjectManager();
	~CObjectManager();
	//-----------------------------------------------------------
	// Object�� ���ӿ� ��������ش�. ( ������ ä���� ���̵� �޴´� )
	//-----------------------------------------------------------
	void ObjectEnterGame(CGameObject* EnterGameObject, int32 ChannelId);
	//-----------------------------------------------------------
	// Object�� ���ӿ��� ��������ش�. ( ������ ä���� ���̵� �޴´� )
	//-----------------------------------------------------------
	bool ObjectLeaveGame(CGameObject* LeaveGameObject, int32 ObjectIndex, int32 _ChannelId, bool IsObjectReturn = true);

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

	//-----------------------------------
	// �ʿ� ������ ������Ʈ ����
	//-----------------------------------
	void MapObjectSpawn(int32 ChannelId);

	//------------------------------------------------------------------------------------------------
	// ������ ����
	//------------------------------------------------------------------------------------------------
	void ItemSpawn(int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnPosition, en_GameObjectType SpawnItemOwnerType, en_ObjectDataType MonsterDataType);		
	
	//----------------------------------------------------------------------------
	// ������Ʈ ����
	//----------------------------------------------------------------------------
	void ObjectSpawn(en_GameObjectType ObjectType, st_Vector2Int SpawnPosition);
};