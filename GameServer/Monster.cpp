#include "pch.h"
#include "Monster.h"
#include "Player.h"
#include "DataManager.h"
#include "ObjectManager.h"
#include "NetworkManager.h"
#include "MapManager.h"
#include "ChannelManager.h"
#include "Skill.h"
#include "RectCollision.h"

CMonster::CMonster()
{
	_CurrentSkill = nullptr;

	_SearchTick = GetTickCount64();
	_MoveTick = GetTickCount64();
	_PatrolTick = GetTickCount64();

	_MonsterState = en_MonsterState::MONSTER_IDLE;
	_MovePoint = st_Vector2::Zero();
	_PatrolPoint = st_Vector2::Zero();

	_DeadTick = 0;

	_Target = nullptr;		

	_GameObjectInfo.ObjectPositionInfo.LookAtDireciton = st_Vector2::Right();
}

CMonster::~CMonster()
{
	
}

void CMonster::Update()	
{
	CGameObject::Update();

	if (_Target != nullptr && 
		(_Target->_NetworkState == en_ObjectNetworkState::LEAVE 			
			|| _Target->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::DEAD))
	{
		_Target = nullptr;
	}

	// ��ȭ, ��ȭȿ�� ������Ʈ
	CheckBufDeBufSkill();		
	// ���� ��ų ������Ʈ
	_MonsterSkillBox.Update();

	if (_RectCollision != nullptr)
	{
		_RectCollision->Update();
	}

	// ������ �� ���� �����̻� �ɸ� ��� ���¿� ���� ������Ʈ �������� �ʰ���
	if (CheckCantControlStatusAbnormal())
	{
		return;
	}

	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::SPAWN_READY:
		UpdateSpawnReady();
		break;
	case en_CreatureState::SPAWN_IDLE:
		UpdateSpawnIdle();
		break;
	case en_CreatureState::IDLE:
		UpdateIdle();
		break;
	case en_CreatureState::RETURN_SPAWN_POSITION:
		UpdateReturnSpawnPosition();
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
		{
			switch (_MonsterState)
			{							
			case en_MonsterState::MONSTER_READY_MOVE:	
				ReadMoving();
				break;
			case en_MonsterState::MONSTER_MOVE:
				UpdateMoving();
				break;				
			}
		}
		break;
	case en_CreatureState::ATTACK:
		UpdateAttack();
		break;				
	case en_CreatureState::DEAD:
		UpdateDead();
		break;
	}	

	// ��ǥ���� ã��
	FindTarget();
	// ��׷� ���̺� �˻�
	AggroTargetListCheck();
	// ��׷� ���̺��� ��ǥ���� ����
	SelectTarget();
	// ��ǥ�� �˻�
	SelectTargetCheck();	
}

void CMonster::PositionReset()
{
}

