#pragma once

#include "Message.h"

class CWeapon;
class CArmor;
class CMaterial;

class CGameServerMessage : public CMessage
{
public:
	static CMemoryPoolTLS<CGameServerMessage> _MessageObjectPool;

	CGameServerMessage();
	~CGameServerMessage();

#pragma region ������ �ֱ�
	CGameServerMessage& operator = (CGameServerMessage& GameServerMessage);
	
	// CMessage���� �����ε��� << >>�� ��� �ް� ����
	using CMessage::operator<<;
	using CMessage::operator>>;

	CGameServerMessage& operator << (st_GameObjectInfo& GameObjectInfo);
	CGameServerMessage& operator << (st_PositionInfo& PositionInfo);
	CGameServerMessage& operator << (st_Vector2Int& CellPositionInfo);
	CGameServerMessage& operator << (st_StatInfo& StatInfo);

	CGameServerMessage& operator << (st_SkillInfo& SkillInfo);

	CGameServerMessage& operator << (st_ItemInfo& ItemInfo);

	CGameServerMessage& operator << (st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo);
	
	CGameServerMessage& operator << (st_Color& Color);

	CGameServerMessage& operator << (CWeapon* Weapon);
	CGameServerMessage& operator << (CArmor* Armor);
	CGameServerMessage& operator << (CMaterial* Material);
#pragma endregion

#pragma region ������ ����
	CGameServerMessage& operator >> (st_Vector2Int& CellPositionInfo);
	CGameServerMessage& operator >> (st_SkillInfo& Value);
	CGameServerMessage& operator >> (st_QuickSlotBarSlotInfo& Value);

#pragma endregion

	static CGameServerMessage* GameServerMessageAlloc();
	virtual void Free() override;
};