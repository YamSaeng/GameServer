#pragma once
#include "GameObjectInfo.h"

class CQuickSlotBar
{
public:
	// ������ �� �ε���
	int8 _Index;

	CQuickSlotBar();
	~CQuickSlotBar();

	void Init();

	// QuickSlot�� ��ų ����
	void AddQuickSlotBarSlot(st_QuickSlotBarSlotInfo* QuickSlotBarSlotInfo);
private:
	map<int8, st_QuickSlotBarSlotInfo*> _QuickSlotBarSlotInfos;	
};

