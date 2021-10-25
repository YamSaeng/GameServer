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

			st_ItemData* PotionItemData = new st_ItemData();
			PotionItemData->LargeItemCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_POTION;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_HEAL")
			{
				PotionItemData->MediumItemCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_HEAL;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL")
			{
				PotionItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_POTION_HEAL_SMALL;
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

			_Items.insert(pair<int16, st_ItemData*>((int16)PotionItemData->SmallItemCategory, PotionItemData));
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

			st_ItemData* SkillBookItemData = new st_ItemData();
			SkillBookItemData->LargeItemCategory = en_LargeItemCategory::ITEM_LARGE_CATEGORY_SKILLBOOK;

			if (MediumCategory == "ITEM_MEDIUM_CATEGORY_NONE")
			{
				SkillBookItemData->MediumItemCategory = en_MediumItemCategory::ITEM_MEDIUM_CATEGORY_NONE;
			}

			if (SmallCategory == "ITEM_SMALL_CATEGORY_SKILLBOOK_CHOHONE")
			{
				SkillBookItemData->SmallItemCategory = en_SmallItemCategory::ITEM_SMALL_CATEGORY_SKILLBOOK_CHOHONE;
			}

			if (ItemObjectType == "OBJECT_ITEM_CONSUMABLE_SKILL_BOOK_CHOHONE")
			{
				SkillBookItemData->ItemObjectType = en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK_CHOHONE;
			}

			SkillBookItemData->ItemExplain = ItemExplain;
			SkillBookItemData->ItemName = ItemName;
			SkillBookItemData->ItemMinDamage = 0;
			SkillBookItemData->ItemMaxDamage = 0;
			SkillBookItemData->ItemDefence = 0;
			SkillBookItemData->ItemMaxCount = ItemMaxCount;
			SkillBookItemData->ItemThumbnailImagePath = ImageFilePath;						

			_Items.insert(pair<int16, st_ItemData*>((int16)SkillBookItemData->SmallItemCategory, SkillBookItemData));
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

void CDataManager::LoadDataStatus(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["Status"].GetArray())
	{
		int16 PlayerType = (int16)Filed["PlayerType"].GetInt();
		int Level = Filed["Level"].GetInt();
		int MaxHP = Filed["MaxHP"].GetInt();
		int MaxMP = Filed["MaxMP"].GetInt();
		int MaxDP = Filed["MaxDP"].GetInt();
		int MinAttackDamage = Filed["MinAttackDamage"].GetInt();
		int MaxAttackDamage = Filed["MaxAttackDamage"].GetInt();
		int16 CriticalPoint = (int16)(Filed["CriticalPoint"].GetInt());
		float Speed = Filed["Speed"].GetFloat();

		st_PlayerStatusData* StatusData = new st_PlayerStatusData();
		StatusData->PlayerType = PlayerType;
		StatusData->Level = Level;
		StatusData->MaxHP = MaxHP;
		StatusData->MaxMP = MaxMP;
		StatusData->MaxDP = MaxDP;
		StatusData->MinAttackDamage = MinAttackDamage;
		StatusData->MaxAttackDamage = MaxAttackDamage;
		StatusData->CriticalPoint = CriticalPoint;
		StatusData->Speed = Speed;

		_Status.insert(pair<int16, st_PlayerStatusData*>(StatusData->PlayerType, StatusData));
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
			int MinAttackDamage = MonsterStatInfoFiled["MinAttackDamage"].GetInt();
			int MaxAttackDamage = MonsterStatInfoFiled["MaxAttackDamage"].GetInt();
			int CriticalPoint = MonsterStatInfoFiled["CriticalPoint"].GetInt();
			float Speed = MonsterStatInfoFiled["Speed"].GetFloat();
			int SearchCellDistance = MonsterStatInfoFiled["SearchCellDistance"].GetInt();
			int ChaseCellDistance = MonsterStatInfoFiled["ChaseCellDistance"].GetInt();
			int SearchTick = MonsterStatInfoFiled["SearchTick"].GetInt();
			int PatrolTick = MonsterStatInfoFiled["PatrolTick"].GetInt();
			int AttackTick = MonsterStatInfoFiled["AttackTick"].GetInt();
			int AttackRange = MonsterStatInfoFiled["AttackRange"].GetInt();
			int16 GetDPPoint = (int16)MonsterStatInfoFiled["GetDPPoint"].GetInt();
			int TotalExp = MonsterStatInfoFiled["TotalExp"].GetInt();

			MonsterData->MonsterStatInfo.Level = Level;
			MonsterData->MonsterStatInfo.MaxHP = MaxHP;
			MonsterData->MonsterStatInfo.MinAttackDamage = MinAttackDamage;
			MonsterData->MonsterStatInfo.MaxAttackDamage = MaxAttackDamage;
			MonsterData->MonsterStatInfo.CriticalPoint = CriticalPoint;
			MonsterData->MonsterStatInfo.Speed = Speed;
			MonsterData->MonsterStatInfo.SearchCellDistance = SearchCellDistance;
			MonsterData->MonsterStatInfo.ChaseCellDistance = ChaseCellDistance;
			MonsterData->SearchTick = SearchTick;
			MonsterData->PatrolTick = PatrolTick;
			MonsterData->AttackTick = AttackTick;
			MonsterData->MonsterStatInfo.AttackRange = AttackRange;
			MonsterData->GetDPPoint = GetDPPoint;
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

	for (auto& Filed : Document["Skills"].GetArray())
	{
		st_SkillData* SkillData = new st_SkillData();

		int SkillDataSheetId = Filed["SkillDataSheetId"].GetInt();
		string SkillName = Filed["SkillName"].GetString();
		int SkillCoolTime = Filed["SkillCoolTime"].GetInt();
		int SkillCastingTime = Filed["SkillCastingTime"].GetInt();
		int SkillDistance = Filed["SkillDistance"].GetInt();
		string SkillImagePath = Filed["SkillThumbnailImagePath"].GetString();

		SkillData->SkillDataId = SkillDataSheetId;
		SkillData->SkillName = SkillName;
		SkillData->SkillCoolTime = SkillCoolTime;
		SkillData->SkillCastingTime = SkillCastingTime;
		SkillData->SkillDistance = SkillDistance;
		SkillData->SkillThumbnailImagePath = SkillImagePath;

		_Skills.insert(pair<int32, st_SkillData*>(SkillDataSheetId, SkillData));
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
