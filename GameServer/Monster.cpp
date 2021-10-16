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
		// Ÿ���� ã������ �߰��� �� �ִ� �Ÿ��� ���� ����
		if (Distance > _ChaseCellDistance)
		{
			// ���� ( Ÿ���� ������ �Ÿ��� �ȵǼ� �� �� ��� ���� ���·� ���� )
			if (Cango == true)
			{
				_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
			}
			// ��� ( Ÿ���� ������ �ٸ� ������Ʈ�� ���� �� �� ��� ��� ���·� ���� )
			else
			{
				return;
			}
		}
		else
		{
			// �߰ݰŸ� ���� ����
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

	// ���͸� �����Ҷ� ������ ���� ��ġ�� �������� �����ص� ���� ��ġ �߿���
	// �������� ���� ��ġ�� ��´�.
	int8 MaxPatrolIndex = (int8)_PatrolPositions.size();
	uniform_int_distribution<int> RandomPatrolPoint(0, MaxPatrolIndex);
	int8 RandomIndex = RandomPatrolPoint(Gen);

	// ������ ���� ������ġ������ ���� ã�´�.
	st_Vector2Int MonsterPosition = GetCellPosition();
	vector<st_Vector2Int> Path = _Channel->_Map->FindPath(MonsterPosition, _PatrolPositions[RandomIndex]);
	if (Path.size() < 2)
	{
		// ���� ��ġ�� �̵� �� �� ���� ��� Idle���·� �ٲ��ش�.
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

	// Ÿ���� ���ų� ���� �ٸ� ä�ο� ���� ��� Idle ���·� ��ȯ�Ѵ�.
	if (_Target == nullptr || _Target->_Channel != _Channel)
	{
		_Target = nullptr;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	st_Vector2Int TargetPosition = _Target->GetCellPosition();
	st_Vector2Int MonsterPosition = GetCellPosition();

	// ���Ⱚ ���Ѵ�.
	st_Vector2Int Direction = TargetPosition - MonsterPosition;
	// Ÿ�ٰ� ������ �Ÿ��� ���.
	int32 Distance = st_Vector2Int::Distance(TargetPosition, MonsterPosition);
	// Ÿ�ٰ��� �Ÿ��� 0 �Ǵ� �߰ݰŸ� ���� �־�����
	if (Distance == 0 || Distance > _ChaseCellDistance)
	{
		_Target = nullptr;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	vector<st_Vector2Int> Path = _Channel->_Map->FindPath(MonsterPosition, TargetPosition);
	// �߰��߿� �÷��̾����� �ٰ����� ���ų� �߰ݰŸ��� ����� �����.
	if (Path.size() < 2 || Distance > _ChaseCellDistance)
	{
		_Target = nullptr;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	// ������ �Ÿ��� ���� �����ȿ� �ְ�, �밢���� ���� ������ ���� ���·� �ٲ۴�.
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

		_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetDirectionFromVector(Direction);

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

		float CriticalPoint = _GameObjectInfo.ObjectStatInfo.CriticalPoint / 1000.0f;
		bernoulli_distribution CriticalCheck(CriticalPoint);
		bool IsCritical = CriticalCheck(Eng);

		// ������ �Ǵ�
		mt19937 Gen(Seed());
		uniform_int_distribution<int> DamageChoiceRandom(_GameObjectInfo.ObjectStatInfo.MinAttackDamage, _GameObjectInfo.ObjectStatInfo.MaxAttackDamage);
		int32 ChoiceDamage = DamageChoiceRandom(Gen);
		int32 FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

		_Target->OnDamaged(this, FinalDamage);

		CMessage* ResBearAttackPacket = G_ObjectManager->GameServer->MakePacketResAttack(_GameObjectInfo.ObjectId, _Target->_GameObjectInfo.ObjectId, en_SkillType::SKILL_BEAR_NORMAL, FinalDamage, IsCritical);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResBearAttackPacket);
		ResBearAttackPacket->Free();

		// ���� �÷��̾�鿡�� ������ ���� ��� ����
		BroadCastPacket(en_PACKET_S2C_CHANGE_OBJECT_STAT);

		// 1.2�ʸ��� ����
		_AttackTick = GetTickCount64() + _AttackTickPoint;

		wchar_t BearAttackMessage[64] = L"0";
		wsprintf(BearAttackMessage, L"%s�� �Ϲ� ������ ����� %s���� %d�� �������� ����ϴ�", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

		wstring BearAttackString = BearAttackMessage;

		CMessage* ResSlimeSystemMessage = G_ObjectManager->GameServer->MakePacketResChattingBoxMessage(_Target->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, IsCritical ? st_Color::Red() : st_Color::White(), IsCritical ? L"ġ��Ÿ! " + BearAttackString : BearAttackString);
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
