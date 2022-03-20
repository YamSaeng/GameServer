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
class CSkill;

class CObjectManager
{
private:
	enum en_ObjectCount
	{
		PLAYER_MAX = 15000,
		MONSTER_MAX = 15000,
		ITEM_MAX = 15000,
		ENVIRONMENT_MAX = 15000
	};	
		
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

	CMemoryPoolTLS<CSkill>* _SkillMemoryPool;
	CMemoryPoolTLS<st_AttackSkillInfo>* _AttackSkillInfoMemoryPool;	
	CMemoryPoolTLS<st_HealSkillInfo>* _HealSkillInfoMemoryPool;
	CMemoryPoolTLS<st_BufSkillInfo>* _BufSkillInfoMemoryPool;
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
	// Object를 게임에 입장시켜준다. ( 입장할 채널의 아이디를 받는다 )
	//-----------------------------------------------------------
	void ObjectEnterGame(CGameObject* EnterGameObject, int32 ChannelId);
	//-----------------------------------------------------------
	// Object를 게임에서 퇴장시켜준다. ( 퇴장할 채널의 아이디를 받는다 )
	//-----------------------------------------------------------
	bool ObjectLeaveGame(CGameObject* LeaveGameObject, int32 ObjectIndex, int32 _ChannelId, bool IsObjectReturn = true);

	//----------------------------------------
	// 플레이어 배열 인덱스 반납
	//----------------------------------------
	void PlayerIndexReturn(int32 PlayerIndex);

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

	//--------------------
	// 스킬 생성
	//--------------------
	CSkill* SkillCreate();
	//------------------------------------
	// 스킬 반납
	//------------------------------------
	void SkillReturn(CSkill* ReturnSkill);

	//-----------------------------------------------
	// 스킬 정보 생성
	//-----------------------------------------------
	st_SkillInfo* SkillInfoCreate(en_SkillMediumCategory SkillMediumCategory);
	//-----------------------------------------------
	// 스킬 정보 반납
	//-----------------------------------------------
	void SkillInfoReturn(en_SkillMediumCategory SkillMediumCategory, st_SkillInfo* ReturnSkillInfo);

	//-----------------------------------
	// 맵에 설정한 오브젝트 스폰
	//-----------------------------------
	void MapObjectSpawn(int32 ChannelId);

	//------------------------------------------------------------------------------------------------
	// 아이템 스폰
	//------------------------------------------------------------------------------------------------
	void ItemSpawn(int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnPosition, en_GameObjectType SpawnItemOwnerType, en_ObjectDataType MonsterDataType);		
	
	//----------------------------------------------------------------------------
	// 오브젝트 스폰
	//----------------------------------------------------------------------------
	void ObjectSpawn(en_GameObjectType ObjectType, st_Vector2Int SpawnPosition);
};