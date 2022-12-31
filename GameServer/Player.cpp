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
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_PLAYER;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
	_DefaultAttackTick = 0;

	_FieldOfViewDistance = 10;	

	_NatureRecoveryTick = GetTickCount64() + 5000;
	_FieldOfViewUpdateTick = GetTickCount64() + 50;

	_GameObjectInfo.ObjectWidth = 1;
	_GameObjectInfo.ObjectHeight = 1;

	_ComboSkill = nullptr;	

	_SpellSkill = nullptr;		

	_RectCollision = new CRectCollision(this);

	_OnPlayerDefaultAttack = false;
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

	if (_ComboSkill != nullptr)
	{
		bool ReturnComboSkill = _ComboSkill->Update();
		if (ReturnComboSkill)
		{
			G_ObjectManager->SkillReturn(_ComboSkill);
			_ComboSkill = nullptr;
		}
	}

	if (_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::READY_DEAD && _GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
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

				CMessage* ResObjectStatPacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId,
					_GameObjectInfo.ObjectStatInfo);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStatPacket);
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
	case en_CreatureState::READY_DEAD:
		break;
	case en_CreatureState::DEAD:
		break;
	}

	if (_OnPlayerDefaultAttack)
	{
		if (_SelectTarget != nullptr)
		{
			float Distance = st_Vector2::Distance(_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);
			if (Distance < 2.0f)
			{
				if (st_Vector2::CheckFieldOfView(_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.MoveDir, 90))
				{
					if (_DefaultAttackTick < GetTickCount64())
					{
						CSkill* DefaultAttackSkill = _SkillBox.FindSkill(en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC, en_SkillType::SKILL_DEFAULT_ATTACK);
						if (DefaultAttackSkill != nullptr)
						{
							_DefaultAttackTick = GetTickCount64() + _GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate;

							st_AttackSkillInfo* DefaultAttackSkillInfo = (st_AttackSkillInfo*)DefaultAttackSkill->GetSkillInfo();

							bool IsCritical = true;
							// 데미지 판단
							int32 Damage = CMath::CalculateMeleeDamage(&IsCritical,
								_SelectTarget->_GameObjectInfo.ObjectStatInfo.Defence,
								_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage + _Equipment._WeaponMinDamage + DefaultAttackSkillInfo->SkillMinDamage,
								_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage + _Equipment._WeaponMaxDamage + DefaultAttackSkillInfo->SkillMaxDamage,
								_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint);

							st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType, IsCritical, Damage, DefaultAttackSkill->GetSkillInfo()->SkillType);
							_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);

							// 애니메이션 출력
							vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldOfViewPlayers(this, 1, false);

							CMessage* AnimationPlayPacket = G_ObjectManager->GameServer->MakePacketResAnimationPlay(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.MoveDir,
								(*DefaultAttackSkill->GetSkillInfo()->SkillAnimations.find(_GameObjectInfo.ObjectPositionInfo.MoveDir)).second);
							G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, AnimationPlayPacket);
							AnimationPlayPacket->Free();

							DefaultAttackSkill->CoolTimeStart();

							// 쿨타임 표시 ( 퀵술롯 바에 등록되어 있는 같은 종류의 스킬을 모두 쿨타임 표시 시켜 준다 )
							for (auto QuickSlotBarPosition : _QuickSlotManager.FindQuickSlotBar(DefaultAttackSkill->GetSkillInfo()->SkillType))
							{
								// 클라에게 쿨타임 표시
								CMessage* ResCoolTimeStartPacket = G_ObjectManager->GameServer->MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
									QuickSlotBarPosition.QuickSlotBarSlotIndex,
									1.0f, DefaultAttackSkill);
								G_ObjectManager->GameServer->SendPacket(_SessionId, ResCoolTimeStartPacket);
								ResCoolTimeStartPacket->Free();
							}
						}
					}
				}
				else
				{
					CMessage* ResDefaultAttackAngleErrPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_ATTACK_ANGLE);
					G_ObjectManager->GameServer->SendPacket(_SessionId, ResDefaultAttackAngleErrPacket);
					ResDefaultAttackAngleErrPacket->Free();
				}					
			}
			else
			{
				_OnPlayerDefaultAttack = false;
			}
		}
	}
}

