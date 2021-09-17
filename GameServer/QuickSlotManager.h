#pragma once
#include "GameObjectInfo.h"

class CQuickSlotBar;

class CQuickSlotManager
{
public:
	CQuickSlotManager();
	~CQuickSlotManager();

	// �����Թ� 3�� ����
	void Init();

	// �����Թٿ� ������ ���
	void AddQuickSlotBarSlot(st_QuickSlotBarSlotInfo* QuickSlotBarSlotInfo);
private:
	// �����Թ� 
	map<int8, CQuickSlotBar*> _QuickSlotBars;
};

