#pragma once
#include "Inventory.h"

class CInventoryManager
{
private:
	enum en_InventoryManager
	{
		INVENTORY_COUNT = 1
	};

public:	
	CItem* _SelectItem;	

	//---------------------
	// 보유하고 있는 동전
	//---------------------
	int16 _BronzeCoinCount;
	int16 _SliverCoinCount;
	int64 _GoldCoinCount;

	CInventory* _Inventorys[en_InventoryManager::INVENTORY_COUNT];	

	CInventoryManager();
	~CInventoryManager();

	void InventoryCreate(int8 Width, int8 Height);		

	//--------------------------------------------------------------
	// 루팅한 아이템 인벤토리에 순서대로 넣기
	//--------------------------------------------------------------
	void InsertItem(int8 SelectInventoryIndex, CItem* NewItem);
	
	//--------------------------------------------------------------
	// DB에 기록된 아이템 인벤토리에 넣기
	//--------------------------------------------------------------
	void DBItemInsertItem(int8 SelectInventoryIndex, CItem* NewItem);

	//----------------------------------------------------------------------------------------
	// 아이템 선택
	//----------------------------------------------------------------------------------------
	CItem* SelectItem(int8 SelectInventoryIndex, int8 TilePositionX, int8 TilePositionY);

	//-----------------------------------------------------------------------------------------------------------
	// 아이템 스왑 
	//-----------------------------------------------------------------------------------------------------------
	CItem* SwapItem(int8 SelectInventoryIndex, int16 PlaceItemTileGridPositionX, int16 PlaceItemTileGridPositionY);

	//-------------------------------------------------------------------------------------
	// 아이템 얻기
	//-------------------------------------------------------------------------------------
	CItem* GetItem(int8 SelectInventoryIndex, int8 TilePositionX, int8 TilePositionY);

	//-----------------------------------------------------------------------------
	// 인벤토리 빈 공간 찾기
	//-----------------------------------------------------------------------------
	st_Vector2Int FindEmptySpace(int8 SelectInventoryIndex, CItem* ItemInfo);

	//-------------------------------------------------------------------------------------------------
	// 인벤토리에 아이템을 찾는다.
	//-------------------------------------------------------------------------------------------------
	CItem* FindInventoryItem(int8 SelectInventoryIndex, en_SmallItemCategory FindItemSmallItemCategory);

	//------------------------------------------------------------------------------------------------------------
	// 인벤토리에 조건에 맞는 아이템을 모두 찾는다.
	//------------------------------------------------------------------------------------------------------------
	vector<CItem*> FindAllInventoryItem(int8 SelectInventoryIndex, en_SmallItemCategory FindItemSmallItemCategory);
};