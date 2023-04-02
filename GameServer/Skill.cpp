#include "pch.h"
#include "Skill.h"
#include "NetworkManager.h"

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

	_CastingUserID = 0;
	_CastingUserObjectType = en_GameObjectType::OBJECT_NON_TYPE;
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
		case en_SkillType::SKILL_GLOBAL_SKILL:
			_SkillKind = en_SkillKinds::SKILL_KIND_GLOBAL_SKILL;
			break;
		case en_SkillType::SKILL_DEFAULT_ATTACK:			
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:		
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_JUMPING_ATTACK:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_PIERCING_WAVE:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FLY_KNIFE:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_COMBO_FLY_KNIFE:
		case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
		case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP:				
			_SkillKind = en_SkillKinds::SKILL_KIND_MELEE_SKILL;
			break;		
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
		case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT:
		case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:		
			_SkillKind = en_SkillKinds::SKILL_KIND_SPELL_SKILL;
			break;
		case en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING:
			_SkillKind = en_SkillKinds::SKILL_KIND_RANGE_SKILL;
			break;
		case en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE:
		case en_SkillType::SKILL_SPELL_ACTIVE_BUF_TELEPORT:
		case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:
		case en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_WEAPON_POISON:
			_SkillKind = en_SkillKinds::SKILL_KIND_BUF_SKILL;
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
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FLY_KNIFE:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_COMBO_FLY_KNIFE:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NORMAL;
			break;
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_JUMPING_ATTACK:
		case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_PIERCING_WAVE:		
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_NORMAL_AND_DEBUF;
			break;
		case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:
			_SkillCharacteristic = en_SkillCharacteristic::SKILL_CATEGORY_FIGHT;

			_BufDeBufSkillKind = en_BufDeBufSkillKind::BUF_DEBUF_SKILL_KIND_BUF;
			break;
		case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
		case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE:
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
		}
	}
}

void CSkill::SetCastingUserID(int64 CastingUserID, en_GameObjectType CastingUserObjectType)
{
	_CastingUserID = CastingUserID;
	_CastingUserObjectType = CastingUserObjectType;
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
	case en_SkillCategory::SKILL_CATEGORY_GLOBAL:
		{
			if (_SkillInfo->CanSkillUse == false)
			{
				_SkillInfo->SkillRemainTime = _SkillCootimeTick - GetTickCount64();

				if (_SkillInfo->SkillRemainTime < 0)
				{
					_SkillInfo->CanSkillUse = true;
						
					_SkillInfo->SkillRemainTime = 0;
					_SkillCootimeTick = 0;					

					vector<st_FieldOfViewInfo> CurrentFieldOfViewInfo = _Owner->GetChannel()->GetMap()->GetFieldAroundPlayers(_Owner, false);
					
					CMessage* ResAttacketPacket = G_NetworkManager->GetGameServer()->MakePacketResAttack(_Owner->_GameObjectInfo.ObjectId);
					G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewInfo, ResAttacketPacket);
					ResAttacketPacket->Free();
				}
			}
		}
		break;
	case en_SkillCategory::SKILL_CATEGORY_ACTIVE_SKILL:
		{
			// ��ų�� ���������
			if (_SkillInfo->CanSkillUse == false)
			{			
				// ��ų ��Ÿ�� ���
				_SkillInfo->SkillRemainTime = _SkillCootimeTick - GetTickCount64();		

				// 0 ���� �۾��� ��� ��Ÿ�� �Ϸ� 
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

					// ������ ó��
				}
			}

			// �����̻� ��ų�� ���� �ð��� 0 ���� �۾��� ��� �����̻� ����
			if (_SkillInfo->SkillRemainTime <= 0)
			{
				_SkillInfo->SkillRemainTime = 0;

				switch (_SkillInfo->SkillType)
				{
				case en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE:
					{
						// ��� ���� ���� ����
						CMessage* ResBufDeBufOffPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, true, _SkillInfo->SkillType);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:
					{					
						// ���� �ڼ� ���� ����
						CMessage* ResBufDeBufOffPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, true, _SkillInfo->SkillType);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);
						ResBufDeBufOffPacket->Free();
					}
					break;			
				case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_JUMPING_ATTACK:
					{
						// ���� ���� �����̻� ����
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_JUMPING_ATTACK_MASK);
						CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, 
							_Owner->_GameObjectInfo.ObjectType, 							
							_SkillInfo->SkillType, 
							false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_JUMPING_ATTACK_MASK);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// ��ȭȿ�� ��ų ������ ����
						CMessage* ResBufDeBufOffPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE:
					{
						// ��ȹ ���� �̻� ����
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_CAPTURE_MASK);
						CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectType,							
							_SkillInfo->SkillType,
							false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_CAPTURE_MASK);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// ��ȭȿ�� ��ų ������ ����
						CMessage* ResBufDeBufOffPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:
					{
						// ���� �ӹ� �����̻� ����
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT_MASK);
						CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectType,							
							_SkillInfo->SkillType, false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT_MASK);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// ��ȭȿ�� ��ų ������ ����
						CMessage* ResBufDeBufOffPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
					{
						// ���� �����罽 �����̻� ����
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_CHAIN_MASK);

						float DebufMovingSpeed = _Owner->_GameObjectInfo.ObjectStatInfo.MaxSpeed * _SkillInfo->SkillDebufMovingSpeed * 0.01f;

						_Owner->_GameObjectInfo.ObjectStatInfo.Speed += DebufMovingSpeed;						

						CMessage* ResChangeObjectStatPacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectStat(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectStatInfo);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResChangeObjectStatPacket);
						ResChangeObjectStatPacket->Free();

						CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectType,							
							_SkillInfo->SkillType, false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_CHAIN_MASK);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// ��ȭȿ�� ��ų ������ ����
						CMessage* ResBufDeBufOffPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
					{
						// ���� �ñ��ĵ� �����̻� ����
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE_MASK);
						CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectType, 							
							_SkillInfo->SkillType, false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE_MASK);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// ��ȭȿ�� ��ų ������ ����
						CMessage* ResBufDeBufOffPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
					{
						// ���� ���� �����̻� ����
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE_MASK);
						CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, 
							_Owner->_GameObjectInfo.ObjectType,							
							_SkillInfo->SkillType, false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE_MASK);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// ��ȭȿ�� ��ų ������ ����
						CMessage* ResBufDeBufOffPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:
					{
						// ���� �ӹ� �����̻� ����
						_Owner->ReleaseStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT_MASK);
						CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, 
							_Owner->_GameObjectInfo.ObjectType,							
							_SkillInfo->SkillType, false, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT_MASK);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// ��ȭȿ�� ��ų ������ ����
						CMessage* ResBufDeBufOffPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;				
				}

				return true;
			}

			// �����̻� ���� ��ų ���ӽð� ���
			_SkillInfo->SkillRemainTime = _SkillDurationTick - GetTickCount64();
		}
		break;
	case en_SkillCategory::SKILL_CATEGORY_COMBO_SKILL:
		if (_ComboSkillTick < GetTickCount64())
		{			
			CMessage* ResNextComboSkillOff = G_NetworkManager->GetGameServer()->MakePacketComboSkillOff(_ComboSkillQuickSlotBarIndex,
				*_PreviousSkillInfo,
				_ComboSkillType);	

			G_NetworkManager->GetGameServer()->SendPacket(((CPlayer*)_Owner)->_SessionId, ResNextComboSkillOff);
			ResNextComboSkillOff->Free();

			_ComboSkillQuickSlotBarIndex.clear();
			return true;
		}
		break;		
	}	
	
	return false;
}
