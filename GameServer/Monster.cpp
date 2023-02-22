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
	_CurrentSkill = nullptr;

	_SearchTick = GetTickCount64();
	_MoveTick = GetTickCount64();
	_PatrolTick = GetTickCount64();

	_MonsterState = en_MonsterState::MONSTER_IDLE;
	_MovePoint = st_Vector2::Zero();
	_PatrolPoint = st_Vector2::Zero();

	_DeadReadyTick = 0;

	_Target = nullptr;		
}

CMonster::~CMonster()
{
	
}

void CMonster::Update()	
{
	CGameObject::Update();

	if (_Target != nullptr && 
		(_Target->_NetworkState == en_ObjectNetworkState::LEAVE 
			|| _Target->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::READY_DEAD
			|| _Target->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::DEAD))
	{
		_Target = nullptr;
	}

	// 강화, 약화효과 업데이트
	CheckBufDeBufSkill();		
	// 몬스터 스킬 업데이트
	_MonsterSkillBox.Update();

	if (_RectCollision != nullptr)
	{
		_RectCollision->Update();
	}

	// 움직일 수 없는 상태이상에 걸릴 경우 상태에 따른 업데이트 적용하지 않게함
	if (CheckCantControlStatusAbnormal())
	{
		return;
	}

	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
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
	case en_CreatureState::READY_DEAD:
		UpdateReadyDead();
		break;
	case en_CreatureState::DEAD:
		UpdateDead();
		break;
	}	

	// 목표물을 찾음
	FindTarget();
	// 어그로 테이블 검사
	AggroTargetListCheck();
	// 어그로 테이블에서 목표물을 선택
	SelectTarget();
	// 목표물 검사
	SelectTargetCheck();	
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
			// 어그로 목록에서 삭제할 조건 
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

