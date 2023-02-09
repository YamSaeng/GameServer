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
			else if (ItemEquipmentPart == "EQUIPMENT_PARTS_BOTH_HAND")
			{
				WeaponItemInfo->ItemEquipmentPart = en_EquipmentParts::EQUIPMENT_PARTS_BOTH_HAND;
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

	for (auto& Filed : Document["Tools"].GetArray())
	{
		for (auto& ToolsListFiled : Filed["ToolList"].GetArray())
		{
			string MediumCategory = ToolsListFiled["ItemMediumCategory"].GetString();
			string SmallCategory = ToolsListFiled["ItemSmallCategory"].GetString();
			string ItemObjectType = ToolsListFiled["ItemObjectType"].GetString();
			string ItemExplain = ToolsListFiled["ItemExplain"].GetString();
			string ItemName = ToolsListFiled["ItemName"].GetString();
			string ItemEquipmentPart = ToolsListFiled["ItemEquipmentPart"].GetString();
			int32 ItemWidth = ToolsListFiled["ItemWidth"].GetInt();
			int32 ItemHeight = ToolsListFiled["ItemHeight"].GetInt();
			int32 ItemMinDamage = ToolsListFiled["ItemMinDamage"].GetInt();
			int32 ItemMaxDamage = ToolsListFiled["ItemMaxDamage"].GetInt();
			int32 ItemMaxDurability = ToolsListFiled["ItemMaxDurability"].GetInt();
			int32 ItemMaxCount = ToolsListFiled["ItemMaxCount"].GetInt();
			int64 ItemCraftingTime = ToolsListFiled["ItemCraftingTime"].GetInt64();

			st_ItemInfo* ToolItemInfo = new st_ItemInfo();

			ToolItemInfo->ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_TOOL;
			
			if (MediumCategory == "ITEM_MEDIUM_CATEOGRY_FARMING")
			{
				ToolItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEOGRY_FARMING;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_TOOL_FARMING_SHOVEL")
			{
				ToolItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_TOOL_FARMING_SHOVEL;
			}

			if (ItemObjectType == "OBJECT_ITEM_TOOL_FARMING_SHOVEL")
			{
				ToolItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_TOOL_FARMING_SHOVEL;
			}

			ToolItemInfo->ItemExplain = (LPWSTR)CA2W(ItemExplain.c_str());
			ToolItemInfo->ItemName = (LPWSTR)CA2W(ItemName.c_str());

			if (ItemEquipmentPart == "EQUIPMENT_PARTS_LEFT_HAND")
			{
				ToolItemInfo->ItemEquipmentPart = en_EquipmentParts::EQUIPMENT_PARTS_LEFT_HAND;
			}
			else if (ItemEquipmentPart == "EQUIPMENT_PARTS_RIGHT_HAND")
			{
				ToolItemInfo->ItemEquipmentPart = en_EquipmentParts::EQUIPMENT_PARTS_RIGHT_HAND;
			}
			else if (ItemEquipmentPart == "EQUIPMENT_PARTS_BOTH_HAND")
			{
				ToolItemInfo->ItemEquipmentPart = en_EquipmentParts::EQUIPMENT_PARTS_BOTH_HAND;
			}

			ToolItemInfo->ItemWidth = ItemWidth;
			ToolItemInfo->ItemHeight = ItemHeight;
			ToolItemInfo->ItemMinDamage = ItemMinDamage;
			ToolItemInfo->ItemMaxDamage = ItemMaxDamage;
			ToolItemInfo->ItemMaxDurability = ItemMaxDurability;
			ToolItemInfo->ItemMaxCount = ItemMaxCount;
			ToolItemInfo->ItemCraftingTime = ItemCraftingTime;

			_Items.insert(pair<int16, st_ItemInfo*>((int16)ToolItemInfo->ItemSmallCategory, ToolItemInfo));
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
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CONVERSION_ATTACK;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SHAEHONE_ATTACK;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHOHONE_ATTACK;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_SMASH_WAVE_ATTACK;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_KNIGHT_CHARGE_POSE;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_BUF;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_FLAME_HARPOON;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SPELL_ACTIVE_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHAMAN_HELL_FIRE;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SPELL_ACTIVE_ATTACK;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEOGRY_SKILLBOOK_TAIOIST_HEALING_LIGHT")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEOGRY_SKILLBOOK_TAIOIST_HEALING_LIGHT;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_DISCIPLINE_ACTIVE_HEAL;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_TAIOIST_HEALING_WIND")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_TAIOIST_HEALING_WIND;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_DISCIPLINE_ACTIVE_HEAL;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE")
			{
				SkillBookItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_SHOCK_RELEASE;
				SkillBookItemInfo->ItemSkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
				SkillBookItemInfo->ItemSkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ACTIVE_BUF;
				SkillBookItemInfo->ItemSkillType = en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE;
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

			if (SmallCategory == "ITEM_SMALL_CATEGORY_CRAFTING_DEFAULT_CRAFTING_TABLE")
			{
				ArchitectureItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_DEFAULT_CRAFTING_TABLE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE")
			{
				ArchitectureItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_CRAFTING_TABLE_SAWMILL")
			{
				ArchitectureItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_TABLE_SAWMILL;
			}

			if (ItemObjectType == "OBJECT_ARCHITECTURE_CRAFTING_DEFAULT_CRAFTING_TABLE")
			{
				ArchitectureItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_DEFAULT_CRAFTING_TABLE;
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
			int8 ItemMaxStep = (int8)CropSeedListFiled["ItemMaxStep"].GetInt();
			int32 ItemGrowTime = CropSeedListFiled["ItemGrowTime"].GetInt();

			st_ItemInfo* CropSeedItemInfo = new st_ItemInfo();			
			CropSeedItemInfo->ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_CROP;
			CropSeedItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_CROP_SEED;

			if (SmallCategory == "ITEM_SMALL_CATEGORY_CROP_SEED_POTATO")
			{
				CropSeedItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_SEED_POTATO;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_CROP_SEED_CORN")
			{
				CropSeedItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_SEED_CORN;
			}

			if (ItemObjectType == "OBJECT_ITEM_CROP_SEED_POTATO")
			{
				CropSeedItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CROP_SEED_POTATO;
			}
			else if (ItemObjectType == "OBJECT_ITEM_CROP_SEED_CORN")
			{
				CropSeedItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CROP_SEED_CORN;
			}
			
			CropSeedItemInfo->ItemExplain = (LPWSTR)CA2W(ItemExplain.c_str());
			CropSeedItemInfo->ItemName = (LPWSTR)CA2W(ItemName.c_str());
			CropSeedItemInfo->ItemWidth = ItemWidth;
			CropSeedItemInfo->ItemHeight = ItemHeight;		
			CropSeedItemInfo->ItemMaxDurability = ItemMaxDurability;
			CropSeedItemInfo->ItemMaxCount = ItemMaxCount;		
			CropSeedItemInfo->ItemMaxstep = ItemMaxStep;
			CropSeedItemInfo->ItemGrowTime = ItemGrowTime;

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

			st_ItemInfo* CropFruitItemInfo = new st_ItemInfo();
			CropFruitItemInfo->ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_CROP;
			CropFruitItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_CROP_FRUIT;

			if (SmallCategory == "ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO")
			{
				CropFruitItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_CROP_FRUIT_CORN")
			{
				CropFruitItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_CORN;
			}

			if (ItemObjectType == "OBJECT_ITEM_CROP_FRUIT_POTATO")
			{
				CropFruitItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO;
			}
			else if (ItemObjectType == "OBJECT_ITEM_CROP_FRUIT_CORN")
			{
				CropFruitItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_CORN;
			}
			
			CropFruitItemInfo->ItemExplain = (LPWSTR)CA2W(ItemExplain.c_str());
			CropFruitItemInfo->ItemName = (LPWSTR)CA2W(ItemName.c_str());
			CropFruitItemInfo->ItemWidth = ItemWidth;
			CropFruitItemInfo->ItemHeight = ItemHeight;
			CropFruitItemInfo->ItemMaxDurability = ItemMaxDurability;
			CropFruitItemInfo->ItemMaxCount = ItemMaxCount;						

			_Items.insert(pair<int16, st_ItemInfo*>((int16)CropFruitItemInfo->ItemSmallCategory, CropFruitItemInfo));
		}
	}
}

void CDataManager::LoadDataPlayerCharacterStatus(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["PlayerCharacterStatus"].GetArray())
	{
		string PlayerType = Filed["PlayerType"].GetString();

		for (auto& PlayerCharacterFiled : Filed["PlayerCharacterLevelDataList"].GetArray())
		{
			int Level = PlayerCharacterFiled["Level"].GetInt();
			int MaxHP = PlayerCharacterFiled["MaxHP"].GetInt();
			int MaxMP = PlayerCharacterFiled["MaxMP"].GetInt();
			int MaxDP = PlayerCharacterFiled["MaxDP"].GetInt();
			int AutoRecoveryHPPercent = PlayerCharacterFiled["AutoRecoveryHPPercent"].GetInt();
			int AutoRecoveryMPPercent = PlayerCharacterFiled["AutoRecoveryMPPercent"].GetInt();
			int MinMeleeAttackDamage = PlayerCharacterFiled["MinMeleeAttackDamage"].GetInt();
			int MaxMeleeAttackDamage = PlayerCharacterFiled["MaxMeleeAttackDamage"].GetInt();
			int16 MeleeAttackHitRate = (int16)PlayerCharacterFiled["MeleeAttackHitRate"].GetInt();
			int16 MagicDamage = (int16)PlayerCharacterFiled["MagicDamage"].GetInt();
			float MagicHitRate = (int16)PlayerCharacterFiled["MagicHitRate"].GetFloat();
			int Defence = PlayerCharacterFiled["Defence"].GetInt();
			int16 EvasionRate = PlayerCharacterFiled["EvasionRate"].GetInt();
			int16 MeleeCriticalPoint = (int16)(PlayerCharacterFiled["MeleeCriticalPoint"].GetInt());
			int16 MagicCriticalPoint = (int16)(PlayerCharacterFiled["MagicCriticalPoint"].GetInt());
			float Speed = PlayerCharacterFiled["Speed"].GetFloat();

			st_ObjectStatusData* PlayerStatusData = new st_ObjectStatusData();

			PlayerStatusData->PlayerType = en_GameObjectType::OBJECT_PLAYER;

			PlayerStatusData->Level = Level;
			PlayerStatusData->MaxHP = MaxHP;
			PlayerStatusData->MaxMP = MaxMP;
			PlayerStatusData->MaxDP = MaxDP;
			PlayerStatusData->AutoRecoveryHPPercent = AutoRecoveryHPPercent;
			PlayerStatusData->AutoRecoveryMPPercent = AutoRecoveryMPPercent;
			PlayerStatusData->MinMeleeAttackDamage = MinMeleeAttackDamage;
			PlayerStatusData->MaxMeleeAttackDamage = MaxMeleeAttackDamage;
			PlayerStatusData->MeleeAttackHitRate = MeleeAttackHitRate;
			PlayerStatusData->MagicDamage = MagicDamage;
			PlayerStatusData->MagicHitRate = MagicHitRate;
			PlayerStatusData->Defence = Defence;
			PlayerStatusData->EvasionRate = EvasionRate;
			PlayerStatusData->MeleeCriticalPoint = MeleeCriticalPoint;
			PlayerStatusData->MagicCriticalPoint = MagicCriticalPoint;
			PlayerStatusData->Speed = Speed;

			_PlayerStatus.insert(pair<int32, st_ObjectStatusData*>(PlayerStatusData->Level, PlayerStatusData));
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

		en_GameObjectType MonsterType = en_GameObjectType::OBJECT_NON_TYPE;

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
			float MovingAttackRange = MonsterStatInfoFiled["MovingAttackRange"].GetFloat();
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
			MonsterData->MonsterStatInfo.MovingAttackRange = MovingAttackRange;
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
				PublicAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ACTIVE_ATTACK;

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
				string SkillExplation = PublicAttackSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "SKILL_DEFAULT_ATTACK")
				{
					PublicAttackSkill->SkillType = en_SkillType::SKILL_DEFAULT_ATTACK;
				}

				PublicAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC;
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

				_PublicAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)PublicAttackSkill->SkillType, PublicAttackSkill));
			}					

			for (auto& PublicBufSkillListFiled : PublicSkillListFiled["PublicBufSkillList"].GetArray())
			{
				st_BufSkillInfo* PublicBufSkill = new st_BufSkillInfo();
				PublicBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
				PublicBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ACTIVE_BUF;

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
				string SkillExplation = PublicBufSkillListFiled["SkillExplanation"].GetString();

				if (SkillType == "SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE")
				{
					PublicBufSkill->SkillType = en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE;
				}

				PublicBufSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC;
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

				_PublicBufSkillDatas.insert(pair<int16, st_BufSkillInfo*>((int16)PublicBufSkill->SkillType, PublicBufSkill));
			}
		}
	}
}

