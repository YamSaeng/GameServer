#include "pch.h"
#include "DataManager.h"

void CDataManager::LoadDataItem(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["Weapons"].GetArray())
	{
		for (auto& WeaponListFiled : Filed["WeaponList"].GetArray())
		{
			string MediumCategory = WeaponListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = WeaponListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = WeaponListFiled["ItemObjectType"].GetString();
			string ItemExplain = WeaponListFiled["ItemExplain"].GetString();
			string ItemName = WeaponListFiled["ItemName"].GetString();
			int32 ItemWidth = WeaponListFiled["ItemWidth"].GetInt();
			int32 ItemHeight = WeaponListFiled["ItemHeight"].GetInt();
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
			WeaponItemData->ItemWidth = ItemWidth;
			WeaponItemData->ItemHeight = ItemHeight;
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
		for (auto& ArmorListFiled : Filed["ArmorList"].GetArray())
		{
			string MediumCategory = ArmorListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = ArmorListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = ArmorListFiled["ItemObjectType"].GetString();
			string ItemExplain = ArmorListFiled["ItemExplain"].GetString();
			string ItemName = ArmorListFiled["ItemName"].GetString();
			int32 ItemWidth = ArmorListFiled["ItemWidth"].GetInt();
			int32 ItemHeight = ArmorListFiled["ItemHeight"].GetInt();
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
			ArmorItemData->ItemWidth = ItemWidth;
			ArmorItemData->ItemHeight = ItemHeight;
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
		for (auto& PotionDataListFiled : Filed["PotionList"].GetArray())
		{
			string MediumCategory = PotionDataListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = PotionDataListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = PotionDataListFiled["ItemObjectType"].GetString();
			string ItemExplain = PotionDataListFiled["ItemExplain"].GetString();
			string ItemName = PotionDataListFiled["ItemName"].GetString();
			int32 ItemWidth = PotionDataListFiled["ItemWidth"].GetInt();
			int32 ItemHeight = PotionDataListFiled["ItemHeight"].GetInt();
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
			PotionItemData->ItemWidth = ItemWidth;
			PotionItemData->ItemHeight = ItemHeight;
			PotionItemData->ItemMinDamage = 0;
			PotionItemData->ItemMaxDamage = 0;
			PotionItemData->ItemDefence = 0;
			PotionItemData->ItemMaxCount = ItemMaxCount;
			PotionItemData->ItemThumbnailImagePath = ImageFilePath;

			_Consumables.insert(pair<int16, st_ConsumableData*>((int16)PotionItemData->SmallItemCategory, PotionItemData));
		}
				
		for (auto& SkillBookDataListFiled : Filed["SkillBookList"].GetArray())
		{
			string MediumCategory = SkillBookDataListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = SkillBookDataListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = SkillBookDataListFiled["ItemObjectType"].GetString();
			string ItemExplain = SkillBookDataListFiled["ItemExplain"].GetString();
			string ItemName = SkillBookDataListFiled["ItemName"].GetString();
			int32 ItemWidth = SkillBookDataListFiled["ItemWidth"].GetInt();
			int32 ItemHeight = SkillBookDataListFiled["ItemHeight"].GetInt();
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
				SkillBookItemData->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK;
				SkillBookItemData->SkillType = en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK;
				SkillBookItemData->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK;
				SkillBookItemData->SkillType = en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK;
				SkillBookItemData->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK;
				SkillBookItemData->SkillType = en_SkillType::SKILL_KNIGHT_SHAEHONE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK;
				SkillBookItemData->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK;
				SkillBookItemData->SkillType = en_SkillType::SKILL_KNIGHT_CHOHONE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK;
				SkillBookItemData->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK;
				SkillBookItemData->SkillType = en_SkillType::SKILL_KNIGHT_SMASH_WAVE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE;
				SkillBookItemData->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_BUF;
				SkillBookItemData->SkillType = en_SkillType::SKILL_KNIGHT_CHARGE_POSE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON;
				SkillBookItemData->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK;
				SkillBookItemData->SkillType = en_SkillType::SKILL_SHAMAN_FLAME_HARPOON;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE;
				SkillBookItemData->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK;
				SkillBookItemData->SkillType = en_SkillType::SKILL_SHAMAN_HELL_FIRE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEOGRY_SKILLBOOK_SHAMAN_HEALING_LIGHT")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEOGRY_SKILLBOOK_SHAMAN_HEALING_LIGHT;
				SkillBookItemData->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL;
				SkillBookItemData->SkillType = en_SkillType::SKILL_TAIOIST_HEALING_LIGHT;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HEALING_WIND")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HEALING_WIND;
				SkillBookItemData->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL;
				SkillBookItemData->SkillType = en_SkillType::SKILL_TAIOIST_HEALING_WIND;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE;
				SkillBookItemData->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF;
				SkillBookItemData->SkillType = en_SkillType::SKILL_SHOCK_RELEASE;
			}

			if (ItemObjectType == "OBJECT_ITEM_CONSUMABLE_SKILL_BOOK")
			{
				SkillBookItemData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK;
			}

			SkillBookItemData->ItemExplain = ItemExplain;
			SkillBookItemData->ItemName = ItemName;
			SkillBookItemData->ItemWidth = ItemWidth;
			SkillBookItemData->ItemHeight = ItemHeight;
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
		for (auto& MaterialDataListFiled : Filed["MaterialList"].GetArray())
		{
			string MediumCategory = MaterialDataListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = MaterialDataListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = MaterialDataListFiled["ItemObjectType"].GetString();
			string ItemExplain = MaterialDataListFiled["ItemExplain"].GetString();
			string ItemName = MaterialDataListFiled["ItemName"].GetString();
			int32 ItemWidth = MaterialDataListFiled["ItemWidth"].GetInt();
			int32 ItemHeight = MaterialDataListFiled["ItemHeight"].GetInt();
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
			MaterialData->ItemWidth = ItemWidth;
			MaterialData->ItemHeight = ItemHeight;
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
			int AutoRecoveryHPPercent = PlayerWarriorCharacterFiled["AutoRecoveryHPPercent"].GetInt();
			int AutoRecoveryMPPercent = PlayerWarriorCharacterFiled["AutoRecoveryMPPercent"].GetInt();
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

			WarriorStatusData->PlayerType = en_GameObjectType::OBJECT_WARRIOR_PLAYER;

			WarriorStatusData->Level = Level;
			WarriorStatusData->MaxHP = MaxHP;
			WarriorStatusData->MaxMP = MaxMP;
			WarriorStatusData->MaxDP = MaxDP;
			WarriorStatusData->AutoRecoveryHPPercent = AutoRecoveryHPPercent;
			WarriorStatusData->AutoRecoveryMPPercent = AutoRecoveryMPPercent;
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
			int AutoRecoveryHPPercent = PlayerShamanCharacterFiled["AutoRecoveryHPPercent"].GetInt();
			int AutoRecoveryMPPercent = PlayerShamanCharacterFiled["AutoRecoveryMPPercent"].GetInt();
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
			ShamanStatusData->AutoRecoveryHPPercent = AutoRecoveryHPPercent;
			ShamanStatusData->AutoRecoveryMPPercent = AutoRecoveryMPPercent;
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

	for (auto& Filed : Document["PlayerTaioistCharacterStatus"].GetArray())
	{
		string PlayerType = Filed["PlayerType"].GetString();

		for (auto& PlayerTaioistCharacterFiled : Filed["PlayerTaioistCharacterLevelDataList"].GetArray())
		{
			int Level = PlayerTaioistCharacterFiled["Level"].GetInt();
			int MaxHP = PlayerTaioistCharacterFiled["MaxHP"].GetInt();
			int MaxMP = PlayerTaioistCharacterFiled["MaxMP"].GetInt();
			int MaxDP = PlayerTaioistCharacterFiled["MaxDP"].GetInt();
			int AutoRecoveryHPPercent = PlayerTaioistCharacterFiled["AutoRecoveryHPPercent"].GetInt();
			int AutoRecoveryMPPercent = PlayerTaioistCharacterFiled["AutoRecoveryMPPercent"].GetInt();
			int MinMeleeAttackDamage = PlayerTaioistCharacterFiled["MinMeleeAttackDamage"].GetInt();
			int MaxMeleeAttackDamage = PlayerTaioistCharacterFiled["MaxMeleeAttackDamage"].GetInt();
			int16 MeleeAttackHitRate = (int16)PlayerTaioistCharacterFiled["MeleeAttackHitRate"].GetInt();
			int16 MagicDamage = (int16)PlayerTaioistCharacterFiled["MagicDamage"].GetInt();
			int16 MagicHitRate = (int16)PlayerTaioistCharacterFiled["MagicHitRate"].GetInt();
			int Defence = PlayerTaioistCharacterFiled["Defence"].GetInt();
			int16 EvasionRate = PlayerTaioistCharacterFiled["EvasionRate"].GetInt();
			int16 MeleeCriticalPoint = (int16)(PlayerTaioistCharacterFiled["MeleeCriticalPoint"].GetInt());
			int16 MagicCriticalPoint = (int16)(PlayerTaioistCharacterFiled["MagicCriticalPoint"].GetInt());
			float Speed = PlayerTaioistCharacterFiled["Speed"].GetFloat();

			st_ObjectStatusData* TaioistStatusData = new st_ObjectStatusData();

			TaioistStatusData->PlayerType = en_GameObjectType::OBJECT_TAIOIST_PLAYER;

			TaioistStatusData->Level = Level;
			TaioistStatusData->MaxHP = MaxHP;
			TaioistStatusData->MaxMP = MaxMP;
			TaioistStatusData->MaxDP = MaxDP;
			TaioistStatusData->AutoRecoveryHPPercent = AutoRecoveryHPPercent;
			TaioistStatusData->AutoRecoveryMPPercent = AutoRecoveryMPPercent;
			TaioistStatusData->MinMeleeAttackDamage = MinMeleeAttackDamage;
			TaioistStatusData->MaxMeleeAttackDamage = MaxMeleeAttackDamage;
			TaioistStatusData->MeleeAttackHitRate = MeleeAttackHitRate;
			TaioistStatusData->MagicDamage = MagicDamage;
			TaioistStatusData->MagicHitRate = MagicHitRate;
			TaioistStatusData->Defence = Defence;
			TaioistStatusData->EvasionRate = EvasionRate;
			TaioistStatusData->MeleeCriticalPoint = MeleeCriticalPoint;
			TaioistStatusData->MagicCriticalPoint = MagicCriticalPoint;
			TaioistStatusData->Speed = Speed;

			_TaioistStatus.insert(pair<int32, st_ObjectStatusData*>(TaioistStatusData->Level, TaioistStatusData));
		}
	}

	for (auto& Filed : Document["PlayerThiefCharacterStatus"].GetArray())
	{
		string PlayerType = Filed["PlayerType"].GetString();

		for (auto& PlayerThiefCharacterFiled : Filed["PlayerThiefCharacterLevelDataList"].GetArray())
		{
			int Level = PlayerThiefCharacterFiled["Level"].GetInt();
			int MaxHP = PlayerThiefCharacterFiled["MaxHP"].GetInt();
			int MaxMP = PlayerThiefCharacterFiled["MaxMP"].GetInt();
			int MaxDP = PlayerThiefCharacterFiled["MaxDP"].GetInt();
			int AutoRecoveryHPPercent = PlayerThiefCharacterFiled["AutoRecoveryHPPercent"].GetInt();
			int AutoRecoveryMPPercent = PlayerThiefCharacterFiled["AutoRecoveryMPPercent"].GetInt();
			int MinMeleeAttackDamage = PlayerThiefCharacterFiled["MinMeleeAttackDamage"].GetInt();
			int MaxMeleeAttackDamage = PlayerThiefCharacterFiled["MaxMeleeAttackDamage"].GetInt();
			int16 MeleeAttackHitRate = (int16)PlayerThiefCharacterFiled["MeleeAttackHitRate"].GetInt();
			int16 MagicDamage = (int16)PlayerThiefCharacterFiled["MagicDamage"].GetInt();
			int16 MagicHitRate = (int16)PlayerThiefCharacterFiled["MagicHitRate"].GetInt();
			int Defence = PlayerThiefCharacterFiled["Defence"].GetInt();
			int16 EvasionRate = PlayerThiefCharacterFiled["EvasionRate"].GetInt();
			int16 MeleeCriticalPoint = (int16)(PlayerThiefCharacterFiled["MeleeCriticalPoint"].GetInt());
			int16 MagicCriticalPoint = (int16)(PlayerThiefCharacterFiled["MagicCriticalPoint"].GetInt());
			float Speed = PlayerThiefCharacterFiled["Speed"].GetFloat();

			st_ObjectStatusData* ThiefStatusData = new st_ObjectStatusData();

			ThiefStatusData->PlayerType = en_GameObjectType::OBJECT_TAIOIST_PLAYER;

			ThiefStatusData->Level = Level;
			ThiefStatusData->MaxHP = MaxHP;
			ThiefStatusData->MaxMP = MaxMP;
			ThiefStatusData->MaxDP = MaxDP;
			ThiefStatusData->AutoRecoveryHPPercent = AutoRecoveryHPPercent;
			ThiefStatusData->AutoRecoveryMPPercent = AutoRecoveryMPPercent;
			ThiefStatusData->MinMeleeAttackDamage = MinMeleeAttackDamage;
			ThiefStatusData->MaxMeleeAttackDamage = MaxMeleeAttackDamage;
			ThiefStatusData->MeleeAttackHitRate = MeleeAttackHitRate;
			ThiefStatusData->MagicDamage = MagicDamage;
			ThiefStatusData->MagicHitRate = MagicHitRate;
			ThiefStatusData->Defence = Defence;
			ThiefStatusData->EvasionRate = EvasionRate;
			ThiefStatusData->MeleeCriticalPoint = MeleeCriticalPoint;
			ThiefStatusData->MagicCriticalPoint = MagicCriticalPoint;
			ThiefStatusData->Speed = Speed;

			_ThiefStatus.insert(pair<int32, st_ObjectStatusData*>(ThiefStatusData->Level, ThiefStatusData));
		}
	}

	for (auto& Filed : Document["PlayerArcherCharacterStatus"].GetArray())
	{
		string PlayerType = Filed["PlayerType"].GetString();

		for (auto& PlayerArcherCharacterFiled : Filed["PlayerArcherCharacterLevelDataList"].GetArray())
		{
			int Level = PlayerArcherCharacterFiled["Level"].GetInt();
			int MaxHP = PlayerArcherCharacterFiled["MaxHP"].GetInt();
			int MaxMP = PlayerArcherCharacterFiled["MaxMP"].GetInt();
			int MaxDP = PlayerArcherCharacterFiled["MaxDP"].GetInt();
			int AutoRecoveryHPPercent = PlayerArcherCharacterFiled["AutoRecoveryHPPercent"].GetInt();
			int AutoRecoveryMPPercent = PlayerArcherCharacterFiled["AutoRecoveryMPPercent"].GetInt();
			int MinMeleeAttackDamage = PlayerArcherCharacterFiled["MinMeleeAttackDamage"].GetInt();
			int MaxMeleeAttackDamage = PlayerArcherCharacterFiled["MaxMeleeAttackDamage"].GetInt();
			int16 MeleeAttackHitRate = (int16)PlayerArcherCharacterFiled["MeleeAttackHitRate"].GetInt();
			int16 MagicDamage = (int16)PlayerArcherCharacterFiled["MagicDamage"].GetInt();
			int16 MagicHitRate = (int16)PlayerArcherCharacterFiled["MagicHitRate"].GetInt();
			int Defence = PlayerArcherCharacterFiled["Defence"].GetInt();
			int16 EvasionRate = PlayerArcherCharacterFiled["EvasionRate"].GetInt();
			int16 MeleeCriticalPoint = (int16)(PlayerArcherCharacterFiled["MeleeCriticalPoint"].GetInt());
			int16 MagicCriticalPoint = (int16)(PlayerArcherCharacterFiled["MagicCriticalPoint"].GetInt());
			float Speed = PlayerArcherCharacterFiled["Speed"].GetFloat();

			st_ObjectStatusData* ArcherStatusData = new st_ObjectStatusData();

			ArcherStatusData->PlayerType = en_GameObjectType::OBJECT_TAIOIST_PLAYER;

			ArcherStatusData->Level = Level;
			ArcherStatusData->MaxHP = MaxHP;
			ArcherStatusData->MaxMP = MaxMP;
			ArcherStatusData->MaxDP = MaxDP;
			ArcherStatusData->AutoRecoveryHPPercent = AutoRecoveryHPPercent;
			ArcherStatusData->AutoRecoveryMPPercent = AutoRecoveryMPPercent;
			ArcherStatusData->MinMeleeAttackDamage = MinMeleeAttackDamage;
			ArcherStatusData->MaxMeleeAttackDamage = MaxMeleeAttackDamage;
			ArcherStatusData->MeleeAttackHitRate = MeleeAttackHitRate;
			ArcherStatusData->MagicDamage = MagicDamage;
			ArcherStatusData->MagicHitRate = MagicHitRate;
			ArcherStatusData->Defence = Defence;
			ArcherStatusData->EvasionRate = EvasionRate;
			ArcherStatusData->MeleeCriticalPoint = MeleeCriticalPoint;
			ArcherStatusData->MagicCriticalPoint = MagicCriticalPoint;
			ArcherStatusData->Speed = Speed;			

			_ArcherStatus.insert(pair<int32, st_ObjectStatusData*>(ArcherStatusData->Level, ArcherStatusData));
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

void CDataManager::LoadDataPublicSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["PublicSkills"].GetArray())
	{
		for (auto& PublicSkillListFiled : Filed["PublicSkillList"].GetArray())
		{
			for (auto& PublicAttackSkillListFiled : PublicSkillListFiled["PublicAttackSkillList"].GetArray())
			{
				st_AttackSkillData* PublicAttackSkill = new st_AttackSkillData();
				PublicAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
				PublicAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK;

				string SkillType = PublicAttackSkillListFiled["SkillType"].GetString();
				string SkillName = PublicAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = PublicAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = PublicAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = PublicAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = PublicAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = PublicAttackSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = PublicAttackSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = PublicAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = PublicAttackSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "SKILL_DEFAULT_ATTACK")
				{
					PublicAttackSkill->SkillType = en_SkillType::SKILL_DEFAULT_ATTACK;
				}

				PublicAttackSkill->SkillName = SkillName;
				PublicAttackSkill->SkillLevel = SkillLevel;
				PublicAttackSkill->SkillMinDamage = SkillMinDamage;
				PublicAttackSkill->SkillMaxDamage = SkillMaxDamage;
				PublicAttackSkill->SkillCoolTime = SkillCoolTime;
				PublicAttackSkill->SkillCastingTime = SkillCastingTime;
				PublicAttackSkill->SkillDistance = SkillDistance;
				PublicAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				PublicAttackSkill->SkillThumbnailImagePath = SkillImagePath;

				_PublicAttackSkillDatas.insert(pair<int16, st_AttackSkillData*>((int16)PublicAttackSkill->SkillType, PublicAttackSkill));
			}

			for (auto& PublicHealSkillListFiled : PublicSkillListFiled["PublicHealSkillList"].GetArray())
			{
				st_HealSkillData* PublicHealSkill = new st_HealSkillData();
				PublicHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
				PublicHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL;

				string SkillType = PublicHealSkillListFiled["SkillType"].GetString();
				string SkillName = PublicHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = PublicHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = PublicHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = PublicHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = PublicHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = PublicHealSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = PublicHealSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = PublicHealSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = PublicHealSkillListFiled["SkillThumbnailImagePath"].GetString();

				PublicHealSkill->SkillName = SkillName;
				PublicHealSkill->SkillLevel = SkillLevel;
				PublicHealSkill->SkillMinHealPoint = SkillMinHeal;
				PublicHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				PublicHealSkill->SkillCoolTime = SkillCoolTime;
				PublicHealSkill->SkillCastingTime = SkillCastingTime;
				PublicHealSkill->SkillDistance = SkillDistance;
				PublicHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				PublicHealSkill->SkillThumbnailImagePath = SkillImagePath;

				_PublicHealSkillDatas.insert(pair<int16, st_HealSkillData*>((int16)PublicHealSkill->SkillType, PublicHealSkill));
			}

			for (auto& PublicBufSkillListFiled : PublicSkillListFiled["PublicBufSkillList"].GetArray())
			{
				st_BufSkillData* PublicBufSkill = new st_BufSkillData();
				PublicBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
				PublicBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF;

				string SkillType = PublicBufSkillListFiled["SkillType"].GetString();
				string SkillName = PublicBufSkillListFiled["SkillName"].GetString();
				int SkillLevel = PublicBufSkillListFiled["SkillLevel"].GetInt();

				int IncreaseMinAttackPoint = PublicBufSkillListFiled["IncreaseMinAttackPoint"].GetInt();
				int IncreaseMaxAttackPoint = PublicBufSkillListFiled["IncreaseMaxAttackPoint"].GetInt();
				int IncreaseMeleeAttackSpeedPoint = PublicBufSkillListFiled["IncreaseMeleeAttackSpeedPoint"].GetInt();
				int16 IncreaseMeleeAttackHitRate = (int16)PublicBufSkillListFiled["IncreaseMeleeAttackHitRate"].GetInt();
				int16 IncreaseMagicAttackPoint = (int16)PublicBufSkillListFiled["IncreaseMagicAttackPoint"].GetInt();
				int16 IncreaseMagicCastingPoint = (int16)PublicBufSkillListFiled["IncreaseMagicCastingPoint"].GetInt();
				int16 IncreaseMagicAttackHitRate = (int16)PublicBufSkillListFiled["IncreaseMagicAttackHitRate"].GetInt();
				int IncreaseDefencePoint = PublicBufSkillListFiled["IncreaseDefencePoint"].GetInt();
				int16 IncreaseEvasionRate = (int16)PublicBufSkillListFiled["IncreaseEvasionRate"].GetInt();
				int16 IncreaseMeleeCriticalPoint = (int16)PublicBufSkillListFiled["IncreaseMeleeCriticalPoint"].GetInt();
				int16 IncreaseMagicCriticalPoint = (int16)PublicBufSkillListFiled["IncreaseMagicCriticalPoint"].GetInt();
				float IncreaseSpeedPoint = PublicBufSkillListFiled["IncreaseSpeedPoint"].GetFloat();
				int16 IncreaseStatusAbnormalityResistance = (int16)PublicBufSkillListFiled["IncreaseStatusAbnormalityResistance"].GetInt();

				int SkillCoolTime = PublicBufSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = PublicBufSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = PublicBufSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = PublicBufSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = PublicBufSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "SKILL_SHOCK_RELEASE")
				{
					PublicBufSkill->SkillType = en_SkillType::SKILL_SHOCK_RELEASE;
				}

				PublicBufSkill->SkillName = SkillName;
				PublicBufSkill->SkillLevel = SkillLevel;
				PublicBufSkill->IncreaseMinAttackPoint = IncreaseMinAttackPoint;
				PublicBufSkill->IncreaseMaxAttackPoint = IncreaseMaxAttackPoint;
				PublicBufSkill->IncreaseMeleeAttackSpeedPoint = IncreaseMeleeAttackSpeedPoint;
				PublicBufSkill->IncreaseMeleeAttackHitRate = IncreaseMeleeAttackHitRate;
				PublicBufSkill->IncreaseMagicAttackPoint = IncreaseMagicAttackPoint;
				PublicBufSkill->IncreaseMagicCastingPoint = IncreaseMagicCastingPoint;
				PublicBufSkill->IncreaseMagicAttackHitRate = IncreaseMagicAttackHitRate;
				PublicBufSkill->IncreaseDefencePoint = IncreaseDefencePoint;
				PublicBufSkill->IncreaseEvasionRate = IncreaseEvasionRate;
				PublicBufSkill->IncreaseMeleeCriticalPoint = IncreaseMeleeCriticalPoint;
				PublicBufSkill->IncreaseMagicCriticalPoint = IncreaseMagicCriticalPoint;
				PublicBufSkill->IncreaseSpeedPoint = IncreaseSpeedPoint;
				PublicBufSkill->IncreaseStatusAbnormalityResistance = IncreaseStatusAbnormalityResistance;
				PublicBufSkill->SkillCoolTime = SkillCoolTime;
				PublicBufSkill->SkillCastingTime = SkillCastingTime;
				PublicBufSkill->SkillDistance = SkillDistance;
				PublicBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				PublicBufSkill->SkillThumbnailImagePath = SkillImagePath;

				_PublicBufSkillDatas.insert(pair<int16, st_BufSkillData*>((int16)PublicBufSkill->SkillType, PublicBufSkill));
			}
		}
	}
}

void CDataManager::LoadDataWarriorSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["WarriorSkills"].GetArray())
	{
		for (auto& WarriorSkillListFiled : Filed["WarriorSkillList"].GetArray())
		{
			for (auto& WarriorAttackSkillListFiled : WarriorSkillListFiled["WarriorAttackSkillList"].GetArray())
			{
				st_AttackSkillData* WarriorAttackSkill = new st_AttackSkillData();
				WarriorAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				WarriorAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK;

				string SkillType = WarriorAttackSkillListFiled["SkillType"].GetString();
				string SkillName = WarriorAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = WarriorAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = WarriorAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = WarriorAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = WarriorAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = WarriorAttackSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = WarriorAttackSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = WarriorAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();
				bool SkillDebuf = WarriorAttackSkillListFiled["SkillDebuf"].GetBool();
				int64 SkillDebufTime = WarriorAttackSkillListFiled["SkillDebufTime"].GetInt64();
				int8 SkillDebufAttackSpeed = (int8)WarriorAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)WarriorAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				bool SkillDebufStun = WarriorAttackSkillListFiled["SkillDebufStun"].GetBool();
				bool SkillDebufPushAway = WarriorAttackSkillListFiled["SkillDebufPushAway"].GetBool();
				bool SkillDebufRoot = WarriorAttackSkillListFiled["SkillDebufRoot"].GetBool();
				int64 SkillDamageOverTime = WarriorAttackSkillListFiled["SkillDebufDamageOverTime"].GetInt64();
				int8 StatusAbnormalityProbability = (int8)WarriorAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();
				string SkillImagePath = WarriorAttackSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "SKILL_KNIGHT_FIERCE_ATTACK")
				{
					WarriorAttackSkill->SkillType = en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK;
				}
				else if (SkillType == "SKILL_KNIGHT_CONVERSION_ATTACK")
				{
					WarriorAttackSkill->SkillType = en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK;
				}
				else if (SkillType == "SKILL_KNIGHT_SHAEHONE")
				{
					WarriorAttackSkill->SkillType = en_SkillType::SKILL_KNIGHT_SHAEHONE;
				}
				else if (SkillType == "SKILL_KNIGHT_CHOHONE")
				{
					WarriorAttackSkill->SkillType = en_SkillType::SKILL_KNIGHT_CHOHONE;
				}
				else if (SkillType == "SKILL_KNIGHT_SMASH_WAVE")
				{
					WarriorAttackSkill->SkillType = en_SkillType::SKILL_KNIGHT_SMASH_WAVE;
				}

				WarriorAttackSkill->SkillName = SkillName;
				WarriorAttackSkill->SkillLevel = SkillLevel;
				WarriorAttackSkill->SkillMinDamage = SkillMinDamage;
				WarriorAttackSkill->SkillMaxDamage = SkillMaxDamage;
				WarriorAttackSkill->SkillCoolTime = SkillCoolTime;
				WarriorAttackSkill->SkillCastingTime = SkillCastingTime;
				WarriorAttackSkill->SkillDistance = SkillDistance;
				WarriorAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;

				WarriorAttackSkill->SkillDebuf = SkillDebuf;
				WarriorAttackSkill->SkillDebufTime = SkillDebufTime;
				WarriorAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				WarriorAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
				WarriorAttackSkill->SkillDebufStun = SkillDebufStun;
				WarriorAttackSkill->SkillDebufPushAway = SkillDebufPushAway;
				WarriorAttackSkill->SkillDebufRoot = SkillDebufRoot;
				WarriorAttackSkill->SkillDamageOverTime = SkillDamageOverTime;
				WarriorAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;
				WarriorAttackSkill->SkillThumbnailImagePath = SkillImagePath;

				_WarriorAttackSkillDatas.insert(pair<int16, st_AttackSkillData*>((int16)WarriorAttackSkill->SkillType, WarriorAttackSkill));
			}			

			for (auto& WarriorHealSkillListFiled : WarriorSkillListFiled["WarriorHealSkillList"].GetArray())
			{
				st_HealSkillData* WarriorHealSkill = new st_HealSkillData();
				WarriorHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				WarriorHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL;

				string SkillType = WarriorHealSkillListFiled["SkillType"].GetString();
				string SkillName = WarriorHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = WarriorHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = WarriorHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = WarriorHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = WarriorHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = WarriorHealSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = WarriorHealSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = WarriorHealSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = WarriorHealSkillListFiled["SkillThumbnailImagePath"].GetString();

				WarriorHealSkill->SkillName = SkillName;
				WarriorHealSkill->SkillLevel = SkillLevel;
				WarriorHealSkill->SkillMinHealPoint = SkillMinHeal;
				WarriorHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				WarriorHealSkill->SkillCoolTime = SkillCoolTime;
				WarriorHealSkill->SkillCastingTime = SkillCastingTime;
				WarriorHealSkill->SkillDistance = SkillDistance;
				WarriorHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				WarriorHealSkill->SkillThumbnailImagePath = SkillImagePath;

				_WarriorHealSkillDatas.insert(pair<int16, st_HealSkillData*>((int16)WarriorHealSkill->SkillType, WarriorHealSkill));
			}

			for (auto& WarriorBufSkillListFiled : WarriorSkillListFiled["WarriorBufSkillList"].GetArray())
			{
				st_BufSkillData* WarriorBufSkill = new st_BufSkillData();
				WarriorBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				WarriorBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF;

				string SkillType = WarriorBufSkillListFiled["SkillType"].GetString();
				string SkillName = WarriorBufSkillListFiled["SkillName"].GetString();
				int SkillLevel = WarriorBufSkillListFiled["SkillLevel"].GetInt();

				int IncreaseMinAttackPoint = WarriorBufSkillListFiled["IncreaseMinAttackPoint"].GetInt();
				int IncreaseMaxAttackPoint = WarriorBufSkillListFiled["IncreaseMaxAttackPoint"].GetInt();
				int IncreaseMeleeAttackSpeedPoint = WarriorBufSkillListFiled["IncreaseMeleeAttackSpeedPoint"].GetInt();
				int16 IncreaseMeleeAttackHitRate = (int16)WarriorBufSkillListFiled["IncreaseMeleeAttackHitRate"].GetInt();
				int16 IncreaseMagicAttackPoint = (int16)WarriorBufSkillListFiled["IncreaseMagicAttackPoint"].GetInt();
				int16 IncreaseMagicCastingPoint = (int16)WarriorBufSkillListFiled["IncreaseMagicCastingPoint"].GetInt();
				int16 IncreaseMagicAttackHitRate = (int16)WarriorBufSkillListFiled["IncreaseMagicAttackHitRate"].GetInt();
				int IncreaseDefencePoint = WarriorBufSkillListFiled["IncreaseDefencePoint"].GetInt();
				int16 IncreaseEvasionRate = (int16)WarriorBufSkillListFiled["IncreaseEvasionRate"].GetInt();
				int16 IncreaseMeleeCriticalPoint = (int16)WarriorBufSkillListFiled["IncreaseMeleeCriticalPoint"].GetInt();
				int16 IncreaseMagicCriticalPoint = (int16)WarriorBufSkillListFiled["IncreaseMagicCriticalPoint"].GetInt();
				float IncreaseSpeedPoint = WarriorBufSkillListFiled["IncreaseSpeedPoint"].GetFloat();
				int16 IncreaseStatusAbnormalityResistance = (int16)WarriorBufSkillListFiled["IncreaseStatusAbnormalityResistance"].GetInt();

				int SkillCoolTime = WarriorBufSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = WarriorBufSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = WarriorBufSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = WarriorBufSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = WarriorBufSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "SKILL_KNIGHT_CHARGE_POSE")
				{
					WarriorBufSkill->SkillType = en_SkillType::SKILL_KNIGHT_CHARGE_POSE;
				}

				WarriorBufSkill->SkillName = SkillName;
				WarriorBufSkill->SkillLevel = SkillLevel;
				WarriorBufSkill->IncreaseMinAttackPoint = IncreaseMinAttackPoint;
				WarriorBufSkill->IncreaseMaxAttackPoint = IncreaseMaxAttackPoint;
				WarriorBufSkill->IncreaseMeleeAttackSpeedPoint = IncreaseMeleeAttackSpeedPoint;
				WarriorBufSkill->IncreaseMeleeAttackHitRate = IncreaseMeleeAttackHitRate;
				WarriorBufSkill->IncreaseMagicAttackPoint = IncreaseMagicAttackPoint;
				WarriorBufSkill->IncreaseMagicCastingPoint = IncreaseMagicCastingPoint;
				WarriorBufSkill->IncreaseMagicAttackHitRate = IncreaseMagicAttackHitRate;
				WarriorBufSkill->IncreaseDefencePoint = IncreaseDefencePoint;
				WarriorBufSkill->IncreaseEvasionRate = IncreaseEvasionRate;
				WarriorBufSkill->IncreaseMeleeCriticalPoint = IncreaseMeleeCriticalPoint;
				WarriorBufSkill->IncreaseMagicCriticalPoint = IncreaseMagicCriticalPoint;
				WarriorBufSkill->IncreaseSpeedPoint = IncreaseSpeedPoint;
				WarriorBufSkill->IncreaseStatusAbnormalityResistance = IncreaseStatusAbnormalityResistance;
				WarriorBufSkill->SkillCoolTime = SkillCoolTime;
				WarriorBufSkill->SkillCastingTime = SkillCastingTime;
				WarriorBufSkill->SkillDistance = SkillDistance;
				WarriorBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				WarriorBufSkill->SkillThumbnailImagePath = SkillImagePath;

				_WarriorBufSkillDatas.insert(pair<int16, st_BufSkillData*>((int16)WarriorBufSkill->SkillType, WarriorBufSkill));
			}
		}
	}
}

