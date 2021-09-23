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
	void UpdateQuickSlotBar(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo);

	bool CanQuickSlotBarUse(st_QuickSlotBarSlotInfo& FindQuickSlotBarSlotInfo);
private:
	// �����Թ� 
	map<int8, CQuickSlotBar*> _QuickSlotBars;
};

