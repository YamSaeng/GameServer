#include "pch.h"
#include "Monster.h"
#include "Player.h"
#include "DataManager.h"
#include "ObjectManager.h"
#include "Skill.h"

CMonster::CMonster()
{
	_SearchTick = GetTickCount64();
	_MoveTick = GetTickCount64();
	_PatrolTick = GetTickCount64();

	_MonsterState = en_MonsterState::MONSTER_IDLE;
	_MovePoint = st_Vector2::Zero();
	_PatrolPoint = st_Vector2::Zero();

	_DeadTick = 0;

	_Target = nullptr;	
}

CMonster::~CMonster()
{

}

void CMonster::Update()	
{
	CGameObject::Update();

	if (_Target != nullptr && _Target->_NetworkState == en_ObjectNetworkState::LEAVE)
	{
		_Target = nullptr;
	}

	for (auto BufSkillIterator : _Bufs)
	{
		bool DeleteBufSkill = BufSkillIterator.second->Update();
		if (DeleteBufSkill)
		{
			DeleteBuf(BufSkillIterator.first);
			G_ObjectManager->SkillInfoReturn(BufSkillIterator.second->GetSkillInfo()->SkillMediumCategory,
				BufSkillIterator.second->GetSkillInfo());
			G_ObjectManager->SkillReturn(BufSkillIterator.second);
		}
	}

	for (auto DebufSkillIterator : _DeBufs)
	{
		bool DeleteDebufSkill = DebufSkillIterator.second->Update();
		if (DeleteDebufSkill)
		{
			DeleteDebuf(DebufSkillIterator.first);
			G_ObjectManager->SkillInfoReturn(DebufSkillIterator.second->GetSkillInfo()->SkillMediumCategory,
				DebufSkillIterator.second->GetSkillInfo());
			G_ObjectManager->SkillReturn(DebufSkillIterator.second);
		}
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
		switch (_MonsterState)
		{
		case en_MonsterState::MONSTER_READY_PATROL:
			ReadyPatrol();
			break;
		case en_MonsterState::MONSTER_PATROL:
			UpdatePatrol();
			break;
		}
		break;
	case en_CreatureState::MOVING:
		switch (_MonsterState)
		{
		case en_MonsterState::MONSTER_IDLE:
		case en_MonsterState::MONSTER_READY_MOVE:
			ReadMoving();
			break;
		case en_MonsterState::MONSTER_MOVE:
			UpdateMoving();
			break;
		}
		break;
	case en_CreatureState::RETURN_SPAWN_POSITION:
		UpdateReturnSpawnPosition();
		break;
	case en_CreatureState::ATTACK:
		UpdateAttack();
		break;
	case en_CreatureState::SPELL:
		UpdateSpell();
		break;			
	case en_CreatureState::READY_DEAD:
		UpdateReadyDead();
	case en_CreatureState::DEAD:
		UpdateDead();
		break;
	}

	SelectTarget();
}

void CMonster::PositionReset()
{
}

bool CMonster::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	if (_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE
		|| _GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::RETURN_SPAWN_POSITION)
	{
		CGameObject::OnDamaged(Attacker, Damage);

		st_GameObjectJob* AggroJob = G_ObjectManager->GameObjectJobCreate();
		AggroJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_AGGRO_LIST_INSERT_OR_UPDATE;

		CGameServerMessage* AggroJobMessage = CGameServerMessage::GameServerMessageAlloc();
		AggroJobMessage->Clear();

		*AggroJobMessage << (int8)en_AggroCategory::AGGRO_CATEGORY_DAMAGE;
		*AggroJobMessage << &Attacker;
		*AggroJobMessage << Damage;

		AggroJob->GameObjectJobMessage = AggroJobMessage;

		_GameObjectJobQue.Enqueue(AggroJob);

		if (_GameObjectInfo.ObjectStatInfo.HP == 0)
		{
			_DeadTick = GetTickCount64() + 1000;

			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::READY_DEAD;

			G_ObjectManager->ItemSpawn(Attacker->_GameObjectInfo.ObjectId, Attacker->_GameObjectInfo.ObjectType, GetCellPosition(), _GameObjectInfo.ObjectType, en_ObjectDataType::SLIME_DATA);

			Attacker->_GameObjectInfo.ObjectStatInfo.DP += _GetDPPoint;

			if (Attacker->_GameObjectInfo.ObjectStatInfo.DP >= Attacker->_GameObjectInfo.ObjectStatInfo.MaxDP)
			{
				Attacker->_GameObjectInfo.ObjectStatInfo.DP = Attacker->_GameObjectInfo.ObjectStatInfo.MaxDP;
			}

			SendMonsterChangeObjectState();			
			return true; 
		}
	}

	return false;
}

