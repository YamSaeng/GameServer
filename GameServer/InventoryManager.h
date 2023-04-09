#pragma once
#include "Inventory.h"

class CInventoryManager
{
public:	
	CItem* _SelectItem;		

	CInventoryManager();
	~CInventoryManager();

	//-----------------------------------------------------------------
	// Inventory ���� ��ȯ
	//-----------------------------------------------------------------
	int8 GetInventoryCount();
	//-----------------------------------------------------------------
	// �������� Inventory ��� ��ȯ
	//-----------------------------------------------------------------
	CInventory** GetInventoryManager();

	//-----------------------------------------------------------------
	// Inventory ����
	//-----------------------------------------------------------------
	void InventoryCreate(int8 InventoryCount, int8 Width, int8 Height);		

	//-----------------------------------------------------------------
	// �ֿ� �� ������ Inventory�� �ֱ�
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
	void DBMoneyInsert(int64 GoldCoin, int16 SliverCoin, int16 BronzeCoin);

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
	Vector2Int FindEmptySpace(int8 SelectInventoryIndex, CItem* ItemInfo);

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

	//--------------------------------------
	// �� ��ȯ
	//--------------------------------------
	int64 GetGoldCoin();
	int16 GetSliverCoin();
	int16 GetBronzeCoin();
private:
	// �����ϴ� Inventory ����
	int8 _InventoryCount;

	// �����ϰ� �ִ� ����, ����, ����
	int16 _BronzeCoinCount;
	int16 _SliverCoinCount;
	int64 _GoldCoinCount;

	// �����ϴ� Inventory ���
	CInventory** _Inventorys;
};