bool CMonster::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	if (_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE
		|| _GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::RETURN_SPAWN_POSITION		
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

CGameObject* CMonster::GetTarget()
{
	return _Target;
}

void CMonster::AggroTargetListCheck()
{
	for (auto AggroTargetIterator : _AggroTargetList)
	{
		CGameObject* AggroTarget = AggroTargetIterator.second.AggroTarget;

		if (AggroTarget != nullptr)
		{
			// ��׷� ��Ͽ��� ������ ���� 
			if (AggroTarget->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::DEAD
				|| AggroTarget->_NetworkState == en_ObjectNetworkState::LEAVE
				|| AggroTarget->_NetworkState == en_ObjectNetworkState::READY)
			{
				_AggroTargetList.erase(AggroTargetIterator.first);
			}

			float Distance = st_Vector2::Distance(AggroTarget->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);

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

void CMonster::SelectTargetCheck()
{
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::RETURN_SPAWN_POSITION:			
		break;
	case en_CreatureState::PATROL:
		if (_Target != nullptr)
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
		}
		break;
	case en_CreatureState::MOVING:		
		if (_Target == nullptr)
		{	
			// ������ ����� ���̻� ��������
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::RETURN_SPAWN_POSITION;
			_MonsterState = en_MonsterState::MONSTER_MOVE;
		}
		else
		{
			// ��ǥ������ �Ÿ��� ����
			float TargetDistance = st_Vector2::Distance(_Target->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);
			// �߰� �Ÿ� �̻����� �Ÿ��� �־�����
			if (TargetDistance >= _GameObjectInfo.ObjectStatInfo.ChaseDistance)
			{
				// ��׷� ��Ͽ��� ��ǥ���� �����Ѵ�.
				auto TargetAggroIter = _AggroTargetList.find(_Target->_GameObjectInfo.ObjectId);
				if (TargetAggroIter != _AggroTargetList.end())
				{
					_AggroTargetList.erase(TargetAggroIter);
				}

				// ��ǥ���� �ٽ� ã�´�.
				SelectTarget();

				if (_Target != nullptr)
				{
					// ReadyMove�� �ٲ㼭 ��ǥ���� ���� ��ġ�� ���� ����
					_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
				}
				else
				{
					_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::RETURN_SPAWN_POSITION;
				}				
			}
						
			if (_Target != nullptr && TargetAttackCheck(_GameObjectInfo.ObjectStatInfo.MovingAttackRange) == true)
			{
				_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
				_MonsterState = en_MonsterState::MONSTER_ATTACK;

				_DefaultAttackTick = 0;

				vector<st_FieldOfViewInfo> CurrentFieldOfView = _Channel->GetMap()->GetFieldAroundPlayers(this, false);
				if (CurrentFieldOfView.size() > 0)
				{
					CMessage* ResAttackPacket = G_NetworkManager->GetGameServer()->MakePacketResToAttack(_GameObjectInfo.ObjectId,
						_Target->_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectPositionInfo.Position._X,
						_GameObjectInfo.ObjectPositionInfo.Position._Y,
						_GameObjectInfo.ObjectPositionInfo.State);
					G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfView, ResAttackPacket);
					ResAttackPacket->Free();					
				}
			}
		}
		break;
	case en_CreatureState::ATTACK:
		{
			if (_Target != nullptr)
			{
				float Distance = st_Vector2::Distance(_Target->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);
				if (Distance > _GameObjectInfo.ObjectStatInfo.AttackRange)
				{
					_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
					_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
				}
			}
		}
		break;
	}	
}

void CMonster::FindTarget()
{	
	en_MonsterAggroType MonsterAggroType;

	bool Cango = false;
	CGameObject* Target = _Channel->GetMap()->MonsterReqFindNearPlayer(this, &MonsterAggroType, &Cango);
	if (Target != nullptr)
	{
		// ��׷� ���̺� ã�� ����� ������ ����
		if (_AggroTargetList.find(Target->_GameObjectInfo.ObjectId) != _AggroTargetList.end())
		{
			return;
		}

		// ��ǥ���� ã��

		// ��׷� ����ؼ� ���
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
	}
}

void CMonster::UpdateSpawnReady()
{
	if (_ReSpawnTick < GetTickCount64())
	{
		CMap* Map = G_MapManager->GetMap(1);
		if (Map != nullptr)
		{
			CGameObject* FindObject = Map->Find(_SpawnPosition);
			if (FindObject == nullptr)
			{
				// ä�ο��� ����
				st_GameObjectJob* LeaveChannerMonsterJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobLeaveChannel(this);
				_Channel->_ChannelJobQue.Enqueue(LeaveChannerMonsterJob);
				
				CGameObject* NewMonster = G_ObjectManager->ObjectCreate(_GameObjectInfo.ObjectType);
				NewMonster->_SpawnPosition = _SpawnPosition;

				// ä�ο��� �����ϰ� �ٽ� ����
				st_GameObjectJob* EnterChannelMonsterJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobObjectEnterChannel(NewMonster);
				_Channel->_ChannelJobQue.Enqueue(EnterChannelMonsterJob);
			}
			else
			{
				_ReSpawnTick = GetTickCount64() + _ReSpawnTime;
			}
		}
	}
}

bool CMonster::UpdateSpawnIdle()
{
	bool ChangeToIdle = CGameObject::UpdateSpawnIdle();

	if (ChangeToIdle)
	{
		
	}	

	return ChangeToIdle;
}

void CMonster::UpdateIdle()
{
	if (_SearchTick > GetTickCount64())
	{
		return;
	}

	_SearchTick = GetTickCount64() + _SearchTickPoint;

	_PatrolTick = GetTickCount64() + _PatrolTickPoint;

	// ���� ���� ����
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::PATROL;
	_MonsterState = en_MonsterState::MONSTER_READY_PATROL;	
}

void CMonster::ReadyPatrol()
{
	if (_PatrolTick > GetTickCount64())
	{
		return;
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
			return;
		}

		// ���� ��ġ�� �����Ѵ�
		_MonsterState = en_MonsterState::MONSTER_PATROL;
		_PatrolPoint = PositionCheck(Path[1]);		

		// ���� ��ġ�� ���ϴ� ���Ⱚ ���ؼ� ����
		st_Vector2 DirectionPatrolPoint = _PatrolPoint - _GameObjectInfo.ObjectPositionInfo.Position;
		_GameObjectInfo.ObjectPositionInfo.LookAtDireciton = DirectionPatrolPoint.Normalize();
		_GameObjectInfo.ObjectPositionInfo.MoveDirection = DirectionPatrolPoint.Normalize();

		// ���� �÷��̾�鿡�� ���� ���� �˸�
		vector<st_FieldOfViewInfo> CurrentFieldOfView = _Channel->GetMap()->GetFieldAroundPlayers(this);
		if (CurrentFieldOfView.size() > 0)
		{
			CMessage* MonsterMoveStartPacket = G_NetworkManager->GetGameServer()->MakePacketResMove(_GameObjectInfo.ObjectId,
				_GameObjectInfo.ObjectPositionInfo.LookAtDireciton,
				_GameObjectInfo.ObjectPositionInfo.MoveDirection,
				_GameObjectInfo.ObjectPositionInfo.Position);
			G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfView, MonsterMoveStartPacket);
			MonsterMoveStartPacket->Free();
		}		
	} while (0);
}

