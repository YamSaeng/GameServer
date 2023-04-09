#include "pch.h"
#include "GameServerMessage.h"
#include "Item.h"
#include "Skill.h"
#include "Data.h"

CMemoryPoolTLS<CGameServerMessage> CGameServerMessage::_MessageObjectPool(0);

CGameServerMessage::CGameServerMessage()
{

}

CGameServerMessage::~CGameServerMessage()
{

}

CGameServerMessage& CGameServerMessage::operator=(CGameServerMessage& GameServerMessage)
{
    memcpy(this, &GameServerMessage, sizeof(CGameServerMessage));
    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_GameObjectInfo& GameObjectInfo)
{
    *this << GameObjectInfo.ObjectId;

    int16 GameObjectInfoNameLen = (int16)GameObjectInfo.ObjectName.length() * 2;
    *this << GameObjectInfoNameLen;    
    InsertData(GameObjectInfo.ObjectName.c_str(), GameObjectInfoNameLen);      

    *this << GameObjectInfo.ObjectCropStep;
    *this << GameObjectInfo.ObjectCropMaxStep;
    *this << GameObjectInfo.ObjectSkillMaxPoint;
    *this << GameObjectInfo.ObjectPositionInfo;
    *this << GameObjectInfo.ObjectStatInfo;
    *this << (int16)GameObjectInfo.ObjectType;
    *this << GameObjectInfo.OwnerObjectId;
    *this << (int16)GameObjectInfo.OwnerObjectType;
    *this << GameObjectInfo.ObjectWidth;
    *this << GameObjectInfo.ObjectHeight;
    *this << GameObjectInfo.PlayerSlotIndex;

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_PositionInfo& PositionInfo)
{
    *this << (int8)PositionInfo.State;
    *this << PositionInfo.CollisionPosition.X;
    *this << PositionInfo.CollisionPosition.Y;      
    *this << PositionInfo.Position.X;
    *this << PositionInfo.Position.Y;
    *this << PositionInfo.LookAtDireciton.X;
    *this << PositionInfo.LookAtDireciton.Y;
    *this << PositionInfo.MoveDirection.X;
    *this << PositionInfo.MoveDirection.Y;

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(Vector2Int& CellPositionInfo)
{
    *this << CellPositionInfo.X;
    *this << CellPositionInfo.Y;

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_StatInfo& StatInfo)
{
    *this << StatInfo.Level;
    *this << StatInfo.HP;
    *this << StatInfo.MaxHP;
    *this << StatInfo.MP;
    *this << StatInfo.MaxMP;
    *this << StatInfo.DP;
    *this << StatInfo.MaxDP;
    *this << StatInfo.AutoRecoveryHPPercent;
    *this << StatInfo.AutoRecoveryMPPercent;
    *this << StatInfo.MinMeleeAttackDamage;
    *this << StatInfo.MaxMeleeAttackDamage;
    *this << StatInfo.MeleeAttackHitRate;
    *this << StatInfo.MagicDamage;
    *this << StatInfo.MagicHitRate;
    *this << StatInfo.Defence;
    *this << StatInfo.EvasionRate;
    *this << StatInfo.MeleeCriticalPoint;
    *this << StatInfo.MagicCriticalPoint;
    *this << StatInfo.StatusAbnormalResistance;
    *this << StatInfo.Speed;  
    *this << StatInfo.MaxSpeed;

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_SkillInfo& SkillInfo)
{
    *this << SkillInfo.CanSkillUse;
    *this << SkillInfo.IsSkillLearn;   
    *this << (int8)SkillInfo.SkillCharacteristic;
    *this << (int8)SkillInfo.SkillLargeCategory;
    *this << (int8)SkillInfo.SkillMediumCategory;
    *this << (int16)SkillInfo.SkillType;
    *this << SkillInfo.SkillLevel;
    *this << SkillInfo.SkillMinDamage;
    *this << SkillInfo.SkillMaxDamage;

    *this << SkillInfo.SkillOverlapStep;

    int16 SkillNameLen = (int16)SkillInfo.SkillName.length() * 2;
    *this << SkillNameLen;
    InsertData(SkillInfo.SkillName.c_str(), SkillNameLen);

    *this << SkillInfo.SkillCoolTime;
    *this << SkillInfo.SkillCastingTime;
    *this << SkillInfo.SkillDurationTime;
    *this << SkillInfo.SkillDotTime;
    *this << SkillInfo.SkillRemainTime;    

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_ItemInfo& ItemInfo)
{
    *this << ItemInfo.ItemDBId;
    *this << ItemInfo.ItemIsEquipped;
    *this << ItemInfo.ItemWidth;
    *this << ItemInfo.ItemHeight;
    *this << ItemInfo.ItemTileGridPositionX;
    *this << ItemInfo.ItemTileGridPositionY;

    *this << (int16)ItemInfo.OwnerCraftingTable;
    *this << (int8)ItemInfo.ItemLargeCategory;
    *this << (int8)ItemInfo.ItemMediumCategory;
    *this << (int16)ItemInfo.ItemSmallCategory; 

    *this << (int8)ItemInfo.ItemEquipmentPart;

    int16 ItemNameLen = (int16)ItemInfo.ItemName.length() * 2;
    *this << ItemNameLen;
    InsertData(ItemInfo.ItemName.c_str(), ItemNameLen);

    int16 ItemExplainLen = (int16)ItemInfo.ItemExplain.length() * 2;
    *this << ItemExplainLen;
	InsertData(ItemInfo.ItemExplain.c_str(), ItemExplainLen);

    *this << ItemInfo.ItemCraftingTime;
    *this << ItemInfo.ItemCraftingRemainTime;
    *this << ItemInfo.ItemMinDamage;
    *this << ItemInfo.ItemMaxDamage;
    *this << ItemInfo.ItemDefence;
    *this << ItemInfo.ItemMaxCount;

    *this << ItemInfo.ItemCount;    

    *this << ItemInfo.ItemIsEquipped;

    int16 ItemCraftingMaterialCount = static_cast<int16>(ItemInfo.Materials.size());
    *this << ItemCraftingMaterialCount;

    if (ItemCraftingMaterialCount > 0)
    {
        for (st_CraftingMaterialItemInfo MaterialItemInfo : ItemInfo.Materials)
        {
            *this << MaterialItemInfo;
        }
    }

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo)
{
    *this << (int8)QuickSlotBarSlotInfo.QuickSlotBarType;
    *this << QuickSlotBarSlotInfo.AccountDBId;
    *this << QuickSlotBarSlotInfo.PlayerDBId;
    *this << QuickSlotBarSlotInfo.QuickSlotBarIndex;
    *this << QuickSlotBarSlotInfo.QuickSlotBarSlotIndex;
    *this << QuickSlotBarSlotInfo.QuickSlotKey;   
    
    bool EmptyQuickSlotSkillInfo;
    if (QuickSlotBarSlotInfo.QuickBarSkill == nullptr)
    {
        EmptyQuickSlotSkillInfo = true;
        *this << EmptyQuickSlotSkillInfo;
    }
    else
    {
        EmptyQuickSlotSkillInfo = false;
        *this << EmptyQuickSlotSkillInfo;
        *this << *(QuickSlotBarSlotInfo.QuickBarSkill->GetSkillInfo());
    }   

    bool EmptyQuickSlotItemInfo;
    if (QuickSlotBarSlotInfo.QuickBarItem == nullptr)
    {
        EmptyQuickSlotItemInfo = true;
        *this << EmptyQuickSlotItemInfo;
    }
    else
    {
        EmptyQuickSlotItemInfo = false;
        *this << EmptyQuickSlotItemInfo;
        *this << QuickSlotBarSlotInfo.QuickBarItem->_ItemInfo;
    }

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_Color& Color)
{
    *this << Color._Red;
    *this << Color._Green;
    *this << Color._Blue;   

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_CraftingItemCategory& CraftingItemCategory)
{
    *this << (int8)CraftingItemCategory.CategoryType;
    
    int16 CraftingItemCategoryNameLen = (int16)(CraftingItemCategory.CategoryName.length() * 2);
    *this << CraftingItemCategoryNameLen;
    InsertData(CraftingItemCategory.CategoryName.c_str(), CraftingItemCategoryNameLen);

    int8 CraftingItemCategoryCompleteItemCount = (int8)CraftingItemCategory.CommonCraftingCompleteItems.size();
    *this << CraftingItemCategoryCompleteItemCount;

    for (CItem* CraftingCompleteItem : CraftingItemCategory.CommonCraftingCompleteItems)
    {
        *this << CraftingCompleteItem->_ItemInfo;
    }

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_CraftingCompleteItem& CraftingCompleteItem)
{
    *this << (int16)CraftingCompleteItem.OwnerCraftingTable;
    *this << (int16)CraftingCompleteItem.CompleteItemType;

    int16 CraftingCompleteItemNameLen = (int16)(CraftingCompleteItem.CompleteItemName.length() * 2);
    *this << CraftingCompleteItemNameLen;
    InsertData(CraftingCompleteItem.CompleteItemName.c_str(), CraftingCompleteItemNameLen);    

    int8 CraftingCompleteItemCount = (int8)CraftingCompleteItem.Materials.size();
    *this << CraftingCompleteItemCount;

    for (st_CraftingMaterialItemInfo MaterialItemInfo : CraftingCompleteItem.Materials)
    {
        *this << MaterialItemInfo;
    }

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_CraftingMaterialItemInfo& CraftingMaterialItemInfo)
{    
    *this << (int16)CraftingMaterialItemInfo.MaterialItemType;
        
    int16 MaterialItemNameLen = static_cast<int16>(CraftingMaterialItemInfo.MaterialItemName.length() * 2);
	*this << MaterialItemNameLen;
    InsertData(CraftingMaterialItemInfo.MaterialItemName.c_str(), MaterialItemNameLen);

    *this << CraftingMaterialItemInfo.ItemCount;    

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_CraftingTableRecipe& CraftingTable)
{
    *this << (int16)CraftingTable.CraftingTableType;

    int16 CraftingTableNameLen = static_cast<int16>(CraftingTable.CraftingTableName.length() * 2);
    *this << CraftingTableNameLen;
    InsertData(CraftingTable.CraftingTableName.c_str(), CraftingTableNameLen);

    int8 CraftingCompleteItemCount = (int8)CraftingTable.CraftingTableCompleteItems.size();
    *this << CraftingCompleteItemCount;

    for (CItem* CraftingComplteItem : CraftingTable.CraftingTableCompleteItems)
    {
        *this << CraftingComplteItem->_ItemInfo;
    }

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(CItem** Item)
{
    memcpy(&_MessageBuf[_Rear], Item, sizeof(CItem*));
    _Rear += sizeof(CItem*);
    _UseBufferSize += sizeof(CItem*);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(CItem* Item)
{
    *this << Item->_ItemInfo;

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_SkillInfo** SkillInfo)
{
    memcpy(&_MessageBuf[_Rear], SkillInfo, sizeof(st_SkillInfo*));
    _Rear += sizeof(st_SkillInfo*);
    _UseBufferSize += sizeof(st_SkillInfo*);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(CSkill** Skill)
{
    memcpy(&_MessageBuf[_Rear], Skill, sizeof(CSkill*));
    _Rear += sizeof(CSkill*);
    _UseBufferSize += sizeof(CSkill*);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(CGameObject** GameObject)
{
    memcpy(&_MessageBuf[_Rear], GameObject, sizeof(CGameObject*));
    _Rear += sizeof(CGameObject*);
    _UseBufferSize += sizeof(CGameObject*);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(CPlayer** Player)
{
    memcpy(&_MessageBuf[_Rear], Player, sizeof(CPlayer*));
    _Rear += sizeof(CPlayer*);
    _UseBufferSize += sizeof(CPlayer*);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_Session** Session)
{
    memcpy(&_MessageBuf[_Rear], Session, sizeof(st_Session*));
    _Rear += sizeof(st_Session*);
    _UseBufferSize += sizeof(st_Session*);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator>>(Vector2Int& CellPositionInfo)
{
    *this >> CellPositionInfo.X;
    *this >> CellPositionInfo.Y;

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator>>(st_SkillInfo& SkillInfo)
{
    int8 SkillLargeCategory = 0;
    *this >> SkillLargeCategory;
    SkillInfo.SkillLargeCategory = (en_SkillLargeCategory)SkillLargeCategory;

    int8 SkillMediumCategory = 0;
    *this >> SkillMediumCategory;
    SkillInfo.SkillMediumCategory = (en_SkillMediumCategory)SkillMediumCategory;

    int16 SkillType = 0;
    *this >> SkillType;
    SkillInfo.SkillType = (en_SkillType)SkillType;

    *this >> SkillInfo.SkillLevel;

    int16 SkillNameLen = 0;
    *this >> SkillNameLen;
    GetData(SkillInfo.SkillName, SkillNameLen);

    *this >> SkillInfo.SkillCoolTime;
    *this >> SkillInfo.SkillCastingTime;
    *this >> SkillInfo.SkillDurationTime;
    *this >> SkillInfo.SkillDotTime;
    *this >> SkillInfo.SkillRemainTime;    

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator>>(st_QuickSlotBarSlotInfo& Value)
{
    *this >> Value.AccountDBId;
    *this >> Value.PlayerDBId;
    *this >> Value.QuickSlotBarIndex;
    *this >> Value.QuickSlotBarSlotIndex;
    *this >> Value.QuickSlotKey;        

    st_SkillInfo* SkillInfo = Value.QuickBarSkill->GetSkillInfo();
    *this >> *SkillInfo;

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator>>(CItem** Item)
{
    memcpy(Item, &_MessageBuf[_Front], sizeof(CItem*));
    _Front += sizeof(CItem*);
    _UseBufferSize -= sizeof(CItem*);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator>>(st_SkillInfo** SkillInfo)
{
    memcpy(SkillInfo, &_MessageBuf[_Front], sizeof(st_SkillInfo*));
    _Front += sizeof(st_SkillInfo*);
    _UseBufferSize -= sizeof(st_SkillInfo*);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator>>(CSkill** Skill)
{
    memcpy(Skill, &_MessageBuf[_Front], sizeof(CSkill*));
    _Front += sizeof(CSkill*);
    _UseBufferSize -= sizeof(CSkill*);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator>>(CGameObject** GameObject)
{
    memcpy(GameObject, &_MessageBuf[_Front], sizeof(CGameObject*));
    _Front += sizeof(CGameObject*);
    _UseBufferSize -= sizeof(CGameObject*);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator>>(CPlayer** Player)
{
    memcpy(Player, &_MessageBuf[_Front], sizeof(CPlayer*));
    _Front += sizeof(CPlayer*);
    _UseBufferSize -= sizeof(CPlayer*);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator>>(st_Session** Session)
{
    memcpy(Session, &_MessageBuf[_Front], sizeof(st_Session*));
    _Front += sizeof(st_Session*);
    _UseBufferSize -= sizeof(st_Session*);

    return *(this);
}

CGameServerMessage* CGameServerMessage::GameServerMessageAlloc()
{
    CGameServerMessage* AllocMessage = _MessageObjectPool.Alloc();
    InterlockedIncrement(AllocMessage->_RetCount);
    return AllocMessage;
}

void CGameServerMessage::Free()
{
    if (InterlockedDecrement(_RetCount) == 0)
    {
        _MessageObjectPool.Free(this);
    }
}
