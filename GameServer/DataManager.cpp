#include "pch.h"
#include "DataManager.h"
#include "ObjectManager.h"
#include <atlbase.h>

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
			string ItemEquipmentPart = WeaponListFiled["ItemEquipmentPart"].GetString();
			int32 ItemWidth = WeaponListFiled["ItemWidth"].GetInt();
			int32 ItemHeight = WeaponListFiled["ItemHeight"].GetInt();
			int32 ItemMinDamage = WeaponListFiled["ItemMinDamage"].GetInt();
			int32 ItemMaxDamage = WeaponListFiled["ItemMaxDamage"].GetInt();
			int32 ItemMaxDurability = WeaponListFiled["ItemMaxDurability"].GetInt();
			int32 ItemMaxCount = WeaponListFiled["ItemMaxCount"].GetInt();
			int64 ItemCraftingTime = WeaponListFiled["ItemCraftingTime"].GetInt64();

			st_ItemInfo* WeaponItemInfo = new st_ItemInfo();

			WeaponItemInfo->ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_SWORD")
			{
				WeaponItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_SWORD;				
			}
			else if (MediumCategory == "ITEM_MEDIUM_CATEGORY_SHIELD")
			{
				WeaponItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_SHIELD;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD")
			{
				WeaponItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD;
			}
			else if (SmallCategory == "ITEM_SAMLL_CATEGORY_WEAPON_WOOD_SHIELD")
			{
				WeaponItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SAMLL_CATEGORY_WEAPON_WOOD_SHIELD;
			}

			if (ItemObjectType == "OBJECT_ITEM_WEAPON_WOOD_SWORD")
			{
				WeaponItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD;
			}
			else if (ItemObjectType == "OBJECT_ITEM_WEAPON_WOOD_SHIELD")
			{
				WeaponItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SHIELD;
			}
						
			WeaponItemInfo->ItemExplain = (LPWSTR)CA2W(ItemExplain.c_str());
			WeaponItemInfo->ItemName = (LPWSTR)CA2W(ItemName.c_str());

			if (ItemEquipmentPart == "EQUIPMENT_PARTS_LEFT_HAND")
			{
				WeaponItemInfo->ItemEquipmentPart = en_EquipmentParts::EQUIPMENT_PARTS_LEFT_HAND;
			}
			else if (ItemEquipmentPart == "EQUIPMENT_PARTS_RIGHT_HAND")
			{
				WeaponItemInfo->ItemEquipmentPart = en_EquipmentParts::EQUIPMENT_PARTS_RIGHT_HAND;
			}

			WeaponItemInfo->ItemWidth = ItemWidth;
			WeaponItemInfo->ItemHeight = ItemHeight;
			WeaponItemInfo->ItemMinDamage = ItemMinDamage;
			WeaponItemInfo->ItemMaxDamage = ItemMaxDamage;			
			WeaponItemInfo->ItemMaxDurability = ItemMaxDurability;
			WeaponItemInfo->ItemMaxCount = ItemMaxCount;
			WeaponItemInfo->ItemCraftingTime = ItemCraftingTime;

			_Items.insert(pair<int16, st_ItemInfo*>((int16)WeaponItemInfo->ItemSmallCategory, WeaponItemInfo));
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
			string ItemEquipmentPart = ArmorListFiled["ItemEquipmentPart"].GetString();
			int32 ItemWidth = ArmorListFiled["ItemWidth"].GetInt();
			int32 ItemHeight = ArmorListFiled["ItemHeight"].GetInt();
			int32 ItemDefence = ArmorListFiled["ItemDefence"].GetInt();
			int32 ItemMaxDurability = ArmorListFiled["ItemMaxDurability"].GetInt();
			int32 ItemMaxCount = ArmorListFiled["ItemMaxCount"].GetInt();
			int64 ItemCraftingTime = ArmorListFiled["ItemCraftingTime"].GetInt64();

			st_ItemInfo* ArmorItemInfo = new st_ItemInfo();

			ArmorItemInfo->ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARMOR;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_HAT")
			{
				ArmorItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_HAT;
			}
			else if (MediumCategory == "ITEM_MEDIUM_CATEGORY_WEAR")
			{
				ArmorItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAR;
			}
			else if (MediumCategory == "ITEM_MEDIUM_CATEGORY_BOOT")
			{
				ArmorItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_BOOT;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER")
			{
				ArmorItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER")
			{
				ArmorItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER")
			{
				ArmorItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER;
			}

			if (ItemObjectType == "OBJECT_ITEM_ARMOR_LEATHER_HELMET")
			{
				ArmorItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET;
			}
			else if (ItemObjectType == "OBJECT_ITEM_ARMOR_LEATHER_ARMOR")
			{
				ArmorItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_ARMOR;
			}
			else if (ItemObjectType == "OBJECT_ITEM_ARMOR_LEATHER_BOOT")
			{
				ArmorItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT;
			}

			ArmorItemInfo->ItemExplain = (LPWSTR)CA2W(ItemExplain.c_str());
			ArmorItemInfo->ItemName = (LPWSTR)CA2W(ItemName.c_str());

			if (ItemEquipmentPart == "EQUIPMENT_PARTS_HEAD")
			{
				ArmorItemInfo->ItemEquipmentPart = en_EquipmentParts::EQUIPMENT_PARTS_HEAD;
			}
			else if (ItemEquipmentPart == "EQUIPMENT_PARTS_BODY")
			{
				ArmorItemInfo->ItemEquipmentPart = en_EquipmentParts::EQUIPMENT_PARTS_BODY;
			}
			else if (ItemEquipmentPart == "EQUIPMENT_PARTS_BOOT")
			{
				ArmorItemInfo->ItemEquipmentPart = en_EquipmentParts::EQUIPMENT_PARTS_BOOT;
			}

			ArmorItemInfo->ItemWidth = ItemWidth;
			ArmorItemInfo->ItemHeight = ItemHeight;			
			ArmorItemInfo->ItemDefence = ItemDefence;
			ArmorItemInfo->ItemMaxDurability = ItemMaxDurability;
			ArmorItemInfo->ItemMaxCount = ItemMaxCount;
			ArmorItemInfo->ItemCraftingTime = ItemCraftingTime;

			_Items.insert(pair<int16, st_ItemInfo*>((int16)ArmorItemInfo->ItemSmallCategory, ArmorItemInfo));
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
			int16 ItemHealPoint = PotionDataListFiled["ItemHealPoint"].GetInt();
			int32 ItemMaxDurability = PotionDataListFiled["ItemMaxDurability"].GetInt();
			int32 ItemMaxCount = PotionDataListFiled["ItemMaxCount"].GetInt();
			int64 ItemCraftingTime = PotionDataListFiled["ItemCraftingTime"].GetInt64();

			st_ItemInfo* PotionItemInfo = new st_ItemInfo();
			PotionItemInfo->ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_POTION;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_HEAL")
			{
				PotionItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_HEAL;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_POTION_HEALTH_RESTORATION_POTION_SMALL")
			{
				PotionItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEALTH_RESTORATION_POTION_SMALL;				
			}
			else if(SmallCategory == "ITEM_SMALL_CATEGORY_POTION_MANA_RESTORATION_POTION_SMALL")
			{
				PotionItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_MANA_RESTORATION_POTION_SMALL;
			}

			if (ItemObjectType == "OBJECT_ITEM_CONSUMABLE_HEALTH_RESTORATION_POTION_SMALL")
			{
				PotionItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CONSUMABLE_HEALTH_RESTORATION_POTION_SMALL;
			}
			else if (ItemObjectType == "OBJECT_ITEM_CONSUMABLE_MANA_RESTORATION_POTION_SMALL")
			{
				PotionItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CONSUMABLE_MANA_RESTORATION_POTION_SMALL;
			}

			PotionItemInfo->ItemExplain = (LPWSTR)CA2W(ItemExplain.c_str());
			PotionItemInfo->ItemName = (LPWSTR)CA2W(ItemName.c_str());
			PotionItemInfo->ItemWidth = ItemWidth;
			PotionItemInfo->ItemHeight = ItemHeight;
			PotionItemInfo->ItemHealPoint = ItemHealPoint;
			PotionItemInfo->ItemMaxDurability = ItemMaxDurability;
			PotionItemInfo->ItemMaxCount = ItemMaxCount;
			PotionItemInfo->ItemCraftingTime = ItemCraftingTime;

			_Items.insert(pair<int16, st_ItemInfo*>((int16)PotionItemInfo->ItemSmallCategory, PotionItemInfo));
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
			int32 ItemMaxDurability = SkillBookDataListFiled["ItemMaxDurability"].GetInt();
			int32 ItemMaxCount = SkillBookDataListFiled["ItemMaxCount"].GetInt();			

			st_ItemInfo* SkillBookItemInfo = new st_ItemInfo();
			SkillBookItemInfo->ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_SKILLBOOK;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_NONE")
			{
				SkillBookItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_FIERCE_ATTACK")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_FIERCE_ATTACK;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_KNIGHT_SHAEHONE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_KNIGHT_CHOHONE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_KNIGHT_SMASH_WAVE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_BUF;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_KNIGHT_CHARGE_POSE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_SHAMAN_FLAME_HARPOON;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_SHAMAN_HELL_FIRE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEOGRY_SKILLBOOK_TAIOIST_HEALING_LIGHT")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEOGRY_SKILLBOOK_TAIOIST_HEALING_LIGHT;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_TAIOIST_HEALING_LIGHT;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_TAIOIST_HEALING_WIND")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_TAIOIST_HEALING_WIND;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_TAIOIST_HEALING_WIND;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_SHOCK_RELEASE;
			}

			if (ItemObjectType == "OBJECT_ITEM_CONSUMABLE_SKILL_BOOK")
			{
				SkillBookItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK;
			}
			
			SkillBookItemInfo->ItemExplain = (LPWSTR)CA2W(ItemExplain.c_str());
			SkillBookItemInfo->ItemName = (LPWSTR)CA2W(ItemName.c_str());
			SkillBookItemInfo->ItemWidth = ItemWidth;
			SkillBookItemInfo->ItemHeight = ItemHeight;			
			SkillBookItemInfo->ItemMaxDurability = ItemMaxDurability;
			SkillBookItemInfo->ItemMaxCount = ItemMaxCount;			

			_Items.insert(pair<int16, st_ItemInfo*>((int16)SkillBookItemInfo->ItemSmallCategory, SkillBookItemInfo));
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
			int32 ItemMaxDurability = MaterialDataListFiled["ItemMaxDurability"].GetInt();
			int32 ItemMaxCount = MaterialDataListFiled["ItemMaxCount"].GetInt();
			int64 ItemCraftingTime = MaterialDataListFiled["ItemCraftingTime"].GetInt64();

			st_ItemInfo* MaterialItemInfo = new st_ItemInfo();
			MaterialItemInfo->ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_NONE")
			{
				MaterialItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_LEATHER")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIMEGEL;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_STONE")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_YARN")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_COPPER_NUGGET")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_NUGGET;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_COPPER_INGOT")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_INGOT;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_IRON_NUGGET")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_NUGGET;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_IRON_INGOT")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_INGOT;
			}


			// 재료 아이템의 스폰 오브젝트 타입
			if (ItemObjectType == "OBJECT_ITEM_MATERIAL_LEATHER")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_SLIME_GEL")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_BRONZE_COIN")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_SLIVER_COIN")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIVER_COIN;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_GOLD_COIN")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_GOLD_COIN;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_STONE")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_WOOD_LOG")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_WOOD_FLANK")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_YARN")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_CHAR_COAL")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_CHAR_COAL;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_COPPER_NUGGET")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_NUGGET;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_COPPER_INGOT")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_INGOT;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_IRON_NUGGET")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_NUGGET;
			}
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_IRON_INGOT")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_INGOT;
			}
						
			MaterialItemInfo->ItemExplain = (LPWSTR)CA2W(ItemExplain.c_str());
			MaterialItemInfo->ItemName = (LPWSTR)CA2W(ItemName.c_str());
			MaterialItemInfo->ItemWidth = ItemWidth;
			MaterialItemInfo->ItemHeight = ItemHeight;
			MaterialItemInfo->ItemMaxDurability = ItemMaxDurability;
			MaterialItemInfo->ItemMaxCount = ItemMaxCount;
			MaterialItemInfo->ItemCraftingTime = ItemCraftingTime;

			_Items.insert(pair<int16, st_ItemInfo*>((int16)MaterialItemInfo->ItemSmallCategory, MaterialItemInfo));
		}
	}

	for (auto& Filed : Document["Architecture"].GetArray())
	{
		for (auto& ArchitectureListFiled : Filed["ArchitectureList"].GetArray())
		{
			string MediumCategory = ArchitectureListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = ArchitectureListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = ArchitectureListFiled["ItemObjectType"].GetString();
			string ItemExplain = ArchitectureListFiled["ItemExplain"].GetString();
			string ItemName = ArchitectureListFiled["ItemName"].GetString();
			int32 ItemWidth = ArchitectureListFiled["ItemWidth"].GetInt();
			int32 ItemHeight = ArchitectureListFiled["ItemHeight"].GetInt();
			int32 ItemMaxDurability = ArchitectureListFiled["ItemMaxDurability"].GetInt();
			int32 ItemMaxCount = ArchitectureListFiled["ItemMaxCount"].GetInt();
			int64 ItemCraftingTime = ArchitectureListFiled["ItemCraftingTime"].GetInt64();

			st_ItemInfo* ArchitectureItemInfo = new st_ItemInfo();

			ArchitectureItemInfo->ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARCHITECTURE;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_CRAFTING_TABLE")
			{
				ArchitectureItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_CRAFTING_TABLE;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE")
			{
				ArchitectureItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_CRAFTING_TABLE_SAWMILL")
			{
				ArchitectureItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_SAWMILL;
			}

			if (ItemObjectType == "OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE")
			{
				ArchitectureItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE;
			}
			else if (ItemObjectType == "OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL")
			{
				ArchitectureItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL;
			}
						
			ArchitectureItemInfo->ItemExplain = (LPWSTR)CA2W(ItemExplain.c_str());
			ArchitectureItemInfo->ItemName = (LPWSTR)CA2W(ItemName.c_str());
			ArchitectureItemInfo->ItemWidth = ItemWidth;
			ArchitectureItemInfo->ItemHeight = ItemHeight;
			ArchitectureItemInfo->ItemMaxDurability = ItemMaxDurability;
			ArchitectureItemInfo->ItemMaxCount = ItemMaxCount;
			ArchitectureItemInfo->ItemCraftingTime = ItemCraftingTime;

			_Items.insert(pair<int16, st_ItemInfo*>((int16)ArchitectureItemInfo->ItemSmallCategory, ArchitectureItemInfo));
		}
	}

	for (auto& Filed : Document["CropSeed"].GetArray())
	{
		for (auto& CropSeedListFiled : Filed["CropSeedList"].GetArray())
		{
			string MediumCategory = CropSeedListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = CropSeedListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = CropSeedListFiled["ItemObjectType"].GetString();
			string ItemExplain = CropSeedListFiled["ItemExplain"].GetString();
			string ItemName = CropSeedListFiled["ItemName"].GetString();
			int32 ItemWidth = CropSeedListFiled["ItemWidth"].GetInt();
			int32 ItemHeight = CropSeedListFiled["ItemHeight"].GetInt();
			int32 ItemMaxDurability = CropSeedListFiled["ItemMaxDurability"].GetInt();			
			int32 ItemMaxCount = CropSeedListFiled["ItemMaxCount"].GetInt();			

			st_ItemInfo* CropSeedItemInfo = new st_ItemInfo();
			CropSeedItemInfo->ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_CROP;
			CropSeedItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_CROP_SEED;

			if (SmallCategory == "ITEM_SMALL_CATEGORY_CROP_SEED_POTATO")
			{
				CropSeedItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_SEED_POTATO;
			}

			if (ItemObjectType == "OBJECT_ITEM_CROP_SEED_POTATO")
			{
				CropSeedItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CROP_SEED_POTATO;
			}
			
			CropSeedItemInfo->ItemExplain = (LPWSTR)CA2W(ItemExplain.c_str());
			CropSeedItemInfo->ItemName = (LPWSTR)CA2W(ItemName.c_str());
			CropSeedItemInfo->ItemWidth = ItemWidth;
			CropSeedItemInfo->ItemHeight = ItemHeight;		
			CropSeedItemInfo->ItemMaxDurability = ItemMaxDurability;
			CropSeedItemInfo->ItemMaxCount = ItemMaxCount;			

			_Items.insert(pair<int16, st_ItemInfo*>((int16)CropSeedItemInfo->ItemSmallCategory, CropSeedItemInfo));
		}
	}

	for (auto& Filed : Document["CropFruit"].GetArray())
	{
		for (auto& CropFruitListFiled : Filed["CropFruitList"].GetArray())
		{
			string MediumCategory = CropFruitListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = CropFruitListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = CropFruitListFiled["ItemObjectType"].GetString();
			string ItemExplain = CropFruitListFiled["ItemExplain"].GetString();
			string ItemName = CropFruitListFiled["ItemName"].GetString();
			int32 ItemWidth = CropFruitListFiled["ItemWidth"].GetInt();
			int32 ItemHeight = CropFruitListFiled["ItemHeight"].GetInt();
			int32 ItemMaxDurability = CropFruitListFiled["ItemMaxDurability"].GetInt();
			int32 ItemMaxCount = CropFruitListFiled["ItemMaxCount"].GetInt();
			int8 ItemMaxStep = (int8)CropFruitListFiled["ItemMaxStep"].GetInt();
			int32 ItemGrowTime = CropFruitListFiled["ItemGrowTime"].GetInt();

			st_ItemInfo* CropFruitItemInfo = new st_ItemInfo();
			CropFruitItemInfo->ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_CROP;
			CropFruitItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_CROP_FRUIT;

			if (SmallCategory == "ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO")
			{
				CropFruitItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO;
			}

			if (ItemObjectType == "OBJECT_ITEM_CROP_FRUIT_POTATO")
			{
				CropFruitItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO;
			}
			
			CropFruitItemInfo->ItemExplain = (LPWSTR)CA2W(ItemExplain.c_str());
			CropFruitItemInfo->ItemName = (LPWSTR)CA2W(ItemName.c_str());
			CropFruitItemInfo->ItemWidth = ItemWidth;
			CropFruitItemInfo->ItemHeight = ItemHeight;
			CropFruitItemInfo->ItemMaxDurability = ItemMaxDurability;
			CropFruitItemInfo->ItemMaxCount = ItemMaxCount;			
			CropFruitItemInfo->ItemMaxstep = ItemMaxStep;
			CropFruitItemInfo->ItemGrowTime = ItemGrowTime;

			_Items.insert(pair<int16, st_ItemInfo*>((int16)CropFruitItemInfo->ItemSmallCategory, CropFruitItemInfo));
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
			float MagicHitRate = (int16)PlayerWarriorCharacterFiled["MagicHitRate"].GetFloat();
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

	for (auto& Filed : Document["PlayerShamanCharacterStatus"].GetArray())
	{
		string PlayerType = Filed["PlayerType"].GetString();

		for (auto& PlayerShamanCharacterFiled : Filed["PlayerShamanCharacterLevelDataList"].GetArray())
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
			float MagicHitRate = (int16)PlayerShamanCharacterFiled["MagicHitRate"].GetFloat();
			int Defence = PlayerShamanCharacterFiled["Defence"].GetInt();
			int16 EvasionRate = PlayerShamanCharacterFiled["EvasionRate"].GetInt();
			int16 MeleeCriticalPoint = (int16)(PlayerShamanCharacterFiled["MeleeCriticalPoint"].GetInt());
			int16 MagicCriticalPoint = (int16)(PlayerShamanCharacterFiled["MagicCriticalPoint"].GetInt());
			float Speed = PlayerShamanCharacterFiled["Speed"].GetFloat();

			st_ObjectStatusData* ShamanStatusData = new st_ObjectStatusData();

			ShamanStatusData->PlayerType = en_GameObjectType::OBJECT_SHAMAN_PLAYER;

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
			float MagicHitRate = (int16)PlayerTaioistCharacterFiled["MagicHitRate"].GetFloat();
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
			float MagicHitRate = (int16)PlayerThiefCharacterFiled["MagicHitRate"].GetFloat();
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
			float MagicHitRate = (int16)PlayerArcherCharacterFiled["MagicHitRate"].GetFloat();
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

		string MonsterName = Filed["Name"].GetString();

		en_GameObjectType MonsterType = en_GameObjectType::NORMAL;

		if (MonsterName == "슬라임")
		{
			MonsterType = en_GameObjectType::OBJECT_SLIME;
		}
		else if (MonsterName == "곰")
		{
			MonsterType = en_GameObjectType::OBJECT_BEAR;
		}

		MonsterData->MonsterName = MonsterName;

		for (auto& MonsterStatInfoFiled : Filed["MonsterStatInfo"].GetArray())
		{
			int Level = MonsterStatInfoFiled["Level"].GetInt();
			int MaxHP = MonsterStatInfoFiled["MaxHP"].GetInt();
			int MaxMP = MonsterStatInfoFiled["MaxMP"].GetInt();
			int MinMeleeAttackDamage = MonsterStatInfoFiled["MinMeleeAttackDamage"].GetInt();
			int MaxMeleeAttackDamage = MonsterStatInfoFiled["MaxMeleeAttackDamage"].GetInt();
			int16 MeleeAttackHitRate = MonsterStatInfoFiled["MeleeAttackHitRate"].GetInt();
			int16 MagicDamage = (int16)MonsterStatInfoFiled["MagicDamage"].GetInt();
			float MagicHitRate = (int16)MonsterStatInfoFiled["MagicHitRate"].GetFloat();
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
			float AttackRange = MonsterStatInfoFiled["AttackRange"].GetFloat();
			int16 GetDPPoint = (int16)MonsterStatInfoFiled["GetDPPoint"].GetInt();
			int GetExpPoint = MonsterStatInfoFiled["GetExpPoint"].GetInt();
			int64 ReSpawnTime = MonsterStatInfoFiled["ReSpawnTime"].GetInt64();

			MonsterData->MonsterStatInfo.Level = Level;
			MonsterData->MonsterStatInfo.MaxHP = MaxHP;
			MonsterData->MonsterStatInfo.MaxMP = MaxMP;
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
			MonsterData->ReSpawnTime = ReSpawnTime;
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

		_Monsters.insert(pair<en_GameObjectType, st_MonsterData*>(MonsterType, MonsterData));
	}
}

void CDataManager::LoadDataMonsterAggro(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["MonsterAggro"].GetArray())
	{
		float MonsterAggroFirstTarget = Filed["MonsterAggroFirstTarget"].GetFloat();
		float MonsterAggroSecondTarget = Filed["MonsterAggroSecondTarget"].GetFloat();
		float MonsterAggroFirstAttacker = Filed["MonsterAggroFirstAttacker"].GetFloat();
		float MonsterAggroAttacker = Filed["MonsterAggroAttacker"].GetFloat();
		float MonsterAggroHeal = Filed["MonsterAggroHeal"].GetFloat();
		float MonsterAggroGroupHeal = Filed["MonsterAggroGroupHeal"].GetFloat();
		float MonsterAggroBuf = Filed["MonsterAggroBuf"].GetFloat();
		float MonsterAggroDebuf = Filed["MonsterAggroDebuf"].GetFloat();

		_MonsterAggroData.MonsterAggroFirstTarget = MonsterAggroFirstTarget;
		_MonsterAggroData.MonsterAggroSecondTarget = MonsterAggroSecondTarget;
		_MonsterAggroData.MonsterAggroFirstAttacker = MonsterAggroFirstAttacker;
		_MonsterAggroData.MonsterAggroAttacker = MonsterAggroAttacker;
		_MonsterAggroData.MonsterAggroHeal = MonsterAggroHeal;
		_MonsterAggroData.MonsterAggroGroupHeal = MonsterAggroGroupHeal;
		_MonsterAggroData.MonsterAggroBuf = MonsterAggroBuf;
		_MonsterAggroData.MonsterAggroDebuf = MonsterAggroFirstTarget;
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
				st_AttackSkillInfo* PublicAttackSkill = new st_AttackSkillInfo();
				PublicAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
				PublicAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK;

				string SkillType = PublicAttackSkillListFiled["SkillType"].GetString();
				string SkillName = PublicAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = PublicAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = PublicAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = PublicAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = PublicAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = PublicAttackSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = PublicAttackSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = PublicAttackSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = PublicAttackSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = PublicAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();
				int8 SkillDebufAttackSpeed = (int8)PublicAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)PublicAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				int8 StatusAbnormalityProbability = (int8)PublicAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();
				string SkillUpAnimation = PublicAttackSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = PublicAttackSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = PublicAttackSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = PublicAttackSkillListFiled["SkillRightAnimation"].GetString();
				string SkillExplation = PublicAttackSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "SKILL_DEFAULT_ATTACK")
				{
					PublicAttackSkill->SkillType = en_SkillType::SKILL_DEFAULT_ATTACK;
				}

				PublicAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				PublicAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				PublicAttackSkill->SkillLevel = SkillLevel;
				PublicAttackSkill->SkillMinDamage = SkillMinDamage;
				PublicAttackSkill->SkillMaxDamage = SkillMaxDamage;
				PublicAttackSkill->SkillCoolTime = SkillCoolTime;
				PublicAttackSkill->SkillCastingTime = SkillCastingTime;
				PublicAttackSkill->SkillDurationTime = SkillDurationTime;
				PublicAttackSkill->SkillDotTime = SkillDotTime;
				PublicAttackSkill->SkillDistance = SkillDistance;
				PublicAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				PublicAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				PublicAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
				PublicAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;
				PublicAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP,    (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				PublicAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN,  (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				PublicAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT,  (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				PublicAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));				

				_PublicAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)PublicAttackSkill->SkillType, PublicAttackSkill));
			}

			for (auto& PublicTacTicSkillListFiled : PublicSkillListFiled["PublicTacTicSkillList"].GetArray())
			{
				st_TacTicSkillInfo* PublicTacTicSkill = new st_TacTicSkillInfo();
				PublicTacTicSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
				PublicTacTicSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_TACTIC;

				string SkillType = PublicTacTicSkillListFiled["SkillType"].GetString();
				string SkillName = PublicTacTicSkillListFiled["SkillName"].GetString();
				int SkillLevel = PublicTacTicSkillListFiled["SkillLevel"].GetInt();
				int SkillCoolTime = PublicTacTicSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = PublicTacTicSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = PublicTacTicSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = PublicTacTicSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = PublicTacTicSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = PublicTacTicSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = PublicTacTicSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = PublicTacTicSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = PublicTacTicSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = PublicTacTicSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = PublicTacTicSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = PublicTacTicSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "")
				{

				}

				PublicTacTicSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				PublicTacTicSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				PublicTacTicSkill->SkillLevel = SkillLevel;
				PublicTacTicSkill->SkillCoolTime = SkillCoolTime;
				PublicTacTicSkill->SkillCastingTime = SkillCastingTime;
				PublicTacTicSkill->SkillDurationTime = SkillDurationTime;
				PublicTacTicSkill->SkillDotTime = SkillDotTime;
				PublicTacTicSkill->SkillDistance = SkillDistance;
				PublicTacTicSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				PublicTacTicSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				PublicTacTicSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				PublicTacTicSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				PublicTacTicSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					PublicTacTicSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_PublicTacTicSkillDatas.insert(pair<int16, st_TacTicSkillInfo*>((int16)PublicTacTicSkill->SkillType, PublicTacTicSkill));
			}

			for (auto& PublicHealSkillListFiled : PublicSkillListFiled["PublicHealSkillList"].GetArray())
			{
				st_HealSkillInfo* PublicHealSkill = new st_HealSkillInfo();
				PublicHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
				PublicHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL;

				string SkillType = PublicHealSkillListFiled["SkillType"].GetString();
				string SkillName = PublicHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = PublicHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = PublicHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = PublicHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = PublicHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = PublicHealSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = PublicHealSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = PublicHealSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = PublicHealSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = PublicHealSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = PublicHealSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = PublicHealSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = PublicHealSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = PublicHealSkillListFiled["SkillRightAnimation"].GetString();
				string SkillExplation = PublicHealSkillListFiled["SkillExplanation"].GetString();

				PublicHealSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				PublicHealSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				PublicHealSkill->SkillLevel = SkillLevel;
				PublicHealSkill->SkillMinHealPoint = SkillMinHeal;
				PublicHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				PublicHealSkill->SkillCoolTime = SkillCoolTime;
				PublicHealSkill->SkillCastingTime = SkillCastingTime;
				PublicHealSkill->SkillDurationTime = SkillDurationTime;
				PublicHealSkill->SkillDotTime = SkillDotTime;
				PublicHealSkill->SkillDistance = SkillDistance;
				PublicHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				PublicHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				PublicHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				PublicHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				PublicHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));				

				_PublicTacTicSkillDatas.insert(pair<int16, st_HealSkillInfo*>((int16)PublicHealSkill->SkillType, PublicHealSkill));
			}

			for (auto& PublicBufSkillListFiled : PublicSkillListFiled["PublicBufSkillList"].GetArray())
			{
				st_BufSkillInfo* PublicBufSkill = new st_BufSkillInfo();
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
				int64 SkillDurationTime = PublicBufSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = PublicBufSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = PublicBufSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = PublicBufSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = PublicBufSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = PublicBufSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = PublicBufSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = PublicBufSkillListFiled["SkillRightAnimation"].GetString();
				string SkillExplation = PublicBufSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "SKILL_SHOCK_RELEASE")
				{
					PublicBufSkill->SkillType = en_SkillType::SKILL_SHOCK_RELEASE;
				}

				PublicBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				PublicBufSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
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
				PublicBufSkill->SkillDurationTime = SkillDurationTime;
				PublicBufSkill->SkillDotTime = SkillDotTime;
				PublicBufSkill->SkillDistance = SkillDistance;
				PublicBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				PublicBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				PublicBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				PublicBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				PublicBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));				

				_PublicBufSkillDatas.insert(pair<int16, st_BufSkillInfo*>((int16)PublicBufSkill->SkillType, PublicBufSkill));
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
				st_AttackSkillInfo* WarriorAttackSkill = new st_AttackSkillInfo();
				WarriorAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				WarriorAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK;

				string SkillType = WarriorAttackSkillListFiled["SkillType"].GetString();
				string SkillName = WarriorAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = WarriorAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = WarriorAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = WarriorAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = WarriorAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = WarriorAttackSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = WarriorAttackSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = WarriorAttackSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = WarriorAttackSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = WarriorAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();
				int8 SkillDebufAttackSpeed = (int8)WarriorAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)WarriorAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				int8 StatusAbnormalityProbability = (int8)WarriorAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();
				string SkillUpAnimation = WarriorAttackSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = WarriorAttackSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = WarriorAttackSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = WarriorAttackSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = WarriorAttackSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = WarriorAttackSkillListFiled["SkillExplanation"].GetString();

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
				else if (SkillType == "SKILL_KNIGHT_SHIELD_SMASH")
				{
					WarriorAttackSkill->SkillType = en_SkillType::SKILL_KNIGHT_SHIELD_SMASH;
				}

				WarriorAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				WarriorAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				WarriorAttackSkill->SkillLevel = SkillLevel;
				WarriorAttackSkill->SkillMinDamage = SkillMinDamage;
				WarriorAttackSkill->SkillMaxDamage = SkillMaxDamage;
				WarriorAttackSkill->SkillCoolTime = SkillCoolTime;
				WarriorAttackSkill->SkillCastingTime = SkillCastingTime;
				WarriorAttackSkill->SkillDurationTime = SkillDurationTime;
				WarriorAttackSkill->SkillDotTime = SkillDotTime;
				WarriorAttackSkill->SkillDistance = SkillDistance;
				WarriorAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				WarriorAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				WarriorAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;

				if (NextComboSkill == "SKILL_KNIGHT_CONVERSION_ATTACK")
				{
					WarriorAttackSkill->NextComboSkill = en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK;
				}
				else if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					WarriorAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}
				
				WarriorAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				WarriorAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				WarriorAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				WarriorAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));
				WarriorAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;

				_WarriorAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)WarriorAttackSkill->SkillType, WarriorAttackSkill));
			}

			for (auto& WarriorHealSkillListFiled : WarriorSkillListFiled["WarriorHealSkillList"].GetArray())
			{
				st_HealSkillInfo* WarriorHealSkill = new st_HealSkillInfo();
				WarriorHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				WarriorHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_HEAL;

				string SkillType = WarriorHealSkillListFiled["SkillType"].GetString();
				string SkillName = WarriorHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = WarriorHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = WarriorHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = WarriorHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = WarriorHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = WarriorHealSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = WarriorHealSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = WarriorHealSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = WarriorHealSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = WarriorHealSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = WarriorHealSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = WarriorHealSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = WarriorHealSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = WarriorHealSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = WarriorHealSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = WarriorHealSkillListFiled["SkillExplanation"].GetString();

				WarriorHealSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				WarriorHealSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				WarriorHealSkill->SkillLevel = SkillLevel;
				WarriorHealSkill->SkillMinHealPoint = SkillMinHeal;
				WarriorHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				WarriorHealSkill->SkillCoolTime = SkillCoolTime;
				WarriorHealSkill->SkillCastingTime = SkillCastingTime;
				WarriorHealSkill->SkillDurationTime = SkillDurationTime;
				WarriorHealSkill->SkillDotTime = SkillDotTime;
				WarriorHealSkill->SkillDistance = SkillDistance;
				WarriorHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					WarriorHealSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}
				
				WarriorHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				WarriorHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				WarriorHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				WarriorHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				_WarriorTacTicSkillDatas.insert(pair<int16, st_HealSkillInfo*>((int16)WarriorHealSkill->SkillType, WarriorHealSkill));
			}

			for (auto& WarriorBufSkillListFiled : WarriorSkillListFiled["WarriorBufSkillList"].GetArray())
			{
				st_BufSkillInfo* WarriorBufSkill = new st_BufSkillInfo();
				WarriorBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				WarriorBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_BUF;

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
				int64 SkillDurationTime = WarriorBufSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = WarriorBufSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = WarriorBufSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = WarriorBufSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = WarriorBufSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = WarriorBufSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = WarriorBufSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = WarriorBufSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = WarriorBufSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = WarriorBufSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "SKILL_KNIGHT_CHARGE_POSE")
				{
					WarriorBufSkill->SkillType = en_SkillType::SKILL_KNIGHT_CHARGE_POSE;
				}

				WarriorBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				WarriorBufSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
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
				WarriorBufSkill->SkillDurationTime = SkillDurationTime;
				WarriorBufSkill->SkillDotTime = SkillDotTime;
				WarriorBufSkill->SkillDistance = SkillDistance;
				WarriorBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				WarriorBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				WarriorBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				WarriorBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				WarriorBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					WarriorBufSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_WarriorBufSkillDatas.insert(pair<int16, st_BufSkillInfo*>((int16)WarriorBufSkill->SkillType, WarriorBufSkill));
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
				st_AttackSkillInfo* ShamanAttackSkill = new st_AttackSkillInfo();
				ShamanAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
				ShamanAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK;

				string SkillType = ShmanAttackSkillListFiled["SkillType"].GetString();
				string SkillName = ShmanAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = ShmanAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = ShmanAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = ShmanAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = ShmanAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ShmanAttackSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = ShmanAttackSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = ShmanAttackSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = ShmanAttackSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ShmanAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();
				int8 SkillDebufAttackSpeed = (int8)ShmanAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)ShmanAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				int8 StatusAbnormalityProbability = (int8)ShmanAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();
				string SkillUpAnimation = ShmanAttackSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = ShmanAttackSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = ShmanAttackSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = ShmanAttackSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = ShmanAttackSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = ShmanAttackSkillListFiled["SkillExplanation"].GetString();

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

				ShamanAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				ShamanAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				ShamanAttackSkill->SkillLevel = SkillLevel;
				ShamanAttackSkill->SkillMinDamage = SkillMinDamage;
				ShamanAttackSkill->SkillMaxDamage = SkillMaxDamage;
				ShamanAttackSkill->SkillCoolTime = SkillCoolTime;
				ShamanAttackSkill->SkillCastingTime = SkillCastingTime;
				ShamanAttackSkill->SkillDurationTime = SkillDurationTime;
				ShamanAttackSkill->SkillDotTime = SkillDotTime;
				ShamanAttackSkill->SkillDistance = SkillDistance;
				ShamanAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ShamanAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				ShamanAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
				ShamanAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;
				ShamanAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				ShamanAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				ShamanAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				ShamanAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					ShamanAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}
				else if (NextComboSkill == "SKILL_SHAMAN_ICE_WAVE")
				{
					ShamanAttackSkill->NextComboSkill = en_SkillType::SKILL_SHAMAN_ICE_WAVE;
				}				

				_ShamanAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)ShamanAttackSkill->SkillType, ShamanAttackSkill));
			}

			for (auto& ShamanTacTicSkillListFiled : ShamanSkillListFiled["ShamanTacTicSkillList"].GetArray())
			{
				st_TacTicSkillInfo* ShamanTacTicSkill = new st_TacTicSkillInfo();
				ShamanTacTicSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
				ShamanTacTicSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_TACTIC;

				string SkillType = ShamanTacTicSkillListFiled["SkillType"].GetString();
				string SkillName = ShamanTacTicSkillListFiled["SkillName"].GetString();
				int SkillLevel = ShamanTacTicSkillListFiled["SkillLevel"].GetInt();
				int SkillCoolTime = ShamanTacTicSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ShamanTacTicSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = ShamanTacTicSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = ShamanTacTicSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = ShamanTacTicSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ShamanTacTicSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = ShamanTacTicSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = ShamanTacTicSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = ShamanTacTicSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = ShamanTacTicSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = ShamanTacTicSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = ShamanTacTicSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "SKILL_SHAMAN_BACK_TELEPORT")
				{
					ShamanTacTicSkill->SkillType = en_SkillType::SKILL_SHAMAN_BACK_TELEPORT;
				}

				ShamanTacTicSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				ShamanTacTicSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				ShamanTacTicSkill->SkillLevel = SkillLevel;
				ShamanTacTicSkill->SkillCoolTime = SkillCoolTime;
				ShamanTacTicSkill->SkillCastingTime = SkillCastingTime;
				ShamanTacTicSkill->SkillDurationTime = SkillDurationTime;
				ShamanTacTicSkill->SkillDotTime = SkillDotTime;
				ShamanTacTicSkill->SkillDistance = SkillDistance;
				ShamanTacTicSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ShamanTacTicSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				ShamanTacTicSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				ShamanTacTicSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				ShamanTacTicSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					ShamanTacTicSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_ShamanTacTicSkillDatas.insert(pair<int16, st_TacTicSkillInfo*>((int16)ShamanTacTicSkill->SkillType, ShamanTacTicSkill));
			}

			for (auto& ShamanHealSkillListFiled : ShamanSkillListFiled["ShamanHealSkillList"].GetArray())
			{
				st_HealSkillInfo* ShamanHealSkill = new st_HealSkillInfo();
				ShamanHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
				ShamanHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_HEAL;

				string SkillType = ShamanHealSkillListFiled["SkillType"].GetString();
				string SkillName = ShamanHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = ShamanHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = ShamanHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = ShamanHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = ShamanHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ShamanHealSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = ShamanHealSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = ShamanHealSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = ShamanHealSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ShamanHealSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = ShamanHealSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = ShamanHealSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = ShamanHealSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = ShamanHealSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = ShamanHealSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = ShamanHealSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "")
				{

				}

				ShamanHealSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				ShamanHealSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				ShamanHealSkill->SkillLevel = SkillLevel;
				ShamanHealSkill->SkillMinHealPoint = SkillMinHeal;
				ShamanHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				ShamanHealSkill->SkillCoolTime = SkillCoolTime;
				ShamanHealSkill->SkillCastingTime = SkillCastingTime;
				ShamanHealSkill->SkillDurationTime = SkillDurationTime;
				ShamanHealSkill->SkillDotTime = SkillDotTime;
				ShamanHealSkill->SkillDistance = SkillDistance;
				ShamanHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ShamanHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				ShamanHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				ShamanHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				ShamanHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));


				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					ShamanHealSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_ShamanTacTicSkillDatas.insert(pair<int16, st_HealSkillInfo*>((int16)ShamanHealSkill->SkillType, ShamanHealSkill));
			}

			for (auto& ShmanBufSkillListFiled : ShamanSkillListFiled["ShamanBufSkillList"].GetArray())
			{
				st_BufSkillInfo* ShamanBufSkill = new st_BufSkillInfo();
				ShamanBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
				ShamanBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_BUF;

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
				int64 SkillDurationTime = ShmanBufSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = ShmanBufSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = ShmanBufSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ShmanBufSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = ShmanBufSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = ShmanBufSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = ShmanBufSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = ShmanBufSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = ShmanBufSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = ShmanBufSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "")
				{

				}

				ShamanBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				ShamanBufSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
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
				ShamanBufSkill->SkillDurationTime = SkillDurationTime;
				ShamanBufSkill->SkillDotTime = SkillDotTime;
				ShamanBufSkill->SkillDistance = SkillDistance;
				ShamanBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ShamanBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				ShamanBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				ShamanBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				ShamanBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					ShamanBufSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_ShamanBufSkillDatas.insert(pair<int16, st_BufSkillInfo*>((int16)ShamanBufSkill->SkillType, ShamanBufSkill));
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
				st_AttackSkillInfo* TaioistAttackSkill = new st_AttackSkillInfo();
				TaioistAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
				TaioistAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK;

				string SkillType = TaioistAttackSkillListFiled["SkillType"].GetString();
				string SkillName = TaioistAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = TaioistAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = TaioistAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = TaioistAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = TaioistAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = TaioistAttackSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = TaioistAttackSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = TaioistAttackSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = TaioistAttackSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = TaioistAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();
				int64 SkillDebufTime = TaioistAttackSkillListFiled["SkillDebufTime"].GetInt64();
				int8 SkillDebufAttackSpeed = (int8)TaioistAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)TaioistAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				int8 StatusAbnormalityProbability = (int8)TaioistAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();
				string SkillUpAnimation = TaioistAttackSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = TaioistAttackSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = TaioistAttackSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = TaioistAttackSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = TaioistAttackSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = TaioistAttackSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "SKILL_TAIOIST_DIVINE_STRIKE")
				{
					TaioistAttackSkill->SkillType = en_SkillType::SKILL_TAIOIST_DIVINE_STRIKE;
				}
				else if (SkillType == "SKILL_TAIOIST_ROOT")
				{
					TaioistAttackSkill->SkillType = en_SkillType::SKILL_TAIOIST_ROOT;
				}

				TaioistAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				TaioistAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				TaioistAttackSkill->SkillLevel = SkillLevel;
				TaioistAttackSkill->SkillMinDamage = SkillMinDamage;
				TaioistAttackSkill->SkillMaxDamage = SkillMaxDamage;
				TaioistAttackSkill->SkillCoolTime = SkillCoolTime;
				TaioistAttackSkill->SkillCastingTime = SkillCastingTime;
				TaioistAttackSkill->SkillDurationTime = SkillDurationTime;
				TaioistAttackSkill->SkillDotTime = SkillDotTime;
				TaioistAttackSkill->SkillDistance = SkillDistance;
				TaioistAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				TaioistAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				TaioistAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
				TaioistAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;
				TaioistAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				TaioistAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				TaioistAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				TaioistAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					TaioistAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_TaioistAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)TaioistAttackSkill->SkillType, TaioistAttackSkill));
			}

			for (auto& TaioistHealSkillListFiled : TaioistSkillListFiled["TaioistHealSkillList"].GetArray())
			{
				st_HealSkillInfo* TaioistHealSkill = new st_HealSkillInfo();
				TaioistHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
				TaioistHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL;

				string SkillType = TaioistHealSkillListFiled["SkillType"].GetString();
				string SkillName = TaioistHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = TaioistHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = TaioistHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = TaioistHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = TaioistHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = TaioistHealSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = TaioistHealSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = TaioistHealSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = TaioistHealSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = TaioistHealSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = TaioistHealSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = TaioistHealSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = TaioistHealSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = TaioistHealSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = TaioistHealSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = TaioistHealSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "SKILL_TAIOIST_HEALING_LIGHT")
				{
					TaioistHealSkill->SkillType = en_SkillType::SKILL_TAIOIST_HEALING_LIGHT;
				}
				else if (SkillType == "SKILL_TAIOIST_HEALING_WIND")
				{
					TaioistHealSkill->SkillType = en_SkillType::SKILL_TAIOIST_HEALING_WIND;
				}

				TaioistHealSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				TaioistHealSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				TaioistHealSkill->SkillLevel = SkillLevel;
				TaioistHealSkill->SkillMinHealPoint = SkillMinHeal;
				TaioistHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				TaioistHealSkill->SkillCoolTime = SkillCoolTime;
				TaioistHealSkill->SkillCastingTime = SkillCastingTime;
				TaioistHealSkill->SkillDurationTime = SkillDurationTime;
				TaioistHealSkill->SkillDotTime = SkillDotTime;
				TaioistHealSkill->SkillDistance = SkillDistance;
				TaioistHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				TaioistHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				TaioistHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				TaioistHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				TaioistHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					TaioistHealSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_TaioistTacTicSkillDatas.insert(pair<int16, st_HealSkillInfo*>((int16)TaioistHealSkill->SkillType, TaioistHealSkill));
			}

			for (auto& TaioistBufSkillListFiled : TaioistSkillListFiled["TaioistBufSkillList"].GetArray())
			{
				st_BufSkillInfo* TaioistBufSkill = new st_BufSkillInfo();
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
				int64 SkillDurationTime = TaioistBufSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = TaioistBufSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = TaioistBufSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = TaioistBufSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = TaioistBufSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = TaioistBufSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = TaioistBufSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = TaioistBufSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = TaioistBufSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = TaioistBufSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "")
				{

				}

				TaioistBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				TaioistBufSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
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
				TaioistBufSkill->SkillDurationTime = SkillDurationTime;
				TaioistBufSkill->SkillDotTime = SkillDotTime;
				TaioistBufSkill->SkillDistance = SkillDistance;
				TaioistBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				TaioistBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				TaioistBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				TaioistBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				TaioistBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					TaioistBufSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_TaioistBufSkillDatas.insert(pair<int16, st_BufSkillInfo*>((int16)TaioistBufSkill->SkillType, TaioistBufSkill));
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
				st_AttackSkillInfo* ThiefAttackSkill = new st_AttackSkillInfo();
				ThiefAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_THIEF;
				ThiefAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_ATTACK;

				string SkillType = ThiefAttackSkillListFiled["SkillType"].GetString();
				string SkillName = ThiefAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = ThiefAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = ThiefAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = ThiefAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = ThiefAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ThiefAttackSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = ThiefAttackSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = ThiefAttackSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = ThiefAttackSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ThiefAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();
				int8 SkillDebufAttackSpeed = (int8)ThiefAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)ThiefAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				int64 SkillDamageOverTime = ThiefAttackSkillListFiled["SkillDebufDamageOverTime"].GetInt64();
				int8 StatusAbnormalityProbability = (int8)ThiefAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();
				string SkillUpAnimation = ThiefAttackSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = ThiefAttackSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = ThiefAttackSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = ThiefAttackSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = ThiefAttackSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = ThiefAttackSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "SKILL_THIEF_QUICK_CUT")
				{
					ThiefAttackSkill->SkillType = en_SkillType::SKILL_THIEF_QUICK_CUT;
				}

				ThiefAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				ThiefAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				ThiefAttackSkill->SkillLevel = SkillLevel;
				ThiefAttackSkill->SkillMinDamage = SkillMinDamage;
				ThiefAttackSkill->SkillMaxDamage = SkillMaxDamage;
				ThiefAttackSkill->SkillCoolTime = SkillCoolTime;
				ThiefAttackSkill->SkillCastingTime = SkillCastingTime;
				ThiefAttackSkill->SkillDurationTime = SkillDurationTime;
				ThiefAttackSkill->SkillDotTime = SkillDotTime;
				ThiefAttackSkill->SkillDistance = SkillDistance;
				ThiefAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ThiefAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				ThiefAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
				ThiefAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;
				ThiefAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				ThiefAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				ThiefAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				ThiefAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					ThiefAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_ThiefAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)ThiefAttackSkill->SkillType, ThiefAttackSkill));
			}

			for (auto& ThiefHealSkillListFiled : ThiefSkillListFiled["ThiefHealSkillList"].GetArray())
			{
				st_HealSkillInfo* ThiefHealSkill = new st_HealSkillInfo();
				ThiefHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_THIEF;
				ThiefHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_HEAL;

				string SkillType = ThiefHealSkillListFiled["SkillType"].GetString();
				string SkillName = ThiefHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = ThiefHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = ThiefHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = ThiefHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = ThiefHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ThiefHealSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = ThiefHealSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = ThiefHealSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = ThiefHealSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ThiefHealSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = ThiefHealSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = ThiefHealSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = ThiefHealSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = ThiefHealSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = ThiefHealSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = ThiefHealSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "")
				{

				}

				ThiefHealSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				ThiefHealSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				ThiefHealSkill->SkillLevel = SkillLevel;
				ThiefHealSkill->SkillMinHealPoint = SkillMinHeal;
				ThiefHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				ThiefHealSkill->SkillCoolTime = SkillCoolTime;
				ThiefHealSkill->SkillCastingTime = SkillCastingTime;
				ThiefHealSkill->SkillDurationTime = SkillDurationTime;
				ThiefHealSkill->SkillDotTime = SkillDotTime;
				ThiefHealSkill->SkillDistance = SkillDistance;
				ThiefHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ThiefHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				ThiefHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				ThiefHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				ThiefHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					ThiefHealSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_ThiefTacTicSkillDatas.insert(pair<int16, st_HealSkillInfo*>((int16)ThiefHealSkill->SkillType, ThiefHealSkill));
			}

			for (auto& ThiefBufSkillListFiled : ThiefSkillListFiled["ThiefBufSkillList"].GetArray())
			{
				st_BufSkillInfo* ThiefBufSkill = new st_BufSkillInfo();
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
				int64 SkillDurationTime = ThiefBufSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = ThiefBufSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = ThiefBufSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ThiefBufSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = ThiefBufSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = ThiefBufSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = ThiefBufSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = ThiefBufSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = ThiefBufSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = ThiefBufSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "")
				{

				}

				ThiefBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				ThiefBufSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
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
				ThiefBufSkill->SkillDurationTime = SkillDurationTime;
				ThiefBufSkill->SkillDotTime = SkillDotTime;
				ThiefBufSkill->SkillDistance = SkillDistance;
				ThiefBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ThiefBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				ThiefBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				ThiefBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				ThiefBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					ThiefBufSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_ThiefBufSkillDatas.insert(pair<int16, st_BufSkillInfo*>((int16)ThiefBufSkill->SkillType, ThiefBufSkill));
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
				st_AttackSkillInfo* ArcherAttackSkill = new st_AttackSkillInfo();
				ArcherAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_ARCHER;
				ArcherAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_ATTACK;

				string SkillType = ArcherAttackSkillListFiled["SkillType"].GetString();
				string SkillName = ArcherAttackSkillListFiled["SkillName"].GetString();
				int SkillLevel = ArcherAttackSkillListFiled["SkillLevel"].GetInt();
				int SkillMinDamage = ArcherAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = ArcherAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = ArcherAttackSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ArcherAttackSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = ArcherAttackSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = ArcherAttackSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = ArcherAttackSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ArcherAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();
				int64 SkillDebufTime = ArcherAttackSkillListFiled["SkillDebufTime"].GetInt64();
				int8 SkillDebufAttackSpeed = (int8)ArcherAttackSkillListFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)ArcherAttackSkillListFiled["SkillDebufMovingSpeed"].GetInt();
				int8 StatusAbnormalityProbability = (int8)ArcherAttackSkillListFiled["StatusAbnormalityProbability"].GetInt();
				string SkillUpAnimation = ArcherAttackSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = ArcherAttackSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = ArcherAttackSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = ArcherAttackSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = ArcherAttackSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = ArcherAttackSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "SKILL_ARCHER_SNIFING")
				{
					ArcherAttackSkill->SkillType = en_SkillType::SKILL_ARCHER_SNIFING;
				}

				ArcherAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				ArcherAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				ArcherAttackSkill->SkillLevel = SkillLevel;
				ArcherAttackSkill->SkillMinDamage = SkillMinDamage;
				ArcherAttackSkill->SkillMaxDamage = SkillMaxDamage;
				ArcherAttackSkill->SkillCoolTime = SkillCoolTime;
				ArcherAttackSkill->SkillCastingTime = SkillCastingTime;
				ArcherAttackSkill->SkillDurationTime = SkillDurationTime;
				ArcherAttackSkill->SkillDotTime = SkillDotTime;
				ArcherAttackSkill->SkillDistance = SkillDistance;
				ArcherAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ArcherAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				ArcherAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
				ArcherAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;
				ArcherAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				ArcherAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				ArcherAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				ArcherAttackSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					ArcherAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_ArcherAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)ArcherAttackSkill->SkillType, ArcherAttackSkill));
			}

			for (auto& ArcherHealSkillListFiled : ArcherSkillListFiled["ArcherHealSkillList"].GetArray())
			{
				st_HealSkillInfo* ArcherHealSkill = new st_HealSkillInfo();
				ArcherHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_ARCHER;
				ArcherHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_HEAL;

				string SkillType = ArcherHealSkillListFiled["SkillType"].GetString();
				string SkillName = ArcherHealSkillListFiled["SkillName"].GetString();
				int SkillLevel = ArcherHealSkillListFiled["SkillLevel"].GetInt();
				int SkillMinHeal = ArcherHealSkillListFiled["SkillMinHeal"].GetInt();
				int SkillMaxHeal = ArcherHealSkillListFiled["SkillMaxHeal"].GetInt();
				int SkillCoolTime = ArcherHealSkillListFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = ArcherHealSkillListFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = ArcherHealSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = ArcherHealSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = ArcherHealSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ArcherHealSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = ArcherHealSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = ArcherHealSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = ArcherHealSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = ArcherHealSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = ArcherHealSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = ArcherHealSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "")
				{

				}

				ArcherHealSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				ArcherHealSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				ArcherHealSkill->SkillLevel = SkillLevel;
				ArcherHealSkill->SkillMinHealPoint = SkillMinHeal;
				ArcherHealSkill->SkillMaxHealPoint = SkillMaxHeal;
				ArcherHealSkill->SkillCoolTime = SkillCoolTime;
				ArcherHealSkill->SkillCastingTime = SkillCastingTime;
				ArcherHealSkill->SkillDurationTime = SkillDurationTime;
				ArcherHealSkill->SkillDotTime = SkillDotTime;
				ArcherHealSkill->SkillDistance = SkillDistance;
				ArcherHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ArcherHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				ArcherHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				ArcherHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				ArcherHealSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					ArcherHealSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_ArcherTacTicSkillDatas.insert(pair<int16, st_HealSkillInfo*>((int16)ArcherHealSkill->SkillType, ArcherHealSkill));
			}

			for (auto& ArcherBufSkillListFiled : ArcherSkillListFiled["ArcherBufSkillList"].GetArray())
			{
				st_BufSkillInfo* ArcherBufSkill = new st_BufSkillInfo();
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
				int64 SkillDurationTime = ArcherBufSkillListFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = ArcherBufSkillListFiled["SkillDotTime"].GetInt64();
				int SkillDistance = ArcherBufSkillListFiled["SkillDistance"].GetInt();
				float SkillTargetEffectTime = ArcherBufSkillListFiled["SkillTargetEffectTime"].GetFloat();
				string SkillUpAnimation = ArcherBufSkillListFiled["SkillUpAnimation"].GetString();
				string SkillDownAnimation = ArcherBufSkillListFiled["SkillDownAnimation"].GetString();
				string SkillLeftAnimation = ArcherBufSkillListFiled["SkillLeftAnimation"].GetString();
				string SkillRightAnimation = ArcherBufSkillListFiled["SkillRightAnimation"].GetString();
				string NextComboSkill = ArcherBufSkillListFiled["NextComboSkill"].GetString();
				string SkillExplation = ArcherBufSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "")
				{

				}

				ArcherBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				ArcherBufSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
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
				ArcherBufSkill->SkillDurationTime = SkillDurationTime;
				ArcherBufSkill->SkillDotTime = SkillDotTime;
				ArcherBufSkill->SkillDistance = SkillDistance;
				ArcherBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				ArcherBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::UP, (LPWSTR)CA2W(SkillUpAnimation.c_str())));
				ArcherBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::DOWN, (LPWSTR)CA2W(SkillDownAnimation.c_str())));
				ArcherBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::LEFT, (LPWSTR)CA2W(SkillLeftAnimation.c_str())));
				ArcherBufSkill->SkillAnimations.insert(pair<en_MoveDir, wstring>(en_MoveDir::RIGHT, (LPWSTR)CA2W(SkillRightAnimation.c_str())));

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					ArcherBufSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}				

				_ArcherBufSkillDatas.insert(pair<int16, st_BufSkillInfo*>((int16)ArcherBufSkill->SkillType, ArcherBufSkill));
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

		string EnvironmentName = Filed["Name"].GetString();

		en_GameObjectType EnvironmentType = en_GameObjectType::NORMAL;

		if (EnvironmentName == "돌")
		{
			EnvironmentType = en_GameObjectType::OBJECT_STONE;
		}
		else if (EnvironmentName == "나무")
		{
			EnvironmentType = en_GameObjectType::OBJECT_TREE;
		}

		EnvironmentData->EnvironmentName = EnvironmentName;

		for (auto& EnvironmentStatInfoFiled : Filed["EnvironmentStatInfo"].GetArray())
		{
			int Level = EnvironmentStatInfoFiled["Level"].GetInt();
			int MaxHP = EnvironmentStatInfoFiled["MaxHP"].GetInt();
			long RecoveryTime = EnvironmentStatInfoFiled["RecoveryTime"].GetInt64();

			EnvironmentData->Level = Level;
			EnvironmentData->MaxHP = MaxHP;
			EnvironmentData->RecoveryTime = RecoveryTime;
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

		_Environments.insert(pair<en_GameObjectType, st_EnvironmentData*>(EnvironmentType, EnvironmentData));
	}
}