void CMonster::Init(st_Vector2Int SpawnPosition)
{
	_SpawnPosition = SpawnPosition;

	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;

	_PatrolPositions = GetAroundCellPositions(GetCellPosition(), 1);
}

void CMonster::SelectTarget()
{
	float AggroPoint = 0.0f;
	CGameObject* Target = nullptr;

	for (auto AggroTargetIterator : _AggroTargetList)
	{
		if (AggroTargetIterator.second.AggroPoint > AggroPoint)
		{
			AggroPoint = AggroTargetIterator.second.AggroPoint;
			Target = AggroTargetIterator.second.AggroTarget;
		}
	}

	_Target = Target;
}

CGameObject* CMonster::FindTarget()
{
	bool Cango = false;
	CGameObject* Target = _Channel->GetMap()->FindNearPlayer(this, 1, &Cango);
	if (Target == nullptr)
	{
		_PatrolTick = GetTickCount64() + _PatrolTickPoint;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
		_MonsterState = en_MonsterState::MONSTER_READY_PATROL;

		SendMonsterChangeObjectState();
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
				_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
				SendMonsterChangeObjectState();
			}
			// 대기 ( 타겟은 있지만 다른 오브젝트에 의해 갈 수 없어서 대기 상태로 변경 )
			else
			{
				return nullptr;
			}
		}
		else
		{
			// 추격거리 도달 쫓음			
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
			SendMonsterChangeObjectState();
		}
	}

	return Target;
}

bool CMonster::UpdateSpawnIdle()
{
	bool ChangeToIdle = CGameObject::UpdateSpawnIdle();

	if (ChangeToIdle)
	{
		SendMonsterChangeObjectState();
	}	

	return ChangeToIdle;
}

void CMonster::UpdateIdle()
{
	bool IsChohoneStun = _StatusAbnormal & STATUS_ABNORMAL_WARRIOR_CHOHONE;
	bool IsLightNingStun = _StatusAbnormal & STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE;
	bool IsShaeHoneRoot = _StatusAbnormal & STATUS_ABNORMAL_WARRIOR_SHAEHONE;
	bool IsShmanRoot = _StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ROOT;
	bool IsTaioistRoot = _StatusAbnormal & STATUS_ABNORMAL_TAIOIST_ROOT;

	if (IsChohoneStun == true
		|| IsLightNingStun == true
		|| IsShaeHoneRoot == true
		|| IsShmanRoot == true
		|| IsTaioistRoot == true)
	{
		return;
	}


	if (_SearchTick > GetTickCount64())
	{
		return;
	}

	_SearchTick = GetTickCount64() + _SearchTickPoint;

	_PatrolTick = GetTickCount64() + _PatrolTickPoint;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
	_MonsterState = en_MonsterState::MONSTER_READY_PATROL;

	SendMonsterChangeObjectState();
}

