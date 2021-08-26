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
		WeaponData->_DataSheetId = DataSheetId;
		WeaponData->_Name = ItemName;

		if (Type == "Sword")
		{
			WeaponData->_ItemType = en_ItemType::ITEM_TYPE_WEAPON_SWORD;
		}		

		WeaponData->_ImagePath = ImageFilePath;
		WeaponData->_Damage = Damage;

		_Items.insert(pair<int32, st_WeaponData*>(WeaponData->_DataSheetId, WeaponData));
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
		ArmorData->_DataSheetId = DataSheetId;
		ArmorData->_Name = ItemName;

		if (Type == "Armor")
		{
			ArmorData->_ItemType = en_ItemType::ITEM_TYPE_ARMOR_ARMOR;
		}
		else if (Type == "Helmet")
		{
			ArmorData->_ItemType = en_ItemType::ITEM_TYPE_ARMOR_HELMET;
		}

		ArmorData->_ImagePath = ImageFilePath;
		ArmorData->_Defence = Defence;

		_Items.insert(pair<int32, st_ArmorData*>(ArmorData->_DataSheetId, ArmorData));
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
		ConsumableData->_DataSheetId = DataSheetId;
		ConsumableData->_Name = ItemName;

		ItemName.assign(ConsumableData->_Name.begin(), ConsumableData->_Name.end());
		if (Type == "Potion")
		{
			ConsumableData->_ItemType = en_ItemType::ITEM_TYPE_CONSUMABLE_POTION;
		}
		ConsumableData->_ImagePath = ImageFilePath;
		ConsumableData->_MaxCount = MaxCount;

		_Items.insert(pair<int32, st_ConsumableData*>(ConsumableData->_DataSheetId, ConsumableData));
	}

	for (auto& Filed : Document["Material"].GetArray())
	{
		int DataSheetId = Filed["DataSheetId"].GetInt();
		string ItemName = Filed["Name"].GetString();		
		int MaxCount = Filed["MaxCount"].GetInt();
		bool IsEquipped = Filed["IsEquipped"].GetBool();
		string ImageFilePath = Filed["ImageFilePath"].GetString();

		st_MaterialData* MaterialData = new st_MaterialData();
		MaterialData->_DataSheetId = DataSheetId;
		MaterialData->_Name = ItemName;

		ItemName.assign(MaterialData->_Name.begin(), MaterialData->_Name.end());
		if (ItemName == "Leather")
		{
			MaterialData->_ItemType = en_ItemType::ITEM_TYPE_LEATHER;
		}
		else if (ItemName == "SlimeGel")
		{
			MaterialData->_ItemType = en_ItemType::ITEM_TYPE_SLIMEGEL;
		}
		else if (ItemName == "BronzeCoin")
		{
			MaterialData->_ItemType = en_ItemType::ITEM_TYPE_BRONZE_COIN;
		}

		MaterialData->_ImagePath = ImageFilePath;
		MaterialData->_MaxCount = MaxCount;

		_Items.insert(pair<int32, st_MaterialData*>(MaterialData->_DataSheetId, MaterialData));
	}
}

void CDataManager::LoadDataStatus(wstring LoadFileName)
{
	char* FileStr = FileUtils::LoadFile(LoadFileName.c_str());

	rapidjson::Document Document;
	Document.Parse(FileStr);

	for (auto& Filed : Document["Status"].GetArray())
	{
		int Level = Filed["Level"].GetInt();
		int MaxHP = Filed["MaxHP"].GetInt();
		int Attack = Filed["Attack"].GetInt();
		int CriticalPoint = Filed["CriticalPoint"].GetInt();
		float Speed = Filed["Speed"].GetFloat();		

		st_StatusData* StatusData = new st_StatusData();
		StatusData->Level = Level;
		StatusData->MaxHP = MaxHP;
		StatusData->Attack = Attack;
		StatusData->CriticalPoint = CriticalPoint;
		StatusData->Speed = Speed;

		_Status.insert(pair<int32, st_StatusData*>(StatusData->Level, StatusData));		
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
				
		MonsterData->_MonsterDataId = MonsterDataId;
		MonsterData->_MonsterName = Name;									

		for (auto& MonsterStatInfoFiled : Filed["MonsterStatInfo"].GetArray())
		{
			int Level = MonsterStatInfoFiled["Level"].GetInt();
			int MaxHP = MonsterStatInfoFiled["MaxHP"].GetInt();
			int Attack = MonsterStatInfoFiled["Attack"].GetInt();
			float Speed = MonsterStatInfoFiled["Speed"].GetFloat();
			int SearchCellDistance = MonsterStatInfoFiled["SearchCellDistance"].GetInt();
			int ChaseCellDistance = MonsterStatInfoFiled["ChaseCellDistance"].GetInt();
			int AttackRange = MonsterStatInfoFiled["AttackRange"].GetInt();
			int TotalExp = MonsterStatInfoFiled["TotalExp"].GetInt();

			MonsterData->_MonsterStatInfo.Level = Level;
			MonsterData->_MonsterStatInfo.MaxHP = MaxHP;
			MonsterData->_MonsterStatInfo.Attack = Attack;
			MonsterData->_MonsterStatInfo.Speed = Speed;
			MonsterData->_MonsterStatInfo.SearchCellDistance = SearchCellDistance;
			MonsterData->_MonsterStatInfo.ChaseCellDistance = ChaseCellDistance;
			MonsterData->_MonsterStatInfo.AttackRange = AttackRange;
		}

		for (auto& DropDataFiled : Filed["DropData"].GetArray())
		{
			int Probability = DropDataFiled["Probability"].GetInt();
			int ItemDataSheetId = DropDataFiled["ItemDataSheetId"].GetInt();
			int Count = DropDataFiled["Count"].GetInt();
						
			st_DropData DropData;
			DropData._Probability = Probability;
			DropData._ItemDataSheetId = ItemDataSheetId;
			DropData._Count = Count;

			MonsterData->_DropItems.push_back(DropData);
		}	

		_Monsters.insert(pair<int32, st_MonsterData*>(MonsterData->_MonsterDataId,MonsterData));
	}
}