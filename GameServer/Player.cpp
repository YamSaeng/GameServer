#include "pch.h"
#include "Player.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "Skill.h"
#include "SkillBox.h"
#include "MapManager.h"
#include "RectCollision.h"

CPlayer::CPlayer()
{
	_AccountId = 0;
	_SessionId = 0;
		
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
	_DefaultAttackTick = 0;

	_FieldOfDirection = st_Vector2::Right();

	_FieldOfViewDistance = 15;		

	_FieldOfAngle = 210;

	_NatureRecoveryTick = GetTickCount64() + 5000;
	_FieldOfViewUpdateTick = GetTickCount64() + 50;

	_GameObjectInfo.ObjectWidth = 1;
	_GameObjectInfo.ObjectHeight = 1;

	_ComboSkill = nullptr;	

	_SpellSkill = nullptr;			

	_SkillBox.SetOwner(this);
}

CPlayer::~CPlayer()
{
	
}

void CPlayer::Update()
{
	CGameObject::Update();

	if (_NetworkState == en_ObjectNetworkState::LEAVE)
	{
		return;
	}	

	// 주위 시야 오브젝트 점검
	CheckFieldOfViewObject();	

	// 스킬목록 업데이트
	_SkillBox.Update();
	
	// 버프, 디버프 업데이트
	CheckBufDeBufSkill();

	if (_RectCollision != nullptr)
	{
		_RectCollision->Update();
	}

	if (_ComboSkill != nullptr)
	{
		bool ReturnComboSkill = _ComboSkill->Update();
		if (ReturnComboSkill)
		{
			G_ObjectManager->SkillReturn(_ComboSkill);
			_ComboSkill = nullptr;
		}
	}

	if (_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
	{
		if (_NatureRecoveryTick < GetTickCount64())
		{
			if (_GameObjectInfo.ObjectStatInfo.HP != _GameObjectInfo.ObjectStatInfo.MaxHP
				|| _GameObjectInfo.ObjectStatInfo.MP != _GameObjectInfo.ObjectStatInfo.MaxMP)
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

				if (_GameObjectInfo.ObjectStatInfo.MP > _GameObjectInfo.ObjectStatInfo.MaxMP)
				{
					_GameObjectInfo.ObjectStatInfo.MP = _GameObjectInfo.ObjectStatInfo.MaxMP;
				}

				_NatureRecoveryTick = GetTickCount64() + 5000;

				vector<st_FieldOfViewInfo> AroundPlayers = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

				CMessage* ResObjectStatPacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId,
					_GameObjectInfo.ObjectStatInfo);
				G_ObjectManager->GameServer->SendPacketFieldOfView(AroundPlayers, ResObjectStatPacket);
				ResObjectStatPacket->Free();
			}						
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
	case en_CreatureState::MOVING:
		UpdateMoving();		
		break;	
	case en_CreatureState::ATTACK:
		UpdateAttack();
		break;
	case en_CreatureState::SPELL:
		UpdateSpell();
		break;
	case en_CreatureState::GATHERING:
		UpdateGathering();
		break;	
	case en_CreatureState::DEAD:
		UpdateDead();
		break;
	}	

	/*G_Logger->WriteStdOut(en_Color::RED, L"Dir X %0.1f Y %0.1f PositionX %0.1f PositionY %0.1f\n", _GameObjectInfo.ObjectPositionInfo.Direction._X,
		_GameObjectInfo.ObjectPositionInfo.Direction._Y, _GameObjectInfo.ObjectPositionInfo.Position._X, _GameObjectInfo.ObjectPositionInfo.Position._Y);*/	
}

bool CPlayer::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	if (_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE
		|| _GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::RETURN_SPAWN_POSITION)
	{
		CGameObject::OnDamaged(Attacker, Damage);

		if (_GameObjectInfo.ObjectStatInfo.HP == 0)
		{
			_RectCollision->SetActive(false);

			_DeadTick = GetTickCount64() + 1000;

			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

			_GameObjectInfo.ObjectPositionInfo.MoveDirection = st_Vector2::Zero();			

			vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

			CGameServerMessage* ResDeadStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId, 
				_GameObjectInfo.ObjectPositionInfo.State);
			G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResDeadStateChangePacket);
			ResDeadStateChangePacket->Free();

			return true;
		}
	}

	return false;
}

