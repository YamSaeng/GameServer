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

	_MonsterState = en_MonsterState::MONSTER_IDLE;
	_MovePoint = st_Vector2::Zero();
	_PatrolPoint = st_Vector2::Zero();
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
		switch (_MonsterState)
		{
		case en_MonsterState::MONSTER_IDLE:
			break;
		case en_MonsterState::MONSTER_READY_ATTACK:
			break;
		case en_MonsterState::MONSTER_ATTACK:			
			break;
		}		
		break;
	case en_CreatureState::SPELL:
		UpdateSpell();
		break;
	case en_CreatureState::DEAD:
		UpdateDead();
		break;
	case en_CreatureState::STUN:
		UpdateStun();
		break;
	case en_CreatureState::PUSH_AWAY:
		UpdatePushAway();
		break;
	case en_CreatureState::ROOT:
		UpdateRoot();
		break;
	default:
		break;
	}	
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

		_Target = (CPlayer*)Attacker;

		if (_GameObjectInfo.ObjectStatInfo.HP == 0)
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

			if (Attacker->_SelectTarget == this)
			{
				Attacker->_SelectTarget = nullptr;
			}

			OnDead(Attacker);

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

CGameObject* CMonster::FindTarget()
{
	bool Cango = false;
	CGameObject* Target = _Channel->FindNearPlayer(this, 1, &Cango);
	if (Target == nullptr)
	{
		_PatrolTick = GetTickCount64() + _PatrolTickPoint;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
		_MonsterState = en_MonsterState::MONSTER_READY_PATROL;

		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
	}
	else
	{
		// Ÿ���� ���°� SPAWN_IDLE�� ��� ���� ���·�
		if (Target->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::SPAWN_IDLE)
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
			_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
			BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
			return nullptr;
		}

		int16 Distance = st_Vector2Int::Distance(Target->GetCellPosition(), GetCellPosition());
		// Ÿ���� ã������ �߰��� �� �ִ� �Ÿ��� ���� ����
		if (Distance > _ChaseCellDistance)
		{
			// ���� ( Ÿ���� ������ �Ÿ��� �ȵǼ� �� �� ��� ���� ���·� ���� )
			if (Cango == true)
			{
				_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
				_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
				BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
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

			_MoveTick = GetTickCount64() + (int)(1000 / _GameObjectInfo.ObjectStatInfo.Speed);

			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
			BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		}
	}

	return Target;
}

void CMonster::UpdateSpawnIdle()
{
	if (_SpawnIdleTick > GetTickCount64())
	{
		return;
	}

	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
}

