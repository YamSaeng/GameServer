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

	_SearchCellDistance = MonsterData.MonsterStatInfo.SearchCellDistance;
	_ChaseCellDistance = MonsterData.MonsterStatInfo.ChaseCellDistance;
	_AttackRange = MonsterData.MonsterStatInfo.AttackRange;

	_SearchTickPoint = MonsterData.SearchTick;
	_PatrolTickPoint = MonsterData.PatrolTick;
	_AttackTickPoint = MonsterData.AttackTick;	

	_GetDPPoint = MonsterData.GetDPPoint;		
	_GetExpPoint = MonsterData.GetExpPoint;

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

void CBear::UpdateSpawnIdle()
{
	CMonster::UpdateSpawnIdle();
}

void CBear::OnDead(CGameObject* Killer)
{
	G_ObjectManager->ItemSpawn(Killer->_GameObjectInfo.ObjectId, Killer->_GameObjectInfo.ObjectType, GetCellPosition(), _GameObjectInfo.ObjectType, en_ObjectDataType::BEAR_DATA);
	
	Killer->_GameObjectInfo.ObjectStatInfo.DP += _GetDPPoint;

	if (Killer->_GameObjectInfo.ObjectStatInfo.DP >= Killer->_GameObjectInfo.ObjectStatInfo.MaxDP)
	{
		Killer->_GameObjectInfo.ObjectStatInfo.DP = Killer->_GameObjectInfo.ObjectStatInfo.MaxDP;
	}

	BroadCastPacket(en_PACKET_S2C_OBJECT_STAT_CHANGE);
	BroadCastPacket(en_PACKET_S2C_DIE);		

	G_ObjectManager->GameServer->SpawnObjectTimeTimerJobCreate((int16)_GameObjectInfo.ObjectType, _SpawnPosition, 10000);

	G_ObjectManager->ObjectLeaveGame(this, _ObjectManagerIndex, 1);
}
