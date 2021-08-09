#include "pch.h"
#include "GameObject.h"

CGameObject::CGameObject()
{
	_Channel = nullptr;
}

CGameObject::CGameObject(st_GameObjectInfo GameObjectInfo)
{
	_GameObjectInfo = GameObjectInfo;	
}

CGameObject::~CGameObject()
{

}

void CGameObject::Update()
{

}

void CGameObject::OnDamaged(CGameObject Attacker, int32 Damage)
{

}

void CGameObject::OnDead(CGameObject Killer)
{

}

st_PositionInfo CGameObject::GetPositionInfo()
{
	return _GameObjectInfo.ObjectPositionInfo;
}

st_Vector2Int CGameObject::GetCellPosition()
{
	return st_Vector2Int(_GameObjectInfo.ObjectPositionInfo.PositionX,_GameObjectInfo.ObjectPositionInfo.PositionY);
}
