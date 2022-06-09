#pragma once
#include "Item.h"

class CInventory
{
public:	
	// 인벤토리 번호
	int8 _InventoryIndex;
	int64 _InventoryItemNumber;
	// 인벤토리 너비
	int32 _InventoryWidth;
	// 인벤토리 높이
	int32 _InventoryHeight;		

	CInventory();
	~CInventory();
	
	//----------------------
	// 인벤토리 초기화
	//----------------------
	void Init(int8 InventoryWidth, int8 InventoryHeight);			

	//-----------------------------------------------------
	// 아이템 선택
	//-----------------------------------------------------
	CItem* SelectItem(int8 TilePositionX, int8 TilePositionY);

	//-------------------------------------------------
	// 아이템이 위치한 인벤토리 정리
	//-------------------------------------------------
	void CleanGridReference(CItem* CleanItem);

	//----------------------------------
	// 아이템 반환
	//----------------------------------
	CItem* GetItem(int8 X, int8 Y);
	
	//--------------------------------------------------
	// 인벤토리에서 비어 있는 공간을 찾아낸다.
	//--------------------------------------------------
	st_Vector2Int FindEmptySpace(CItem* ItemInfo);

	//---------------------------------------------------
	// 인벤토리에 아이템이 차지할 위치가 비어 있는지 확인
	//---------------------------------------------------
	bool FindItemSpaceEmpty(CItem* Item);

	//-----------------------------------------------------------------------------
	// 매개 변수로 받은 공간이 비어 있는지 확인한다.
	//-----------------------------------------------------------------------------
	bool CheckEmptySpace(int8 PositionX, int8 PositionY, int32 Width, int32 Height);

	//------------------------------------------------------------------------------------------------
	// 아이템을 인벤토리에 넣는다. ( 아이템 범위 체크, 중복 아이템 체크 )
	//------------------------------------------------------------------------------------------------
	bool PlaceItem(CItem* PlaceItemInfo, int16 PositionX, int16 PositionY, CItem** OverlapItem);
	void PlaceItem(CItem* PlaceItemInfo, int16 PositionX, int16 PositionY);	

	//----------------------------------------------------
	// 매개변수로 받은 위치 아이템 초기화
	//----------------------------------------------------
	void InitItem(int8 TilePositionX, int8 TilePositionY);

	//----------------------------------------------------------------------------------------
	// 아이템의 Grid 인벤토리 위치를 계산한다.
	//----------------------------------------------------------------------------------------
	st_Vector2Int CalculatePositionOnGrid(CItem* Item, int8 TilePositionX, int8 TilePositionY);

	//---------------------------------------------------------------------------
	// 범위 체크
	//---------------------------------------------------------------------------
	bool BoundryCheck(int16 PositionX, int16 PositionY, int32 Width, int32 Height);
	
	//------------------------------------------------
	// 위치 검사
	//------------------------------------------------
	bool PositionCheck(int16 TilePositionX, int16 TilePositionY);

	//------------------------------------------------------------------------------------------------------
	// 아이템을 놓을 위치에 이미 아이템이 있는지 확인
	//------------------------------------------------------------------------------------------------------
	bool OverlapCheck(int8 TilePositionX, int8 TilePositionY, int16 Width, int16 Height, CItem** OverlapItem);		
	
	//----------------------------------------------------------------------
	// ItemType 받아서 해당 타입과 맞는 첫번째로 발견한 아이템 정보 반환
	//----------------------------------------------------------------------
	CItem* FindInventoryItem(en_SmallItemCategory FindItemSmallItemCategory);

	//---------------------------------------------------------------------------------
	// ItemType 받아서 해당 타입과 맞는 모든 아이템 정보 반환
	//---------------------------------------------------------------------------------
	vector<CItem*> FindAllInventoryItem(en_SmallItemCategory FindItemSmallItemCategory);

	//-----------------------------------------------------
	// 서버에 저장하기 위해 소유중인 아이템 정보를 반환한다.
	//-----------------------------------------------------
	vector<st_ItemInfo> DBInventorySaveReturnItems();
private:
	struct st_InventoryItem
	{
		bool IsEmptySlot;
		CItem* InventoryItem;
	};

	enum en_Inventory
	{
		TILE_SIZE_WIDTH = 64,
		TILE_SIZE_HEIGHT = 64
	};

	// 보유중인 아이템 목록
	st_InventoryItem*** _Items;
};