void CDataManager::LoadDataShamanSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["ShamanSkills"].GetArray())
	{
		for (auto& ShamanSkillListFiled : Filed["ShamanSkillList"].GetArray())
		{
			for (auto& ShmanAttackSkillListFiled : ShamanSkillListFiled["ShamanAttackSkillList"].GetArray())
			{
				st_AttackSkillData* ShamanAttackSkill = new st_AttackSkillData();
				ShamanAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
				ShamanAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK;

				string SkillType = ShmanAttackSkillListFiled["SkillType"].GetString();
				string SkillName = ShmanAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = ShmanAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = ShmanAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = ShmanAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = ShmanAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ShmanAttackSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = ShmanAttackSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ShmanAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();
				bool SkillDebuf = ShmanAttackSkillListFiled["SkillDebuf"].GetBool();
				int64 SkillDebufTime = ShmanAttackSkillListFiled["SkillDebufTime"].GetInt64();
				int8 SkillDebufAttackSpeed = (int8)ShmanAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)ShmanAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				bool SkillDebufStun = ShmanAttackSkillListFiled["SkillDebufStun"].GetBool();
				bool SkillDebufPushAway = ShmanAttackSkillListFiled["SkillDebufPushAway"].GetBool();
				bool SkillDebufRoot = ShmanAttackSkillListFiled["SkillDebufRoot"].GetBool();
				int64 SkillDamageOverTime = ShmanAttackSkillListFiled["SkillDebufDamageOverTime"].GetInt64();
				int8 StatusAbnormalityProbability = (int8)ShmanAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();				
				string SkillImagePath = ShmanAttackSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "SKILL_SHAMAN_FLAME_HARPOON")
				{
					ShamanAttackSkill->SkillType = en_SkillType::SKILL_SHAMAN_FLAME_HARPOON;
				}
				else if (SkillType == "SKILL_SHAMAN_ROOT")
				{
					ShamanAttackSkill->SkillType = en_SkillType::SKILL_SHAMAN_ROOT;
				}
				else if (SkillType == "SKILL_SHAMAN_ICE_CHAIN")
				{
					ShamanAttackSkill->SkillType = en_SkillType::SKILL_SHAMAN_ICE_CHAIN;
				}
				else if (SkillType == "SKILL_SHAMAN_ICE_WAVE")
				{
					ShamanAttackSkill->SkillType = en_SkillType::SKILL_SHAMAN_ICE_WAVE;
				}
				else if (SkillType == "SKILL_SHAMAN_LIGHTNING_STRIKE")
				{
					ShamanAttackSkill->SkillType = en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE;
				}
				else if (SkillType == "SKILL_SHAMAN_HELL_FIRE")
				{
					ShamanAttackSkill->SkillType = en_SkillType::SKILL_SHAMAN_HELL_FIRE;
				}

				ShamanAttackSkill->SkillName = SkillName;
				ShamanAttackSkill->SkillLevel = SkillLevel;
				ShamanAttackSkill->SkillMinDamage = SkillMinDamage;
				ShamanAttackSkill->SkillMaxDamage = SkillMaxDamage;
				ShamanAttackSkill->SkillCoolTime = SkillCoolTime;
				ShamanAttackSkill->SkillCastingTime = SkillCastingTime;
				ShamanAttackSkill->SkillDistance = SkillDistance;
				ShamanAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;

				ShamanAttackSkill->SkillDebuf = SkillDebuf;
				ShamanAttackSkill->SkillDebufTime = SkillDebufTime;
				ShamanAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				ShamanAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
				ShamanAttackSkill->SkillDebufStun = SkillDebufStun;
				ShamanAttackSkill->SkillDebufPushAway = SkillDebufPushAway;
				ShamanAttackSkill->SkillDebufRoot = SkillDebufRoot;
				ShamanAttackSkill->SkillDamageOverTime = SkillDamageOverTime;
				ShamanAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;
				ShamanAttackSkill->SkillThumbnailImagePath = SkillImagePath;

				_ShamanAttackSkillDatas.insert(pair<int16, st_AttackSkillData*>((int16)ShamanAttackSkill->SkillType, ShamanAttackSkill));
			}			

			for (auto& ShamanHealSkillListFiled : ShamanSkillListFiled["ShamanHealSkillList"].GetArray())
			{
				st_HealSkillData* ShamanHealSkill = new st_HealSkillData();
				ShamanHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
				ShamanHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_HEAL;

				string SkillType = ShamanHealSkillListFiled["SkillType"].GetString();
				string SkillName = ShamanHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = ShamanHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = ShamanHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = ShamanHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = ShamanHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ShamanHealSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = ShamanHealSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ShamanHealSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = ShamanHealSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "")
				{

				}

				ShamanHealSkill->SkillName = SkillName;
				ShamanHealSkill->SkillLevel = SkillLevel;
				ShamanHealSkill->SkillMinHealPoint = SkillMinHeal;
				ShamanHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				ShamanHealSkill->SkillCoolTime = SkillCoolTime;
				ShamanHealSkill->SkillCastingTime = SkillCastingTime;
				ShamanHealSkill->SkillDistance = SkillDistance;
				ShamanHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ShamanHealSkill->SkillThumbnailImagePath = SkillImagePath;

				_ShamanHealSkillDatas.insert(pair<int16, st_HealSkillData*>((int16)ShamanHealSkill->SkillType, ShamanHealSkill));
			}			

			for (auto& ShmanBufSkillListFiled : ShamanSkillListFiled["ShamanBufSkillList"].GetArray())
			{
				st_BufSkillData* ShamanBufSkill = new st_BufSkillData();
				ShamanBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
				ShamanBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_HEAL;

				string SkillType = ShmanBufSkillListFiled["SkillType"].GetString();
				string SkillName = ShmanBufSkillListFiled["SkillName"].GetString();
				int SkillLevel = ShmanBufSkillListFiled["SkillLevel"].GetInt();

				int IncreaseMinAttackPoint = ShmanBufSkillListFiled["IncreaseMinAttackPoint"].GetInt();
				int IncreaseMaxAttackPoint = ShmanBufSkillListFiled["IncreaseMaxAttackPoint"].GetInt();
				int IncreaseMeleeAttackSpeedPoint = ShmanBufSkillListFiled["IncreaseMeleeAttackSpeedPoint"].GetInt();
				int16 IncreaseMeleeAttackHitRate = (int16)ShmanBufSkillListFiled["IncreaseMeleeAttackHitRate"].GetInt();
				int16 IncreaseMagicAttackPoint = (int16)ShmanBufSkillListFiled["IncreaseMagicAttackPoint"].GetInt();
				int16 IncreaseMagicCastingPoint = (int16)ShmanBufSkillListFiled["IncreaseMagicCastingPoint"].GetInt();
				int16 IncreaseMagicAttackHitRate = (int16)ShmanBufSkillListFiled["IncreaseMagicAttackHitRate"].GetInt();
				int IncreaseDefencePoint = ShmanBufSkillListFiled["IncreaseDefencePoint"].GetInt();
				int16 IncreaseEvasionRate = (int16)ShmanBufSkillListFiled["IncreaseEvasionRate"].GetInt();
				int16 IncreaseMeleeCriticalPoint = (int16)ShmanBufSkillListFiled["IncreaseMeleeCriticalPoint"].GetInt();
				int16 IncreaseMagicCriticalPoint = (int16)ShmanBufSkillListFiled["IncreaseMagicCriticalPoint"].GetInt();
				float IncreaseSpeedPoint = ShmanBufSkillListFiled["IncreaseSpeedPoint"].GetFloat();
				int16 IncreaseStatusAbnormalityResistance = (int16)ShmanBufSkillListFiled["IncreaseStatusAbnormalityResistance"].GetInt();

				int SkillCoolTime = ShmanBufSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ShmanBufSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = ShmanBufSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ShmanBufSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = ShmanBufSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "")
				{

				}

				ShamanBufSkill->SkillName = SkillName;
				ShamanBufSkill->SkillLevel = SkillLevel;
				ShamanBufSkill->IncreaseMinAttackPoint = IncreaseMinAttackPoint;
				ShamanBufSkill->IncreaseMaxAttackPoint = IncreaseMaxAttackPoint;
				ShamanBufSkill->IncreaseMeleeAttackSpeedPoint = IncreaseMeleeAttackSpeedPoint;
				ShamanBufSkill->IncreaseMeleeAttackHitRate = IncreaseMeleeAttackHitRate;
				ShamanBufSkill->IncreaseMagicAttackPoint = IncreaseMagicAttackPoint;
				ShamanBufSkill->IncreaseMagicCastingPoint = IncreaseMagicCastingPoint;
				ShamanBufSkill->IncreaseMagicAttackHitRate = IncreaseMagicAttackHitRate;
				ShamanBufSkill->IncreaseDefencePoint = IncreaseDefencePoint;
				ShamanBufSkill->IncreaseEvasionRate = IncreaseEvasionRate;
				ShamanBufSkill->IncreaseMeleeCriticalPoint = IncreaseMeleeCriticalPoint;
				ShamanBufSkill->IncreaseMagicCriticalPoint = IncreaseMagicCriticalPoint;
				ShamanBufSkill->IncreaseSpeedPoint = IncreaseSpeedPoint;
				ShamanBufSkill->IncreaseStatusAbnormalityResistance = IncreaseStatusAbnormalityResistance;
				ShamanBufSkill->SkillCoolTime = SkillCoolTime;
				ShamanBufSkill->SkillCastingTime = SkillCastingTime;
				ShamanBufSkill->SkillDistance = SkillDistance;
				ShamanBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ShamanBufSkill->SkillThumbnailImagePath = SkillImagePath;

				_ShamanBufSkillDatas.insert(pair<int16, st_BufSkillData*>((int16)ShamanBufSkill->SkillType, ShamanBufSkill));
			}
		}
	}
}

