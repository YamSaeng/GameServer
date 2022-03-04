#include "pch.h"
#include "Player.h"
#include "ObjectManager.h"
#include "DataManager.h"

CPlayer::CPlayer()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_PLAYER;	
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
	_AttackTick = 0;
	_SpellTick = 0;
	_SkillJob = nullptr;

	_FieldOfViewDistance = 10;

	_IsSendPacketTarget = true;

	_NatureRecoveryTick = GetTickCount64() + 5000;
	_FieldOfViewUpdateTick = GetTickCount64() + 100;
}

CPlayer::~CPlayer()
{
}

void CPlayer::Update()
{		
	// 시야범위 객체 업데이트	
	if (_FieldOfViewUpdateTick < GetTickCount64())
	{	
		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetFieldOfViewObjects(this, 1);
		//vector<st_FieldOfViewInfo> PreviousFieldOfViewObjects = _FieldOfViewInfos;
		vector<st_FieldOfViewInfo> SpawnObjectIds;
		vector<st_FieldOfViewInfo> DeSpawnObjectIds;		

		if (CurrentFieldOfViewObjectIds.size() > 1)
		{
			sort(CurrentFieldOfViewObjectIds.begin(), CurrentFieldOfViewObjectIds.end());
		}

		if (_FieldOfViewInfos.size() > 1)
		{
			sort(_FieldOfViewInfos.begin(), _FieldOfViewInfos.end());
		}		

		SpawnObjectIds.resize(CurrentFieldOfViewObjectIds.size());

		set_difference(CurrentFieldOfViewObjectIds.begin(), CurrentFieldOfViewObjectIds.end(),
			_FieldOfViewInfos.begin(), _FieldOfViewInfos.end(),
			SpawnObjectIds.begin());
		
		DeSpawnObjectIds.resize(_FieldOfViewInfos.size());

		set_difference(_FieldOfViewInfos.begin(), _FieldOfViewInfos.end(),
			CurrentFieldOfViewObjectIds.begin(), CurrentFieldOfViewObjectIds.end(),
			DeSpawnObjectIds.begin());
		
		// 한번 더 검사
		if (SpawnObjectIds.size() > 0)
		{
			// 스폰 해야할 대상들을 스폰
			vector<CGameObject*> SpawnObjectInfos;
			for (st_FieldOfViewInfo SpawnObject : SpawnObjectIds)
			{
				if (SpawnObject.ObjectId != 0 && SpawnObject.ObjectType != en_GameObjectType::NORMAL)
				{
					CGameObject* FindObject = G_ObjectManager->Find(SpawnObject.ObjectId, SpawnObject.ObjectType);
					if (FindObject != nullptr)
					{
						// 시야 범위 안에 존재할 경우 스폰정보 담음
						int16 Distance = st_Vector2Int::Distance(FindObject->GetCellPosition(), GetCellPosition());
						if (Distance <= _FieldOfViewDistance)
						{
							SpawnObjectInfos.push_back(FindObject);
						}						
					}
				}								
			}

			// 스폰해야 할 대상을 나에게 스폰하라고 알림
			if (SpawnObjectInfos.size() > 0)
			{
				CMessage* ResOtherObjectSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn((int32)SpawnObjectInfos.size(), SpawnObjectInfos);
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResOtherObjectSpawnPacket);
				ResOtherObjectSpawnPacket->Free();

				// 스폰 해야할 대상들 기준에서 나와의 거리가 스폰 해야할 대상들 시야범위안에 있을 경우 나를 스폰하라고 알려줌
				SpawnObjectInfos.clear();
				SpawnObjectInfos.push_back(this);

				CMessage* ResMyObjectSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn(1, SpawnObjectInfos);
				for (CGameObject* SpawnObject : SpawnObjectInfos)
				{
					if (SpawnObject->_IsSendPacketTarget == true)
					{
						int16 Distance = st_Vector2Int::Distance(GetCellPosition(), SpawnObject->GetCellPosition());

						if (Distance <= SpawnObject->_FieldOfViewDistance)
						{
							G_ObjectManager->GameServer->SendPacket(((CPlayer*)SpawnObject)->_SessionId, ResMyObjectSpawnPacket);
						}
					}
				}
				ResMyObjectSpawnPacket->Free();
			}						
		}

		// 한번 더 검사
		if (DeSpawnObjectIds.size() > 0)
		{
			vector<CGameObject*> DeSpawnObjectInfos;
			for (st_FieldOfViewInfo DeSpawnObject : DeSpawnObjectIds)
			{
				if (DeSpawnObject.ObjectId != 0 && DeSpawnObject.ObjectType != en_GameObjectType::NORMAL)
				{
					CGameObject* FindObject = G_ObjectManager->Find(DeSpawnObject.ObjectId, DeSpawnObject.ObjectType);

					if (FindObject != nullptr && FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
					{
						DeSpawnObjectInfos.push_back(FindObject);
					}
				}				
			}

			// 디스폰해야 할 대상을 나에게 스폰하라고 알림
			if (DeSpawnObjectInfos.size() > 0)
			{
				CMessage* ResOtherObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn((int32)DeSpawnObjectInfos.size(), DeSpawnObjectInfos);
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResOtherObjectDeSpawnPacket);
				ResOtherObjectDeSpawnPacket->Free();

				// 하지만 나와 대상들의 거리가 대상들 시야범위 안에 속할 경우 나를 디스폰하지는 않는다.
				DeSpawnObjectInfos.clear();
				DeSpawnObjectInfos.push_back(this);

				CMessage* ResMyObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, DeSpawnObjectInfos);
				for (CGameObject* DeSpawnObject : DeSpawnObjectInfos)
				{
					if (DeSpawnObject->_IsSendPacketTarget)
					{
						int16 Distance = st_Vector2Int::Distance(GetCellPosition(), DeSpawnObject->GetCellPosition());
						if (Distance > DeSpawnObject->_FieldOfViewDistance)
						{
							G_ObjectManager->GameServer->SendPacket(((CPlayer*)DeSpawnObject)->_SessionId, ResMyObjectDeSpawnPacket);
						}
					}
				}
				ResMyObjectDeSpawnPacket->Free();
			}			
		}

		_FieldOfViewInfos = CurrentFieldOfViewObjectIds;				
	}	

	PlayerJobQueProc();

	if (_NatureRecoveryTick < GetTickCount64())
	{
		int32 AutoHPRecoveryPoint = 0;
		int32 AutoMPRecoveryPoint = 0;	

		AutoHPRecoveryPoint = (_GameObjectInfo.ObjectStatInfo.MaxHP / 100) * _GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent;
		AutoMPRecoveryPoint = (_GameObjectInfo.ObjectStatInfo.MaxMP / 100) * _GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent;	
						
		_GameObjectInfo.ObjectStatInfo.HP += AutoHPRecoveryPoint;
		_GameObjectInfo.ObjectStatInfo.MP += AutoMPRecoveryPoint;

		if (_GameObjectInfo.ObjectStatInfo.HP > _GameObjectInfo.ObjectStatInfo.MaxHP)
		{
			_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
		}

		if (_GameObjectInfo.ObjectStatInfo.HP < 0)
		{
			_GameObjectInfo.ObjectStatInfo.HP = 0;
		}

		_NatureRecoveryTick = GetTickCount64() + 5000;		
				
		CMessage* ResObjectStatPacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectStatInfo);
		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStatPacket, this);		
		ResObjectStatPacket->Free();
	}
		
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{	
	case en_CreatureState::MOVING:		
		UpdateMove();
		break;
	case en_CreatureState::ATTACK:		
		UpdateAttack();
		break;
	case en_CreatureState::SPELL:		
		UpdateSpell();
		break;	
	}
}

