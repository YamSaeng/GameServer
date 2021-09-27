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
		string ItemName = Filed["Name"].GetString();
		string Type = Filed["WeaponType"].GetString();
		int Damage = Filed["Damage"].GetInt();
		bool IsEquipped = Filed["IsEquipped"].GetBool();
		string ImageFilePath = Filed["ImageFilePath"].GetString();

		st_WeaponData* WeaponData = new st_WeaponData();
		WeaponData->DataSheetId = DataSheetId;
		WeaponData->ItemName = ItemName;

		if (Type == "Sword")
		{
			WeaponData->ItemType = en_ItemType::ITEM_TYPE_WEAPON_SWORD;
		}		
		
		WeaponData->ItemConsumableType = en_ConsumableType::NONE;

		WeaponData->ThumbnailImagePath = ImageFilePath;
		WeaponData->_Damage = Damage;

		_Items.insert(pair<int32, st_WeaponData*>(WeaponData->DataSheetId, WeaponData));
	}

	for (auto& Filed : Document["Armors"].GetArray())
	{
		int DataSheetId = Filed["DataSheetId"].GetInt();
		string ItemName = Filed["Name"].GetString();
		string Type = Filed["ArmorType"].GetString();
		int Defence = Filed["Defence"].GetInt();
		bool IsEquipped = Filed["IsEquipped"].GetBool();
		string ImageFilePath = Filed["ImageFilePath"].GetString();

		st_ArmorData* ArmorData = new st_ArmorData();
		ArmorData->DataSheetId = DataSheetId;
		ArmorData->ItemName = ItemName;

		if (Type == "Armor")
		{
			ArmorData->ItemType = en_ItemType::ITEM_TYPE_ARMOR_ARMOR;
		}
		else if (Type == "Helmet")
		{
			ArmorData->ItemType = en_ItemType::ITEM_TYPE_ARMOR_HELMET;
		}

		ArmorData->ItemConsumableType = en_ConsumableType::NONE;

		ArmorData->ThumbnailImagePath = ImageFilePath;
		ArmorData->_Defence = Defence;

		_Items.insert(pair<int32, st_ArmorData*>(ArmorData->DataSheetId, ArmorData));
	}

	for (auto& Filed : Document["Consumables"].GetArray())
	{
		int DataSheetId = Filed["DataSheetId"].GetInt();
		string ItemName = Filed["Name"].GetString();
		string Type = Filed["ConsumableType"].GetString();
		int MaxCount = Filed["MaxCount"].GetInt();
		bool IsEquipped = Filed["IsEquipped"].GetBool();
		string ImageFilePath = Filed["ImageFilePath"].GetString();

		st_ConsumableData* ConsumableData = new st_ConsumableData();
		ConsumableData->DataSheetId = DataSheetId;
		ConsumableData->ItemName = ItemName;

		ItemName.assign(ConsumableData->ItemName.begin(), ConsumableData->ItemName.end());
		if (Type == "Potion")
		{
			ConsumableData->ItemType = en_ItemType::ITEM_TYPE_CONSUMABLE_POTION;
			ConsumableData->ItemConsumableType = en_ConsumableType::POTION;
		}
		else if (Type == "SkillBook")
		{
			ConsumableData->ItemType = en_ItemType::ITEM_TYPE_SKILL_BOOK;
			ConsumableData->ItemConsumableType = en_ConsumableType::SKILL_BOOK;
		}

		ConsumableData->ThumbnailImagePath = ImageFilePath;
		ConsumableData->_MaxCount = MaxCount;

		_Items.insert(pair<int32, st_ConsumableData*>(ConsumableData->DataSheetId, ConsumableData));
	}

	for (auto& Filed : Document["Material"].GetArray())
	{
		int DataSheetId = Filed["DataSheetId"].GetInt();
		string ItemName = Filed["Name"].GetString();		
		int MaxCount = Filed["MaxCount"].GetInt();
		bool IsEquipped = Filed["IsEquipped"].GetBool();
		string ImageFilePath = Filed["ImageFilePath"].GetString();

		st_MaterialData* MaterialData = new st_MaterialData();
		MaterialData->DataSheetId = DataSheetId;
		MaterialData->ItemName = ItemName;

		ItemName.assign(MaterialData->ItemName.begin(), MaterialData->ItemName.end());
		if (ItemName == "°õ °¡Á×")
		{
			MaterialData->ItemType = en_ItemType::ITEM_TYPE_LEATHER;
		}
		else if (ItemName == "½½¶óÀÓÁ©¸®")
		{
			MaterialData->ItemType = en_ItemType::ITEM_TYPE_SLIMEGEL;
		}
		else if (ItemName == "µ¿Àü")
		{
			MaterialData->ItemType = en_ItemType::ITEM_TYPE_BRONZE_COIN;
		}
		else if (ItemName == "Åë³ª¹«")
		{
			MaterialData->ItemType = en_ItemType::ITEM_TYPE_WOOD_LOG;
		}

		MaterialData->ItemConsumableType = en_ConsumableType::NONE;		

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

		_Monsters.insert(pair<int32, st_MonsterData*>(MonsterData->MonsterDataId,MonsterData));
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
		float SkillCastingTime = Filed["SkillCastingTime"].GetFloat();
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
