#include "pch.h"
#include "Goblin.h"
#include "ObjectManager.h"
#include "NetworkManager.h"
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

	_GetExpPoint = GoblinMonsterData.GetExpPoint;

	_ReSpawnTime = GoblinMonsterData.ReSpawnTime;

	_FieldOfViewDistance = 14;	

	_FieldOfAngle = 210;	
	
	_PlayerSearchDistance = 3;

	_MonsterSkillBox.SetOwner(this);
	_MonsterSkillBox.Init(_GameObjectInfo.ObjectType);		
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
		if (IsDead == true)
		{
			CPlayer* AttackerPlayer = dynamic_cast<CPlayer*>(Attacker);
			if (AttackerPlayer != nullptr)
			{
				if (AttackerPlayer->GetChannel() != nullptr)
				{
					AttackerPlayer->GetChannel()->ExperienceCalculate(AttackerPlayer, _GameObjectInfo.ObjectType, G_Datamanager->FindMonsterExperienceData(_GameObjectInfo.ObjectType));
				}
				else
				{
					CRASH("Channel nullptr")
				}
			}
			else
			{
				CRASH("Exp Get Not Player")
			}
			
			if (Attacker->_SelectTarget != nullptr && Attacker->_SelectTarget->_GameObjectInfo.ObjectId == _GameObjectInfo.ObjectId)
			{
				Attacker->_SelectTarget = nullptr;
			}			
		}

		_DeadTick = GetTickCount64() + 3000;

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ROOTING;		

		_GameObjectInfo.ObjectPositionInfo.MoveDirection = Vector2::Zero;			

		_RectCollision->DeadPositionUpdate();

		End();

		vector<st_FieldOfViewInfo> AroundPlayers = _Channel->GetMap()->GetFieldAroundPlayers(this);
		
		CMessage* ResDieMessagePacket = G_NetworkManager->GetGameServer()->MakePacketObjectDie(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.State);
		G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, ResDieMessagePacket);
		ResDieMessagePacket->Free();				
	}

	return IsDead;
}

void CGoblin::PositionReset()
{
	
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

void CGoblin::UpdateRooting()
{
	CMonster::UpdateRooting();
}

void CGoblin::UpdateDead()
{
	CMonster::UpdateDead();
}
