#include "pch.h"
#include "Skill.h"
#include "ObjectManager.h"

CSkill::CSkill()
{
	_Owner = nullptr;

	_SkillInfo = nullptr;

	_QuickSlotBarIndex = -1;
	_QuickSlotBarSlotIndex = -1;

	_ComboSkillType = en_SkillType::SKILL_TYPE_NONE;

	_SkillCootimeTick = 0;
	_SkillDotTick = 0;
	_SkillDurationTick = 0;	
	_ComboSkillTick = 0;

	_MeleeAttackTick = 0;
	_MagicTick = 0;

	_SkillKind = en_SkillKinds::SKILL_KIND_NONE;

	_IsDot = false;
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
		case en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK:
		case en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK:
		case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
		case en_SkillType::SKILL_KNIGHT_SHAEHONE:
		case en_SkillType::SKILL_KNIGHT_CHOHONE:
			_SkillKind = en_SkillKinds::MELEE_SKILL;
			break;
		case en_SkillType::SKILL_KNIGHT_CHARGE_POSE:
		case en_SkillType::SKILL_SHAMAN_FLAME_HARPOON:
		case en_SkillType::SKILL_SHAMAN_ROOT:
		case en_SkillType::SKILL_SHAMAN_ICE_CHAIN:
		case en_SkillType::SKILL_SHAMAN_ICE_WAVE:
		case en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE:
		case en_SkillType::SKILL_SHAMAN_HELL_FIRE:
		case en_SkillType::SKILL_TAIOIST_DIVINE_STRIKE:
		case en_SkillType::SKILL_TAIOIST_ROOT:
		case en_SkillType::SKILL_SHAMAN_BACK_TELEPORT:
		case en_SkillType::SKILL_TAIOIST_HEALING_LIGHT:
		case en_SkillType::SKILL_TAIOIST_HEALING_WIND:
		case en_SkillType::SKILL_SHOCK_RELEASE:
			_SkillKind = en_SkillKinds::MAGIC_SKILL;
			break;
		}
	}
}

void CSkill::CoolTimeStart()
{
	_SkillInfo->CanSkillUse = false;
	_SkillCootimeTick = _SkillInfo->SkillCoolTime + GetTickCount64();
	_SkillInfo->SkillRemainTime = _SkillCootimeTick - GetTickCount64();
}

void CSkill::StatusAbnormalDurationTimeStart()
{
	_SkillDurationTick = _SkillInfo->SkillDurationTime + GetTickCount64();

	_SkillDotTick = _SkillInfo->SkillDotTime + GetTickCount64();

	_SkillInfo->SkillRemainTime = _SkillDurationTick - GetTickCount64();
}

void CSkill::ComboSkillStart(int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, en_SkillType ComboSkilltype)
{
	_QuickSlotBarIndex = QuickSlotBarIndex;
	_QuickSlotBarSlotIndex = QuickSlotBarSlotIndex;

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

bool CSkill::Update()
{
	switch (_SkillCategory)
	{
	case en_SkillCategory::QUICK_SLOT_SKILL_COOLTIME:
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
	case en_SkillCategory::STATUS_ABNORMAL_SKILL:
		{
			if (_IsDot == true)
			{
				if (_SkillDotTick < GetTickCount64())
				{
					switch (_SkillInfo->SkillType)
					{

					}
				}
			}
			
			// 0 보다 작아질 경우 상태이상 해제
			if (_SkillInfo->SkillRemainTime <= 0)
			{
				_SkillInfo->SkillRemainTime = 0;

				switch (_SkillInfo->SkillType)
				{
				case en_SkillType::SKILL_SHOCK_RELEASE:
					{
						// 충격해제 버프 삭제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, true, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_KNIGHT_CHARGE_POSE:
					{					
						// 돌격자세 버프 삭제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, true, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);
						ResBufDeBufOffPacket->Free();
					}
					break;			
				case en_SkillType::SKILL_KNIGHT_SHAEHONE:
					{
						// 전사 쇄혼비무 상태이상 해제
						_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_WARRIOR_SHAEHONE_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, 
							_Owner->_GameObjectInfo.ObjectType, 
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType, 
							false, STATUS_ABNORMAL_WARRIOR_SHAEHONE_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_KNIGHT_CHOHONE:
					{
						// 전사 초혼비무 상태이상 해제
						_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_WARRIOR_CHOHONE_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectType,
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType,
							false, STATUS_ABNORMAL_WARRIOR_CHOHONE_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SHAMAN_ROOT:
					{
						// 주술사 속박 상태이상 해제
						_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_SHAMAN_ROOT_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectType,
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType, false, STATUS_ABNORMAL_SHAMAN_ROOT_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SHAMAN_ICE_CHAIN:
					{
						// 주술사 얼음사슬 상태이상 해제
						_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_SHAMAN_ICE_CHAIN_MASK);

						_Owner->_GameObjectInfo.ObjectStatInfo.Speed += ((st_AttackSkillInfo*)_SkillInfo)->SkillDebufMovingSpeed;

						CMessage* ResChangeObjectStatPacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectStatInfo);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResChangeObjectStatPacket);
						ResChangeObjectStatPacket->Free();

						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectType,
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType, false, STATUS_ABNORMAL_SHAMAN_ICE_CHAIN_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SHAMAN_ICE_WAVE:
					{
						// 주술사 냉기파동 상태이상 해제
						_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_SHAMAN_ICE_WAVE_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
							_Owner->_GameObjectInfo.ObjectType, 
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType, false, STATUS_ABNORMAL_SHAMAN_ICE_WAVE_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE:
					{
						// 주술사 낙뢰 상태이상 해제
						_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, 
							_Owner->_GameObjectInfo.ObjectType,
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType, false, STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_TAIOIST_ROOT:
					{
						// 도사 속박 상태이상 해제
						_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_TAIOIST_ROOT_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, 
							_Owner->_GameObjectInfo.ObjectType,
							_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
							_SkillInfo->SkillType, false, STATUS_ABNORMAL_TAIOIST_ROOT_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
						ResStatusAbnormalPacket->Free();

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
	case en_SkillCategory::COMBO_SKILL:
		if (_ComboSkillTick < GetTickCount64())
		{			
			CMessage* ResNextComboSkillOff = G_ObjectManager->GameServer->MakePacketComboSkillOff(_QuickSlotBarIndex,
				_QuickSlotBarSlotIndex,
				*_PreviousSkillInfo,
				_ComboSkillType);	

			G_ObjectManager->GameServer->SendPacket(((CPlayer*)_Owner)->_SessionId, ResNextComboSkillOff);
			ResNextComboSkillOff->Free();			
			return true;
		}
		break;
	case en_SkillCategory::REQ_MELEE_SKILL_INIT:
		if (_MeleeAttackTick < GetTickCount64())
		{			
			((CPlayer*)_Owner)->_IsReqAttack = false;			
			return true;
		}		
		break;
	case en_SkillCategory::REQ_MAGIC_SKILL_INIT:
		if (_MagicTick < GetTickCount64())
		{
			((CPlayer*)_Owner)->_IsReqMagic = false;			
			return true;
		}
		break;
	}	
	
	return false;
}
