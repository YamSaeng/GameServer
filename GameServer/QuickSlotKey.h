#pragma once
#include "GameObjectInfo.h"

class CQuickSlotKey
{
public:
	CQuickSlotKey();
	~CQuickSlotKey();

	void QuickSlotKeyInit(vector<st_BindingKey> QuickSlotKeys);
	void QuickSlotKeyClear();
	
	void SetQuickSlotKey(en_UserQuickSlot UserQuickSlot, en_KeyCode KeyCode);

private:
	vector<st_BindingKey> _QuickSlotKeys;
};