bool CPlayer::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	if (_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE
		|| _GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::RETURN_SPAWN_POSITION)
	{
		CGameObject::OnDamaged(Attacker, Damage);

		if (_GameObjectInfo.ObjectStatInfo.HP == 0)
		{
			_DeadReadyTick = GetTickCount64() + 1000;

			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::READY_DEAD;

			CMap* Map = G_MapManager->GetMap(1);

			vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = Map->GetFieldOfViewPlayers(this, 1, false);

			CGameServerMessage* ResDeadStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.MoveDir, _GameObjectInfo.ObjectType, _GameObjectInfo.ObjectPositionInfo.State);
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

	_GameObjectInfo.ObjectId = 0;
	_GameObjectInfo.ObjectName = L"";

	_NetworkState = en_ObjectNetworkState::READY;
}

void CPlayer::PositionReset()
{	
	switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
	{
	case en_MoveDir::UP:
		_GameObjectInfo.ObjectPositionInfo.Position._Y +=
			(st_Vector2::Down()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::DOWN:
		_GameObjectInfo.ObjectPositionInfo.Position._Y +=
			(st_Vector2::Up()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::LEFT:
		_GameObjectInfo.ObjectPositionInfo.Position._X +=
			(st_Vector2::Right()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::RIGHT:
		_GameObjectInfo.ObjectPositionInfo.Position._X +=
			(st_Vector2::Left()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	}

	_RectCollision->CollisionUpdate();

	//G_Logger->WriteStdOut(en_Color::RED, L"PX : %0.2f PY : %0.2f CX : %0.2f CY : 0.2f\n", PreviousXYPosition._X, PreviousXYPosition._Y, _GameObjectInfo.ObjectPositionInfo.Position._X, _GameObjectInfo.ObjectPositionInfo.Position._Y);
}

bool CPlayer::UpdateSpawnIdle()
{
	bool ChangeToIdle = CGameObject::UpdateSpawnIdle();

	if (ChangeToIdle)
	{
		CGameServerMessage* ChangeToIdlePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.MoveDir,
			_GameObjectInfo.ObjectType,
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
	case en_MoveDir::LEFT_UP:
		_GameObjectInfo.ObjectPositionInfo.Position._X +=
			(st_Vector2::Left()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		_GameObjectInfo.ObjectPositionInfo.Position._Y +=
			(st_Vector2::Up()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::LEFT_DOWN:
		_GameObjectInfo.ObjectPositionInfo.Position._X +=
			(st_Vector2::Left()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		_GameObjectInfo.ObjectPositionInfo.Position._Y +=
			(st_Vector2::Down()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::RIGHT_UP:
		_GameObjectInfo.ObjectPositionInfo.Position._X +=
			(st_Vector2::Right()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		_GameObjectInfo.ObjectPositionInfo.Position._Y +=
			(st_Vector2::Up()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::RIGHT_DOWN:
		_GameObjectInfo.ObjectPositionInfo.Position._X +=
			(st_Vector2::Right()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		_GameObjectInfo.ObjectPositionInfo.Position._Y +=
			(st_Vector2::Down()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	}	

	_RectCollision->CollisionUpdate();

	bool CanMove = _Channel->GetMap()->Cango(this, _GameObjectInfo.ObjectPositionInfo.Position._X, _GameObjectInfo.ObjectPositionInfo.Position._Y);
	if (CanMove == true)
	{
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

		PositionReset();

		// 바뀐 좌표 값 시야범위 오브젝트들에게 전송
		CMessage* ResMovePacket = G_ObjectManager->GameServer->MakePacketResMove(
			_GameObjectInfo.ObjectId,
			CanMove,
			_GameObjectInfo.ObjectPositionInfo);		
		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMovePacket);
		ResMovePacket->Free();
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

		CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.MoveDir,
			_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.State);
		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStateChangePacket);
		ResObjectStateChangePacket->Free();

		if (_SpellSkill != nullptr && _SelectTarget != nullptr)
		{			
			en_EffectType HitEffectType = en_EffectType::EFFECT_TYPE_NONE;

			// 크리티컬 판단
			random_device Seed;
			default_random_engine Eng(Seed());

			float CriticalPoint = _GameObjectInfo.ObjectStatInfo.MagicCriticalPoint / 1000.0f;
			bernoulli_distribution CriticalCheck(CriticalPoint);
			bool IsCritical = CriticalCheck(Eng);

			int32 FinalDamage = 0;

			mt19937 Gen(Seed());	

			bool TargetIsDead = false;

			switch (_SpellSkill->GetSkillInfo()->SkillType)
			{
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON:
			{
				HitEffectType = en_EffectType::EFFECT_FLAME_HARPOON_TARGET;

				int32 MagicDamage = (int32)(_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6);

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_SpellSkill->GetSkillInfo();

				uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
				int32 ChoiceDamage = DamageChoiceRandom(Gen);
				FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;				

				st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType, IsCritical, FinalDamage, _SpellSkill->GetSkillInfo()->SkillType);
				_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);				
			}
			break;			
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
			{				
				HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;

				int32 MagicDamage = (int32)(_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6);

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_SpellSkill->GetSkillInfo();

				uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
				int32 ChoiceDamage = DamageChoiceRandom(Gen);
				FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

				st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType, IsCritical, FinalDamage, _SpellSkill->GetSkillInfo()->SkillType);
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
				
				float DebufMovingSpeed = _SelectTarget->_GameObjectInfo.ObjectStatInfo.MaxSpeed * AttackSkillInfo->SkillDebufMovingSpeed * 0.01;
				_SelectTarget->_GameObjectInfo.ObjectStatInfo.Speed -= DebufMovingSpeed;

				CMessage* ResObjectStatChange = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId,
					_GameObjectInfo.ObjectStatInfo);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStatChange);
				ResObjectStatChange->Free();

				bool IsShamanIceChain = _StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_CHAIN;
				if (IsShamanIceChain == false)
				{
					CSkill* NewSkill = G_ObjectManager->SkillCreate();

					st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(_SpellSkill->GetSkillInfo()->SkillType, _SpellSkill->GetSkillInfo()->SkillLevel);					
					NewSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
					NewSkill->StatusAbnormalDurationTimeStart();

					_SelectTarget->AddDebuf(NewSkill);
					_SelectTarget->SetStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_CHAIN);

					CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
						_SelectTarget->_GameObjectInfo.ObjectType,
						_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
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
				HitEffectType = en_EffectType::EFFECT_LIGHTNING;

				int32 MagicDamage = _GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6;

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_SpellSkill->GetSkillInfo();

				uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
				int32 ChoiceDamage = DamageChoiceRandom(Gen);
				FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

				st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType, IsCritical, FinalDamage, _SpellSkill->GetSkillInfo()->SkillType);
				_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);

				bool IsLightningStrike = _StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE;
				if (IsLightningStrike == false)
				{
					CSkill* NewSkill = G_ObjectManager->SkillCreate();

					st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(_SpellSkill->GetSkillInfo()->SkillType, _SpellSkill->GetSkillInfo()->SkillLevel);
					NewSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
					NewSkill->StatusAbnormalDurationTimeStart();

					_SelectTarget->AddDebuf(NewSkill);
					_SelectTarget->SetStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE);

					CMessage* SelectTargetMoveStopMessage = G_ObjectManager->GameServer->MakePacketResMoveStop(_SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectPositionInfo);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, SelectTargetMoveStopMessage);
					SelectTargetMoveStopMessage->Free();

					CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
						_SelectTarget->_GameObjectInfo.ObjectType,
						_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
						_SpellSkill->GetSkillInfo()->SkillType,
						true, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResStatusAbnormalPacket);
					ResStatusAbnormalPacket->Free();

					CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
					G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResBufDeBufSkillPacket);
					ResBufDeBufSkillPacket->Free();
				}			

				float EffectPrintTime = _SpellSkill->GetSkillInfo()->SkillDurationTime / 1000.0f;

				// 이펙트 출력
				CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(_SelectTarget->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_DEBUF_STUN, EffectPrintTime);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResEffectPacket);
				ResEffectPacket->Free();
			}
			break;
			case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:
			{
				HitEffectType = en_EffectType::EFFECT_FLAME_HARPOON_TARGET;

				int32 MagicDamage = _GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6;

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_SpellSkill->GetSkillInfo();

				uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
				int32 ChoiceDamage = DamageChoiceRandom(Gen);
				FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;				

				st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType, IsCritical, FinalDamage, _SpellSkill->GetSkillInfo()->SkillType);
				_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);				
			}
			break;
			case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:
			{
				HitEffectType = en_EffectType::EFFECT_FLAME_HARPOON_TARGET;

				int32 MagicDamage = (int32)(_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6);

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_SpellSkill->GetSkillInfo();

				uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
				int32 ChoiceDamage = DamageChoiceRandom(Gen);
				FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;				

				st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType, IsCritical, FinalDamage, _SpellSkill->GetSkillInfo()->SkillType);
				_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);				
			}
			break;			
			case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT:
			{
				HitEffectType = en_EffectType::EFFECT_HEALING_LIGHT_TARGET;

				st_HealSkillInfo* HealSkillInfo = (st_HealSkillInfo*)_SpellSkill->GetSkillInfo();

				uniform_int_distribution<int> HealChoiceRandom(HealSkillInfo->SkillMinHealPoint, HealSkillInfo->SkillMaxHealPoint);
				FinalDamage = HealChoiceRandom(Gen);

				st_GameObjectJob* HealJob = G_ObjectManager->GameServer->MakeGameObjectJobHPHeal(this, false, FinalDamage, _SpellSkill->GetSkillInfo()->SkillType);
				_SelectTarget->_GameObjectJobQue.Enqueue(HealJob);								
			}
			break;
			case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:
			{
				HitEffectType = en_EffectType::EFFECT_HEALING_WIND_TARGET;

				st_HealSkillInfo* HealSkillInfo = (st_HealSkillInfo*)_SpellSkill->GetSkillInfo();

				uniform_int_distribution<int> HealChoiceRandom(HealSkillInfo->SkillMinHealPoint, HealSkillInfo->SkillMaxHealPoint);
				FinalDamage = HealChoiceRandom(Gen);

				st_GameObjectJob* HealJob = G_ObjectManager->GameServer->MakeGameObjectJobHPHeal(this, false, FinalDamage, _SpellSkill->GetSkillInfo()->SkillType);
				_SelectTarget->_GameObjectJobQue.Enqueue(HealJob);				
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

			// 이펙트 출력
			CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(_SelectTarget->_GameObjectInfo.ObjectId, HitEffectType, _SpellSkill->GetSkillInfo()->SkillTargetEffectTime);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResEffectPacket);
			ResEffectPacket->Free();					

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
			_GameObjectInfo.ObjectPositionInfo.MoveDir,
			_GameObjectInfo.ObjectType,
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