void CDataManager::LoadDataTaioistSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["TaioistSkills"].GetArray())
	{
		for (auto& TaioistSkillListFiled : Filed["TaioistSkillList"].GetArray())
		{
			for (auto& TaioistAttackSkillListFiled : TaioistSkillListFiled["TaioistAttackSkillList"].GetArray())
			{
				st_AttackSkillData* TaioistAttackSkill = new st_AttackSkillData();
				TaioistAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
				TaioistAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK;

				string SkillType = TaioistAttackSkillListFiled["SkillType"].GetString();
				string SkillName = TaioistAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = TaioistAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = TaioistAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = TaioistAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = TaioistAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = TaioistAttackSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = TaioistAttackSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = TaioistAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();
				bool SkillDebuf = TaioistAttackSkillListFiled["SkillDebuf"].GetBool();
				int64 SkillDebufTime = TaioistAttackSkillListFiled["SkillDebufTime"].GetInt64();
				int8 SkillDebufAttackSpeed = (int8)TaioistAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)TaioistAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				bool SkillDebufStun = TaioistAttackSkillListFiled["SkillDebufStun"].GetBool();
				bool SkillDebufPushAway = TaioistAttackSkillListFiled["SkillDebufPushAway"].GetBool();
				bool SkillDebufRoot = TaioistAttackSkillListFiled["SkillDebufRoot"].GetBool();
				int64 SkillDamageOverTime = TaioistAttackSkillListFiled["SkillDebufDamageOverTime"].GetInt64();
				int8 StatusAbnormalityProbability = (int8)TaioistAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();				
				string SkillImagePath = TaioistAttackSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "SKILL_TAIOIST_DIVINE_STRIKE")
				{
					TaioistAttackSkill->SkillType = en_SkillType::SKILL_TAIOIST_DIVINE_STRIKE;
				}
				else if (SkillType == "SKILL_TAIOIST_ROOT")
				{
					TaioistAttackSkill->SkillType = en_SkillType::SKILL_TAIOIST_ROOT;
				}

				TaioistAttackSkill->SkillName = SkillName;
				TaioistAttackSkill->SkillLevel = SkillLevel;
				TaioistAttackSkill->SkillMinDamage = SkillMinDamage;
				TaioistAttackSkill->SkillMaxDamage = SkillMaxDamage;
				TaioistAttackSkill->SkillCoolTime = SkillCoolTime;
				TaioistAttackSkill->SkillCastingTime = SkillCastingTime;
				TaioistAttackSkill->SkillDistance = SkillDistance;
				TaioistAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;

				TaioistAttackSkill->SkillDebuf = SkillDebuf;
				TaioistAttackSkill->SkillDebufTime = SkillDebufTime;
				TaioistAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				TaioistAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
				TaioistAttackSkill->SkillDebufStun = SkillDebufStun;
				TaioistAttackSkill->SkillDebufPushAway = SkillDebufPushAway;
				TaioistAttackSkill->SkillDebufRoot = SkillDebufRoot;
				TaioistAttackSkill->SkillDamageOverTime = SkillDamageOverTime;
				TaioistAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;
				TaioistAttackSkill->SkillThumbnailImagePath = SkillImagePath;
				TaioistAttackSkill->SkillThumbnailImagePath = SkillImagePath;

				_TaioistAttackSkillDatas.insert(pair<int16, st_AttackSkillData*>((int16)TaioistAttackSkill->SkillType, TaioistAttackSkill));
			}

			for (auto& TaioistHealSkillListFiled : TaioistSkillListFiled["TaioistHealSkillList"].GetArray())
			{
				st_HealSkillData* TaioistHealSkill = new st_HealSkillData();
				TaioistHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
				TaioistHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL;

				string SkillType = TaioistHealSkillListFiled["SkillType"].GetString();
				string SkillName = TaioistHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = TaioistHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = TaioistHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = TaioistHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = TaioistHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = TaioistHealSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = TaioistHealSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = TaioistHealSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = TaioistHealSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "SKILL_TAIOIST_HEALING_LIGHT")
				{
					TaioistHealSkill->SkillType = en_SkillType::SKILL_TAIOIST_HEALING_LIGHT;
				}
				else if (SkillType == "SKILL_TAIOIST_HEALING_WIND")
				{
					TaioistHealSkill->SkillType = en_SkillType::SKILL_TAIOIST_HEALING_WIND;
				}

				TaioistHealSkill->SkillName = SkillName;
				TaioistHealSkill->SkillLevel = SkillLevel;
				TaioistHealSkill->SkillMinHealPoint = SkillMinHeal;
				TaioistHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				TaioistHealSkill->SkillCoolTime = SkillCoolTime;
				TaioistHealSkill->SkillCastingTime = SkillCastingTime;
				TaioistHealSkill->SkillDistance = SkillDistance;
				TaioistHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				TaioistHealSkill->SkillThumbnailImagePath = SkillImagePath;

				_TaioistHealSkillDatas.insert(pair<int16, st_HealSkillData*>((int16)TaioistHealSkill->SkillType, TaioistHealSkill));
			}

			for (auto& TaioistBufSkillListFiled : TaioistSkillListFiled["TaioistBufSkillList"].GetArray())
			{
				st_BufSkillData* TaioistBufSkill = new st_BufSkillData();
				TaioistBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
				TaioistBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_BUF;

				string SkillType = TaioistBufSkillListFiled["SkillType"].GetString();
				string SkillName = TaioistBufSkillListFiled["SkillName"].GetString();
				int SkillLevel = TaioistBufSkillListFiled["SkillLevel"].GetInt();

				int IncreaseMinAttackPoint = TaioistBufSkillListFiled["IncreaseMinAttackPoint"].GetInt();
				int IncreaseMaxAttackPoint = TaioistBufSkillListFiled["IncreaseMaxAttackPoint"].GetInt();
				int IncreaseMeleeAttackSpeedPoint = TaioistBufSkillListFiled["IncreaseMeleeAttackSpeedPoint"].GetInt();
				int16 IncreaseMeleeAttackHitRate = (int16)TaioistBufSkillListFiled["IncreaseMeleeAttackHitRate"].GetInt();
				int16 IncreaseMagicAttackPoint = (int16)TaioistBufSkillListFiled["IncreaseMagicAttackPoint"].GetInt();
				int16 IncreaseMagicCastingPoint = (int16)TaioistBufSkillListFiled["IncreaseMagicCastingPoint"].GetInt();
				int16 IncreaseMagicAttackHitRate = (int16)TaioistBufSkillListFiled["IncreaseMagicAttackHitRate"].GetInt();
				int IncreaseDefencePoint = TaioistBufSkillListFiled["IncreaseDefencePoint"].GetInt();
				int16 IncreaseEvasionRate = (int16)TaioistBufSkillListFiled["IncreaseEvasionRate"].GetInt();
				int16 IncreaseMeleeCriticalPoint = (int16)TaioistBufSkillListFiled["IncreaseMeleeCriticalPoint"].GetInt();
				int16 IncreaseMagicCriticalPoint = (int16)TaioistBufSkillListFiled["IncreaseMagicCriticalPoint"].GetInt();
				float IncreaseSpeedPoint = TaioistBufSkillListFiled["IncreaseSpeedPoint"].GetFloat();
				int16 IncreaseStatusAbnormalityResistance = (int16)TaioistBufSkillListFiled["IncreaseStatusAbnormalityResistance"].GetInt();

				int SkillCoolTime = TaioistBufSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = TaioistBufSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = TaioistBufSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = TaioistBufSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = TaioistBufSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "")
				{

				}

				TaioistBufSkill->SkillName = SkillName;
				TaioistBufSkill->SkillLevel = SkillLevel;
				TaioistBufSkill->IncreaseMinAttackPoint = IncreaseMinAttackPoint;
				TaioistBufSkill->IncreaseMaxAttackPoint = IncreaseMaxAttackPoint;
				TaioistBufSkill->IncreaseMeleeAttackSpeedPoint = IncreaseMeleeAttackSpeedPoint;
				TaioistBufSkill->IncreaseMeleeAttackHitRate = IncreaseMeleeAttackHitRate;
				TaioistBufSkill->IncreaseMagicAttackPoint = IncreaseMagicAttackPoint;
				TaioistBufSkill->IncreaseMagicCastingPoint = IncreaseMagicCastingPoint;
				TaioistBufSkill->IncreaseMagicAttackHitRate = IncreaseMagicAttackHitRate;
				TaioistBufSkill->IncreaseDefencePoint = IncreaseDefencePoint;
				TaioistBufSkill->IncreaseEvasionRate = IncreaseEvasionRate;
				TaioistBufSkill->IncreaseMeleeCriticalPoint = IncreaseMeleeCriticalPoint;
				TaioistBufSkill->IncreaseMagicCriticalPoint = IncreaseMagicCriticalPoint;
				TaioistBufSkill->IncreaseSpeedPoint = IncreaseSpeedPoint;
				TaioistBufSkill->IncreaseStatusAbnormalityResistance = IncreaseStatusAbnormalityResistance;
				TaioistBufSkill->SkillCoolTime = SkillCoolTime;
				TaioistBufSkill->SkillCastingTime = SkillCastingTime;
				TaioistBufSkill->SkillDistance = SkillDistance;
				TaioistBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				TaioistBufSkill->SkillThumbnailImagePath = SkillImagePath;

				_TaioistBufSkillDatas.insert(pair<int16, st_BufSkillData*>((int16)TaioistBufSkill->SkillType, TaioistBufSkill));
			}
		}
	}
}

