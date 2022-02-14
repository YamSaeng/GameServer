#pragma once
#include <map>
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

	void SwapQuickSlot(st_QuickSlotBarSlotInfo& SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo& SwapBQuickSlotInfo);
private:
	// �����Թ� 
	map<int8, CQuickSlotBar*> _QuickSlotBars;
};