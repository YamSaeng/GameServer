#pragma once
#include "GameObject.h"
#include "InventoryManager.h"

class CStorageBox : public CGameObject
{
public:
	CStorageBox();
	~CStorageBox();

	virtual void Update() override;

	//--------------------------------------------------------------
	// ���� â�� ������ �ְ� ����
	//--------------------------------------------------------------
	void StorageBoxInsertItem(CItem* InsertItem);	
	CItem* StorageBoxGetItem(int8 TilePositionX, int8 TilePositionY);

	//--------------------------------------------------------------
	// ���� â�� �� ���� ã��
	//--------------------------------------------------------------
	st_Vector2Int StorageBoxFindEmptySpace(CItem* Item);
	//--------------------------------------------------------------
	// ���� â�� ������ ����
	//--------------------------------------------------------------
	void StorageBoxInitItem(int8 TilePositionX, int8 TilePositionY);

	//--------------------------------------------------------------
	// ���� â�� ������ ����
	//--------------------------------------------------------------
	CItem* SelectItem(int8 TilePositionX, int8 TilePositionY);

	//--------------------------------------------------------------
	// ���� â�� ������ ����
	//--------------------------------------------------------------
	CItem* SwapItem(int8 PlaceItemTilePositionX, int8 PlaceItemTilePositionY);

private:
	CInventoryManager _StorageBoxInventory;
};