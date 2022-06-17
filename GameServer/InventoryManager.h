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
	// �����ϰ� �ִ� ����
	//---------------------
	int16 _BronzeCoinCount;
	int16 _SliverCoinCount;
	int64 _GoldCoinCount;

	CInventory* _Inventorys[en_InventoryManager::INVENTORY_COUNT];	

	CInventoryManager();
	~CInventoryManager();

	void InventoryCreate(int8 Width, int8 Height);		

	//-----------------------------------------------------------------
	// �ֿ� �� ������ �κ��丮�� �ֱ�
	//-----------------------------------------------------------------
	void InsertMoney(int8 SelectInventoryIndex, CItem* InsertMoneyItem);
	//--------------------------------------------------------------
	// ������ ������ �κ��丮�� ������� �ֱ�
	//--------------------------------------------------------------
	void InsertItem(int8 SelectInventoryIndex, CItem* InsertNewItem);
	//--------------------------------------------------------------------
	// ������ ���濡 �ֱ�
	//--------------------------------------------------------------------
	CItem* InsertItem(int8 SelectInventoryIndex, en_SmallItemCategory InsertItemCategory, int16 InsertItemCount, bool *IsExistItem);
	
	//--------------------------------------------------------------
	// DB�� ��ϵ� ������ �κ��丮�� �ֱ�
	//--------------------------------------------------------------
	void DBItemInsertItem(int8 SelectInventoryIndex, CItem* NewItem);

	//----------------------------------------------------------------------------------------
	// ������ ����
	//----------------------------------------------------------------------------------------
	CItem* SelectItem(int8 SelectInventoryIndex, int8 TilePositionX, int8 TilePositionY);

	//-----------------------------------------------------------------------------------------------------------
	// ������ ���� 
	//-----------------------------------------------------------------------------------------------------------
	CItem* SwapItem(int8 SelectInventoryIndex, int16 PlaceItemTileGridPositionX, int16 PlaceItemTileGridPositionY);

	//-------------------------------------------------------------------------------------
	// ������ ���
	//-------------------------------------------------------------------------------------
	CItem* GetItem(int8 SelectInventoryIndex, int8 TilePositionX, int8 TilePositionY);

	//-----------------------------------------------------------------------------
	// �κ��丮 �� ���� ã��
	//-----------------------------------------------------------------------------
	st_Vector2Int FindEmptySpace(int8 SelectInventoryIndex, CItem* ItemInfo);

	//-------------------------------------------------------------------------------------------------
	// �κ��丮�� �������� ã�´�.
	//-------------------------------------------------------------------------------------------------
	CItem* FindInventoryItem(int8 SelectInventoryIndex, en_SmallItemCategory FindItemSmallItemCategory);

	//------------------------------------------------------------------------------
	// ������ �����
	//------------------------------------------------------------------------------
	void InitItem(int8 SelectInventoryIndex, int8 TilePositionX, int8 TilePositionY);

	//------------------------------------------------------------------------------------------------------------
	// �κ��丮�� ���ǿ� �´� �������� ��� ã�´�.
	//------------------------------------------------------------------------------------------------------------
	vector<CItem*> FindAllInventoryItem(int8 SelectInventoryIndex, en_SmallItemCategory FindItemSmallItemCategory);	
};