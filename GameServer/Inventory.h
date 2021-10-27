#pragma once
#include "Item.h"

class CInventory
{
public:	

	//---------------------
	// 보유하고 있는 동전
	//---------------------
	int16 _BronzeCoinCount;
	int16 _SliverCoinCount;
	int64 _GoldCoinCount;

	CInventory();
	~CInventory();
	
	//----------------------
	// 인벤토리 초기화
	//----------------------
	void Init();
		
	//------------------------
	// 인벤토리에 아이템 추가
	//------------------------
	void AddItem(CItem* Item);
	//--------------------------
	// 아이템 반환
	//--------------------------
	CItem* Get(int8 SlotIndex);
	
	//-----------------------------------------------------------------------
	// 인벤토리에 이미 아이템이 있는지 확인 ( 아이템 인벤토리에 넣을때 확인 )
	//-----------------------------------------------------------------------
	bool IsExistItem(en_SmallItemCategory ItemType, int16& ItemEach,int8* SlotIndex);

	//--------------------------------------
	// ItemType 받아서 아이템정보 반환
	//--------------------------------------
	vector<CItem*> Find(en_LargeItemCategory ItemLargeType);
	vector<CItem*> Find(en_MediumItemCategory ItemMediumType);
	vector<CItem*> Find(en_SmallItemCategory ItemSmallType);	

	//----------------------------------
	// 비어 있는 슬롯 반환
	//----------------------------------
	bool GetEmptySlot(int8* SlotIndex);

	//--------------------------------------
	// 인벤토리 아이템 스왑
	//--------------------------------------
	void SwapItem(int8 SwapAIndex, int8 SwapBIndex);

	//--------------------------------
	// 인벤토리 슬롯 초기화
	//--------------------------------
	void SlotInit(int8 InitSlotIndex);
private:
	//---------------------------------------------
	// 인벤토리가 소유중인 아이템 ( 포인터로 관리 )
	//   ( SlotIndex, 아이템정보 )
	//---------------------------------------------
	map<int8, CItem*> _Items;
};