void CMonster::ReadyPatrol()
{
	if (_PatrolTick > GetTickCount64())
	{
		return;
	}

	CGameObject* Target = FindTarget();
	if (Target != nullptr)
	{
		int16 Distance = st_Vector2Int::Distance(Target->GetCellPosition(), GetCellPosition());
		if (Distance <= _ChaseCellDistance) // 대상이 추적 거리 안에 있는지 확인한다.
		{
			st_Aggro Aggro;
			Aggro.AggroTarget = Target;
			Aggro.AggroPoint = _GameObjectInfo.ObjectStatInfo.MaxHP * G_Datamanager->_MonsterAggroData.MonsterAggroFirstTarget;

			_AggroTargetList.insert(pair<int64, st_Aggro>(Target->_GameObjectInfo.ObjectId, Aggro));

			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			_MonsterState = en_MonsterState::MONSTER_READY_MOVE;

			return;
		}

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
	}

	random_device Seed;
	mt19937 Gen(Seed());

	do
	{
		// 몬스터를 생성할때 정해준 스폰 위치를 기준으로 저장해둔 정찰 위치 중
		// 랜덤으로 정찰 위치를 얻는다.
		int8 MaxPatrolIndex = (int8)_PatrolPositions.size();
		uniform_int_distribution<int> RandomPatrolPoint(0, MaxPatrolIndex - 1);
		int8 RandomIndex = RandomPatrolPoint(Gen);

		// 앞서 얻은 정찰위치까지의 길을 찾는다.
		st_Vector2Int MonsterPosition;
		MonsterPosition._X = _GameObjectInfo.ObjectPositionInfo.CollisionPositionX;
		MonsterPosition._Y = _GameObjectInfo.ObjectPositionInfo.CollisionPositionY;

		if (_PatrolPositions[RandomIndex] == MonsterPosition)
		{
			continue;
		}

		vector<st_Vector2Int> Path = _Channel->GetMap()->FindPath(this, MonsterPosition, _PatrolPositions[RandomIndex]);
		if (Path.size() < 2)
		{
			// 정찰 위치로 이동 할 수 없을 경우 상태값을 IDLE로 초기화한다.
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			_MonsterState = en_MonsterState::MONSTER_IDLE;
			SendMonsterChangeObjectState();
			return;
		}

		// 정찰 위치를 저장한다
		_MonsterState = en_MonsterState::MONSTER_PATROL;
		_PatrolPoint = PositionCheck(Path[1]);
	} while (0);
}

void CMonster::UpdatePatrol()
{
	st_Vector2 MonsterPosition;
	MonsterPosition._X = _GameObjectInfo.ObjectPositionInfo.PositionX;
	MonsterPosition._Y = _GameObjectInfo.ObjectPositionInfo.PositionY;

	st_Vector2 DirectionVector = _PatrolPoint - MonsterPosition;
	st_Vector2 NormalVector = st_Vector2::Normalize(DirectionVector);

	_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2::GetMoveDir(NormalVector);

	/*switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
	{
	case en_MoveDir::UP:
		G_Logger->WriteStdOut(en_Color::GREEN, L"Dir : 위 ");
		break;
	case en_MoveDir::DOWN:
		G_Logger->WriteStdOut(en_Color::GREEN, L"Dir : 아래 ");
		break;
	case en_MoveDir::LEFT:
		G_Logger->WriteStdOut(en_Color::GREEN, L"Dir : 왼쪽 ");
		break;
	case en_MoveDir::RIGHT:
		G_Logger->WriteStdOut(en_Color::GREEN, L"Dir : 오른쪽 ");
		break;
	}

	G_Logger->WriteStdOut(en_Color::RED, L"PositionX [%0.1f] PositionY [%0.1f] GoalPositionX [%0.1f] GoalPosition [%0.1f]  \n",
		_GameObjectInfo.ObjectPositionInfo.PositionX,
		_GameObjectInfo.ObjectPositionInfo.PositionY,
		_PatrolPoint._X,
		_PatrolPoint._Y);*/

	Move();
}