void CDataManager::LoadDataFightSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["FightSkills"].GetArray())
	{
		for (auto& FightSkillListFiled : Filed["FightSkillList"].GetArray())
		{
			for (auto& PassiveSkillListFiled : FightSkillListFiled["PassiveSkillList"].GetArray())
			{
				st_PassiveSkillInfo* FightPassiveSkill = new st_PassiveSkillInfo();

				int8 SkillNumber = (int8)PassiveSkillListFiled["SkillNumber"].GetInt();
				string SkillType = PassiveSkillListFiled["SkillType"].GetString();
				string SkillName = PassiveSkillListFiled["SkillName"].GetString();
				int8 SkillMaxLevel = PassiveSkillListFiled["SkillMaxLevel"].GetInt();
				string SkillExplation = PassiveSkillListFiled["SkillExplanation"].GetString();

				FightPassiveSkill->SkillNumber = SkillNumber;
				FightPassiveSkill->SkillMaxLevel = SkillMaxLevel;
				FightPassiveSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				FightPassiveSkill->SkillName = (LPWSTR)CA2W(SkillExplation.c_str());
				FightPassiveSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;

				if (SkillType == "SKILL_FIGHT_TWO_HAND_SWORD_MASTER")
				{
					FightPassiveSkill->SkillType = en_SkillType::SKILL_FIGHT_TWO_HAND_SWORD_MASTER;
				}					

				_FightPassiveSkillDatas.insert(pair<int16, st_PassiveSkillInfo*>((int16)FightPassiveSkill->SkillType, FightPassiveSkill));
			}

			for (auto& ActiveSkillListFiled : FightSkillListFiled["ActiveSkillList"].GetArray())
			{
				for (auto& AttackSkillFiled : ActiveSkillListFiled["AttackSkill"].GetArray())
				{
					st_AttackSkillInfo* FightAttackSkill = new st_AttackSkillInfo();

					int8 SkillNumber = (int8)AttackSkillFiled["SkillNumber"].GetInt();
					string SkillType = AttackSkillFiled["SkillType"].GetString();
					string SkillName = AttackSkillFiled["SkillName"].GetString();
					int8 SkillMaxLevel = AttackSkillFiled["SkillMaxLevel"].GetInt();
					int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
					int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
					int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
					int SkillDistance = AttackSkillFiled["SkillDistance"].GetInt();
					int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
					int8 SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetInt();
					int8 SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetInt();
					int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();
					string NextComboSkill = AttackSkillFiled["NextComboSkill"].GetString();
					string SkillExplation = AttackSkillFiled["SkillExplanation"].GetString();

					if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK;
					}
					else if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK;
					}
					else if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE;
					}
					else if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE;
					}
					else if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE;
					}
					else if (SkillType == "SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH;
					}

					FightAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;
					FightAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
					FightAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
					FightAttackSkill->SkillMaxLevel = SkillMaxLevel;					
					FightAttackSkill->SkillMinDamage = SkillMinDamage;
					FightAttackSkill->SkillMaxDamage = SkillMaxDamage;
					FightAttackSkill->SkillCoolTime = SkillCoolTime;
					FightAttackSkill->SkillCastingTime = SkillCastingTime;
					FightAttackSkill->SkillDurationTime = SkillDurationTime;
					FightAttackSkill->SkillDotTime = SkillDotTime;
					FightAttackSkill->SkillMotionTime = SkillMotionTime;
					FightAttackSkill->SkillDistance = SkillDistance;
					FightAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
					FightAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
					FightAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;					

					if (NextComboSkill == "SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK")
					{
						FightAttackSkill->NextComboSkill = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK;
					}
					else if (NextComboSkill == "SKILL_TYPE_NONE")
					{
						FightAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
					}

					FightAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;

					_FightAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)FightAttackSkill->SkillType, FightAttackSkill));
				}

				for (auto& BufSkillFiled : ActiveSkillListFiled["BufSkill"].GetArray())
				{
					st_BufSkillInfo* FightBufSkill = new st_BufSkillInfo();
					FightBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
					FightBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_BUF;

					int8 SkillNumber = (int8)BufSkillFiled["SkillNumber"].GetInt();
					string SkillType = BufSkillFiled["SkillType"].GetString();
					string SkillName = BufSkillFiled["SkillName"].GetString();
					int SkillMaxLevel = BufSkillFiled["SkillMaxLevel"].GetInt();

					int IncreaseMinAttackPoint = BufSkillFiled["IncreaseMinAttackPoint"].GetInt();
					int IncreaseMaxAttackPoint = BufSkillFiled["IncreaseMaxAttackPoint"].GetInt();
					int IncreaseMeleeAttackSpeedPoint = BufSkillFiled["IncreaseMeleeAttackSpeedPoint"].GetInt();
					int16 IncreaseMeleeAttackHitRate = (int16)BufSkillFiled["IncreaseMeleeAttackHitRate"].GetInt();
					int16 IncreaseMagicAttackPoint = (int16)BufSkillFiled["IncreaseMagicAttackPoint"].GetInt();
					int16 IncreaseMagicCastingPoint = (int16)BufSkillFiled["IncreaseMagicCastingPoint"].GetInt();
					int16 IncreaseMagicAttackHitRate = (int16)BufSkillFiled["IncreaseMagicAttackHitRate"].GetInt();
					int IncreaseDefencePoint = BufSkillFiled["IncreaseDefencePoint"].GetInt();
					int16 IncreaseEvasionRate = (int16)BufSkillFiled["IncreaseEvasionRate"].GetInt();
					int16 IncreaseMeleeCriticalPoint = (int16)BufSkillFiled["IncreaseMeleeCriticalPoint"].GetInt();
					int16 IncreaseMagicCriticalPoint = (int16)BufSkillFiled["IncreaseMagicCriticalPoint"].GetInt();
					float IncreaseSpeedPoint = BufSkillFiled["IncreaseSpeedPoint"].GetFloat();
					int16 IncreaseStatusAbnormalityResistance = (int16)BufSkillFiled["IncreaseStatusAbnormalityResistance"].GetInt();

					int SkillCoolTime = BufSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = BufSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = BufSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = BufSkillFiled["SkillDotTime"].GetInt64();
					int SkillDistance = BufSkillFiled["SkillDistance"].GetInt();
					int32 SkillMotionTime = BufSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = BufSkillFiled["SkillTargetEffectTime"].GetFloat();
					string NextComboSkill = BufSkillFiled["NextComboSkill"].GetString();
					string SkillExplation = BufSkillFiled["SkillExplanation"].GetString();

					if (SkillType == "SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE")
					{
						FightBufSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE;
					}

					FightBufSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;
					FightBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
					FightBufSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
					FightBufSkill->SkillMaxLevel = SkillMaxLevel;
					FightBufSkill->IncreaseMinAttackPoint = IncreaseMinAttackPoint;
					FightBufSkill->IncreaseMaxAttackPoint = IncreaseMaxAttackPoint;
					FightBufSkill->IncreaseMeleeAttackSpeedPoint = IncreaseMeleeAttackSpeedPoint;
					FightBufSkill->IncreaseMeleeAttackHitRate = IncreaseMeleeAttackHitRate;
					FightBufSkill->IncreaseMagicAttackPoint = IncreaseMagicAttackPoint;
					FightBufSkill->IncreaseMagicCastingPoint = IncreaseMagicCastingPoint;
					FightBufSkill->IncreaseMagicAttackHitRate = IncreaseMagicAttackHitRate;
					FightBufSkill->IncreaseDefencePoint = IncreaseDefencePoint;
					FightBufSkill->IncreaseEvasionRate = IncreaseEvasionRate;
					FightBufSkill->IncreaseMeleeCriticalPoint = IncreaseMeleeCriticalPoint;
					FightBufSkill->IncreaseMagicCriticalPoint = IncreaseMagicCriticalPoint;
					FightBufSkill->IncreaseSpeedPoint = IncreaseSpeedPoint;
					FightBufSkill->IncreaseStatusAbnormalityResistance = IncreaseStatusAbnormalityResistance;
					FightBufSkill->SkillCoolTime = SkillCoolTime;
					FightBufSkill->SkillCastingTime = SkillCastingTime;
					FightBufSkill->SkillDurationTime = SkillDurationTime;
					FightBufSkill->SkillDotTime = SkillDotTime;
					FightBufSkill->SkillDistance = SkillDistance;
					FightBufSkill->SkillMotionTime = SkillMotionTime;
					FightBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;

					_FightBufSkillDatas.insert(pair<int16, st_BufSkillInfo*>((int16)FightBufSkill->SkillType, FightBufSkill));
				}
			}
		}
	}
}

