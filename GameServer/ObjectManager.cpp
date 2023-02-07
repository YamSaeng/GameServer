#include "pch.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "GameServerMessage.h"
#include "Item.h"
#include "Skill.h"
#include "MapManager.h"
#include "CraftingTable.h"
#include "Furnace.h"
#include "Sawmill.h"
#include "Potato.h"
#include "Corn.h"
#include "NonPlayer.h"
#include "ChannelManager.h"
#include <atlbase.h>

CObjectManager::CObjectManager()
{
	_PlayerMemoryPool = new CMemoryPoolTLS<CPlayer>();
	_NonPlayerMemoryPool = new CMemoryPoolTLS<CNonPlayer>();
	_SlimeMemoryPool = new CMemoryPoolTLS<CSlime>();
	_BearMemoryPool = new CMemoryPoolTLS<CBear>();

	_ItemMemoryPool = new CMemoryPoolTLS<CItem>();
	_WeaponMemoryPool = new CMemoryPoolTLS<CWeaponItem>();
	_ArmorMemoryPool = new CMemoryPoolTLS<CArmorItem>();
	_ToolMemoryPool = new CMemoryPoolTLS<CToolItem>();
	_MaterialMemoryPool = new CMemoryPoolTLS<CMaterialItem>();
	_ArchitectureMemoryPool = new CMemoryPoolTLS<CArchitectureItem>();
	_CropItemMemoryPool = new CMemoryPoolTLS<CCropItem>();
	_ConsumableMemoryPool = new CMemoryPoolTLS<CConsumable>();

	_TreeMemoryPool = new CMemoryPoolTLS<CTree>();
	_StoneMemoryPool = new CMemoryPoolTLS<CStone>();
	_FurnaceMemoryPool = new CMemoryPoolTLS<CFurnace>();
	_SamillMemoryPool = new CMemoryPoolTLS<CSawmill>();	
	
	_PotatoMemoryPool = new CMemoryPoolTLS<CPotato>();
	_CornMemoryPool = new CMemoryPoolTLS<CCorn>();

	_SkillMemoryPool = new CMemoryPoolTLS<CSkill>();

	_PassiveSkillInfoMemoryPool = new CMemoryPoolTLS<st_PassiveSkillInfo>();
	_AttackSkillInfoMemoryPool = new CMemoryPoolTLS<st_AttackSkillInfo>();	
	_HealSkillInfoMemoryPool = new CMemoryPoolTLS<st_HealSkillInfo>();
	_BufSkillInfoMemoryPool = new CMemoryPoolTLS<st_BufSkillInfo>();

	_GameObjectJobMemoryPool = new CMemoryPoolTLS<st_GameObjectJob>();

	_GameServerObjectId = 10000;

	// 오브젝트 매니저가 소유중인 플레이어, 몬스터, 아이템 미리 할당해서 보관
	for (int PlayerCount = OBJECT_MANAGER_PLAYER_MAX - 1; PlayerCount >= 0; --PlayerCount)
	{
		CPlayer* InitPlayer = dynamic_cast<CPlayer*>(ObjectCreate(en_GameObjectType::OBJECT_PLAYER));
		if (InitPlayer != nullptr)
		{
			_PlayersArray[PlayerCount] = InitPlayer;
			_PlayersArrayIndexs.Push(PlayerCount);
		}		 		
	}	

	for (int ItemCount = OBJECT_MANAGER_ITEM_MAX - 1; ItemCount >= 0; --ItemCount)
	{		
		_ItemsArrayIndexs.Push(ItemCount);
	}

	for (int EnvironmentCount = OBJECT_MANAGER_ENVIRONMENT_MAX - 1; EnvironmentCount >= 0; --EnvironmentCount)
	{		
		_EnvironmentsArrayIndexs.Push(EnvironmentCount);
	}

	for (int CraftingTableCount = OBJECT_MANAGER_CRAFTINGTABLE_MAX - 1; CraftingTableCount >= 0; --CraftingTableCount)
	{		
		_CraftingTableArrayIndexs.Push(CraftingTableCount);
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

void CObjectManager::PlayerIndexReturn(int32 PlayerIndex)
{
	_PlayersArrayIndexs.Push(PlayerIndex);
}

CGameObject* CObjectManager::ObjectCreate(en_GameObjectType ObjectType)
{
	CGameObject* NewObject = nullptr;

	switch (ObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:
		NewObject = _PlayerMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_NON_PLAYER:
		NewObject = _NonPlayerMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_SLIME:
		NewObject = _SlimeMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_BEAR:
		NewObject = _BearMemoryPool->Alloc();
		break;	
	case en_GameObjectType::OBJECT_STONE:
		NewObject = _StoneMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_TREE:
		NewObject = _TreeMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
		NewObject = _FurnaceMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		NewObject = _SamillMemoryPool->Alloc();
		break;	
	case en_GameObjectType::OBJECT_CROP_POTATO:
		NewObject = _PotatoMemoryPool->Alloc();
		break;
	case en_GameObjectType::OBJECT_CROP_CORN:
		NewObject = _CornMemoryPool->Alloc();
		break;	
	}

	if (NewObject != nullptr)
	{
		NewObject->_GameObjectInfo.ObjectId = InterlockedIncrement64(&_GameServerObjectId);
	}	

	return NewObject;
}

void CObjectManager::ObjectReturn(CGameObject* ReturnObject)
{
	if (ReturnObject != nullptr)
	{
		switch (ReturnObject->_GameObjectInfo.ObjectType)
		{
		case en_GameObjectType::OBJECT_PLAYER:		
		case en_GameObjectType::OBJECT_PLAYER_DUMMY:
			_PlayerMemoryPool->Free((CPlayer*)ReturnObject);
			break;
		case en_GameObjectType::OBJECT_NON_PLAYER:
			_NonPlayerMemoryPool->Free((CNonPlayer*)ReturnObject);
			break;
		case en_GameObjectType::OBJECT_SLIME:
			_SlimeMemoryPool->Free((CSlime*)ReturnObject);
			break;
		case en_GameObjectType::OBJECT_BEAR:
			_BearMemoryPool->Free((CBear*)ReturnObject);
			break;
		case en_GameObjectType::OBJECT_STONE:
			_StoneMemoryPool->Free((CStone*)ReturnObject);
			break;
		case en_GameObjectType::OBJECT_TREE:
			_TreeMemoryPool->Free((CTree*)ReturnObject);
			break;
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
			_FurnaceMemoryPool->Free((CFurnace*)ReturnObject);
			break;
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
			_SamillMemoryPool->Free((CSawmill*)ReturnObject);
			break;
		case en_GameObjectType::OBJECT_CROP_POTATO:
			_PotatoMemoryPool->Free((CPotato*)ReturnObject);
			break;
		case en_GameObjectType::OBJECT_CROP_CORN:
			_CornMemoryPool->Free((CCorn*)ReturnObject);
			break;		
		}
	}	
	else
	{
		CRASH("빈 오브젝트를 반납하려고 시도");
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

CItem* CObjectManager::ItemCreate(en_SmallItemCategory NewItemSmallCategory)
{
	CItem* NewItem = nullptr;

	switch (NewItemSmallCategory)
	{
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
	case en_SmallItemCategory::ITEM_SAMLL_CATEGORY_WEAPON_WOOD_SHIELD:	
		NewItem = _WeaponMemoryPool->Alloc();
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
		NewItem = _ArmorMemoryPool->Alloc();
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_TOOL_FARMING_SHOVEL:
		NewItem = _ToolMemoryPool->Alloc();
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEALTH_RESTORATION_POTION_SMALL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_MANA_RESTORATION_POTION_SMALL:
		NewItem = _ConsumableMemoryPool->Alloc();
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_NUGGET:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_INGOT:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_NUGGET:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_INGOT:
		NewItem = _MaterialMemoryPool->Alloc();
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_DEFAULT_CRAFTING_TABLE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_SAWMILL:	
		NewItem = _ArchitectureMemoryPool->Alloc();
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_SEED_POTATO:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_SEED_CORN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_CORN:
		NewItem = _CropItemMemoryPool->Alloc();
		break;		
	}

	if (NewItem != nullptr)
	{
		// 기본 아이템 정보 저장		
		st_ItemInfo NewItemInfo = *G_Datamanager->FindItemData(NewItemSmallCategory);
		NewItem->_ItemInfo = NewItemInfo;

		NewItem->_GameObjectInfo.ObjectId = InterlockedIncrement64(&_GameServerObjectId);
	}

	return NewItem;
}

void CObjectManager::ItemReturn(CItem* ReturnItem)
{
	switch (ReturnItem->_ItemInfo.ItemSmallCategory)
	{
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD:
	case en_SmallItemCategory::ITEM_SAMLL_CATEGORY_WEAPON_WOOD_SHIELD:	
		_WeaponMemoryPool->Free((CWeaponItem*)ReturnItem);
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER:
		_ArmorMemoryPool->Free((CArmorItem*)ReturnItem);
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_TOOL_FARMING_SHOVEL:
		_ToolMemoryPool->Free((CToolItem*)ReturnItem);
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEALTH_RESTORATION_POTION_SMALL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_MANA_RESTORATION_POTION_SMALL:
		_ConsumableMemoryPool->Free((CConsumable*)ReturnItem);
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_NUGGET:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_INGOT:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_NUGGET:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_INGOT:
		_MaterialMemoryPool->Free((CMaterialItem*)ReturnItem);
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_DEFAULT_CRAFTING_TABLE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_SAWMILL:
		_ArchitectureMemoryPool->Free((CArchitectureItem*)ReturnItem);
		break;
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_SEED_POTATO:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_SEED_CORN:
	case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_CORN:
		_CropItemMemoryPool->Free((CCropItem*)ReturnItem);
		break;
	}
}

st_SkillInfo* CObjectManager::SkillInfoCreate(en_SkillType SkillType, int8 SkillLevel)
{
	st_SkillInfo* NewSkillInfo = nullptr;

	switch (SkillType)
	{
	case en_SkillType::SKILL_TYPE_NONE:
		CRASH("None 스킬 데이터 찾기 요청");
		break;
	case en_SkillType::SKILL_FIGHT_TWO_HAND_SWORD_MASTER:
		NewSkillInfo = _PassiveSkillInfoMemoryPool->Alloc();		
		break;		
	case en_SkillType::SKILL_DEFAULT_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE:
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP:	
	case en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING:	
		{
			st_AttackSkillInfo* NewAttackSkillInfo = _AttackSkillInfoMemoryPool->Alloc();
			st_AttackSkillInfo* FindAttackSkillData = (st_AttackSkillInfo*)G_Datamanager->FindSkillData(SkillType);
			*NewAttackSkillInfo = *FindAttackSkillData;			
			NewAttackSkillInfo->SkillLevel = SkillLevel;
			
			TCHAR SkillExplanationMessage[256] = L"0";

			switch (NewAttackSkillInfo->SkillType)
			{
			case en_SkillType::SKILL_DEFAULT_ATTACK:
				wsprintf(SkillExplanationMessage, NewAttackSkillInfo->SkillExplanation.c_str());
				break;
			case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
			case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
			case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewAttackSkillInfo->SkillExplanation.c_str(), NewAttackSkillInfo->SkillDistance, NewAttackSkillInfo->SkillMinDamage, NewAttackSkillInfo->SkillMaxDamage);
				break;
			case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewAttackSkillInfo->SkillExplanation.c_str(), NewAttackSkillInfo->SkillMinDamage, NewAttackSkillInfo->SkillMaxDamage);
				break;
			case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE:
			case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewAttackSkillInfo->SkillExplanation.c_str(), NewAttackSkillInfo->SkillDistance, NewAttackSkillInfo->SkillDurationTime / 1000.0f, NewAttackSkillInfo->SkillMinDamage, NewAttackSkillInfo->SkillMaxDamage);
				break;
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewAttackSkillInfo->SkillExplanation.c_str(), NewAttackSkillInfo->SkillDistance, NewAttackSkillInfo->SkillMinDamage, NewAttackSkillInfo->SkillMaxDamage);
				break;
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewAttackSkillInfo->SkillExplanation.c_str(), NewAttackSkillInfo->SkillDistance, NewAttackSkillInfo->SkillDurationTime / 1000.0f);
				break;
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewAttackSkillInfo->SkillExplanation.c_str(), NewAttackSkillInfo->SkillDistance, NewAttackSkillInfo->SkillMinDamage, NewAttackSkillInfo->SkillMaxDamage, NewAttackSkillInfo->SkillDebufMovingSpeed, NewAttackSkillInfo->SkillDurationTime / 1000.0f);
				break;
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewAttackSkillInfo->SkillExplanation.c_str(), NewAttackSkillInfo->SkillDistance, NewAttackSkillInfo->SkillMinDamage, NewAttackSkillInfo->SkillMaxDamage);
				break;
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewAttackSkillInfo->SkillExplanation.c_str(), NewAttackSkillInfo->SkillDistance, NewAttackSkillInfo->SkillMinDamage, NewAttackSkillInfo->SkillMaxDamage, NewAttackSkillInfo->SkillDurationTime / 1000.0f);
				break;
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewAttackSkillInfo->SkillExplanation.c_str(), NewAttackSkillInfo->SkillDistance, NewAttackSkillInfo->SkillMinDamage, NewAttackSkillInfo->SkillMaxDamage);
				break;
			case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewAttackSkillInfo->SkillExplanation.c_str(), NewAttackSkillInfo->SkillDistance, NewAttackSkillInfo->SkillMinDamage, NewAttackSkillInfo->SkillMaxDamage);
				break;
			case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewAttackSkillInfo->SkillExplanation.c_str(), NewAttackSkillInfo->SkillDistance, NewAttackSkillInfo->SkillDurationTime / 1000.0f);
				break;
			}

			NewAttackSkillInfo->SkillExplanation = SkillExplanationMessage;			

			NewSkillInfo = NewAttackSkillInfo;
		}
		break;		
	case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:
	case en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE:
	case en_SkillType::SKILL_SPELL_ACTIVE_BUF_TELEPORT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_WEAPON_POISON:
		{
			st_BufSkillInfo* NewBufSkillInfo = _BufSkillInfoMemoryPool->Alloc();;
			st_BufSkillInfo* FindBufSkillData = (st_BufSkillInfo*)G_Datamanager->FindSkillData(SkillType);
			*NewBufSkillInfo = *FindBufSkillData;
			NewBufSkillInfo->SkillLevel = SkillLevel;

			TCHAR SkillExplanationMessage[256] = L"0";

			switch (NewBufSkillInfo->SkillType)
			{
			case en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewBufSkillInfo->SkillExplanation.c_str(), NewBufSkillInfo->SkillDurationTime / 1000.0f);
				break;
			case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:
				_stprintf_s(SkillExplanationMessage, sizeof(TCHAR) * 256, NewBufSkillInfo->SkillExplanation.c_str(), NewBufSkillInfo->IncreaseMaxAttackPoint, NewBufSkillInfo->SkillDurationTime / 1000.0f);
				break;
			}

			NewBufSkillInfo->SkillExplanation = SkillExplanationMessage;

			NewSkillInfo = NewBufSkillInfo;
		}
		break;
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:
		return _HealSkillInfoMemoryPool->Alloc();
	case en_SkillType::SKILL_SLIME_ACTIVE_POISION_ATTACK:
		{
			st_AttackSkillInfo* NewSlimeActivePoision = _AttackSkillInfoMemoryPool->Alloc();
			st_AttackSkillInfo* FindSlimeData = (st_AttackSkillInfo*)G_Datamanager->FindSkillData(SkillType);
			*NewSlimeActivePoision = *FindSlimeData;

			NewSlimeActivePoision->SkillLevel = SkillLevel;

			NewSkillInfo = NewSlimeActivePoision;
		}	
	}		

	return NewSkillInfo;
}

