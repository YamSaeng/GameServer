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

void CSkill::ComboSkillStart(vector<Vector2Int> ComboSkillQuickSlotIndex, en_SkillType ComboSkilltype)
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

					// 데미지 처리
				}
			}

			// 상태이상 스킬의 남은 시간이 0 보다 작아질 경우 상태이상 해제
			if (_SkillInfo->SkillRemainTime <= 0)
			{
				_SkillInfo->SkillRemainTime = 0;

				if (_SkillInfo->SkillStatusAbnormal != en_GameObjectStatusType::STATUS_ABNORMAL_NONE)
				{
					_Owner->ReleaseStatusAbnormal((int64)_SkillInfo->SkillStatusAbnormalMask);
					CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
						_Owner->_GameObjectInfo.ObjectPositionInfo.Position.X,
						_Owner->_GameObjectInfo.ObjectPositionInfo.Position.Y,
						_SkillInfo,
						false, (int64)_SkillInfo->SkillStatusAbnormalMask);
				}

				CMessage* ResBufDeBufOffPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, true, _SkillInfo->SkillType);
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);
				ResBufDeBufOffPacket->Free();

				return true;
			}

			// 상태이상 적용 스킬 지속시간 재기
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