void CPlayer::Start()
{
	
}

void CPlayer::End()
{
	CGameObject::End();

	_FieldOfViewInfos.clear();

	// 스킬 목록 정리
	_SkillBox.Empty();
	// 퀵슬롯 정리
	_QuickSlotManager.Empty();

	// 남아 있는 플레이어 잡 큐 처리
	CGameObject::Update();
	
	_GameObjectInfo.ObjectName = L"";	
}

bool CPlayer::UpdateSpawnIdle()
{
	bool ChangeToIdle = CGameObject::UpdateSpawnIdle();

	if (ChangeToIdle)
	{
		CGameServerMessage* ChangeToIdlePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,						
			_GameObjectInfo.ObjectPositionInfo.State);
		G_ObjectManager->GameServer->SendPacketFieldOfView(this, ChangeToIdlePacket);
		ChangeToIdlePacket->Free();
	}

	return ChangeToIdle;	
}

void CPlayer::UpdateIdle()
{

}

void CPlayer::UpdateMoving()
{	
	st_Vector2 NextPosition;

	bool CanMove = _Channel->GetMap()->Cango(this, &NextPosition);

	/*G_Logger->WriteStdOut(en_Color::BLUE, L"Dir X : %0.1f Y : %0.1f \n",
		_GameObjectInfo.ObjectPositionInfo.Direction._X,
		_GameObjectInfo.ObjectPositionInfo.Direction._Y);*/	

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
	}
	else
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;		

		vector<st_FieldOfViewInfo> AroundPlayers = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

		CMessage* ResMoveStopPacket = G_ObjectManager->GameServer->MakePacketResMoveStop(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.Position._X,
			_GameObjectInfo.ObjectPositionInfo.Position._Y);			
		G_ObjectManager->GameServer->SendPacketFieldOfView(AroundPlayers, ResMoveStopPacket);
		ResMoveStopPacket->Free();
	}
}

void CPlayer::UpdateAttack()
{
	
}

