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

	void CraftingStart(int64 CraftingTime);	
	void CraftingStop();

	// ���õ� ���� �ϼ� ������
	en_SmallItemCategory _SelectCraftingTableCompleteItem;

	// ���۴밡 �������� ��� ������ ��� ��ȯ
	map<en_SmallItemCategory, CItem*> GetMaterialItems();
	// ���۴밡 �������� �ϼ� ����ǰ ��� ��ȯ
	map<en_SmallItemCategory, CItem*> GetCompleteItems();
protected:
	// ���� ���� �ð�
	int64 _CraftingRemainTime;

	// ���۴밡 �������� ��� ������ ���
	map<en_SmallItemCategory, CItem*> _MaterialItems;
	// ���۴밡 �������� �ϼ� ����ǰ ������ ���
	map<en_SmallItemCategory, CItem*> _CompleteItems;
};