#include "pch.h"
#include "Skill.h"
#include "ObjectManager.h"

CSkill::CSkill()
{
	_Owner = nullptr;

	_SkillInfo = nullptr;		
	_PreviousSkillInfo = nullptr;

	_SkillCootimeTick = 0;
	_SkillDurationTick = 0;
	_SkillDotTick = 0;		

	_ComboSkillTick = 0;

	_MeleeAttackTick = 0;
	_MagicTick = 0;
		
	_IsDot = false;

	_SkillCategory = en_SkillCategory::SKILL_CATEGORY_NONE;
	_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_NONE;
	_SkillKind = en_SkillKinds::SKILL_KIND_NONE;
	_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NONE;
	_ComboSkillType = en_SkillType::SKILL_TYPE_NONE;	
}

CSkill::~CSkill()
{

}

st_SkillInfo* CSkill::GetSkillInfo()
{
	return _SkillInfo;
}

void CSkill::SetOwner(CGameObject* Owner)
{
	_Owner = Owner;
}

void CSkill::SetSkillInfo(en_SkillCategory SkillCategory, st_SkillInfo* SkillInfo, st_SkillInfo* PreviousSkillInfo)
{
	_SkillCategory = SkillCategory;
	_SkillInfo = SkillInfo;
	_PreviousSkillInfo = PreviousSkillInfo;

	if (_SkillInfo != nullptr)
	{
		switch (_SkillInfo->SkillType)
		{
		case en_SkillType::SKILL_DEFAULT_ATTACK:			
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE:
		case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP:
		case en_SkillType::SKILL_SLIME_ACTIVE_POISION_ATTACK:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_WEAPON_POISON:
			_SkillKind = en_SkillKinds::SKILL_KIND_MELEE_SKILL;
			break;
		case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:
		case en_SkillType::SKILL_SPELL_ACTIVE_BUF_TELEPORT:
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT:
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:
		case en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE:		
			_SkillKind = en_SkillKinds::SKILL_KIND_SPELL_SKILL;
			break;
		case en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING:
			_SkillKind = en_SkillKinds::SKILL_KIND_RANGE_SKILL;
			break;
		}

		switch (_SkillInfo->SkillType)
		{
		case en_SkillType::SKILL_DEFAULT_ATTACK:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NORMAL;
			break;
		case en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_BUF;
			break;
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE:				
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NORMAL;
			break;
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NORMAL_AND_DEBUF;
			break;
		case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_BUF;
			break;
		case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_PROTECTION;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NORMAL_AND_DEBUF;
			break;		
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON:				
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:		
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_SPELL;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NORMAL;
			break;
		case en_SkillType::SKILL_SPELL_ACTIVE_BUF_TELEPORT:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_SPELL;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_BUF;
			break;
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_SPELL;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NORMAL_AND_DEBUF;
			break;
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_SPELL;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_DEBUF;
			break;
		case en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_SHOOTING;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NORMAL;
			break;
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:		
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT:
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_DISCIPLINE;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NORMAL;
			break;			
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_DISCIPLINE;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_DEBUF;
			break;
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP:		
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_ASSASSINATION;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NORMAL;
			break;
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_WEAPON_POISON:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_ASSASSINATION;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_DEBUF;
			break;
		case en_SkillType::SKILL_SLIME_ACTIVE_POISION_ATTACK:	
			_IsDot = true;			

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_DEBUF;
			break;
		}
	}
}

void CSkill::SetCastingUserID(int64 CastingUserID)
{
	_CastingUserID = CastingUserID;
}

int64 CSkill::GetCastingUserID()
{
	return _CastingUserID;
}

void CSkill::CoolTimeStart()
{
	_SkillInfo->CanSkillUse = false;
	_SkillCootimeTick = _SkillInfo->SkillCoolTime + GetTickCount64();
	_SkillInfo->SkillRemainTime = _SkillCootimeTick - GetTickCount64();
}

void CSkill::GlobalCoolTimeStart(int32 GlobalCoolTime)
{
	_SkillInfo->CanSkillUse = false;
	_SkillCootimeTick = GlobalCoolTime + GetTickCount64();
	_SkillInfo->SkillRemainTime = _SkillCootimeTick - GetTickCount64();
}

void CSkill::StatusAbnormalDurationTimeStart()
{
	_SkillDurationTick = _SkillInfo->SkillDurationTime + GetTickCount64();

	_SkillDotTick = _SkillInfo->SkillDotTime + GetTickCount64();

	_SkillInfo->SkillRemainTime = _SkillDurationTick - GetTickCount64();
}

void CSkill::ComboSkillStart(vector<st_Vector2Int> ComboSkillQuickSlotIndex, en_SkillType ComboSkilltype)
{
	_ComboSkillQuickSlotBarIndex = ComboSkillQuickSlotIndex;

	_ComboSkillTick = GetTickCount64() + 3000;

	_ComboSkillType = ComboSkilltype;
}

void CSkill::ReqMeleeSkillInit(int64 AttackEndTick)
{
	_MeleeAttackTick = GetTickCount64() + AttackEndTick;
}

void CSkill::ReqMagicSkillInit(float MagicHitRate)
{
	_MagicTick = GetTickCount64() + (int64)(500 * MagicHitRate);
}

