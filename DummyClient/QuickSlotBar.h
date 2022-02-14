#pragma once
#include <map>
#include "GameObjectInfo.h"

class CQuickSlotBar
{
public:
	// 퀵슬롯 바 인덱스
	int8 _QuickSlotBarIndex;

	CQuickSlotBar();
	~CQuickSlotBar();

	void Init();

	// QuickSlot에 스킬 저장
	void UpdateQuickSlotBarSlot(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo);

private:
	map<int8, st_QuickSlotBarSlotInfo*> _QuickSlotBarSlotInfos;
};