void CPlayer::UpdateSpell()
{
	if (_SpellTick < GetTickCount64())
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

		vector<st_FieldOfViewInfo> AroundPlayers = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

		CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,			
			_GameObjectInfo.ObjectPositionInfo.State);
		G_ObjectManager->GameServer->SendPacketFieldOfView(AroundPlayers, ResObjectStateChangePacket);
		ResObjectStateChangePacket->Free();

		if (_SpellSkill != nullptr && _SelectTarget != nullptr)
		{
			st_SkillInfo* AttackSkillInfo = _SpellSkill->GetSkillInfo();

			switch (_SpellSkill->GetSkillInfo()->SkillType)
			{
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON:
				{
					st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType,
						AttackSkillInfo->SkillType,
						AttackSkillInfo->SkillMinDamage,
						AttackSkillInfo->SkillMaxDamage);
					_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);
				}
				break;			
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
				{	
					st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType,
						AttackSkillInfo->SkillType,
						AttackSkillInfo->SkillMinDamage,
						AttackSkillInfo->SkillMaxDamage);
					_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);

					if (AttackSkillInfo->NextComboSkill != en_SkillType::SKILL_TYPE_NONE)
					{
						CSkill* FindNextComboSkill = _SkillBox.FindSkill(AttackSkillInfo->SkillCharacteristic, AttackSkillInfo->NextComboSkill);
						if (FindNextComboSkill->GetSkillInfo()->CanSkillUse == true)
						{
							st_GameObjectJob* ComboAttackCreateJob = G_ObjectManager->GameServer->MakeGameObjectJobComboSkillCreate(_SpellSkill); G_ObjectManager->GameObjectJobCreate();
							_GameObjectJobQue.Enqueue(ComboAttackCreateJob);
						}					
					}									
					
					float DebufMovingSpeed = _SelectTarget->_GameObjectInfo.ObjectStatInfo.MaxSpeed * AttackSkillInfo->SkillDebufMovingSpeed * 0.01f;
					_SelectTarget->_GameObjectInfo.ObjectStatInfo.Speed -= DebufMovingSpeed;

					CMessage* ResObjectStatChange = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectStatInfo);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStatChange);
					ResObjectStatChange->Free();

					bool IsShamanIceChain = _StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_CHAIN;
					if (IsShamanIceChain == false)
					{
						CSkill* NewSkill = G_ObjectManager->SkillCreate();

						st_SkillInfo* NewAttackSkillInfo = G_ObjectManager->SkillInfoCreate(_SpellSkill->GetSkillInfo()->SkillType, _SpellSkill->GetSkillInfo()->SkillLevel);
						NewSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
						NewSkill->StatusAbnormalDurationTimeStart();

						_SelectTarget->AddDebuf(NewSkill);
						_SelectTarget->SetStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_CHAIN);

						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
							_SelectTarget->_GameObjectInfo.ObjectType,							
							_SpellSkill->GetSkillInfo()->SkillType, true, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_CHAIN);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResStatusAbnormalPacket);
						ResStatusAbnormalPacket->Free();

						CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
						G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResBufDeBufSkillPacket);
						ResBufDeBufSkillPacket->Free();
					}				
				}
				break;
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
				{
					st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType,
						AttackSkillInfo->SkillType,
						AttackSkillInfo->SkillMinDamage,
						AttackSkillInfo->SkillMaxDamage);
					_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);

					bool IsLightningStrike = _StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE;
					if (IsLightningStrike == false)
					{
						CSkill* NewSkill = G_ObjectManager->SkillCreate();

						st_SkillInfo* NewAttackSkillInfo = G_ObjectManager->SkillInfoCreate(_SpellSkill->GetSkillInfo()->SkillType, _SpellSkill->GetSkillInfo()->SkillLevel);
						NewSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
						NewSkill->StatusAbnormalDurationTimeStart();

						_SelectTarget->AddDebuf(NewSkill);
						_SelectTarget->SetStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE);

						CMessage* SelectTargetMoveStopMessage = G_ObjectManager->GameServer->MakePacketResMoveStop(_SelectTarget->_GameObjectInfo.ObjectId,
							_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._X,
							_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._Y);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, SelectTargetMoveStopMessage);
						SelectTargetMoveStopMessage->Free();

						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
							_SelectTarget->_GameObjectInfo.ObjectType,							
							_SpellSkill->GetSkillInfo()->SkillType,
							true, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResStatusAbnormalPacket);
						ResStatusAbnormalPacket->Free();

						CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
						G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResBufDeBufSkillPacket);
						ResBufDeBufSkillPacket->Free();
					}			

					float EffectPrintTime = _SpellSkill->GetSkillInfo()->SkillDurationTime / 1000.0f;
				}
				break;
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:
				{
					st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType,
						AttackSkillInfo->SkillType,
						AttackSkillInfo->SkillMinDamage,
						AttackSkillInfo->SkillMaxDamage);
					_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);				
				}
				break;
			case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:
				{
					st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType, 
						AttackSkillInfo->SkillType,
						AttackSkillInfo->SkillMinDamage,
						AttackSkillInfo->SkillMaxDamage);
					_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);				
				}
				break;			
			case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT:
				{										
					/*st_GameObjectJob* HealJob = G_ObjectManager->GameServer->MakeGameObjectJobHPHeal(this, false, FinalDamage, _SpellSkill->GetSkillInfo()->SkillType);
					_SelectTarget->_GameObjectJobQue.Enqueue(HealJob);	*/							
				}
				break;
			case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:
				{
					/*st_GameObjectJob* HealJob = G_ObjectManager->GameServer->MakeGameObjectJobHPHeal(this, false, FinalDamage, _SpellSkill->GetSkillInfo()->SkillType);
					_SelectTarget->_GameObjectJobQue.Enqueue(HealJob);*/
				}
				break;
			}					

			_SpellSkill->CoolTimeStart();

			for (auto QuickSlotBarPosition : _QuickSlotManager.FindQuickSlotBar(_SpellSkill->GetSkillInfo()->SkillType))
			{
				// 클라에게 쿨타임 표시
				CMessage* ResCoolTimeStartPacket = G_ObjectManager->GameServer->MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
					QuickSlotBarPosition.QuickSlotBarSlotIndex,
					1.0f, _SpellSkill);
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResCoolTimeStartPacket);
				ResCoolTimeStartPacket->Free();
			}

			vector<CSkill*> GlobalSkills = _SkillBox.GetGlobalSkills(_SpellSkill->GetSkillInfo()->SkillType, _SpellSkill->GetSkillKind());
			
			// 전역 쿨타임 적용
			for (CSkill* GlobalSkill : GlobalSkills)
			{
				GlobalSkill->GlobalCoolTimeStart(_SpellSkill->GetSkillInfo()->SkillMotionTime);

				for (st_Vector2Int QuickSlotPosition : GlobalSkill->_QuickSlotBarPosition)
				{
					CMessage* ResCoolTimeStartPacket = G_ObjectManager->GameServer->MakePacketCoolTime((int8)QuickSlotPosition._Y,
						(int8)QuickSlotPosition._X,
						1.0f, nullptr, _SpellSkill->GetSkillInfo()->SkillMotionTime);
					G_ObjectManager->GameServer->SendPacket(_SessionId, ResCoolTimeStartPacket);
					ResCoolTimeStartPacket->Free();
				}
			}
			
			// 스펠창 끝
			CMessage* ResMagicPacket = G_ObjectManager->GameServer->MakePacketResMagic(_GameObjectInfo.ObjectId, false);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMagicPacket);
			ResMagicPacket->Free();			
		}
	}
}

