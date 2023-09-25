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
			int ItemSearchingTime = WeaponListFiled["ItemSearchingTime"].GetInt();
			string ItemEquipmentPart = WeaponListFiled["ItemEquipmentPart"].GetString();
			int32 ItemWidth = WeaponListFiled["ItemWidth"].GetInt();	
			int32 ItemHeight = WeaponListFiled["ItemHeight"].GetInt();
			float ItemCollisionX = WeaponListFiled["ItemCollisionX"].GetFloat();
			float ItemCollisionY = WeaponListFiled["ItemCollisionY"].GetFloat();
			int32 ItemMinDamage = WeaponListFiled["ItemMinDamage"].GetInt();
			int32 ItemMaxDamage = WeaponListFiled["ItemMaxDamage"].GetInt();
			int32 ItemMaxDurability = WeaponListFiled["ItemMaxDurability"].GetInt();
			int32 ItemMaxCount = WeaponListFiled["ItemMaxCount"].GetInt();
			int64 ItemCraftingTime = WeaponListFiled["ItemCraftingTime"].GetInt64();

			st_ItemInfo* WeaponItemInfo = new st_ItemInfo();

			WeaponItemInfo->ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_WEAPON_DAGGER")
			{
				WeaponItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAPON_DAGGER;
			}
			else if (MediumCategory == "ITEM_MEDIUM_CATEGORY_WEAPON_LONG_SWORD")
			{
				WeaponItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAPON_LONG_SWORD;
			}
			else if (MediumCategory == "ITEM_MEDIUM_CATEGORY_WEAPON_GREAT_SWORD")
			{
				WeaponItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAPON_GREAT_SWORD;
			}
			else if (MediumCategory == "ITEM_MEDIUM_CATEGORY_WEAPON_SHIELD")
			{
				WeaponItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAPON_SHIELD;
			}
			else if (MediumCategory == "ITEM_MEDIUM_CATEGORY_WEAPON_BOW")
			{
				WeaponItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAPON_BOW;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_WEAPON_DAGGER_WOOD")
			{
				WeaponItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_DAGGER_WOOD;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_WEAPON_LONG_SWORD_WOOD")
			{
				WeaponItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_LONG_SWORD_WOOD;
			}
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_WEAPON_GREAT_SWORD_WOOD")
			{
				WeaponItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_GREAT_SWORD_WOOD;
			}	
			else if (SmallCategory == "ITEM_SAMLL_CATEGORY_WEAPON_SHIELD_WOOD")
			{
				WeaponItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SAMLL_CATEGORY_WEAPON_SHIELD_WOOD;
			}
			else if (SmallCategory == "ITEM_SAMLL_CATEGORY_WEAPON_BOW_WOOD")
			{
				WeaponItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SAMLL_CATEGORY_WEAPON_BOW_WOOD;
			}

			if (ItemObjectType == "OBJECT_ITEM_WEAPON_WOOD_DAGGER")
			{
				WeaponItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_DAGGER;
			}
			else if (ItemObjectType == "OBJECT_ITEM_WEAPON_WOOD_LONG_SWORD")
			{
				WeaponItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_LONG_SWORD;
			}
			else if (ItemObjectType == "OBJECT_ITEM_WEAPON_WOOD_GREAT_SWORD")
			{
				WeaponItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_GREAT_SWORD;
			}
			else if (ItemObjectType == "OBJECT_ITEM_WEAPON_WOOD_SHIELD")
			{
				WeaponItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SHIELD;
			}
			else if (ItemObjectType == "OBJECT_ITEM_WEAPON_WOOD_BOW")
			{
				WeaponItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_BOW;
			}			
						
			WeaponItemInfo->ItemExplain = (LPWSTR)CA2W(ItemExplain.c_str());
			WeaponItemInfo->ItemName = (LPWSTR)CA2W(ItemName.c_str());
			WeaponItemInfo->ItemSearchingTime = ItemSearchingTime;

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
			WeaponItemInfo->ItemCollisionX = ItemCollisionX;
			WeaponItemInfo->ItemCollisionY = ItemCollisionY;
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

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_ARMOR_HAT")
			{
				ArmorItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_ARMOR_HAT;
			}
			else if (MediumCategory == "ITEM_MEDIUM_CATEGORY_ARMOR_WEAR")
			{
				ArmorItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_ARMOR_WEAR;
			}
			else if (MediumCategory == "ITEM_MEDIUM_CATEGORY_ARMOR_GLOVE")
			{
				ArmorItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_ARMOR_GLOVE;
			}
			else if (MediumCategory == "ITEM_MEDIUM_CATEGORY_ARMOR_BOOT")
			{
				ArmorItemInfo->ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_ARMOR_BOOT;
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
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_COIN")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COIN;
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
			else if (SmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_FABRIC")
			{
				MaterialItemInfo->ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_FABRIC;
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
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_COIN")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_COIN;
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
			else if (ItemObjectType == "OBJECT_ITEM_MATERIAL_FABRIC")
			{
				MaterialItemInfo->ItemObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL_FABRIC;
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
		for (auto& WarriorIncreaseStatusFiled : Filed["WarriorTypeLevelUpIncreaseStatus"].GetArray())
		{
			int8 Level = (int8)WarriorIncreaseStatusFiled["Level"].GetInt();

			int16 Str = (int16)WarriorIncreaseStatusFiled["Str"].GetInt();
			int16 Dex = (int16)WarriorIncreaseStatusFiled["Dex"].GetInt();
			int16 Int = (int16)WarriorIncreaseStatusFiled["Int"].GetInt();
			int16 Luck = (int16)WarriorIncreaseStatusFiled["Luck"].GetInt();

			int16 Con = (int16)WarriorIncreaseStatusFiled["Con"].GetInt();
			int16 Wis = (int16)WarriorIncreaseStatusFiled["Wis"].GetInt();

			int16 Def = (int16)WarriorIncreaseStatusFiled["Def"].GetInt();

			int16 HP = (int16)WarriorIncreaseStatusFiled["HP"].GetInt();
			int16 MP = (int16)WarriorIncreaseStatusFiled["MP"].GetInt();

			int AutoRecoveryHPPercent = WarriorIncreaseStatusFiled["AutoRecoveryHPPercent"].GetInt();
			int AutoRecoveryMPPercent = WarriorIncreaseStatusFiled["AutoRecoveryMPPercent"].GetInt();
			int16 Stamina = (int16)WarriorIncreaseStatusFiled["Stamina"].GetInt();

			float Speed = WarriorIncreaseStatusFiled["Speed"].GetFloat();

			st_StatInfo* WarriorLevelStatus = new st_StatInfo();

			WarriorLevelStatus->Level = Level;
			WarriorLevelStatus->Str = Str;
			WarriorLevelStatus->Dex = Dex;
			WarriorLevelStatus->Int = Int;
			WarriorLevelStatus->Luck = Luck;
			WarriorLevelStatus->Con = Con;
			WarriorLevelStatus->Wis = Wis;
			WarriorLevelStatus->Defence = Def;
			WarriorLevelStatus->HP = HP;
			WarriorLevelStatus->MP = MP;
			WarriorLevelStatus->Stamina = Stamina;
			WarriorLevelStatus->AutoRecoveryHPPercent = AutoRecoveryHPPercent;
			WarriorLevelStatus->AutoRecoveryMPPercent = AutoRecoveryMPPercent;
			WarriorLevelStatus->Speed = Speed;

			_WarriorStatus.insert(pair<int8, st_StatInfo*>(WarriorLevelStatus->Level, WarriorLevelStatus));
		}

		for (auto& ThiefIncreaseStatusFiled : Filed["ThiefTypeLevelUpIncreaseStatus"].GetArray())
		{
			int8 Level = (int8)ThiefIncreaseStatusFiled["Level"].GetInt();

			int16 Str = (int16)ThiefIncreaseStatusFiled["Str"].GetInt();
			int16 Dex = (int16)ThiefIncreaseStatusFiled["Dex"].GetInt();
			int16 Int = (int16)ThiefIncreaseStatusFiled["Int"].GetInt();
			int16 Luck = (int16)ThiefIncreaseStatusFiled["Luck"].GetInt();

			int16 Con = (int16)ThiefIncreaseStatusFiled["Con"].GetInt();		
			int16 Wis = (int16)ThiefIncreaseStatusFiled["Wis"].GetInt();

			int16 Def = (int16)ThiefIncreaseStatusFiled["Def"].GetInt();

			int16 HP = (int16)ThiefIncreaseStatusFiled["HP"].GetInt();		
			int16 MP = (int16)ThiefIncreaseStatusFiled["MP"].GetInt();

			int AutoRecoveryHPPercent = ThiefIncreaseStatusFiled["AutoRecoveryHPPercent"].GetInt();
			int AutoRecoveryMPPercent = ThiefIncreaseStatusFiled["AutoRecoveryMPPercent"].GetInt();	
			int16 Stamina = (int16)ThiefIncreaseStatusFiled["Stamina"].GetInt();
			
			float Speed = ThiefIncreaseStatusFiled["Speed"].GetFloat();

			st_StatInfo* ThiefLevelStatus = new st_StatInfo();

			ThiefLevelStatus->Level = Level;	
			ThiefLevelStatus->Str = Str;
			ThiefLevelStatus->Dex = Dex;
			ThiefLevelStatus->Int = Int;
			ThiefLevelStatus->Luck = Luck;
			ThiefLevelStatus->Con = Con;
			ThiefLevelStatus->Wis = Wis;
			ThiefLevelStatus->Defence = Def;
			ThiefLevelStatus->HP = HP;
			ThiefLevelStatus->MP = MP;	
			ThiefLevelStatus->Stamina = Stamina;
			ThiefLevelStatus->AutoRecoveryHPPercent = AutoRecoveryHPPercent;
			ThiefLevelStatus->AutoRecoveryMPPercent = AutoRecoveryMPPercent;												
			ThiefLevelStatus->Speed = Speed;					

			_ThiefStatus.insert(pair<int8, st_StatInfo*>(ThiefLevelStatus->Level, ThiefLevelStatus));
		}

		for (auto& MageIncreaseStatusFiled : Filed["MageTypeLevelUpIncreaseStatus"].GetArray())
		{
			int8 Level = (int8)MageIncreaseStatusFiled["Level"].GetInt();

			int16 Str = (int16)MageIncreaseStatusFiled["Str"].GetInt();
			int16 Dex = (int16)MageIncreaseStatusFiled["Dex"].GetInt();
			int16 Int = (int16)MageIncreaseStatusFiled["Int"].GetInt();
			int16 Luck = (int16)MageIncreaseStatusFiled["Luck"].GetInt();

			int16 Con = (int16)MageIncreaseStatusFiled["Con"].GetInt();
			int16 Wis = (int16)MageIncreaseStatusFiled["Wis"].GetInt();

			int16 Def = (int16)MageIncreaseStatusFiled["Def"].GetInt();

			int16 HP = (int16)MageIncreaseStatusFiled["HP"].GetInt();
			int16 MP = (int16)MageIncreaseStatusFiled["MP"].GetInt();

			int AutoRecoveryHPPercent = MageIncreaseStatusFiled["AutoRecoveryHPPercent"].GetInt();
			int AutoRecoveryMPPercent = MageIncreaseStatusFiled["AutoRecoveryMPPercent"].GetInt();
			int16 Stamina = (int16)MageIncreaseStatusFiled["Stamina"].GetInt();

			float Speed = MageIncreaseStatusFiled["Speed"].GetFloat();

			st_StatInfo* MageLevelStatus = new st_StatInfo();

			MageLevelStatus->Level = Level;
			MageLevelStatus->Str = Str;
			MageLevelStatus->Dex = Dex;
			MageLevelStatus->Int = Int;
			MageLevelStatus->Luck = Luck;
			MageLevelStatus->Con = Con;
			MageLevelStatus->Wis = Wis;
			MageLevelStatus->Defence = Def;
			MageLevelStatus->HP = HP;
			MageLevelStatus->MP = MP;
			MageLevelStatus->Stamina = Stamina;
			MageLevelStatus->AutoRecoveryHPPercent = AutoRecoveryHPPercent;
			MageLevelStatus->AutoRecoveryMPPercent = AutoRecoveryMPPercent;
			MageLevelStatus->Speed = Speed;

			_MageStatus.insert(pair<int8, st_StatInfo*>(MageLevelStatus->Level, MageLevelStatus));
		}

		for (auto& HunterIncreaseStatusFiled : Filed["HunterTypeLevelUpIncreaseStatus"].GetArray())
		{
			int8 Level = (int8)HunterIncreaseStatusFiled["Level"].GetInt();

			int16 Str = (int16)HunterIncreaseStatusFiled["Str"].GetInt();
			int16 Dex = (int16)HunterIncreaseStatusFiled["Dex"].GetInt();
			int16 Int = (int16)HunterIncreaseStatusFiled["Int"].GetInt();
			int16 Luck = (int16)HunterIncreaseStatusFiled["Luck"].GetInt();

			int16 Con = (int16)HunterIncreaseStatusFiled["Con"].GetInt();
			int16 Wis = (int16)HunterIncreaseStatusFiled["Wis"].GetInt();

			int16 Def = (int16)HunterIncreaseStatusFiled["Def"].GetInt();

			int16 HP = (int16)HunterIncreaseStatusFiled["HP"].GetInt();
			int16 MP = (int16)HunterIncreaseStatusFiled["MP"].GetInt();

			int AutoRecoveryHPPercent = HunterIncreaseStatusFiled["AutoRecoveryHPPercent"].GetInt();
			int AutoRecoveryMPPercent = HunterIncreaseStatusFiled["AutoRecoveryMPPercent"].GetInt();
			int16 Stamina = (int16)HunterIncreaseStatusFiled["Stamina"].GetInt();

			float Speed = HunterIncreaseStatusFiled["Speed"].GetFloat();

			st_StatInfo* HunterLevelStatus = new st_StatInfo();

			HunterLevelStatus->Level = Level;
			HunterLevelStatus->Str = Str;
			HunterLevelStatus->Dex = Dex;
			HunterLevelStatus->Int = Int;
			HunterLevelStatus->Luck = Luck;
			HunterLevelStatus->Con = Con;
			HunterLevelStatus->Wis = Wis;
			HunterLevelStatus->Defence = Def;
			HunterLevelStatus->HP = HP;
			HunterLevelStatus->MP = MP;
			HunterLevelStatus->Stamina = Stamina;
			HunterLevelStatus->AutoRecoveryHPPercent = AutoRecoveryHPPercent;
			HunterLevelStatus->AutoRecoveryMPPercent = AutoRecoveryMPPercent;
			HunterLevelStatus->Speed = Speed;

			_MageStatus.insert(pair<int8, st_StatInfo*>(HunterLevelStatus->Level, HunterLevelStatus));
		}

		for (auto& MainCharacterIncreaseStatusFiled : Filed["MainCharacterLevelUpIncreaseStatus"].GetArray())
		{
			int8 Level = (int8)MainCharacterIncreaseStatusFiled["Level"].GetInt();

			int16 Str = (int16)MainCharacterIncreaseStatusFiled["Str"].GetInt();
			int16 Dex = (int16)MainCharacterIncreaseStatusFiled["Dex"].GetInt();
			int16 Int = (int16)MainCharacterIncreaseStatusFiled["Int"].GetInt();
			int16 Luck = (int16)MainCharacterIncreaseStatusFiled["Luck"].GetInt();

			int16 Con = (int16)MainCharacterIncreaseStatusFiled["Con"].GetInt();
			int16 Wis = (int16)MainCharacterIncreaseStatusFiled["Wis"].GetInt();

			int16 Def = (int16)MainCharacterIncreaseStatusFiled["Def"].GetInt();

			int16 HP = (int16)MainCharacterIncreaseStatusFiled["HP"].GetInt();
			int16 MP = (int16)MainCharacterIncreaseStatusFiled["MP"].GetInt();

			int AutoRecoveryHPPercent = MainCharacterIncreaseStatusFiled["AutoRecoveryHPPercent"].GetInt();
			int AutoRecoveryMPPercent = MainCharacterIncreaseStatusFiled["AutoRecoveryMPPercent"].GetInt();
			int16 Stamina = (int16)MainCharacterIncreaseStatusFiled["Stamina"].GetInt();

			float Speed = MainCharacterIncreaseStatusFiled["Speed"].GetFloat();

			st_StatInfo* MainCharacterLevelStatus = new st_StatInfo();

			MainCharacterLevelStatus->Level = Level;
			MainCharacterLevelStatus->Str = Str;
			MainCharacterLevelStatus->Dex = Dex;
			MainCharacterLevelStatus->Int = Int;
			MainCharacterLevelStatus->Luck = Luck;
			MainCharacterLevelStatus->Con = Con;
			MainCharacterLevelStatus->Wis = Wis;
			MainCharacterLevelStatus->Defence = Def;
			MainCharacterLevelStatus->HP = HP;
			MainCharacterLevelStatus->MP = MP;
			MainCharacterLevelStatus->Stamina = Stamina;
			MainCharacterLevelStatus->AutoRecoveryHPPercent = AutoRecoveryHPPercent;
			MainCharacterLevelStatus->AutoRecoveryMPPercent = AutoRecoveryMPPercent;
			MainCharacterLevelStatus->Speed = Speed;

			_MainCharacterStatus.insert(pair<int8, st_StatInfo*>(MainCharacterLevelStatus->Level, MainCharacterLevelStatus));
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

		if (MonsterName == "고블린")
		{
			MonsterType = en_GameObjectType::OBJECT_GOBLIN;
		}		

		MonsterData->MonsterName = MonsterName;

		for (auto& MonsterStatInfoFiled : Filed["MonsterStatInfo"].GetArray())
		{
			int Level = MonsterStatInfoFiled["Level"].GetInt();
			int Str = MonsterStatInfoFiled["Str"].GetInt();
			int Dex = MonsterStatInfoFiled["Dex"].GetInt();
			int Int = MonsterStatInfoFiled["Int"].GetInt();
			int Luck = MonsterStatInfoFiled["Luck"].GetInt();
			int MaxHP = MonsterStatInfoFiled["MaxHP"].GetInt();
			int MaxMP = MonsterStatInfoFiled["MaxMP"].GetInt();
			int MinAttackPoint = MonsterStatInfoFiled["MinAttackPoint"].GetInt();
			int MaxAttackPoint = MonsterStatInfoFiled["MaxAttackPoint"].GetInt();
			int16 SpiritPoint = (int16)MonsterStatInfoFiled["SpiritPoint"].GetInt();
			int16 AttackHitRate = MonsterStatInfoFiled["AttackHitRate"].GetInt();			
			float SpellCastingRate = (int16)MonsterStatInfoFiled["SpellCastingRate"].GetFloat();
			int Defence = MonsterStatInfoFiled["Defence"].GetInt();
			int16 EvasionRate = (int16)MonsterStatInfoFiled["EvasionRate"].GetInt();
			int16 AttackCriticalPoint = (int16)MonsterStatInfoFiled["AttackCriticalPoint"].GetInt();
			int16 SpellCriticalPoint = (int16)MonsterStatInfoFiled["SpellCriticalPoint"].GetInt();
			float Speed = MonsterStatInfoFiled["Speed"].GetFloat();
			float SearchDistance = MonsterStatInfoFiled["SearchDistance"].GetFloat();
			float ChaseDistance = MonsterStatInfoFiled["ChaseDistance"].GetFloat();
			int SearchTick = MonsterStatInfoFiled["SearchTick"].GetInt();
			int PatrolTick = MonsterStatInfoFiled["PatrolTick"].GetInt();
			int AttackTick = MonsterStatInfoFiled["AttackTick"].GetInt();
			float MovingAttackRange = MonsterStatInfoFiled["MovingAttackRange"].GetFloat();
			float AttackRange = MonsterStatInfoFiled["AttackRange"].GetFloat();			
			int GetExpPoint = MonsterStatInfoFiled["GetExpPoint"].GetInt();
			int64 ReSpawnTime = MonsterStatInfoFiled["ReSpawnTime"].GetInt64();

			MonsterData->MonsterStatInfo.Level = Level;
			MonsterData->MonsterStatInfo.Str = Str;
			MonsterData->MonsterStatInfo.Dex = Dex;
			MonsterData->MonsterStatInfo.Int = Int;
			MonsterData->MonsterStatInfo.Luck = Luck;
			MonsterData->MonsterStatInfo.MaxHP = MaxHP;
			MonsterData->MonsterStatInfo.MaxMP = MaxMP;
			MonsterData->MonsterStatInfo.MinAttackPoint = MinAttackPoint;
			MonsterData->MonsterStatInfo.MaxAttackPoint = MaxAttackPoint;
			MonsterData->MonsterStatInfo.SpiritPoint = SpiritPoint;
			MonsterData->MonsterStatInfo.AttackHitRate = AttackHitRate;
			MonsterData->MonsterStatInfo.SpellCastingRate = SpellCastingRate;
			MonsterData->MonsterStatInfo.Defence = Defence;
			MonsterData->MonsterStatInfo.EvasionRate = EvasionRate;
			MonsterData->MonsterStatInfo.AttackCriticalPoint = AttackCriticalPoint;
			MonsterData->MonsterStatInfo.SpellCriticalPoint = SpellCriticalPoint;
			MonsterData->MonsterStatInfo.Speed = Speed;
			MonsterData->MonsterStatInfo.SearchDistance = SearchDistance;
			MonsterData->MonsterStatInfo.ChaseDistance = ChaseDistance;
			MonsterData->SearchTick = SearchTick;
			MonsterData->PatrolTick = PatrolTick;
			MonsterData->AttackTick = AttackTick;
			MonsterData->MonsterStatInfo.MovingAttackRange = MovingAttackRange;
			MonsterData->MonsterStatInfo.AttackRange = AttackRange;			
			MonsterData->GetExpPoint = GetExpPoint;
			MonsterData->ReSpawnTime = ReSpawnTime;
		}

		for (auto& EquipmentFiled : Filed["MonsterEquipmentData"].GetArray())
		{
			string Head = EquipmentFiled["Head"].GetString();
			string Body = EquipmentFiled["Body"].GetString();
			string LeftHand = EquipmentFiled["LeftHand"].GetString();
			string RightHand = EquipmentFiled["RightHand"].GetString();
			string Boot = EquipmentFiled["Boot"].GetString();
			
			en_SmallItemCategory HeadEquipmentItemType;
			en_SmallItemCategory BodyEquipmentItemType;
			en_SmallItemCategory LeftHandEquipmentItemType;
			en_SmallItemCategory RightHandEquipmentItemType;
			en_SmallItemCategory BootEquipmentItemType;

			if (Head == "ITEM_SMALL_CATEGORY_NONE")
			{
				HeadEquipmentItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;
			}

			MonsterData->EquipmentItems.push_back(HeadEquipmentItemType);

			if (Body == "ITEM_SMALL_CATEGORY_NONE")
			{
				BodyEquipmentItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;
			}

			MonsterData->EquipmentItems.push_back(BodyEquipmentItemType);

			if (LeftHand == "ITEM_SMALL_CATEGORY_NONE")
			{
				LeftHandEquipmentItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;
			}

			MonsterData->EquipmentItems.push_back(LeftHandEquipmentItemType);

			if (RightHand == "ITEM_SMALL_CATEGORY_NONE")
			{
				RightHandEquipmentItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;
			}
			else if (RightHand == "ITEM_SMALL_CATEGORY_WEAPON_LONG_SWORD_WOOD")
			{
				RightHandEquipmentItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_LONG_SWORD_WOOD;
			}

			MonsterData->EquipmentItems.push_back(RightHandEquipmentItemType);

			if (Boot == "ITEM_SMALL_CATEGORY_NONE")
			{
				BootEquipmentItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;
			}			

			MonsterData->EquipmentItems.push_back(BootEquipmentItemType);
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
				st_SkillInfo* PublicSkill = new st_SkillInfo();
				PublicSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
				PublicSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ACTIVE_ATTACK;

				string SkillType = PublicAttackSkillListFiled["SkillType"].GetString();
				string SkillKind = PublicAttackSkillListFiled["SkillKind"].GetString();
				string SkillName = PublicAttackSkillListFiled["SkillName"].GetString();
				bool SkillIsDamage = PublicAttackSkillListFiled["SkillIsDamage"].GetBool();
				int SkillMinDamage = PublicAttackSkillListFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = PublicAttackSkillListFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = PublicAttackSkillListFiled["SkillCoolTime"].GetInt();
				float SkillDistance = PublicAttackSkillListFiled["SkillDistance"].GetFloat();
				float SkillTargetEffectTime = PublicAttackSkillListFiled["SkillTargetEffectTime"].GetFloat();				

				if (SkillKind == "SKILL_KIND_NONE")
				{
					PublicSkill->SkillKind = en_SkillKinds::SKILL_KIND_NONE;
				}
				else if (SkillKind == "SKILL_KIND_MELEE_SKILL")
				{
					PublicSkill->SkillKind = en_SkillKinds::SKILL_KIND_MELEE_SKILL;
				}

				if (SkillType == "SKILL_DEFAULT_ATTACK")
				{
					PublicSkill->SkillType = en_SkillType::SKILL_DEFAULT_ATTACK;
				}
				else if (SkillType == "SKILL_GLOBAL_SKILL")
				{
					PublicSkill->SkillType = en_SkillType::SKILL_GLOBAL_SKILL;
				}

				PublicSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC;
				PublicSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());	
				PublicSkill->SkillIsDamage = SkillIsDamage;
				PublicSkill->SkillMinDamage = SkillMinDamage;
				PublicSkill->SkillMaxDamage = SkillMaxDamage;
				PublicSkill->SkillCoolTime = SkillCoolTime;
				PublicSkill->SkillDistance = SkillDistance;
				PublicSkill->SkillTargetEffectTime = SkillTargetEffectTime;

				_PublicSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)PublicSkill->SkillType, PublicSkill));
			}					

			for (auto& PublicBufSkillListFiled : PublicSkillListFiled["PublicBufSkillList"].GetArray())
			{
				st_SkillInfo* PublicBufSkill = new st_SkillInfo();
				PublicBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_PUBLIC;
				PublicBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_PUBLIC_ACTIVE_BUF;

				string SkillType = PublicBufSkillListFiled["SkillType"].GetString();
				string SkillKind = PublicBufSkillListFiled["SkillKind"].GetString();
				string SkillName = PublicBufSkillListFiled["SkillName"].GetString();
				int16 IncreaseStatusAbnormalityResistance = (int16)PublicBufSkillListFiled["IncreaseStatusAbnormalityResistance"].GetInt();

				int SkillCoolTime = PublicBufSkillListFiled["SkillCoolTime"].GetInt();
				int64 SkillDurationTime = PublicBufSkillListFiled["SkillDurationTime"].GetInt64();
				float SkillTargetEffectTime = PublicBufSkillListFiled["SkillTargetEffectTime"].GetFloat();				

				if (SkillKind == "SKILL_KIND_SPELL_SKILL")
				{
					PublicBufSkill->SkillKind = en_SkillKinds::SKILL_KIND_SPELL_SKILL;
				}

				if (SkillType == "SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE")
				{
					PublicBufSkill->SkillType = en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE;
				}

				PublicBufSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC;
				PublicBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());				
				PublicBufSkill->IncreaseStatusAbnormalityResistance = IncreaseStatusAbnormalityResistance;
				PublicBufSkill->SkillCoolTime = SkillCoolTime;
				PublicBufSkill->SkillDurationTime = SkillDurationTime;
				PublicBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;

				_PublicSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)PublicBufSkill->SkillType, PublicBufSkill));
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

			}

			for (auto& ActiveSkillListFiled : FightSkillListFiled["ActiveSkillList"].GetArray())
			{
				for (auto& AttackSkillFiled : ActiveSkillListFiled["AttackSkill"].GetArray())
				{
					st_SkillInfo* FightAttackSkill = new st_SkillInfo();

					int8 SkillNumber = (int8)AttackSkillFiled["SkillNumber"].GetInt();					
					string SkillKind = AttackSkillFiled["SkillKind"].GetString();
					string SkillType = AttackSkillFiled["SkillType"].GetString();
					string SkillName = AttackSkillFiled["SkillName"].GetString();
					string SkillStatusAbnormal = AttackSkillFiled["SkillStatusAbnormal"].GetString();
					string SkillStatusAbnormalMask = AttackSkillFiled["SkillStatusAbnormalMask"].GetString();
					int8 SkillMaxLevel = (int8)AttackSkillFiled["SkillMaxLevel"].GetInt();
					bool SkillIsDamage = AttackSkillFiled["SkillIsDamage"].GetBool();
					int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
					int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
					int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
					float SkillDistance = AttackSkillFiled["SkillDistance"].GetFloat();
					float SkillRangeX = AttackSkillFiled["SkillRangeX"].GetFloat();
					float SkillRangeY = AttackSkillFiled["SkillRangeY"].GetFloat();
					int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
					float SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetFloat();
					float SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetFloat();
					int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();
					string NextComboSkill = AttackSkillFiled["NextComboSkill"].GetString();					
					string RollBackSkill = AttackSkillFiled["RollBackSkill"].GetString();

					if (SkillStatusAbnormal == "STATUS_ABNORMAL_NONE")
					{
						FightAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_FIGHT_WRATH_ATTACK")
					{
						FightAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_WRATH_ATTACK;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_FIGHT_JUMPING_ATTACK")
					{
						FightAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_JUMPING_ATTACK;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_FIGHT_FIERCING_WAVE")
					{
						FightAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_FIERCING_WAVE;
					}

					if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_NONE")
					{
						FightAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_FIGHT_WRATH_ATTACK_MASK")
					{
						FightAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_WRATH_ATTACK_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_FIGHT_JUMPING_ATTACK_MASK")
					{
						FightAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_JUMPING_ATTACK_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_FIGHT_FIERCING_WAVE_MASK")
					{
						FightAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_FIERCING_WAVE_MASK;
					}

					if (SkillKind == "SKILL_KIND_MELEE_SKILL")
					{
						FightAttackSkill->SkillKind = en_SkillKinds::SKILL_KIND_MELEE_SKILL;
					}
				
					if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK;
					}
					else if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK;
					}
					else if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_WRATH_ATTACK")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_WRATH_ATTACK;
					}
					else if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE;
					}
					else if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_FLY_KNIFE")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FLY_KNIFE;
					}
					else if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_COMBO_FLY_KNIFE")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_COMBO_FLY_KNIFE;
					}
					else if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_JUMPING_ATTACK")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_JUMPING_ATTACK;
					}	
					else if (SkillType == "SKILL_FIGHT_ACTIVE_ATTACK_PIERCING_WAVE")
					{
						FightAttackSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_PIERCING_WAVE;
					}					

					FightAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;
					FightAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());					
					FightAttackSkill->SkillMaxLevel = SkillMaxLevel;	
					FightAttackSkill->SkillIsDamage = SkillIsDamage;
					FightAttackSkill->SkillMinDamage = SkillMinDamage;
					FightAttackSkill->SkillMaxDamage = SkillMaxDamage;
					FightAttackSkill->SkillCoolTime = SkillCoolTime;
					FightAttackSkill->SkillCastingTime = SkillCastingTime;
					FightAttackSkill->SkillDurationTime = SkillDurationTime;
					FightAttackSkill->SkillDotTime = SkillDotTime;
					FightAttackSkill->SkillMotionTime = SkillMotionTime;
					FightAttackSkill->SkillDistance = SkillDistance;
					FightAttackSkill->SkillRangeX = SkillRangeX;
					FightAttackSkill->SkillRangeY = SkillRangeY;
					FightAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
					FightAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
					FightAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;					

					if (NextComboSkill == "SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK")
					{
						FightAttackSkill->NextComboSkill = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK;
					}
					else if (NextComboSkill == "SKILL_FIGHT_ACTIVE_ATTACK_WRATH_ATTACK")
					{
						FightAttackSkill->NextComboSkill = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_WRATH_ATTACK;
					}
					else if (NextComboSkill == "SKILL_FIGHT_ACTIVE_ATTACK_COMBO_FLY_KNIFE")
					{
						FightAttackSkill->NextComboSkill = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_COMBO_FLY_KNIFE;
					}
					else if (NextComboSkill == "SKILL_TYPE_NONE")
					{
						FightAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
					}					

					if (RollBackSkill == "SKILL_TYPE_NONE")
					{
						FightAttackSkill->RollBackSkill = en_SkillType::SKILL_TYPE_NONE;
					}
					else if (RollBackSkill == "SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK")
					{
						FightAttackSkill->RollBackSkill = en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK;
					}

					FightAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;

					_FightSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)FightAttackSkill->SkillType, FightAttackSkill));
				}

				for (auto& BufSkillFiled : ActiveSkillListFiled["BufSkill"].GetArray())
				{
					st_SkillInfo* FightBufSkill = new st_SkillInfo();
					FightBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
					FightBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_BUF;

					int8 SkillNumber = (int8)BufSkillFiled["SkillNumber"].GetInt();
					string SkillType = BufSkillFiled["SkillType"].GetString();
					string SkillKind = BufSkillFiled["SkillKind"].GetString();
					string SkillName = BufSkillFiled["SkillName"].GetString();
					int8 SkillMaxLevel = (int8)BufSkillFiled["SkillMaxLevel"].GetInt();

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
					float SkillDistance = BufSkillFiled["SkillDistance"].GetFloat();
					int32 SkillMotionTime = BufSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = BufSkillFiled["SkillTargetEffectTime"].GetFloat();					

					if (SkillKind == "SKILL_KIND_SPELL_SKILL")
					{
						FightBufSkill->SkillKind = en_SkillKinds::SKILL_KIND_SPELL_SKILL;
					}

					if (SkillType == "SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE")
					{
						FightBufSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE;
					}
					else if (SkillType == "SKILL_FIGHT_ACTIVE_BUF_COUNTER_ARMOR")
					{
						FightBufSkill->SkillType = en_SkillType::SKILL_FIGHT_ACTIVE_BUF_COUNTER_ARMOR;
					}

					FightBufSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;
					FightBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());					
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

					_FightSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)FightBufSkill->SkillType, FightBufSkill));
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
					st_SkillInfo* ProtectionAttackSkill = new st_SkillInfo();

					int8 SkillNumber = (int8)AttackSkillFiled["SkillNumber"].GetInt();
					string SkillType = AttackSkillFiled["SkillType"].GetString();
					string SkillKind = AttackSkillFiled["SkillKind"].GetString();
					string SkillName = AttackSkillFiled["SkillName"].GetString();
					string SkillStatusAbnormal = AttackSkillFiled["SkillStatusAbnormal"].GetString();
					string SkillStatusAbnormalMask = AttackSkillFiled["SkillStatusAbnormalMask"].GetString();
					int8 SkillMaxLevel = (int8)AttackSkillFiled["SkillMaxLevel"].GetInt();
					bool SkillIsDamage = AttackSkillFiled["SkillIsDamage"].GetBool();
					int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
					int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
					int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
					float SkillDistance = AttackSkillFiled["SkillDistance"].GetFloat();
					float SkillRangeX = AttackSkillFiled["SkillRangeX"].GetFloat();
					float SkillRangeY = AttackSkillFiled["SkillRangeY"].GetFloat();
					int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
					float SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetFloat();
					float SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetFloat();
					int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();
					string NextComboSkill = AttackSkillFiled["NextComboSkill"].GetString();					

					if (SkillStatusAbnormal == "STATUS_ABNORMAL_NONE")
					{
						ProtectionAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_PROTECTION_LAST_ATTACK")
					{
						ProtectionAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_LAST_ATTACK;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_PROTECTION_SHIELD_SMASH")
					{
						ProtectionAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_SHIELD_SMASH;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_PROTECTION_SHIELD_COUNTER")
					{
						ProtectionAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_SHIELD_COUNTER;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_PROTECTION_SWORD_STORM")
					{
						ProtectionAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_SWORD_STORM;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_PROTECTION_CAPTURE")
					{
						ProtectionAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_CAPTURE;
					}

					if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_NONE")
					{
						ProtectionAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_PROTECTION_LAST_MASK")
					{
						ProtectionAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_LAST_MASK;
					}					
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_PROTECTION_SHIELD_SMASH_MASK")
					{
						ProtectionAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_SHIELD_SMASH_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_PROTECTION_SHIELD_COUNTER_MASK")
					{
						ProtectionAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_SHIELD_COUNTER_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_PROTECTION_SWORD_STORM_MASK")
					{
						ProtectionAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_SWORD_STORM_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_PROTECTION_CAPTURE_MASK")
					{
						ProtectionAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_CAPTURE_MASK;
					}

					if (SkillKind == "SKILL_KIND_MELEE_SKILL")
					{
						ProtectionAttackSkill->SkillKind = en_SkillKinds::SKILL_KIND_MELEE_SKILL;
					}					

					if (SkillType == "SKILL_PROTECTION_ACTIVE_ATTACK_POWERFUL_ATTACK")
					{
						ProtectionAttackSkill->SkillType = en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_POWERFUL_ATTACK;
					}
					else if (SkillType == "SKILL_PROTECTION_ACTIVE_ATTACK_SHARP_ATTACK")
					{
						ProtectionAttackSkill->SkillType = en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHARP_ATTACK;
					}
					else if (SkillType == "SKILL_PROTECTION_ACTIVE_ATTACK_LAST_ATTACK")
					{
						ProtectionAttackSkill->SkillType = en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_LAST_ATTACK;
					}
					else if (SkillType == "SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH")
					{
						ProtectionAttackSkill->SkillType = en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH;
					}
					else if (SkillType == "SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_COUNTER")
					{
						ProtectionAttackSkill->SkillType = en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_COUNTER;
					}
					else if (SkillType == "SKILL_PROTECTION_ACTIVE_ATTACK_SWORD_STORM")
					{
						ProtectionAttackSkill->SkillType = en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SWORD_STORM;
					}
					else if (SkillType == "SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE")
					{
						ProtectionAttackSkill->SkillType = en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE;
					}

					ProtectionAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_PROTECTION;
					ProtectionAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());					
					ProtectionAttackSkill->SkillMaxLevel = SkillMaxLevel;
					ProtectionAttackSkill->SkillIsDamage = SkillIsDamage;
					ProtectionAttackSkill->SkillMinDamage = SkillMinDamage;
					ProtectionAttackSkill->SkillMaxDamage = SkillMaxDamage;
					ProtectionAttackSkill->SkillCoolTime = SkillCoolTime;
					ProtectionAttackSkill->SkillCastingTime = SkillCastingTime;
					ProtectionAttackSkill->SkillDurationTime = SkillDurationTime;
					ProtectionAttackSkill->SkillDotTime = SkillDotTime;
					ProtectionAttackSkill->SkillMotionTime = SkillMotionTime;
					ProtectionAttackSkill->SkillDistance = SkillDistance;
					ProtectionAttackSkill->SkillRangeX = SkillRangeX;
					ProtectionAttackSkill->SkillRangeY = SkillRangeY;
					ProtectionAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
					ProtectionAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
					ProtectionAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;

					if (NextComboSkill == "SKILL_PROTECTION_ACTIVE_ATTACK_SHARP_ATTACK")
					{
						ProtectionAttackSkill->NextComboSkill = en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHARP_ATTACK;
					}
					else if (NextComboSkill == "SKILL_PROTECTION_ACTIVE_ATTACK_CRASH_ATTACK")
					{
						ProtectionAttackSkill->NextComboSkill = en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_LAST_ATTACK;
					}
					else if (NextComboSkill == "SKILL_TYPE_NONE")
					{
						ProtectionAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
					}

					ProtectionAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;

					_ProtectionSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)ProtectionAttackSkill->SkillType, ProtectionAttackSkill));
				}

				for (auto& BufSkillFiled : ActiveSkillListFiled["BufSkill"].GetArray())
				{
					st_SkillInfo* ProtectionBufSkill = new st_SkillInfo();
					ProtectionBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
					ProtectionBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_BUF;

					int8 SkillNumber = (int8)BufSkillFiled["SkillNumber"].GetInt();
					string SkillType = BufSkillFiled["SkillType"].GetString();
					string SkillKind = BufSkillFiled["SkillKind"].GetString();
					string SkillName = BufSkillFiled["SkillName"].GetString();
					int8 SkillMaxLevel = (int8)BufSkillFiled["SkillMaxLevel"].GetInt();

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
					float SkillDistance = BufSkillFiled["SkillDistance"].GetFloat();
					int32 SkillMotionTime = BufSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = BufSkillFiled["SkillTargetEffectTime"].GetFloat();					

					if (SkillKind == "SKILL_KIND_SPELL_SKILL")
					{
						ProtectionBufSkill->SkillKind = en_SkillKinds::SKILL_KIND_SPELL_SKILL;
					}

					if (SkillType == "SKILL_PROTECTION_ACTIVE_BUF_FURY")
					{
						ProtectionBufSkill->SkillType = en_SkillType::SKILL_PROTECTION_ACTIVE_BUF_FURY;
					}
					else if (SkillType == "SKILL_PROTECTION_ACTIVE_DOUBLE_ARMOR")
					{
						ProtectionBufSkill->SkillType = en_SkillType::SKILL_PROTECTION_ACTIVE_DOUBLE_ARMOR;
					}

					ProtectionBufSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;
					ProtectionBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
					ProtectionBufSkill->SkillMaxLevel = SkillMaxLevel;
					ProtectionBufSkill->IncreaseMinAttackPoint = IncreaseMinAttackPoint;
					ProtectionBufSkill->IncreaseMaxAttackPoint = IncreaseMaxAttackPoint;
					ProtectionBufSkill->IncreaseMeleeAttackSpeedPoint = IncreaseMeleeAttackSpeedPoint;
					ProtectionBufSkill->IncreaseMeleeAttackHitRate = IncreaseMeleeAttackHitRate;
					ProtectionBufSkill->IncreaseMagicAttackPoint = IncreaseMagicAttackPoint;
					ProtectionBufSkill->IncreaseMagicCastingPoint = IncreaseMagicCastingPoint;
					ProtectionBufSkill->IncreaseMagicAttackHitRate = IncreaseMagicAttackHitRate;
					ProtectionBufSkill->IncreaseDefencePoint = IncreaseDefencePoint;
					ProtectionBufSkill->IncreaseEvasionRate = IncreaseEvasionRate;
					ProtectionBufSkill->IncreaseMeleeCriticalPoint = IncreaseMeleeCriticalPoint;
					ProtectionBufSkill->IncreaseMagicCriticalPoint = IncreaseMagicCriticalPoint;
					ProtectionBufSkill->IncreaseSpeedPoint = IncreaseSpeedPoint;
					ProtectionBufSkill->IncreaseStatusAbnormalityResistance = IncreaseStatusAbnormalityResistance;
					ProtectionBufSkill->SkillCoolTime = SkillCoolTime;
					ProtectionBufSkill->SkillCastingTime = SkillCastingTime;
					ProtectionBufSkill->SkillDurationTime = SkillDurationTime;
					ProtectionBufSkill->SkillDotTime = SkillDotTime;
					ProtectionBufSkill->SkillDistance = SkillDistance;
					ProtectionBufSkill->SkillMotionTime = SkillMotionTime;
					ProtectionBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;

					_ProtectionSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)ProtectionBufSkill->SkillType, ProtectionBufSkill));
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
					st_SkillInfo* AssassinationAttackSkill = new st_SkillInfo();
					AssassinationAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_THIEF;
					AssassinationAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_ASSASSINATION_ACTIVE_ATTACK;

					string SkillType = AttackSkillFiled["SkillType"].GetString();
					string SkillKind = AttackSkillFiled["SkillKind"].GetString();
					string SkillName = AttackSkillFiled["SkillName"].GetString();
					string SkillStatusAbnormal = AttackSkillFiled["SkillStatusAbnormal"].GetString();
					string SkillStatusAbnormalMask = AttackSkillFiled["SkillStatusAbnormalMask"].GetString();
					int8 SkillMaxLevel = (int8)AttackSkillFiled["SkillMaxLevel"].GetInt();
					bool SkillIsDamage = AttackSkillFiled["SkillIsDamage"].GetBool();
					int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
					int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
					int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
					float SkillDistance = AttackSkillFiled["SkillDistance"].GetFloat();
					float SkillRangeX = AttackSkillFiled["SkillRangeX"].GetFloat();
					float SkillRangeY = AttackSkillFiled["SkillRangeY"].GetFloat();
					int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
					float SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetFloat();
					float SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetFloat();
					int64 SkillDamageOverTime = AttackSkillFiled["SkillDebufDamageOverTime"].GetInt64();
					int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();
					string NextComboSkill = AttackSkillFiled["NextComboSkill"].GetString();					

					if (SkillStatusAbnormal == "STATUS_ABNORMAL_NONE")
					{
						AssassinationAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_ASSASSINATION_POISON_INJECTION")
					{
						AssassinationAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_ASSASSINATION_POISON_INJECTION;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_ASSASSINATION_POISON_STUN")
					{
						AssassinationAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_ASSASSINATION_POISON_STUN;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_ASSASSINATION_BACK_STEP")
					{
						AssassinationAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_ASSASSINATION_BACK_STEP;
					}					

					if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_NONE")
					{
						AssassinationAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_ASSASSINATION_POISON_INJECTION_MASK")
					{
						AssassinationAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_ASSASSINATION_POISON_INJECTION_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_ASSASSINATION_POISON_STUN_MASK")
					{
						AssassinationAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_ASSASSINATION_POISON_STUN_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_ASSASSINATION_BACK_STEP_MASK")
					{
						AssassinationAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_ASSASSINATION_BACK_STEP_MASK;
					}					

					if (SkillKind == "SKILL_KIND_MELEE_SKILL")
					{
						AssassinationAttackSkill->SkillKind = en_SkillKinds::SKILL_KIND_MELEE_SKILL;
					}					

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
					else if (SkillType == "SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_CUT")
					{
						AssassinationAttackSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_CUT;
					}
					else if (SkillType == "SKILL_ASSASSINATION_ACTIVE_ATTACK_ADVANCE_CUT")
					{
						AssassinationAttackSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_ADVANCE_CUT;
					}
					else if (SkillType == "SKILL_ASSASSINATION_ACTIVE_ATTACK_POISON_INJECTION")
					{
						AssassinationAttackSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_POISON_INJECTION;
					}
					else if (SkillType == "SKILL_ASSASSINATION_ACTIVE_ATTACK_POISON_STUN")
					{
						AssassinationAttackSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_POISON_STUN;
					}
					else if (SkillType == "SKILL_ASSASSINATION_ACTIVE_ATTACK_ASSASSINATION")
					{
						AssassinationAttackSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_ASSASSINATION;
					}
					else if (SkillType == "SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP")
					{
						AssassinationAttackSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP;
					}

					AssassinationAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_ASSASSINATION;
					AssassinationAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());					
					AssassinationAttackSkill->SkillMaxLevel = SkillMaxLevel;
					AssassinationAttackSkill->SkillIsDamage = SkillIsDamage;
					AssassinationAttackSkill->SkillMinDamage = SkillMinDamage;
					AssassinationAttackSkill->SkillMaxDamage = SkillMaxDamage;
					AssassinationAttackSkill->SkillCoolTime = SkillCoolTime;
					AssassinationAttackSkill->SkillCastingTime = SkillCastingTime;
					AssassinationAttackSkill->SkillDurationTime = SkillDurationTime;
					AssassinationAttackSkill->SkillDotTime = SkillDotTime;
					AssassinationAttackSkill->SkillDistance = SkillDistance;
					AssassinationAttackSkill->SkillRangeX = SkillRangeX;
					AssassinationAttackSkill->SkillRangeY = SkillRangeY;
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
					else if (NextComboSkill == "SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_CUT")
					{
						AssassinationAttackSkill->NextComboSkill = en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_CUT;
					}

					_AssassinationSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)AssassinationAttackSkill->SkillType, AssassinationAttackSkill));
				}

				for (auto& BufSkillFiled : ActiveSkillListFiled["BufSkill"].GetArray())
				{
					st_SkillInfo* AssassinationBufSkill = new st_SkillInfo();
					AssassinationBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_WARRIOR;
					AssassinationBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_FIGHT_ACTIVE_BUF;

					int8 SkillNumber = (int8)BufSkillFiled["SkillNumber"].GetInt();
					string SkillType = BufSkillFiled["SkillType"].GetString();
					string SkillKind = BufSkillFiled["SkillKind"].GetString();
					string SkillName = BufSkillFiled["SkillName"].GetString();
					int8 SkillMaxLevel = (int8)BufSkillFiled["SkillMaxLevel"].GetInt();

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
					float SkillDistance = BufSkillFiled["SkillDistance"].GetFloat();
					int32 SkillMotionTime = BufSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = BufSkillFiled["SkillTargetEffectTime"].GetFloat();										

					if (SkillKind == "SKILL_KIND_SPELL_SKILL")
					{
						AssassinationBufSkill->SkillKind = en_SkillKinds::SKILL_KIND_SPELL_SKILL;
					}

					if (SkillType == "SKILL_ASSASSINATION_ACTIVE_BUF_STEALTH")
					{
						AssassinationBufSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_STEALTH;
					}
					else if (SkillType == "SKILL_ASSASSINATION_ACTIVE_BUF_SIXTH_SENSE_MAXIMIZE")
					{
						AssassinationBufSkill->SkillType = en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_SIXTH_SENSE_MAXIMIZE;
					}

					AssassinationBufSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;
					AssassinationBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());					
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

					_AssassinationSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)AssassinationBufSkill->SkillType, AssassinationBufSkill));
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
					st_SkillInfo* SpellAttackSkill = new st_SkillInfo();
					SpellAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
					SpellAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SPELL_ACTIVE_ATTACK;
					
					string SkillType = AttackSkillFiled["SkillType"].GetString();
					string SkillKind = AttackSkillFiled["SkillKind"].GetString();
					string SkillName = AttackSkillFiled["SkillName"].GetString();
					string SkillStatusAbnormal = AttackSkillFiled["SkillStatusAbnormal"].GetString();
					string SkillStatusAbnormalMask = AttackSkillFiled["SkillStatusAbnormalMask"].GetString();
					int8 SkillMaxLevel = (int8)AttackSkillFiled["SkillMaxLevel"].GetInt();
					bool SkillIsDamage = AttackSkillFiled["SkillIsDamage"].GetBool();
					int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
					int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
					int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
					float SkillDistance = AttackSkillFiled["SkillDistance"].GetFloat();
					float SkillRangeX = AttackSkillFiled["SkillRangeX"].GetFloat();
					float SkillRangeY = AttackSkillFiled["SkillRangeY"].GetFloat();
					int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
					float SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetFloat();
					float SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetFloat();
					int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();					
					string NextComboSkill = AttackSkillFiled["NextComboSkill"].GetString();					

					if (SkillStatusAbnormal == "STATUS_ABNORMAL_NONE")
					{
						SpellAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_SPELL_ICE_CHAIN")
					{
						SpellAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_CHAIN;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_SPELL_ICE_WAVE")
					{
						SpellAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_SPELL_ROOT")
					{
						SpellAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_SPELL_SLEEP")
					{
						SpellAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_SLEEP;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_SPELL_WINTER_BINDING")
					{
						SpellAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_WINTER_BINDING;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE")
					{
						SpellAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE;
					}

					if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_NONE")
					{
						SpellAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_SPELL_ICE_CHAIN_MASK")
					{
						SpellAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_CHAIN_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_SPELL_ICE_WAVE_MASK")
					{
						SpellAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_SPELL_ROOT_MASK")
					{
						SpellAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_SPELL_SLEEP_MASK")
					{
						SpellAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_SLEEP_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_SPELL_WINTER_BINDING_MASK")
					{
						SpellAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_WINTER_BINDING_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE_MASK")
					{
						SpellAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE_MASK;
					}

					if (SkillKind == "SKILL_KIND_SPELL_SKILL")
					{
						SpellAttackSkill->SkillKind = en_SkillKinds::SKILL_KIND_SPELL_SKILL;
					}					

					if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_FLAME_BOLT")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_BOLT;
					}
					else if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_FLAME_BLAZE")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_BLAZE;
					}
					else if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN;
					}
					else if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE;
					}
					else if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_ROOT")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT;
					}					
					else if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_SLEEP")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_SLEEP;
					}
					else if (SkillType == "SKILL_SPELL_ACTIVE_ATTACK_WINTER_BINDING")
					{
						SpellAttackSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_WINTER_BINDING;
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
					SpellAttackSkill->SkillMaxLevel = SkillMaxLevel;
					SpellAttackSkill->SkillIsDamage = SkillIsDamage;
					SpellAttackSkill->SkillMinDamage = SkillMinDamage;
					SpellAttackSkill->SkillMaxDamage = SkillMaxDamage;
					SpellAttackSkill->SkillCoolTime = SkillCoolTime;
					SpellAttackSkill->SkillCastingTime = SkillCastingTime;
					SpellAttackSkill->SkillDurationTime = SkillDurationTime;
					SpellAttackSkill->SkillDotTime = SkillDotTime;
					SpellAttackSkill->SkillDistance = SkillDistance;
					SpellAttackSkill->SkillRangeX = SkillRangeX;
					SpellAttackSkill->SkillRangeY = SkillRangeY;
					SpellAttackSkill->SkillMotionTime = SkillMotionTime;
					SpellAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
					SpellAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
					SpellAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
					SpellAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;					

					if (NextComboSkill == "SKILL_TYPE_NONE")
					{
						SpellAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
					}
					else if (NextComboSkill == "SKILL_SPELL_ACTIVE_ATTACK_FLAME_BLAZE")
					{
						SpellAttackSkill->NextComboSkill = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_BLAZE;
					}
					else if (NextComboSkill == "SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE")
					{
						SpellAttackSkill->NextComboSkill = en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE;
					}

					_SpellSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)SpellAttackSkill->SkillType, SpellAttackSkill));
				}

				for (auto& BufSkillFiled : ActiveSkillListFiled["BufSkill"].GetArray())
				{
					st_SkillInfo* SpellBufSkill = new st_SkillInfo();
					SpellBufSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_SHMAN;
					SpellBufSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SPELL_ACTIVE_BUF;

					string SkillType = BufSkillFiled["SkillType"].GetString();
					string SkillKind = BufSkillFiled["SkillKind"].GetString();
					string SkillName = BufSkillFiled["SkillName"].GetString();
					int8 SkillMaxLevel = (int8)BufSkillFiled["SkillMaxLevel"].GetInt();
					int SkillCoolTime = BufSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = BufSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = BufSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = BufSkillFiled["SkillDotTime"].GetInt64();
					float SkillDistance = BufSkillFiled["SkillDistance"].GetFloat();
					int32 SkillMotionTime = BufSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = BufSkillFiled["SkillTargetEffectTime"].GetFloat();								

					if (SkillKind == "SKILL_KIND_SPELL_SKILL")
					{
						SpellBufSkill->SkillKind = en_SkillKinds::SKILL_KIND_SPELL_SKILL;
					}

					if (SkillType == "SKILL_SPELL_ACTIVE_BUF_BACK_TELEPORT")
					{
						SpellBufSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_BUF_BACK_TELEPORT;
					}
					else if(SkillType == "SKILL_SPELL_ACTIVE_BUF_ILLUSION")
					{
						SpellBufSkill->SkillType = en_SkillType::SKILL_SPELL_ACTIVE_BUF_ILLUSION;
					}

					SpellBufSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_SPELL;
					SpellBufSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());					
					SpellBufSkill->SkillMaxLevel = SkillMaxLevel;
					SpellBufSkill->SkillCoolTime = SkillCoolTime;
					SpellBufSkill->SkillCastingTime = SkillCastingTime;
					SpellBufSkill->SkillDurationTime = SkillDurationTime;
					SpellBufSkill->SkillDotTime = SkillDotTime;
					SpellBufSkill->SkillDistance = SkillDistance;
					SpellBufSkill->SkillMotionTime = SkillMotionTime;
					SpellBufSkill->SkillTargetEffectTime = SkillTargetEffectTime;				

					_SpellSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)SpellBufSkill->SkillType, SpellBufSkill));
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
					st_SkillInfo* ShootingAttackSkill = new st_SkillInfo();
					ShootingAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_ARCHER;
					ShootingAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_SHOOTING_ACTIVE_ATTACK;

					string SkillType = AttackSkillFiled["SkillType"].GetString();
					string SkillKind = AttackSkillFiled["SkillKind"].GetString();
					string SkillName = AttackSkillFiled["SkillName"].GetString();
					string SkillStatusAbnormal = AttackSkillFiled["SkillStatusAbnormal"].GetString();
					string SkillStatusAbnormalMask = AttackSkillFiled["SkillStatusAbnormalMask"].GetString();
					int8 SkillMaxLevel = (int8)AttackSkillFiled["SkillMaxLevel"].GetInt();
					bool SkillIsDamage = AttackSkillFiled["SkillIsDamage"].GetBool();
					int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
					int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
					int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
					float SkillDistance = AttackSkillFiled["SkillDistance"].GetFloat();
					float SkillRangeX = AttackSkillFiled["SkillRangeX"].GetFloat();
					float SkillRangeY = AttackSkillFiled["SkillRangeY"].GetFloat();
					int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
					int64 SkillDebufTime = AttackSkillFiled["SkillDebufTime"].GetInt64();
					float SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetFloat();
					float SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetFloat();
					int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();										

					if (SkillStatusAbnormal == "STATUS_ABNORMAL_NONE")
					{
						ShootingAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
					}	

					if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_NONE")
					{
						ShootingAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
					}

					if (SkillKind == "SKILL_KIND_RANGE_SKILL")
					{
						ShootingAttackSkill->SkillKind = en_SkillKinds::SKILL_KIND_RANGE_SKILL;
					}

					if (SkillType == "SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING")
					{
						ShootingAttackSkill->SkillType = en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING;
					}

					ShootingAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_SHOOTING;
					ShootingAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());					
					ShootingAttackSkill->SkillMaxLevel = SkillMaxLevel;
					ShootingAttackSkill->SkillIsDamage = SkillIsDamage;
					ShootingAttackSkill->SkillMinDamage = SkillMinDamage;
					ShootingAttackSkill->SkillMaxDamage = SkillMaxDamage;
					ShootingAttackSkill->SkillCoolTime = SkillCoolTime;
					ShootingAttackSkill->SkillCastingTime = SkillCastingTime;
					ShootingAttackSkill->SkillDurationTime = SkillDurationTime;
					ShootingAttackSkill->SkillDotTime = SkillDotTime;
					ShootingAttackSkill->SkillDistance = SkillDistance;
					ShootingAttackSkill->SkillRangeX = SkillRangeX;
					ShootingAttackSkill->SkillRangeY = SkillRangeY;
					ShootingAttackSkill->SkillMotionTime = SkillMotionTime;
					ShootingAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
					ShootingAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
					ShootingAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
					ShootingAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;										

					_ShootingSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)ShootingAttackSkill->SkillType, ShootingAttackSkill));
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
					st_SkillInfo* DisciplineAttackSkill = new st_SkillInfo();
					DisciplineAttackSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
					DisciplineAttackSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_DISCIPLINE_ACTIVE_ATTACK;

					string SkillType = AttackSkillFiled["SkillType"].GetString();
					string SkillKind = AttackSkillFiled["SkillKind"].GetString();
					string SkillName = AttackSkillFiled["SkillName"].GetString();
					string SkillStatusAbnormal = AttackSkillFiled["SkillStatusAbnormal"].GetString();
					string SkillStatusAbnormalMask = AttackSkillFiled["SkillStatusAbnormalMask"].GetString();
					int8 SkillMaxLevel = (int8)AttackSkillFiled["SkillMaxLevel"].GetInt();
					bool SkillIsDamage = AttackSkillFiled["SkillIsDamage"].GetBool();
					int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
					int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
					int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
					float SkillDistance = AttackSkillFiled["SkillDistance"].GetFloat();
					float SkillRangeX = AttackSkillFiled["SkillRangeX"].GetFloat();
					float SkillRangeY = AttackSkillFiled["SkillRangeY"].GetFloat();
					int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
					int64 SkillDebufTime = AttackSkillFiled["SkillDebufTime"].GetInt64();
					float SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetFloat();
					float SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetFloat();
					int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();
					string NextComboSkill = AttackSkillFiled["NextComboSkill"].GetString();					

					if (SkillStatusAbnormal == "STATUS_ABNORMAL_NONE")
					{
						DisciplineAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_DISCIPLINE_ROOT")
					{
						DisciplineAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT;
					}
					else if (SkillStatusAbnormal == "STATUS_ABNORMAL_DISCIPLINE_JUDGMENT")
					{
						DisciplineAttackSkill->SkillStatusAbnormal = en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_JUDGMENT;
					}

					if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_NONE")
					{
						DisciplineAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_NONE;
					}					
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_DISCIPLINE_ROOT_MASK")
					{
						DisciplineAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT_MASK;
					}
					else if (SkillStatusAbnormalMask == "STATUS_ABNORMAL_DISCIPLINE_JUDGMENT_MASK")
					{
						DisciplineAttackSkill->SkillStatusAbnormalMask = en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_JUDGMENT_MASK;
					}

					if (SkillKind == "SKILL_KIND_SPELL_SKILL")
					{
						DisciplineAttackSkill->SkillKind = en_SkillKinds::SKILL_KIND_SPELL_SKILL;
					}					

					if (SkillType == "SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE")
					{
						DisciplineAttackSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE;
					}
					else if (SkillType == "SKILL_DISCIPLINE_ACTIVE_ATTACK_THUNDER_BOLT")
					{
						DisciplineAttackSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_THUNDER_BOLT;
					}
					else if (SkillType == "SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT")
					{
						DisciplineAttackSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT;
					}
					else if (SkillType == "SKILL_DISCIPLINE_ACTIVE_ATTACK_JUDGMENT")
					{
						DisciplineAttackSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_JUDGMENT;
					}

					DisciplineAttackSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_DISCIPLINE;
					DisciplineAttackSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());					
					DisciplineAttackSkill->SkillMaxLevel = SkillMaxLevel;
					DisciplineAttackSkill->SkillIsDamage = SkillIsDamage;
					DisciplineAttackSkill->SkillMinDamage = SkillMinDamage;
					DisciplineAttackSkill->SkillMaxDamage = SkillMaxDamage;
					DisciplineAttackSkill->SkillCoolTime = SkillCoolTime;
					DisciplineAttackSkill->SkillCastingTime = SkillCastingTime;
					DisciplineAttackSkill->SkillDurationTime = SkillDurationTime;
					DisciplineAttackSkill->SkillDotTime = SkillDotTime;
					DisciplineAttackSkill->SkillDistance = SkillDistance;
					DisciplineAttackSkill->SkillRangeX = SkillRangeX;
					DisciplineAttackSkill->SkillRangeY = SkillRangeY;
					DisciplineAttackSkill->SkillMotionTime = SkillMotionTime;
					DisciplineAttackSkill->SkillTargetEffectTime = SkillTargetEffectTime;
					DisciplineAttackSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
					DisciplineAttackSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
					DisciplineAttackSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;

					if (NextComboSkill == "SKILL_TYPE_NONE")
					{
						DisciplineAttackSkill->NextComboSkill = en_SkillType::SKILL_TYPE_NONE;
					}
					else if (NextComboSkill == "SKILL_DISCIPLINE_ACTIVE_ATTACK_THUNDER_BOLT")
					{
						DisciplineAttackSkill->NextComboSkill = en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_THUNDER_BOLT;
					}

					_DisciplineSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)DisciplineAttackSkill->SkillType, DisciplineAttackSkill));
				}

				for (auto& HealSkillFiled : ActiveSkillListFiled["HealSkill"].GetArray())
				{
					st_SkillInfo* DisciplineHealSkill = new st_SkillInfo();
					DisciplineHealSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_TAOIST;
					DisciplineHealSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_DISCIPLINE_ACTIVE_HEAL;

					string SkillType = HealSkillFiled["SkillType"].GetString();
					string SkillKind = HealSkillFiled["SkillKind"].GetString();
					string SkillName = HealSkillFiled["SkillName"].GetString();
					int8 SkillMaxLevel = (int8)HealSkillFiled["SkillMaxLevel"].GetInt();
					int SkillMinHeal = HealSkillFiled["SkillMinHeal"].GetInt();
					int SkillMaxHeal = HealSkillFiled["SkillMaxHeal"].GetInt();
					int SkillCoolTime = HealSkillFiled["SkillCoolTime"].GetInt();
					int SkillCastingTime = HealSkillFiled["SkillCastingTime"].GetInt();
					int64 SkillDurationTime = HealSkillFiled["SkillDurationTime"].GetInt64();
					int64 SkillDotTime = HealSkillFiled["SkillDotTime"].GetInt64();
					float SkillDistance = HealSkillFiled["SkillDistance"].GetFloat();
					int32 SkillMotionTime = HealSkillFiled["SkillMotionTime"].GetInt();
					float SkillTargetEffectTime = HealSkillFiled["SkillTargetEffectTime"].GetFloat();
					string NextComboSkill = HealSkillFiled["NextComboSkill"].GetString();					

					if (SkillType == "SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT")
					{
						DisciplineHealSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT;
					}
					else if (SkillType == "SKILL_DISCIPLINE_ACTIVE_HEAL_RECOVERY_LIGHT")
					{
						DisciplineHealSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_RECOVERY_LIGHT;
					}
					else if (SkillType == "SKILL_DISCIPLINE_ACTIVE_HEAL_VITALITY_LIGHT")
					{
						DisciplineHealSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_VITALITY_LIGHT;
					}
					else if (SkillType == "SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_GRACE")
					{
						DisciplineHealSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_GRACE;
					}
					else if (SkillType == "SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND")
					{
						DisciplineHealSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND;
					}
					else if (SkillType == "SKILL_DISCIPLINE_ACTIVE_HEAL_RECOVERY_WIND")
					{
						DisciplineHealSkill->SkillType = en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_RECOVERY_WIND;
					}

					if (SkillKind == "SKILL_KIND_HEAL_SKILL")
					{
						DisciplineHealSkill->SkillKind = en_SkillKinds::SKILL_KIND_HEAL_SKILL;
					}					

					DisciplineHealSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_DISCIPLINE;
					DisciplineHealSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());					
					DisciplineHealSkill->SkillMaxLevel = SkillMaxLevel;
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

					_DisciplineSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)DisciplineHealSkill->SkillType, DisciplineHealSkill));
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
		for (auto& GoblinMonsterSkills : Filed["GoblinSkillList"].GetArray())
		{
			for (auto& PassiveSkillFiled : GoblinMonsterSkills["PassiveSkillList"].GetArray())
			{

			}

			for (auto& AttackSkillFiled : GoblinMonsterSkills["ActiveSkillList"].GetArray())
			{
				st_SkillInfo* GoblinActiveSkill = new st_SkillInfo();
				GoblinActiveSkill->SkillLargeCategory = en_SkillLargeCategory::SKILL_LARGE_CATEGORY_MONSTER_MELEE;
				GoblinActiveSkill->SkillMediumCategory = en_SkillMediumCategory::SKILL_MEDIUM_CATEGORY_NONE;

				string SkillType = AttackSkillFiled["SkillType"].GetString();
				string SkillKind = AttackSkillFiled["SkillKind"].GetString();
				string SkillName = AttackSkillFiled["SkillName"].GetString();
				int8 SkillMaxLevel = (int8)AttackSkillFiled["SkillMaxLevel"].GetInt();
				bool SkillIsDamage = AttackSkillFiled["SkillIsDamage"].GetBool();
				int SkillMinDamage = AttackSkillFiled["SkillMinDamage"].GetInt();
				int SkillMaxDamage = AttackSkillFiled["SkillMaxDamage"].GetInt();
				int SkillCoolTime = AttackSkillFiled["SkillCoolTime"].GetInt();
				int SkillCastingTime = AttackSkillFiled["SkillCastingTime"].GetInt();
				int64 SkillDurationTime = AttackSkillFiled["SkillDurationTime"].GetInt64();
				int64 SkillDotTime = AttackSkillFiled["SkillDotTime"].GetInt64();
				float SkillDistance = AttackSkillFiled["SkillDistance"].GetFloat();
				int32 SkillMotionTime = AttackSkillFiled["SkillMotionTime"].GetInt();
				float SkillTargetEffectTime = AttackSkillFiled["SkillTargetEffectTime"].GetFloat();
				float SkillDebufAttackSpeed = (int8)AttackSkillFiled["SkillDebufAttackSpeed"].GetFloat();
				float SkillDebufMovingSpeed = (int8)AttackSkillFiled["SkillDebufMovingSpeed"].GetFloat();
				int8 StatusAbnormalityProbability = (int8)AttackSkillFiled["StatusAbnormalityProbability"].GetInt();				

				if (SkillType == "SKILL_GOBLIN_ACTIVE_MELEE_DEFAULT_ATTACK")
				{
					GoblinActiveSkill->SkillType = en_SkillType::SKILL_GOBLIN_ACTIVE_MELEE_DEFAULT_ATTACK;
				}

				if (SkillKind == "SKILL_KIND_MELEE_SKILL")
				{
					GoblinActiveSkill->SkillKind = en_SkillKinds::SKILL_KIND_MELEE_SKILL;
				}

				GoblinActiveSkill->SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_NONE;
				GoblinActiveSkill->SkillName = (LPWSTR)CA2W(SkillName.c_str());
				GoblinActiveSkill->SkillMaxLevel = SkillMaxLevel;
				GoblinActiveSkill->SkillIsDamage = SkillIsDamage;
				GoblinActiveSkill->SkillMinDamage = SkillMinDamage;
				GoblinActiveSkill->SkillMaxDamage = SkillMaxDamage;
				GoblinActiveSkill->SkillCoolTime = SkillCoolTime;
				GoblinActiveSkill->SkillCastingTime = SkillCastingTime;
				GoblinActiveSkill->SkillDurationTime = SkillDurationTime;
				GoblinActiveSkill->SkillDotTime = SkillDotTime;
				GoblinActiveSkill->SkillDistance = SkillDistance;
				GoblinActiveSkill->SkillMotionTime = SkillMotionTime;
				GoblinActiveSkill->SkillTargetEffectTime = SkillTargetEffectTime;
				GoblinActiveSkill->SkillDebufAttackSpeed = SkillDebufAttackSpeed;
				GoblinActiveSkill->SkillDebufMovingSpeed = SkillDebufMovingSpeed;
				GoblinActiveSkill->StatusAbnormalityProbability = StatusAbnormalityProbability;				

				_GoblinSkillDatas.insert(pair<int16, st_SkillInfo*>((int16)GoblinActiveSkill->SkillType, GoblinActiveSkill));
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

		if (EnvironmentName == "벽")
		{
			EnvironmentType = en_GameObjectType::OBJECT_WALL;
		}
		else if (EnvironmentName == "돌")
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
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_WEAPON_DAGGER_WOOD")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_DAGGER_WOOD);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAPON_LONG_SWORD;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_DAGGER_WOOD;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_WEAPON_LONG_SWORD_WOOD")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_LONG_SWORD_WOOD);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAPON_LONG_SWORD;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_LONG_SWORD_WOOD;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_WEAPON_GREAT_SWORD_WOOD")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_GREAT_SWORD_WOOD);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAPON_LONG_SWORD;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_WEAPON_GREAT_SWORD_WOOD;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SAMLL_CATEGORY_WEAPON_SHIELD_WOOD")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SAMLL_CATEGORY_WEAPON_SHIELD_WOOD);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAPON_LONG_SWORD;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SAMLL_CATEGORY_WEAPON_SHIELD_WOOD;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SAMLL_CATEGORY_WEAPON_BOW_WOOD")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SAMLL_CATEGORY_WEAPON_BOW_WOOD);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_WEAPON;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_WEAPON_LONG_SWORD;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SAMLL_CATEGORY_WEAPON_BOW_WOOD;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARMOR;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_ARMOR_WEAR;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_WEAR_LEATHER;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARMOR;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_ARMOR_WEAR;
				CommonCraftingCompleteItem->_ItemInfo.ItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_HAT_LEATHER;
				CommonCraftingCompleteItem->_ItemInfo.OwnerCraftingTable = en_UIObjectInfo::UI_OBJECT_INFO_CRAFTING_TABLE_COMMON;
			}
			else if (CraftingCompleteItemSmallCategory == "ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER")
			{
				CommonCraftingCompleteItem = G_ObjectManager->ItemCreate(en_SmallItemCategory::ITEM_SMALL_CATEGORY_ARMOR_BOOT_LEATHER);
				CommonCraftingCompleteItem->_ItemInfo.ItemLargeCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_ARMOR;
				CommonCraftingCompleteItem->_ItemInfo.ItemMediumCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_ARMOR_WEAR;
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

		for (auto& MapInfoListField : Filed["MapInfoList"].GetArray())
		{
			int16 MapID = (int16)MapInfoListField["MapID"].GetInt();
			string MapName = MapInfoListField["MapName"].GetString();
			int32 MapSectorSize = MapInfoListField["MapSectorSize"].GetInt();
			int8 ChannelCount = (int8)MapInfoListField["ChannelCount"].GetInt();
			int32 Left = MapInfoListField["Left"].GetInt();
			int32 Right = MapInfoListField["Right"].GetInt();
			int32 Up = MapInfoListField["Up"].GetInt();
			int32 Down = MapInfoListField["Down"].GetInt();

			for (auto& GameObjectListField : MapInfoListField["GameObjectList"].GetArray())
			{
				en_GameObjectType GameObjectType = en_GameObjectType::OBJECT_NON_TYPE;

				string ObjectTypeName = GameObjectListField["ObjectType"].GetString();
				
				if (ObjectTypeName == "OBJECT_NON_PLAYER_GENERAL_MERCHANT")
				{
					GameObjectType = en_GameObjectType::OBJECT_NON_PLAYER_GENERAL_MERCHANT;
				}
				else if (ObjectTypeName == "OBJECT_WALL")
				{
					GameObjectType = en_GameObjectType::OBJECT_WALL;
				}
				else if(ObjectTypeName == "OBJECT_GOBLIN")
				{
					GameObjectType = en_GameObjectType::OBJECT_GOBLIN;
				}
				else if (ObjectTypeName == "OBJECT_STORAGE_SMALL_BOX")
				{
					GameObjectType = en_GameObjectType::OBJECT_STORAGE_SMALL_BOX;
				}

				vector<Vector2Int> Positions;

				for (auto& PositionListField : GameObjectListField["PositionList"].GetArray())
				{
					Vector2Int Position;

					int32 XPosition = PositionListField["X"].GetInt();
					int32 YPosition = PositionListField["Y"].GetInt();

					if (XPosition == 0 && YPosition == 0)
					{
						continue;
					}

					Position.X = XPosition;
					Position.Y = YPosition;

					Positions.push_back(Position);
				}

				MapInfoData->GameObjectList.insert(pair<en_GameObjectType, vector<Vector2Int>>(GameObjectType, Positions));
			}

			MapInfoData->MapID = MapID;
			MapInfoData->MapName = MapName;
			MapInfoData->MapSectorSize = MapSectorSize;
			MapInfoData->ChannelCount = ChannelCount;
			MapInfoData->Left = Left;
			MapInfoData->Right = Right;
			MapInfoData->Up = Up;
			MapInfoData->Down = Down;

			_MapInfoDatas.insert(pair<int64, st_MapInfoData*>(MapInfoData->MapID, MapInfoData));
		}		
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

void CDataManager::LoadDataDropItem(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["DropItems"].GetArray())
	{
		vector<st_DropData> GoblinDropItems;
		for (auto& GoblinFiled : Filed["GoblinDropItems"].GetArray())
		{
			string DropItemSmallCategory = GoblinFiled["DropItemSmallCategory"].GetString();
			int Probability = GoblinFiled["Probability"].GetInt();
			int8 MinCount = (int8)(GoblinFiled["MinCount"].GetInt());
			int16 MaxCount = (int16)(GoblinFiled["MaxCount"].GetInt());

			st_DropData DropData;

			if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_FABRIC")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_FABRIC;
			}
			else if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_COIN")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_COIN;
			}

			DropData.Probability = Probability;
			DropData.MinCount = MinCount;
			DropData.MaxCount = MaxCount;

			GoblinDropItems.push_back(DropData);
		}

		vector<st_DropData> StoneDropItmes;
		for (auto& StoneFiled : Filed["StoneDropItems"].GetArray())
		{
			string DropItemSmallCategory = StoneFiled["DropItemSmallCategory"].GetString();
			int Probability = StoneFiled["Probability"].GetInt();
			int8 MinCount = (int8)(StoneFiled["MinCount"].GetInt());
			int16 MaxCount = (int16)(StoneFiled["MaxCount"].GetInt());

			st_DropData DropData;

			if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_STONE")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_STONE;
			}			

			DropData.Probability = Probability;
			DropData.MinCount = MinCount;
			DropData.MaxCount = MaxCount;

			StoneDropItmes.push_back(DropData);
		}

		vector<st_DropData> TreeDropItmes;
		for (auto& TreeFiled : Filed["TreeDropItems"].GetArray())
		{
			string DropItemSmallCategory = TreeFiled["DropItemSmallCategory"].GetString();
			int Probability = TreeFiled["Probability"].GetInt();
			int8 MinCount = (int8)(TreeFiled["MinCount"].GetInt());
			int16 MaxCount = (int16)(TreeFiled["MaxCount"].GetInt());

			st_DropData DropData;

			if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_LOG;
			}

			DropData.Probability = Probability;
			DropData.MinCount = MinCount;
			DropData.MaxCount = MaxCount;

			TreeDropItmes.push_back(DropData);
		}		

		vector<st_DropData> PotatoDropItmes;
		for (auto& PotatoFiled : Filed["PotatoDropItems"].GetArray())
		{
			string DropItemSmallCategory = PotatoFiled["DropItemSmallCategory"].GetString();
			int Probability = PotatoFiled["Probability"].GetInt();
			int8 MinCount = (int8)(PotatoFiled["MinCount"].GetInt());
			int16 MaxCount = (int16)(PotatoFiled["MaxCount"].GetInt());

			st_DropData DropData;

			if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_POTATO;
			}

			DropData.Probability = Probability;
			DropData.MinCount = MinCount;
			DropData.MaxCount = MaxCount;

			PotatoDropItmes.push_back(DropData);
		}

		vector<st_DropData> CornDropItmes;
		for (auto& CornFiled : Filed["CornDropItems"].GetArray())
		{
			string DropItemSmallCategory = CornFiled["DropItemSmallCategory"].GetString();
			int Probability = CornFiled["Probability"].GetInt();
			int8 MinCount = (int8)(CornFiled["MinCount"].GetInt());
			int16 MaxCount = (int16)(CornFiled["MaxCount"].GetInt());

			st_DropData DropData;

			if (DropItemSmallCategory == "ITEM_SMALL_CATEGORY_CROP_FRUIT_CORN")
			{
				DropData.DropItemSmallCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_CORN;
			}

			DropData.Probability = Probability;
			DropData.MinCount = MinCount;
			DropData.MaxCount = MaxCount;

			CornDropItmes.push_back(DropData);
		}

		_DropItems.insert(pair<en_GameObjectType, vector<st_DropData>>(en_GameObjectType::OBJECT_GOBLIN, GoblinDropItems));
		_DropItems.insert(pair<en_GameObjectType, vector<st_DropData>>(en_GameObjectType::OBJECT_STONE, StoneDropItmes));
		_DropItems.insert(pair<en_GameObjectType, vector<st_DropData>>(en_GameObjectType::OBJECT_TREE, TreeDropItmes));
		_DropItems.insert(pair<en_GameObjectType, vector<st_DropData>>(en_GameObjectType::OBJECT_CROP_POTATO, PotatoDropItmes));
		_DropItems.insert(pair<en_GameObjectType, vector<st_DropData>>(en_GameObjectType::OBJECT_CROP_CORN, CornDropItmes));
	}
}

