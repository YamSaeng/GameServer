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

	for (auto& Filed : Document["PlayerTaioistCharacterStatus"].GetArray())
	{
		string PlayerType = Filed["PlayerType"].GetString();

		for (auto& PlayerTaioistCharacterFiled : Filed["PlayerTaioistCharacterLevelDataList"].GetArray())
		{
			int Level = PlayerTaioistCharacterFiled["Level"].GetInt();
			int MaxHP = PlayerTaioistCharacterFiled["MaxHP"].GetInt();
			int MaxMP = PlayerTaioistCharacterFiled["MaxMP"].GetInt();
			int MaxDP = PlayerTaioistCharacterFiled["MaxDP"].GetInt();
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
		string PublicSkillLargeCategory = Filed["SkillLargeCategory"].GetString();

		for (auto& PublicSkillListFiled : Filed["PublicSkillList"].GetArray())
		{
			for (auto& PublicAttackSkillListFiled : PublicSkillListFiled["PublicAttackSkillList"].GetArray())
			{
				st_AttackSkillData* PublicAttackSkill = new st_AttackSkillData();
				PublicAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEOGRY_PUBLIC;
				PublicAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK;

				string SkillType = PublicAttackSkillListFiled["SkillType"].GetString();
				string SkillName = PublicAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = PublicAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = PublicAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = PublicAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = PublicAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = PublicAttackSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = PublicAttackSkillListFiled["SkillDistance"].GetInt();
				string SkillImagePath = PublicAttackSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "SKILL_NORMAL")
				{
					PublicAttackSkill->SkillType = en_SkillType::SKILL_NORMAL;
				}

				PublicAttackSkill->SkillName = SkillName;
				PublicAttackSkill->SkillLevel = SkillLevel;
				PublicAttackSkill->SkillMinDamage = SkillMinDamage;
				PublicAttackSkill->SkillMaxDamage = SkillMaxDamage;
				PublicAttackSkill->SkillCoolTime = SkillCoolTime;
				PublicAttackSkill->SkillCastingTime = SkillCastingTime;
				PublicAttackSkill->SkillDistance = SkillDistance;
				PublicAttackSkill->SkillThumbnailImagePath = SkillImagePath;

				_PublicAttackSkillDatas.insert(pair<int16, st_AttackSkillData*>((int16)PublicAttackSkill->SkillType, PublicAttackSkill));
			}

			for (auto& PublicHealSkillListFiled : PublicSkillListFiled["PublicHealSkillList"].GetArray())
			{
				st_HealSkillData* PublicHealSkill = new st_HealSkillData();
				PublicHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEOGRY_PUBLIC;
				PublicHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL;

				string SkillType = PublicHealSkillListFiled["SkillType"].GetString();
				string SkillName = PublicHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = PublicHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = PublicHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = PublicHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = PublicHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = PublicHealSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = PublicHealSkillListFiled["SkillDistance"].GetInt();
				string SkillImagePath = PublicHealSkillListFiled["SkillThumbnailImagePath"].GetString();

				PublicHealSkill->SkillName = SkillName;
				PublicHealSkill->SkillLevel = SkillLevel;
				PublicHealSkill->SkillMinHealPoint = SkillMinHeal;
				PublicHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				PublicHealSkill->SkillCoolTime = SkillCoolTime;
				PublicHealSkill->SkillCastingTime = SkillCastingTime;
				PublicHealSkill->SkillDistance = SkillDistance;
				PublicHealSkill->SkillThumbnailImagePath = SkillImagePath;

				_PublicHealSkillDatas.insert(pair<int16, st_HealSkillData*>((int16)PublicHealSkill->SkillType, PublicHealSkill));
			}

			for (auto& PublicBufSkillListFiled : PublicSkillListFiled["PublicBufSkillList"].GetArray())
			{
				st_BufSkillData* PublicBufSkill = new st_BufSkillData();
				PublicBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEOGRY_PUBLIC;
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
		string WarriorSkillLargeCategory = Filed["SkillLargeCategory"].GetString();

		for (auto& WarriorSkillListFiled : Filed["WarriorSkillList"].GetArray())
		{
			string WarriorAttackSkillMediumCategory = WarriorSkillListFiled["SkillMediumCategory"].GetString();

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
				bool SkillDebuf = WarriorAttackSkillListFiled["SkillDebuf"].GetBool();
				int64 SkillDebufTime = WarriorAttackSkillListFiled["SkillDebufTime"].GetInt64();
				int8 SkillDebufAttackSpeed = (int8)WarriorAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)WarriorAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				bool SkillDebufStun = WarriorAttackSkillListFiled["SkillDebufStun"].GetBool();
				bool SkillDebufPushAway = WarriorAttackSkillListFiled["SkillDebufPushAway"].GetBool();
				int64 SkillDamageOverTime = WarriorAttackSkillListFiled["SkillDamageOverTime"].GetInt64();
				int8 StatusAbnormalityProbability = (int8)WarriorAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();
				int SkillDebufRoot = WarriorAttackSkillListFiled["SkillDebufRoot"].GetBool();
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

			string WarriorHealSkillMediumCategory = WarriorSkillListFiled["SkillMediumCategory"].GetString();

			for (auto& WarriorHealSkillListFiled : WarriorSkillListFiled["WarriorHealSkillList"].GetArray())
			{
				st_HealSkillData* WarriorHealSkill = new st_HealSkillData();
				WarriorHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEOGRY_PUBLIC;
				WarriorHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL;

				string SkillType = WarriorHealSkillListFiled["SkillType"].GetString();
				string SkillName = WarriorHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = WarriorHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = WarriorHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = WarriorHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = WarriorHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = WarriorHealSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = WarriorHealSkillListFiled["SkillDistance"].GetInt();
				string SkillImagePath = WarriorHealSkillListFiled["SkillThumbnailImagePath"].GetString();

				WarriorHealSkill->SkillName = SkillName;
				WarriorHealSkill->SkillLevel = SkillLevel;
				WarriorHealSkill->SkillMinHealPoint = SkillMinHeal;
				WarriorHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				WarriorHealSkill->SkillCoolTime = SkillCoolTime;
				WarriorHealSkill->SkillCastingTime = SkillCastingTime;
				WarriorHealSkill->SkillDistance = SkillDistance;
				WarriorHealSkill->SkillThumbnailImagePath = SkillImagePath;

				_WarriorHealSkillDatas.insert(pair<int16, st_HealSkillData*>((int16)WarriorHealSkill->SkillType, WarriorHealSkill));
			}

			string WarriorBufSkillMediumCategory = WarriorSkillListFiled["SkillMediumCategory"].GetString();

			for (auto& WarriorBufSkillListFiled : WarriorSkillListFiled["WarriorBufSkillList"].GetArray())
			{
				st_BufSkillData* WarriorBufSkill = new st_BufSkillData();
				WarriorBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEOGRY_PUBLIC;
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
		string ShamanSkillLargeCategory = Filed["SkillLargeCategory"].GetString();

		for (auto& ShamanSkillListFiled : Filed["ShamanSkillList"].GetArray())
		{
			string ShamanAttackSkillMediumCategory = ShamanSkillListFiled["SkillMediumCategory"].GetString();

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
				bool SkillDebuf = ShmanAttackSkillListFiled["SkillDebuf"].GetBool();
				int64 SkillDebufTime = ShmanAttackSkillListFiled["SkillDebufTime"].GetInt64();
				int8 SkillDebufAttackSpeed = (int8)ShmanAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)ShmanAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				bool SkillDebufStun = ShmanAttackSkillListFiled["SkillDebufStun"].GetBool();
				bool SkillDebufPushAway = ShmanAttackSkillListFiled["SkillDebufPushAway"].GetBool();
				int64 SkillDamageOverTime = ShmanAttackSkillListFiled["SkillDamageOverTime"].GetInt64();
				int8 StatusAbnormalityProbability = (int8)ShmanAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();
				int SkillDebufRoot = ShmanAttackSkillListFiled["SkillDebufRoot"].GetBool();
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

			string ShamanHealSkillMediumCategory = ShamanSkillListFiled["SkillMediumCategory"].GetString();

			for (auto& ShamanHealSkillListFiled : ShamanSkillListFiled["ShamanHealSkillList"].GetArray())
			{
				st_HealSkillData* ShamanHealSkill = new st_HealSkillData();
				ShamanHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEOGRY_PUBLIC;
				ShamanHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL;

				string SkillType = ShamanHealSkillListFiled["SkillType"].GetString();
				string SkillName = ShamanHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = ShamanHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = ShamanHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = ShamanHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = ShamanHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ShamanHealSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = ShamanHealSkillListFiled["SkillDistance"].GetInt();
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
				ShamanHealSkill->SkillThumbnailImagePath = SkillImagePath;

				_ShamanHealSkillDatas.insert(pair<int16, st_HealSkillData*>((int16)ShamanHealSkill->SkillType, ShamanHealSkill));
			}

			string ShmanBufSkillMediumCategory = ShamanSkillListFiled["SkillMediumCategory"].GetString();

			for (auto& ShmanBufSkillListFiled : ShamanSkillListFiled["ShamanBufSkillList"].GetArray())
			{
				st_BufSkillData* ShamanBufSkill = new st_BufSkillData();
				ShamanBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEOGRY_PUBLIC;
				ShamanBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF;

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
		string TaioistSkillLargeCategory = Filed["SkillLargeCategory"].GetString();

		for (auto& TaioistSkillListFiled : Filed["TaioistSkillList"].GetArray())
		{
			string TaioistAttackSkillMediumCategory = TaioistSkillListFiled["SkillMediumCategory"].GetString();

			for (auto& TaioistAttackSkillListFiled : TaioistSkillListFiled["TaioistAttackSkillLit"].GetArray())
			{
				st_AttackSkillData* TaioistAttackSkill = new st_AttackSkillData();
				TaioistAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
				TaioistAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK;

				string SkillType = TaioistAttackSkillListFiled["SkillType"].GetString();
				string SkillName = TaioistAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = TaioistAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = TaioistAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = TaioistAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = TaioistAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = TaioistAttackSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = TaioistAttackSkillListFiled["SkillDistance"].GetInt();
				bool SkillDebuf = TaioistAttackSkillListFiled["SkillDebuf"].GetBool();
				int64 SkillDebufTime = TaioistAttackSkillListFiled["SkillDebufTime"].GetInt64();
				int8 SkillDebufAttackSpeed = (int8)TaioistAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)TaioistAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				bool SkillDebufStun = TaioistAttackSkillListFiled["SkillDebufStun"].GetBool();
				bool SkillDebufPushAway = TaioistAttackSkillListFiled["SkillDebufPushAway"].GetBool();
				int64 SkillDamageOverTime = TaioistAttackSkillListFiled["SkillDamageOverTime"].GetInt64();
				int8 StatusAbnormalityProbability = (int8)TaioistAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();
				int SkillDebufRoot = TaioistAttackSkillListFiled["SkillDebufRoot"].GetBool();
				string SkillImagePath = TaioistAttackSkillListFiled["SkillThumbnailImagePath"].GetString();

				if (SkillType == "SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK")
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

			string TaioistHealSkillMediumCategory = TaioistSkillListFiled["SkillMediumCategory"].GetString();

			for (auto& TaioistHealSkillListFiled : TaioistSkillListFiled["TaioistHealSkillList"].GetArray())
			{
				st_HealSkillData* TaioistHealSkill = new st_HealSkillData();
				TaioistHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEOGRY_PUBLIC;
				TaioistHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL;

				string SkillType = TaioistHealSkillListFiled["SkillType"].GetString();
				string SkillName = TaioistHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = TaioistHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = TaioistHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = TaioistHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = TaioistHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = TaioistHealSkillListFiled["SkillCastingTime"].GetInt();
				int SkillDistance = TaioistHealSkillListFiled["SkillDistance"].GetInt();
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
				TaioistHealSkill->SkillThumbnailImagePath = SkillImagePath;

				_TaioistHealSkillDatas.insert(pair<int16, st_HealSkillData*>((int16)TaioistHealSkill->SkillType, TaioistHealSkill));
			}

			string TaioistBufSkillMediumCategory = TaioistSkillListFiled["SkillMediumCategory"].GetString();

			for (auto& TaioistBufSkillListFiled : TaioistSkillListFiled["TaioistBufSkillList"].GetArray())
			{
				st_BufSkillData* TaioistBufSkill = new st_BufSkillData();
				TaioistBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEOGRY_PUBLIC;
				TaioistBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF;

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
				TaioistBufSkill->SkillThumbnailImagePath = SkillImagePath;

				_TaioistBufSkillDatas.insert(pair<int16, st_BufSkillData*>((int16)TaioistBufSkill->SkillType, TaioistBufSkill));
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
	}
}