void CPlayer::UpdateGathering()
{
	if (_GatheringTick < GetTickCount64())
	{
		if (_GatheringTarget == nullptr)
		{
			CRASH("채집할 대상이 없는데 채집 요청함");
			return;
		}		

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

		_GatheringTarget->OnDamaged(this, 1);		

		CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,						
			_GameObjectInfo.ObjectPositionInfo.State);
		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStateChangePacket);

		CMessage* ResObjectStatChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GatheringTarget->_GameObjectInfo.ObjectId,
			_GatheringTarget->_GameObjectInfo.ObjectStatInfo);
		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStatChangePacket);
		ResObjectStatChangePacket->Free();

		CMessage* ResGatheringDamagePacket = G_ObjectManager->GameServer->MakePacketResGatheringDamage(_GatheringTarget->_GameObjectInfo.ObjectId);
		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResGatheringDamagePacket);
		ResGatheringDamagePacket->Free();

		// 채집창 끝
		CMessage* ResGatheringPacket = G_ObjectManager->GameServer->MakePacketResGathering(_GameObjectInfo.ObjectId, false, L"");
		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResGatheringPacket);
		ResGatheringPacket->Free();		
	}
}

void CPlayer::UpdateDead()
{
	if (_DeadTick < GetTickCount64())
	{
		st_GameObjectJob* DeSpawnPlayerJob = G_ObjectManager->GameServer->MakeGameObjectJobObjectDeSpawnObjectChannel(this);
		_Channel->_ChannelJobQue.Enqueue(DeSpawnPlayerJob);
	}
}