void CDataManager::LoadDataDropMoney(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["DropMoneyTable"].GetArray())
	{
		int8 Level = (int8)Filed["Level"].GetInt();
		int32 MoneyAmountMin = Filed["MoneyAmountMin"].GetInt();
		int32 MoneyAmountMax = Filed["MoneyAmountMax"].GetInt();

		Vector2Int Money;
		Money.X = MoneyAmountMin;
		Money.Y = MoneyAmountMax;

		_DropMoneys.insert(pair<int8, Vector2Int>(Level,Money));
	}
}

st_SkillInfo* CDataManager::FindSkillData(en_SkillType FindSkillType)
{
	switch (FindSkillType)
	{
	case en_SkillType::SKILL_TYPE_NONE:
		CRASH("None 스킬 데이터 찾기 요청");
		break;	
	case en_SkillType::SKILL_GLOBAL_SKILL:
	case en_SkillType::SKILL_DEFAULT_ATTACK:
	case en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE:
		return (*_PublicSkillDatas.find((int16)FindSkillType)).second;		
	case en_SkillType::SKILL_FIGHT_TWO_HAND_SWORD_MASTER:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:	
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_WRATH_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FLY_KNIFE:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_COMBO_FLY_KNIFE:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_JUMPING_ATTACK:	
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_PIERCING_WAVE:	
	case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:		
	case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_COUNTER_ARMOR:
		return (*_FightSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_POWERFUL_ATTACK:
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHARP_ATTACK:
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_LAST_ATTACK:
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_COUNTER:
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SWORD_STORM:
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE:
	case en_SkillType::SKILL_PROTECTION_ACTIVE_BUF_FURY:
	case en_SkillType::SKILL_PROTECTION_ACTIVE_DOUBLE_ARMOR:	
		return (*_ProtectionSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_BOLT:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_BLAZE:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:		
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_SLEEP:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_WINTER_BINDING:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:
	case en_SkillType::SKILL_SPELL_ACTIVE_BUF_BACK_TELEPORT:
	case en_SkillType::SKILL_SPELL_ACTIVE_BUF_ILLUSION:	
		return (*_SpellSkillDatas.find((int16)FindSkillType)).second;			
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_THUNDER_BOLT:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_JUDGMENT:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_RECOVERY_LIGHT:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_VITALITY_LIGHT:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_GRACE:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_RECOVERY_WIND:
		return (*_DisciplineSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_CUT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_ADVANCE_CUT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_POISON_INJECTION:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_POISON_STUN:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_ASSASSINATION:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_STEALTH:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_SIXTH_SENSE_MAXIMIZE:
		return (*_AssassinationSkillDatas.find((int16)FindSkillType)).second;
	case en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING:
		return (*_ShootingSkillDatas.find((int16)FindSkillType)).second;	
	case en_SkillType::SKILL_GOBLIN_ACTIVE_MELEE_DEFAULT_ATTACK:
		return (*_GoblinSkillDatas.find((int16)FindSkillType)).second;
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
	case en_GameObjectType::OBJECT_GOBLIN:	
		return (*_Monsters.find(MonsterGameObjectType)).second->GetExpPoint;
	}
}
