#pragma once
#include "GameObjectInfo.h"

class CQuickSlotBar;

class CQuickSlotManager
{
public:
	CQuickSlotManager();
	~CQuickSlotManager();

	// Äü½½·Ô¹Ù 3°³ »ý¼º
	void Init();

	// Äü½½·Ô¹Ù¿¡ Äü½½·Ô µî·Ï
	void UpdateQuickSlotBar(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo);

	// Äü½½·Ô¹Ù ½½·Ô ½º¿Ò
	void SwapQuickSlot(st_QuickSlotBarSlotInfo& SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo& SwapBQuickSlotInfo);
	
	// Äü½½·Ô¹Ù¿¡¼­ Äü½½·Ô Á¤º¸ Ã£±â
	st_QuickSlotBarSlotInfo* FindQuickSlotBar(int8 QuickSlotBarIndex, int8 QuickSlotbarSlotIndex);

	// Äü½½·Ô¹Ù¿¡¼­ Äü½½·Ô À§Ä¡ ½ºÅ³ Á¤º¸·Î Ã£¾Æ¼­ ¹ÝÈ¯
	vector<st_QuickSlotBarPosition> FindQuickSlotBar(en_SkillType FindSkillType);
	// Äü½½·Ô¹Ù¿¡¼­ ¸Å°³ º¯¼ö·Î ¹ÞÀº À§Ä¡¸¦ Á¦¿ÜÇÑ À§Ä¡ÀÇ Äü½½·Ô¹Ù¸¦ ¹ÝÈ¯
	vector<st_QuickSlotBarPosition> ExceptionFindQuickSlotBar(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex);
	
	// ³»¿ë ºñ¿ì±â
	void Empty();	
private:
	// Äü½½·Ô¹Ù 
	map<int8, CQuickSlotBar*> _QuickSlotBars;
};