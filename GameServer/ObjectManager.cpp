#include "pch.h"
#include "ObjectManager.h"
#include "Environment.h"
#include "GameServerMessage.h"
#include "Item.h"
#include "Skill.h"

CObjectManager::CObjectManager()
{
	_PlayerMemoryPool = new CMemoryPoolTLS<CPlayer>();
	_SlimeMemoryPool = new CMemoryPoolTLS<CSlime>();
	_BearMemoryPool = new CMemoryPoolTLS<CBear>();

	_ItemMemoryPool = new CMemoryPoolTLS<CItem>();
	_WeaponMemoryPool = new CMemoryPoolTLS<CWeapon>();
	_ArmorMemoryPool = new CMemoryPoolTLS<CArmor>();
	_MaterialMemoryPool = new CMemoryPoolTLS<CMaterial>();
	_ConsumableMemoryPool = new CMemoryPoolTLS<CConsumable>();
	
	_TreeMemoryPool = new CMemoryPoolTLS<CTree>();
	_StoneMemoryPool = new CMemoryPoolTLS<CStone>();

	_SkillMemoryPool = new CMemoryPoolTLS<CSkill>();

	_AttackSkillInfoMemoryPool = new CMemoryPoolTLS<st_AttackSkillInfo>();
	_TacTicSkillInfoMemoryPool = new CMemoryPoolTLS<st_TacTicSkillInfo>();
	_HealSkillInfoMemoryPool = new CMemoryPoolTLS<st_HealSkillInfo>();
	_BufSkillInfoMemoryPool = new CMemoryPoolTLS<st_BufSkillInfo>();

	_GameServerObjectId = 10000;

	// 오브젝트 매니저가 소유중인 플레이어, 몬스터, 아이템 미리 할당해서 보관
	for (int PlayerCount = PLAYER_MAX - 1; PlayerCount >= 0; --PlayerCount)
	{
		_PlayersArray[PlayerCount] = (CPlayer*)ObjectCreate(en_GameObjectType::OBJECT_PLAYER);
		_PlayersArrayIndexs.Push(PlayerCount);
	}

	for (int MonsterCount = MONSTER_MAX - 1; MonsterCount >= 0; --MonsterCount)
	{
		_MonstersArray[MonsterCount] = nullptr;
		_MonstersArrayIndexs.Push(MonsterCount);
	}

	for (int ItemCount = ITEM_MAX - 1; ItemCount >= 0; --ItemCount)
	{
		_ItemsArray[ItemCount] = nullptr;
		_ItemsArrayIndexs.Push(ItemCount);
	}
	
	for (int EnvironmentCount = ENVIRONMENT_MAX - 1; EnvironmentCount >= 0; --EnvironmentCount)
	{
		_EnvironmentsArray[EnvironmentCount] = nullptr;
		_EnvironmentsArrayIndexs.Push(EnvironmentCount);
	}
}

CObjectManager::~CObjectManager()
{
	delete _PlayerMemoryPool;
	delete _SlimeMemoryPool;
	delete _BearMemoryPool;
	delete _WeaponMemoryPool;
	delete _MaterialMemoryPool;
	delete _ConsumableMemoryPool;
	delete _TreeMemoryPool;
	delete _StoneMemoryPool;
}

