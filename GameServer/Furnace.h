#pragma once
#include "CraftingTable.h"

class CItem;

class CFurnace : public CCraftingTable
{
public:
	CFurnace();

	// 용광로 소유중인 재료 아이템 반환
	map<en_SmallItemCategory, CItem*> GetMaterialItems();
	// 용광로에 재료 아이템 넣기
	void InputMaterialItem(CItem* MaterialItem, int16 MaterialItemCount);	

	void CraftingStart();
private:	
	// 용광로가 소유중인 재료 아이템 
	map<en_SmallItemCategory, CItem*> _MaterialItems;

	// 용광로 제작템 목록
	st_CraftingTable _FurnaceCraftingTable;	
};

