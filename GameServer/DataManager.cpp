#include "pch.h"
#include "DataManager.h"

void CDataManager::LoadDataItem(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["Weapons"].GetArray())
	{
		string LargeCategory = Filed["ItemLargeCategory"].GetString();

		for (auto& WeaponListFiled : Filed["List"].GetArray())
		{
			string MediumCategory = WeaponListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = WeaponListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = WeaponListFiled["ItemObjectType"].GetString();
			string ItemExplain = WeaponListFiled["ItemExplain"].GetString();
			string ItemName = WeaponListFiled["ItemName"].GetString();
			int ItemMinDamage = WeaponListFiled["ItemMinDamage"].GetInt();
			int ItemMaxDamage = WeaponListFiled["ItemMaxDamage"].GetInt();
			string ImageFilePath = WeaponListFiled["ImageFilePath"].GetString();
						
			st_ItemData* WeaponItemData = new st_ItemData();					

			WeaponItemData->LargeItemCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_SWORD")
			{
				WeaponItemData->MediumItemCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_SWORD;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD")
			{
				WeaponItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD;
			}			

			if (ItemObjectType == "OBJECT_ITEM_WEAPON_WOOD_SWORD")
			{
				WeaponItemData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD;
			}

			WeaponItemData->ItemExplain = ItemExplain;
			WeaponItemData->ItemName = ItemName;						
			WeaponItemData->ItemMinDamage = ItemMinDamage;
			WeaponItemData->ItemMaxDamage = ItemMaxDamage;
			WeaponItemData->ItemDefence = 0;
			WeaponItemData->ItemMaxCount = 1;
			WeaponItemData->ItemThumbnailImagePath = ImageFilePath;		

			_Items.insert(pair<int16, st_ItemData*>((int16)WeaponItemData->SmallItemCategory, WeaponItemData));
		}		
	}

	for (auto& Filed : Document["Armors"].GetArray())
	{
		string LargeCategory = Filed["ItemLargeCategory"].GetString();

		for (auto& ArmorListFiled : Filed["List"].GetArray())
		{
			string MediumCategory = ArmorListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = ArmorListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = ArmorListFiled["ItemObjectType"].GetString();
			string ItemExplain = ArmorListFiled["ItemExplain"].GetString();
			string ItemName = ArmorListFiled["ItemName"].GetString();
			int ItemDefence = ArmorListFiled["ItemDefence"].GetInt();
			string ImageFilePath = ArmorListFiled["ImageFilePath"].GetString();

			st_ItemData* ArmorItemData = new st_ItemData();

			ArmorItemData->LargeItemCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARMOR;
			
			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_HAT")
			{
				ArmorItemData->MediumItemCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_HAT;
			}
			else if (MediumCategory == "ITEM_MEDIUM_CATEGORY_WEAR")
			{
				ArmorItemData->MediumItemCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAR;
			}
			else if (MediumCategory == "ITEM_MEDIUM_CATEGORY_BOOT")
			{
				ArmorItemData->MediumItemCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_BOOT;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER")
			{
				ArmorItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD")
			{
				ArmorItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER")
			{
				ArmorItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER;
			}

			if (ItemObjectType == "OBJECT_ITEM_ARMOR_LEATHER_HELMET")
			{
				ArmorItemData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET;
			}
			else if (ItemObjectType == "OBJECT_ITEM_ARMOR_WOOD_ARMOR")
			{
				ArmorItemData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR;
			}
			else if (ItemObjectType == "OBJECT_ITEM_ARMOR_LEATHER_BOOT")
			{
				ArmorItemData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT;
			}	

			ArmorItemData->ItemExplain = ItemExplain;
			ArmorItemData->ItemName = ItemName;	
			ArmorItemData->ItemMinDamage = 0;
			ArmorItemData->ItemMaxDamage = 0;
			ArmorItemData->ItemDefence = ItemDefence;			
			ArmorItemData->ItemMaxCount = 1;
			ArmorItemData->ItemThumbnailImagePath = ImageFilePath;	

			_Items.insert(pair<int16, st_ItemData*>((int16)ArmorItemData->SmallItemCategory, ArmorItemData));
		}		
	}

	for (auto& Filed : Document["Consumables"].GetArray())
	{
		string PotionLargeCategory = Filed["ItemLargeCategory"].GetString();
		for (auto& PotionDataListFiled : Filed["PotionList"].GetArray())
		{
			string MediumCategory = PotionDataListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = PotionDataListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = PotionDataListFiled["ItemObjectType"].GetString();
			string ItemExplain = PotionDataListFiled["ItemExplain"].GetString();
			string ItemName = PotionDataListFiled["ItemName"].GetString();
			int ItemMaxCount = PotionDataListFiled["ItemMaxCount"].GetInt();
			string ImageFilePath = PotionDataListFiled["ImageFilePath"].GetString();

			st_ConsumableData* PotionItemData = new st_ConsumableData();
			PotionItemData->LargeItemCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_POTION;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_HEAL")
			{
				PotionItemData->MediumItemCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_HEAL;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL")
			{
				PotionItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL;
				PotionItemData->HealPoint = 50;
			}

			if (ItemObjectType == "OBJECT_ITEM_CONSUMABLE_HEAL_POTION_SMALL")
			{
				PotionItemData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CONSUMABLE_HEAL_POTION_SMALL;
			}					

			PotionItemData->ItemExplain = ItemExplain;
			PotionItemData->ItemName = ItemName;
			PotionItemData->ItemMinDamage = 0;
			PotionItemData->ItemMaxDamage = 0;
			PotionItemData->ItemDefence = 0;
			PotionItemData->ItemMaxCount = ItemMaxCount;
			PotionItemData->ItemThumbnailImagePath = ImageFilePath;			

			_Consumables.insert(pair<int16, st_ConsumableData*>((int16)PotionItemData->SmallItemCategory, PotionItemData));
		}

		string SkillBookLargeCategory = Filed["ItemLargeCategory"].GetString();
		for (auto& SkillBookDataListFiled : Filed["SkillBookList"].GetArray())
		{
			string MediumCategory = SkillBookDataListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = SkillBookDataListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = SkillBookDataListFiled["ItemObjectType"].GetString();
			string ItemExplain = SkillBookDataListFiled["ItemExplain"].GetString();
			string ItemName = SkillBookDataListFiled["ItemName"].GetString();
			int ItemMaxCount = SkillBookDataListFiled["ItemMaxCount"].GetInt();
			string ImageFilePath = SkillBookDataListFiled["ImageFilePath"].GetString();
						
			st_ConsumableData* SkillBookItemData = new st_ConsumableData();
			SkillBookItemData->LargeItemCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_SKILLBOOK;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_NONE")
			{
				SkillBookItemData->MediumItemCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_FIERCE_ATTACK")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_FIERCE_ATTACK;
				SkillBookItemData->SkillType = en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK;
				SkillBookItemData->SkillType = en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK;
				SkillBookItemData->SkillType = en_SkillType::SKILL_KNIGHT_SHAEHONE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK;
				SkillBookItemData->SkillType = en_SkillType::SKILL_KNIGHT_CHOHONE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK;
				SkillBookItemData->SkillType = en_SkillType::SKILL_KNIGHT_SMASH_WAVE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE;
				SkillBookItemData->SkillType = en_SkillType::SKILL_KNIGHT_CHARGE_POSE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON;
				SkillBookItemData->SkillType = en_SkillType::SKILL_SHAMAN_FLAME_HARPOON;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE;
				SkillBookItemData->SkillType = en_SkillType::SKILL_SHAMAN_HELL_FIRE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEOGRY_SKILLBOOK_SHAMAN_HEALING_LIGHT")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEOGRY_SKILLBOOK_SHAMAN_HEALING_LIGHT;
				SkillBookItemData->SkillType = en_SkillType::SKILL_SHAMAN_HEALING_LIGHT;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HEALING_WIND")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HEALING_WIND;
				SkillBookItemData->SkillType = en_SkillType::SKILL_SHAMAN_HEALING_WIND;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE;
				SkillBookItemData->SkillType = en_SkillType::SKILL_SHOCK_RELEASE;
			}

			if (ItemObjectType == "OBJECT_ITEM_CONSUMABLE_SKILL_BOOK")
			{
				SkillBookItemData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK;
			}

			SkillBookItemData->ItemExplain = ItemExplain;
			SkillBookItemData->ItemName = ItemName;
			SkillBookItemData->ItemMinDamage = 0;
			SkillBookItemData->ItemMaxDamage = 0;
			SkillBookItemData->ItemDefence = 0;
			SkillBookItemData->ItemMaxCount = ItemMaxCount;
			SkillBookItemData->ItemThumbnailImagePath = ImageFilePath;						

			_Consumables.insert(pair<int16, st_ConsumableData*>((int16)SkillBookItemData->SmallItemCategory, SkillBookItemData));
		}
	}

	for (auto& Filed : Document["Material"].GetArray())
	{
		string MaterialLargeCategory = Filed["ItemLargeCategory"].GetString();
		for (auto& MaterialDataListFiled : Filed["MaterialList"].GetArray())
		{
			string MediumCategory = MaterialDataListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = MaterialDataListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = MaterialDataListFiled["ItemObjectType"].GetString();
			string ItemExplain = MaterialDataListFiled["ItemExplain"].GetString();
			string ItemName = MaterialDataListFiled["ItemName"].GetString();
			int ItemMaxCount = MaterialDataListFiled["ItemMaxCount"].GetInt();
			string ImageFilePath = MaterialDataListFiled["ImageFilePath"].GetString();

			st_ItemData* MaterialData = new st_ItemData();
			MaterialData->LargeItemCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_NONE")
			{
				MaterialData->MediumItemCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_LEATHER")
			{
				MaterialData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL")
			{
				MaterialData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN")
			{
				MaterialData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN")
			{
				MaterialData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN")
			{
				MaterialData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_STONE")
			{
				MaterialData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG")
			{
				MaterialData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK")
			{
				MaterialData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_YARN")
			{
				MaterialData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN;
			}

			// 재료 아이템의 스폰 오브젝트 타입
			if (ItemObjectType == "OBJECT_ITEM_MATERIAL_LEATHER")
			{
				MaterialData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_SLIME_GEL")
			{
				MaterialData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_BRONZE_COIN")
			{
				MaterialData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_SLIVER_COIN")
			{
				MaterialData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIVER_COIN;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_GOLD_COIN")
			{
				MaterialData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_GOLD_COIN;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_STONE")
			{
				MaterialData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_WOOD_LOG")
			{
				MaterialData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_WOOD_FLANK")
			{
				MaterialData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_YARN")
			{
				MaterialData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN;
			}

			MaterialData->ItemExplain = ItemExplain;
			MaterialData->ItemName = ItemName;
			MaterialData->ItemMinDamage = 0;
			MaterialData->ItemMaxDamage = 0;
			MaterialData->ItemDefence = 0;
			MaterialData->ItemMaxCount = ItemMaxCount;
			MaterialData->ItemThumbnailImagePath = ImageFilePath;			

			_Items.insert(pair<int16, st_ItemData*>((int16)MaterialData->SmallItemCategory, MaterialData));
		}	
	}
}

void CDataManager::LoadDataPlayerCharacterStatus(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["PlayerWarriorCharacterStatus"].GetArray())
	{
		string PlayerType = Filed["PlayerType"].GetString();
		
		for (auto& PlayerWarriorCharacterFiled : Filed["PlayerWarriorCharacterLevelDataList"].GetArray())
		{
			int Level = PlayerWarriorCharacterFiled["Level"].GetInt();
			int MaxHP = PlayerWarriorCharacterFiled["MaxHP"].GetInt();
			int MaxMP = PlayerWarriorCharacterFiled["MaxMP"].GetInt();
			int MaxDP = PlayerWarriorCharacterFiled["MaxDP"].GetInt();
			int MinMeleeAttackDamage = PlayerWarriorCharacterFiled["MinMeleeAttackDamage"].GetInt();
			int MaxMeleeAttackDamage = PlayerWarriorCharacterFiled["MaxMeleeAttackDamage"].GetInt();
			int16 MeleeAttackHitRate = (int16)PlayerWarriorCharacterFiled["MeleeAttackHitRate"].GetInt();
			int16 MagicDamage = (int16)PlayerWarriorCharacterFiled["MagicDamage"].GetInt();
			int16 MagicHitRate = (int16)PlayerWarriorCharacterFiled["MagicHitRate"].GetInt();
			int Defence = PlayerWarriorCharacterFiled["Defence"].GetInt();
			int16 EvasionRate = PlayerWarriorCharacterFiled["EvasionRate"].GetInt();
			int16 MeleeCriticalPoint = (int16)(PlayerWarriorCharacterFiled["MeleeCriticalPoint"].GetInt());
			int16 MagicCriticalPoint = (int16)(PlayerWarriorCharacterFiled["MagicCriticalPoint"].GetInt());
			float Speed = PlayerWarriorCharacterFiled["Speed"].GetFloat();

			st_ObjectStatusData* WarriorStatusData = new st_ObjectStatusData();
			
			WarriorStatusData->PlayerType = en_GameObjectType::OBJECT_MELEE_PLAYER;	

			WarriorStatusData->Level = Level;
			WarriorStatusData->MaxHP = MaxHP;
			WarriorStatusData->MaxMP = MaxMP;
			WarriorStatusData->MaxDP = MaxDP;
			WarriorStatusData->MinMeleeAttackDamage = MinMeleeAttackDamage;
			WarriorStatusData->MaxMeleeAttackDamage = MaxMeleeAttackDamage;
			WarriorStatusData->MeleeAttackHitRate = MeleeAttackHitRate;
			WarriorStatusData->MagicDamage = MagicDamage;
			WarriorStatusData->MagicHitRate = MagicHitRate;
			WarriorStatusData->Defence = Defence;
			WarriorStatusData->EvasionRate = EvasionRate;
			WarriorStatusData->MeleeCriticalPoint = MeleeCriticalPoint;
			WarriorStatusData->MagicCriticalPoint = MagicCriticalPoint;
			WarriorStatusData->Speed = Speed;

			_WarriorStatus.insert(pair<int32, st_ObjectStatusData*>(WarriorStatusData->Level, WarriorStatusData));
		}		
	}

	for (auto& Filed : Document["PlayerMagicCharacterStatus"].GetArray())
	{
		string PlayerType = Filed["PlayerType"].GetString();

		for (auto& PlayerShamanCharacterFiled : Filed["PlayerMagicCharacterLevelDataList"].GetArray())
		{
			int Level = PlayerShamanCharacterFiled["Level"].GetInt();
			int MaxHP = PlayerShamanCharacterFiled["MaxHP"].GetInt();
			int MaxMP = PlayerShamanCharacterFiled["MaxMP"].GetInt();
			int MaxDP = PlayerShamanCharacterFiled["MaxDP"].GetInt();
			int MinMeleeAttackDamage = PlayerShamanCharacterFiled["MinMeleeAttackDamage"].GetInt();
			int MaxMeleeAttackDamage = PlayerShamanCharacterFiled["MaxMeleeAttackDamage"].GetInt();
			int16 MeleeAttackHitRate = (int16)PlayerShamanCharacterFiled["MeleeAttackHitRate"].GetInt();
			int16 MagicDamage = (int16)PlayerShamanCharacterFiled["MagicDamage"].GetInt();
			int16 MagicHitRate = (int16)PlayerShamanCharacterFiled["MagicHitRate"].GetInt();
			int Defence = PlayerShamanCharacterFiled["Defence"].GetInt();
			int16 EvasionRate = PlayerShamanCharacterFiled["EvasionRate"].GetInt();
			int16 MeleeCriticalPoint = (int16)(PlayerShamanCharacterFiled["MeleeCriticalPoint"].GetInt());
			int16 MagicCriticalPoint = (int16)(PlayerShamanCharacterFiled["MagicCriticalPoint"].GetInt());
			float Speed = PlayerShamanCharacterFiled["Speed"].GetFloat();

			st_ObjectStatusData* ShamanStatusData = new st_ObjectStatusData();

			ShamanStatusData->PlayerType = en_GameObjectType::OBJECT_MAGIC_PLAYER;

			ShamanStatusData->Level = Level;
			ShamanStatusData->MaxHP = MaxHP;
			ShamanStatusData->MaxMP = MaxMP;
			ShamanStatusData->MaxDP = MaxDP;
			ShamanStatusData->MinMeleeAttackDamage = MinMeleeAttackDamage;
			ShamanStatusData->MaxMeleeAttackDamage = MaxMeleeAttackDamage;
			ShamanStatusData->MeleeAttackHitRate = MeleeAttackHitRate;
			ShamanStatusData->MagicDamage = MagicDamage;
			ShamanStatusData->MagicHitRate = MagicHitRate;
			ShamanStatusData->Defence = Defence;
			ShamanStatusData->EvasionRate = EvasionRate;
			ShamanStatusData->MeleeCriticalPoint = MeleeCriticalPoint;
			ShamanStatusData->MagicCriticalPoint = MagicCriticalPoint;
			ShamanStatusData->Speed = Speed;

			_ShamanStatus.insert(pair<int32, st_ObjectStatusData*>(ShamanStatusData->Level, ShamanStatusData));
		}
	}
}

void CDataManager::LoadDataLevel(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["LevelingData"].GetArray())
	{
		st_LevelData* LevelData = new st_LevelData();

		int Level = Filed["Level"].GetInt();
		int64 RequireExperience = Filed["RequireExperience"].GetInt64();
		int64 TotalExperience = Filed["TotalExperience"].GetInt64();

		LevelData->Level = Level;
		LevelData->RequireExperience = RequireExperience;
		LevelData->TotalExperience = TotalExperience;

		_LevelDatas.insert(pair<int32, st_LevelData*>(LevelData->Level, LevelData));
	}
}

void CDataManager::LoadDataMonster(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["Monsters"].GetArray())
	{
		st_MonsterData* MonsterData = new st_MonsterData();

		int MonsterDataId = Filed["MonsterDataId"].GetInt();
		string Name = Filed["Name"].GetString();

		MonsterData->MonsterDataId = MonsterDataId;
		MonsterData->MonsterName = Name;

		for (auto& MonsterStatInfoFiled : Filed["MonsterStatInfo"].GetArray())
		{
			int Level = MonsterStatInfoFiled["Level"].GetInt();
			int MaxHP = MonsterStatInfoFiled["MaxHP"].GetInt();
			int MaxMP = MonsterStatInfoFiled["MaxMP"].GetInt();
			int MinMeleeAttackDamage = MonsterStatInfoFiled["MinMeleeAttackDamage"].GetInt();
			int MaxMeleeAttackDamage = MonsterStatInfoFiled["MaxMeleeAttackDamage"].GetInt();
			int16 MeleeAttackHitRate = MonsterStatInfoFiled["MeleeAttackHitRate"].GetInt();
			int16 MagicDamage = (int16)MonsterStatInfoFiled["MagicDamage"].GetInt();
			int16 MagicHitRate = (int16)MonsterStatInfoFiled["MagicHitRate"].GetInt();
			int Defence = MonsterStatInfoFiled["Defence"].GetInt();
			int16 EvasionRate = (int16)MonsterStatInfoFiled["EvasionRate"].GetInt();
			int16 MeleeCriticalPoint = (int16)MonsterStatInfoFiled["MeleeCriticalPoint"].GetInt();
			int16 MagicCriticalPoint = (int16)MonsterStatInfoFiled["MagicCriticalPoint"].GetInt();
			float Speed = MonsterStatInfoFiled["Speed"].GetFloat();
			int SearchCellDistance = MonsterStatInfoFiled["SearchCellDistance"].GetInt();
			int ChaseCellDistance = MonsterStatInfoFiled["ChaseCellDistance"].GetInt();
			int SearchTick = MonsterStatInfoFiled["SearchTick"].GetInt();
			int PatrolTick = MonsterStatInfoFiled["PatrolTick"].GetInt();
			int AttackTick = MonsterStatInfoFiled["AttackTick"].GetInt();
			int AttackRange = MonsterStatInfoFiled["AttackRange"].GetInt();
			int16 GetDPPoint = (int16)MonsterStatInfoFiled["GetDPPoint"].GetInt();
			int GetExpPoint = MonsterStatInfoFiled["GetExpPoint"].GetInt();

			MonsterData->MonsterStatInfo.Level = Level;
			MonsterData->MonsterStatInfo.MaxHP = MaxHP;
			MonsterData->MonsterStatInfo.MinMeleeAttackDamage = MinMeleeAttackDamage;
			MonsterData->MonsterStatInfo.MaxMeleeAttackDamage = MaxMeleeAttackDamage;
			MonsterData->MonsterStatInfo.MeleeAttackHitRate = MeleeAttackHitRate;
			MonsterData->MonsterStatInfo.MagicDamage = MagicDamage;
			MonsterData->MonsterStatInfo.MagicHitRate = MagicHitRate;
			MonsterData->MonsterStatInfo.Defence = Defence;
			MonsterData->MonsterStatInfo.EvasionRate = EvasionRate;
			MonsterData->MonsterStatInfo.MeleeCriticalPoint = MeleeCriticalPoint;
			MonsterData->MonsterStatInfo.MagicCriticalPoint = MagicCriticalPoint;
			MonsterData->MonsterStatInfo.Speed = Speed;
			MonsterData->MonsterStatInfo.SearchCellDistance = SearchCellDistance;
			MonsterData->MonsterStatInfo.ChaseCellDistance = ChaseCellDistance;
			MonsterData->SearchTick = SearchTick;
			MonsterData->PatrolTick = PatrolTick;
			MonsterData->AttackTick = AttackTick;
			MonsterData->MonsterStatInfo.AttackRange = AttackRange;
			MonsterData->GetDPPoint = GetDPPoint;
			MonsterData->GetExpPoint = GetExpPoint;
		}

		for (auto& DropDataFiled : Filed["MonsterDropData"].GetArray())
		{
			int Probability = DropDataFiled["Probability"].GetInt();
			string DropItemSmallCategory = DropDataFiled["DropItemSmallCategory"].GetString();
			int8 MinCount = (int8)(DropDataFiled["MinCount"].GetInt());
			int16 MaxCount = (int16)(DropDataFiled["MaxCount"].GetInt());

			st_DropData DropData;
			
			if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL;
			}
			else if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_LEATHER")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER;
			}
			else if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN;
			}			

			DropData.Probability = Probability;
			DropData.MinCount = MinCount;
			DropData.MaxCount = MaxCount;

			MonsterData->DropItems.push_back(DropData);
		}

		_Monsters.insert(pair<int32, st_MonsterData*>(MonsterData->MonsterDataId, MonsterData));
	}
}

void CDataManager::LoadDataSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["PlayerMeleeSkills"].GetArray())
	{
		string MeleeSkillLargeCategory = Filed["SKillLargeCategory"].GetString();		
		for (auto& PlayerMeleeSkillListFiled : Filed["PlayerMeleeSkillList"].GetArray())
		{
			st_SkillData* PlayerMeleeSkillData = new st_SkillData();
			PlayerMeleeSkillData->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PLAYER_MELEE;

			string SkillType = PlayerMeleeSkillListFiled["SkillType"].GetString();
			string SkillName = PlayerMeleeSkillListFiled["SkillName"].GetString();
			int SkillCoolTime = PlayerMeleeSkillListFiled["SkillCoolTime"].GetInt();
			int SkillCastingTime = PlayerMeleeSkillListFiled["SkillCastingTime"].GetInt();
			int SkillDistance = PlayerMeleeSkillListFiled["SkillDistance"].GetInt();
			string SkillImagePath = PlayerMeleeSkillListFiled["SkillThumbnailImagePath"].GetString();

			if (SkillType == "SKILL_NORMAL")
			{
				PlayerMeleeSkillData->SkillType = en_SkillType::SKILL_NORMAL;
			}
			else if (SkillType == "SKILL_KNIGHT_FIERCE_ATTACK")
			{
				PlayerMeleeSkillData->SkillType = en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK;
			}
			else if (SkillType == "SKILL_KNIGHT_CONVERSION_ATTACK")
			{
				PlayerMeleeSkillData->SkillType = en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK;
			}
			else if (SkillType == "SKILL_KNIGHT_SHAEHONE")
			{
				PlayerMeleeSkillData->SkillType = en_SkillType::SKILL_KNIGHT_SHAEHONE;
			}
			else if (SkillType == "SKILL_KNIGHT_CHOHONE")
			{
				PlayerMeleeSkillData->SkillType = en_SkillType::SKILL_KNIGHT_CHOHONE;
			}
			else if (SkillType == "SKILL_KNIGHT_SMASH_WAVE")
			{
				PlayerMeleeSkillData->SkillType = en_SkillType::SKILL_KNIGHT_SMASH_WAVE;
			}	

			PlayerMeleeSkillData->SkillName = SkillName;
			PlayerMeleeSkillData->SkillCoolTime = SkillCoolTime;
			PlayerMeleeSkillData->SkillCastingTime = SkillCastingTime;
			PlayerMeleeSkillData->SkillDistance = SkillDistance;
			PlayerMeleeSkillData->SkillThumbnailImagePath = SkillImagePath;

			_PlayerMeleeSkills.insert(pair<int16, st_SkillData*>((int16)PlayerMeleeSkillData->SkillType, PlayerMeleeSkillData));
		}		
	}

	for (auto& Filed : Document["PlayerMagicSkills"].GetArray())
	{
		string MagicSkillLargeCategory = Filed["SKillLargeCategory"].GetString();
		for (auto& PlayerMagicSkillListFiled : Filed["PlayerMagicSkillList"].GetArray())
		{
			st_SkillData* PlayerMagicSkillData = new st_SkillData();
			PlayerMagicSkillData->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PLAYER_MAGIC;

			string SkillType = PlayerMagicSkillListFiled["SkillType"].GetString();
			string SkillName = PlayerMagicSkillListFiled["SkillName"].GetString();
			int SkillCoolTime = PlayerMagicSkillListFiled["SkillCoolTime"].GetInt();
			int SkillCastingTime = PlayerMagicSkillListFiled["SkillCastingTime"].GetInt();
			int SkillDistance = PlayerMagicSkillListFiled["SkillDistance"].GetInt();
			string SkillImagePath = PlayerMagicSkillListFiled["SkillThumbnailImagePath"].GetString();

			if (SkillType == "SKILL_KNIGHT_CHARGE_POSE")
			{
				PlayerMagicSkillData->SkillType = en_SkillType::SKILL_KNIGHT_CHARGE_POSE;
			}
			else if (SkillType == "SKILL_SHAMNA_FLAME_HARPOON")
			{
				PlayerMagicSkillData->SkillType = en_SkillType::SKILL_SHAMAN_FLAME_HARPOON;
			}
			else if (SkillType == "SKILL_SHAMAN_HEALING_LIGHT")
			{
				PlayerMagicSkillData->SkillType = en_SkillType::SKILL_SHAMAN_HEALING_LIGHT;
			}
			else if (SkillType == "SKILL_SHAMAN_HEALING_WIND")
			{
				PlayerMagicSkillData->SkillType = en_SkillType::SKILL_SHAMAN_HEALING_WIND;
			}
			else if (SkillType == "SKILL_SHAMAN_HELL_FIRE")
			{
				PlayerMagicSkillData->SkillType = en_SkillType::SKILL_SHAMAN_HELL_FIRE;
			}
			else if (SkillType == "SKILL_SHOCK_RELEASE")
			{
				PlayerMagicSkillData->SkillType = en_SkillType::SKILL_SHOCK_RELEASE;
			}

			PlayerMagicSkillData->SkillName = SkillName;
			PlayerMagicSkillData->SkillCoolTime = SkillCoolTime;
			PlayerMagicSkillData->SkillCastingTime = SkillCastingTime;
			PlayerMagicSkillData->SkillDistance = SkillDistance;
			PlayerMagicSkillData->SkillThumbnailImagePath = SkillImagePath;

			_PlayerMagicSkills.insert(pair<int16, st_SkillData*>((int16)PlayerMagicSkillData->SkillType, PlayerMagicSkillData));
		}
	}

	for (auto& Filed : Document["MonsterMeleeSkills"].GetArray())
	{
		string MonsterMeleeSkillLargeCategory = Filed["SKillLargeCategory"].GetString();
		for (auto& MonsterMeleeSkillListFiled : Filed["MonsterMeleeSkillList"].GetArray())
		{
			st_SkillData* MonsterMeleeSkillData = new st_SkillData();
			MonsterMeleeSkillData->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_MONSTER_MELEE;

			string SkillType = MonsterMeleeSkillListFiled["SkillType"].GetString();
			string SkillName = MonsterMeleeSkillListFiled["SkillName"].GetString();
			int SkillCoolTime = MonsterMeleeSkillListFiled["SkillCoolTime"].GetInt();
			int SkillCastingTime = MonsterMeleeSkillListFiled["SkillCastingTime"].GetInt();
			int SkillDistance = MonsterMeleeSkillListFiled["SkillDistance"].GetInt();
			string SkillImagePath = MonsterMeleeSkillListFiled["SkillThumbnailImagePath"].GetString();

			if (SkillType == "SKILL_SLIME_NORMAL")
			{
				MonsterMeleeSkillData->SkillType = en_SkillType::SKILL_KNIGHT_CHARGE_POSE;
			}
			else if (SkillType == "SKILL_BEAR_NORMAL")
			{
				MonsterMeleeSkillData->SkillType = en_SkillType::SKILL_SHAMAN_FLAME_HARPOON;
			}

			MonsterMeleeSkillData->SkillName = SkillName;
			MonsterMeleeSkillData->SkillCoolTime = SkillCoolTime;
			MonsterMeleeSkillData->SkillCastingTime = SkillCastingTime;
			MonsterMeleeSkillData->SkillDistance = SkillDistance;
			MonsterMeleeSkillData->SkillThumbnailImagePath = SkillImagePath;

			_PlayerMagicSkills.insert(pair<int16, st_SkillData*>((int16)MonsterMeleeSkillData->SkillType, MonsterMeleeSkillData));
		}
	}

	for (auto& Filed : Document["MonsterMagicSkills"].GetArray())
	{
		string MonsterMagicSkillLargeCategory = Filed["SKillLargeCategory"].GetString();
		for (auto& MonsterMagicSkillListFiled : Filed["MonsterMeleeSkillList"].GetArray())
		{
			st_SkillData* MonsterMagicSkillData = new st_SkillData();
			MonsterMagicSkillData->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_MONSTER_MAGIC;

			string SkillType = MonsterMagicSkillListFiled["SkillType"].GetString();
			string SkillName = MonsterMagicSkillListFiled["SkillName"].GetString();
			int SkillCoolTime = MonsterMagicSkillListFiled["SkillCoolTime"].GetInt();
			int SkillCastingTime = MonsterMagicSkillListFiled["SkillCastingTime"].GetInt();
			int SkillDistance = MonsterMagicSkillListFiled["SkillDistance"].GetInt();
			string SkillImagePath = MonsterMagicSkillListFiled["SkillThumbnailImagePath"].GetString();
			
			MonsterMagicSkillData->SkillName = SkillName;
			MonsterMagicSkillData->SkillCoolTime = SkillCoolTime;
			MonsterMagicSkillData->SkillCastingTime = SkillCastingTime;
			MonsterMagicSkillData->SkillDistance = SkillDistance;
			MonsterMagicSkillData->SkillThumbnailImagePath = SkillImagePath;

			_PlayerMagicSkills.insert(pair<int16, st_SkillData*>((int16)MonsterMagicSkillData->SkillType, MonsterMagicSkillData));
		}
	}

	st_SkillData* PlayerMagicSkillData = new st_SkillData();
}

void CDataManager::LoadDataEnvironment(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["Environments"].GetArray())
	{
		st_EnvironmentData* EnvironmentData = new st_EnvironmentData();

		int EnvironmentDataId = Filed["EnvironmentDataId"].GetInt();
		string Name = Filed["Name"].GetString();

		EnvironmentData->EnvironmentDataId = EnvironmentDataId;
		EnvironmentData->EnvironmentName = Name;

		for (auto& EnvironmentStatInfoFiled : Filed["EnvironmentStatInfo"].GetArray())
		{
			int Level = EnvironmentStatInfoFiled["Level"].GetInt();
			int MaxHP = EnvironmentStatInfoFiled["MaxHP"].GetInt();

			EnvironmentData->Level = Level;
			EnvironmentData->MaxHP = MaxHP;
		}

		for (auto& DropDataFiled : Filed["EnvironmentDropData"].GetArray())
		{
			int Probability = DropDataFiled["Probability"].GetInt();
			string DropItemSmallCategory = DropDataFiled["DropItemSmallCategory"].GetString();
			int8 MinCount = (int8)(DropDataFiled["MinCount"].GetInt());
			int16 MaxCount = (int16)(DropDataFiled["MaxCount"].GetInt());

			st_DropData DropData;

			if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_STONE")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE;
			}
			else if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG;
			}			

			DropData.Probability = Probability;			
			DropData.MinCount = MinCount;
			DropData.MaxCount = MaxCount;

			EnvironmentData->DropItems.push_back(DropData);
		}

		_Environments.insert(pair<int32, st_EnvironmentData*>(EnvironmentData->EnvironmentDataId, EnvironmentData));
	}
}

void CDataManager::LoadDataCrafting(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["CraftingDatas"].GetArray())
	{
		st_CraftingItemCategoryData* CraftingData = new st_CraftingItemCategoryData();

		string CraftingItemLargeCategory = Filed["CraftingCompleteItemLargeCategory"].GetString();
		string CraftingTypeName = Filed["CraftingTypeName"].GetString();

		if (CraftingItemLargeCategory == "ITEM_LARGE_CATEGORY_WEAPON")
		{
			CraftingData->CraftingType = en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON;
		}		
		else if (CraftingItemLargeCategory == "ITEM_LARGE_CATEGORY_ARMOR")
		{
			CraftingData->CraftingType = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARMOR;
		}
		else if (CraftingItemLargeCategory == "ITEM_LARGE_CATEGORY_MATERIAL")
		{
			CraftingData->CraftingType = en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL;
		}

		CraftingData->CraftingTypeName = CraftingTypeName;

		for (auto& CraftingCompleteItemFiled : Filed["CraftingCompleteItem"].GetArray())
		{
			st_CraftingCompleteItemData CraftingCompleteItemData;

			string CraftingCompleteItemMediumCategory = CraftingCompleteItemFiled["CraftingCompleteItemMediumCategory"].GetString();
			string CraftingCompleteItemSmallCategory = CraftingCompleteItemFiled["CraftingCompleteItemSmallCategory"].GetString();
					
			string CraftingCompleteItemName = CraftingCompleteItemFiled["CraftingCompleteItemName"].GetString();
			string CraftingCompleteItemThumbnailImagePath = CraftingCompleteItemFiled["CraftingCompleteItemThumbnailImagePath"].GetString();

			if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD")
			{
				CraftingCompleteItemData.CraftingCompleteItemDataId = en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD")
			{
				CraftingCompleteItemData.CraftingCompleteItemDataId = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_WOOD;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER")
			{
				CraftingCompleteItemData.CraftingCompleteItemDataId = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER")
			{
				CraftingCompleteItemData.CraftingCompleteItemDataId = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK")
			{
				CraftingCompleteItemData.CraftingCompleteItemDataId = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_YARN")
			{
				CraftingCompleteItemData.CraftingCompleteItemDataId = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN;
			}
			
			CraftingCompleteItemData.CraftingCompleteName = CraftingCompleteItemName;
			CraftingCompleteItemData.CraftingCompleteThumbnailImagePath = CraftingCompleteItemThumbnailImagePath;

			for (auto& CraftingMaterialFiled : CraftingCompleteItemFiled["CraftingMaterial"].GetArray())
			{
				st_CraftingMaterialItemData CraftingMaterialData;

				string MaterialSmallCategory = CraftingMaterialFiled["MaterialSmallCategory"].GetString();
				string MaterialName = CraftingMaterialFiled["MaterialName"].GetString();
				int16 MaterialCount = (int16)CraftingMaterialFiled["MaterialCount"].GetInt();
				string MaterialThumbnailImagePath = CraftingMaterialFiled["MaterialThumbnailImagePath"].GetString();

				if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_LEATHER")
				{
					CraftingMaterialData.MaterialDataId = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER;
				}
				else if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG")
				{
					CraftingMaterialData.MaterialDataId = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG;
				}
				else if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK")
				{
					CraftingMaterialData.MaterialDataId = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK;
				}
				else if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_YARN")
				{
					CraftingMaterialData.MaterialDataId = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN;
				}			
				
				CraftingMaterialData.MaterialName = MaterialName;
				CraftingMaterialData.MaterialCount = MaterialCount;
				CraftingMaterialData.MaterialThumbnailImagePath = MaterialThumbnailImagePath;

				CraftingCompleteItemData.CraftingMaterials.push_back(CraftingMaterialData);
			}

			CraftingData->CraftingCompleteItems.push_back(CraftingCompleteItemData);
		}

		_CraftingData.insert(pair<int8, st_CraftingItemCategoryData*>((int8)CraftingData->CraftingType, CraftingData));
	}
}
