#pragma once

#include "Message.h"

class CWeaponItem;
class CArmorItem;
class CMaterialItem;
struct st_Session;
struct st_CraftingTableData;

class CGameServerMessage : public CMessage
{
public:
	static CMemoryPoolTLS<CGameServerMessage> _MessageObjectPool;

	CGameServerMessage();
	~CGameServerMessage();

#pragma region 데이터 넣기
	CGameServerMessage& operator = (CGameServerMessage& GameServerMessage);
	
	// CMessage에서 오버로딩한 << >>를 상속 받게 해줌
	using CMessage::operator<<;
	using CMessage::operator>>;

	CGameServerMessage& operator << (st_GameObjectInfo& GameObjectInfo);
	CGameServerMessage& operator << (st_PositionInfo& PositionInfo);
	CGameServerMessage& operator << (Vector2Int& CellPositionInfo);
	CGameServerMessage& operator << (st_StatInfo& StatInfo);

	CGameServerMessage& operator << (st_SkillInfo& SkillInfo);

	CGameServerMessage& operator << (st_ItemInfo& ItemInfo);

	CGameServerMessage& operator << (st_TileInfo& TileInfo);

	CGameServerMessage& operator << (st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo);
	
	CGameServerMessage& operator << (st_Color& Color);

	CGameServerMessage& operator << (st_CraftingItemCategory& CraftingItemCategory);
	CGameServerMessage& operator << (st_CraftingCompleteItem& CraftingCompleteItem);
	CGameServerMessage& operator << (st_CraftingMaterialItemInfo& CraftingMaterialItemInfo);

	CGameServerMessage& operator << (st_CraftingTableRecipe& CraftingTable);

	CGameServerMessage& operator << (CItem** Item);
	CGameServerMessage& operator << (CItem* Item);

	CGameServerMessage& operator << (st_SkillInfo** SkillInfo);
	CGameServerMessage& operator << (CSkill** Skill);
	CGameServerMessage& operator << (CGameObject** GameObject);
	CGameServerMessage& operator << (CPlayer** Player);	
	CGameServerMessage& operator << (st_Session** Session);
#pragma endregion

#pragma region 데이터 빼기
	CGameServerMessage& operator >> (Vector2Int& CellPositionInfo);
	CGameServerMessage& operator >> (st_SkillInfo& SkillInfo);	
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