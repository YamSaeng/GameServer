#include "pch.h"
#include "Skill.h"
#include "ObjectManager.h"

CSkill::CSkill()
{	
	_Owner = nullptr;

	_SkillInfo = nullptr;

	_SkillCootimeTick = 0;
	_SkillDurationTick = 0;
	_SkillDotTick = 0;	

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

void CSkill::SetSkillInfo(en_SkillCategory SkillCategory, st_SkillInfo* SkillInfo)
{
	_SkillCategory = SkillCategory;
	_SkillInfo = SkillInfo;

	switch (SkillCategory)
	{
	case en_SkillCategory::QUICK_SLOT_SKILL:				
		break;
	case en_SkillCategory::STATUS_ABNORMAL_SKILL:
		_SkillDurationTick = _SkillInfo->SkillDurationTime + GetTickCount64();
		_SkillDotTick = _SkillInfo->SkillDotTime + GetTickCount64();		
		break;	
	}	
}

void CSkill::CoolTimeStart()
{
	_SkillCootimeTick = _SkillInfo->SkillCoolTime + GetTickCount64();

	_SkillInfo->CanSkillUse = false;

	_SkillInfo->SkillRemainTime = _SkillCootimeTick - GetTickCount64();
}

bool CSkill::Update()
{
	switch (_SkillCategory)
	{
	case en_SkillCategory::QUICK_SLOT_SKILL:
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

			// 상태이상 적용 스킬 지속시간 재기
			_SkillInfo->SkillRemainTime = _SkillDurationTick - GetTickCount64();			

			// 0 보다 작아질 경우 상태이상 해제
			if (_SkillInfo->SkillRemainTime < 0)
			{
				_SkillInfo->SkillRemainTime = 0;

				switch (_SkillInfo->SkillType)
				{
				case en_SkillType::SKILL_KNIGHT_SHAEHONE:
					{
						// 발묶음 상태이상 해제
						_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_ROOT_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, _SkillInfo->SkillType, false, STATUS_ABNORMAL_ROOT_MASK);
						//G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);
						G_ObjectManager->GameServer->SendPacketFieldOfView(((CPlayer*)_Owner)->_FieldOfViewInfos, ResStatusAbnormalPacket, _Owner);
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(((CPlayer*)_Owner)->_FieldOfViewInfos, ResBufDeBufOffPacket, _Owner);
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_KNIGHT_CHOHONE:
					{
						// 스턴 상태이상 해제
						_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_STUN_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, _SkillInfo->SkillType, false, STATUS_ABNORMAL_STUN_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(((CPlayer*)_Owner)->_FieldOfViewInfos, ResStatusAbnormalPacket, _Owner);
						ResStatusAbnormalPacket->Free();

						// 약화효과 스킬 아이콘 해제
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(((CPlayer*)_Owner)->_FieldOfViewInfos, ResBufDeBufOffPacket, _Owner);
						ResBufDeBufOffPacket->Free();
					}
					break;
				}				

				return true;
			}
		}
		break;	
	}

	return false;
}
