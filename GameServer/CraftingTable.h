#pragma once

#include "GameObject.h"

class CCraftingTable : public CGameObject
{
public:
	// ���۴� ���� ����
	bool _SelectedCraftingTable;

	// ���۴븦 �������� ObjectID
	int64 _SelectedObjectID;

	CCraftingTable();

	virtual void Start();

	void CraftingStart();	

	// ���õ� ���� �ϼ� ������
	en_SmallItemCategory _SelectCraftingTableCompleteItem;	
};