void CDataManager::LoadDataProtectionSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["ProtectionSkills"].GetArray())
	{
		for (auto& ProtectionSkillListFiled : Filed["ProtectionSkillList"].GetArray())
		{
			for (auto& PassiveSkillListFiled : ProtectionSkillListFiled["PassiveSkillList"].GetArray())
			{

			}

			for (auto& ActiveSkillListFiled : ProtectionSkillListFiled["ActiveSkillList"].GetArray())
			{
				for (auto& AttackSkillFiled : ActiveSkillListFiled["AttackSkill"].GetArray())
				{
					st_AttackSkillInfo* ProtectionAttackSkill = new st_AttackSkillInfo();

					int8 SkillNumber = (int8)AttackSkillFiled["SkillNumber"].GetInt();
					string SkillType = AttackSkillFiled["SkillType"].GetString();
					string SkillName = AttackSkillFiled["SkillName"].GetString();
					int8 SkillMaxLevel = AttackSkillFiled["SkillMaxLevel"].GetInt();
					int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
					int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
					int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
					int SkillDistance = AttackSkillFiled["SkillDistance"].GetInt();
					int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
					int8 SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetInt();
					int8 SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetInt();
					int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();
					string NextComboSkill = AttackSkillFiled["NextComboSkill"].GetString();
					string SkillExplation = AttackSkillFiled["SkillExplanation"].GetString();

					if (SkillType == "SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH")
					{
						ProtectionAttackSkill->SkillType = en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH;
					}					

					ProtectionAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_PROTECTION;
					ProtectionAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
					ProtectionAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
					ProtectionAttackSkill->SkillMaxLevel = SkillMaxLevel;
					ProtectionAttackSkill->SkillMinDamage = SkillMinDamage;
					ProtectionAttackSkill->SkillMaxDamage = SkillMaxDamage;
					ProtectionAttackSkill->SkillCoolTime = SkillCoolTime;
					ProtectionAttackSkill->SkillCastingTime = SkillCastingTime;
					ProtectionAttackSkill->SkillDurationTime = SkillDurationTime;
					ProtectionAttackSkill->SkillDotTime = SkillDotTime;
					ProtectionAttackSkill->SkillMotionTime = SkillMotionTime;
					ProtectionAttackSkill->SkillDistance = SkillDistance;
					ProtectionAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
					ProtectionAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
					ProtectionAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;

					if (NextComboSkill == "SKILL_TYPE_NONE")
					{
						ProtectionAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
					}

					ProtectionAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;

					_ProtectionAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)ProtectionAttackSkill->SkillType, ProtectionAttackSkill));
				}

				for (auto& BufSkillFiled : ActiveSkillListFiled["BufSkill"].GetArray())
				{

				}
			}
		}
	}
}

