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
		
		// 0 ���� �۾��� ��� �����̻� ����
		if (_SkillInfo->SkillRemainTime <= 0)
		{
			_SkillInfo->SkillRemainTime = 0;

			switch (_SkillInfo->SkillType)
			{
			case en_SkillType::SKILL_SHOCK_RELEASE:
				{
					// ������� ���� ����
					CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, true, _SkillInfo->SkillType);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);
					ResBufDeBufOffPacket->Free();
				}
				break;
			case en_SkillType::SKILL_KNIGHT_CHARGE_POSE:
				{					
					// �����ڼ� ���� ����
					CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, true, _SkillInfo->SkillType);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);
					ResBufDeBufOffPacket->Free();
				}
				break;			
			case en_SkillType::SKILL_KNIGHT_SHAEHONE:
				{
					// ���� ��ȥ�� �����̻� ����
					_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_WARRIOR_SHAEHONE_MASK);
					CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, 
						_Owner->_GameObjectInfo.ObjectType, 
						_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
						_SkillInfo->SkillType, 
						false, STATUS_ABNORMAL_WARRIOR_SHAEHONE_MASK);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
					ResStatusAbnormalPacket->Free();

					// ��ȭȿ�� ��ų ������ ����
					CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
					ResBufDeBufOffPacket->Free();
				}
				break;
			case en_SkillType::SKILL_KNIGHT_CHOHONE:
				{
					// ���� ��ȥ�� �����̻� ����
					_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_WARRIOR_CHOHONE_MASK);
					CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
						_Owner->_GameObjectInfo.ObjectType,
						_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
						_SkillInfo->SkillType,
						false, STATUS_ABNORMAL_WARRIOR_CHOHONE_MASK);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
					ResStatusAbnormalPacket->Free();

					// ��ȭȿ�� ��ų ������ ����
					CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
					ResBufDeBufOffPacket->Free();
				}
				break;
			case en_SkillType::SKILL_SHAMAN_ROOT:
				{
					// �ּ��� �ӹ� �����̻� ����
					_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_SHAMAN_ROOT_MASK);
					CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
						_Owner->_GameObjectInfo.ObjectType,
						_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
						_SkillInfo->SkillType, false, STATUS_ABNORMAL_SHAMAN_ROOT_MASK);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
					ResStatusAbnormalPacket->Free();

					// ��ȭȿ�� ��ų ������ ����
					CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
					ResBufDeBufOffPacket->Free();
				}
				break;
			case en_SkillType::SKILL_SHAMAN_ICE_CHAIN:
				{
					// �ּ��� �����罽 �����̻� ����
					_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_SHAMAN_ICE_CHAIN_MASK);

					_Owner->_GameObjectInfo.ObjectStatInfo.Speed += ((st_AttackSkillInfo*)_SkillInfo)->SkillDebufMovingSpeed;

					CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
						_Owner->_GameObjectInfo.ObjectType,
						_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
						_SkillInfo->SkillType, false, STATUS_ABNORMAL_SHAMAN_ICE_CHAIN_MASK);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
					ResStatusAbnormalPacket->Free();

					// ��ȭȿ�� ��ų ������ ����
					CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
					ResBufDeBufOffPacket->Free();
				}
				break;
			case en_SkillType::SKILL_SHAMAN_ICE_WAVE:
				{
					// �ּ��� �ñ��ĵ� �����̻� ����
					_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_SHAMAN_ICE_WAVE_MASK);
					CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId,
						_Owner->_GameObjectInfo.ObjectType, 
						_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
						_SkillInfo->SkillType, false, STATUS_ABNORMAL_SHAMAN_ICE_WAVE_MASK);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
					ResStatusAbnormalPacket->Free();

					// ��ȭȿ�� ��ų ������ ����
					CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
					ResBufDeBufOffPacket->Free();
				}
				break;
			case en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE:
				{
					// �ּ��� ���� �����̻� ����
					_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE_MASK);
					CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, 
						_Owner->_GameObjectInfo.ObjectType,
						_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
						_SkillInfo->SkillType, false, STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE_MASK);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
					ResStatusAbnormalPacket->Free();

					// ��ȭȿ�� ��ų ������ ����
					CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
					ResBufDeBufOffPacket->Free();
				}
				break;
			case en_SkillType::SKILL_TAIOIST_ROOT:
				{
					// ���� �ӹ� �����̻� ����
					_Owner->ReleaseStatusAbnormal(STATUS_ABNORMAL_TAIOIST_ROOT_MASK);
					CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_Owner->_GameObjectInfo.ObjectId, 
						_Owner->_GameObjectInfo.ObjectType,
						_Owner->_GameObjectInfo.ObjectPositionInfo.MoveDir,
						_SkillInfo->SkillType, false, STATUS_ABNORMAL_TAIOIST_ROOT_MASK);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResStatusAbnormalPacket);					
					ResStatusAbnormalPacket->Free();

					// ��ȭȿ�� ��ų ������ ����
					CMessage* ResBufDeBufOffPacket = G_ObjectManager->GameServer->MakePacketBufDeBufOff(_Owner->_GameObjectInfo.ObjectId, false, _SkillInfo->SkillType);
					G_ObjectManager->GameServer->SendPacketFieldOfView(_Owner, ResBufDeBufOffPacket);					
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
	}

	return false;
}