void CDataManager::LoadDataThiefSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["ThiefSkills"].GetArray())
	{
		for (auto& ThiefSkillListFiled : Filed["ThiefSkillList"].GetArray())
		{
			for (auto& ThiefAttackSkillListFiled : ThiefSkillListFiled["ThiefAttackSkillList"].GetArray())
			{
				st_AttackSkillData* ThiefAttackSkill = new st_AttackSkillData();
				ThiefAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_THIEF;
				ThiefAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_ATTACK;

				string SkillType = ThiefAttackSkillListFiled["SkillType"].GetString();
				string SkillName = ThiefAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = ThiefAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = ThiefAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = ThiefAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = ThiefAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ThiefAttackSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = ThiefAttackSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ThiefAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();
				bool SkillDebuf = ThiefAttackSkillListFiled["SkillDebuf"].GetBool();
				int64 SkillDebufTime = ThiefAttackSkillListFiled["SkillDebufTime"].GetInt64();
				int8 SkillDebufAttackSpeed = (int8)ThiefAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)ThiefAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				bool SkillDebufStun = ThiefAttackSkillListFiled["SkillDebufStun"].GetBool();
				bool SkillDebufPushAway = ThiefAttackSkillListFiled["SkillDebufPushAway"].GetBool();
				bool SkillDebufRoot = ThiefAttackSkillListFiled["SkillDebufRoot"].GetBool();
				int64 SkillDamageOverTime = ThiefAttackSkillListFiled["SkillDebufDamageOverTime"].GetInt64();								
				int8 StatusAbnormalityProbability = (int8)ThiefAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();
				string SkillImagePath = ThiefAttackSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "SKILL_THIEF_QUICK_CUT")
				{
					ThiefAttackSkill->SkillType = en_SkillType::SKILL_THIEF_QUICK_CUT;
				}				

				ThiefAttackSkill->SkillName = SkillName;
				ThiefAttackSkill->SkillLevel = SkillLevel;
				ThiefAttackSkill->SkillMinDamage = SkillMinDamage;
				ThiefAttackSkill->SkillMaxDamage = SkillMaxDamage;
				ThiefAttackSkill->SkillCoolTime = SkillCoolTime;
				ThiefAttackSkill->SkillCastingTime = SkillCastingTime;
				ThiefAttackSkill->SkillDistance = SkillDistance;
				ThiefAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;

				ThiefAttackSkill->SkillDebuf = SkillDebuf;
				ThiefAttackSkill->SkillDebufTime = SkillDebufTime;
				ThiefAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				ThiefAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
				ThiefAttackSkill->SkillDebufStun = SkillDebufStun;
				ThiefAttackSkill->SkillDebufPushAway = SkillDebufPushAway;
				ThiefAttackSkill->SkillDebufRoot = SkillDebufRoot;
				ThiefAttackSkill->SkillDamageOverTime = SkillDamageOverTime;
				ThiefAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;
				ThiefAttackSkill->SkillThumbnailImagePath = SkillImagePath;
				ThiefAttackSkill->SkillThumbnailImagePath = SkillImagePath;

				_ThiefAttackSkillDatas.insert(pair<int16, st_AttackSkillData*>((int16)ThiefAttackSkill->SkillType, ThiefAttackSkill));
			}			

			for (auto& ThiefHealSkillListFiled : ThiefSkillListFiled["ThiefHealSkillList"].GetArray())
			{
				st_HealSkillData* ThiefHealSkill = new st_HealSkillData();
				ThiefHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_THIEF;
				ThiefHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_HEAL;

				string SkillType = ThiefHealSkillListFiled["SkillType"].GetString();
				string SkillName = ThiefHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = ThiefHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = ThiefHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = ThiefHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = ThiefHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ThiefHealSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = ThiefHealSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ThiefHealSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = ThiefHealSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "")
				{
					
				}				

				ThiefHealSkill->SkillName = SkillName;
				ThiefHealSkill->SkillLevel = SkillLevel;
				ThiefHealSkill->SkillMinHealPoint = SkillMinHeal;
				ThiefHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				ThiefHealSkill->SkillCoolTime = SkillCoolTime;
				ThiefHealSkill->SkillCastingTime = SkillCastingTime;
				ThiefHealSkill->SkillDistance = SkillDistance;
				ThiefHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ThiefHealSkill->SkillThumbnailImagePath = SkillImagePath;

				_ThiefHealSkillDatas.insert(pair<int16, st_HealSkillData*>((int16)ThiefHealSkill->SkillType, ThiefHealSkill));
			}

			for (auto& ThiefBufSkillListFiled : ThiefSkillListFiled["ThiefBufSkillList"].GetArray())
			{
				st_BufSkillData* ThiefBufSkill = new st_BufSkillData();
				ThiefBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_THIEF;
				ThiefBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_BUF;

				string SkillType = ThiefBufSkillListFiled["SkillType"].GetString();
				string SkillName = ThiefBufSkillListFiled["SkillName"].GetString();
				int SkillLevel = ThiefBufSkillListFiled["SkillLevel"].GetInt();

				int IncreaseMinAttackPoint = ThiefBufSkillListFiled["IncreaseMinAttackPoint"].GetInt();
				int IncreaseMaxAttackPoint = ThiefBufSkillListFiled["IncreaseMaxAttackPoint"].GetInt();
				int IncreaseMeleeAttackSpeedPoint = ThiefBufSkillListFiled["IncreaseMeleeAttackSpeedPoint"].GetInt();
				int16 IncreaseMeleeAttackHitRate = (int16)ThiefBufSkillListFiled["IncreaseMeleeAttackHitRate"].GetInt();
				int16 IncreaseMagicAttackPoint = (int16)ThiefBufSkillListFiled["IncreaseMagicAttackPoint"].GetInt();
				int16 IncreaseMagicCastingPoint = (int16)ThiefBufSkillListFiled["IncreaseMagicCastingPoint"].GetInt();
				int16 IncreaseMagicAttackHitRate = (int16)ThiefBufSkillListFiled["IncreaseMagicAttackHitRate"].GetInt();
				int IncreaseDefencePoint = ThiefBufSkillListFiled["IncreaseDefencePoint"].GetInt();
				int16 IncreaseEvasionRate = (int16)ThiefBufSkillListFiled["IncreaseEvasionRate"].GetInt();
				int16 IncreaseMeleeCriticalPoint = (int16)ThiefBufSkillListFiled["IncreaseMeleeCriticalPoint"].GetInt();
				int16 IncreaseMagicCriticalPoint = (int16)ThiefBufSkillListFiled["IncreaseMagicCriticalPoint"].GetInt();
				float IncreaseSpeedPoint = ThiefBufSkillListFiled["IncreaseSpeedPoint"].GetFloat();
				int16 IncreaseStatusAbnormalityResistance = (int16)ThiefBufSkillListFiled["IncreaseStatusAbnormalityResistance"].GetInt();

				int SkillCoolTime = ThiefBufSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ThiefBufSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = ThiefBufSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ThiefBufSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = ThiefBufSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "")
				{

				}

				ThiefBufSkill->SkillName = SkillName;
				ThiefBufSkill->SkillLevel = SkillLevel;
				ThiefBufSkill->IncreaseMinAttackPoint = IncreaseMinAttackPoint;
				ThiefBufSkill->IncreaseMaxAttackPoint = IncreaseMaxAttackPoint;
				ThiefBufSkill->IncreaseMeleeAttackSpeedPoint = IncreaseMeleeAttackSpeedPoint;
				ThiefBufSkill->IncreaseMeleeAttackHitRate = IncreaseMeleeAttackHitRate;
				ThiefBufSkill->IncreaseMagicAttackPoint = IncreaseMagicAttackPoint;
				ThiefBufSkill->IncreaseMagicCastingPoint = IncreaseMagicCastingPoint;
				ThiefBufSkill->IncreaseMagicAttackHitRate = IncreaseMagicAttackHitRate;
				ThiefBufSkill->IncreaseDefencePoint = IncreaseDefencePoint;
				ThiefBufSkill->IncreaseEvasionRate = IncreaseEvasionRate;
				ThiefBufSkill->IncreaseMeleeCriticalPoint = IncreaseMeleeCriticalPoint;
				ThiefBufSkill->IncreaseMagicCriticalPoint = IncreaseMagicCriticalPoint;
				ThiefBufSkill->IncreaseSpeedPoint = IncreaseSpeedPoint;
				ThiefBufSkill->IncreaseStatusAbnormalityResistance = IncreaseStatusAbnormalityResistance;
				ThiefBufSkill->SkillCoolTime = SkillCoolTime;
				ThiefBufSkill->SkillCastingTime = SkillCastingTime;
				ThiefBufSkill->SkillDistance = SkillDistance;
				ThiefBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ThiefBufSkill->SkillThumbnailImagePath = SkillImagePath;

				_ThiefBufSkillDatas.insert(pair<int16, st_BufSkillData*>((int16)ThiefBufSkill->SkillType, ThiefBufSkill));
			}
		}
	}
}