void CObjectManager::ObjectEnterGame(CGameObject* EnterGameObject, int32 ChannelId)
{
	bool IsEnterChannel = true;

	// 채널 찾는다.
	CChannel* EnterChannel = G_ChannelManager->Find(ChannelId);
	if (EnterChannel == nullptr)
	{
		CRASH("ObjectManager Add EnterChannel이 nullptr");
	}		

	switch (EnterGameObject->_GameObjectInfo.ObjectType)
	{
		case en_GameObjectType::OBJECT_PLAYER_DUMMY:
			{
				CPlayer* Player = (CPlayer*)EnterGameObject;
				
				Player->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;

				EnterChannel->EnterChannel(EnterGameObject);
			}
			break;
		case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
		case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
		case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
		case en_GameObjectType::OBJECT_THIEF_PLAYER:
		case en_GameObjectType::OBJECT_ARCHER_PLAYER:
			{
				CPlayer* Player = (CPlayer*)EnterGameObject;

				Player->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;
				
				// 채널 입장
				EnterChannel->EnterChannel(EnterGameObject, &Player->_SpawnPosition);				
			}
			break;
		case en_GameObjectType::OBJECT_SLIME:
		case en_GameObjectType::OBJECT_BEAR:
			{				
				vector<CGameObject*> SpawnMonster;				

				CMonster* Monster = (CMonster*)EnterGameObject;				

				// 인덱스 가져오기
				_MonstersArrayIndexs.Pop(&EnterGameObject->_ObjectManagerArrayIndex);				
				// 배열에 저장
				_MonstersArray[EnterGameObject->_ObjectManagerArrayIndex] = Monster;

				// 몬스터 주위 오브젝트 정보 저장
				Monster->_FieldOfViewPlayers = EnterChannel->GetFieldOfViewPlayer(Monster, Monster->_FieldOfViewDistance);

				// 채널 입장
				EnterChannel->EnterChannel(EnterGameObject, &Monster->_SpawnPosition);			

				SpawnMonster.push_back(Monster);								

				// 몬스터 추가하면 몬스터 주위 플레이어들에게 몬스터를 소환하라고 알림
				CMessage* ResSpawnPacket = GameServer->MakePacketResObjectSpawn(1, SpawnMonster);				
				GameServer->SendPacketFieldOfView(Monster, ResSpawnPacket);
				ResSpawnPacket->Free();

				// 몬스터 소환할때 소환 이펙트 출력
				CMessage* ResEffectPacket = GameServer->MakePacketEffect(Monster->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_OBJECT_SPAWN, 0.5f);
				GameServer->SendPacketFieldOfView(Monster, ResEffectPacket);
				ResEffectPacket->Free();
			} 
			break;		
		case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
		case en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR:
		case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
		case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
		case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:		
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
			{
				vector<CGameObject*> SpawnItem;
				
				CItem* Item = (CItem*)EnterGameObject;								

				IsEnterChannel = EnterChannel->EnterChannel(EnterGameObject, &Item->_SpawnPosition);
				if (IsEnterChannel == true)
				{
					// 중복되지 않은 아이템 스폰
					// 인덱스 가져오기	
					_ItemsArrayIndexs.Pop(&EnterGameObject->_ObjectManagerArrayIndex);
					// 배열에 저장
					_ItemsArray[EnterGameObject->_ObjectManagerArrayIndex] = Item;

					SpawnItem.push_back(Item);

					Item->SetDestoryTime(1800);
					Item->ItemSetTarget(Item->_GameObjectInfo.OwnerObjectType, Item->_GameObjectInfo.OwnerObjectId);

					CMessage* ResSpawnPacket = GameServer->MakePacketResObjectSpawn(1, SpawnItem);
					GameServer->SendPacketFieldOfView(Item, ResSpawnPacket);
					ResSpawnPacket->Free();
				}
				else
				{
					// 중복된 아이템의 경우 메모리에 반납
					ObjectReturn(Item->_GameObjectInfo.ObjectType, Item);
				}				
			}
			break;
		case en_GameObjectType::OBJECT_STONE:
		case en_GameObjectType::OBJECT_TREE:
			{
				vector<CGameObject*> SpawnEnvironment;
					
				CEnvironment* Environment = (CEnvironment*)EnterGameObject;

				// 인덱스 가져오기
				_EnvironmentsArrayIndexs.Pop(&EnterGameObject->_ObjectManagerArrayIndex);
				// 배열에 저장
				_EnvironmentsArray[EnterGameObject->_ObjectManagerArrayIndex] = Environment;

				EnterChannel->EnterChannel(EnterGameObject, &Environment->_SpawnPosition);				

				SpawnEnvironment.push_back(Environment);

				CMessage* ResSpawnPacket = GameServer->MakePacketResObjectSpawn(1, SpawnEnvironment);
				GameServer->SendPacketFieldOfView(Environment, ResSpawnPacket);
				ResSpawnPacket->Free();
			}
			break;
	}	
}

bool CObjectManager::ObjectLeaveGame(CGameObject* LeaveGameObject, int32 ObjectIndex, int32 _ChannelId, bool IsObjectReturn)
{
	bool RemoveSuccess = false;
	
	// 타입에 따라 관리당하고 있는 자료구조에서 자신을 삭제
	// 채널에서 삭제	
	switch (LeaveGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		LeaveGameObject->_Channel->LeaveChannel(LeaveGameObject);

		_MonstersArrayIndexs.Push(ObjectIndex);	
		break;	
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:	
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
		LeaveGameObject->_Channel->LeaveChannel(LeaveGameObject);

		_ItemsArrayIndexs.Push(ObjectIndex);		
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		LeaveGameObject->_Channel->LeaveChannel(LeaveGameObject);

		_EnvironmentsArrayIndexs.Push(ObjectIndex);		
		break;
	}

	if (IsObjectReturn == true)
	{
		// 오브젝트 메모리 풀에 반환
		ObjectReturn(LeaveGameObject->_GameObjectInfo.ObjectType, LeaveGameObject);
	}

	return RemoveSuccess;
}

void CObjectManager::PlayerIndexReturn(int32 PlayerIndex)
{
	_PlayersArrayIndexs.Push(PlayerIndex);
}

