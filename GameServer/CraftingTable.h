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

	void CraftingStart();	
	void CraftingStop();

	// 선택된 제작하고자하는 아이템
	en_SmallItemCategory _SelectCraftingItemType;
	// 제작 시작 아이템
	en_SmallItemCategory _CraftingStartCompleteItem;

	// 제작대가 소유중인 재료 아이템 목록 반환
	map<en_SmallItemCategory, CItem*> GetMaterialItems();
	// 제작대가 소유중인 완성 제작품 목록 반환
	map<en_SmallItemCategory, CItem*> GetCompleteItems();

	st_CraftingTableRecipe GetCraftingTableRecipe();

	// 제작대에 재료 아이템 넣기
	void InputMaterialItem(CItem* MaterialItem, int16 MaterialItemCount);
	// 제작대에 아이템이 개수 만큼 있는지 확인
	bool FindMaterialItem(en_SmallItemCategory FindSmallItemCategory, int16 ItemCount);
protected:
	// 제작대가 소유중인 재료 아이템 목록
	map<en_SmallItemCategory, CItem*> _MaterialItems;
	// 제작대가 소유중인 완성 제작품 아이템 목록
	map<en_SmallItemCategory, CItem*> _CompleteItems;

	// 제작대가 가지고 있는 제작법
	st_CraftingTableRecipe _CraftingTableRecipe;
};