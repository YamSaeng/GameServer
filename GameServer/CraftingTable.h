#pragma once

#include "GameObject.h"

class CCraftingTable : public CGameObject
{
public:
	// 제작대 선택 여부
	bool _SelectedCraftingTable;

	// 제작대를 조종중인 ObjectID
	int64 _OwnerObjectID;

	CCraftingTable();

	virtual void Start();
protected:
	// 선택된 제작 완성 아이템
	en_SmallItemCategory _SelectCraftingTableCompleteItem;
};