#include "pch.h"
#include "Slime.h"
#include "DataManager.h"
#include "Player.h"
#include "ObjectManager.h"
#include <atlbase.h>

CSlime::CSlime()
{	
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_SLIME;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;

	auto FindMonsterStat = G_Datamanager->_Monsters.find(_GameObjectInfo.ObjectType);
	st_MonsterData MonsterData = *(*FindMonsterStat).second;

	// ½ºÅÈ ¼ÂÆÃ	
	_GameObjectInfo.ObjectName = (LPWSTR)CA2W(MonsterData.MonsterName.c_str());
	_GameObjectInfo.ObjectStatInfo.Level = MonsterData.MonsterStatInfo.Level;
	_GameObjectInfo.ObjectStatInfo.MaxHP = MonsterData.MonsterStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.HP = MonsterData.MonsterStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.MaxMP = MonsterData.MonsterStatInfo.MaxMP;
	_GameObjectInfo.ObjectStatInfo.MP = MonsterData.MonsterStatInfo.MaxMP;
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

	_GameObjectInfo.ObjectWidth = 1;
	_GameObjectInfo.ObjectHeight = 1;

	_SearchCellDistance = MonsterData.MonsterStatInfo.SearchCellDistance;
	_ChaseCellDistance = MonsterData.MonsterStatInfo.ChaseCellDistance;
	_AttackRange = MonsterData.MonsterStatInfo.AttackRange;

	_SearchTickPoint = MonsterData.SearchTick;
	_PatrolTickPoint = MonsterData.PatrolTick;
	_AttackTickPoint = MonsterData.AttackTick;

	_GetDPPoint = MonsterData.GetDPPoint;
	_GetExpPoint = MonsterData.GetExpPoint;

	_ReSpawnTime = MonsterData.ReSpawnTime;

	_FieldOfViewDistance = 10;

	_SpawnIdleTick = GetTickCount64() + 2000;		
}

CSlime::~CSlime()
{
}

void CSlime::Start()
{
	CMonster::Start();	

	_SpawnIdleTick = GetTickCount64() + 2000;
	_SearchTick = GetTickCount64() + _SearchTickPoint;	
}

bool CSlime::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	bool IsDead = CMonster::OnDamaged(Attacker, Damage);
	if (IsDead == true)
	{
		_DeadReadyTick = GetTickCount64() + 1500;

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::READY_DEAD;

		CMessage* ResChangeStatePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.MoveDir,
			_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.State);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResChangeStatePacket);
		ResChangeStatePacket->Free();

		G_ObjectManager->ObjectItemSpawn(Attacker->_GameObjectInfo.ObjectId,
			Attacker->_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.CollisionPosition,
			_GameObjectInfo.ObjectType, en_GameObjectType::OBJECT_SLIME);

		Attacker->_GameObjectInfo.ObjectStatInfo.DP += _GetDPPoint;

		if (Attacker->_GameObjectInfo.ObjectStatInfo.DP >= Attacker->_GameObjectInfo.ObjectStatInfo.MaxDP)
		{
			Attacker->_GameObjectInfo.ObjectStatInfo.DP = Attacker->_GameObjectInfo.ObjectStatInfo.MaxDP;
		}
	}

	return IsDead;
}

bool CSlime::UpdateSpawnIdle()
{
	return CMonster::UpdateSpawnIdle();
}

void CSlime::UpdateIdle()
{
	CMonster::UpdateIdle();
}

void CSlime::UpdatePatrol()
{
	CMonster::UpdatePatrol();
}

void CSlime::UpdateMoving()
{
	CMonster::UpdateMoving();
}

void CSlime::UpdateAttack()
{
	CMonster::UpdateAttack();
}

void CSlime::UpdateReadyDead()
{
	CMonster::UpdateReadyDead();
}

void CSlime::UpdateDead()
{
	CMonster::UpdateDead();
}

void CSlime::PositionReset()
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

