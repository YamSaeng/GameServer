#pragma once

#include "GameObject.h"

class CCraftingTable : public CGameObject
{
public:
	// 제작대 선택 여부
	bool _SelectedCraftingTable;

	// 제작대를 선택중인 Object
	CGameObject* _SelectedObject;

	CCraftingTable();

	virtual void Start();

	void CraftingStart(int64 CraftingTime);	
	void CraftingStop();

	// 선택된 제작 완성 아이템
	en_SmallItemCategory _SelectCraftingTableCompleteItem;

	// 제작대가 소유중인 재료 아이템 목록 반환
	map<en_SmallItemCategory, CItem*> GetMaterialItems();
	// 제작대가 소유중인 완성 제작품 목록 반환
	map<en_SmallItemCategory, CItem*> GetCompleteItems();
protected:
	// 제작 남은 시간
	int64 _CraftingRemainTime;

	// 제작대가 소유중인 재료 아이템 목록
	map<en_SmallItemCategory, CItem*> _MaterialItems;
	// 제작대가 소유중인 완성 제작품 아이템 목록
	map<en_SmallItemCategory, CItem*> _CompleteItems;
};