void CMonster::ReadMoving()
{
	do
	{
		// 벽이나 다른 몬스터에게 부딪혓을 경우 잠시 동안 대기 한후 다시 진행 
		if (_MonsterState == en_MonsterState::MONSTER_IDLE)
		{
			if (_MoveTick > GetTickCount64())
			{
				return;
			}
			else
			{
				break;
			}
		}

	} while (0);

	if (_Target == nullptr)
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		_MonsterState = en_MonsterState::MONSTER_IDLE;
		return;
	}

	st_Vector2Int MonsterPosition;
	MonsterPosition._X = _GameObjectInfo.ObjectPositionInfo.CollisionPositionX;
	MonsterPosition._Y = _GameObjectInfo.ObjectPositionInfo.CollisionPositionY;

	st_Vector2Int TargetPosition;
	TargetPosition._X = _Target->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX;
	TargetPosition._Y = _Target->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY;

	// 타겟 위치까지 길을 찾는다.
	vector<st_Vector2Int> Path = _Channel->GetMap()->FindPath(this, MonsterPosition, TargetPosition);
	if (Path.size() < 2)
	{
		// 타겟 위치로 이동 할 수 없을 경우 상태값을 IDLE로 초기화한다.
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		_MonsterState = en_MonsterState::MONSTER_IDLE;
		SendMonsterChangeObjectState();
		return;
	}

	// 다음 칸 위치가 타겟이라면
	if (Path[1] == TargetPosition)
	{
		st_Vector2 TargetPosition;
		TargetPosition._X = _Target->_GameObjectInfo.ObjectPositionInfo.PositionX;
		TargetPosition._Y = _Target->_GameObjectInfo.ObjectPositionInfo.PositionY;

		st_Vector2 MonsterPosition;
		MonsterPosition._X = _GameObjectInfo.ObjectPositionInfo.PositionX;
		MonsterPosition._Y = _GameObjectInfo.ObjectPositionInfo.PositionY;

		st_Vector2 DirectionVector = TargetPosition - MonsterPosition;
		st_Vector2 NormalVector = st_Vector2::Normalize(DirectionVector);

		en_MoveDir Dir = st_Vector2::GetMoveDir(NormalVector);
		_GameObjectInfo.ObjectPositionInfo.MoveDir = Dir;

		// 공격 상태로 바꾼다.
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
		_MonsterState = en_MonsterState::MONSTER_ATTACK;

		_DefaultAttackTick = GetTickCount64() + _AttackTickPoint;

		SendMonsterChangeObjectState();
	}
	else
	{
		_MonsterState = en_MonsterState::MONSTER_MOVE;
		_MovePoint = PositionCheck(Path[1]);
	}
}

void CMonster::UpdateMoving()
{
	// 목표가 사라질 경우
	if (_Target == nullptr)
	{
		// 소환 했던 위치로 되돌아 간다.
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::RETURN_SPAWN_POSITION;
		_MonsterState = en_MonsterState::MONSTER_MOVE;		
		return;
	}	

	// 목표와의 거리를 잰 후 거리가 추적 거리보다 크거나 같고 소환 위치에서 일정 거리 떨어져 있을 경우 소환 했던 위치로 되돌아 간다.
	int16 SpawnPositionDistance = st_Vector2Int::Distance(_SpawnPosition, GetCellPosition());

	int16 TargetDistance = st_Vector2Int::Distance(_Target->GetCellPosition(), GetCellPosition());
	if (TargetDistance >= _ChaseCellDistance && SpawnPositionDistance >= 5)
	{	
		_Target = nullptr;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::RETURN_SPAWN_POSITION;
		_MonsterState = en_MonsterState::MONSTER_MOVE;		
		return;
	}

	bool IsChohoneStun = _StatusAbnormal & STATUS_ABNORMAL_WARRIOR_CHOHONE;
	bool IsLightNingStun = _StatusAbnormal & STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE;
	bool IsShaeHoneRoot = _StatusAbnormal & STATUS_ABNORMAL_WARRIOR_SHAEHONE;
	bool IsShmanRoot = _StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ROOT;
	bool IsTaioistRoot = _StatusAbnormal & STATUS_ABNORMAL_TAIOIST_ROOT;

	// 기절 상태일 경우 
	if (IsChohoneStun || IsLightNingStun)
	{
		return;
	}

	// 발묶임 상태일 경우
	if (IsShaeHoneRoot || IsShmanRoot || IsTaioistRoot)
	{
		if (_Target != nullptr)
		{
			TargetAttackCheck();
		}
		else
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			_MonsterState = en_MonsterState::MONSTER_IDLE;
		}

		return;
	}

	st_Vector2 MonsterPosition = GetPosition();

	st_Vector2 DirectionVector = _MovePoint - MonsterPosition;
	st_Vector2 NormalVector = st_Vector2::Normalize(DirectionVector);

	//G_Logger->WriteStdOut(en_Color::RED, L"X : %0.1f Y : %0.1f\n", NormalVector._X, NormalVector._Y);

	_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2::GetMoveDir(NormalVector);

	/*
	G_Logger->WriteStdOut(en_Color::RED, L"PositionX [%0.1f] PositionY [%0.1f] GoalPositionX [%0.1f] GoalPosition [%0.1f]  \n",
		_GameObjectInfo.ObjectPositionInfo.PositionX,
		_GameObjectInfo.ObjectPositionInfo.PositionY,
		_MovePoint._X,
		_MovePoint._Y);*/

	Move();

	if (_Target != nullptr)
	{
		TargetAttackCheck();
	}
	else
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		_MonsterState = en_MonsterState::MONSTER_IDLE;
	}		
}

