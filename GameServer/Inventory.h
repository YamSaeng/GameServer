#pragma once
#include "Item.h"

class CInventory
{
private:
	//---------------------------------------------
	// 인벤토리가 소유중인 아이템 ( 포인터로 관리 )
	//   ( SlotIndex, 아이템정보 )
	//---------------------------------------------
	map<byte, st_ItemInfo*> _Items;			
public:	

	//-------------------
	// 보유하고 있는 동전
	//-------------------
	int16 _BronzeCoinCount;
	int16 _SliverCoinCount;
	int64 _GoldCoinCount;

	CInventory();
	~CInventory();
	
	void Init();
	//------------------------
	// 인벤토리에 Item 추가
	//------------------------
	void AddItem(st_ItemInfo& ItemInfo);	
	//------------------------
	// 인벤토리에 Coin 추가
	//------------------------
	void AddCoin(CItem* Item);
	//--------------------------
	// 아이템 반환
	//--------------------------
	st_ItemInfo* Get(int8 SlotIndex);
	
	//-----------------------------------------------------------------------
	// 인벤토리에 이미 아이템이 있는지 확인 ( 아이템 인벤토리에 넣을때 확인 )
	//-----------------------------------------------------------------------
	bool IsExistItem(en_ItemType ItemType, int16* ItemCount, int16& ItemEach,int8* SlotIndex);

	//--------------------------------------
	// ItemType 받아서 아이템정보 반환
	//--------------------------------------
	vector<st_ItemInfo*> Find(en_ItemType ItemType);

	//----------------------------------
	// 비어 있는 슬롯 반환
	//----------------------------------
	bool GetEmptySlot(int8* SlotIndex);
	void SwapItem(st_ItemInfo& SwapAItemInfo, st_ItemInfo& SwapBItemInfo);
};

