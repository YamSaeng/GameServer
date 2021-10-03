#include "pch.h"
#include "Monster.h"
#include "Player.h"
#include "DataManager.h"
#include "ObjectManager.h"

CMonster::CMonster()
{
	_SearchTick = GetTickCount64();
	_MoveTick = GetTickCount64();	
	_PatrolTick = GetTickCount64();
}

CMonster::~CMonster()
{

}

void CMonster::Update()
{
	if (_Target && _Target->_NetworkState == en_ObjectNetworkState::LEAVE)
	{
		_Target = nullptr;		
	}

	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::IDLE:
		UpdateIdle();
		break;
	case en_CreatureState::PATROL:
		UpdatePatrol();
		break;
	case en_CreatureState::MOVING:
		UpdateMoving();
		break;
	case en_CreatureState::ATTACK:
		UpdateAttack();
		break;
	case en_CreatureState::DEAD:
		UpdateDead();
		break;
	default:
		break;
	}
}

void CMonster::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	CGameObject::OnDamaged(Attacker, Damage);
	
	_Target = (CPlayer*)Attacker;	
	
	if (_GameObjectInfo.ObjectStatInfo.HP == 0)
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;
		
		if (Attacker->_SelectTarget == this)
		{
			Attacker->_SelectTarget = nullptr;
		}

		OnDead(Attacker);
	}
}

void CMonster::Init(st_Vector2Int SpawnPosition)
{
	_PatrolPositions = GetAroundCellPositions(GetCellPosition(), 2);
}
