#pragma once
#include "CraftingTable.h"

class CItem;

class CFurnace : public CCraftingTable
{
public:
	CFurnace();

	// 용광로에 재료 아이템 넣기
	void InputMaterialItem(CItem* MaterialItem, int16 MaterialItemCount);	
	// 용광로에 아이템이 개수 만큼 있는지 확인
	bool FindMaterialItem(en_SmallItemCategory FindSmallItemCategory, int16 ItemCount);

	virtual void Update() override;
	
protected:
	virtual void UpdateCrafting() override;		
};

