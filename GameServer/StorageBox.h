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
	// 상자 창고에 아이템 넣고 빼기
	//--------------------------------------------------------------
	void StorageBoxInsertItem(CItem* InsertItem);	
	CItem* StorageBoxGetItem(int8 TilePositionX, int8 TilePositionY);

	//--------------------------------------------------------------
	// 상자 창고에 빈 공간 찾기
	//--------------------------------------------------------------
	st_Vector2Int StorageBoxFindEmptySpace(CItem* Item);
	//--------------------------------------------------------------
	// 상자 창고 아이템 비우기
	//--------------------------------------------------------------
	void StorageBoxInitItem(int8 TilePositionX, int8 TilePositionY);

	//--------------------------------------------------------------
	// 상자 창고 아이템 선택
	//--------------------------------------------------------------
	CItem* SelectItem(int8 TilePositionX, int8 TilePositionY);

	//--------------------------------------------------------------
	// 상자 창고 아이템 스왑
	//--------------------------------------------------------------
	CItem* SwapItem(int8 PlaceItemTilePositionX, int8 PlaceItemTilePositionY);

private:
	CInventoryManager _StorageBoxInventory;
};