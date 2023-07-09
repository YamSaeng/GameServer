#include "pch.h"
#include "Skill.h"
#include "NetworkManager.h"

CSkill::CSkill()
{
	_Target = nullptr;

	_SkillInfo = nullptr;			

	_SkillCootimeTick = 0;
	_SkillDurationTick = 0;
	_SkillDotTick = 0;		

	_ComboSkillTick = 0;

	_MeleeAttackTick = 0;
	_MagicTick = 0;
		
	_IsDot = false;

	_SkillCategory = en_SkillCategory::SKILL_CATEGORY_NONE;			

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

void CSkill::SetTarget(CGameObject* Target)
{
	_Target = Target;
}

void CSkill::SetSkillInfo(en_SkillCategory SkillCategory, st_SkillInfo* SkillInfo)
{
	_SkillCategory = SkillCategory;
	_SkillInfo = SkillInfo;	
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

void CSkill::BufTimeStart()
{
	_SkillDurationTick = _SkillInfo->SkillDurationTime + GetTickCount64();

	_SkillDotTick = _SkillInfo->SkillDotTime + GetTickCount64();

	_SkillInfo->SkillRemainTime = _SkillDurationTick - GetTickCount64();	
}

void CSkill::StatusAbnormalDurationTimeStart()
{	
	_SkillDurationTick = _SkillInfo->SkillDurationTime + GetTickCount64();

	_SkillDotTick = _SkillInfo->SkillDotTime + GetTickCount64();

	_SkillInfo->SkillRemainTime = _SkillDurationTick - GetTickCount64();

	switch (_SkillInfo->SkillType)
	{
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
		{
			float DebufSlowSpeed = _Target->_GameObjectInfo.ObjectStatInfo.MaxSpeed * _SkillInfo->SkillDebufMovingSpeed;
			_Target->_GameObjectInfo.ObjectStatInfo.Speed -= DebufSlowSpeed;			
		}
		break;	
	}
}

void CSkill::ComboSkillStart(vector<st_QuickSlotBarPosition> ComboSkillQuickSlotIndex)
{
	_ComboSkillQuickSlotBarPosition = ComboSkillQuickSlotIndex;
	_ComboSkillTick = GetTickCount64() + 3000;
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

					vector<st_FieldOfViewInfo> CurrentFieldOfViewInfo = _Target->GetChannel()->GetMap()->GetFieldAroundPlayers(_Target, false);
					
					CMessage* ResAttacketPacket = G_NetworkManager->GetGameServer()->MakePacketResAttack(_Target->_GameObjectInfo.ObjectId);
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
	case en_SkillCategory::SKILL_CATEGORY_BUF_SKILL:
		{
			if (_IsDot == true)
			{
				if (_SkillDotTick < GetTickCount64())
				{
					_SkillDotTick = _SkillInfo->SkillDotTime + GetTickCount64();

				}
			}

			if (_SkillInfo->SkillRemainTime <= 0)
			{
				_SkillInfo->SkillRemainTime = 0;				

				CMessage* ResBufDeBufOffPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBufOff(_Target->_GameObjectInfo.ObjectId, true, _SkillInfo->SkillType);
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Target, ResBufDeBufOffPacket);
				ResBufDeBufOffPacket->Free();

				return true;
			}

			// 강화효과 스킬 지속시간 재기
			_SkillInfo->SkillRemainTime = _SkillDurationTick - GetTickCount64();
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
					_Target->ReleaseStatusAbnormal((int64)_SkillInfo->SkillStatusAbnormalMask);
					CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(_Target->_GameObjectInfo.ObjectId,
						_Target->_GameObjectInfo.ObjectPositionInfo.Position.X,
						_Target->_GameObjectInfo.ObjectPositionInfo.Position.Y,
						_SkillInfo,
						false, (int64)_SkillInfo->SkillStatusAbnormalMask);
				}

				CMessage* ResBufDeBufOffPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBufOff(_Target->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_Target, ResBufDeBufOffPacket);
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
			CPlayer* Player = dynamic_cast<CPlayer*>(_Target);
			if (Player != nullptr)
			{
				for (auto QuickSlotBarPosition : _ComboSkillQuickSlotBarPosition)
				{
					st_QuickSlotBarSlotInfo* QuickSlotBarInfo = Player->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarPosition.QuickSlotBarIndex, QuickSlotBarPosition.QuickSlotBarSlotIndex);
					if (QuickSlotBarInfo != nullptr)
					{
						switch (QuickSlotBarInfo->QuickBarSkill->GetSkillInfo()->SkillType)
						{
						case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
							switch (_SkillInfo->SkillType)
							{
							case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:								
								{
									CMessage* ResNextComboSkillOff = G_NetworkManager->GetGameServer()->MakePacketComboSkillOff(_ComboSkillQuickSlotBarPosition,
										_SkillInfo->SkillType,
										_SkillInfo->RollBackSkill);
									G_NetworkManager->GetGameServer()->SendPacket(((CPlayer*)_Target)->_SessionId, ResNextComboSkillOff);
									ResNextComboSkillOff->Free();
								}
								break;
							case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_WRATH_ATTACK:
								{
									CMessage* ResNextComboSkillOff = G_NetworkManager->GetGameServer()->MakePacketComboSkillOff(_ComboSkillQuickSlotBarPosition,
										_SkillInfo->SkillType,
										_SkillInfo->RollBackSkill);
									G_NetworkManager->GetGameServer()->SendPacket(((CPlayer*)_Target)->_SessionId, ResNextComboSkillOff);
									ResNextComboSkillOff->Free();
								}
								break;
							}
							break;
						case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
							switch (_SkillInfo->SkillType)
							{
							case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_WRATH_ATTACK:
								{
									CMessage* ResNextComboSkillOff = G_NetworkManager->GetGameServer()->MakePacketComboSkillOff(_ComboSkillQuickSlotBarPosition,
										_SkillInfo->SkillType,
										_SkillInfo->RollBackSkill);
									G_NetworkManager->GetGameServer()->SendPacket(((CPlayer*)_Target)->_SessionId, ResNextComboSkillOff);
									ResNextComboSkillOff->Free();
								}
								break;
							}
							break;						
						}						
					}
				}			
			}			

			_ComboSkillQuickSlotBarPosition.clear();
			return true;
		}
		break;		
	}	
	
	return false;
}