void CDataManager::LoadDataArcherSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["ArcherSkills"].GetArray())
	{
		for (auto& ArcherSkillListFiled : Filed["ArcherSkillList"].GetArray())
		{
			for (auto& ArcherAttackSkillListFiled : ArcherSkillListFiled["ArcherAttackSkillList"].GetArray())
			{
				st_AttackSkillData* ArcherAttackSkill = new st_AttackSkillData();
				ArcherAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_ARCHER;
				ArcherAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_ATTACK;

				string SkillType = ArcherAttackSkillListFiled["SkillType"].GetString();
				string SkillName = ArcherAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = ArcherAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = ArcherAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = ArcherAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = ArcherAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ArcherAttackSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = ArcherAttackSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ArcherAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();
				bool SkillDebuf = ArcherAttackSkillListFiled["SkillDebuf"].GetBool();
				int64 SkillDebufTime = ArcherAttackSkillListFiled["SkillDebufTime"].GetInt64();
				int8 SkillDebufAttackSpeed = (int8)ArcherAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)ArcherAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				bool SkillDebufStun = ArcherAttackSkillListFiled["SkillDebufStun"].GetBool();
				bool SkillDebufPushAway = ArcherAttackSkillListFiled["SkillDebufPushAway"].GetBool();
				int64 SkillDamageOverTime = ArcherAttackSkillListFiled["SkillDebufDamageOverTime"].GetInt64();
				int8 StatusAbnormalityProbability = (int8)ArcherAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();
				int SkillDebufRoot = ArcherAttackSkillListFiled["SkillDebufRoot"].GetBool();
				string SkillImagePath = ArcherAttackSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "SKILL_ARCHER_SNIFING")
				{
					ArcherAttackSkill->SkillType = en_SkillType::SKILL_ARCHER_SNIFING;
				}			

				ArcherAttackSkill->SkillName = SkillName;
				ArcherAttackSkill->SkillLevel = SkillLevel;
				ArcherAttackSkill->SkillMinDamage = SkillMinDamage;
				ArcherAttackSkill->SkillMaxDamage = SkillMaxDamage;
				ArcherAttackSkill->SkillCoolTime = SkillCoolTime;
				ArcherAttackSkill->SkillCastingTime = SkillCastingTime;
				ArcherAttackSkill->SkillDistance = SkillDistance;
				ArcherAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;

				ArcherAttackSkill->SkillDebuf = SkillDebuf;
				ArcherAttackSkill->SkillDebufTime = SkillDebufTime;
				ArcherAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				ArcherAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
				ArcherAttackSkill->SkillDebufStun = SkillDebufStun;
				ArcherAttackSkill->SkillDebufPushAway = SkillDebufPushAway;
				ArcherAttackSkill->SkillDebufRoot = SkillDebufRoot;
				ArcherAttackSkill->SkillDamageOverTime = SkillDamageOverTime;
				ArcherAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;
				ArcherAttackSkill->SkillThumbnailImagePath = SkillImagePath;
				ArcherAttackSkill->SkillThumbnailImagePath = SkillImagePath;

				_ArcherAttackSkillDatas.insert(pair<int16, st_AttackSkillData*>((int16)ArcherAttackSkill->SkillType, ArcherAttackSkill));
			}			

			for (auto& ArcherHealSkillListFiled : ArcherSkillListFiled["ArcherHealSkillList"].GetArray())
			{
				st_HealSkillData* ArcherHealSkill = new st_HealSkillData();
				ArcherHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_ARCHER;
				ArcherHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_HEAL;

				string SkillType = ArcherHealSkillListFiled["SkillType"].GetString();
				string SkillName = ArcherHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = ArcherHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = ArcherHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = ArcherHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = ArcherHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ArcherHealSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = ArcherHealSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ArcherHealSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = ArcherHealSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "")
				{
					
				}				

				ArcherHealSkill->SkillName = SkillName;
				ArcherHealSkill->SkillLevel = SkillLevel;
				ArcherHealSkill->SkillMinHealPoint = SkillMinHeal;
				ArcherHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				ArcherHealSkill->SkillCoolTime = SkillCoolTime;
				ArcherHealSkill->SkillCastingTime = SkillCastingTime;
				ArcherHealSkill->SkillDistance = SkillDistance;
				ArcherHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ArcherHealSkill->SkillThumbnailImagePath = SkillImagePath;

				_ArcherHealSkillDatas.insert(pair<int16, st_HealSkillData*>((int16)ArcherHealSkill->SkillType, ArcherHealSkill));
			}

			for (auto& ArcherBufSkillListFiled : ArcherSkillListFiled["ArcherBufSkillList"].GetArray())
			{
				st_BufSkillData* ArcherBufSkill = new st_BufSkillData();
				ArcherBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_ARCHER;
				ArcherBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_BUF;

				string SkillType = ArcherBufSkillListFiled["SkillType"].GetString();
				string SkillName = ArcherBufSkillListFiled["SkillName"].GetString();
				int SkillLevel = ArcherBufSkillListFiled["SkillLevel"].GetInt();

				int IncreaseMinAttackPoint = ArcherBufSkillListFiled["IncreaseMinAttackPoint"].GetInt();
				int IncreaseMaxAttackPoint = ArcherBufSkillListFiled["IncreaseMaxAttackPoint"].GetInt();
				int IncreaseMeleeAttackSpeedPoint = ArcherBufSkillListFiled["IncreaseMeleeAttackSpeedPoint"].GetInt();
				int16 IncreaseMeleeAttackHitRate = (int16)ArcherBufSkillListFiled["IncreaseMeleeAttackHitRate"].GetInt();
				int16 IncreaseMagicAttackPoint = (int16)ArcherBufSkillListFiled["IncreaseMagicAttackPoint"].GetInt();
				int16 IncreaseMagicCastingPoint = (int16)ArcherBufSkillListFiled["IncreaseMagicCastingPoint"].GetInt();
				int16 IncreaseMagicAttackHitRate = (int16)ArcherBufSkillListFiled["IncreaseMagicAttackHitRate"].GetInt();
				int IncreaseDefencePoint = ArcherBufSkillListFiled["IncreaseDefencePoint"].GetInt();
				int16 IncreaseEvasionRate = (int16)ArcherBufSkillListFiled["IncreaseEvasionRate"].GetInt();
				int16 IncreaseMeleeCriticalPoint = (int16)ArcherBufSkillListFiled["IncreaseMeleeCriticalPoint"].GetInt();
				int16 IncreaseMagicCriticalPoint = (int16)ArcherBufSkillListFiled["IncreaseMagicCriticalPoint"].GetInt();
				float IncreaseSpeedPoint = ArcherBufSkillListFiled["IncreaseSpeedPoint"].GetFloat();
				int16 IncreaseStatusAbnormalityResistance = (int16)ArcherBufSkillListFiled["IncreaseStatusAbnormalityResistance"].GetInt();

				int SkillCoolTime = ArcherBufSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ArcherBufSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = ArcherBufSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ArcherBufSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillImagePath = ArcherBufSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "")
				{

				}

				ArcherBufSkill->SkillName = SkillName;
				ArcherBufSkill->SkillLevel = SkillLevel;
				ArcherBufSkill->IncreaseMinAttackPoint = IncreaseMinAttackPoint;
				ArcherBufSkill->IncreaseMaxAttackPoint = IncreaseMaxAttackPoint;
				ArcherBufSkill->IncreaseMeleeAttackSpeedPoint = IncreaseMeleeAttackSpeedPoint;
				ArcherBufSkill->IncreaseMeleeAttackHitRate = IncreaseMeleeAttackHitRate;
				ArcherBufSkill->IncreaseMagicAttackPoint = IncreaseMagicAttackPoint;
				ArcherBufSkill->IncreaseMagicCastingPoint = IncreaseMagicCastingPoint;
				ArcherBufSkill->IncreaseMagicAttackHitRate = IncreaseMagicAttackHitRate;
				ArcherBufSkill->IncreaseDefencePoint = IncreaseDefencePoint;
				ArcherBufSkill->IncreaseEvasionRate = IncreaseEvasionRate;
				ArcherBufSkill->IncreaseMeleeCriticalPoint = IncreaseMeleeCriticalPoint;
				ArcherBufSkill->IncreaseMagicCriticalPoint = IncreaseMagicCriticalPoint;
				ArcherBufSkill->IncreaseSpeedPoint = IncreaseSpeedPoint;
				ArcherBufSkill->IncreaseStatusAbnormalityResistance = IncreaseStatusAbnormalityResistance;
				ArcherBufSkill->SkillCoolTime = SkillCoolTime;
				ArcherBufSkill->SkillCastingTime = SkillCastingTime;
				ArcherBufSkill->SkillDistance = SkillDistance;
				ArcherBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ArcherBufSkill->SkillThumbnailImagePath = SkillImagePath;

				_ArcherBufSkillDatas.insert(pair<int16, st_BufSkillData*>((int16)ArcherBufSkill->SkillType, ArcherBufSkill));
			}
		}
	}
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