void CPlayer::CheckFieldOfViewObject()
{
	// 시야범위 객체 조사
	if (_FieldOfViewUpdateTick < GetTickCount64() && _Channel != nullptr)
	{
		// 시야범위 오브젝트를 조사해서 저장		
		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldOfViewObjects(this);
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

		vector<CGameObject*> SpawnObjectInfos;
		vector<CGameObject*> DeSpawnObjectInfos;

		// 한번 더 검사
		if (SpawnObjectIds.size() > 0)
		{
			// 스폰 해야할 대상들을 스폰			
			for (st_FieldOfViewInfo SpawnObject : SpawnObjectIds)
			{
				if (SpawnObject.ObjectID != 0 && SpawnObject.ObjectType != en_GameObjectType::OBJECT_NON_TYPE)
				{
					CGameObject* FindObject = _Channel->FindChannelObject(SpawnObject.ObjectID, SpawnObject.ObjectType);
					if (FindObject != nullptr
						&& FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD
						&& FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_READY)
					{
						SpawnObjectInfos.push_back(FindObject);
					}
				}
			}			
		}

		// 한번 더 검사
		if (DeSpawnObjectIds.size() > 0)
		{
			for (st_FieldOfViewInfo DeSpawnObject : DeSpawnObjectIds)
			{
				if (DeSpawnObject.ObjectID != 0 && DeSpawnObject.ObjectType != en_GameObjectType::OBJECT_NON_TYPE)
				{
					CGameObject* FindObject = _Channel->FindChannelObject(DeSpawnObject.ObjectID, DeSpawnObject.ObjectType);

					// 추가적으로 검사
					// 소환해제 해야할 대상이 죽음 준비 상태 또는 죽음 상태일 경우에는 알아서 소환해제 되기 때문에
					// 죽음 준비 상태 그리고 죽음 상태가 아닐 경우에만 소환해제 하도록 설정					
					
					// 하지만 플레이어의 시야를 대상이 벗어난다면 바로 디스폰 해줘야함
					if (FindObject != nullptr						
						&& FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD
						&& FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_READY)
					{
						if (_SelectTarget != nullptr && FindObject->_GameObjectInfo.ObjectId == _SelectTarget->_GameObjectInfo.ObjectId)
						{
							_SelectTarget = nullptr;
						}

						DeSpawnObjectInfos.push_back(FindObject);
					}
				}
			}			
		}

		_FieldOfViewInfos = CurrentFieldOfViewObjectIds;

		_FieldOfViewObjects.clear();

		for (st_FieldOfViewInfo FieldOfViewInfo : _FieldOfViewInfos)
		{
			CGameObject* FindObject = _Channel->FindChannelObject(FieldOfViewInfo.ObjectID, FieldOfViewInfo.ObjectType);
			if (FindObject != nullptr
				&& (FindObject->_GameObjectInfo.ObjectType == en_GameObjectType::OBJECT_PLAYER
					|| FindObject->_GameObjectInfo.ObjectType == en_GameObjectType::OBJECT_GOBLIN))
			{
				_FieldOfViewObjects.push_back(FindObject);
			}
		}

		RayCastingToFieldOfViewObjects(&SpawnObjectInfos, &DeSpawnObjectInfos);

		if (SpawnObjectInfos.size() > 0)
		{
			// 스폰해야 할 대상들을 나에게 스폰하라고 알림
			CMessage* ResOtherObjectSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn((int32)SpawnObjectInfos.size(), SpawnObjectInfos);
			G_ObjectManager->GameServer->SendPacket(_SessionId, ResOtherObjectSpawnPacket);
			ResOtherObjectSpawnPacket->Free();
		}

		// 소환해제해야 할 대상을 나에게 스폰 해제하라고 알림
		if (DeSpawnObjectInfos.size() > 0)
		{
			CMessage* ResOtherObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn((int32)DeSpawnObjectInfos.size(), DeSpawnObjectInfos);
			G_ObjectManager->GameServer->SendPacket(_SessionId, ResOtherObjectDeSpawnPacket);
			ResOtherObjectDeSpawnPacket->Free();
		}
	}	
}

