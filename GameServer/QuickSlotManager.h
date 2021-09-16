#pragma once

class CQuickSlotBar;

class CQuickSlotManager
{
public:
	// 퀵슬롯 저장
	void AddQuickSlotBar(int8 QuickSlotBarIndex, CQuickSlotBar* NewQuickSlotBar);
private:
	// QuickSlotBarIndex, QuickSlotBar 저장
	map<int8, CQuickSlotBar*> _QuickSlotBars;
};

