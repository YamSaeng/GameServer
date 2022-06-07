#include "pch.h"
#include "Furnace.h"
#include "Item.h"
#include "ObjectManager.h"
#include "DataManager.h"

CFurnace::CFurnace()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_FURNACE;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	_GameObjectInfo.ObjectName = L"�뱤��";

	auto FindFurnaceData = G_Datamanager->_CraftingTableData.find((int16)_GameObjectInfo.ObjectType);
	_FurnaceCraftingTable = *(*FindFurnaceData).second;	
}

map<en_SmallItemCategory, CItem*> CFurnace::GetMaterialItems()
{
	return _MaterialItems;
}

void CFurnace::InputMaterialItem(CItem* MaterialItem, int16 MaterialItemCount)
{
	// �켱 �뱤�� ��� �۵� �߿� ���� �������� �����ϴ��� Ȯ��
	auto FindMaterialIter = _MaterialItems.find(MaterialItem->_ItemInfo.ItemSmallCategory);
	// �߰����� ������ ���
	if (FindMaterialIter == _MaterialItems.end())
	{		
		// ������ ����
		CItem* NewMaterialItem = G_ObjectManager->ItemCreate(MaterialItem->_ItemInfo.ItemSmallCategory);	
		if (NewMaterialItem != nullptr)
		{
			NewMaterialItem->_ItemInfo.ItemDBId = 0;
			NewMaterialItem->_ItemInfo.InventoryItemNumber = 0;
			NewMaterialItem->_ItemInfo.ItemIsQuickSlotUse = false;
			NewMaterialItem->_ItemInfo.Rotated = false;
			NewMaterialItem->_ItemInfo.Width = 1;
			NewMaterialItem->_ItemInfo.Height = 1;
			NewMaterialItem->_ItemInfo.TileGridPositionX = MaterialItem->_ItemInfo.TileGridPositionX;
			NewMaterialItem->_ItemInfo.TileGridPositionY = MaterialItem->_ItemInfo.TileGridPositionY;
			NewMaterialItem->_ItemInfo.ItemLargeCategory = MaterialItem->_ItemInfo.ItemLargeCategory;
			NewMaterialItem->_ItemInfo.ItemMediumCategory = MaterialItem->_ItemInfo.ItemMediumCategory;
			NewMaterialItem->_ItemInfo.ItemSmallCategory = MaterialItem->_ItemInfo.ItemSmallCategory;
			NewMaterialItem->_ItemInfo.ItemName = MaterialItem->_ItemInfo.ItemName;
			NewMaterialItem->_ItemInfo.ItemExplain = MaterialItem->_ItemInfo.ItemExplain;
			NewMaterialItem->_ItemInfo.ItemMaxCount = MaterialItem->_ItemInfo.ItemMaxCount;
			NewMaterialItem->_ItemInfo.ItemCount = MaterialItemCount;
			NewMaterialItem->_ItemInfo.ItemThumbnailImagePath = MaterialItem->_ItemInfo.ItemThumbnailImagePath;
			NewMaterialItem->_ItemInfo.ItemIsEquipped = MaterialItem->_ItemInfo.ItemIsEquipped;

			_MaterialItems.insert(pair<en_SmallItemCategory, CItem*>(NewMaterialItem->_ItemInfo.ItemSmallCategory, NewMaterialItem));
		}		
	}	
	else
	{
		// �߰� ���� ��� ������ �÷���
		(*FindMaterialIter).second->_ItemInfo.ItemCount += MaterialItemCount;
	}
}

void CFurnace::CraftingStart()
{

}
