#pragma once

#include "GameObject.h"

class CCraftingTable : public CGameObject
{
public:
	// ���۴� ���� ����
	bool _SelectedCraftingTable;

	// ���۴븦 �������� Object
	CGameObject* _SelectedObject;	

	CCraftingTable();

	virtual void Start();

	void CraftingStart();	
	void CraftingStop();

	// ���õ� ���� �ϼ� ������
	en_SmallItemCategory _SelectCraftingTableCompleteItem;
	// ���� ���� ������
	en_SmallItemCategory _CraftingStartCompleteItem;

	// ���۴밡 �������� ��� ������ ��� ��ȯ
	map<en_SmallItemCategory, CItem*> GetMaterialItems();
	// ���۴밡 �������� �ϼ� ����ǰ ��� ��ȯ
	map<en_SmallItemCategory, CItem*> GetCompleteItems();

	st_CraftingTableRecipe GetCraftingTableRecipe();
protected:
	// ���۴밡 �������� ��� ������ ���
	map<en_SmallItemCategory, CItem*> _MaterialItems;
	// ���۴밡 �������� �ϼ� ����ǰ ������ ���
	map<en_SmallItemCategory, CItem*> _CompleteItems;

	// ���۴밡 ������ �ִ� ���۹�
	st_CraftingTableRecipe _CraftingTableRecipe;
};