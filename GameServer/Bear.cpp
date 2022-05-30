#include "pch.h"
#include "Bear.h"
#include "DataManager.h"
#include "Player.h"
#include "ObjectManager.h"
#include <atlbase.h>

CBear::CBear()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_BEAR;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;

	auto FindMonsterStat = G_Datamanager->_Monsters.find(en_ObjectDataType::BEAR_DATA);
	st_MonsterData MonsterData = *(*FindMonsterStat).second;

	// ½ºÅÈ ¼ÂÆÃ		
	_GameObjectInfo.ObjectName = (LPWSTR)CA2W(MonsterData.MonsterName.c_str());
	_GameObjectInfo.ObjectStatInfo.Level = MonsterData.MonsterStatInfo.Level;
	_GameObjectInfo.ObjectStatInfo.MaxHP = MonsterData.MonsterStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.HP = MonsterData.MonsterStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.MaxMP = MonsterData.MonsterStatInfo.MaxMP;
	_GameObjectInfo.ObjectStatInfo.MP = MonsterData.MonsterStatInfo.MP;
	_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = MonsterData.MonsterStatInfo.MinMeleeAttackDamage;
	_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = MonsterData.MonsterStatInfo.MaxMeleeAttackDamage;
	_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = MonsterData.MonsterStatInfo.MeleeAttackHitRate;
	_GameObjectInfo.ObjectStatInfo.MagicDamage = MonsterData.MonsterStatInfo.MagicDamage;
	_GameObjectInfo.ObjectStatInfo.MagicHitRate = MonsterData.MonsterStatInfo.MagicHitRate;
	_GameObjectInfo.ObjectStatInfo.Defence = MonsterData.MonsterStatInfo.Defence;
	_GameObjectInfo.ObjectStatInfo.EvasionRate = MonsterData.MonsterStatInfo.EvasionRate;
	_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = MonsterData.MonsterStatInfo.MeleeCriticalPoint;	
	_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = MonsterData.MonsterStatInfo.MagicCriticalPoint;	
	_GameObjectInfo.ObjectStatInfo.Speed = MonsterData.MonsterStatInfo.Speed;	
	_GameObjectInfo.ObjectStatInfo.MaxSpeed = MonsterData.MonsterStatInfo.Speed;

	_SearchCellDistance = MonsterData.MonsterStatInfo.SearchCellDistance;
	_ChaseCellDistance = MonsterData.MonsterStatInfo.ChaseCellDistance;
	_AttackRange = MonsterData.MonsterStatInfo.AttackRange;

	_SearchTickPoint = MonsterData.SearchTick;
	_PatrolTickPoint = MonsterData.PatrolTick;
	_AttackTickPoint = MonsterData.AttackTick;	

	_GetDPPoint = MonsterData.GetDPPoint;		
	_GetExpPoint = MonsterData.GetExpPoint;

	_FieldOfViewDistance = 10;

	_SpawnIdleTick = GetTickCount64() + 2000;
}

CBear::~CBear()
{
	
}

void CBear::Init(st_Vector2Int SpawnPosition)
{
	CMonster::Init(SpawnPosition);	

	_SpawnIdleTick = GetTickCount64() + 2000;
	_SearchTick = GetTickCount64() + _SearchTickPoint;
}

bool CBear::UpdateSpawnIdle()
{
	return CMonster::UpdateSpawnIdle();
}

void CBear::UpdateIdle()
{
	CMonster::UpdateIdle();
}

void CBear::UpdatePatrol()
{
	CMonster::UpdatePatrol();
}

void CBear::UpdateMoving()
{
	CMonster::UpdateMoving();
}

void CBear::UpdateAttack()
{
	CMonster::UpdateAttack();
}

void CBear::UpdateDead()
{

}

void CBear::PositionReset()
{
	switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
	{
	case en_MoveDir::LEFT:
		_GameObjectInfo.ObjectPositionInfo.Position._X =
			_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.3f;
		break;
	case en_MoveDir::RIGHT:
		_GameObjectInfo.ObjectPositionInfo.Position._X =
			_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.7f;
		break;
	case en_MoveDir::UP:
		_GameObjectInfo.ObjectPositionInfo.Position._Y =
			_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.7f;
		break;
	case en_MoveDir::DOWN:
		_GameObjectInfo.ObjectPositionInfo.Position._Y =
			_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.3f;
		break;
	}
}
