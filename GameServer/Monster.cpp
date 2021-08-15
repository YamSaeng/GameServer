#include "pch.h"
#include "Monster.h"
#include "Player.h"
#include "DataManager.h"

CMonster::CMonster()
{
	_NextSearchTick = GetTickCount64();
	_NextMoveTick = GetTickCount64();

	_GameObjectInfo.ObjectType = en_GameObjectType::MONSTER;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
	
	auto FindMonsterStat = G_Datamanager->_Monsters.find(1);
	st_MonsterData MonsterData = *(*FindMonsterStat).second;
	
	// ½ºÅÈ ¼ÂÆÃ
	_GameObjectInfo.ObjectName.assign(MonsterData._MonsterName.begin(),MonsterData._MonsterName.end());
	_GameObjectInfo.ObjectStatInfo.Attack = MonsterData._MonsterStatInfo.Attack;
	_GameObjectInfo.ObjectStatInfo.MaxHP = MonsterData._MonsterStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.HP = MonsterData._MonsterStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.Level = MonsterData._MonsterStatInfo.Level;
	_GameObjectInfo.ObjectStatInfo.Speed = MonsterData._MonsterStatInfo.Speed;

	_SearchCellDistance = 10;
}

CMonster::~CMonster()
{

}

void CMonster::Init(int32 DataSheetId)
{
	_DataSheetId = DataSheetId;	

	
}

void CMonster::Update()
{
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::IDLE:
		UpdateIdle();
		break;
	case en_CreatureState::MOVING:
		UpdateMoving();
		break;
	case en_CreatureState::ATTACK:
		break;
	case en_CreatureState::DEAD:
		break;
	default:
		break;
	}
}

void CMonster::UpdateIdle()
{
	if (_NextSearchTick > GetTickCount64())
	{
		return;
	}

	_NextSearchTick = GetTickCount64() + 1000;
		
	CPlayer* Target = _Channel->FindNearPlayer(this, _SearchCellDistance);

	if (Target == nullptr)
	{
		return;
	}

	_Target = Target;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
}

void CMonster::UpdateMoving()
{
	if (_NextMoveTick > GetTickCount64())
	{
		return;
	}

	int MoveTick = (int)(1000 / _GameObjectInfo.ObjectStatInfo.Speed);
	_NextMoveTick = GetTickCount64() + MoveTick;

	G_Logger->WriteStdOut(en_Color::RED, L"UpdateMoving Func CAll\n");	
}

void CMonster::UpdateAttack()
{
}

void CMonster::UpdateDead()
{
}

void CMonster::OnDamaged(CGameObject* Attacker, int32 Damage)
{
}

void CMonster::OnDead(CGameObject* Killer)
{
}