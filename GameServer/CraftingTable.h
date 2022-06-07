#pragma once

#include "GameObject.h"

class CCraftingTable : public CGameObject
{
public:
	// ���۴� ���� ����
	bool _SelectedCraftingTable;

	// ���۴븦 �������� ObjectID
	int64 _OwnerObjectID;

	CCraftingTable();

	virtual void Start();
protected:
	// ���õ� ���� �ϼ� ������
	en_SmallItemCategory _SelectCraftingTableCompleteItem;
};