en_SkillKinds CSkill::GetSkillKind()
{
	return _SkillKind;
}

en_BufDeBufSkillKind CSkill::GetBufDeBufSkillKind()
{
	return _BufDeBufSkillKind;
}

void CSkill::ComboSkillOff()
{
	_ComboSkillTick = 0;
}

bool CSkill::Update()
{
	switch (_SkillCategory)
	{
	case en_SkillCategory::SKILL_CATEGORY_ACTIVE_SKILL:
		{
			// 스킬을 사용햇으면
			if (_SkillInfo->CanSkillUse == false)
			{			
				// 스킬 쿨타임 재기
				_SkillInfo->SkillRemainTime = _SkillCootimeTick - GetTickCount64();		

				// 0 보다 작아질 경우 쿨타임 완료 
				if (_SkillInfo->SkillRemainTime < 0)
				{
					_SkillInfo->CanSkillUse = true;

					_SkillInfo->SkillRemainTime = 0;
					_SkillCootimeTick = 0;
				}
			}
		}
		break;
	case en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL:
		{
			if (_IsDot == true)
			{
				if (_SkillDotTick < GetTickCount64())
				{
					_SkillDotTick = _SkillInfo->SkillDotTime + GetTickCount64();

					st_AttackSkillInfo* DotSkillInfo = (st_AttackSkillInfo*)_SkillInfo;

					bool IsCritical = false;
					int32 DotDamage = CMath::CalculateMeleeDamage(&IsCritical, _Owner->_GameObjectInfo.ObjectStatInfo.Defence,
						DotSkillInfo->SkillMinDamage,
						DotSkillInfo->SkillMaxDamage,
						0);					

					switch (_SkillInfo->SkillType)
					{
					case en_SkillType::SKILL_SLIME_ACTIVE_POISION_ATTACK:
						{							
							// 스킬 중첩 단계에 따라 데미지 적용							
							st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_CastingUserID,
								IsCritical, DotDamage * DotSkillInfo->SkillOverlapStep, en_SkillType::SKILL_SLIME_ACTIVE_POISION_ATTACK);
							_Owner->_GameObjectJobQue.Enqueue(DamageJob);							
						}
						break;
					}
				}
			}

			// 상태이상 스킬의 남은 시간이 0 보다 작아질 경우 상태이상 해제
			if (_SkillInfo->SkillRemainTime <= 0)
			{
				_SkillInfo->SkillRemainTime = 0;

				switch (_SkillInfo->SkillType)
				{
				case en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE:
					{
						// 충격해제 버프 삭제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, true, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:
					{					
						// 돌격자세 버프 삭제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, true, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);
						ResBufDeBufOffPacket->Free();
					}
					break;			
				case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE:
					{
						// 전사 쇄혼비무 상태이상 해제
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_SHAEHONE_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, 
							_Owner->_GameObjectInfo.ObjectType, 
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType, 
							false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_SHAEHONE_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE:
					{
						// 전사 초혼비무 상태이상 해제
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_CHOHONE_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectType,
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType,
							false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_CHOHONE_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:
					{
						// 주술사 속박 상태이상 해제
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectType,
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType, false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
					{
						// 주술사 얼음사슬 상태이상 해제
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_CHAIN_MASK);

						float DebufMovingSpeed = _Owner->_GameObjectInfo.ObjectStatInfo.MaxSpeed * ((st_AttackSkillInfo*)_SkillInfo)->SkillDebufMovingSpeed * 0.01;

						_Owner->_GameObjectInfo.ObjectStatInfo.Speed += DebufMovingSpeed;						

						CMessage* ResChangeObjectStatPacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectStatInfo);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResChangeObjectStatPacket);
						ResChangeObjectStatPacket->Free();

						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectType,
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType, false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_CHAIN_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
					{
						// 주술사 냉기파동 상태이상 해제
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectType, 
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType, false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
					{
						// 주술사 낙뢰 상태이상 해제
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, 
							_Owner->_GameObjectInfo.ObjectType,
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType, false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:
					{
						// 도사 속박 상태이상 해제
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, 
							_Owner->_GameObjectInfo.ObjectType,
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType, false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SLIME_ACTIVE_POISION_ATTACK:
					{
						_IsDot = false;						

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);
						ResBufDeBufOffPacket->Free();
					}
					break;
				}

				return true;
			}

			// 상태이상 적용 스킬 지속시간 재기
			_SkillInfo->SkillRemainTime = _SkillDurationTick - GetTickCount64();
		}
		break;
	case en_SkillCategory::SKILL_CATEGORY_COMBO_SKILL:
		if (_ComboSkillTick < GetTickCount64())
		{			
			CMessage* ResNextComboSkillOff = G_ObjectManager->GameServer->MakePacketComboSkillOff(_ComboSkillQuickSlotBarIndex,
				*_PreviousSkillInfo,
				_ComboSkillType);	

			G_ObjectManager->GameServer->SendPacket(((CPlayer*)_Owner)->_SessionId, ResNextComboSkillOff);
			ResNextComboSkillOff->Free();

			_ComboSkillQuickSlotBarIndex.clear();
			return true;
		}
		break;		
	}	
	
	return false;
}