void CDataManager::LoadDataCrop(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["Crops"].GetArray())
	{
		st_CropData* CropData = new st_CropData();

		string CropName = Filed["CropName"].GetString();

		en_GameObjectType CropType = en_GameObjectType::NORMAL;

		if (CropName == "감자")
		{
			CropType = en_GameObjectType::OBJECT_CROP_POTATO;
		}

		CropData->CropName = CropName;

		for (auto& CropStatInfoFiled : Filed["CropStatInfo"].GetArray())
		{
			int MaxHP = CropStatInfoFiled["MaxHP"].GetInt();

			CropData->MaxHP = MaxHP;
		}

		for (auto& CropDropDataFiled : Filed["CropDropData"].GetArray())
		{
			int Probability = CropDropDataFiled["Probability"].GetInt();
			string DropItemSmallCategory = CropDropDataFiled["DropItemSmallCategory"].GetString();
			int8 MinCount = (int8)(CropDropDataFiled["MinCount"].GetInt());
			int16 MaxCount = (int16)(CropDropDataFiled["MaxCount"].GetInt());

			st_DropData DropData;

			if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO;
			}

			DropData.Probability = Probability;
			DropData.MinCount = MinCount;
			DropData.MaxCount = MaxCount;

			CropData->DropItems.push_back(DropData);
		}

		_Crops.insert(pair<en_GameObjectType, st_CropData*>(CropType, CropData));
	}
}