bool CPlayer::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	return CGameObject::OnDamaged(Attacker, Damage);
}

void CPlayer::OnDead(CGameObject* Killer)
{

}

void CPlayer::Init()
{
	wstring InitString = L"";

	_GameObjectInfo.ObjectId = 0;
	_GameObjectInfo.ObjectName = InitString;
	_GameObjectInfo.OwnerObjectType = en_GameObjectType::NORMAL;
	memset(&_GameObjectInfo.ObjectStatInfo, 0, sizeof(st_StatInfo));
	_SpawnPosition._X = 0;
	_SpawnPosition._Y = 0;	
}

void CPlayer::PositionReset()
{
	switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
	{
	case en_MoveDir::LEFT:
		_GameObjectInfo.ObjectPositionInfo.PositionX =
			_GameObjectInfo.ObjectPositionInfo.CollisionPositionX + 0.3f;
		break;
	case en_MoveDir::RIGHT:
		_GameObjectInfo.ObjectPositionInfo.PositionX =
			_GameObjectInfo.ObjectPositionInfo.CollisionPositionX + 0.7f;
		break;
	case en_MoveDir::UP:
		_GameObjectInfo.ObjectPositionInfo.PositionY =
			_GameObjectInfo.ObjectPositionInfo.CollisionPositionY + 0.7f;
		break;
	case en_MoveDir::DOWN:
		_GameObjectInfo.ObjectPositionInfo.PositionY =
			_GameObjectInfo.ObjectPositionInfo.CollisionPositionY + 0.3f;
		break;	
	}	
}