void CObjectManager::SkillInfoReturn(en_SkillType SkillType, st_SkillInfo* ReturnSkillInfo)
{
	switch (SkillType)
	{
	case en_SkillType::SKILL_TYPE_NONE:
		CRASH("None 스킬 데이터 찾기 요청");
		break;
	case en_SkillType::SKILL_FIGHT_TWO_HAND_SWORD_MASTER:
		_PassiveSkillInfoMemoryPool->Free((st_PassiveSkillInfo*)ReturnSkillInfo);
		break;
	case en_SkillType::SKILL_DEFAULT_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE:
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP:
	case en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING:
		_AttackSkillInfoMemoryPool->Free((st_AttackSkillInfo*)ReturnSkillInfo);
		break;
	case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:
	case en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE:
	case en_SkillType::SKILL_SPELL_ACTIVE_BUF_TELEPORT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_WEAPON_POISON:
		_BufSkillInfoMemoryPool->Free((st_BufSkillInfo*)ReturnSkillInfo);
		break;
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:
		_HealSkillInfoMemoryPool->Free((st_HealSkillInfo*)ReturnSkillInfo);
		break;
	}	
}

void CObjectManager::MapObjectSpawn(int64& MapID)
{
	CMap* Map = G_MapManager->GetMap(MapID);

	int32 SizeX = Map->_SizeX;
	int32 SizeY = Map->_SizeY;

	for (int Y = 0; Y < SizeY; Y++)
	{
		for (int X = 0; X < SizeX; X++)
		{
			CGameObject* NewObject = nullptr;

			switch (Map->_CollisionMapInfos[Y][X])
			{
			case en_MapObjectInfo::TILE_MAP_NONE:				
				break;
			case en_MapObjectInfo::TILE_MAP_TREE:
				NewObject = ObjectCreate(en_GameObjectType::OBJECT_TREE);
				break;
			case en_MapObjectInfo::TILE_MAP_STONE:
				NewObject = ObjectCreate(en_GameObjectType::OBJECT_STONE);
				break;
			case en_MapObjectInfo::TILE_MAP_SLIME:
				NewObject = ObjectCreate(en_GameObjectType::OBJECT_SLIME);
				break;
			case en_MapObjectInfo::TILE_MAP_BEAR:
				NewObject = ObjectCreate(en_GameObjectType::OBJECT_BEAR);
				break;			
			case en_MapObjectInfo::TILE_MAP_FURNACE:
				NewObject = ObjectCreate(en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE);
				break;
			case en_MapObjectInfo::TILE_MAP_SAMILL:
				NewObject = ObjectCreate(en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL);
				break;
			case en_MapObjectInfo::TILE_MAP_POTATO:
				NewObject = ObjectCreate(en_GameObjectType::OBJECT_CROP_POTATO);
				break;
			case en_MapObjectInfo::TILE_MAP_GENERAL_MERCHANT:	
				NewObject = ObjectCreate(en_GameObjectType::OBJECT_NON_PLAYER);
				break;
			}

			if (NewObject != nullptr)
			{
				int SpawnPositionX = X + Map->_Left;
				int SpawnPositionY = Map->_Down - Y;

				st_Vector2Int NewPosition;
				NewPosition._Y = SpawnPositionY;
				NewPosition._X = SpawnPositionX;

				NewObject->_SpawnPosition = NewPosition;
				NewObject->_NetworkState = en_ObjectNetworkState::LIVE;
							
				CChannel* Channel = Map->GetChannelManager()->Find(1);
				Channel->EnterChannel(NewObject, &NewObject->_SpawnPosition);
			}
		}
	}
}