void CDataManager::LoadDataCrafting(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["CraftingData"].GetArray())
	{
		st_CraftingItemCategory* CraftingItemCategory = new st_CraftingItemCategory();

		string CraftingItemLargeCategory = Filed["CraftingCompleteItemLargeCategory"].GetString();
		string CraftingTypeName = Filed["CraftingTypeName"].GetString();

		if (CraftingItemLargeCategory == "ITEM_LARGE_CATEGORY_ARCHITECTURE")
		{
			CraftingItemCategory->CategoryType = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARCHITECTURE;
		}
		else if (CraftingItemLargeCategory == "ITEM_LARGE_CATEGORY_WEAPON")
		{
			CraftingItemCategory->CategoryType = en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON;
		}
		else if (CraftingItemLargeCategory == "ITEM_LARGE_CATEGORY_ARMOR")
		{
			CraftingItemCategory->CategoryType = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARMOR;
		}
		else if (CraftingItemLargeCategory == "ITEM_LARGE_CATEGORY_MATERIAL")
		{
			CraftingItemCategory->CategoryType = en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL;
		}

		CraftingItemCategory->CategoryName = (LPWSTR)CA2W(CraftingTypeName.c_str());

		for (auto& CraftingCompleteItemFiled : Filed["CraftingCompleteItem"].GetArray())
		{
			CItem* CommonCraftingCompleteItem = nullptr;

			string CraftingCompleteItemMediumCategory = CraftingCompleteItemFiled["CraftingCompleteItemMediumCategory"].GetString();
			string CraftingCompleteItemSmallCategory = CraftingCompleteItemFiled["CraftingCompleteItemSmallCategory"].GetString();

			string CraftingCompleteItemName = CraftingCompleteItemFiled["CraftingCompleteItemName"].GetString();
			string CraftingCompleteItemThumbnailImagePath = CraftingCompleteItemFiled["CraftingCompleteItemThumbnailImagePath"].GetString();

			if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARCHITECTURE;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_CRAFTING_TABLE;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_CRAFTING_TABLE_SAWMILL")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_SAWMILL);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARCHITECTURE;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_CRAFTING_TABLE;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_SAWMILL;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_SWORD;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_SWORD_WOOD;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARMOR;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAR;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARMOR;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAR;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARMOR;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAR;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_YARN")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}

			st_ItemInfo* CraftingCompleteItemData = FindItemData(CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory);

			CommonCraftingCompleteItem->_ItemInfo.ItemExplain = CraftingCompleteItemData->ItemExplain;
			CommonCraftingCompleteItem->_ItemInfo.ItemName = CraftingCompleteItemData->ItemName;
			CommonCraftingCompleteItem->_ItemInfo.ItemWidth = CraftingCompleteItemData->ItemWidth;
			CommonCraftingCompleteItem->_ItemInfo.ItemHeight = CraftingCompleteItemData->ItemHeight;
			CommonCraftingCompleteItem->_ItemInfo.ItemCraftingTime = CraftingCompleteItemData->ItemCraftingTime;
			CommonCraftingCompleteItem->_ItemInfo.ItemMaxCount = CraftingCompleteItemData->ItemMaxCount;

			for (auto& CraftingMaterialFiled : CraftingCompleteItemFiled["CraftingMaterial"].GetArray())
			{
				st_CraftingMaterialItemInfo CraftingMaterialItemInfo;

				string MaterialSmallCategory = CraftingMaterialFiled["MaterialSmallCategory"].GetString();
				string MaterialName = CraftingMaterialFiled["MaterialName"].GetString();
				int16 MaterialCount = (int16)CraftingMaterialFiled["MaterialCount"].GetInt();

				if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_LEATHER")
				{
					CraftingMaterialItemInfo.MaterialItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_LEATHER;
				}
				else if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG")
				{
					CraftingMaterialItemInfo.MaterialItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG;
				}
				else if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK")
				{
					CraftingMaterialItemInfo.MaterialItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK;
				}
				else if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_YARN")
				{
					CraftingMaterialItemInfo.MaterialItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_YARN;
				}
				else if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_STONE")
				{
					CraftingMaterialItemInfo.MaterialItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE;
				}

				CraftingMaterialItemInfo.MaterialItemName = (LPWSTR)CA2W(MaterialName.c_str());
				CraftingMaterialItemInfo.ItemCount = MaterialCount;

				CommonCraftingCompleteItem->_ItemInfo.Materials.push_back(CraftingMaterialItemInfo);
			}

			CraftingItemCategory->CommonCraftingCompleteItems.push_back(CommonCraftingCompleteItem);
		}

		_CraftingData.insert(pair<int8, st_CraftingItemCategory*>((int8)CraftingItemCategory->CategoryType, CraftingItemCategory));
	}
}