void CMonster::UpdateIdle()
{
	if (_SearchTick > GetTickCount64())
	{
		return;
	}

	_SearchTick = GetTickCount64() + _SearchTickPoint;
	
	_PatrolTick = GetTickCount64() + _PatrolTickPoint;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
	_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
	
	BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
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
		if (Distance <= _ChaseCellDistance)
		{
			// ��� ���� �Ÿ��� ���. 
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
		MonsterPosition._X = _GameObjectInfo.ObjectPositionInfo.CollisionPositionX;
		MonsterPosition._Y = _GameObjectInfo.ObjectPositionInfo.CollisionPositionY;
				
		if (_PatrolPositions[RandomIndex] == MonsterPosition)
		{
			continue;
		}

		vector<st_Vector2Int> Path = _Channel->_Map->FindPath(this, MonsterPosition, _PatrolPositions[RandomIndex]);
		if (Path.size() < 2)
		{
			// ���� ��ġ�� �̵� �� �� ���� ��� ���°��� IDLE�� �ʱ�ȭ�Ѵ�.
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			_MonsterState = en_MonsterState::MONSTER_IDLE;
			BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
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
	MonsterPosition._X = _GameObjectInfo.ObjectPositionInfo.PositionX;
	MonsterPosition._Y = _GameObjectInfo.ObjectPositionInfo.PositionY;

	st_Vector2 DirectionVector = _PatrolPoint - MonsterPosition;
	st_Vector2 NormalVector = st_Vector2::Normalize(DirectionVector);

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

	// Ÿ�� ��ġ���� ���� ã�´�.
	vector<st_Vector2Int> Path = _Channel->_Map->FindPath(this, MonsterPosition, TargetPosition);
	if (Path.size() < 2)
	{
		// Ÿ�� ��ġ�� �̵� �� �� ���� ��� ���°��� IDLE�� �ʱ�ȭ�Ѵ�.
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		_MonsterState = en_MonsterState::MONSTER_IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	// ���� ĭ ��ġ�� Ÿ���̶��
	if (Path[1] == TargetPosition)
	{
		// ���� ���·� �ٲ۴�.
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
	}
	else
	{
		_MonsterState = en_MonsterState::MONSTER_MOVE;
		_MovePoint = PositionCheck(Path[1]);
	}
}

void CMonster::UpdateMoving()
{
	st_Vector2 MonsterPosition;
	MonsterPosition._X = _GameObjectInfo.ObjectPositionInfo.PositionX;
	MonsterPosition._Y = _GameObjectInfo.ObjectPositionInfo.PositionY;

	st_Vector2 DirectionVector = _MovePoint - MonsterPosition;
	st_Vector2 NormalVector = st_Vector2::Normalize(DirectionVector);

	//G_Logger->WriteStdOut(en_Color::RED, L"X : %0.1f Y : %0.1f\n", NormalVector._X, NormalVector._Y);

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

	G_Logger->WriteStdOut(en_Color::RED, L"PositionX [%0.1f] PositionY [%0.1f] GoalPositionX [%0.1f] GoalPosition [%0.1f]  \n",
		_GameObjectInfo.ObjectPositionInfo.PositionX,
		_GameObjectInfo.ObjectPositionInfo.PositionY,
		_MovePoint._X,
		_MovePoint._Y);*/
		
	Move();	

	//if (_MoveTick > GetTickCount64())
	//{
	//	return;
	//}		
	//
	//int MoveTick = (int)(1000 / 3);
	//_MoveTick = GetTickCount64() + MoveTick;

	//// Ÿ���� ���ų� ���� �ٸ� ä�ο� ���� ��� Idle ���·� ��ȯ�Ѵ�.
	//if (_Target == nullptr || _Target->_Channel != _Channel)
	//{
	//	_Target = nullptr;
	//	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::RETURN_SPAWN_POSITION;
	//	BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
	//	return;
	//}	

	//st_Vector2Int TargetPosition = _Target->GetCellPosition();
	//st_Vector2Int MonsterPosition = GetCellPosition();

	//// ���Ⱚ ���Ѵ�.
	//st_Vector2Int Direction = TargetPosition - MonsterPosition;
	//// Ÿ�ٰ� ������ �Ÿ��� ���.
	//int32 Distance = st_Vector2Int::Distance(TargetPosition, MonsterPosition);
	//// Ÿ�ٰ��� �Ÿ��� 0 �Ǵ� �߰ݰŸ� ���� �־�����
	//if (Distance == 0 || Distance > _ChaseCellDistance)
	//{
	//	_Target = nullptr;
	//	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::RETURN_SPAWN_POSITION;
	//	BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
	//	return;
	//}

	//vector<st_Vector2Int> Path = _Channel->_Map->FindPath(this, MonsterPosition, TargetPosition);
	//// �߰��߿� �÷��̾����� �ٰ����� ���ų� �߰ݰŸ��� ����� �����.
	//if (Path.size() < 2 || Distance > _ChaseCellDistance)
	//{
	//	_Target = nullptr;
	//	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::RETURN_SPAWN_POSITION;
	//	BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
	//	return;
	//}

	//_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetDirectionFromVector(Path[1] - MonsterPosition);
	//st_Vector2Int NowCellPosition = GetCellPosition();
	//st_Vector2 NowPosition = PositionCheck(NowCellPosition);
	//_GameObjectInfo.ObjectPositionInfo.PositionX = NowPosition._X;
	//_GameObjectInfo.ObjectPositionInfo.PositionY = NowPosition._Y;

	//// ������ �Ÿ��� ���� �����ȿ� �ְ�, �밢���� ���� ������ ���� ���·� �ٲ۴�.
	///*if (Distance <= _AttackRange && (Direction._X == 0 || Direction._Y == 0))
	//{
	//	_AttackTick = GetTickCount64() + _AttackTickPoint;
	//	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
	//	_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetDirectionFromVector(Direction);
	//	BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
	//	return;
	//}	*/

	//_Channel->_Map->ApplyMove(this, Path[1]);

	//BroadCastPacket(en_PACKET_S2C_MONSTER_MOVE);
}

void CMonster::UpdateReturnSpawnPosition()
{
	if (_MoveTick > GetTickCount64())
	{
		return;
	}

	st_Vector2Int MonsterPosition = GetCellPosition();
	
	// ReturnSpawnPosition�� �����ϸ� Idle���·� ��ȯ�Ѵ�.
	if (st_Vector2Int::Distance(_SpawnPosition, MonsterPosition) <= 2)
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	int MoveTick = (int)(1000 / _GameObjectInfo.ObjectStatInfo.Speed);
	_MoveTick = GetTickCount64() + MoveTick;

	vector<st_Vector2Int> Path;

	for (st_Vector2Int ReturnSpawnPosition : _PatrolPositions)
	{
		Path = _Channel->_Map->FindPath(this, MonsterPosition, ReturnSpawnPosition);
		if (Path.size() < 2)
		{
			continue;
		}
		
		break;
	}

	_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetMoveDir(Path[1] - MonsterPosition);
	_Channel->_Map->ApplyMove(this, Path[1]);
	BroadCastPacket(en_PACKET_S2C_MONSTER_MOVE);	
}

void CMonster::ReadAttack()
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

	switch (Dir)
	{
	case en_MoveDir::UP:
		break;
	case en_MoveDir::DOWN:
	
		break;
	case en_MoveDir::LEFT:
		break;
	case en_MoveDir::RIGHT:
		break;	
	}	
}

void CMonster::UpdateAttack()
{
	if (_AttackTick == 0)
	{
		// Ÿ���� ������ų� ä���� �޶��� ��� Ÿ���� ����
		if (_Target == nullptr || _Target->_Channel != _Channel)
		{
			_Target = nullptr;
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			return;
		}

		// ������ �������� Ȯ��
		st_Vector2Int TargetCellPosition = _Target->GetCellPosition();
		st_Vector2Int MyCellPosition = GetCellPosition();
		st_Vector2Int Direction = TargetCellPosition - MyCellPosition;

		_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetMoveDir(Direction);

		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);

		int32 Distance = st_Vector2Int::Distance(TargetCellPosition, MyCellPosition);
		// Ÿ�ٰ��� �Ÿ��� ���� ���� �ȿ� ���ϰ� X==0 || Y ==0 �϶�( �밢���� ����) ����
		bool CanUseAttack = (Distance <= _AttackRange && (Direction._X == 0 || Direction._Y == 0));
		if (CanUseAttack == false)
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
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

		_Target->OnDamaged(this, FinalDamage);

		CMessage* ResBearAttackPacket = G_ObjectManager->GameServer->MakePacketResAttack(_GameObjectInfo.ObjectId, _Target->_GameObjectInfo.ObjectId, en_SkillType::SKILL_BEAR_NORMAL, FinalDamage, IsCritical);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResBearAttackPacket);
		ResBearAttackPacket->Free();

		// ���� �÷��̾�鿡�� ������ ���� ��� ����
		BroadCastPacket(en_PACKET_S2C_OBJECT_STAT_CHANGE);

		// 1.2�ʸ��� ����
		_AttackTick = GetTickCount64() + _AttackTickPoint;

		wchar_t BearAttackMessage[64] = L"0";
		wsprintf(BearAttackMessage, L"%s�� �Ϲ� ������ ����� %s���� %d�� �������� ����ϴ�", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

		wstring BearAttackString = BearAttackMessage;

		CMessage* ResSlimeSystemMessage = G_ObjectManager->GameServer->MakePacketResChattingBoxMessage(_Target->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, IsCritical ? st_Color::Red() : st_Color::White(), IsCritical ? L"ġ��Ÿ! " + BearAttackString : BearAttackString);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResSlimeSystemMessage);
		ResSlimeSystemMessage->Free();
	}

	if (_AttackTick > GetTickCount64())
	{
		return;
	}

	_AttackTick = 0;
}

void CMonster::UpdateSpell()
{
}

void CMonster::UpdateDead()
{

}

void CMonster::UpdateStun()
{
}

void CMonster::UpdatePushAway()
{
}

void CMonster::UpdateRoot()
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

	bool CanMove = _Channel->_Map->Cango(this, _GameObjectInfo.ObjectPositionInfo.PositionX, _GameObjectInfo.ObjectPositionInfo.PositionY);
	if (CanMove == true)
	{
		st_Vector2Int CollisionPosition;
		CollisionPosition._X = _GameObjectInfo.ObjectPositionInfo.PositionX;
		CollisionPosition._Y = _GameObjectInfo.ObjectPositionInfo.PositionY;

		if (CollisionPosition._X != _GameObjectInfo.ObjectPositionInfo.CollisionPositionX
			|| CollisionPosition._Y != _GameObjectInfo.ObjectPositionInfo.CollisionPositionY)
		{
			_Channel->_Map->ApplyMove(this, CollisionPosition);
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
	}	
}
