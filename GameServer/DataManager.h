#pragma once
#include "Data.h"
#include "FileUtils.h"
#undef min
#undef max
#include "rapidjson/document.h"

class CDataManager
{
public:
	map<int32, st_ItemData*>* _Items = nullptr;

	CDataManager()
	{
		_Items = new map<int32, st_ItemData*>();
	}

	~CDataManager()
	{
		
	}

	map<int32,st_ItemData*>* LoadData(wstring LoadFileName)
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
			string ImageFilePath = Filed["ImageFilePath"].GetString();
						
			st_WeaponData* WeaponData = new st_WeaponData();
			WeaponData->_DataSheetId = DataSheetId;						
			WeaponData->_Name = ItemName;

			if (Type == "Sword")
			{
				WeaponData->_ItemType = en_ItemType::ITEM_TYPE_WEAPON_SWORD;
			}
			else if (Type == "Bow")
			{
				WeaponData->_ItemType = en_ItemType::ITEM_TYPE_WEAPON_BOW;
			}

			WeaponData->_ImagePath = ImageFilePath;
			WeaponData->_Damage = Damage;
			
			_Items->insert(pair<int32, st_WeaponData*>(WeaponData->_DataSheetId,WeaponData));
		}

		auto a = _Items->find(1);
		st_WeaponData* Weapon = (st_WeaponData * )(a->second);
		

		for (auto& Filed : Document["Armors"].GetArray())
		{
			int DataSheetId = Filed["DataSheetId"].GetInt();
			string ItemName = Filed["Name"].GetString();
			string Type = Filed["ArmorType"].GetString();
			int Defence = Filed["Defence"].GetInt();
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

			_Items->insert(pair<int32, st_ArmorData*>(ArmorData->_DataSheetId, ArmorData));
		}

		for (auto& Filed : Document["Armors"].GetArray())
		{
			int DataSheetId = Filed["DataSheetId"].GetInt();
			string ItemName = Filed["Name"].GetString();
			string Type = Filed["ConsumableType"].GetString();
			int MaxCount = Filed["MaxCount"].GetInt();
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
			
			_Items->insert(pair<int32, st_ConsumableData*>(ConsumableData->_DataSheetId, ConsumableData));
		}

		return _Items;
	}	
};

