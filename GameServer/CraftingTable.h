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

	// ���õ� �����ϰ����ϴ� ������
	en_SmallItemCategory _SelectCraftingItemType;
	// ���� ���� ������
	en_SmallItemCategory _CraftingStartCompleteItem;

	// ���۴밡 �������� ��� ������ ��� ��ȯ
	map<en_SmallItemCategory, CItem*> GetMaterialItems();
	// ���۴밡 �������� �ϼ� ����ǰ ��� ��ȯ
	map<en_SmallItemCategory, CItem*> GetCompleteItems();

	st_CraftingTableRecipe GetCraftingTableRecipe();

	// ���۴뿡 ��� ������ �ֱ�
	void InputMaterialItem(CItem* MaterialItem, int16 MaterialItemCount);
	// ���۴뿡 �������� ���� ��ŭ �ִ��� Ȯ��
	bool FindMaterialItem(en_SmallItemCategory FindSmallItemCategory, int16 ItemCount);
protected:
	// ���۴밡 �������� ��� ������ ���
	map<en_SmallItemCategory, CItem*> _MaterialItems;
	// ���۴밡 �������� �ϼ� ����ǰ ������ ���
	map<en_SmallItemCategory, CItem*> _CompleteItems;

	// ���۴밡 ������ �ִ� ���۹�
	st_CraftingTableRecipe _CraftingTableRecipe;
};