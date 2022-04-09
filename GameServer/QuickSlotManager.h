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

	// �����Թ� ���� ����
	void SwapQuickSlot(st_QuickSlotBarSlotInfo& SwapAQuickSlotInfo, st_QuickSlotBarSlotInfo& SwapBQuickSlotInfo);
	
	// �����Թٿ��� ������ ���� ã��
	st_QuickSlotBarSlotInfo* FindQuickSlotBar(int8 QuickSlotBarIndex, int8 QuickSlotbarSlotIndex);

	// �����Թٿ��� ������ ��ġ ��ų ������ ã�Ƽ� ��ȯ
	vector<st_QuickSlotBarPosition> FindQuickSlotBar(en_SkillType FindSkillType);
	// �����Թٿ��� �Ű� ������ ���� ��ġ�� ������ ��ġ�� �����Թٸ� ��ȯ
	vector<st_QuickSlotBarPosition> ExceptionFindQuickSlotBar(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex);
	
	// ���� ����
	void Empty();	
private:
	// �����Թ� 
	map<int8, CQuickSlotBar*> _QuickSlotBars;
};