void CMonster::UpdatePatrol()
{	
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

	if (_Target != nullptr)
	{
		// Ÿ�� ��ġ���� ���� ã�´�.
		vector<st_Vector2Int> MovePath = _Channel->GetMap()->FindPath(this, _GameObjectInfo.ObjectPositionInfo.CollisionPosition, _Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
		if (MovePath.size() < 2)
		{
			// Ÿ�� ��ġ�� �̵� �� �� ���� ��� ���°��� IDLE�� �ʱ�ȭ�Ѵ�.
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
			return;
		}

		_MovePoint = PositionCheck(MovePath[1]);
		_MonsterState = en_MonsterState::MONSTER_MOVE;

		// ������ ��ġ�� ���ϴ� ���Ⱚ ���ؼ� ����
		st_Vector2 DirectionMovePoint = _MovePoint - _GameObjectInfo.ObjectPositionInfo.Position;
		_GameObjectInfo.ObjectPositionInfo.LookAtDireciton = DirectionMovePoint.Normalize();
		_GameObjectInfo.ObjectPositionInfo.MoveDirection = DirectionMovePoint.Normalize();

		// ���� �÷��̾�鿡�� ������ ���� �˸�
		vector<st_FieldOfViewInfo> CurrentFieldOfView = _Channel->GetMap()->GetFieldAroundPlayers(this);
		if (CurrentFieldOfView.size() > 0)
		{
			CMessage* MonsterMoveStartPacket = G_NetworkManager->GetGameServer()->MakePacketResMove(_GameObjectInfo.ObjectId,
				_GameObjectInfo.ObjectPositionInfo.LookAtDireciton,
				_GameObjectInfo.ObjectPositionInfo.MoveDirection,
				_GameObjectInfo.ObjectPositionInfo.Position,
				_Target->_GameObjectInfo.ObjectId);
			G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfView, MonsterMoveStartPacket);
			MonsterMoveStartPacket->Free();
		}
	}	
}

void CMonster::UpdateMoving()
{
	st_Vector2 FaceDirection = _Target->_GameObjectInfo.ObjectPositionInfo.Position - _GameObjectInfo.ObjectPositionInfo.Position;
	_FieldOfDirection = FaceDirection.Normalize();

	vector<st_FieldOfViewInfo> CurrentFieldOfView = _Channel->GetMap()->GetFieldAroundPlayers(this);
	if (CurrentFieldOfView.size() > 0)
	{	
		CMessage* MonsterMoveStartPacket = G_NetworkManager->GetGameServer()->MakePacketResMove(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.LookAtDireciton,
			_GameObjectInfo.ObjectPositionInfo.MoveDirection,
			_GameObjectInfo.ObjectPositionInfo.Position,
			_Target->_GameObjectInfo.ObjectId);
		G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfView, MonsterMoveStartPacket);
		MonsterMoveStartPacket->Free();		
	}		

	Move();	
}

