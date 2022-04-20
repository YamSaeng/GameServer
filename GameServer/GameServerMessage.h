#pragma once

#include "Message.h"

class CWeapon;
class CArmor;
class CMaterial;
struct st_Session;

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

	CGameServerMessage& operator << (CItem** Item);
	CGameServerMessage& operator << (CItem* Item);

	CGameServerMessage& operator << (st_SkillInfo** SkillInfo);
	CGameServerMessage& operator << (CSkill** Skill);
	CGameServerMessage& operator << (CGameObject** GameObject);
	CGameServerMessage& operator << (CPlayer** Player);	
	CGameServerMessage& operator << (st_Session** Session);
#pragma endregion

#pragma region ������ ����
	CGameServerMessage& operator >> (st_Vector2Int& CellPositionInfo);
	CGameServerMessage& operator >> (st_SkillInfo& SkillInfo);
	CGameServerMessage& operator >> (st_QuickSlotBarSlotInfo& Value);	
	CGameServerMessage& operator >> (CItem** Item);
	CGameServerMessage& operator >> (st_SkillInfo** SkillInfo);
	CGameServerMessage& operator >> (CSkill** Skill);
	CGameServerMessage& operator >> (CGameObject** GameObject);
	CGameServerMessage& operator >> (CPlayer** Player);
	CGameServerMessage& operator >> (st_Session** Session);
#pragma endregion

	static CGameServerMessage* GameServerMessageAlloc();
	virtual void Free() override;
};