void CDataManager::LoadDataAssassinationSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["AssassinationSkills"].GetArray())
	{
		for (auto& AssassinationListFiled : Filed["AssassinationSkillList"].GetArray())
		{
			for (auto& PassiveSkillListFiled : AssassinationListFiled["PassiveSkillList"].GetArray())
			{

			}

			for (auto& ActiveSkillListFiled : AssassinationListFiled["ActiveSkillList"].GetArray())
			{
				for (auto& AttackSkillFiled : ActiveSkillListFiled["AttackSkill"].GetArray())
				{
					st_AttackSkillInfo* AssassinationAttackSkill = new st_AttackSkillInfo();
					AssassinationAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_THIEF;
					AssassinationAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ASSASSINATION_ACTIVE_ATTACK;

					string SkillType = AttackSkillFiled["SkillType"].GetString();
					string SkillName = AttackSkillFiled["SkillName"].GetString();
					int SkillLevel = AttackSkillFiled["SkillLevel"].GetInt();
					int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
					int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
					int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
					int SkillDistance = AttackSkillFiled["SkillDistance"].GetInt();
					int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
					int8 SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetInt();
					int8 SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetInt();
					int64 SkillDamageOverTime = AttackSkillFiled["SkillDebufDamageOverTime"].GetInt64();
					int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();
					string NextComboSkill = AttackSkillFiled["NextComboSkill"].GetString();
					string SkillExplation = AttackSkillFiled["SkillExplanation"].GetString();

					if (SkillType == "SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT")
					{
						AssassinationAttackSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT;
					}
					else if (SkillType == "SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT")
					{
						AssassinationAttackSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT;
					}
					else if (SkillType == "SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK")
					{
						AssassinationAttackSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK;
					}
					else if (SkillType == "SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP")
					{
						AssassinationAttackSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP;
					}

					AssassinationAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_ASSASSINATION;
					AssassinationAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
					AssassinationAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
					AssassinationAttackSkill->SkillLevel = SkillLevel;
					AssassinationAttackSkill->SkillMinDamage = SkillMinDamage;
					AssassinationAttackSkill->SkillMaxDamage = SkillMaxDamage;
					AssassinationAttackSkill->SkillCoolTime = SkillCoolTime;
					AssassinationAttackSkill->SkillCastingTime = SkillCastingTime;
					AssassinationAttackSkill->SkillDurationTime = SkillDurationTime;
					AssassinationAttackSkill->SkillDotTime = SkillDotTime;
					AssassinationAttackSkill->SkillDistance = SkillDistance;
					AssassinationAttackSkill->SkillMotionTime = SkillMotionTime;
					AssassinationAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
					AssassinationAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
					AssassinationAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
					AssassinationAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;

					if (NextComboSkill == "SKILL_TYPE_NONE")
					{
						AssassinationAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
					}
					else if (NextComboSkill == "SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT")
					{
						AssassinationAttackSkill->NextComboSkill = en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT;
					}

					_AssassinationAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)AssassinationAttackSkill->SkillType, AssassinationAttackSkill));
				}

				for (auto& BufSkillFiled : ActiveSkillListFiled["BufSkill"].GetArray())
				{
					st_BufSkillInfo* AssassinationBufSkill = new st_BufSkillInfo();
					AssassinationBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
					AssassinationBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_BUF;

					int8 SkillNumber = (int8)BufSkillFiled["SkillNumber"].GetInt();
					string SkillType = BufSkillFiled["SkillType"].GetString();
					string SkillName = BufSkillFiled["SkillName"].GetString();
					int SkillMaxLevel = BufSkillFiled["SkillMaxLevel"].GetInt();

					int IncreaseMinAttackPoint = BufSkillFiled["IncreaseMinAttackPoint"].GetInt();
					int IncreaseMaxAttackPoint = BufSkillFiled["IncreaseMaxAttackPoint"].GetInt();
					int IncreaseMeleeAttackSpeedPoint = BufSkillFiled["IncreaseMeleeAttackSpeedPoint"].GetInt();
					int16 IncreaseMeleeAttackHitRate = (int16)BufSkillFiled["IncreaseMeleeAttackHitRate"].GetInt();
					int16 IncreaseMagicAttackPoint = (int16)BufSkillFiled["IncreaseMagicAttackPoint"].GetInt();
					int16 IncreaseMagicCastingPoint = (int16)BufSkillFiled["IncreaseMagicCastingPoint"].GetInt();
					int16 IncreaseMagicAttackHitRate = (int16)BufSkillFiled["IncreaseMagicAttackHitRate"].GetInt();
					int IncreaseDefencePoint = BufSkillFiled["IncreaseDefencePoint"].GetInt();
					int16 IncreaseEvasionRate = (int16)BufSkillFiled["IncreaseEvasionRate"].GetInt();
					int16 IncreaseMeleeCriticalPoint = (int16)BufSkillFiled["IncreaseMeleeCriticalPoint"].GetInt();
					int16 IncreaseMagicCriticalPoint = (int16)BufSkillFiled["IncreaseMagicCriticalPoint"].GetInt();
					float IncreaseSpeedPoint = BufSkillFiled["IncreaseSpeedPoint"].GetFloat();
					int16 IncreaseStatusAbnormalityResistance = (int16)BufSkillFiled["IncreaseStatusAbnormalityResistance"].GetInt();

					int SkillCoolTime = BufSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = BufSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = BufSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = BufSkillFiled["SkillDotTime"].GetInt64();
					int SkillDistance = BufSkillFiled["SkillDistance"].GetInt();
					int32 SkillMotionTime = BufSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = BufSkillFiled["SkillTargetEffectTime"].GetFloat();					
					string NextComboSkill = BufSkillFiled["NextComboSkill"].GetString();
					string SkillExplation = BufSkillFiled["SkillExplanation"].GetString();

					if (SkillType == "SKILL_ASSASSINATION_ACTIVE_BUF_WEAPON_POISON")
					{
						AssassinationBufSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_WEAPON_POISON;
					}

					AssassinationBufSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;
					AssassinationBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
					AssassinationBufSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
					AssassinationBufSkill->SkillMaxLevel = SkillMaxLevel;
					AssassinationBufSkill->IncreaseMinAttackPoint = IncreaseMinAttackPoint;
					AssassinationBufSkill->IncreaseMaxAttackPoint = IncreaseMaxAttackPoint;
					AssassinationBufSkill->IncreaseMeleeAttackSpeedPoint = IncreaseMeleeAttackSpeedPoint;
					AssassinationBufSkill->IncreaseMeleeAttackHitRate = IncreaseMeleeAttackHitRate;
					AssassinationBufSkill->IncreaseMagicAttackPoint = IncreaseMagicAttackPoint;
					AssassinationBufSkill->IncreaseMagicCastingPoint = IncreaseMagicCastingPoint;
					AssassinationBufSkill->IncreaseMagicAttackHitRate = IncreaseMagicAttackHitRate;
					AssassinationBufSkill->IncreaseDefencePoint = IncreaseDefencePoint;
					AssassinationBufSkill->IncreaseEvasionRate = IncreaseEvasionRate;
					AssassinationBufSkill->IncreaseMeleeCriticalPoint = IncreaseMeleeCriticalPoint;
					AssassinationBufSkill->IncreaseMagicCriticalPoint = IncreaseMagicCriticalPoint;
					AssassinationBufSkill->IncreaseSpeedPoint = IncreaseSpeedPoint;
					AssassinationBufSkill->IncreaseStatusAbnormalityResistance = IncreaseStatusAbnormalityResistance;
					AssassinationBufSkill->SkillCoolTime = SkillCoolTime;
					AssassinationBufSkill->SkillCastingTime = SkillCastingTime;
					AssassinationBufSkill->SkillDurationTime = SkillDurationTime;
					AssassinationBufSkill->SkillDotTime = SkillDotTime;
					AssassinationBufSkill->SkillDistance = SkillDistance;
					AssassinationBufSkill->SkillMotionTime = SkillMotionTime;
					AssassinationBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;					

					_AssassinationBufSkillDatas.insert(pair<int16, st_BufSkillInfo*>((int16)AssassinationBufSkill->SkillType, AssassinationBufSkill));
				}
			}
		}
	}
}

