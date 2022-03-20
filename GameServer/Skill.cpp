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

			// �����̻� ���� ��ų ���ӽð� ���
			_SkillInfo->SkillRemainTime = _SkillDurationTick - GetTickCount64();			

			// 0 ���� �۾��� ��� �����̻� ����
			if (_SkillInfo->SkillRemainTime < 0)
			{
				_SkillInfo->SkillRemainTime = 0;

				switch (_SkillInfo->SkillType)
				{
				case en_SkillType::SKILL_KNIGHT_SHAEHONE:
					{
						// �߹��� �����̻� ����
						_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_ROOT_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, _SkillInfo->SkillType, false, STATUS_ABNORMAL_ROOT_MASK);
						//G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);
						G_ObjectManager->GameServer->SendPacketFieldOfView(((CPlayer*)_Owner)->_FieldOfViewInfos, ResStatusAbnormalPacket, _Owner);
						ResStatusAbnormalPacket->Free();

						// ��ȭȿ�� ��ų ������ ����
						CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
						G_ObjectManager->GameServer->SendPacketFieldOfView(((CPlayer*)_Owner)->_FieldOfViewInfos, ResBufDeBufOffPacket, _Owner);
						ResBufDeBufOffPacket->Free();
					}
					break;
				case en_SkillType::SKILL_KNIGHT_CHOHONE:
					{
						// ���� �����̻� ����
						_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_STUN_MASK);
						CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, _SkillInfo->SkillType, false, STATUS_ABNORMAL_STUN_MASK);
						G_ObjectManager->GameServer->SendPacketFieldOfView(((CPlayer*)_Owner)->_FieldOfViewInfos, ResStatusAbnormalPacket, _Owner);
						ResStatusAbnormalPacket->Free();

						// ��ȭȿ�� ��ų ������ ����
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