void CMonster::UpdateReturnSpawnPosition()
{
	st_Vector2Int MonsterPosition = GetCellPosition();

	// ReturnSpawnPosition에 도착하면 Idle상태로 전환한다.
	if (st_Vector2Int::Distance(_SpawnPosition, MonsterPosition) <= 2)
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		_MonsterState = en_MonsterState::MONSTER_IDLE;

		SendMonsterChangeObjectState();
		return;
	}

	vector<st_Vector2Int> Path;

	for (st_Vector2Int ReturnSpawnPosition : _PatrolPositions)
	{
		Path = _Channel->GetMap()->FindPath(this, MonsterPosition, ReturnSpawnPosition);
		if (Path.size() < 2)
		{
			continue;
		}

		break;
	}

	st_Vector2 MonsterPositionFloat = GetPosition();
	st_Vector2 DirectionVector = PositionCheck(Path[1]) - MonsterPositionFloat;
	st_Vector2 NormalVector = st_Vector2::Normalize(DirectionVector);

	_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2::GetMoveDir(NormalVector);

	Move();		
}

void CMonster::UpdateAttack()
{
	bool IsChohoneStun = _StatusAbnormal & STATUS_ABNORMAL_WARRIOR_CHOHONE;
	bool IsLightNingStun = _StatusAbnormal & STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE;

	if (IsChohoneStun == true || IsLightNingStun == true)
	{
		return;
	}

	if (_DefaultAttackTick == 0)
	{
		// 목표물이 사라지거나 죽음 준비 또는 죽음 상태일 경우 목표물 해제하고 제자리로 돌아감
		if (_Target == nullptr 
			|| _Target->_NetworkState == en_ObjectNetworkState::LEAVE
			|| _Target->_NetworkState == en_ObjectNetworkState::READY
			|| _Target->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::READY_DEAD
			|| _Target->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::DEAD)
		{
			_Target = nullptr;
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::RETURN_SPAWN_POSITION;
			_MonsterState = en_MonsterState::MONSTER_MOVE;				
			return;
		}

		// 공격이 가능한지 확인
		st_Vector2Int TargetCellPosition = _Target->GetCellPosition();
		st_Vector2Int MyCellPosition = GetCellPosition();
		st_Vector2Int Direction = TargetCellPosition - MyCellPosition;

		_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetMoveDir(Direction);

		SendMonsterChangeObjectState();

		int32 Distance = st_Vector2Int::Distance(TargetCellPosition, MyCellPosition);
		// 타겟과의 거리가 공격 범위 안에 속하고 X==0 || Y ==0 일때( 대각선은 제한) 공격
		bool CanUseAttack = (Distance <= _AttackRange && (Direction._X == 0 || Direction._Y == 0));
		if (CanUseAttack == false)
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
			return;
		}

		// 크리티컬 판단		
		random_device Seed;
		default_random_engine Eng(Seed());

		float CriticalPoint = _GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint / 1000.0f;
		bernoulli_distribution CriticalCheck(CriticalPoint);
		bool IsCritical = CriticalCheck(Eng);

		// 데미지 판단
		mt19937 Gen(Seed());
		uniform_int_distribution<int> DamageChoiceRandom(_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage, _GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage);
		int32 ChoiceDamage = DamageChoiceRandom(Gen);
		int32 FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

		bool TargetIsDead = _Target->OnDamaged(this, FinalDamage);

		CMessage* ResBearAttackPacket = G_ObjectManager->GameServer->MakePacketResAttack(_GameObjectInfo.ObjectId,
			_Target->_GameObjectInfo.ObjectId,
			en_SkillType::SKILL_SLIME_NORMAL,
			FinalDamage, IsCritical);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResBearAttackPacket);
		ResBearAttackPacket->Free();

		wstring SlimeAttackAnimation = L"SLIME_ATTACK";

		// 공격 애니메이션 출력
		CMessage* AnimationPlayPacket = G_ObjectManager->GameServer->MakePacketResAnimationPlay(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.MoveDir, SlimeAttackAnimation);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, AnimationPlayPacket);
		AnimationPlayPacket->Free();

		// 주위 플레이어들에게 데미지 적용 결과 전송
		CMessage* ResChangeObjectStatPacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_Target->_GameObjectInfo.ObjectId, _Target->_GameObjectInfo.ObjectStatInfo);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResChangeObjectStatPacket);
		ResChangeObjectStatPacket->Free();

		// 1.2초마다 공격
		_DefaultAttackTick = GetTickCount64() + _AttackTickPoint;

		wchar_t BearAttackMessage[64] = L"0";
		wsprintf(BearAttackMessage, L"%s이 일반 공격을 사용해 %s에게 %d의 데미지를 줬습니다", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

		wstring BearAttackString = BearAttackMessage;

		CMessage* ResSlimeSystemMessage = G_ObjectManager->GameServer->MakePacketResChattingBoxMessage(_Target->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, IsCritical ? st_Color::Red() : st_Color::White(), IsCritical ? L"치명타! " + BearAttackString : BearAttackString);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResSlimeSystemMessage);
		ResSlimeSystemMessage->Free();
	}

	if (_DefaultAttackTick > GetTickCount64())
	{
		return;
	}

	_DefaultAttackTick = 0;
}