void CDataManager::LoadDataSpellSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["SpellSkills"].GetArray())
	{
		for (auto& SpellSkillListFiled : Filed["SpellSkillList"].GetArray())
		{
			for (auto& PassiveSkillListFiled : SpellSkillListFiled["PassiveSkillList"].GetArray())
			{

			}

			for (auto& ActiveSkillListFiled : SpellSkillListFiled["ActiveSkillList"].GetArray())
			{
				for (auto& AttackSkillFiled : ActiveSkillListFiled["AttackSkill"].GetArray())
				{
					st_AttackSkillInfo* SpellAttackSkill = new st_AttackSkillInfo();
					SpellAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
					SpellAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SPELL_ACTIVE_ATTACK;

					string SkillType = AttackSkillFiled["SkillType"].GetString();
					string SkillName = AttackSkillFiled["SkillName"].GetString();
					int SkillLevel = AttackSkillFiled["SkillLevel"].GetInt();
					int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
					int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
					int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
					int SkillDistance = AttackSkillFiled["SkillDistance"].GetInt();
					int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
					int8 SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetInt();
					int8 SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetInt();
					int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();					
					string NextComboSkill = AttackSkillFiled["NextComboSkill"].GetString();
					string SkillExplation = AttackSkillFiled["SkillExplanation"].GetString();

					if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON;
					}
					else if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_ROOT")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT;
					}
					else if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN;
					}
					else if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE;
					}
					else if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE;
					}
					else if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE;
					}

					SpellAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_SPELL;
					SpellAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
					SpellAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
					SpellAttackSkill->SkillLevel = SkillLevel;
					SpellAttackSkill->SkillMinDamage = SkillMinDamage;
					SpellAttackSkill->SkillMaxDamage = SkillMaxDamage;
					SpellAttackSkill->SkillCoolTime = SkillCoolTime;
					SpellAttackSkill->SkillCastingTime = SkillCastingTime;
					SpellAttackSkill->SkillDurationTime = SkillDurationTime;
					SpellAttackSkill->SkillDotTime = SkillDotTime;
					SpellAttackSkill->SkillDistance = SkillDistance;
					SpellAttackSkill->SkillMotionTime = SkillMotionTime;
					SpellAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
					SpellAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
					SpellAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
					SpellAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;					

					if (NextComboSkill == "SKILL_TYPE_NONE")
					{
						SpellAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
					}
					else if (NextComboSkill == "SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE")
					{
						SpellAttackSkill->NextComboSkill = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE;
					}

					_SpellAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)SpellAttackSkill->SkillType, SpellAttackSkill));
				}

				for (auto& BufSkillFiled : ActiveSkillListFiled["BufSkill"].GetArray())
				{
					st_BufSkillInfo* SpellBufSkill = new st_BufSkillInfo();
					SpellBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
					SpellBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SPELL_ACTIVE_BUF;

					string SkillType = BufSkillFiled["SkillType"].GetString();
					string SkillName = BufSkillFiled["SkillName"].GetString();
					int SkillLevel = BufSkillFiled["SkillLevel"].GetInt();
					int SkillCoolTime = BufSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = BufSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = BufSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = BufSkillFiled["SkillDotTime"].GetInt64();
					int SkillDistance = BufSkillFiled["SkillDistance"].GetInt();
					int32 SkillMotionTime = BufSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = BufSkillFiled["SkillTargetEffectTime"].GetFloat();
					string NextComboSkill = BufSkillFiled["NextComboSkill"].GetString();
					string SkillExplation = BufSkillFiled["SkillExplanation"].GetString();

					if (SkillType == "SKILL_SPELL_ACTIVE_BUF_TELEPORT")
					{
						SpellBufSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_BUF_TELEPORT;
					}

					SpellBufSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_SPELL;
					SpellBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
					SpellBufSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
					SpellBufSkill->SkillLevel = SkillLevel;
					SpellBufSkill->SkillCoolTime = SkillCoolTime;
					SpellBufSkill->SkillCastingTime = SkillCastingTime;
					SpellBufSkill->SkillDurationTime = SkillDurationTime;
					SpellBufSkill->SkillDotTime = SkillDotTime;
					SpellBufSkill->SkillDistance = SkillDistance;
					SpellBufSkill->SkillMotionTime = SkillMotionTime;
					SpellBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;					

					if (NextComboSkill == "SKILL_TYPE_NONE")
					{
						SpellBufSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
					}

					_SpellBufSkillDatas.insert(pair<int16, st_BufSkillInfo*>((int16)SpellBufSkill->SkillType, SpellBufSkill));
				}
			}
		}
	}
}

