#include "pch.h"
#include "StorageBox.h"

CStorageBox::CStorageBox()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_STORAGE_BOX;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	_FieldOfViewDistance = 10;

	_StorageBoxInventory.InventoryCreate(1, 10, 10);
}

CStorageBox::~CStorageBox()
{
}

void CStorageBox::Update()
{

}

void CStorageBox::StorageBoxInsertItem(CItem* InsertItem)
{
	_StorageBoxInventory.InsertItem(0, InsertItem);
}

CItem* CStorageBox::StorageBoxGetItem(int8 TilePositionX, int8 TilePositionY)
{
	return _StorageBoxInventory.GetItem(0, TilePositionX, TilePositionY);
}

st_Vector2Int CStorageBox::StorageBoxFindEmptySpace(CItem* Item)
{
	return _StorageBoxInventory.FindEmptySpace(0, Item);
}

void CStorageBox::StorageBoxInitItem(int8 TilePositionX, int8 TilePositionY)
{
	_StorageBoxInventory.InitItem(0, TilePositionX, TilePositionY);
}

CItem* CStorageBox::SelectItem(int8 TilePositionX, int8 TilePositionY)
{
	return _StorageBoxInventory.SelectItem(0, TilePositionX, TilePositionY);
}

CItem* CStorageBox::SwapItem(int8 PlaceItemTilePositionX, int8 PlaceItemTilePositionY)
{
	return _StorageBoxInventory.SwapItem(0, PlaceItemTilePositionX, PlaceItemTilePositionY);
}