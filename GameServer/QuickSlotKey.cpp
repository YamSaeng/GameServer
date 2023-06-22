#include "pch.h"
#include "QuickSlotKey.h"

CQuickSlotKey::CQuickSlotKey()
{
	
}

CQuickSlotKey::~CQuickSlotKey()
{
}

void CQuickSlotKey::QuickSlotKeyInit(vector<st_BindingKey> QuickSlotKeys)
{
	_QuickSlotKeys = QuickSlotKeys;
}

void CQuickSlotKey::QuickSlotKeyClear()
{
	_QuickSlotKeys.clear();
}

void CQuickSlotKey::SetQuickSlotKey(en_UserQuickSlot UserQuickSlot, en_KeyCode KeyCode)
{
	for (st_BindingKey QuickSlotKey : _QuickSlotKeys)
	{
		if (QuickSlotKey.UserQuickSlot == UserQuickSlot)
		{
			QuickSlotKey.KeyCode = KeyCode;
		}
	}
}