void CDataManager::LoadDataShootingSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["ShootingSkills"].GetArray())
	{
		for (auto& ShootingSkillListFiled : Filed["ShootingSkillList"].GetArray())
		{
			for (auto& PassiveSkillListFiled : ShootingSkillListFiled["PassiveSkillList"].GetArray())
			{

			}

			for (auto& ActiveSkillListFiled : ShootingSkillListFiled["ActiveSkillList"].GetArray())
			{
				for (auto& AttackSkillFiled : ActiveSkillListFiled["AttackSkill"].GetArray())
				{
					st_AttackSkillInfo* ShootingAttackSkill = new st_AttackSkillInfo();
					ShootingAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_ARCHER;
					ShootingAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHOOTING_ACTIVE_ATTACK;

					string SkillType = AttackSkillFiled["SkillType"].GetString();
					string SkillName = AttackSkillFiled["SkillName"].GetString();
					int SkillLevel = AttackSkillFiled["SkillLevel"].GetInt();
					int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
					int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
					int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
					int SkillDistance = AttackSkillFiled["SkillDistance"].GetInt();
					int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
					int64 SkillDebufTime = AttackSkillFiled["SkillDebufTime"].GetInt64();
					int8 SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetInt();
					int8 SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetInt();
					int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();					
					string NextComboSkill = AttackSkillFiled["NextComboSkill"].GetString();
					string SkillExplation = AttackSkillFiled["SkillExplanation"].GetString();

					if (SkillType == "SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING")
					{
						ShootingAttackSkill->SkillType = en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING;
					}

					ShootingAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_SHOOTING;
					ShootingAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
					ShootingAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
					ShootingAttackSkill->SkillLevel = SkillLevel;
					ShootingAttackSkill->SkillMinDamage = SkillMinDamage;
					ShootingAttackSkill->SkillMaxDamage = SkillMaxDamage;
					ShootingAttackSkill->SkillCoolTime = SkillCoolTime;
					ShootingAttackSkill->SkillCastingTime = SkillCastingTime;
					ShootingAttackSkill->SkillDurationTime = SkillDurationTime;
					ShootingAttackSkill->SkillDotTime = SkillDotTime;
					ShootingAttackSkill->SkillDistance = SkillDistance;
					ShootingAttackSkill->SkillMotionTime = SkillMotionTime;
					ShootingAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
					ShootingAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
					ShootingAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
					ShootingAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;					

					if (NextComboSkill == "SKILL_TYPE_NONE")
					{
						ShootingAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
					}

					_ShootingAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)ShootingAttackSkill->SkillType, ShootingAttackSkill));
				}

				for (auto& BufSkillFiled : ActiveSkillListFiled["BufSkill"].GetArray())
				{

				}
			}
		}
	}
}

