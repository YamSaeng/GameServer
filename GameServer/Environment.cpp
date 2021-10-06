#include "pch.h"
#include "Environment.h"
#include "DataManager.h"
#include "ObjectManager.h"
#include <atlbase.h>

CEnvironment::CEnvironment()
{
}

void CEnvironment::Init(st_Vector2Int SpawnPosition)
{
	_SpawnPosition = SpawnPosition;
}

void CEnvironment::Update()
{
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::IDLE:
		UpdateIdle();
		break;
	default:
		break;
	}

}

void CEnvironment::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	CGameObject::OnDamaged(Attacker, Damage);

	_Target = (CPlayer*)Attacker;

	if (_GameObjectInfo.ObjectStatInfo.HP == 0)
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

		OnDead(Attacker);
	}
}

void CEnvironment::UpdateIdle()
{

}

CStone::CStone()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_STONE;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	auto FindEnvironmentData = G_Datamanager->_Environments.find(en_ObjectDataType::STONE_DATA);
	st_EnvironmentData EnvironmentData = *(*FindEnvironmentData).second;

	_GameObjectInfo.ObjectName = (LPWSTR)CA2W(EnvironmentData.EnvironmentName.c_str());
	_GameObjectInfo.ObjectStatInfo.MaxHP = EnvironmentData.MaxHP;
	_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.Level = EnvironmentData.Level;
}

void CStone::Init(st_Vector2Int SpawnPosition)
{
	CEnvironment::Init(SpawnPosition);
}

void CStone::OnDead(CGameObject* Killer)
{
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

	G_ObjectManager->ItemSpawn(Killer->_GameObjectInfo.ObjectId, Killer->_GameObjectInfo.ObjectType, GetCellPosition(), _GameObjectInfo.ObjectType, en_ObjectDataType::STONE_DATA);

	BroadCastPacket(en_PACKET_S2C_CHANGE_OBJECT_STAT);
	BroadCastPacket(en_PACKET_S2C_DIE);

	G_ObjectManager->Remove(this, 1);
}

void CStone::UpdateIdle()
{
}

CTree::CTree()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_TREE;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	auto FindEnvironmentData = G_Datamanager->_Environments.find(en_ObjectDataType::TREE_DATA);
	st_EnvironmentData EnvironmentData = *(*FindEnvironmentData).second;

	_GameObjectInfo.ObjectName = (LPWSTR)CA2W(EnvironmentData.EnvironmentName.c_str());
	_GameObjectInfo.ObjectStatInfo.MaxHP = EnvironmentData.MaxHP;
	_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.Level = EnvironmentData.Level;
}

void CTree::Init(st_Vector2Int SpawnPosition)
{
	CEnvironment::Init(SpawnPosition);
}

void CTree::OnDead(CGameObject* Killer)
{
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

	G_ObjectManager->ItemSpawn(Killer->_GameObjectInfo.ObjectId, Killer->_GameObjectInfo.ObjectType, GetCellPosition(), _GameObjectInfo.ObjectType, en_ObjectDataType::TREE_DATA);
	
	BroadCastPacket(en_PACKET_S2C_CHANGE_OBJECT_STAT);
	BroadCastPacket(en_PACKET_S2C_DIE);

	G_ObjectManager->Remove(this, 1);
}

void CTree::UpdateIdle()
{	
	
}

