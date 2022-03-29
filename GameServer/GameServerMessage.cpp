#include "pch.h"
#include "GameServerMessage.h"
#include "Item.h"
#include "Skill.h"

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

    *this << GameObjectInfo.ObjectPositionInfo;
    *this << GameObjectInfo.ObjectStatInfo;
    *this << (int16)GameObjectInfo.ObjectType;
    *this << GameObjectInfo.OwnerObjectId;
    *this << (int16)GameObjectInfo.OwnerObjectType;
    *this << GameObjectInfo.PlayerSlotIndex;

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_PositionInfo& PositionInfo)
{
    *this << (int8)PositionInfo.State;
    *this << PositionInfo.CollisionPositionX;
    *this << PositionInfo.CollisionPositionY;
    *this << (int8)PositionInfo.MoveDir;    
    *this << PositionInfo.PositionX;
    *this << PositionInfo.PositionY;

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_Vector2Int& CellPositionInfo)
{
    *this << CellPositionInfo._X;
    *this << CellPositionInfo._Y;

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

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_SkillInfo& SkillInfo)
{
    *this << SkillInfo.IsSkillLearn;
    *this << (int8)SkillInfo.SkillLargeCategory;
    *this << (int8)SkillInfo.SkillMediumCategory;
    *this << (int16)SkillInfo.SkillType;
    *this << SkillInfo.SkillLevel;

    int16 SkillNameLen = (int16)SkillInfo.SkillName.length() * 2;
    *this << SkillNameLen;
    InsertData(SkillInfo.SkillName.c_str(), SkillNameLen);

    *this << SkillInfo.SkillCoolTime;
    *this << SkillInfo.SkillCastingTime;
    *this << SkillInfo.SkillDurationTime;
    *this << SkillInfo.SkillDotTime;
    *this << SkillInfo.SkillRemainTime;

    int16 SkillExplanationLen = (int16)SkillInfo.SkillExplanation.length() * 2;
    *this << SkillExplanationLen;
    InsertData(SkillInfo.SkillExplanation.c_str(), SkillExplanationLen);

    int16 SkillImagePathLen = (int16)SkillInfo.SkillImagePath.length() * 2;
    *this << SkillImagePathLen;
    InsertData(SkillInfo.SkillImagePath.c_str(), SkillImagePathLen);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_ItemInfo& ItemInfo)
{
    *this << ItemInfo.ItemDBId;
    *this << ItemInfo.ItemIsQuickSlotUse;
    *this << ItemInfo.Rotated;
    *this << ItemInfo.Width;
    *this << ItemInfo.Height;
    *this << ItemInfo.TileGridPositionX;
    *this << ItemInfo.TileGridPositionY;

    *this << (int8)ItemInfo.ItemLargeCategory;
    *this << (int8)ItemInfo.ItemMediumCategory;
    *this << (int16)ItemInfo.ItemSmallCategory;    

    int16 ItemNameLen = (int16)ItemInfo.ItemName.length() * 2;
    *this << ItemNameLen;
    InsertData(ItemInfo.ItemName.c_str(), ItemNameLen);

    *this << ItemInfo.ItemMinDamage;
    *this << ItemInfo.ItemMaxDamage;
    *this << ItemInfo.ItemDefence;
    *this << ItemInfo.ItemMaxCount;

    *this << ItemInfo.ItemCount;

    int16 ItemImagePathLen = (int16)ItemInfo.ItemThumbnailImagePath.length() * 2;
    *this << ItemImagePathLen;
    InsertData(ItemInfo.ItemThumbnailImagePath.c_str(), ItemImagePathLen);

    *this << ItemInfo.ItemIsEquipped;         

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo)
{
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

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_Color& Color)
{
    *this << Color._Red;
    *this << Color._Green;
    *this << Color._Blue;   

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

CGameServerMessage& CGameServerMessage::operator>>(st_Vector2Int& CellPositionInfo)
{
    *this >> CellPositionInfo._X;
    *this >> CellPositionInfo._Y;

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

    int16 SkillImagePath = 0;
    *this >> SkillImagePath;
    GetData(SkillInfo.SkillImagePath, SkillImagePath);

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