void CMonster::SelectTargetCheck()
{
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::RETURN_SPAWN_POSITION:
		{
			st_Vector2Int MonsterPosition = _GameObjectInfo.ObjectPositionInfo.CollisionPosition;
			
			if (st_Vector2Int::Distance(_SpawnPosition, _GameObjectInfo.ObjectPositionInfo.CollisionPosition) <= 2)
			{
				_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
				_MonsterState = en_MonsterState::MONSTER_IDLE;				
			}
		}
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
			// 추적할 대상이 더이상 없어지면
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::RETURN_SPAWN_POSITION;
			_MonsterState = en_MonsterState::MONSTER_MOVE;
		}
		else
		{
			// 목표물까지 거리를 구함
			float TargetDistance = st_Vector2::Distance(_Target->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);
			// 추격 거리 이상으로 거리가 멀어지면
			if (TargetDistance >= _GameObjectInfo.ObjectStatInfo.ChaseDistance)
			{
				// 어그로 목록에서 목표물을 삭제한다.
				auto TargetAggroIter = _AggroTargetList.find(_Target->_GameObjectInfo.ObjectId);
				if (TargetAggroIter != _AggroTargetList.end())
				{
					_AggroTargetList.erase(TargetAggroIter);
				}

				// 목표물을 다시 찾는다.
				SelectTarget();

				// ReadyMove로 바꿔서 목표물로 가는 위치를 새로 잡음
				_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
			}
						
			if (_Target != nullptr && TargetAttackCheck(_GameObjectInfo.ObjectStatInfo.MovingAttackRange) == true)
			{
				_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
				_MonsterState = en_MonsterState::MONSTER_ATTACK;

				_DefaultAttackTick = 0;

				vector<st_FieldOfViewInfo> CurrentFieldOfView = _Channel->GetMap()->GetFieldOfViewAttackObjects(this, 1);
				if (CurrentFieldOfView.size() > 0)
				{
					CMessage* MonsterMoveStop = G_ObjectManager->GameServer->MakePacketResMoveStop(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.Position._X, _GameObjectInfo.ObjectPositionInfo.Position._Y);
					G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfView, MonsterMoveStop);
					MonsterMoveStop->Free();
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
		// 어그로 테이블에 찾은 대상이 있으면 무시
		if (_AggroTargetList.find(Target->_GameObjectInfo.ObjectId) != _AggroTargetList.end())
		{
			return;
		}

		// 목표물을 찾음

		// 어그로 계산해서 기록
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

	// 정찰 상태 진입
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
		// 몬스터를 생성할때 정해준 스폰 위치를 기준으로 저장해둔 정찰 위치 중
		// 랜덤으로 정찰 위치를 얻는다.
		int8 MaxPatrolIndex = (int8)_PatrolPositions.size();
		uniform_int_distribution<int> RandomPatrolPoint(0, MaxPatrolIndex - 1);
		int8 RandomIndex = RandomPatrolPoint(Gen);

		// 앞서 얻은 정찰위치까지의 길을 찾는다.
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
			// 정찰 위치로 이동 할 수 없을 경우 상태값을 IDLE로 초기화한다.
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			_MonsterState = en_MonsterState::MONSTER_IDLE;			
			return;
		}

		// 정찰 위치를 저장한다
		_MonsterState = en_MonsterState::MONSTER_PATROL;
		_PatrolPoint = PositionCheck(Path[1]);

		// 정찰 위치로 향하는 방향값 구해서 저장
		st_Vector2 DirectionPatrolPoint = _PatrolPoint - _GameObjectInfo.ObjectPositionInfo.Position;
		_GameObjectInfo.ObjectPositionInfo.Direction = DirectionPatrolPoint.Normalize();

		// 주위 플레이어들에게 정찰 시작 알림
		vector<st_FieldOfViewInfo> CurrentFieldOfView = _Channel->GetMap()->GetFieldAroundPlayers(this);
		if (CurrentFieldOfView.size() > 0)
		{
			CMessage* MonsterMoveStartPacket = G_ObjectManager->GameServer->MakePacketResMove(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.Direction._X, _GameObjectInfo.ObjectPositionInfo.Direction._Y, 
				_GameObjectInfo.ObjectPositionInfo.Position._X, _GameObjectInfo.ObjectPositionInfo.Position._Y);
			G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfView, MonsterMoveStartPacket);
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

	if (_Target != nullptr)
	{
		// 타겟 위치까지 길을 찾는다.
		vector<st_Vector2Int> MovePath = _Channel->GetMap()->FindPath(this, _GameObjectInfo.ObjectPositionInfo.CollisionPosition, _Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
		if (MovePath.size() < 2)
		{
			// 타겟 위치로 이동 할 수 없을 경우 상태값을 IDLE로 초기화한다.
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
			_MonsterState = en_MonsterState::MONSTER_READY_MOVE;
			return;
		}

		_MovePoint = PositionCheck(MovePath[1]);
		_MonsterState = en_MonsterState::MONSTER_MOVE;

		// 움직임 위치로 향하는 방향값 구해서 저장
		st_Vector2 DirectionPatrolPoint = _MovePoint - _GameObjectInfo.ObjectPositionInfo.Position;
		_GameObjectInfo.ObjectPositionInfo.Direction = DirectionPatrolPoint.Normalize();

		// 주위 플레이어들에게 움직임 시작 알림
		vector<st_FieldOfViewInfo> CurrentFieldOfView = _Channel->GetMap()->GetFieldAroundPlayers(this);
		if (CurrentFieldOfView.size() > 0)
		{
			CMessage* MonsterMoveStartPacket = G_ObjectManager->GameServer->MakePacketResMove(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.Direction._X, _GameObjectInfo.ObjectPositionInfo.Direction._Y,
				_GameObjectInfo.ObjectPositionInfo.Position._X, _GameObjectInfo.ObjectPositionInfo.Position._Y);
			G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfView, MonsterMoveStartPacket);
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
		CMessage* MonsterMoveStartPacket = G_ObjectManager->GameServer->MakePacketResMove(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.Direction._X, _GameObjectInfo.ObjectPositionInfo.Direction._Y,
			_GameObjectInfo.ObjectPositionInfo.Position._X, _GameObjectInfo.ObjectPositionInfo.Position._Y);
		G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfView, MonsterMoveStartPacket);
		MonsterMoveStartPacket->Free();		

		CMessage* ResFaceDirectionPacket = G_ObjectManager->GameServer->MakePacketResFaceDirection(_GameObjectInfo.ObjectId, _Target->_GameObjectInfo.ObjectPositionInfo.Position._X, _Target->_GameObjectInfo.ObjectPositionInfo.Position._Y);
		G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfView, ResFaceDirectionPacket);
		ResFaceDirectionPacket->Free();
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
	st_Vector2 NormalVector = DirectionVector.Normalize();

	_GameObjectInfo.ObjectPositionInfo.Direction = NormalVector;	

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
		
	//타겟과의 거리가 공격 범위를 벗어나면 다시 추격
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
			// 시전한 스킬 쿨타임 시작
			_SpellSkill->CoolTimeStart();

			// 스펠창 끝
			CMessage* ResMagicPacket = G_ObjectManager->GameServer->MakePacketResMagic(_GameObjectInfo.ObjectId, false);
			G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResMagicPacket);
			ResMagicPacket->Free();

			if (_Target != nullptr)
			{
				switch (_SpellSkill->GetSkillInfo()->SkillType)
				{
				case en_SkillType::SKILL_SLIME_ACTIVE_POISION_ATTACK:
				{
					float Distance = st_Vector2::Distance(_Target->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);
					if (Distance > _SpellSkill->GetSkillInfo()->SkillDistance)
					{
						IsSpellSuccess = false;
					}
				}
				break;
				default:
					break;
				}

				if (IsSpellSuccess == true)
				{
					// 일반, 버프, 디버프 처리			
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
								// 시전 완료된 스킬이 상대방의 디버프 목록에 있을 경우
								CSkill* DeBufSkill = DeBufsIter->second;
								if (DeBufSkill != nullptr)
								{
									DeBufSkill->GetSkillInfo()->SkillOverlapStep++;

									DeBufSkill->StatusAbnormalDurationTimeStart();

									CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_Target->_GameObjectInfo.ObjectId, false, DeBufSkill->GetSkillInfo());
									G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
									ResBufDeBufSkillPacket->Free();
								}
								else
								{
									CRASH("DeBufSkill nullptr")
								}
							}
							else
							{
								// 시전 완료된 스킬이 상대방의 버프 목록에 없을 경우
								switch (_SpellSkill->GetSkillInfo()->SkillType)
								{
								case en_SkillType::SKILL_SLIME_ACTIVE_POISION_ATTACK:
									{
										CSkill* SlimePoisionAttack = G_ObjectManager->SkillCreate();
										st_SkillInfo* SlimePoisionSkillInfo = G_ObjectManager->SkillInfoCreate(en_SkillType::SKILL_SLIME_ACTIVE_POISION_ATTACK, _SpellSkill->GetSkillInfo()->SkillLevel);
										SlimePoisionAttack->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, SlimePoisionSkillInfo);
										SlimePoisionAttack->StatusAbnormalDurationTimeStart();
										SlimePoisionAttack->SetCastingUserID(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType);

										SlimePoisionAttack->GetSkillInfo()->SkillOverlapStep++;

										_Target->AddDebuf(SlimePoisionAttack);

										CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_Target->_GameObjectInfo.ObjectId, false, SlimePoisionAttack->GetSkillInfo());
										G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
										ResBufDeBufSkillPacket->Free();
									}
									break;
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

