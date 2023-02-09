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

	// ���� �þ� ������Ʈ ����
	CheckFieldOfViewObject();

	// ��ų��� ������Ʈ
	_SkillBox.Update();
	
	// ����, ����� ������Ʈ
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

	/*G_Logger->WriteStdOut(en_Color::RED, L"Dir X %0.1f Y %0.1f PositionX %0.1f PositionY %0.1f\n", _GameObjectInfo.ObjectPositionInfo.Direction._X,
		_GameObjectInfo.ObjectPositionInfo.Direction._Y, _GameObjectInfo.ObjectPositionInfo.Position._X, _GameObjectInfo.ObjectPositionInfo.Position._Y);*/

	if (_OnPlayerDefaultAttack)
	{
		if (_SelectTarget != nullptr)
		{
			float Distance = st_Vector2::Distance(_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position);
			if (Distance < 2.0f)
			{
				if (st_Vector2::CheckFieldOfView(_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Position, _GameObjectInfo.ObjectPositionInfo.Direction, 90))
				{
					if (_DefaultAttackTick < GetTickCount64())
					{
						CSkill* DefaultAttackSkill = _SkillBox.FindSkill(en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC, en_SkillType::SKILL_DEFAULT_ATTACK);
						if (DefaultAttackSkill != nullptr)
						{
							_DefaultAttackTick = GetTickCount64() + _GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate;

							st_AttackSkillInfo* DefaultAttackSkillInfo = (st_AttackSkillInfo*)DefaultAttackSkill->GetSkillInfo();

							bool IsCritical = true;
							// ������ �Ǵ�
							int32 Damage = CMath::CalculateMeleeDamage(&IsCritical,
								_SelectTarget->_GameObjectInfo.ObjectStatInfo.Defence,
								_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage + _Equipment._WeaponMinDamage + DefaultAttackSkillInfo->SkillMinDamage,
								_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage + _Equipment._WeaponMaxDamage + DefaultAttackSkillInfo->SkillMaxDamage,
								_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint);

							st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType, IsCritical, Damage, DefaultAttackSkill->GetSkillInfo()->SkillType);
							_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);

							// ���� �ִϸ��̼� ���

							DefaultAttackSkill->CoolTimeStart();

							// ��Ÿ�� ǥ�� ( ������ �ٿ� ��ϵǾ� �ִ� ���� ������ ��ų�� ��� ��Ÿ�� ǥ�� ���� �ش� )
							for (auto QuickSlotBarPosition : _QuickSlotManager.FindQuickSlotBar(DefaultAttackSkill->GetSkillInfo()->SkillType))
							{
								// Ŭ�󿡰� ��Ÿ�� ǥ��
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
					CMessage* ResDefaultAttackAngleErrPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_GlobalMessageType::PERSONAL_MESSAGE_ATTACK_ANGLE);
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

			CGameServerMessage* ResDeadStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId, 
				_GameObjectInfo.ObjectType, _GameObjectInfo.ObjectPositionInfo.State);
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

	// ��ų ��� ����
	_SkillBox.Empty();
	// ������ ����
	_QuickSlotManager.Empty();

	// ���� �ִ� �÷��̾� �� ť ó��
	CGameObject::Update();

	_GameObjectInfo.ObjectId = 0;
	_GameObjectInfo.ObjectName = L"";	
}

bool CPlayer::UpdateSpawnIdle()
{
	bool ChangeToIdle = CGameObject::UpdateSpawnIdle();

	if (ChangeToIdle)
	{
		CGameServerMessage* ChangeToIdlePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,			
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
	st_Vector2 NextPosition;

	bool CanMove = _Channel->GetMap()->Cango(this, &NextPosition);

	/*G_Logger->WriteStdOut(en_Color::BLUE, L"Dir X : %0.1f Y : %0.1f \n",
		_GameObjectInfo.ObjectPositionInfo.Direction._X,
		_GameObjectInfo.ObjectPositionInfo.Direction._Y);*/	

	if (CanMove == true)
	{
		_GameObjectInfo.ObjectPositionInfo.Position = NextPosition;

		_RectCollision->CollisionUpdate();

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

		CMessage* ResMoveStopPacket = G_ObjectManager->GameServer->MakePacketResMoveStop(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.Position._X,
			_GameObjectInfo.ObjectPositionInfo.Position._Y);			
		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMoveStopPacket);
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

		CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.State);
		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStateChangePacket);
		ResObjectStateChangePacket->Free();

		if (_SpellSkill != nullptr && _SelectTarget != nullptr)
		{
			// ũ��Ƽ�� �Ǵ�
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

						st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(_SpellSkill->GetSkillInfo()->SkillType, _SpellSkill->GetSkillInfo()->SkillLevel);					
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
					int32 MagicDamage = static_cast<int32>(_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6f);

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
					int32 MagicDamage = static_cast<int32>(_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6f);

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
					st_HealSkillInfo* HealSkillInfo = (st_HealSkillInfo*)_SpellSkill->GetSkillInfo();

					uniform_int_distribution<int> HealChoiceRandom(HealSkillInfo->SkillMinHealPoint, HealSkillInfo->SkillMaxHealPoint);
					FinalDamage = HealChoiceRandom(Gen);

					st_GameObjectJob* HealJob = G_ObjectManager->GameServer->MakeGameObjectJobHPHeal(this, false, FinalDamage, _SpellSkill->GetSkillInfo()->SkillType);
					_SelectTarget->_GameObjectJobQue.Enqueue(HealJob);								
				}
				break;
			case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:
				{
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
				// Ŭ�󿡰� ��Ÿ�� ǥ��
				CMessage* ResCoolTimeStartPacket = G_ObjectManager->GameServer->MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
					QuickSlotBarPosition.QuickSlotBarSlotIndex,
					1.0f, _SpellSkill);
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResCoolTimeStartPacket);
				ResCoolTimeStartPacket->Free();
			}

			vector<CSkill*> GlobalSkills = _SkillBox.GetGlobalSkills(_SpellSkill->GetSkillInfo()->SkillType, _SpellSkill->GetSkillKind());
			
			// ���� ��Ÿ�� ����
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

			// ����â ��
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
			CRASH("ä���� ����� ���µ� ä�� ��û��");
			return;
		}		

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

		_GatheringTarget->OnDamaged(this, 1);		

		CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,			
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

		// ä��â ��
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
	// �þ߹��� ��ü ����
	if (_FieldOfViewUpdateTick < GetTickCount64() && _Channel != nullptr)
	{
		// �þ߹��� ������Ʈ�� �����ؼ� ����
		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldOfViewObjects(this, false);
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

		// �ѹ� �� �˻�
		if (SpawnObjectIds.size() > 0)
		{
			// ���� �ؾ��� ������ ����
			vector<CGameObject*> SpawnObjectInfos;
			for (st_FieldOfViewInfo SpawnObject : SpawnObjectIds)
			{
				if (SpawnObject.ObjectID != 0 && SpawnObject.ObjectType != en_GameObjectType::OBJECT_NON_TYPE)
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
				// �����ؾ� �� ������ ������ �����϶�� �˸�
				CMessage* ResOtherObjectSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn((int32)SpawnObjectInfos.size(), SpawnObjectInfos);
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResOtherObjectSpawnPacket);
				ResOtherObjectSpawnPacket->Free();
			}
		}

		// �ѹ� �� �˻�
		if (DeSpawnObjectIds.size() > 0)
		{
			vector<CGameObject*> DeSpawnObjectInfos;
			for (st_FieldOfViewInfo DeSpawnObject : DeSpawnObjectIds)
			{
				if (DeSpawnObject.ObjectID != 0 && DeSpawnObject.ObjectType != en_GameObjectType::OBJECT_NON_TYPE)
				{
					CGameObject* FindObject = _Channel->FindChannelObject(DeSpawnObject.ObjectID, DeSpawnObject.ObjectType);

					// �߰������� �˻�
					// ��ȯ���� �ؾ��� ����� ���� �غ� ���� �Ǵ� ���� ������ ��쿡�� �˾Ƽ� ��ȯ���� �Ǳ� ������
					// ���� �غ� ���� �׸��� ���� ���°� �ƴ� ��쿡�� ��ȯ���� �ϵ��� ����					
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

			// ��ȯ�����ؾ� �� ����� ������ ���� �����϶�� �˸�
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