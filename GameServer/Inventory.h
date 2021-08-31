#pragma once
#include "Item.h"
#include "LockFreeStack.h"

#define INVENTORY_SIZE 30

class CInventory
{
private:
	//---------------------------------------------
	// 인벤토리가 소유중인 아이템 ( 포인터로 관리 )
	//---------------------------------------------
	CItem** _Items;	
	
	//------------------------------------
	// 인벤토리 빈슬롯을 보관할 스택
	//------------------------------------	
	CLockFreeStack<int8> _ItemsSlotIndex;		
public:
	//-------------------
	// 보유하고 있는 동전
	//-------------------
	int8 _BronzeCoinCount;
	int8 _SliverCoinCount;
	int64 _GoldCoinCount;

	CInventory();
	~CInventory();
	
	//------------------------
	// 인벤토리에 Item 추가
	//------------------------
	void AddItem(CItem* Item);	
	//------------------------
	// 인벤토리에 Coin 추가
	//------------------------
	void AddCoin(CItem* Item);
	//--------------------------
	// 아이템 반환
	//--------------------------
	CItem* Get(int32 _ItemDBId);
	
	//-------------------------------------
	// 인벤토리에 이미 아이템이 있는지 확인
	//-------------------------------------
	bool IsExistItem(en_ItemType ItemType, int16* Count, int8* SlotIndex);
	//----------------------------------
	// 비어 있는 슬롯 반환
	//----------------------------------
	bool GetEmptySlot(int8* SlotIndex);
};