void CPlayer::RayCastingToFieldOfViewObjects(vector<CGameObject*>* SpawnObjects, vector<CGameObject*>* DespawnObjects)
{
	for (CGameObject* FieldOfViewObject : _FieldOfViewObjects)
	{
		st_Vector2 FieldOfViewObjectDir = FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.Position - _GameObjectInfo.ObjectPositionInfo.Position;
		st_Vector2 FieldOfViewRay = FieldOfViewObjectDir.Normalize();

		// 레이캐스팅 검사할때 움직일 단위 x, y 값
		st_Vector2 RayUnitStepSize;
		RayUnitStepSize._X = sqrt(1 + (FieldOfViewRay._Y / FieldOfViewRay._X) * (FieldOfViewRay._Y / FieldOfViewRay._X));
		RayUnitStepSize._Y = sqrt(1 + (FieldOfViewRay._X / FieldOfViewRay._Y) * (FieldOfViewRay._X / FieldOfViewRay._Y));

		// 맵 좌표 위치 
		st_Vector2Int MapCheck;
		MapCheck._X = _GameObjectInfo.ObjectPositionInfo.Position._X;
		MapCheck._Y = _GameObjectInfo.ObjectPositionInfo.Position._Y;

		// 현재 위치에서 다음 위치의 Ray 길이
		st_Vector2 RayLength1D;

		// 탐색 방향
		st_Vector2Int Step;

		// 탐색 방향 정하고 다음 위치 Ray 길이 값 정하기
		if (FieldOfViewRay._X < 0)
		{
			Step._X = -1;
			RayLength1D._X = (_GameObjectInfo.ObjectPositionInfo.Position._X - float(MapCheck._X)) * RayUnitStepSize._X;
		}
		else
		{
			Step._X = 1;
			RayLength1D._X = (float(MapCheck._X + 1) - _GameObjectInfo.ObjectPositionInfo.Position._X) * RayUnitStepSize._X;
		}

		if (FieldOfViewRay._Y < 0)
		{
			Step._Y = -1;
			RayLength1D._Y = (_GameObjectInfo.ObjectPositionInfo.Position._Y - float(MapCheck._Y)) * RayUnitStepSize._Y;
		}
		else
		{
			Step._Y = 1;
			RayLength1D._Y = (float(MapCheck._Y + 1) - _GameObjectInfo.ObjectPositionInfo.Position._Y) * RayUnitStepSize._Y;
		}

		// 검사
		bool WallFound = false;
		bool TargetFound = false;
		float MaxDistance = _FieldOfViewDistance;
		float Distance = 0.0f;
		// 벽을 찾거나 목표물을 찾거나 최대 광선 거리에 도착하면 나옴
		while (!WallFound && !TargetFound && Distance < MaxDistance)
		{
			// 다음 타일 위치로 옮김
			if (RayLength1D._X < RayLength1D._Y)
			{
				MapCheck._X += Step._X;
				Distance = RayLength1D._X;
				RayLength1D._X += RayUnitStepSize._X;
			}
			else
			{
				MapCheck._Y += Step._Y;
				Distance = RayLength1D._Y;
				RayLength1D._Y += RayUnitStepSize._Y;
			}

			// 맵에서 계산한 타일 위치 조사
			CMap* Map = _Channel->GetMap();
			if (MapCheck._X >= 0 && MapCheck._X < Map->_Right && MapCheck._Y >= 0 && MapCheck._Y < Map->_Down)
			{
				int X = MapCheck._X - Map->_Left;
				int Y = Map->_Down - MapCheck._Y;

				CGameObject* GameObject = Map->_ObjectsInfos[Y][X];
				if (GameObject != nullptr)
				{
					if (GameObject->_GameObjectInfo.ObjectId == FieldOfViewObject->_GameObjectInfo.ObjectId)
					{
						TargetFound = true;
					}
					else
					{
						switch (GameObject->_GameObjectInfo.ObjectType)
						{
						case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
							WallFound = true;
							break;
						}
					}
				}
			}
		}

		// 목표물을 먼저 찾은 경우
		if (TargetFound == true)
		{
			if (FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD
				&& FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_READY)
			{
				SpawnObjects->push_back(FieldOfViewObject);
			}			
		}

		// 벽과 같은 시야에 가려지는 물체를 먼저 찾은 경우
		if (WallFound == true)
		{
			DespawnObjects->push_back(FieldOfViewObject);
		}		
	}	
}