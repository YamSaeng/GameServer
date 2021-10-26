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

	auto FindMonsterStat = G_Datamanager->_Monsters.find(en_ObjectDataType::SLIME_DATA);
	st_MonsterData MonsterData = *(*FindMonsterStat).second;

	// ½ºÅÈ ¼ÂÆÃ	
	_GameObjectInfo.ObjectName = (LPWSTR)CA2W(MonsterData.MonsterName.c_str());
	_GameObjectInfo.ObjectStatInfo.MinAttackDamage = MonsterData.MonsterStatInfo.MinAttackDamage;
	_GameObjectInfo.ObjectStatInfo.MaxAttackDamage = MonsterData.MonsterStatInfo.MaxAttackDamage;
	_GameObjectInfo.ObjectStatInfo.CriticalPoint = MonsterData.MonsterStatInfo.CriticalPoint;
	_GameObjectInfo.ObjectStatInfo.MaxHP = MonsterData.MonsterStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.HP = MonsterData.MonsterStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.Level = MonsterData.MonsterStatInfo.Level;
	_GameObjectInfo.ObjectStatInfo.Speed = MonsterData.MonsterStatInfo.Speed;

	_SearchCellDistance = MonsterData.MonsterStatInfo.SearchCellDistance;
	_ChaseCellDistance = MonsterData.MonsterStatInfo.ChaseCellDistance;
	_AttackRange = MonsterData.MonsterStatInfo.AttackRange;

	_SearchTickPoint = MonsterData.SearchTick;
	_PatrolTickPoint = MonsterData.PatrolTick;
	_AttackTickPoint = MonsterData.AttackTick;

	_GetDPPoint = MonsterData.GetDPPoint;

	_SpawnIdleTick = GetTickCount64() + 2000;	
}

CSlime::~CSlime()
{
}

void CSlime::Init(st_Vector2Int SpawnPosition)
{
	CMonster::Init(SpawnPosition);	

	_SpawnIdleTick = GetTickCount64() + 2000;
	_SearchTick = GetTickCount64() + _SearchTickPoint;	
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

void CSlime::UpdateDead()
{

}

void CSlime::UpdateSpawnIdle()
{
	CMonster::UpdateSpawnIdle();
}

void CSlime::OnDead(CGameObject* Killer)
{
	G_ObjectManager->ItemSpawn(Killer->_GameObjectInfo.ObjectId, Killer->_GameObjectInfo.ObjectType, GetCellPosition(), _GameObjectInfo.ObjectType, en_ObjectDataType::SLIME_DATA);
	
	Killer->_GameObjectInfo.ObjectStatInfo.DP += _GetDPPoint;

	if (Killer->_GameObjectInfo.ObjectStatInfo.DP >= Killer->_GameObjectInfo.ObjectStatInfo.MaxDP)
	{
		Killer->_GameObjectInfo.ObjectStatInfo.DP = Killer->_GameObjectInfo.ObjectStatInfo.MaxDP;
	}

	BroadCastPacket(en_PACKET_S2C_CHANGE_OBJECT_STAT);
	BroadCastPacket(en_PACKET_S2C_DIE);		

	G_ObjectManager->GameServer->SpawnObjectTime((int16)_GameObjectInfo.ObjectType, _SpawnPosition , 10000);

	G_ObjectManager->Remove(this, 1);		
}

