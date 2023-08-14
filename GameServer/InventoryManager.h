#pragma once
#include "Inventory.h"

class CInventoryManager
{
public:	
	CItem* _SelectItem;		

	CInventoryManager();
	~CInventoryManager();

	//-----------------------------------------------------------------
	// Inventory 개수 반환
	//-----------------------------------------------------------------
	int8 GetInventoryCount();
	//-----------------------------------------------------------------
	// 관리중인 Inventory 목록 반환
	//-----------------------------------------------------------------
	vector<CInventory*> GetInventoryManager();

	//-----------------------------------------------------------------
	// Inventory 생성
	//-----------------------------------------------------------------
	void InventoryCreate(int8 InventoryCount, int8 Width, int8 Height);		

	//-----------------------------------------------------------------
	// 주운 돈 아이템 Inventory에 넣기
	//-----------------------------------------------------------------
	void InsertMoney(int8 SelectInventoryIndex, CItem* InsertMoneyItem);
	//--------------------------------------------------------------
	// 루팅한 아이템 인벤토리에 순서대로 넣기
	//--------------------------------------------------------------
	void InsertItem(int8 SelectInventoryIndex, CItem* InsertNewItem);
	//--------------------------------------------------------------------
	// 아이템 가방에 넣기
	//--------------------------------------------------------------------
	CItem* InsertItem(int8 SelectInventoryIndex, en_SmallItemCategory InsertItemCategory, int16 InsertItemCount, bool *IsExistItem);
	
	//--------------------------------------------------------------
	// DB에 기록된 아이템 인벤토리에 넣기
	//--------------------------------------------------------------
	void DBItemInsertItem(int8 SelectInventoryIndex, CItem* NewItem);
	void DBMoneyInsert(int16 Coin);

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
	Vector2Int FindEmptySpace(int8 SelectInventoryIndex, CItem* ItemInfo);

	//-------------------------------------------------------------------------------------------------
	// 인벤토리에 아이템을 찾는다.
	//-------------------------------------------------------------------------------------------------
	CItem* FindInventoryItem(int8 SelectInventoryIndex, en_SmallItemCategory FindItemSmallItemCategory);

	//------------------------------------------------------------------------------
	// 아이템 지우기
	//------------------------------------------------------------------------------
	void InitItem(int8 SelectInventoryIndex, int8 TilePositionX, int8 TilePositionY);

	//------------------------------------------------------------------------------------------------------------
	// 인벤토리에 조건에 맞는 아이템을 모두 찾는다.
	//------------------------------------------------------------------------------------------------------------
	vector<CItem*> FindAllInventoryItem(int8 SelectInventoryIndex, en_SmallItemCategory FindItemSmallItemCategory);		

	//--------------------------------------
	// 돈 반환
	//--------------------------------------
	int16 GetCoin();	
private:
	// 관리하는 Inventory 개수
	int8 _InventoryCount;

	// 보유하고 돈
	int16 _Coin;	

	// 관리하는 Inventory 목록	
	vector<CInventory*> _Inventorys;
};