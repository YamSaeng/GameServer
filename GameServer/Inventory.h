#pragma once
#include "Item.h"
#include "LockFreeStack.h"

#define INVENTORY_SIZE 30

class CInventory
{
private:
	//---------------------------------------------
	// �κ��丮�� �������� ������ ( �����ͷ� ���� )
	//---------------------------------------------
	CItem** _Items;	
	
	//------------------------------------
	// �κ��丮 �󽽷��� ������ ����
	//------------------------------------	
	CLockFreeStack<int8> _ItemsSlotIndex;		
public:
	//-------------------
	// �����ϰ� �ִ� ����
	//-------------------
	int8 _BronzeCoinCount;
	int8 _SliverCoinCount;
	int64 _GoldCoinCount;

	CInventory();
	~CInventory();
	
	//------------------------
	// �κ��丮�� Item �߰�
	//------------------------
	void AddItem(CItem* Item);	
	//------------------------
	// �κ��丮�� Coin �߰�
	//------------------------
	void AddCoin(CItem* Item);
	//--------------------------
	// ������ ��ȯ
	//--------------------------
	CItem* Get(int32 _ItemDBId);
	
	//-------------------------------------
	// �κ��丮�� �̹� �������� �ִ��� Ȯ��
	//-------------------------------------
	bool IsExistItem(en_ItemType ItemType, int16* Count, int8* SlotIndex);
	//----------------------------------
	// ��� �ִ� ���� ��ȯ
	//----------------------------------
	bool GetEmptySlot(int8* SlotIndex);
};