void CObjectManager::MapTileInfoSpawn(int64& MapID)
{

}

void CObjectManager::ObjectItemSpawn(CChannel* SpawnChannel, int64 KillerId, en_GameObjectType KillerObjectType, st_Vector2Int SpawnIntPosition, st_Vector2 SpawnPosition, en_GameObjectType SpawnItemOwnerType, en_GameObjectType ItemDataType)
{
	bool Find = false;
	en_SmallItemCategory DropItemCategory;
	int16 DropItemCount = 0;	

	random_device RD;
	mt19937 Gen(RD());
	uniform_real_distribution<float> RandomDropPoint(0, 1); // 0.0 ~ 1.0
	float RandomPoint = 100 * RandomDropPoint(Gen);

	int32 Sum = 0;

	switch ((en_GameObjectType)SpawnItemOwnerType)
	{
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		{
			auto FindMonsterDropItem = G_Datamanager->_Monsters.find(ItemDataType);
			st_MonsterData MonsterData = *(*FindMonsterDropItem).second;

			for (st_DropData DropItem : MonsterData.DropItems)
			{
				Sum += DropItem.Probability;

				if (Sum >= RandomPoint)
				{
					Find = true;
					// 드랍 확정 되면 해당 아이템 읽어오기
					auto FindDropItemInfo = G_Datamanager->_Items.find((int16)DropItem.DropItemSmallCategory);
					if (FindDropItemInfo == G_Datamanager->_Items.end())
					{
						CRASH("DropItemInfo를 찾지 못함");
					}

					uniform_int_distribution<int> RandomDropItemCount(DropItem.MinCount, DropItem.MaxCount);
					DropItemCount = RandomDropItemCount(Gen);
					DropItemCategory = DropItem.DropItemSmallCategory;
					break;
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		{
			auto FindEnvironmentDropItem = G_Datamanager->_Environments.find(ItemDataType);
			st_EnvironmentData EnvironmentData = *(*FindEnvironmentDropItem).second;

			for (st_DropData DropItem : EnvironmentData.DropItems)
			{
				Sum += DropItem.Probability;

				if (Sum >= RandomPoint)
				{
					Find = true;
					// 드랍 확정 되면 해당 아이템 읽어오기
					auto FindDropItemInfo = G_Datamanager->_Items.find((int16)DropItem.DropItemSmallCategory);
					if (FindDropItemInfo == G_Datamanager->_Items.end())
					{
						CRASH("DropItemInfo를 찾지 못함");
					}					

					uniform_int_distribution<int> RandomDropItemCount(DropItem.MinCount, DropItem.MaxCount);
					DropItemCount = RandomDropItemCount(Gen);
					DropItemCategory = DropItem.DropItemSmallCategory;
					break;
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_CROP_POTATO:
	case en_GameObjectType::OBJECT_CROP_CORN:
		{
			auto FindCropDropItemIter = G_Datamanager->_Crops.find(ItemDataType);
			st_CropData CropData = *(*FindCropDropItemIter).second;

			for (st_DropData DropItem : CropData.DropItems)
			{
				Sum += DropItem.Probability;

				if (Sum >= RandomPoint)
				{
					Find = true;
					// 드랍 확정 되면 해당 아이템 읽어오기
					auto FindDropItemInfo = G_Datamanager->_Items.find((int16)DropItem.DropItemSmallCategory);
					if (FindDropItemInfo == G_Datamanager->_Items.end())
					{
						CRASH("DropItemInfo를 찾지 못함");
					}					

					uniform_int_distribution<int> RandomDropItemCount(DropItem.MinCount, DropItem.MaxCount);
					DropItemCount = RandomDropItemCount(Gen);
					DropItemCategory = DropItem.DropItemSmallCategory;
					break;
				}
			}
		}
		break;
	}

	if (Find == true)
	{
		// 아이템 생성
		CItem* NewItem = ItemCreate(DropItemCategory);
		NewItem->_ItemInfo.ItemCount = DropItemCount;		

		NewItem->_GameObjectInfo.ObjectType = NewItem->_ItemInfo.ItemObjectType;
		NewItem->_GameObjectInfo.OwnerObjectId = KillerId;
		NewItem->_GameObjectInfo.OwnerObjectType = (en_GameObjectType)KillerObjectType;
		NewItem->_SpawnPosition = SpawnIntPosition;
		NewItem->_GameObjectInfo.ObjectPositionInfo.Position = SpawnPosition;			

		st_GameObjectJob* EnterChannelItemJob = GameServer->MakeGameObjectJobObjectEnterChannel(NewItem);
		SpawnChannel->_ChannelJobQue.Enqueue(EnterChannelItemJob);
	}
}

void CObjectManager::ObjectItemDropToSpawn(CGameObject* DropOwnerObject, CChannel* SpawnChannel, en_SmallItemCategory DropItemType, int32 DropItemCount)
{
	CItem* NewItem = ItemCreate(DropItemType);
	if (NewItem != nullptr)
	{
		NewItem->_ItemInfo.ItemCount = DropItemCount;
		NewItem->_ItemInfo.ItemDBId = NewItem->_GameObjectInfo.ObjectId;		
		
		NewItem->_GameObjectInfo.ObjectType = NewItem->_ItemInfo.ItemObjectType;
		NewItem->_GameObjectInfo.OwnerObjectId = DropOwnerObject->_GameObjectInfo.ObjectId;
		NewItem->_GameObjectInfo.OwnerObjectType = DropOwnerObject->_GameObjectInfo.ObjectType;
		NewItem->_SpawnPosition = DropOwnerObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition;
		NewItem->_GameObjectInfo.ObjectPositionInfo.Position = DropOwnerObject->_GameObjectInfo.ObjectPositionInfo.Position;

		st_GameObjectJob* EnterChannelItemJob = GameServer->MakeGameObjectJobObjectEnterChannel(NewItem);
		SpawnChannel->_ChannelJobQue.Enqueue(EnterChannelItemJob);		
	}	
}

st_GameObjectJob* CObjectManager::GameObjectJobCreate()
{
	return _GameObjectJobMemoryPool->Alloc();
}

void CObjectManager::GameObjectJobReturn(st_GameObjectJob* GameObjectJob)
{
	_GameObjectJobMemoryPool->Free(GameObjectJob);
}
