#include "pch.h"
#include "Goblin.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "RectCollision.h"

CGoblin::CGoblin()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_GOBLIN;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;

	auto MonsterStat = G_Datamanager->_Monsters.find(_GameObjectInfo.ObjectType);
	st_MonsterData GoblinMonsterData = *(*MonsterStat).second;

	_GameObjectInfo.ObjectName = (LPWSTR)CA2W(GoblinMonsterData.MonsterName.c_str());
	_GameObjectInfo.ObjectStatInfo = GoblinMonsterData.MonsterStatInfo;

	_GameObjectInfo.ObjectStatInfo.MaxSpeed = _GameObjectInfo.ObjectStatInfo.Speed;

	_GameObjectInfo.ObjectWidth = 1;
	_GameObjectInfo.ObjectHeight = 1;

	_SearchTickPoint = GoblinMonsterData.SearchTick;
	_PatrolTickPoint = GoblinMonsterData.PatrolTick;
	_AttackTickPoint = GoblinMonsterData.AttackTick;

	_GetDPPoint = GoblinMonsterData.GetDPPoint;
	_GetExpPoint = GoblinMonsterData.GetExpPoint;

	_ReSpawnTime = GoblinMonsterData.ReSpawnTime;

	_FieldOfViewDistance = 10;	

	_MonsterSkillBox.Init(_GameObjectInfo.ObjectType);

	_RectCollision = new CRectCollision(this);
}

CGoblin::~CGoblin()
{
}

void CGoblin::Start()
{
	CMonster::Start();

	_SpawnIdleTick = GetTickCount64() + 2000;
	_SearchTick = GetTickCount64() + _SearchTickPoint;
}

bool CGoblin::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	bool IsDead = CMonster::OnDamaged(Attacker, Damage);
	if (IsDead == true)
	{
		_DeadReadyTick = GetTickCount64() + 1500;

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::READY_DEAD;

		CMessage* ResChangeStatePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.State);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResChangeStatePacket);
		ResChangeStatePacket->Free();

		G_ObjectManager->ObjectItemSpawn(_Channel, Attacker->_GameObjectInfo.ObjectId,
			Attacker->_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.CollisionPosition,
			_GameObjectInfo.ObjectPositionInfo.Position,
			_GameObjectInfo.ObjectType, en_GameObjectType::OBJECT_GOBLIN);

		Attacker->_GameObjectInfo.ObjectStatInfo.DP += _GetDPPoint;

		if (Attacker->_GameObjectInfo.ObjectStatInfo.DP >= Attacker->_GameObjectInfo.ObjectStatInfo.MaxDP)
		{
			Attacker->_GameObjectInfo.ObjectStatInfo.DP = Attacker->_GameObjectInfo.ObjectStatInfo.MaxDP;
		}
	}

	return IsDead;
}

void CGoblin::PositionReset()
{
	if (_RectCollision != nullptr)
	{
		_RectCollision->CollisionUpdate();
	}	
}

bool CGoblin::UpdateSpawnIdle()
{
	return CMonster::UpdateSpawnIdle();
}

void CGoblin::UpdateIdle()
{
	CMonster::UpdateIdle();
}

void CGoblin::UpdatePatrol()
{
	CMonster::UpdatePatrol();
}

void CGoblin::UpdateMoving()
{
	CMonster::UpdateMoving();
}

void CGoblin::UpdateAttack()
{
	CMonster::UpdateAttack();
}

void CGoblin::UpdateReadyDead()
{
	CMonster::UpdateReadyDead();
}

void CGoblin::UpdateDead()
{
	CMonster::UpdateDead();
}
