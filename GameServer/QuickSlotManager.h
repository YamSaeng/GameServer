#pragma once
#include "GameObjectInfo.h"

class CQuickSlotBar;

class CQuickSlotManager
{
public:
	CQuickSlotManager();
	~CQuickSlotManager();

	// 퀵슬롯바 3개 생성
	void Init();

	// 퀵슬롯바에 퀵슬롯 등록
	void UpdateQuickSlotBar(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo);

	// 퀵슬롯바 슬롯 스왑
	void SwapQuickSlot(st_QuickSlotBarSlotInfo& SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo& SwapBQuickSlotInfo);
	
	// 퀵슬롯바에서 퀵슬롯 위치 정보로 퀵슬롯 찾기
	st_QuickSlotBarSlotInfo* FindQuickSlotBar(int8 QuickSlotBarIndex, int8 QuickSlotbarSlotIndex);
	// 퀵슬롯바에 등록되어 있는 스킬을 찾아서 모두 반환
	vector<st_QuickSlotBarSlotInfo*> FindQuickSlotBarInfo(en_SkillType FindSkillType);
	// 퀵슬롯바에서 퀵슬롯 위치 스킬 정보로 찾아서 반환
	vector<st_QuickSlotBarPosition> FindQuickSlotBar(en_SkillType FindSkillType);
	// 퀵슬롯바에서 매개 변수로 받은 위치를 제외한 위치의 퀵슬롯바를 반환
	vector<st_QuickSlotBarPosition> GlobalCoolTimeFindQuickSlotBar(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, en_SkillKinds SkillKind);
	
	// 내용 비우기
	void Empty();	

	map<int8, CQuickSlotBar*> GetQuickSlotBar();
private:
	// 퀵슬롯바 
	map<int8, CQuickSlotBar*> _QuickSlotBars;
};