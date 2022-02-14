#pragma once
#include <map>
#include "GameObjectInfo.h"

class CQuickSlotBar
{
public:
	// ������ �� �ε���
	int8 _QuickSlotBarIndex;

	CQuickSlotBar();
	~CQuickSlotBar();

	void Init();

	// QuickSlot�� ��ų ����
	void UpdateQuickSlotBarSlot(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo);

private:
	map<int8, st_QuickSlotBarSlotInfo*> _QuickSlotBarSlotInfos;
};

