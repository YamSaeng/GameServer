#pragma once

class CQuickSlotBar;

class CQuickSlotManager
{
public:
	// ������ ����
	void AddQuickSlotBar(int8 QuickSlotBarIndex, CQuickSlotBar* NewQuickSlotBar);
private:
	// QuickSlotBarIndex, QuickSlotBar ����
	map<int8, CQuickSlotBar*> _QuickSlotBars;
};

