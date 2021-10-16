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
	case en_CreatureState::SPAWN_IDLE:
		UpdateSpawnIdle();
		break;
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
	if (_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
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
}

void CMonster::Init(st_Vector2Int SpawnPosition)
{
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;

	_PatrolPositions = GetAroundCellPositions(GetCellPosition(), 2);
}

void CMonster::UpdateIdle()
{
	if (_SearchTick > GetTickCount64())
	{
		return;
	}

	_SearchTick = GetTickCount64() + _SearchTickPoint;

	bool Cango = false;
	CGameObject* Target = _Channel->FindNearPlayer(this, 1, &Cango);
	if (Target == nullptr)
	{
		_PatrolTick = GetTickCount64() + _PatrolTickPoint;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
	}
	else
	{
		int16 Distance = st_Vector2Int::Distance(Target->GetCellPosition(), GetCellPosition());
		// 타겟은 찾앗지만 추격할 수 있는 거리가 되지 않음
		if (Distance > _ChaseCellDistance)
		{
			// 정찰 ( 타겟은 있지만 거리가 안되서 갈 수 없어서 정찰 상태로 변경 )
			if (Cango == true)
			{
				_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
			}
			// 대기 ( 타겟은 있지만 다른 오브젝트에 의해 갈 수 없어서 대기 상태로 변경 )
			else
			{
				return;
			}
		}
		else
		{
			// 추격거리 도달 쫓음
			_Target = Target;
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
		}
	}
}