void CDataManager::LoadDataCraftingTable(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["CraftingTableData"].GetArray())
	{
		st_CraftingTableRecipe* CraftingTableRecipe = new st_CraftingTableRecipe();

		string CraftingTableName = Filed["CraftingTableName"].GetString();

		CraftingTableRecipe->CraftingTableName = (LPWSTR)CA2W(CraftingTableName.c_str());

		if (CraftingTableName == "용광로")
		{
			CraftingTableRecipe->CraftingTableType = en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE;
		}
		else if (CraftingTableName == "제재소")
		{
			CraftingTableRecipe->CraftingTableType = en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL;
		}

		for (auto& CraftingTableCompleteItemFiled : Filed["CraftingTableCompleteItem"].GetArray())
		{
			CItem* CraftingCompleteItem = nullptr;

			string CraftingCompleteItemMediumCategory = CraftingTableCompleteItemFiled["CraftingCompleteItemMediumCategory"].GetString();
			string CraftingCompleteItemSmallCategory = CraftingTableCompleteItemFiled["CraftingCompleteItemSmallCategory"].GetString();

			string CraftingCompleteItemName = CraftingTableCompleteItemFiled["CraftingCompleteItemName"].GetString();

			if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL")
			{
				CraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL);
				CraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL;
				CraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL;
				CraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_FURNACE;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_COPPER_INGOT")
			{
				CraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_INGOT);
				CraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL;
				CraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_INGOT;
				CraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_FURNACE;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_IRON_INGOT")
			{
				CraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_INGOT);
				CraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL;
				CraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_INGOT;
				CraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_FURNACE;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK")
			{
				CraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK);
				CraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_MATERIAL;
				CraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK;
				CraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_SAWMILL;
			}

			st_ItemInfo* CraftingCompleteItemData = FindItemData(CraftingCompleteItem->_ItemInfo.ItemSmallCategory);

			CraftingCompleteItem->_ItemInfo.ItemExplain = CraftingCompleteItemData->ItemExplain;
			CraftingCompleteItem->_ItemInfo.ItemName = CraftingCompleteItemData->ItemName;
			CraftingCompleteItem->_ItemInfo.ItemWidth = CraftingCompleteItemData->ItemWidth;
			CraftingCompleteItem->_ItemInfo.ItemHeight = CraftingCompleteItemData->ItemHeight;
			CraftingCompleteItem->_ItemInfo.ItemCraftingTime = CraftingCompleteItemData->ItemCraftingTime;
			CraftingCompleteItem->_ItemInfo.ItemMaxCount = CraftingCompleteItemData->ItemMaxCount;

			for (auto& CraftingMaterialFiled : CraftingTableCompleteItemFiled["CraftingMaterial"].GetArray())
			{
				st_CraftingMaterialItemInfo CraftingMaterialItemInfo;

				string MaterialSmallCategory = CraftingMaterialFiled["MaterialSmallCategory"].GetString();
				string MaterialName = CraftingMaterialFiled["MaterialName"].GetString();
				int16 MaterialCount = (int16)CraftingMaterialFiled["MaterialCount"].GetInt();

				if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG")
				{
					CraftingMaterialItemInfo.MaterialItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG;
				}
				else if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_COPPER_NUGGET")
				{
					CraftingMaterialItemInfo.MaterialItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COPPER_NUGGET;
				}
				else if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_IRON_NUGGET")
				{
					CraftingMaterialItemInfo.MaterialItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_IRON_NUGGET;
				}
				else if (MaterialSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL")
				{
					CraftingMaterialItemInfo.MaterialItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL;
				}

				CraftingMaterialItemInfo.MaterialItemName = (LPWSTR)CA2W(MaterialName.c_str());
				CraftingMaterialItemInfo.ItemCount = MaterialCount;

				CraftingCompleteItem->_ItemInfo.Materials.push_back(CraftingMaterialItemInfo);
			}

			CraftingTableRecipe->CraftingTableCompleteItems.push_back(CraftingCompleteItem);
		}

		_CraftingTableData.insert(pair<int16, st_CraftingTableRecipe*>((int16)CraftingTableRecipe->CraftingTableType, CraftingTableRecipe));
	}
}