void CDataManager::LoadDataDisCiplineSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["DisciplineSkills"].GetArray())
	{
		for (auto& DisciplineSkillListFiled : Filed["DisciplineSkillList"].GetArray())
		{
			for (auto& PassiveSkillListFiled : DisciplineSkillListFiled["PassiveSkillList"].GetArray())
			{

			}

			for (auto& ActiveSkillListFiled : DisciplineSkillListFiled["ActiveSkillList"].GetArray())
			{
				for (auto& AttackSkillFiled : ActiveSkillListFiled["AttackSkill"].GetArray())
				{
					st_AttackSkillInfo* DisciplineAttackSkill = new st_AttackSkillInfo();
					DisciplineAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
					DisciplineAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_DISCIPLINE_ACTIVE_ATTACK;

					string SkillType = AttackSkillFiled["SkillType"].GetString();
					string SkillName = AttackSkillFiled["SkillName"].GetString();
					int SkillLevel = AttackSkillFiled["SkillLevel"].GetInt();
					int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
					int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
					int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
					int SkillDistance = AttackSkillFiled["SkillDistance"].GetInt();
					int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
					int64 SkillDebufTime = AttackSkillFiled["SkillDebufTime"].GetInt64();
					int8 SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetInt();
					int8 SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetInt();
					int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();
					string NextComboSkill = AttackSkillFiled["NextComboSkill"].GetString();
					string SkillExplation = AttackSkillFiled["SkillExplanation"].GetString();

					if (SkillType == "SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE")
					{
						DisciplineAttackSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE;
					}
					else if (SkillType == "SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT")
					{
						DisciplineAttackSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT;
					}

					DisciplineAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_DISCIPLINE;
					DisciplineAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
					DisciplineAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
					DisciplineAttackSkill->SkillLevel = SkillLevel;
					DisciplineAttackSkill->SkillMinDamage = SkillMinDamage;
					DisciplineAttackSkill->SkillMaxDamage = SkillMaxDamage;
					DisciplineAttackSkill->SkillCoolTime = SkillCoolTime;
					DisciplineAttackSkill->SkillCastingTime = SkillCastingTime;
					DisciplineAttackSkill->SkillDurationTime = SkillDurationTime;
					DisciplineAttackSkill->SkillDotTime = SkillDotTime;
					DisciplineAttackSkill->SkillDistance = SkillDistance;
					DisciplineAttackSkill->SkillMotionTime = SkillMotionTime;
					DisciplineAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
					DisciplineAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
					DisciplineAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
					DisciplineAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;

					if (NextComboSkill == "SKILL_TYPE_NONE")
					{
						DisciplineAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
					}

					_DisciplineAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)DisciplineAttackSkill->SkillType, DisciplineAttackSkill));
				}

				for (auto& HealSkillFiled : ActiveSkillListFiled["HealSkill"].GetArray())
				{
					st_HealSkillInfo* DisciplineHealSkill = new st_HealSkillInfo();
					DisciplineHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
					DisciplineHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_DISCIPLINE_ACTIVE_HEAL;

					string SkillType = HealSkillFiled["SkillType"].GetString();
					string SkillName = HealSkillFiled["SkillName"].GetString();
					int SkillLevel = HealSkillFiled["SkillLevel"].GetInt();
					int SkillMinHeal = HealSkillFiled["SkillMinHeal"].GetInt();
					int SkillMaxHeal = HealSkillFiled["SkillMaxHeal"].GetInt();
					int SkillCoolTime = HealSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = HealSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = HealSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = HealSkillFiled["SkillDotTime"].GetInt64();
					int SkillDistance = HealSkillFiled["SkillDistance"].GetInt();
					int32 SkillMotionTime = HealSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = HealSkillFiled["SkillTargetEffectTime"].GetFloat();
					string NextComboSkill = HealSkillFiled["NextComboSkill"].GetString();
					string SkillExplation = HealSkillFiled["SkillExplanation"].GetString();

					if (SkillType == "SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT")
					{
						DisciplineHealSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT;
					}
					else if (SkillType == "SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND")
					{
						DisciplineHealSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND;
					}

					DisciplineHealSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_DISCIPLINE;
					DisciplineHealSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
					DisciplineHealSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
					DisciplineHealSkill->SkillLevel = SkillLevel;
					DisciplineHealSkill->SkillMinHealPoint = SkillMinHeal;
					DisciplineHealSkill->SkillMaxHealPoint = SkillMaxHeal;
					DisciplineHealSkill->SkillCoolTime = SkillCoolTime;
					DisciplineHealSkill->SkillCastingTime = SkillCastingTime;
					DisciplineHealSkill->SkillDurationTime = SkillDurationTime;
					DisciplineHealSkill->SkillDotTime = SkillDotTime;
					DisciplineHealSkill->SkillDistance = SkillDistance;
					DisciplineHealSkill->SkillMotionTime = SkillMotionTime;
					DisciplineHealSkill->SkillTargetEffectTime = SkillTargetEffectTime;

					if (NextComboSkill == "SKILL_TYPE_NONE")
					{
						DisciplineHealSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
					}

					_DisciplineHealSkillDatas.insert(pair<int16, st_HealSkillInfo*>((int16)DisciplineHealSkill->SkillType, DisciplineHealSkill));
				}

				for (auto& BufSkillFiled : ActiveSkillListFiled["BufSkill"].GetArray())
				{

				}
			}
		}
	}
}