st_SkillData* CDataManager::FindSkillData(en_SkillMediumCategory FindAttackSkillMediumCategory, en_SkillType FindSkillType)
{
	switch (FindAttackSkillMediumCategory)
	{
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_NONE:
		return nullptr;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK:
		return (*_PublicAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL:
		return (*_PublicHealSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF:
		return (*_PublicBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK:
		return (*_WarriorAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_HEAL:
		return (*_WarriorHealSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_BUF:
		return (*_WarriorBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK:
		return (*_ShamanAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_HEAL:
		return (*_ShamanHealSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_BUF:
		return (*_ShamanBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK:
		return (*_TaioistAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL:
		return (*_TaioistHealSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_BUF:
		return (*_TaioistBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_ATTACK:
		return (*_ThiefAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_HEAL:
		return (*_ThiefHealSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_BUF:
		return (*_ThiefBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_ATTACK:
		return (*_ArcherAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_HEAL:
		return (*_ArcherHealSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_BUF:
		return (*_ArcherBufSkillDatas.find((int16)FindSkillType)).second;
	}
}

st_ObjectStatusData* CDataManager::FindObjectStatusData(en_GameObjectType GameObjectType, int16 Level)
{
	switch (GameObjectType)
	{
	case en_GameObjectType::NORMAL:
		return nullptr;
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
		return (*_WarriorStatus.find(Level)).second;		
	case en_GameObjectType::OBJECT_MAGIC_PLAYER:
		return (*_ShamanStatus.find(Level)).second;		
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
		return (*_TaioistStatus.find(Level)).second;		
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
		return (*_ThiefStatus.find(Level)).second;
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
		return (*_ArcherStatus.find(Level)).second;
	case en_GameObjectType::OBJECT_SLIME:
		break;
	case en_GameObjectType::OBJECT_BEAR:
		break;
	case en_GameObjectType::OBJECT_STONE:
		break;
	case en_GameObjectType::OBJECT_TREE:
		break;
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		return (*_WarriorStatus.find(Level)).second;
	}	
}
