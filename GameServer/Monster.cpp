#include "pch.h"
#include "Monster.h"
#include "Player.h"
#include "DataManager.h"
#include "ObjectManager.h"
#include "MapManager.h"
#include "ChannelManager.h"
#include "Skill.h"
#include "RectCollision.h"

CMonster::CMonster()
{
	_SearchTick = GetTickCount64();
	_MoveTick = GetTickCount64();
	_PatrolTick = GetTickCount64();

	_MonsterState = en_MonsterState::MONSTER_IDLE;
	_MovePoint = st_Vector2::Zero();
	_PatrolPoint = st_Vector2::Zero();

	_DeadReadyTick = 0;

	_Target = nullptr;	

	_RectCollision = new CRectCollision(this);
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
		break;
	case en_CreatureState::DEAD:
		UpdateDead();
		break;
	}

	AggroTargetListCheck();
	SelectTarget();
}

void CMonster::PositionReset()
{
}

bool CMonster::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	if (_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE
		|| _GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::RETURN_SPAWN_POSITION
		|| _GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::READY_DEAD
		|| _GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
	{
		CGameObject::OnDamaged(Attacker, Damage);

		st_GameObjectJob* AggroJob = G_ObjectManager->GameObjectJobCreate();
		AggroJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_AGGRO_LIST_INSERT_OR_UPDATE;

		CGameServerMessage* AggroJobMessage = CGameServerMessage::GameServerMessageAlloc();
		AggroJobMessage->Clear();

		*AggroJobMessage << (int8)en_AggroCategory::AGGRO_CATEGORY_DAMAGE;
		*AggroJobMessage << &Attacker;
		*AggroJobMessage << Damage;

		AggroJob->GameObjectJobMessage = AggroJobMessage;

		_GameObjectJobQue.Enqueue(AggroJob);

		if (_GameObjectInfo.ObjectStatInfo.HP == 0)
		{		
			return true; 
		}
	}

	return false;
}

void CMonster::Start()
{	
	_SpawnIdleTick = GetTickCount64() + 5000;

	_SpawnPosition = _GameObjectInfo.ObjectPositionInfo.CollisionPosition;

	_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.MP = _GameObjectInfo.ObjectStatInfo.MaxMP;
	_GameObjectInfo.ObjectStatInfo.Speed = _GameObjectInfo.ObjectStatInfo.MaxSpeed;

	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;
	_MonsterState = en_MonsterState::MONSTER_IDLE;

	_PatrolPositions = GetAroundCellPositions(_GameObjectInfo.ObjectPositionInfo.CollisionPosition, 1);
}

void CMonster::End()
{
	CGameObject::End();
}