void CMonster::UpdatePatrol()
{
	if (_PatrolTick > GetTickCount64())
	{
		return;
	}	

	_PatrolTick = GetTickCount64() + _PatrolTickPoint;

	random_device Seed;
	mt19937 Gen(Seed());

	// 몬스터를 생성할때 정해준 스폰 위치를 기준으로 저장해둔 정찰 위치 중에서
	// 랜덤으로 정찰 위치를 얻는다.
	int8 MaxPatrolIndex = (int8)_PatrolPositions.size();
	uniform_int_distribution<int> RandomPatrolPoint(0, MaxPatrolIndex);
	int8 RandomIndex = RandomPatrolPoint(Gen);

	// 위에서 얻은 정찰위치까지의 길을 찾는다.
	st_Vector2Int MonsterPosition = GetCellPosition();
	vector<st_Vector2Int> Path = _Channel->_Map->FindPath(MonsterPosition, _PatrolPositions[RandomIndex]);
	if (Path.size() < 2)
	{
		// 정찰 위치로 이동 할 수 없을 경우 Idle상태로 바꿔준다.
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetDirectionFromVector(Path[1] - MonsterPosition);

	_Channel->_Map->ApplyMove(this, Path[1]);
	BroadCastPacket(en_PACKET_S2C_PATROL);
}

void CMonster::UpdateMoving()
{
	if (_MoveTick > GetTickCount64())
	{
		return;
	}

	int MoveTick = (int)(1000 / _GameObjectInfo.ObjectStatInfo.Speed);
	_MoveTick = GetTickCount64() + MoveTick;

	// 타겟이 없거나 나와 다른 채널에 있을 경우 Idle 상태로 전환한다.
	if (_Target == nullptr || _Target->_Channel != _Channel)
	{
		_Target = nullptr;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	st_Vector2Int TargetPosition = _Target->GetCellPosition();
	st_Vector2Int MonsterPosition = GetCellPosition();

	// 방향값 구한다.
	st_Vector2Int Direction = TargetPosition - MonsterPosition;
	// 타겟과 몬스터의 거리를 잰다.
	int32 Distance = st_Vector2Int::Distance(TargetPosition, MonsterPosition);
	// 타겟과의 거리가 0 또는 추격거리 보다 멀어지면
	if (Distance == 0 || Distance > _ChaseCellDistance)
	{
		_Target = nullptr;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	vector<st_Vector2Int> Path = _Channel->_Map->FindPath(MonsterPosition, TargetPosition);
	// 추격중에 플레이어한테 다가갈수 없거나 추격거리를 벗어나면 멈춘다.
	if (Path.size() < 2 || Distance > _ChaseCellDistance)
	{
		_Target = nullptr;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	// 대상과의 거리가 공격 범위안에 있고, 대각선에 있지 않을때 공격 상태로 바꾼다.
	if (Distance <= _AttackRange && (Direction._X == 0 || Direction._Y == 0))
	{
		_AttackTick = GetTickCount64() + _AttackTickPoint;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
		_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetDirectionFromVector(Direction);
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetDirectionFromVector(Path[1] - MonsterPosition);
	_Channel->_Map->ApplyMove(this, Path[1]);

	BroadCastPacket(en_PACKET_S2C_MOVE);
}

void CMonster::UpdateAttack()
{
	if (_AttackTick == 0)
	{
		// 타겟이 사라지거나 채널이 달라질 경우 타겟을 해제
		if (_Target == nullptr || _Target->_Channel != _Channel)
		{
			_Target = nullptr;
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			return;
		}

		// 공격이 가능한지 확인
		st_Vector2Int TargetCellPosition = _Target->GetCellPosition();
		st_Vector2Int MyCellPosition = GetCellPosition();
		st_Vector2Int Direction = TargetCellPosition - MyCellPosition;

		_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetDirectionFromVector(Direction);

		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);

		int32 Distance = st_Vector2Int::Distance(TargetCellPosition, MyCellPosition);
		// 타겟과의 거리가 공격 범위 안에 속하고 X==0 || Y ==0 일때( 대각선은 제한) 공격
		bool CanUseAttack = (Distance <= _AttackRange && (Direction._X == 0 || Direction._Y == 0));
		if (CanUseAttack == false)
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			return;
		}

		// 크리티컬 판단		
		random_device Seed;
		default_random_engine Eng(Seed());

		float CriticalPoint = _GameObjectInfo.ObjectStatInfo.CriticalPoint / 1000.0f;
		bernoulli_distribution CriticalCheck(CriticalPoint);
		bool IsCritical = CriticalCheck(Eng);

		// 데미지 판단
		mt19937 Gen(Seed());
		uniform_int_distribution<int> DamageChoiceRandom(_GameObjectInfo.ObjectStatInfo.MinAttackDamage, _GameObjectInfo.ObjectStatInfo.MaxAttackDamage);
		int32 ChoiceDamage = DamageChoiceRandom(Gen);
		int32 FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

		_Target->OnDamaged(this, FinalDamage);

		CMessage* ResBearAttackPacket = G_ObjectManager->GameServer->MakePacketResAttack(_GameObjectInfo.ObjectId, _Target->_GameObjectInfo.ObjectId, en_SkillType::SKILL_BEAR_NORMAL, FinalDamage, IsCritical);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResBearAttackPacket);
		ResBearAttackPacket->Free();

		// 주위 플레이어들에게 데미지 적용 결과 전송
		BroadCastPacket(en_PACKET_S2C_CHANGE_OBJECT_STAT);

		// 1.2초마다 공격
		_AttackTick = GetTickCount64() + _AttackTickPoint;

		wchar_t BearAttackMessage[64] = L"0";
		wsprintf(BearAttackMessage, L"%s이 일반 공격을 사용해 %s에게 %d의 데미지를 줬습니다", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

		wstring BearAttackString = BearAttackMessage;

		CMessage* ResSlimeSystemMessage = G_ObjectManager->GameServer->MakePacketResChattingBoxMessage(_Target->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, IsCritical ? st_Color::Red() : st_Color::White(), IsCritical ? L"치명타! " + BearAttackString : BearAttackString);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResSlimeSystemMessage);
		ResSlimeSystemMessage->Free();
	}

	if (_AttackTick > GetTickCount64())
	{
		return;
	}

	_AttackTick = 0;
}

void CMonster::UpdateSpawnIdle()
{
	if (_SpawnIdleTick > GetTickCount64())
	{
		return;
	}

	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;	
}
