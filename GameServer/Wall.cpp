#include "pch.h"
#include "Wall.h"
#include "DataManager.h"
#include "RectCollision.h"

CWall::CWall()
{	
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	auto FindEnvironmentData = G_Datamanager->_Environments.find(en_GameObjectType::OBJECT_WALL);
	st_EnvironmentData EnvironmentData = *(*FindEnvironmentData).second;

	_GameObjectInfo.ObjectName = (LPWSTR)CA2W(EnvironmentData.EnvironmentName.c_str());
	_GameObjectInfo.ObjectStatInfo.MaxHP = EnvironmentData.MaxHP;
	_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.Level = EnvironmentData.Level;

	_GameObjectInfo.ObjectWidth = 1;
	_GameObjectInfo.ObjectHeight = 1;	
}

void CWall::Start()
{
}

bool CWall::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	return false;
}

void CWall::UpdateIdle()
{
}