void CMonster::UpdateSpell()
{

}

void CMonster::UpdateReadyDead()
{		
	// 죽음 준비 상태에 진입하면 지정해준 시간 만큼 대기 후 로직 진행
	if (_DeadTick > GetTickCount64())
	{
		return;
	}

	// 시야 범위 내 대상들에게 대상이 죽었다고 알림
	CMessage* SelectTargetDeadPacket = G_ObjectManager->GameServer->MakePacketObjectDie(_GameObjectInfo.ObjectId);
	G_ObjectManager->GameServer->SendPacketFieldOfView(this, SelectTargetDeadPacket);
	SelectTargetDeadPacket->Free();

	// 소환 장소에서 재 소환 될 수 있도록 예약
	G_ObjectManager->GameServer->SpawnObjectTimeTimerJobCreate((int16)_GameObjectInfo.ObjectType, _SpawnPosition, 3000);

	// 접속중인 채널에서 최종적으로 퇴장
	G_ObjectManager->ObjectLeaveGame(this, _ObjectManagerArrayIndex, 1);

	// 상태값 DEAD로 바꿔서 로직 쓰레드에서 업데이트 대상이 되지 않도록 해줌
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;
}

void CMonster::UpdateDead()
{
	
}

void CMonster::Move()
{
	switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
	{
	case en_MoveDir::UP:
		_GameObjectInfo.ObjectPositionInfo.PositionY += (st_Vector2::Up()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::DOWN:
		_GameObjectInfo.ObjectPositionInfo.PositionY += (st_Vector2::Down()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::LEFT:
		_GameObjectInfo.ObjectPositionInfo.PositionX += (st_Vector2::Left()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::RIGHT:
		_GameObjectInfo.ObjectPositionInfo.PositionX += (st_Vector2::Right()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	}

	bool CanMove = _Channel->GetMap()->Cango(this, _GameObjectInfo.ObjectPositionInfo.PositionX, _GameObjectInfo.ObjectPositionInfo.PositionY);
	if (CanMove == true)
	{
		st_Vector2Int CollisionPosition;
		CollisionPosition._X = _GameObjectInfo.ObjectPositionInfo.PositionX;
		CollisionPosition._Y = _GameObjectInfo.ObjectPositionInfo.PositionY;

		if (CollisionPosition._X != _GameObjectInfo.ObjectPositionInfo.CollisionPositionX
			|| CollisionPosition._Y != _GameObjectInfo.ObjectPositionInfo.CollisionPositionY)
		{
			_Channel->GetMap()->ApplyMove(this, CollisionPosition);
		}

		switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
		{
		case en_MoveDir::UP:
			switch (_GameObjectInfo.ObjectPositionInfo.State)
			{
			case en_CreatureState::PATROL:
				if (_GameObjectInfo.ObjectPositionInfo.PositionY >= _PatrolPoint._Y)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
					_PatrolTick = GetTickCount64() + _PatrolTickPoint;
					CanMove = false;
				}
				break;
			case en_CreatureState::MOVING:
				if (_GameObjectInfo.ObjectPositionInfo.PositionY >= _MovePoint._Y)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
				}
				break;
			case en_CreatureState::ATTACK:
				if (abs(_Target->_GameObjectInfo.ObjectPositionInfo.PositionY - _GameObjectInfo.ObjectPositionInfo.PositionY) <= 0.5f)
				{
					_MonsterState = en_MonsterState::MONSTER_ATTACK;
				}
				break;
			}
			break;
		case en_MoveDir::DOWN:
			switch (_GameObjectInfo.ObjectPositionInfo.State)
			{
			case en_CreatureState::PATROL:
				if (_GameObjectInfo.ObjectPositionInfo.PositionY <= _PatrolPoint._Y)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
					_PatrolTick = GetTickCount64() + _PatrolTickPoint;
					CanMove = false;
				}
				break;
			case en_CreatureState::MOVING:
				if (_GameObjectInfo.ObjectPositionInfo.PositionY <= _MovePoint._Y)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
				}
				break;
			case en_CreatureState::ATTACK:
				if (abs(_Target->_GameObjectInfo.ObjectPositionInfo.PositionY - _GameObjectInfo.ObjectPositionInfo.PositionY) <= 0.5f)
				{
					_MonsterState = en_MonsterState::MONSTER_ATTACK;
				}
				break;
			}
			break;
		case en_MoveDir::LEFT:
			switch (_GameObjectInfo.ObjectPositionInfo.State)
			{
			case en_CreatureState::PATROL:
				if (_GameObjectInfo.ObjectPositionInfo.PositionX <= _PatrolPoint._X)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
					_PatrolTick = GetTickCount64() + _PatrolTickPoint;
					CanMove = false;
				}
				break;
			case en_CreatureState::MOVING:
				if (_GameObjectInfo.ObjectPositionInfo.PositionX <= _MovePoint._X)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
				}
				break;
			case en_CreatureState::ATTACK:
				if (abs(_Target->_GameObjectInfo.ObjectPositionInfo.PositionX - _GameObjectInfo.ObjectPositionInfo.PositionX) <= 0.5f)
				{
					_MonsterState = en_MonsterState::MONSTER_ATTACK;
				}
				break;
			}
			break;
		case en_MoveDir::RIGHT:
			switch (_GameObjectInfo.ObjectPositionInfo.State)
			{
			case en_CreatureState::PATROL:
				if (_GameObjectInfo.ObjectPositionInfo.PositionX >= _PatrolPoint._X)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
					_PatrolTick = GetTickCount64() + _PatrolTickPoint;
					CanMove = false;
				}
				break;
			case en_CreatureState::MOVING:
				if (_GameObjectInfo.ObjectPositionInfo.PositionX >= _MovePoint._X)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
				}
				break;
			case en_CreatureState::ATTACK:
				if (abs(_Target->_GameObjectInfo.ObjectPositionInfo.PositionX - _GameObjectInfo.ObjectPositionInfo.PositionX) <= 0.5f)
				{
					_MonsterState = en_MonsterState::MONSTER_ATTACK;
				}
				break;
			}
			break;
		}
	}
	else
	{
		switch (_GameObjectInfo.ObjectPositionInfo.State)
		{
		case en_CreatureState::PATROL:
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			_MonsterState = en_MonsterState::MONSTER_IDLE;
			break;
		case en_CreatureState::MOVING:
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			_MonsterState = en_MonsterState::MONSTER_IDLE;		
			_MoveTick = GetTickCount64() + 300;
			break;
		case en_CreatureState::ATTACK:
			_MonsterState = en_MonsterState::MONSTER_ATTACK;
			break;
		}

		PositionReset();
	}

	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::PATROL:
		{
			CMessage* MonsterPatrolPacket = G_ObjectManager->GameServer->MakePacketPatrol(_GameObjectInfo.ObjectId,
				_GameObjectInfo.ObjectType,
				CanMove,
				_GameObjectInfo.ObjectPositionInfo,
				_MonsterState);
			G_ObjectManager->GameServer->SendPacketFieldOfView(this, MonsterPatrolPacket);
			MonsterPatrolPacket->Free();
		}
		break;
	case en_CreatureState::MOVING:
		{
			CMessage* MonsterMovePacket = G_ObjectManager->GameServer->MakePacketResMonsterMove(_GameObjectInfo.ObjectId,
				_GameObjectInfo.ObjectType,
				CanMove,
				_GameObjectInfo.ObjectPositionInfo,
				_MonsterState);
			G_ObjectManager->GameServer->SendPacketFieldOfView(this, MonsterMovePacket);
			MonsterMovePacket->Free();
		}
		break;
	case en_CreatureState::ATTACK:
		{
			CMessage* MonsterMovePacket = G_ObjectManager->GameServer->MakePacketResMonsterMove(_GameObjectInfo.ObjectId,
				_GameObjectInfo.ObjectType,
				CanMove,
				_GameObjectInfo.ObjectPositionInfo,
				_MonsterState);
			G_ObjectManager->GameServer->SendPacketFieldOfView(this, MonsterMovePacket);
			MonsterMovePacket->Free();
		}
		break;
	case en_CreatureState::RETURN_SPAWN_POSITION:
		{
			CMessage* MonsterMovePacket = G_ObjectManager->GameServer->MakePacketResMonsterMove(_GameObjectInfo.ObjectId,
				_GameObjectInfo.ObjectType,
				CanMove,
				_GameObjectInfo.ObjectPositionInfo,
				_MonsterState);
			G_ObjectManager->GameServer->SendPacketFieldOfView(this, MonsterMovePacket);
			MonsterMovePacket->Free();
		}
		break;
	}
}

