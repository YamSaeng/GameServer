#pragma once
#include "Item.h"

class CInventory
{
private:
	//---------------------------------------------
	// �κ��丮�� �������� ������ ( �����ͷ� ���� )
	//   ( SlotIndex, ���������� )
	//---------------------------------------------
	map<byte, st_ItemInfo*> _Items;			
public:	

	//-------------------
	// �����ϰ� �ִ� ����
	//-------------------
	int16 _BronzeCoinCount;
	int16 _SliverCoinCount;
	int64 _GoldCoinCount;

	CInventory();
	~CInventory();
	
	void Init();
	//------------------------
	// �κ��丮�� Item �߰�
	//------------------------
	void AddItem(st_ItemInfo& ItemInfo);	
	//------------------------
	// �κ��丮�� Coin �߰�
	//------------------------
	void AddCoin(CItem* Item);
	//--------------------------
	// ������ ��ȯ
	//--------------------------
	st_ItemInfo* Get(int8 SlotIndex);
	
	//-----------------------------------------------------------------------
	// �κ��丮�� �̹� �������� �ִ��� Ȯ�� ( ������ �κ��丮�� ������ Ȯ�� )
	//-----------------------------------------------------------------------
	bool IsExistItem(en_ItemType ItemType, int16* ItemCount, int16& ItemEach,int8* SlotIndex);

	//--------------------------------------
	// ItemType �޾Ƽ� ���������� ��ȯ
	//--------------------------------------
	vector<st_ItemInfo*> Find(en_ItemType ItemType);

	//----------------------------------
	// ��� �ִ� ���� ��ȯ
	//----------------------------------
	bool GetEmptySlot(int8* SlotIndex);
	void SwapItem(st_ItemInfo& SwapAItemInfo, st_ItemInfo& SwapBItemInfo);
};