void CMonster::UpdateReturnSpawnPosition()
{
	vector<st_Vector2Int> Path;

	for (st_Vector2Int ReturnSpawnPosition : _PatrolPositions)
	{
		Path = _Channel->GetMap()->FindPath(this, _GameObjectInfo.ObjectPositionInfo.CollisionPosition, ReturnSpawnPosition);
		if (Path.size() < 2)
		{
			continue;
		}

		break;
	}
		
	st_Vector2 DirectionVector = PositionCheck(Path[1]) - _GameObjectInfo.ObjectPositionInfo.Position;
	_GameObjectInfo.ObjectPositionInfo.LookAtDireciton = DirectionVector.Normalize();
	_GameObjectInfo.ObjectPositionInfo.MoveDirection = DirectionVector.Normalize();

	Move();		
}

void CMonster::UpdateAttack()
{
	if (_Target == nullptr)
	{
		return;
	}

	if (_DefaultAttackTick > GetTickCount64())
	{
		return;
	}

	_DefaultAttackTick = GetTickCount64() + _AttackTickPoint;
		
	//Ÿ�ٰ��� �Ÿ��� ���� ������ ����� �ٽ� �߰�
	float Distance = st_Vector2::Distance(_Target->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);	
	bool CanUseAttack = (Distance <= _GameObjectInfo.ObjectStatInfo.AttackRange);
	if (CanUseAttack == false)
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
		_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
		return;
	}	

	_MonsterSkillBox.SkillProcess(this, nullptr, en_SkillType::SKILL_GOBLIN_ACTIVE_MELEE_DEFAULT_ATTACK);	
}

void CMonster::UpdateSpell()
{
	if (_SpellTick < GetTickCount64())
	{
		bool IsSpellSuccess = true;

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;		

		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = GetChannel()->GetMap()->GetFieldAroundPlayers(this, false);

		if (_SpellSkill != nullptr)
		{
			// ������ ��ų ��Ÿ�� ����
			_SpellSkill->CoolTimeStart();

			// ����â ��
			CMessage* ResMagicPacket = G_NetworkManager->GetGameServer()->MakePacketResMagic(_GameObjectInfo.ObjectId, false);
			G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResMagicPacket);
			ResMagicPacket->Free();

			if (_Target != nullptr)
			{
				switch (_SpellSkill->GetSkillInfo()->SkillType)
				{				
				default:
					break;
				}

				if (IsSpellSuccess == true)
				{
					// �Ϲ�, ����, ����� ó��			
					switch (_SpellSkill->GetBufDeBufSkillKind())
					{
					case en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NORMAL:						
						break;
					case en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_BUF:
						break;
					case en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_DEBUF:
						{
							auto DeBufsIter = _Target->_DeBufs.find(_SpellSkill->GetSkillInfo()->SkillType);
							if (DeBufsIter != _Target->_DeBufs.end())
							{
								// ���� �Ϸ�� ��ų�� ������ ����� ��Ͽ� ���� ���
								CSkill* DeBufSkill = DeBufsIter->second;
								if (DeBufSkill != nullptr)
								{
									DeBufSkill->GetSkillInfo()->SkillOverlapStep++;

									DeBufSkill->StatusAbnormalDurationTimeStart();

									CMessage* ResBufDeBufSkillPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBuf(_Target->_GameObjectInfo.ObjectId, false, DeBufSkill->GetSkillInfo());
									G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
									ResBufDeBufSkillPacket->Free();
								}
								else
								{
									CRASH("DeBufSkill nullptr")
								}
							}
							else
							{
								// ���� �Ϸ�� ��ų�� ������ ���� ��Ͽ� ���� ���
								switch (_SpellSkill->GetSkillInfo()->SkillType)
								{								
								default:
									break;
								}
							}
						}
						break;
					default:
						break;
					}
				}
			}
		}			
	}
}

void CMonster::UpdateRooting()
{

}

void CMonster::UpdateDead()
{
	// ���� �غ� ���¿� �����ϸ� �������� �ð� ��ŭ ��� �� ���� ����
	if (_DeadTick < GetTickCount64())
	{
		_ReSpawnTick = GetTickCount64() + _ReSpawnTime;

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_READY;

		if (_Channel == nullptr)
		{
			CRASH("�����Ϸ��� ä���� �������� ����");
		}

		// ���Ͱ� �־��� �ڸ��� ���
		st_GameObjectJob* DeSpawnMonsterChannelJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobObjectDeSpawnObjectChannel(this);
		_Channel->_ChannelJobQue.Enqueue(DeSpawnMonsterChannelJob);
	}
}