void CMonster::AggroTargetListCheck()
{
	for (auto AggroTargetIterator : _AggroTargetList)
	{
		CGameObject* AggroTarget = AggroTargetIterator.second.AggroTarget;

		if (AggroTarget != nullptr)
		{
			// ��׷� ��Ͽ��� ������ ���� 
			if (AggroTarget->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::READY_DEAD
				|| AggroTarget->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::DEAD
				|| AggroTarget->_NetworkState == en_ObjectNetworkState::LEAVE
				|| AggroTarget->_NetworkState == en_ObjectNetworkState::READY)
			{
				_AggroTargetList.erase(AggroTargetIterator.first);
			}

			int16 Distance = st_Vector2Int::Distance(AggroTarget->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, _GameObjectInfo.ObjectPositionInfo.CollisionPosition);

			if (Distance > _FieldOfViewDistance)
			{
				_AggroTargetList.erase(AggroTargetIterator.first);
			}
		}	
	}
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

CGameObject* CMonster::FindTarget(en_MonsterAggroType* AggroType)
{	
	bool Cango = false;
	CGameObject* Target = _Channel->GetMap()->MonsterReqFindNearPlayer(this, AggroType, 1, &Cango);
	if (Target == nullptr)
	{
		_PatrolTick = GetTickCount64() + _PatrolTickPoint;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
		_MonsterState = en_MonsterState::MONSTER_READY_PATROL;

		SendMonsterChangeObjectState();
	}
	else
	{
		int16 Distance = st_Vector2Int::Distance(Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, _GameObjectInfo.ObjectPositionInfo.CollisionPosition);
		// Ÿ���� ã������ �߰��� �� �ִ� �Ÿ��� ���� ����
		if (Distance > _ChaseCellDistance)
		{
			// ���� ( Ÿ���� ������ �Ÿ��� �ȵǼ� �� �� ��� ���� ���·� ���� )
			if (Cango == true)
			{
				_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
				_MonsterState = en_MonsterState::MONSTER_READY_PATROL;

				SendMonsterChangeObjectState();
			}
			// ��� ( Ÿ���� ������ �ٸ� ������Ʈ�� ���� �� �� ��� ��� ���·� ���� )
			else
			{
				return nullptr;
			}
		}
		else
		{
			// �߰ݰŸ� ���� ����			
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

	en_MonsterAggroType MonsterAggroType;

	CGameObject* Target = FindTarget(&MonsterAggroType);
	if (Target != nullptr)
	{
		int16 Distance = st_Vector2Int::Distance(Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, _GameObjectInfo.ObjectPositionInfo.CollisionPosition);
		if (Distance <= _ChaseCellDistance) // ����� ���� �Ÿ� �ȿ� �ִ��� Ȯ���Ѵ�.
		{
			st_Aggro Aggro;
			Aggro.AggroTarget = Target;

			switch (MonsterAggroType)
			{
			case en_MonsterAggroType::MONSTER_AGGRO_FIRST_TARGET:
				Aggro.AggroPoint = _GameObjectInfo.ObjectStatInfo.MaxHP * G_Datamanager->_MonsterAggroData.MonsterAggroFirstTarget;
				break;
			case en_MonsterAggroType::MONSTER_AGGRO_SECOND_TARGET:
				Aggro.AggroPoint = _GameObjectInfo.ObjectStatInfo.MaxHP * G_Datamanager->_MonsterAggroData.MonsterAggroSecondTarget;
				break;							
			}			

			_AggroTargetList.insert(pair<int64, st_Aggro>(Target->_GameObjectInfo.ObjectId, Aggro));

			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			_MonsterState = en_MonsterState::MONSTER_READY_MOVE;

			_Target = Target;

			return;
		}

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
	}

	random_device Seed;
	mt19937 Gen(Seed());

	do
	{
		// ���͸� �����Ҷ� ������ ���� ��ġ�� �������� �����ص� ���� ��ġ ��
		// �������� ���� ��ġ�� ��´�.
		int8 MaxPatrolIndex = (int8)_PatrolPositions.size();
		uniform_int_distribution<int> RandomPatrolPoint(0, MaxPatrolIndex - 1);
		int8 RandomIndex = RandomPatrolPoint(Gen);

		// �ռ� ���� ������ġ������ ���� ã�´�.
		st_Vector2Int MonsterPosition;
		MonsterPosition._X = _GameObjectInfo.ObjectPositionInfo.CollisionPosition._X;
		MonsterPosition._Y = _GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y;

		if (_PatrolPositions[RandomIndex] == MonsterPosition)
		{
			continue;
		}

		vector<st_Vector2Int> Path = _Channel->GetMap()->FindPath(this, MonsterPosition, _PatrolPositions[RandomIndex]);
		if (Path.size() < 2)
		{
			// ���� ��ġ�� �̵� �� �� ���� ��� ���°��� IDLE�� �ʱ�ȭ�Ѵ�.
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			_MonsterState = en_MonsterState::MONSTER_IDLE;
			SendMonsterChangeObjectState();
			return;
		}

		// ���� ��ġ�� �����Ѵ�
		_MonsterState = en_MonsterState::MONSTER_PATROL;
		_PatrolPoint = PositionCheck(Path[1]);
	} while (0);
}

void CMonster::UpdatePatrol()
{
	st_Vector2 MonsterPosition;
	MonsterPosition._X = _GameObjectInfo.ObjectPositionInfo.Position._X;
	MonsterPosition._Y = _GameObjectInfo.ObjectPositionInfo.Position._Y;

	st_Vector2 DirectionVector = _PatrolPoint - MonsterPosition;	
	st_Vector2 NormalVector = DirectionVector.Normalize();//st_Vector2::Normalize(DirectionVector);

	_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2::GetMoveDir(NormalVector);

	/*switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
	{
	case en_MoveDir::UP:
		G_Logger->WriteStdOut(en_Color::GREEN, L"Dir : �� ");
		break;
	case en_MoveDir::DOWN:
		G_Logger->WriteStdOut(en_Color::GREEN, L"Dir : �Ʒ� ");
		break;
	case en_MoveDir::LEFT:
		G_Logger->WriteStdOut(en_Color::GREEN, L"Dir : ���� ");
		break;
	case en_MoveDir::RIGHT:
		G_Logger->WriteStdOut(en_Color::GREEN, L"Dir : ������ ");
		break;
	}

	G_Logger->WriteStdOut(en_Color::RED, L"Position._X [%0.1f] Position._Y [%0.1f] GoalPosition._X [%0.1f] GoalPosition [%0.1f]  \n",
		_GameObjectInfo.ObjectPositionInfo.Position._X,
		_GameObjectInfo.ObjectPositionInfo.Position._Y,
		_PatrolPoint._X,
		_PatrolPoint._Y);*/

	Move();
}

void CMonster::ReadMoving()
{
	do
	{
		// ���̳� �ٸ� ���Ϳ��� �ε����� ��� ��� ���� ��� ���� �ٽ� ���� 
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
	
	bool IsAttack = TargetAttackCheck();
	if (IsAttack == false)
	{
		// Ÿ�� ��ġ���� ���� ã�´�.
		vector<st_Vector2Int> Path = _Channel->GetMap()->FindPath(this, _GameObjectInfo.ObjectPositionInfo.CollisionPosition, _Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
		if (Path.size() < 2)
		{
			// Ÿ�� ��ġ�� �̵� �� �� ���� ��� ���°��� IDLE�� �ʱ�ȭ�Ѵ�.
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			_MonsterState = en_MonsterState::MONSTER_IDLE;
			SendMonsterChangeObjectState();
			return;
		}

		_MonsterState = en_MonsterState::MONSTER_MOVE;
		_MovePoint = PositionCheck(Path[1]);
	}
}

void CMonster::UpdateMoving()
{
	// ��ǥ�� ����� ���
	if (_Target == nullptr)
	{
		// ��ȯ �ߴ� ��ġ�� �ǵ��� ����.
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::RETURN_SPAWN_POSITION;
		_MonsterState = en_MonsterState::MONSTER_MOVE;		
		return;
	}	

	// ��ǥ���� �Ÿ��� �� �� �Ÿ��� ���� �Ÿ����� ũ�ų� ���� ��ȯ ��ġ���� ���� �Ÿ� ������ ���� ��� ��ȯ �ߴ� ��ġ�� �ǵ��� ����.
	int16 SpawnPositionDistance = st_Vector2Int::Distance(_SpawnPosition, _GameObjectInfo.ObjectPositionInfo.CollisionPosition);

	int16 TargetDistance = st_Vector2Int::Distance(_Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, _GameObjectInfo.ObjectPositionInfo.CollisionPosition);
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

	// ���� ������ ��� 
	if (IsChohoneStun || IsLightNingStun)
	{
		return;
	}

	// �߹��� ������ ���
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

	st_Vector2 MonsterPosition = _GameObjectInfo.ObjectPositionInfo.Position;

	st_Vector2 DirectionVector = _MovePoint - MonsterPosition;
	st_Vector2 NormalVector = DirectionVector.Normalize();

	//G_Logger->WriteStdOut(en_Color::RED, L"X : %0.1f Y : %0.1f\n", NormalVector._X, NormalVector._Y);

	_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2::GetMoveDir(NormalVector);

	/*
	G_Logger->WriteStdOut(en_Color::RED, L"Position._X [%0.1f] Position._Y [%0.1f] GoalPosition._X [%0.1f] GoalPosition [%0.1f]  \n",
		_GameObjectInfo.ObjectPositionInfo.Position._X,
		_GameObjectInfo.ObjectPositionInfo.Position._Y,
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
	st_Vector2Int MonsterPosition = _GameObjectInfo.ObjectPositionInfo.CollisionPosition;

	// ReturnSpawnPosition�� �����ϸ� Idle���·� ��ȯ�Ѵ�.
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
		
	st_Vector2 DirectionVector = PositionCheck(Path[1]) - _GameObjectInfo.ObjectPositionInfo.Position;
	st_Vector2 NormalVector = DirectionVector.Normalize();

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
		// ��ǥ���� ������ų� ���� �غ� �Ǵ� ���� ������ ��� ��ǥ�� �����ϰ� ���ڸ��� ���ư�
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

		// ������ �������� Ȯ��
		st_Vector2Int TargetCellPosition = _Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition;
		st_Vector2Int MyCellPosition = _GameObjectInfo.ObjectPositionInfo.CollisionPosition;
		st_Vector2Int Direction = TargetCellPosition - MyCellPosition;

		_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetMoveDir(Direction);

		SendMonsterChangeObjectState();

		int Distance = st_Vector2Int::Distance(_Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, _GameObjectInfo.ObjectPositionInfo.CollisionPosition);
		//float Distance = st_Vector2::Distance(_Target->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);
		// Ÿ�ٰ��� �Ÿ��� ���� ���� �ȿ� ���ϰ� X==0 || Y ==0 �϶�( �밢���� ����) ����
		bool CanUseAttack = (Distance <= _AttackRange && (Direction._X == 0 || Direction._Y == 0));
		if (CanUseAttack == false)
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
			return;
		}

		// ũ��Ƽ�� �Ǵ�		
		random_device Seed;
		default_random_engine Eng(Seed());

		float CriticalPoint = _GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint / 1000.0f;
		bernoulli_distribution CriticalCheck(CriticalPoint);
		bool IsCritical = CriticalCheck(Eng);

		// ������ �Ǵ�
		mt19937 Gen(Seed());
		uniform_int_distribution<int> DamageChoiceRandom(_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage, _GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage);
		int32 ChoiceDamage = DamageChoiceRandom(Gen);
		int32 FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

		bool TargetIsDead = _Target->OnDamaged(this, FinalDamage);

		// ������ ���
		CMessage* ResMonsterDamagePacket = G_ObjectManager->GameServer->MakePacketResDamage(_GameObjectInfo.ObjectId,
			_Target->_GameObjectInfo.ObjectId,
			en_SkillType::SKILL_SLIME_NORMAL,
			FinalDamage, IsCritical);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResMonsterDamagePacket);
		ResMonsterDamagePacket->Free();

		wstring SlimeAttackAnimation = L"SLIME_ATTACK";

		// ���� �ִϸ��̼� ���
		CMessage* AnimationPlayPacket = G_ObjectManager->GameServer->MakePacketResAnimationPlay(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.MoveDir, SlimeAttackAnimation);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, AnimationPlayPacket);
		AnimationPlayPacket->Free();

		// ���� �÷��̾�鿡�� ������ ���� ��� ����
		CMessage* ResChangeObjectStatPacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_Target->_GameObjectInfo.ObjectId, _Target->_GameObjectInfo.ObjectStatInfo);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResChangeObjectStatPacket);
		ResChangeObjectStatPacket->Free();

		// 1.2�ʸ��� ����
		_DefaultAttackTick = GetTickCount64() + _AttackTickPoint;

		wchar_t BearAttackMessage[64] = L"0";
		wsprintf(BearAttackMessage, L"%s�� �Ϲ� ������ ����� %s���� %d�� �������� ����ϴ�", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

		wstring BearAttackString = BearAttackMessage;

		CMessage* ResSlimeSystemMessage = G_ObjectManager->GameServer->MakePacketResChattingBoxMessage(_Target->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, IsCritical ? st_Color::Red() : st_Color::White(), IsCritical ? L"ġ��Ÿ! " + BearAttackString : BearAttackString);
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
	// ���� �غ� ���¿� �����ϸ� �������� �ð� ��ŭ ��� �� ���� ����
	if (_DeadReadyTick < GetTickCount64())
	{
		_DeadTick = GetTickCount64() + _ReSpawnTime;

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;
		
		if (_Channel == nullptr)
		{
			CRASH("�����Ϸ��� ä���� �������� ����");
		}		

		// ���Ͱ� �־��� �ڸ��� ���� ��ȯ����
		st_GameObjectJob* DeSpawnMonsterChannelJob = G_ObjectManager->GameServer->MakeGameObjectJobObjectDeSpawnObjectChannel(this);
		_Channel->_ChannelJobQue.Enqueue(DeSpawnMonsterChannelJob);
	}		
}

void CMonster::UpdateDead()
{
	if (_DeadTick < GetTickCount64())
	{
		CMap* Map = G_MapManager->GetMap(1);
		if (Map != nullptr)
		{
			CGameObject* FindObject = Map->Find(_SpawnPosition);
			if (FindObject == nullptr)
			{				
				_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_READY;				
				
				// ä�ο��� ����
				st_GameObjectJob* LeaveChannerMonsterJob = G_ObjectManager->GameServer->MakeGameObjectJobLeaveChannel(this);
				_Channel->_ChannelJobQue.Enqueue(LeaveChannerMonsterJob);				
				// ä�ο��� �����ϰ� �ٽ� ����
				st_GameObjectJob* EnterChannelMonsterJob = G_ObjectManager->GameServer->MakeGameObjectJobObjectEnterChannel(this);
				_Channel->_ChannelJobQue.Enqueue(EnterChannelMonsterJob);				
			}
			else
			{
				_DeadTick = GetTickCount64() + _ReSpawnTime;
			}
		}
	}	
}

void CMonster::Move()
{
	switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
	{
	case en_MoveDir::UP:
		_GameObjectInfo.ObjectPositionInfo.Position._Y +=
			(st_Vector2::Up()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::DOWN:
		_GameObjectInfo.ObjectPositionInfo.Position._Y += 
			(st_Vector2::Down()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::LEFT:
		_GameObjectInfo.ObjectPositionInfo.Position._X += 
			(st_Vector2::Left()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::RIGHT:
		_GameObjectInfo.ObjectPositionInfo.Position._X +=
			(st_Vector2::Right()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	}

	_RectCollision->CollisionUpdate();

	bool CanMove = _Channel->GetMap()->Cango(this, _GameObjectInfo.ObjectPositionInfo.Position._X, _GameObjectInfo.ObjectPositionInfo.Position._Y);
	if (CanMove == true)
	{
		st_Vector2Int CollisionPosition;
		CollisionPosition._X = _GameObjectInfo.ObjectPositionInfo.Position._X;
		CollisionPosition._Y = _GameObjectInfo.ObjectPositionInfo.Position._Y;

		if (CollisionPosition._X != _GameObjectInfo.ObjectPositionInfo.CollisionPosition._X
			|| CollisionPosition._Y != _GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y)
		{
			_Channel->GetMap()->ApplyMove(this, CollisionPosition);
		}

		switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
		{
		case en_MoveDir::UP:
			switch (_GameObjectInfo.ObjectPositionInfo.State)
			{
			case en_CreatureState::PATROL:
				if (_GameObjectInfo.ObjectPositionInfo.Position._Y >= _PatrolPoint._Y)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
					_PatrolTick = GetTickCount64() + _PatrolTickPoint;
					CanMove = false;
				}
				break;
			case en_CreatureState::MOVING:
				if (_GameObjectInfo.ObjectPositionInfo.Position._Y >= _MovePoint._Y)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
				}
				break;
			case en_CreatureState::ATTACK:
				if (abs(_Target->_GameObjectInfo.ObjectPositionInfo.Position._Y - _GameObjectInfo.ObjectPositionInfo.Position._Y) <= 0.5f)
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
				if (_GameObjectInfo.ObjectPositionInfo.Position._Y <= _PatrolPoint._Y)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
					_PatrolTick = GetTickCount64() + _PatrolTickPoint;
					CanMove = false;
				}
				break;
			case en_CreatureState::MOVING:
				if (_GameObjectInfo.ObjectPositionInfo.Position._Y <= _MovePoint._Y)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
				}
				break;
			case en_CreatureState::ATTACK:
				if (abs(_Target->_GameObjectInfo.ObjectPositionInfo.Position._Y - _GameObjectInfo.ObjectPositionInfo.Position._Y) <= 0.5f)
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
				if (_GameObjectInfo.ObjectPositionInfo.Position._X <= _PatrolPoint._X)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
					_PatrolTick = GetTickCount64() + _PatrolTickPoint;
					CanMove = false;
				}
				break;
			case en_CreatureState::MOVING:
				if (_GameObjectInfo.ObjectPositionInfo.Position._X <= _MovePoint._X)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
				}
				break;
			case en_CreatureState::ATTACK:
				if (abs(_Target->_GameObjectInfo.ObjectPositionInfo.Position._X - _GameObjectInfo.ObjectPositionInfo.Position._X) <= 0.5f)
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
				if (_GameObjectInfo.ObjectPositionInfo.Position._X >= _PatrolPoint._X)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
					_PatrolTick = GetTickCount64() + _PatrolTickPoint;
					CanMove = false;
				}
				break;
			case en_CreatureState::MOVING:
				if (_GameObjectInfo.ObjectPositionInfo.Position._X >= _MovePoint._X)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
				}
				break;
			case en_CreatureState::ATTACK:
				if (abs(_Target->_GameObjectInfo.ObjectPositionInfo.Position._X - _GameObjectInfo.ObjectPositionInfo.Position._X) <= 0.5f)
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

bool CMonster::TargetAttackCheck()
{
	// ���� ���� �Ǵ�
	// ��ã�� ����
	vector<st_Vector2Int> Path = _Channel->GetMap()->FindPath(this, _GameObjectInfo.ObjectPositionInfo.CollisionPosition, _Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
	// ���� ��ġ�� Ÿ�� ��ġ�� ��� 
	if (Path[1] == _Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition)
	{
		// ���Ⱚ ����
		st_Vector2 DirectionVector = _Target->_GameObjectInfo.ObjectPositionInfo.Position - _GameObjectInfo.ObjectPositionInfo.Position;
		st_Vector2 NormalVector = DirectionVector.Normalize();

		en_MoveDir Dir = st_Vector2::GetMoveDir(NormalVector);

		_GameObjectInfo.ObjectPositionInfo.MoveDir = Dir;
		// ���� ���·� �ٲ۴�.
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
		_MonsterState = en_MonsterState::MONSTER_ATTACK;

		_DefaultAttackTick = GetTickCount64() + _AttackTickPoint;

		SendMonsterChangeObjectState();

		return true;

		//// �Ÿ��� ���.
		//float Distance = st_Vector2::Distance(_Target->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);
		//// �Ÿ��� Ư�� �Ÿ� ���̶��
		//if (Distance <= _AttackRange)
		//{
		//	// ���Ⱚ ����
		//	st_Vector2 DirectionVector = _Target->_GameObjectInfo.ObjectPositionInfo.Position - _GameObjectInfo.ObjectPositionInfo.Position;
		//	st_Vector2 NormalVector = DirectionVector.Normalize();

		//	en_MoveDir Dir = st_Vector2::GetMoveDir(NormalVector);

		//	_GameObjectInfo.ObjectPositionInfo.MoveDir = Dir;
		//	// ���� ���·� �ٲ۴�.
		//	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
		//	_MonsterState = en_MonsterState::MONSTER_ATTACK;

		//	_DefaultAttackTick = GetTickCount64() + _AttackTickPoint;

		//	SendMonsterChangeObjectState();

		//	return true;
		//}

		//return false;		
	}

	return false;	
}
