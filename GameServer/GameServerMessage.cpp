#include "pch.h"
#include "GameServerMessage.h"
#include "Item.h"

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

    int8 GameObjectInfoNameLen = (int8)GameObjectInfo.ObjectName.length() * 2;
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
    *this << PositionInfo.PositionX;
    *this << PositionInfo.PositionY;
    *this << (int8)PositionInfo.MoveDir;    

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
    *this << StatInfo.MinAttackDamage;
    *this << StatInfo.MaxAttackDamage;
    *this << StatInfo.CriticalPoint;
    *this << StatInfo.Speed;    

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_SkillInfo& SkillInfo)
{
    *this << (int16)SkillInfo.SkillType;
    *this << SkillInfo.SkillLevel;

    int8 SkillNameLen = (int8)SkillInfo.SkillName.length() * 2;
    *this << SkillNameLen;
    InsertData(SkillInfo.SkillName.c_str(), SkillNameLen);

    *this << SkillInfo.SkillCoolTime;
    *this << SkillInfo.SkillCastingTime;

    int8 SkillImagePathLen = (int8)SkillInfo.SkillImagePath.length() * 2;
    *this << SkillImagePathLen;
    InsertData(SkillInfo.SkillImagePath.c_str(), SkillImagePathLen);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_ItemInfo& ItemInfo)
{
    *this << ItemInfo.ItemDBId;
    *this << ItemInfo.ItemIsQuickSlotUse;
    *this << (int8)ItemInfo.ItemLargeCategory;
    *this << (int8)ItemInfo.ItemMediumCategory;
    *this << (int16)ItemInfo.ItemSmallCategory;    

    int8 ItemNameLen = (int8)ItemInfo.ItemName.length() * 2;
    *this << ItemNameLen;
    InsertData(ItemInfo.ItemName.c_str(), ItemNameLen);

    *this << ItemInfo.ItemMinDamage;
    *this << ItemInfo.ItemMaxDamage;
    *this << ItemInfo.ItemDefence;
    *this << ItemInfo.ItemMaxCount;

    *this << ItemInfo.ItemCount;

    int8 ItemImagePathLen = (int8)ItemInfo.ItemThumbnailImagePath.length() * 2;
    *this << ItemImagePathLen;
    InsertData(ItemInfo.ItemThumbnailImagePath.c_str(), ItemImagePathLen);

    *this << ItemInfo.ItemSlotIndex;
    *this << ItemInfo.ItemIsEquipped;        

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator<<(st_QuickSlotBarSlotInfo& QuickSlotBarSlotInfo)
{
    *this << QuickSlotBarSlotInfo.AccountDBId;
    *this << QuickSlotBarSlotInfo.PlayerDBId;
    *this << QuickSlotBarSlotInfo.QuickSlotBarIndex;
    *this << QuickSlotBarSlotInfo.QuickSlotBarSlotIndex;

    int8 QuickSlotKeyLen = (int8)QuickSlotBarSlotInfo.QuickSlotKey.length() * 2;
    *this << QuickSlotKeyLen;
    InsertData(QuickSlotBarSlotInfo.QuickSlotKey.c_str(), QuickSlotKeyLen);

    *this << QuickSlotBarSlotInfo.QuickBarSkillInfo;

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

CGameServerMessage& CGameServerMessage::operator>>(st_Vector2Int& CellPositionInfo)
{
    *this >> CellPositionInfo._X;
    *this >> CellPositionInfo._Y;

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator>>(st_SkillInfo& Value)
{
    int16 SkillType = 0;
    *this >> SkillType;
    Value.SkillType = (en_SkillType)SkillType;

    *this >> Value.SkillLevel;

    int8 SkillNameLen = 0;
    *this >> SkillNameLen;
    GetData(Value.SkillName, SkillNameLen);

    *this >> Value.SkillCoolTime;
    *this >> Value.SkillCastingTime;

    int8 SkillImagePath = 0;
    *this >> SkillImagePath;
    GetData(Value.SkillImagePath, SkillImagePath);

    return *(this);
}

CGameServerMessage& CGameServerMessage::operator>>(st_QuickSlotBarSlotInfo& Value)
{
    *this >> Value.AccountDBId;
    *this >> Value.PlayerDBId;
    *this >> Value.QuickSlotBarIndex;
    *this >> Value.QuickSlotBarSlotIndex;

    int8 QuickSlotKeyLen = 0;
    *this >> QuickSlotKeyLen;
    GetData(Value.QuickSlotKey, QuickSlotKeyLen);

    *this >> Value.QuickBarSkillInfo;

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