void CPlayer::PlayerJobQueProc()
{
	while (!_PlayerJobQue.IsEmpty())
	{
		st_PlayerJob* PlayerJob = nullptr;

		if (!_PlayerJobQue.Dequeue(&PlayerJob))
		{
			break;
		}

		vector<CGameObject*> Targets;
		st_SkillInfo* FindSkillInfo = nullptr;
		CMessage* ResErrorPacket = nullptr;		
		CMessage* ResMagicCancelPacket = nullptr;

		switch (PlayerJob->Type)
		{	
		case en_PlayerJobType::PLAYER_MELEE_JOB:
			{
				int8 QuickSlotBarIndex;
				*PlayerJob->Message >> QuickSlotBarIndex;

				int8 QuickSlotBarSlotIndex;
				*PlayerJob->Message >> QuickSlotBarSlotIndex;

				// 공격한 방향
				int8 ReqMoveDir;
				*PlayerJob->Message >> ReqMoveDir;

				// 스킬 종류
				int16 ReqSkillType;
				*PlayerJob->Message >> ReqSkillType;

				en_MoveDir MoveDir = (en_MoveDir)ReqMoveDir;
				_GameObjectInfo.ObjectPositionInfo.MoveDir = MoveDir;

				st_Vector2Int FrontCell;				
				CGameObject* Target = nullptr;
				CMessage* ResSyncPositionPacket = nullptr;

				// 퀵바에 등록되지 않은 스킬을 요청했을 경우
				if ((en_SkillType)ReqSkillType == en_SkillType::SKILL_TYPE_NONE)
				{
					break;
				}

				FindSkillInfo = _SkillBox.FindSkill((en_SkillType)ReqSkillType);		

				// 요청한 스킬이 스킬창에 있는지 확인			
				if (FindSkillInfo != nullptr && FindSkillInfo->CanSkillUse == true)
				{
					// 스킬 지정
					_SkillType = (en_SkillType)ReqSkillType;
					// 공격 상태로 변경
					_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
					// 클라에게 알려줘서 공격 애니메이션 출력
					CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectPositionInfo.MoveDir, 
						_GameObjectInfo.ObjectType,
						_GameObjectInfo.ObjectPositionInfo.State);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStateChangePacket, this);
					ResObjectStateChangePacket->Free();

					// 타겟 위치 확인
					switch (FindSkillInfo->SkillType)
					{
					case en_SkillType::SKILL_TYPE_NONE:
						break;
					case en_SkillType::SKILL_DEFAULT_ATTACK:
						FrontCell = GetFrontCellPosition(_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);
						Target = _Channel->_Map->Find(FrontCell);
						if (Target != nullptr)
						{
							Targets.push_back(Target);
						}
						break;
					case en_SkillType::SKILL_KNIGHT_CHOHONE:
					{
						if (_SelectTarget != nullptr)
						{
							st_Vector2Int TargetPosition = G_ObjectManager->Find(_SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectType)->GetCellPosition();
							st_Vector2Int MyPosition = GetCellPosition();
							st_Vector2Int Direction = TargetPosition - MyPosition;

							int32 Distance = st_Vector2Int::Distance(TargetPosition, MyPosition);

							if (Distance <= 4)
							{
								Target = _Channel->_Map->Find(TargetPosition);
								if (Target != nullptr)
								{
									// 내 앞쪽 위치를 구한다.
									st_Vector2Int MyFrontCellPotision = GetFrontCellPosition(_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);

									if (_Channel->_Map->ApplyMove(Target, MyFrontCellPotision))
									{
										Targets.push_back(Target);

										switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
										{
										case en_MoveDir::UP:
											Target->_GameObjectInfo.ObjectPositionInfo.PositionX = _GameObjectInfo.ObjectPositionInfo.PositionX;
											Target->_GameObjectInfo.ObjectPositionInfo.PositionY = MyFrontCellPotision._Y + 0.5f;
											break;
										case en_MoveDir::DOWN:
											Target->_GameObjectInfo.ObjectPositionInfo.PositionX = _GameObjectInfo.ObjectPositionInfo.PositionX;
											Target->_GameObjectInfo.ObjectPositionInfo.PositionY = MyFrontCellPotision._Y + 0.5f;
											break;
										case en_MoveDir::LEFT:
											Target->_GameObjectInfo.ObjectPositionInfo.PositionX = MyFrontCellPotision._X + 0.5f;
											Target->_GameObjectInfo.ObjectPositionInfo.PositionY = _GameObjectInfo.ObjectPositionInfo.PositionY;
											break;
										case en_MoveDir::RIGHT:
											Target->_GameObjectInfo.ObjectPositionInfo.PositionX = MyFrontCellPotision._X + 0.5f;
											Target->_GameObjectInfo.ObjectPositionInfo.PositionY = _GameObjectInfo.ObjectPositionInfo.PositionY;
											break;
										}

										ResSyncPositionPacket = G_ObjectManager->GameServer->MakePacketResSyncPosition(Target->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectPositionInfo);
										G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResSyncPositionPacket, this);
										ResSyncPositionPacket->Free();
									}
									else
									{
										ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(_GameObjectInfo.ObjectId, en_SkillErrorType::ERROR_PLACE_BLOCK, FindSkillInfo->SkillName.c_str());
										G_ObjectManager->GameServer->SendPacket(_SessionId, ResErrorPacket);
										ResErrorPacket->Free();
									}
								}
							}
							else
							{
								ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(_GameObjectInfo.ObjectId, en_SkillErrorType::ERROR_DISTANCE, FindSkillInfo->SkillName.c_str(), Distance);
								G_ObjectManager->GameServer->SendPacket(_SessionId, ResErrorPacket);
								ResErrorPacket->Free();
							}
						}
						else
						{
							ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(_GameObjectInfo.ObjectId, en_SkillErrorType::ERROR_NON_SELECT_OBJECT, FindSkillInfo->SkillName.c_str());
							G_ObjectManager->GameServer->SendPacket(_SessionId, ResErrorPacket);
							ResErrorPacket->Free();
						}
					}
					break;
					case en_SkillType::SKILL_KNIGHT_SHAEHONE:
					{
						if (_SelectTarget != nullptr)
						{
							st_Vector2Int TargetPosition = G_ObjectManager->Find(_SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectType)->GetCellPosition();
							st_Vector2Int MyPosition = GetCellPosition();
							st_Vector2Int Direction = TargetPosition - MyPosition;

							// 타겟이 어느 방향에 있는지 확인한다.
							en_MoveDir Dir = st_Vector2Int::GetMoveDir(Direction);
							// 타겟과의 거리를 구한다.
							int32 Distance = st_Vector2Int::Distance(TargetPosition, MyPosition);

							if (Distance <= 4)
							{
								Target = _Channel->_Map->Find(TargetPosition);
								st_Vector2Int MovePosition;
								if (Target != nullptr)
								{
									switch (Dir)
									{
									case en_MoveDir::UP:
										MovePosition = Target->GetFrontCellPosition(en_MoveDir::DOWN, 1);
										break;
									case en_MoveDir::DOWN:
										MovePosition = Target->GetFrontCellPosition(en_MoveDir::UP, 1);
										break;
									case en_MoveDir::LEFT:
										MovePosition = Target->GetFrontCellPosition(en_MoveDir::RIGHT, 1);
										break;
									case en_MoveDir::RIGHT:
										MovePosition = Target->GetFrontCellPosition(en_MoveDir::LEFT, 1);
										break;
									default:
										break;
									}

									if (_Channel->_Map->ApplyMove(this, MovePosition))
									{
										Targets.push_back(Target);

										switch (Dir)
										{
										case en_MoveDir::UP:
											_GameObjectInfo.ObjectPositionInfo.PositionX = Target->_GameObjectInfo.ObjectPositionInfo.PositionX;
											_GameObjectInfo.ObjectPositionInfo.PositionY = MovePosition._Y + 0.5f;
											_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::UP;
											break;
										case en_MoveDir::DOWN:
											_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
											_GameObjectInfo.ObjectPositionInfo.PositionX = Target->_GameObjectInfo.ObjectPositionInfo.PositionX;
											_GameObjectInfo.ObjectPositionInfo.PositionY = MovePosition._Y + 0.5f;
											break;
										case en_MoveDir::LEFT:
											_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::LEFT;
											_GameObjectInfo.ObjectPositionInfo.PositionX = MovePosition._X + 0.5f;
											_GameObjectInfo.ObjectPositionInfo.PositionY = Target->_GameObjectInfo.ObjectPositionInfo.PositionY;
											break;
										case en_MoveDir::RIGHT:
											_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::RIGHT;
											_GameObjectInfo.ObjectPositionInfo.PositionX = MovePosition._X + 0.5f;
											_GameObjectInfo.ObjectPositionInfo.PositionY = Target->_GameObjectInfo.ObjectPositionInfo.PositionY;
											break;
										}

										ResSyncPositionPacket = G_ObjectManager->GameServer->MakePacketResSyncPosition(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo);
										G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResSyncPositionPacket, this);
										ResSyncPositionPacket->Free();
									}
									else
									{
										ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(_GameObjectInfo.ObjectId, en_SkillErrorType::ERROR_PLACE_BLOCK, FindSkillInfo->SkillName.c_str());
										G_ObjectManager->GameServer->SendPacket(_SessionId, ResErrorPacket);
										ResErrorPacket->Free();
									}
								}
							}
							else
							{
								ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(_GameObjectInfo.ObjectId, en_SkillErrorType::ERROR_DISTANCE, FindSkillInfo->SkillName.c_str(), Distance);
								G_ObjectManager->GameServer->SendPacket(_SessionId, ResErrorPacket);
								ResErrorPacket->Free();
							}
						}
						else
						{
							ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(_GameObjectInfo.ObjectId, en_SkillErrorType::ERROR_NON_SELECT_OBJECT, FindSkillInfo->SkillName.c_str());
							G_ObjectManager->GameServer->SendPacket(_SessionId, ResErrorPacket);
							ResErrorPacket->Free();
						}
					}
					break;
					case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
					{
						vector<st_Vector2Int> TargetPositions;

						TargetPositions = GetAroundCellPositions(GetCellPosition(), 1);
						for (st_Vector2Int TargetPosition : TargetPositions)
						{
							CGameObject* Target = _Channel->_Map->Find(TargetPosition);
							if (Target != nullptr)
							{
								Targets.push_back(Target);
							}
						}

						// 이펙트 출력
						CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(_GameObjectInfo.ObjectId, en_EffectType::EFFECT_SMASH_WAVE, 2.0f);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResEffectPacket, this);
						ResEffectPacket->Free();
					}
					break;
					default:
						break;
					}

					wstring SkillTypeString;
					wchar_t SkillTypeMessage[64] = L"0";
					wchar_t SkillDamageMessage[64] = L"0";

					// 타겟 데미지 적용
					for (CGameObject* Target : Targets)
					{
						if (Target->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
						{
							st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)FindSkillInfo;

							// 크리티컬 판단
							random_device Seed;
							default_random_engine Eng(Seed());

							float CriticalPoint = _GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint / 1000.0f;
							bernoulli_distribution CriticalCheck(CriticalPoint);
							bool IsCritical = CriticalCheck(Eng);

							// 데미지 판단
							mt19937 Gen(Seed());
							uniform_int_distribution<int> DamageChoiceRandom(
								_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage + _Equipment._WeaponMinDamage + AttackSkillInfo->SkillMinDamage,
								_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage + _Equipment._WeaponMaxDamage + AttackSkillInfo->SkillMaxDamage);
							int32 ChoiceDamage = DamageChoiceRandom(Gen);

							int32 CriticalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

							float DefenceRate = (float)pow(((float)(200 - Target->_GameObjectInfo.ObjectStatInfo.Defence)) / 20, 2) * 0.01f;

							int32 FinalDamage = CriticalDamage * DefenceRate;

							bool TargetIsDead = Target->OnDamaged(this, FinalDamage);
							if (TargetIsDead == true)
							{
								CMonster* TargetMonster = (CMonster*)Target;
								if (TargetMonster != nullptr)
								{
									_Experience.CurrentExperience += TargetMonster->_GetExpPoint;
									_Experience.CurrentExpRatio = ((float)_Experience.CurrentExperience) / _Experience.RequireExperience;

									if (_Experience.CurrentExpRatio >= 1.0f)
									{
										// 레벨 증가
										_GameObjectInfo.ObjectStatInfo.Level += 1;

										// 해당 레벨에 
										st_ObjectStatusData NewCharacterStatus;
										st_LevelData LevelData;

										switch (_GameObjectInfo.ObjectType)
										{
										case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
										{
											auto FindStatus = G_Datamanager->_WarriorStatus.find(_GameObjectInfo.ObjectStatInfo.Level);
											if (FindStatus == G_Datamanager->_WarriorStatus.end())
											{
												CRASH("레벨 스테이터스 찾지 못함");
											}

											NewCharacterStatus = *(*FindStatus).second;

											_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
											_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
											_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
											_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
											_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
											_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
											_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
											_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
											_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
											_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
											_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
											_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
											_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
											_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
											_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
											_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
											_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
											_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
										}
										break;
										case en_GameObjectType::OBJECT_MAGIC_PLAYER:
										{
											auto FindStatus = G_Datamanager->_ShamanStatus.find(_GameObjectInfo.ObjectStatInfo.Level);
											if (FindStatus == G_Datamanager->_WarriorStatus.end())
											{
												CRASH("레벨 데이터 찾지 못함");
											}

											NewCharacterStatus = *(*FindStatus).second;

											_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
											_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
											_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
											_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
											_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
											_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
											_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
											_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
											_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
											_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
											_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
											_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
											_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
											_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
											_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
											_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
											_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
											_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
										}
										break;
										case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
										{
											auto FindStatus = G_Datamanager->_TaioistStatus.find(_GameObjectInfo.ObjectStatInfo.Level);
											if (FindStatus == G_Datamanager->_TaioistStatus.end())
											{
												CRASH("레벨 데이터 찾지 못함");
											}

											NewCharacterStatus = *(*FindStatus).second;

											_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
											_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
											_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
											_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
											_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
											_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
											_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
											_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
											_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
											_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
											_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
											_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
											_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
											_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
											_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
											_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
											_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
											_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
										}
										break;
										case en_GameObjectType::OBJECT_THIEF_PLAYER:
										{
											auto FindStatus = G_Datamanager->_ThiefStatus.find(_GameObjectInfo.ObjectStatInfo.Level);
											if (FindStatus == G_Datamanager->_ThiefStatus.end())
											{
												CRASH("레벨 데이터 찾지 못함");
											}

											NewCharacterStatus = *(*FindStatus).second;

											_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
											_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
											_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
											_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
											_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
											_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
											_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
											_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
											_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
											_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
											_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
											_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
											_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
											_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
											_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
											_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
											_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
											_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
										}
										break;
										case en_GameObjectType::OBJECT_ARCHER_PLAYER:
										{
											auto FindStatus = G_Datamanager->_ArcherStatus.find(_GameObjectInfo.ObjectStatInfo.Level);
											if (FindStatus == G_Datamanager->_ArcherStatus.end())
											{
												CRASH("레벨 데이터 찾지 못함");
											}

											NewCharacterStatus = *(*FindStatus).second;

											_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
											_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
											_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
											_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
											_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
											_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
											_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
											_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
											_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
											_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
											_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
											_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
											_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
											_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
											_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
											_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
											_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
											_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
										}
										break;
										default:
											break;
										}

										CGameServerMessage* ResObjectStatChangeMessage = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectStatInfo);
										G_ObjectManager->GameServer->SendPacket(_SessionId, ResObjectStatChangeMessage);
										ResObjectStatChangeMessage->Free();

										auto FindLevelData = G_Datamanager->_LevelDatas.find(_GameObjectInfo.ObjectStatInfo.Level);
										if (FindLevelData == G_Datamanager->_LevelDatas.end())
										{
											CRASH("레벨 데이터 찾지 못함");
										}

										LevelData = *(*FindLevelData).second;

										_Experience.CurrentExperience = 0;
										_Experience.RequireExperience = LevelData.RequireExperience;
										_Experience.TotalExperience = LevelData.TotalExperience;
									}

									CGameServerMessage* ResMonsterGetExpMessage = G_ObjectManager->GameServer->MakePacketExperience(_AccountId, _GameObjectInfo.ObjectId, TargetMonster->_GetExpPoint, _Experience.CurrentExperience, _Experience.RequireExperience, _Experience.TotalExperience);
									G_ObjectManager->GameServer->SendPacket(_SessionId, ResMonsterGetExpMessage);
									ResMonsterGetExpMessage->Free();
								}
							}

							en_EffectType HitEffectType;

							// 시스템 메세지 생성
							switch ((en_SkillType)ReqSkillType)
							{
							case en_SkillType::SKILL_TYPE_NONE:
								CRASH("SkillType None");
								break;
							case en_SkillType::SKILL_DEFAULT_ATTACK:
								wsprintf(SkillTypeMessage, L"%s가 일반공격을 사용해 %s에게 %d의 데미지를 줬습니다.", _GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
								HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
								break;
							case en_SkillType::SKILL_KNIGHT_CHOHONE:
								wsprintf(SkillTypeMessage, L"%s가 초혼비무를 사용해 %s에게 %d의 데미지를 줬습니다.", _GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
								HitEffectType = en_EffectType::EFFECT_CHOHONE_TARGET_HIT;
								break;
							case en_SkillType::SKILL_KNIGHT_SHAEHONE:
								wsprintf(SkillTypeMessage, L"%s가 쇄혼비무를 사용해 %s에게 %d의 데미지를 줬습니다.", _GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
								HitEffectType = en_EffectType::EFFECT_SHAHONE_TARGET_HIT;
								break;
							case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
								wsprintf(SkillTypeMessage, L"%s가 분쇄파동을 사용해 %s에게 %d의 데미지를 줬습니다.", _GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
								HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
								break;
							default:
								break;
							}

							SkillTypeString = SkillTypeMessage;
							SkillTypeString = IsCritical ? L"치명타! " + SkillTypeString : SkillTypeString;

							// 데미지 시스템 메세지 전송
							CMessage* ResSkillSystemMessagePacket = G_ObjectManager->GameServer->MakePacketResChattingBoxMessage(_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, IsCritical ? st_Color::Red() : st_Color::White(), SkillTypeString);
							G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResSkillSystemMessagePacket, this);
							ResSkillSystemMessagePacket->Free();

							// 공격 응답 메세지 전송
							CMessage* ResMyAttackOtherPacket = G_ObjectManager->GameServer->MakePacketResAttack(_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectId, (en_SkillType)ReqSkillType, FinalDamage, IsCritical);
							G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMyAttackOtherPacket, this);
							ResMyAttackOtherPacket->Free();

							// 이펙트 출력
							CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(Target->_GameObjectInfo.ObjectId, HitEffectType, AttackSkillInfo->SkillTargetEffectTime);
							G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResEffectPacket, this);
							ResEffectPacket->Free();

							// 스탯 변경 메세지 전송
							CMessage* ResChangeObjectStat = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(Target->_GameObjectInfo.ObjectId, Target->_GameObjectInfo.ObjectStatInfo);
							G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResChangeObjectStat, this);
							ResChangeObjectStat->Free();
						}
					}

					G_ObjectManager->GameServer->SkillCoolTimeTimerJobCreate(this, 500, FindSkillInfo, en_TimerJobType::TIMER_MELEE_ATTACK_END, QuickSlotBarIndex, QuickSlotBarSlotIndex);								
				}
				else
				{
					ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(_GameObjectInfo.ObjectId, en_SkillErrorType::ERROR_SKILL_COOLTIME, FindSkillInfo->SkillName.c_str());
					G_ObjectManager->GameServer->SendPacket(_SessionId, ResErrorPacket);
					ResErrorPacket->Free();					
				}				
			}
			break;
		case en_PlayerJobType::PLAYER_MAGIC_JOB:
			{
				int8 QuickSlotBarIndex;
				*PlayerJob->Message >> QuickSlotBarIndex;

				int8 QuickSlotBarSlotIndex;
				*PlayerJob->Message >> QuickSlotBarSlotIndex;

				// 공격한 방향
				int8 ReqMoveDir;
				*PlayerJob->Message >> ReqMoveDir;

				// 스킬 종류
				int16 ReqSkillType;
				*PlayerJob->Message >> ReqSkillType;

				CGameObject* FindGameObject = nullptr;
				float SpellCastingTime = 0.0f;

				CMessage* ResEffectPacket = nullptr;
				CMessage* ResMagicPacket = nullptr;

				FindSkillInfo = _SkillBox.FindSkill((en_SkillType)ReqSkillType);
				if (FindSkillInfo != nullptr && FindSkillInfo->CanSkillUse)
				{
					switch (FindSkillInfo->SkillType)
					{
					case en_SkillType::SKILL_KNIGHT_CHARGE_POSE:
						_SpellTick = GetTickCount64() + FindSkillInfo->SkillCastingTime;
						SpellCastingTime = FindSkillInfo->SkillCastingTime / 1000.0f;

						ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(_GameObjectInfo.ObjectId, en_EffectType::EFFECT_CHARGE_POSE, 2.8f);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResEffectPacket, this);
						ResEffectPacket->Free();
						break;
					case en_SkillType::SKILL_SHAMAN_FLAME_HARPOON:
					case en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE:
					case en_SkillType::SKILL_SHAMAN_HELL_FIRE:
					case en_SkillType::SKILL_SHAMAN_ROOT:
					case en_SkillType::SKILL_SHAMAN_ICE_CHAIN:
					case en_SkillType::SKILL_SHAMAN_ICE_WAVE:
					case en_SkillType::SKILL_TAIOIST_DIVINE_STRIKE:
					case en_SkillType::SKILL_TAIOIST_ROOT:
						if (_SelectTarget != nullptr)
						{
							_SpellTick = GetTickCount64() + FindSkillInfo->SkillCastingTime;
							SpellCastingTime = FindSkillInfo->SkillCastingTime / 1000.0f;

							int16 Distance = st_Vector2Int::Distance(_SelectTarget->GetCellPosition(), GetCellPosition());

							if (Distance <= 6)
							{
								FindGameObject = G_ObjectManager->Find(_SelectTarget->_GameObjectInfo.ObjectId,_SelectTarget->_GameObjectInfo.ObjectType);
								if (FindGameObject != nullptr)
								{
									Targets.push_back(FindGameObject);
								}

								// 스펠창 시작
								ResMagicPacket = G_ObjectManager->GameServer->MakePacketResMagic(_GameObjectInfo.ObjectId, true, FindSkillInfo->SkillType, SpellCastingTime);
								G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMagicPacket, this);
								ResMagicPacket->Free();

								_SkillType = FindSkillInfo->SkillType;
							}
							else
							{
								ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(_GameObjectInfo.ObjectId, en_SkillErrorType::ERROR_DISTANCE, FindSkillInfo->SkillName.c_str(), Distance);
								G_ObjectManager->GameServer->SendPacket(_SessionId, ResErrorPacket);
								ResErrorPacket->Free();
							}
						}
						else
						{
							ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(_GameObjectInfo.ObjectId, en_SkillErrorType::ERROR_NON_SELECT_OBJECT, FindSkillInfo->SkillName.c_str());
							G_ObjectManager->GameServer->SendPacket(_SessionId, ResErrorPacket);
							ResErrorPacket->Free();
						}
						break;
					case en_SkillType::SKILL_TAIOIST_HEALING_LIGHT:
						_SpellTick = GetTickCount64() + FindSkillInfo->SkillCastingTime;

						SpellCastingTime = FindSkillInfo->SkillCastingTime / 1000.0f;
						
						if (_SelectTarget != nullptr)
						{
							FindGameObject = G_ObjectManager->Find(_SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectType);
							if (FindGameObject != nullptr)
							{
								Targets.push_back(FindGameObject);
							}

							// 스펠창 시작
							ResMagicPacket = G_ObjectManager->GameServer->MakePacketResMagic(_SelectTarget->_GameObjectInfo.ObjectId, true, FindSkillInfo->SkillType, SpellCastingTime);
							G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMagicPacket, this);
							ResMagicPacket->Free();

							_SkillType = FindSkillInfo->SkillType;
						}
						else
						{
							Targets.push_back(this);

							ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(_GameObjectInfo.ObjectId, en_SkillErrorType::ERROR_HEAL_NON_SELECT_OBJECT, FindSkillInfo->SkillName.c_str());
							G_ObjectManager->GameServer->SendPacket(_SessionId, ResErrorPacket);
							ResErrorPacket->Free();
						}

						break;
					case en_SkillType::SKILL_TAIOIST_HEALING_WIND:
						break;
					case en_SkillType::SKILL_SHOCK_RELEASE:
						break;
					}

					if (Targets.size() >= 1)
					{
						_Target = Targets[0];

						_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPELL;

						// 마법 스킬 모션 출력
						CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId, 
							_GameObjectInfo.ObjectPositionInfo.MoveDir, _GameObjectInfo.ObjectType,
							_GameObjectInfo.ObjectPositionInfo.State);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStateChangePacket, this);
						ResObjectStateChangePacket->Free();

						G_ObjectManager->GameServer->SkillCoolTimeTimerJobCreate(this, FindSkillInfo->SkillCastingTime, FindSkillInfo, en_TimerJobType::TIMER_SPELL_END, QuickSlotBarIndex, QuickSlotBarSlotIndex);
					}
				}
				else
				{
					if (FindSkillInfo == nullptr)
					{
						break;
					}
					
					ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(_GameObjectInfo.ObjectId, en_SkillErrorType::ERROR_SKILL_COOLTIME, FindSkillInfo->SkillName.c_str());
					G_ObjectManager->GameServer->SendPacket(_SessionId, ResErrorPacket);
					ResErrorPacket->Free();
				}
			}
			break;
		case en_PlayerJobType::PLAYER_MAGIC_CANCEL_JOB:
			if (_SkillJob != nullptr)
			{
				_SkillJob->TimerJobCancel = true;
				_SkillJob = nullptr;

				ResMagicCancelPacket = G_ObjectManager->GameServer->MakePacketMagicCancel(_AccountId, _GameObjectInfo.ObjectId);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMagicCancelPacket, this);
				ResMagicCancelPacket->Free();
			}
			break;
		}		

		if (PlayerJob->Message != nullptr)
		{
			PlayerJob->Message->Free();
		}		

		G_ObjectManager->GameServer->_PlayerJobMemoryPool->Free(PlayerJob);
	}
}

void CPlayer::UpdateMove()
{	
	switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
	{
	case en_MoveDir::UP:		
		_GameObjectInfo.ObjectPositionInfo.PositionY +=
			(st_Vector2::Up()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::DOWN:		
		_GameObjectInfo.ObjectPositionInfo.PositionY +=
			(st_Vector2::Down()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::LEFT:		
		_GameObjectInfo.ObjectPositionInfo.PositionX += 
			(st_Vector2::Left()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::RIGHT:		
		_GameObjectInfo.ObjectPositionInfo.PositionX += 
			(st_Vector2::Right()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
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
	}
	else
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

		PositionReset();

		// 바뀐 좌표 값 시야범위 오브젝트들에게 전송
		CMessage* ResMovePacket = G_ObjectManager->GameServer->MakePacketResMove(_AccountId,
			_GameObjectInfo.ObjectId,
			CanMove,
			_GameObjectInfo.ObjectPositionInfo);

		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMovePacket, this);
		ResMovePacket->Free();
	}		
}

void CPlayer::UpdateAttack()
{

}

void CPlayer::UpdateSpell()
{	

}