void CDataManager::LoadDataMapInfo(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["MapInfo"].GetArray())
	{
		st_MapInfoData* MapInfoData = new st_MapInfoData();

		int64 MapID = Filed["MapID"].GetInt64();
		string MapName = Filed["MapName"].GetString();
		int32 MapSectorSize = Filed["MapSectorSize"].GetInt();
		int8 ChannelCount = (int8)Filed["ChannelCount"].GetInt();

		MapInfoData->MapID = MapID;
		MapInfoData->MapName = MapName;
		MapInfoData->MapSectorSize = MapSectorSize;
		MapInfoData->ChannelCount = ChannelCount;

		_MapInfoDatas.insert(pair<int64, st_MapInfoData*>(MapInfoData->MapID, MapInfoData));
	}
}

void CDataManager::LoadDataOptionInfo(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Field : Document["OptionInfo"].GetArray())
	{
		st_OptionItemInfo* OptionItemInfo = new	st_OptionItemInfo();

		string OptionName = Field["OptionName"].GetString();
		string OptionType = Field["OptionType"].GetString();

		OptionItemInfo->OptionName = (LPWSTR)CA2W(OptionName.c_str());

		if (OptionType == "TileBuy")
		{
			OptionItemInfo->OptionType = en_OptionType::OPTION_TYPE_TILE_BUY;
		}

		_OptionItemInfoDatas.insert(pair<int8, st_OptionItemInfo*>((int8)OptionItemInfo->OptionType, OptionItemInfo));
	}
}