void CMonster::Move()
{	
	st_Vector2 NextPosition;

	bool CanMove = _Channel->GetMap()->MonsterCango(this, &NextPosition);
	if (CanMove == true)
	{
		_GameObjectInfo.ObjectPositionInfo.Position = NextPosition;		

		st_Vector2Int CollisionPosition;
		CollisionPosition._X = (int32)_GameObjectInfo.ObjectPositionInfo.Position._X;
		CollisionPosition._Y = (int32)_GameObjectInfo.ObjectPositionInfo.Position._Y;

		if (CollisionPosition._X != _GameObjectInfo.ObjectPositionInfo.CollisionPosition._X
			|| CollisionPosition._Y != _GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y)
		{
			_Channel->GetMap()->ApplyMove(this, CollisionPosition);
		}
		
		// �����̰� ���� �߰��� �˻�
		switch (_GameObjectInfo.ObjectPositionInfo.State)
		{
		case en_CreatureState::PATROL:
			{				
				// ���� ���� ����
				float Distance = st_Vector2::Distance(_PatrolPoint, _GameObjectInfo.ObjectPositionInfo.Position);
				if (Distance < 0.05f)
				{				
					_GameObjectInfo.ObjectPositionInfo.MoveDirection = st_Vector2::Zero();
					// �ٽ� ���� ���� ���� �� �ֵ��� ��
					_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
					_PatrolTick = GetTickCount64() + _PatrolTickPoint;

					// ���� �÷��̾� �鿡�� ����ٰ� �˷���
					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldAroundPlayers(this);

					CMessage* ResMoveStopPacket = G_NetworkManager->GetGameServer()->MakePacketResMoveStop(_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectPositionInfo.Position._X,
						_GameObjectInfo.ObjectPositionInfo.Position._Y);
					G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIds, ResMoveStopPacket);
					ResMoveStopPacket->Free();
				}
			}
			break;		
		case en_CreatureState::MOVING:
			{
				float Ditance = st_Vector2::Distance(_MovePoint, _GameObjectInfo.ObjectPositionInfo.Position);
				if (Ditance < 0.05f)
				{
					_MonsterState = en_MonsterState::MONSTER_READY_MOVE;

					// ���� �÷��̾� �鿡�� ����ٰ� �˷���
					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldAroundPlayers(this);

					CMessage* ResMoveStopPacket = G_NetworkManager->GetGameServer()->MakePacketResMoveStop(_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectPositionInfo.Position._X,
						_GameObjectInfo.ObjectPositionInfo.Position._Y);
					G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIds, ResMoveStopPacket);
					ResMoveStopPacket->Free();
				}
			}
			break;
		case en_CreatureState::RETURN_SPAWN_POSITION:
			{
				_GameObjectInfo.ObjectPositionInfo.MoveDirection = st_Vector2::Zero();

				st_Vector2Int MonsterPosition = _GameObjectInfo.ObjectPositionInfo.CollisionPosition;

				if (st_Vector2Int::Distance(_SpawnPosition, _GameObjectInfo.ObjectPositionInfo.CollisionPosition) <= 2)
				{
					_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
					_MonsterState = en_MonsterState::MONSTER_IDLE;
				}
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
			_MonsterState = en_MonsterState::MONSTER_READY_MOVE;		
			_MoveTick = GetTickCount64() + 300;
			break;
		case en_CreatureState::ATTACK:
			_MonsterState = en_MonsterState::MONSTER_ATTACK;
			break;
		}			

		// ���� �÷��̾� �鿡�� ����ٰ� �˷���
		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldAroundPlayers(this);

		CMessage* ResMoveStopPacket = G_NetworkManager->GetGameServer()->MakePacketResMoveStop(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.Position._X,
			_GameObjectInfo.ObjectPositionInfo.Position._Y);
		G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIds, ResMoveStopPacket);
		ResMoveStopPacket->Free();
	}	
}

bool CMonster::TargetAttackCheck(float CheckDistance)
{
	// ��ǥ���� �Ÿ��� ����ؼ� ���� ���� �Ǵ�
	float Distance = st_Vector2::Distance(_Target->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);
	if (Distance <= CheckDistance)
	{		
		st_Vector2 DirectionVector = _Target->_GameObjectInfo.ObjectPositionInfo.Position - _GameObjectInfo.ObjectPositionInfo.Position;
		st_Vector2 NormalVector = DirectionVector.Normalize();
		
		_GameObjectInfo.ObjectPositionInfo.LookAtDireciton = NormalVector;		
		_GameObjectInfo.ObjectPositionInfo.MoveDirection = st_Vector2::Zero();
		
		return true;
	}

	return false;	
}