void CMonster::SendMonsterChangeObjectState()
{
	CMessage* MonsterObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeMonsterObjectState(_GameObjectInfo.ObjectId,
		_GameObjectInfo.ObjectPositionInfo.MoveDir,
		_GameObjectInfo.ObjectType,
		_GameObjectInfo.ObjectPositionInfo.State,
		_MonsterState);
	G_ObjectManager->GameServer->SendPacketFieldOfView(this, MonsterObjectStateChangePacket);
	MonsterObjectStateChangePacket->Free();
}

void CMonster::TargetAttackCheck()
{
	st_Vector2Int MonsterCellPosition;
	MonsterCellPosition._X = _GameObjectInfo.ObjectPositionInfo.CollisionPositionX;
	MonsterCellPosition._Y = _GameObjectInfo.ObjectPositionInfo.CollisionPositionY;

	st_Vector2Int TargetPosition;
	TargetPosition._X = _Target->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX;
	TargetPosition._Y = _Target->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY;

	// 타겟 위치까지 길을 찾는다.
	vector<st_Vector2Int> Path = _Channel->GetMap()->FindPath(this, MonsterCellPosition, TargetPosition);
	if (Path[1] == TargetPosition)
	{
		st_Vector2 TargetPosition;
		TargetPosition._X = _Target->_GameObjectInfo.ObjectPositionInfo.PositionX;
		TargetPosition._Y = _Target->_GameObjectInfo.ObjectPositionInfo.PositionY;

		st_Vector2 MonsterPosition;
		MonsterPosition._X = _GameObjectInfo.ObjectPositionInfo.PositionX;
		MonsterPosition._Y = _GameObjectInfo.ObjectPositionInfo.PositionY;

		st_Vector2 DirectionVector = TargetPosition - MonsterPosition;
		st_Vector2 NormalVector = st_Vector2::Normalize(DirectionVector);

		en_MoveDir Dir = st_Vector2::GetMoveDir(NormalVector);
		_GameObjectInfo.ObjectPositionInfo.MoveDir = Dir;

		// 공격 상태로 바꾼다.
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
		_MonsterState = en_MonsterState::MONSTER_ATTACK;

		_DefaultAttackTick = GetTickCount64() + _AttackTickPoint;

		CMessage* MonsterObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeMonsterObjectState(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.MoveDir,
			_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.State,
			_MonsterState);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, MonsterObjectStateChangePacket);
		MonsterObjectStateChangePacket->Free();
	}
}
