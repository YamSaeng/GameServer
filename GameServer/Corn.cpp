#include "pch.h"
#include "Corn.h"
#include "DataManager.h"

CCorn::CCorn()
{
    _GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_CROP_CORN;
    _GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

    st_ItemInfo* FindCropItemInfo = G_Datamanager->FindItemData(en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_FRUIT_CORN);

    _GameObjectInfo.ObjectName = FindCropItemInfo->ItemName;

    _GameObjectInfo.ObjectStatInfo.MaxHP = FindCropItemInfo->ItemMaxGatheringHP;
    _GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;

    _GameObjectInfo.ObjectWidth = 1;
    _GameObjectInfo.ObjectHeight = 1;
}

CCorn::~CCorn()
{

}

bool CCorn::OnDamaged(CGameObject* Attacker, int32 Damage)
{
    return false;
}