void CDataManager::LoadDataMonsterSkill(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["MonsterSkills"].GetArray())
	{
		for (auto& SlimeMonsterSkills : Filed["SlimeSkillList"].GetArray())
		{
			for (auto& SlimePassiveSkillFiled : SlimeMonsterSkills["PassiveSkillList"].GetArray())
			{

			}

			for (auto& SlimeActiveSkillFiled : SlimeMonsterSkills["ActiveSkillList"].GetArray())
			{
				st_AttackSkillInfo* SlimeAttackSkill = new st_AttackSkillInfo();
				SlimeAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_MONSTER_MELEE;
				SlimeAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_NONE;

				string SkillType = SlimeActiveSkillFiled["SkillType"].GetString();
				string SkillName = SlimeActiveSkillFiled["SkillName"].GetString();
				int SkillLevel = SlimeActiveSkillFiled["SkillLevel"].GetInt();
				int SkillMinDamage = SlimeActiveSkillFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = SlimeActiveSkillFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = SlimeActiveSkillFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = SlimeActiveSkillFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = SlimeActiveSkillFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = SlimeActiveSkillFiled["SkillDotTime"].GetInt64();
				int SkillDistance = SlimeActiveSkillFiled["SkillDistance"].GetInt();
				int32 SkillMotionTime = SlimeActiveSkillFiled["SkillMotionTime"].GetInt();
				float SkillTargetEffectTime = SlimeActiveSkillFiled["SkillTargetEffectTime"].GetFloat();
				int8 SkillDebufAttackSpeed = (int8)SlimeActiveSkillFiled["SkillDebufAttackSpeed"].GetInt();
				int8 SkillDebufMovingSpeed = (int8)SlimeActiveSkillFiled["SkillDebufMovingSpeed"].GetInt();
				int8 StatusAbnormalityProbability = (int8)SlimeActiveSkillFiled["StatusAbnormalityProbability"].GetInt();
				string NextComboSkill = SlimeActiveSkillFiled["NextComboSkill"].GetString();
				string SkillExplation = SlimeActiveSkillFiled["SkillExplanation"].GetString();

				if (SkillType == "SKILL_SLIME_ACTIVE_POISION_ATTACK")
				{
					SlimeAttackSkill->SkillType = en_SkillType::SKILL_SLIME_ACTIVE_POISION_ATTACK;
				}

				SlimeAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_NONE;
				SlimeAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				SlimeAttackSkill->SkillExplanation = (LPWSTR)CA2W(SkillExplation.c_str());
				SlimeAttackSkill->SkillLevel = SkillLevel;
				SlimeAttackSkill->SkillMinDamage = SkillMinDamage;
				SlimeAttackSkill->SkillMaxDamage = SkillMaxDamage;
				SlimeAttackSkill->SkillCoolTime = SkillCoolTime;
				SlimeAttackSkill->SkillCastingTime = SkillCastingTime;
				SlimeAttackSkill->SkillDurationTime = SkillDurationTime;
				SlimeAttackSkill->SkillDotTime = SkillDotTime;
				SlimeAttackSkill->SkillDistance = SkillDistance;
				SlimeAttackSkill->SkillMotionTime = SkillMotionTime;
				SlimeAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				SlimeAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				SlimeAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
				SlimeAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;				

				if (NextComboSkill == "SKILL_TYPE_NONE")
				{
					SlimeAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
				}

				_SlimeAttackSkillDatas.insert(pair<int16, st_AttackSkillInfo*>((int16)SlimeAttackSkill->SkillType, SlimeAttackSkill));
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

		en_GameObjectType EnvironmentType = en_GameObjectType::OBJECT_NON_TYPE;

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
			int64 RecoveryTime = (int64)EnvironmentStatInfoFiled["RecoveryTime"].GetInt64();

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

		en_GameObjectType CropType = en_GameObjectType::OBJECT_NON_TYPE;

		if (CropName == "감자")
		{
			CropType = en_GameObjectType::OBJECT_CROP_POTATO;
		}
		else if (CropName == "옥수수")
		{
			CropType = en_GameObjectType::OBJECT_CROP_CORN;
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
			else if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_CROP_FRUIT_CORN")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_CORN;
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

			if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_CRAFTING_DEFAULT_CRAFTING_TABLE")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_DEFAULT_CRAFTING_TABLE);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARCHITECTURE;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_CRAFTING_TABLE;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CRAFTING_DEFAULT_CRAFTING_TABLE;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_CRAFTING_TABLE_FURANCE")
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

void CDataManager::LoadDataMerchant(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["MerchantItems"].GetArray())
	{
		for (auto& GeneralMerchantItemsFiled : Filed["GeneralMerchantItemList"].GetArray())
		{
			string ItemName = GeneralMerchantItemsFiled["ItemName"].GetString();
			string ItemSmallCategoryString = GeneralMerchantItemsFiled["ItemSmallCategory"].GetString();

			int16 ItemSmallcategory;

			if (ItemSmallCategoryString == "ITEM_SMALL_CATEGORY_POTION_HEALTH_RESTORATION_POTION_SMALL")
			{
				ItemSmallcategory = (int16)en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEALTH_RESTORATION_POTION_SMALL;
			}
			else if (ItemSmallCategoryString == "ITEM_SMALL_CATEGORY_POTION_MANA_RESTORATION_POTION_SMALL")
			{
				ItemSmallcategory = (int16)en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_MANA_RESTORATION_POTION_SMALL;
			}			

			_GeneralMerchantItems.insert(pair<int16, st_ItemInfo*>(ItemSmallcategory, FindItemData((en_SmallItemCategory)ItemSmallcategory)));
		}		
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

st_SkillInfo* CDataManager::FindSkillData(en_SkillType FindSkillType)
{
	switch (FindSkillType)
	{
	case en_SkillType::SKILL_TYPE_NONE:
		CRASH("None 스킬 데이터 찾기 요청");
		break;
	case en_SkillType::SKILL_DEFAULT_ATTACK:
		return (*_PublicAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE:
		return (*_PublicBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_FIGHT_TWO_HAND_SWORD_MASTER:
		return (*_FightPassiveSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:		
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:		
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE:		
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE:		
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE:
		return (*_FightAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:		
		return (*_FightBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
		return (*_ProtectionAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON:		
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:		
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:		
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:		
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:		
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:			
		return (*_SpellAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_SPELL_ACTIVE_BUF_TELEPORT:
		return (*_SpellBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:		
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:		
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT:		
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:
		return (*_DisciplineAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP:
		return (*_AssassinationAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_WEAPON_POISON:
		return (*_AssassinationBufSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING:
		return (*_ShootingAttackSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_SLIME_ACTIVE_POISION_ATTACK:
		return (*_SlimeAttackSkillDatas.find((int16)FindSkillType)).second;
	}	
}

st_ObjectStatusData* CDataManager::FindObjectStatusData(en_GameObjectType GameObjectType, int16 Level)
{
	switch (GameObjectType)
	{
	case en_GameObjectType::OBJECT_NON_TYPE:
		return nullptr;
	case en_GameObjectType::OBJECT_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		return (*_PlayerStatus.find(Level)).second;	
	case en_GameObjectType::OBJECT_SLIME:
		break;
	case en_GameObjectType::OBJECT_BEAR:
		break;
	case en_GameObjectType::OBJECT_STONE:
		break;
	case en_GameObjectType::OBJECT_TREE:
		break;		
	}
}

st_ItemInfo* CDataManager::FindItemData(en_SmallItemCategory FindItemCategory)
{
	return (*_Items.find((int16)FindItemCategory)).second;
}

int32 CDataManager::FindMonsterExperienceData(en_GameObjectType MonsterGameObjectType)
{
	switch (MonsterGameObjectType)
	{		
	case en_GameObjectType::OBJECT_SLIME:		
	case en_GameObjectType::OBJECT_BEAR:		
		return (*_Monsters.find(MonsterGameObjectType)).second->GetExpPoint;
	}
}
