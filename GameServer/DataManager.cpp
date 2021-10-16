#include "pch.h"
#include "DataManager.h"

void CDataManager::LoadDataItem(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["Weapons"].GetArray())
	{
		int DataSheetId = Filed["DataSheetId"].GetInt();
		string ItemName = Filed["ItemName"].GetString();
		int Damage = Filed["Damage"].GetInt();
		bool IsEquipped = Filed["IsEquipped"].GetBool();
		string ImageFilePath = Filed["ImageFilePath"].GetString();

		st_WeaponData* WeaponData = new st_WeaponData();
		WeaponData->DataSheetId = DataSheetId;
		WeaponData->ItemName = ItemName;

		if (ItemName == "나무 검")
		{
			WeaponData->ItemType = en_ItemType::ITEM_TYPE_WEAPON_WOOD_SWORD;
		}

		WeaponData->ItemCategory = en_ItemCategory::ITEM_CATEGORY_WEAPON;

		WeaponData->ThumbnailImagePath = ImageFilePath;
		WeaponData->_Damage = Damage;

		_Items.insert(pair<int32, st_WeaponData*>(WeaponData->DataSheetId, WeaponData));
	}

	for (auto& Filed : Document["Armors"].GetArray())
	{
		int DataSheetId = Filed["DataSheetId"].GetInt();
		string ItemName = Filed["ItemName"].GetString();
		int Defence = Filed["Defence"].GetInt();
		bool IsEquipped = Filed["IsEquipped"].GetBool();
		string ImageFilePath = Filed["ImageFilePath"].GetString();

		st_ArmorData* ArmorData = new st_ArmorData();
		ArmorData->DataSheetId = DataSheetId;
		ArmorData->ItemName = ItemName;

		if (ItemName == "나무 갑옷")
		{
			ArmorData->ItemType = en_ItemType::ITEM_TYPE_ARMOR_WOOD_ARMOR;
		}
		else if (ItemName == "가죽 모자")
		{
			ArmorData->ItemType = en_ItemType::ITEM_TYPE_ARMOR_LETHER_HAT;
		}

		ArmorData->ItemCategory = en_ItemCategory::ITEM_CATEGORY_ARMOR;

		ArmorData->ThumbnailImagePath = ImageFilePath;
		ArmorData->_Defence = Defence;

		_Items.insert(pair<int32, st_ArmorData*>(ArmorData->DataSheetId, ArmorData));
	}

	for (auto& Filed : Document["Consumables"].GetArray())
	{
		int DataSheetId = Filed["DataSheetId"].GetInt();
		string ItemName = Filed["ItemName"].GetString();
		int MaxCount = Filed["MaxCount"].GetInt();
		bool IsEquipped = Filed["IsEquipped"].GetBool();
		string ImageFilePath = Filed["ImageFilePath"].GetString();

		st_ConsumableData* ConsumableData = new st_ConsumableData();
		ConsumableData->DataSheetId = DataSheetId;
		ConsumableData->ItemName = ItemName;

		ItemName.assign(ConsumableData->ItemName.begin(), ConsumableData->ItemName.end());
		if (ItemName == "회복물약(소)")
		{
			ConsumableData->ItemType = en_ItemType::ITEM_TYPE_POTION_HEAL_SMALL;
			ConsumableData->ItemCategory = en_ItemCategory::ITEM_CATEGORY_POTION;
		}
		else if (ItemName == "초혼비무 스킬책")
		{
			ConsumableData->ItemType = en_ItemType::ITEM_TYPE_SKILL_BOOK_CHOHONE;
			ConsumableData->ItemCategory = en_ItemCategory::ITEM_CATEGORY_SKILLBOOK;
		}

		ConsumableData->ThumbnailImagePath = ImageFilePath;
		ConsumableData->_MaxCount = MaxCount;

		_Items.insert(pair<int32, st_ConsumableData*>(ConsumableData->DataSheetId, ConsumableData));
	}

	for (auto& Filed : Document["Material"].GetArray())
	{
		int DataSheetId = Filed["DataSheetId"].GetInt();
		string ItemName = Filed["ItemName"].GetString();
		int MaxCount = Filed["MaxCount"].GetInt();
		bool IsEquipped = Filed["IsEquipped"].GetBool();
		string ImageFilePath = Filed["ImageFilePath"].GetString();

		st_MaterialData* MaterialData = new st_MaterialData();
		MaterialData->DataSheetId = DataSheetId;
		MaterialData->ItemName = ItemName;

		ItemName.assign(MaterialData->ItemName.begin(), MaterialData->ItemName.end());
		if (ItemName == "가죽")
		{
			MaterialData->ItemType = en_ItemType::ITEM_TYPE_LEATHER;
		}
		else if (ItemName == "슬라임젤리")
		{
			MaterialData->ItemType = en_ItemType::ITEM_TYPE_SLIMEGEL;
		}
		else if (ItemName == "동전")
		{
			MaterialData->ItemType = en_ItemType::ITEM_TYPE_BRONZE_COIN;
		}
		else if (ItemName == "암석")
		{
			MaterialData->ItemType = en_ItemType::ITEM_TYPE_STONE;
		}
		else if (ItemName == "통나무")
		{
			MaterialData->ItemType = en_ItemType::ITEM_TYPE_WOOD_LOG;
		}
		else if (ItemName == "나무판자")
		{
			MaterialData->ItemType = en_ItemType::ITEM_TYPE_WOOD_FLANK;
		}
		else if (ItemName == "실")
		{
			MaterialData->ItemType = en_ItemType::ITEM_TYPE_YARN;
		}

		MaterialData->ItemCategory = en_ItemCategory::ITEM_CATEGORY_MATERIAL;

		MaterialData->ThumbnailImagePath = ImageFilePath;
		MaterialData->_MaxCount = MaxCount;

		_Items.insert(pair<int32, st_MaterialData*>(MaterialData->DataSheetId, MaterialData));
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

		for (auto& DropDataFiled : Filed["DropData"].GetArray())
		{
			int Probability = DropDataFiled["Probability"].GetInt();
			int ItemDataSheetId = DropDataFiled["ItemDataSheetId"].GetInt();
			int8 MinCount = (int8)(DropDataFiled["MinCount"].GetInt());
			int16 MaxCount = (int16)(DropDataFiled["MaxCount"].GetInt());

			st_DropData DropData;
			DropData.Probability = Probability;
			DropData.ItemDataSheetId = ItemDataSheetId;
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
			int ItemDataSheetId = DropDataFiled["ItemDataSheetId"].GetInt();
			int8 MinCount = (int8)(DropDataFiled["MinCount"].GetInt());
			int16 MaxCount = (int16)(DropDataFiled["MaxCount"].GetInt());

			st_DropData DropData;
			DropData.Probability = Probability;
			DropData.ItemDataSheetId = ItemDataSheetId;
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

		int CraftingDataId = Filed["CraftingDataId"].GetInt();
		string CraftingTypeName = Filed["CraftingTypeName"].GetString();

		CraftingData->CraftingDataId = CraftingDataId;
		CraftingData->CraftingTypeName = CraftingTypeName;

		if (CraftingTypeName == "무기")
		{
			CraftingData->CraftingType = en_ItemCategory::ITEM_CATEGORY_WEAPON;
		}
		else if (CraftingTypeName == "방어구")
		{
			CraftingData->CraftingType = en_ItemCategory::ITEM_CATEGORY_ARMOR;
		}
		else if (CraftingTypeName == "음식")
		{
			CraftingData->CraftingType = en_ItemCategory::ITEM_CATEGORY_FOOD;
		}
		else if (CraftingTypeName == "물약")
		{
			CraftingData->CraftingType = en_ItemCategory::ITEM_CATEGORY_POTION;
		}
		else if (CraftingTypeName == "재료")
		{
			CraftingData->CraftingType = en_ItemCategory::ITEM_CATEGORY_MATERIAL;
		}

		for (auto& CraftingCompleteFiled : Filed["CraftingCompleteItem"].GetArray())
		{
			st_CraftingCompleteItemData CraftingCompleteItemData;

			int16 CraftingCompleteItemDataId = (int16)CraftingCompleteFiled["CraftingCompleteItemDataId"].GetInt();
			string CraftingCompleteItemName = CraftingCompleteFiled["CraftingCompleteItemName"].GetString();
			string CraftingCompleteItemThumbnailImagePath = CraftingCompleteFiled["CraftingCompleteItemThumbnailImagePath"].GetString();

			CraftingCompleteItemData.CraftingCompleteItemDataId = (en_ItemType)CraftingCompleteItemDataId;
			CraftingCompleteItemData.CraftingCompleteName = CraftingCompleteItemName;
			CraftingCompleteItemData.CraftingCompleteThumbnailImagePath = CraftingCompleteItemThumbnailImagePath;

			for (auto& CraftingMaterialFiled : CraftingCompleteFiled["CraftingMaterial"].GetArray())
			{
				st_CraftingMaterialItemData CraftingMaterialData;

				int16 MaterialDataId = (int16)CraftingMaterialFiled["MaterialDataId"].GetInt();
				string MaterialName = CraftingMaterialFiled["MaterialName"].GetString();
				int16 MaterialCount = (int16)CraftingMaterialFiled["MaterialCount"].GetInt();
				string MaterialThumbnailImagePath = CraftingMaterialFiled["MaterialThumbnailImagePath"].GetString();

				CraftingMaterialData.MaterialDataId = (en_ItemType)(MaterialDataId);
				CraftingMaterialData.MaterialName = MaterialName;
				CraftingMaterialData.MaterialCount = MaterialCount;
				CraftingMaterialData.MaterialThumbnailImagePath = MaterialThumbnailImagePath;

				CraftingCompleteItemData.CraftingMaterials.push_back(CraftingMaterialData);
			}

			CraftingData->CraftingCompleteItems.push_back(CraftingCompleteItemData);
		}

		_CraftingData.insert(pair<int32, st_CraftingItemCategoryData*>(CraftingData->CraftingDataId, CraftingData));
	}
}