CGameObject* CObjectManager::Find(int64 ObjectId, en_GameObjectType GameObjectType)
{	
	switch (GameObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		{
			for (int32 i = 0; i < PLAYER_MAX; i++)
			{
				if (_PlayersArray[i] != nullptr 
					&& _PlayersArray[i]->_GameObjectInfo.ObjectId == ObjectId
					&& _PlayersArray[i]->_NetworkState == en_ObjectNetworkState::LIVE)
				{
					return _PlayersArray[i];
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_MONSTER:
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		{
			for (int32 i = 0; i < PLAYER_MAX; i++)
			{
				if (_MonstersArray[i] != nullptr && _MonstersArray[i]->_GameObjectInfo.ObjectId == ObjectId)
				{
					return _MonstersArray[i];
				}
			}		
		}
		break;
	case en_GameObjectType::OBJECT_ENVIRONMENT:
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		{
			for (int32 i = 0; i < PLAYER_MAX; i++)
			{
				if (_EnvironmentsArray[i] != nullptr && _EnvironmentsArray[i]->_GameObjectInfo.ObjectId == ObjectId)
				{
					return _EnvironmentsArray[i];
				}
			}		
		}
		break;
	case en_GameObjectType::OBJECT_ITEM:	
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:	
	case en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_HEAL_POTION_SMALL:	
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIVER_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_GOLD_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
		{
			for (int32 i = 0; i < PLAYER_MAX; i++)
			{
				if (_MonstersArray[i] != nullptr && _MonstersArray[i]->_GameObjectInfo.ObjectId == ObjectId)
				{
					return _EnvironmentsArray[i];
				}
			}		
		}
		break;
	default:
		return nullptr;		
	}

	return nullptr;
}

CGameObject* CObjectManager::ObjectCreate(en_GameObjectType ObjectType)
{
	CGameObject* NewObject = nullptr;

	switch (ObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:	
		NewObject = _PlayerMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_SLIME:
		NewObject = _SlimeMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_BEAR:
		NewObject = _BearMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_ITEM:
		NewObject = _ItemMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON:
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
		NewObject = _WeaponMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_ITEM_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR:
		NewObject = _ArmorMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
		NewObject = _ConsumableMemoryPool->Alloc();
		break;		
	case en_GameObjectType::OBJECT_ITEM_MATERIAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
		NewObject = _MaterialMemoryPool->Alloc();
		break;	
	case en_GameObjectType::OBJECT_STONE:
		NewObject = _StoneMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_TREE:
		NewObject = _TreeMemoryPool->Alloc();
		break;
	}

	return NewObject;
}

void CObjectManager::ObjectReturn(en_GameObjectType ObjectType, CGameObject* ReturnObject)
{
	switch (ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		_PlayerMemoryPool->Free((CPlayer*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_SLIME:
		_SlimeMemoryPool->Free((CSlime*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_BEAR:
		_BearMemoryPool->Free((CBear*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_ITEM:
		_ItemMemoryPool->Free((CItem*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
		_WeaponMemoryPool->Free((CWeapon*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
		_ConsumableMemoryPool->Free((CConsumable*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_ITEM_MATERIAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
		_MaterialMemoryPool->Free((CMaterial*)ReturnObject);
		break;	
	case en_GameObjectType::OBJECT_STONE:
		_StoneMemoryPool->Free((CStone*)ReturnObject);
		break;
	case en_GameObjectType::OBJECT_TREE:
		_TreeMemoryPool->Free((CTree*)ReturnObject);
		break;
	}		
}

CSkill* CObjectManager::SkillCreate()
{
	return _SkillMemoryPool->Alloc();
}

void CObjectManager::SkillReturn(CSkill* ReturnSkill)
{
	_SkillMemoryPool->Free(ReturnSkill);
}

st_SkillInfo* CObjectManager::SkillInfoCreate(en_SkillMediumCategory SkillMediumCategory)
{
	switch (SkillMediumCategory)
	{	
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_ATTACK:
		return _AttackSkillInfoMemoryPool->Alloc();		
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_TACTIC:		
		return _TacTicSkillInfoMemoryPool->Alloc();
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_HEAL:
		return _HealSkillInfoMemoryPool->Alloc();
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_BUF:
		return _BufSkillInfoMemoryPool->Alloc();		
	}	
}

void CObjectManager::SkillInfoReturn(en_SkillMediumCategory SkillMediumCategory, st_SkillInfo* ReturnSkillInfo)
{
	switch (SkillMediumCategory)
	{
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_ATTACK:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_ATTACK:
		_AttackSkillInfoMemoryPool->Free((st_AttackSkillInfo*)ReturnSkillInfo);
		break;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_TACTIC:
		_TacTicSkillInfoMemoryPool->Free((st_TacTicSkillInfo*)ReturnSkillInfo);
		return;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_HEAL:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_HEAL:
		_HealSkillInfoMemoryPool->Free((st_HealSkillInfo*)ReturnSkillInfo);
		break;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_BUF:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_BUF:
		_BufSkillInfoMemoryPool->Free((st_BufSkillInfo*)ReturnSkillInfo);
		break;
	}
}

void CObjectManager::MapObjectSpawn(int32 ChannelId)
{
	CChannel* Channel = G_ChannelManager->Find(ChannelId);
	if (Channel == nullptr)
	{
		return;
	}

	int32 SizeX = Channel->_Map->_SizeX;
	int32 SizeY = Channel->_Map->_SizeY;

	for (int Y = 0; Y < SizeY; Y++)
	{
		for (int X = 0; X < SizeX; X++)
		{
			CGameObject* NewObject = nullptr;

			switch (Channel->_Map->_CollisionMapInfos[Y][X])
			{
			case en_TileMapEnvironment::TILE_MAP_TREE:
				NewObject = (CTree*)ObjectCreate(en_GameObjectType::OBJECT_TREE);
				break;
			case en_TileMapEnvironment::TILE_MAP_STONE:
				NewObject = (CTree*)ObjectCreate(en_GameObjectType::OBJECT_STONE);
				break;
			case en_TileMapEnvironment::TILE_MAP_SLIME:
				NewObject = (CSlime*)ObjectCreate(en_GameObjectType::OBJECT_SLIME);
				break;
			case en_TileMapEnvironment::TILE_MAP_BEAR:
				NewObject = (CBear*)ObjectCreate(en_GameObjectType::OBJECT_BEAR);
				break;
			}

			if (NewObject != nullptr)
			{
				int SpawnPositionX = X + Channel->_Map->_Left;
				int SpawnPositionY = Channel->_Map->_Down - Y;

				st_Vector2Int NewPosition;
				NewPosition._Y = SpawnPositionY;
				NewPosition._X = SpawnPositionX;

				NewObject->_GameObjectInfo.ObjectId = _GameServerObjectId++;
				NewObject->_SpawnPosition = NewPosition;
				NewObject->_NetworkState = en_ObjectNetworkState::LIVE;
				ObjectEnterGame(NewObject, ChannelId);
			}
		}
	}
}

void CObjectManager::ItemSpawn(int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnPosition, en_GameObjectType SpawnItemOwnerType, en_ObjectDataType MonsterDataType)
{
	// 아이템 생성 메세지 생성
	CGameServerMessage* ReqItemCreateMessage = CGameServerMessage::GameServerMessageAlloc();

	ReqItemCreateMessage->Clear();

	*ReqItemCreateMessage << KillerId;
	*ReqItemCreateMessage << (int16)KillerObjectType;
	*ReqItemCreateMessage << SpawnPosition._X;
	*ReqItemCreateMessage << SpawnPosition._Y;
	*ReqItemCreateMessage << (int16)SpawnItemOwnerType;
	*ReqItemCreateMessage << (int32)MonsterDataType;

	st_GameServerJob* ReqDBaseItemCreateJob = GameServer->_GameServerJobMemoryPool->Alloc();
	ReqDBaseItemCreateJob->Type = en_GameServerJobType::DATA_BASE_ITEM_CREATE;
	ReqDBaseItemCreateJob->SessionId = -1;
	ReqDBaseItemCreateJob->Session = nullptr;
	ReqDBaseItemCreateJob->Message = ReqItemCreateMessage;

	GameServer->_GameServerWorldDataBaseThreadMessageQue.Enqueue(ReqDBaseItemCreateJob);
	SetEvent(GameServer->_WorldDataBaseWakeEvent);	
}

void CObjectManager::ObjectSpawn(en_GameObjectType ObjectType, st_Vector2Int SpawnPosition)
{
	CGameObject* SpawnGameObject = nullptr;

	switch (ObjectType)
	{
	case en_GameObjectType::OBJECT_SLIME:
		SpawnGameObject = ObjectCreate(en_GameObjectType::OBJECT_SLIME);
		break;
	case en_GameObjectType::OBJECT_BEAR:
		SpawnGameObject = ObjectCreate(en_GameObjectType::OBJECT_BEAR);
		break;
	case en_GameObjectType::OBJECT_STONE:
		SpawnGameObject = ObjectCreate(en_GameObjectType::OBJECT_STONE);
		break;
	case en_GameObjectType::OBJECT_TREE:
		SpawnGameObject = ObjectCreate(en_GameObjectType::OBJECT_TREE);
		break;
	default:
		break;
	}

	if (SpawnGameObject != nullptr)
	{	
		SpawnGameObject->_GameObjectInfo.ObjectId = _GameServerObjectId++;
		SpawnGameObject->_SpawnPosition = SpawnPosition;		
		ObjectEnterGame(SpawnGameObject, 1);
	}
}