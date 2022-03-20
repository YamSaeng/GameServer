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

	void SwapQuickSlot(st_QuickSlotBarSlotInfo& SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo& SwapBQuickSlotInfo);
	
	CSkill* FindQuickSlotBar(int8 QuickSlotBarIndex,int8 QuickSlotbarSlotIndex);
	
	void Empty();
private:
	// �����Թ� 
	map<int8, CQuickSlotBar*> _QuickSlotBars;
};