void CPlayer::UpdateReadyDead()
{

}

void CPlayer::UpdateDead()
{

}

void CPlayer::CheckFieldOfViewObject()
{
	// 시야범위 객체 조사
	if (_FieldOfViewUpdateTick < GetTickCount64() && _Channel != nullptr)
	{
		// 시야범위 오브젝트를 조사해서 저장
		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldOfViewObjects(this, 1, false);
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
				if (SpawnObject.ObjectID != 0 && SpawnObject.ObjectType != en_GameObjectType::NORMAL)
				{
					CGameObject* FindObject = _Channel->FindChannelObject(SpawnObject.ObjectID, SpawnObject.ObjectType);
					if (FindObject != nullptr)
					{
						SpawnObjectInfos.push_back(FindObject);
					}
				}
			}

			if (SpawnObjectInfos.size() > 0)
			{
				// 스폰해야 할 대상들을 나에게 스폰하라고 알림
				CMessage* ResOtherObjectSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn((int32)SpawnObjectInfos.size(), SpawnObjectInfos);
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResOtherObjectSpawnPacket);
				ResOtherObjectSpawnPacket->Free();
			}
		}

		// 한번 더 검사
		if (DeSpawnObjectIds.size() > 0)
		{
			vector<CGameObject*> DeSpawnObjectInfos;
			for (st_FieldOfViewInfo DeSpawnObject : DeSpawnObjectIds)
			{
				if (DeSpawnObject.ObjectID != 0 && DeSpawnObject.ObjectType != en_GameObjectType::NORMAL)
				{
					CGameObject* FindObject = _Channel->FindChannelObject(DeSpawnObject.ObjectID, DeSpawnObject.ObjectType);

					// 추가적으로 검사
					// 소환해제 해야할 대상이 죽음 준비 상태 또는 죽음 상태일 경우에는 알아서 소환해제 되기 때문에
					// 죽음 준비 상태 그리고 죽음 상태가 아닐 경우에만 소환해제 하도록 설정					
					if (FindObject != nullptr
						&& FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::READY_DEAD
						&& FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
					{
						if (_SelectTarget != nullptr && FindObject->_GameObjectInfo.ObjectId == _SelectTarget->_GameObjectInfo.ObjectId)
						{
							_SelectTarget = nullptr;
						}

						DeSpawnObjectInfos.push_back(FindObject);
					}
				}
			}

			// 소환해제해야 할 대상을 나에게 스폰 해제하라고 알림
			if (DeSpawnObjectInfos.size() > 0)
			{
				CMessage* ResOtherObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn((int32)DeSpawnObjectInfos.size(), DeSpawnObjectInfos);
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResOtherObjectDeSpawnPacket);
				ResOtherObjectDeSpawnPacket->Free();
			}
		}

		_FieldOfViewInfos = CurrentFieldOfViewObjectIds;
	}
}