void CMonster::UpdateReadyDead()
{		
	// 죽음 준비 상태에 진입하면 지정해준 시간 만큼 대기 후 로직 진행
	if (_DeadReadyTick < GetTickCount64())
	{
		_DeadTick = GetTickCount64() + _ReSpawnTime;

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;
		
		if (_Channel == nullptr)
		{
			CRASH("퇴장하려는 채널이 존재하지 않음");
		}		

		// 몬스터가 있었던 자리를 비우고 소환해제
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
				
				// 채널에서 퇴장
				st_GameObjectJob* LeaveChannerMonsterJob = G_ObjectManager->GameServer->MakeGameObjectJobLeaveChannel(this);
				_Channel->_ChannelJobQue.Enqueue(LeaveChannerMonsterJob);				
				// 채널에서 퇴장하고 다시 입장
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
		
		// 움직이고 나서 추가로 검사
		switch (_GameObjectInfo.ObjectPositionInfo.State)
		{
		case en_CreatureState::PATROL:
			{				
				// 정찰 지점 도착
				float Distance = st_Vector2::Distance(_PatrolPoint, _GameObjectInfo.ObjectPositionInfo.Position);
				if (Distance < 0.05f)
				{				
					// 다시 정찰 지점 얻을 수 있도록 함
					_MonsterState = en_MonsterState::MONSTER_READY_PATROL;
					_PatrolTick = GetTickCount64() + _PatrolTickPoint;

					// 주위 플레이어 들에게 멈췄다고 알려줌
					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldAroundPlayers(this);

					CMessage* ResMoveStopPacket = G_ObjectManager->GameServer->MakePacketResMoveStop(_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectPositionInfo.Position._X,
						_GameObjectInfo.ObjectPositionInfo.Position._Y);
					G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIds, ResMoveStopPacket);
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

					// 주위 플레이어 들에게 멈췄다고 알려줌
					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldAroundPlayers(this);

					CMessage* ResMoveStopPacket = G_ObjectManager->GameServer->MakePacketResMoveStop(_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectPositionInfo.Position._X,
						_GameObjectInfo.ObjectPositionInfo.Position._Y);
					G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIds, ResMoveStopPacket);
					ResMoveStopPacket->Free();
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

		// 주위 플레이어 들에게 멈췄다고 알려줌
		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldAroundPlayers(this);

		CMessage* ResMoveStopPacket = G_ObjectManager->GameServer->MakePacketResMoveStop(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.Position._X,
			_GameObjectInfo.ObjectPositionInfo.Position._Y);
		G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIds, ResMoveStopPacket);
		ResMoveStopPacket->Free();
	}	
}

bool CMonster::TargetAttackCheck(float CheckDistance)
{
	// 목표물과 거리를 계산해서 공격 여부 판단
	float Distance = st_Vector2::Distance(_Target->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);
	if (Distance <= CheckDistance)
	{		
		st_Vector2 DirectionVector = _Target->_GameObjectInfo.ObjectPositionInfo.Position - _GameObjectInfo.ObjectPositionInfo.Position;
		st_Vector2 NormalVector = DirectionVector.Normalize();
		
		_GameObjectInfo.ObjectPositionInfo.Direction = NormalVector;
		
		return true;
	}

	return false;	
}