st_SkillInfo* CDataManager::FindSkillData(en_SkillMediumCategory FindAttackSkillMediumCategory, en_SkillType FindSkillType)
{
	switch (FindAttackSkillMediumCategory)
	{
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_NONE:
		return nullptr;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ATTACK:
		return (*_PublicAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_HEAL:
		return (*_PublicTacTicSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_BUF:
		return (*_PublicBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_ATTACK:
		return (*_WarriorAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_HEAL:
		return (*_WarriorTacTicSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_WARRIOR_BUF:
		return (*_WarriorBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_ATTACK:
		return (*_ShamanAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_HEAL:
		return (*_ShamanTacTicSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHMAN_BUF:
		return (*_ShamanBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_ATTACK:
		return (*_TaioistAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_HEAL:
		return (*_TaioistTacTicSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_TAOIST_BUF:
		return (*_TaioistBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_ATTACK:
		return (*_ThiefAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_HEAL:
		return (*_ThiefTacTicSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_THIEF_BUF:
		return (*_ThiefBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_ATTACK:
		return (*_ArcherAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_TACTIC:
	case en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ARCHER_HEAL:
		return (*_ArcherTacTicSkillDatas.find((int16)FindSkillType)).second;
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
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
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

st_ItemInfo* CDataManager::FindItemData(en_SmallItemCategory FindItemCategory)
{
	return (*_Items.find((int16)FindItemCategory)).second;
}