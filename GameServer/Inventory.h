#pragma once
#include "Item.h"
#include "LockFreeStack.h"

class CInventory
{
private:
	//---------------------------------------------
	// �κ��丮�� �������� ������ ( �����ͷ� ���� )
	//---------------------------------------------
	map<byte, st_ItemInfo*> _Items;			
public:	

	//-------------------
	// �����ϰ� �ִ� ����
	//-------------------
	int8 _BronzeCoinCount;
	int8 _SliverCoinCount;
	int64 _GoldCoinCount;

	CInventory();
	~CInventory();
	
	void Init();
	//------------------------
	// �κ��丮�� Item �߰�
	//------------------------
	void AddItem(int8 SlotIndex, CItem* Item);	
	//------------------------
	// �κ��丮�� Coin �߰�
	//------------------------
	void AddCoin(CItem* Item);
	//--------------------------
	// ������ ��ȯ
	//--------------------------
	st_ItemInfo* Get(int8 _SlotIndex);
	
	//-------------------------------------
	// �κ��丮�� �̹� �������� �ִ��� Ȯ��
	//-------------------------------------
	bool IsExistItem(en_ItemType ItemType, int16* Count, int8* SlotIndex);
	//----------------------------------
	// ��� �ִ� ���� ��ȯ
	//----------------------------------
	bool GetEmptySlot(int8* SlotIndex);
	void SwapItem(st_ItemInfo& SwapAItemInfo, st_ItemInfo& SwapBItemInfo);
};

