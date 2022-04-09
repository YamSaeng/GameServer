#include "pch.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "Skill.h"

CGameObject::CGameObject()
{
	_StatusAbnormal = 0;

	_ObjectManagerArrayIndex = -1;
	_ChannelArrayIndex = -1;
	_NetworkState = en_ObjectNetworkState::READY;
	_GameObjectInfo.OwnerObjectId = 0;	
	_Channel = nullptr;
	_Owner = nullptr;
	_SelectTarget = nullptr;

	_NatureRecoveryTick = 0;	
	_FieldOfViewUpdateTick = 0;
	_IsSendPacketTarget = false;
}

CGameObject::CGameObject(st_GameObjectInfo GameObjectInfo)
{
	_GameObjectInfo = GameObjectInfo;
	_NetworkState = en_ObjectNetworkState::READY;
	_GameObjectInfo.OwnerObjectId = 0;
}

CGameObject::~CGameObject()
{

}

void CGameObject::Update()
{
	while (!_GameObjectJobQue.IsEmpty())
	{
		st_GameObjectJob* GameObjectJob = nullptr;

		if (!_GameObjectJobQue.Dequeue(&GameObjectJob))
		{
			break;
		}
		
		switch ((en_GameObjectJobType)GameObjectJob->GameObjectJobType)
		{
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SHOCK_RELEASE:
			{
				for (auto DebufSkillIter : _DeBufs)
				{
					if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_KNIGHT_CHOHONE
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE)
					{
						DebufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
					}
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_BACK_TELEPORT:
			{
				for (auto DebufSkillIter : _DeBufs)
				{
					if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SHAMAN_ROOT
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_TAIOIST_ROOT)
					{
						DebufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
					}
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_MELEE_ATTACK:
			{				
				CPlayer* MyPlayer = ((CPlayer*)this);

				MyPlayer->_IsReqAttack = true;				

				CSkill* MeleeSkill;
				*GameObjectJob->GameObjectJobMessage >> &MeleeSkill;

				int16 ReqSkillType;
				*GameObjectJob->GameObjectJobMessage >> ReqSkillType;					
			
				CSkill* MeleeAttackSkill = G_ObjectManager->SkillCreate();
				st_AttackSkillInfo* MeleeAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(MeleeSkill->GetSkillInfo()->SkillMediumCategory);
				*MeleeAttackSkillInfo = *((st_AttackSkillInfo*)MeleeSkill->GetSkillInfo());

				MeleeAttackSkill->SetSkillInfo(en_SkillCategory::MELEE_SKILL, MeleeAttackSkillInfo);
				MeleeAttackSkill->SetOwner(this);
				MeleeAttackSkill->MeleeAttackSkillStart(MyPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate);

				MyPlayer->_CurrentMeleeAttack = MeleeAttackSkill;
			}
			break;
		}

		if (GameObjectJob->GameObjectJobMessage != nullptr)
		{
			GameObjectJob->GameObjectJobMessage->Free();
		}
		
		G_ObjectManager->GameObjectJobReturn(GameObjectJob);
	}
}

bool CGameObject::OnDamaged(CGameObject* Attacker, int32 DamagePoint)
{	
	_GameObjectInfo.ObjectStatInfo.HP -= DamagePoint;
	
	if (_GameObjectInfo.ObjectStatInfo.HP <= 0)
	{
		_GameObjectInfo.ObjectStatInfo.HP = 0;

		return true;
	}

	return false;
}

void CGameObject::OnHeal(CGameObject* Healer, int32 HealPoint)
{
	_GameObjectInfo.ObjectStatInfo.HP += HealPoint;

	if (_GameObjectInfo.ObjectStatInfo.HP >= _GameObjectInfo.ObjectStatInfo.MaxHP)
	{
		_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
	}
}

void CGameObject::OnDead(CGameObject* Killer)
{
	
}

st_Vector2 CGameObject::PositionCheck(st_Vector2Int& CheckPosition)
{
	st_Vector2 ResultPosition;

	if (CheckPosition._Y > 0)
	{
		ResultPosition._Y = 
			CheckPosition._Y + 0.5f;			
	}
	else if (CheckPosition._Y == 0)
	{
		ResultPosition._Y =
			CheckPosition._Y;
	}
	else if (CheckPosition._Y < 0)
	{
		ResultPosition._Y =
			CheckPosition._Y - 0.5f;		
	}
	
	if (CheckPosition._X > 0)
	{
		ResultPosition._X =
			CheckPosition._X + 0.5f;
	}
	else if (CheckPosition._X == 0)
	{
		ResultPosition._X =
			CheckPosition._X;
	}
	else if (CheckPosition._X < 0)
	{
		ResultPosition._X =
			CheckPosition._X - 0.5f;
	}	

	return ResultPosition;
}

void CGameObject::PositionReset()
{
}

st_PositionInfo CGameObject::GetPositionInfo()
{
	return _GameObjectInfo.ObjectPositionInfo;
}

st_Vector2Int CGameObject::GetCellPosition()
{
	return st_Vector2Int(_GameObjectInfo.ObjectPositionInfo.CollisionPositionX, _GameObjectInfo.ObjectPositionInfo.CollisionPositionY);
}

st_Vector2 CGameObject::GetPosition()
{
	return st_Vector2(_GameObjectInfo.ObjectPositionInfo.PositionX, _GameObjectInfo.ObjectPositionInfo.PositionY);
}

st_Vector2Int CGameObject::GetFrontCellPosition(en_MoveDir Dir, int8 Distance)
{
	st_Vector2Int FrontPosition = GetCellPosition();

	st_Vector2Int DirVector;
	switch (Dir)
	{
	case en_MoveDir::UP:
		DirVector = st_Vector2Int::Up() * Distance;
		break;
	case en_MoveDir::DOWN:
		DirVector = st_Vector2Int::Down() * Distance;
		break;
	case en_MoveDir::LEFT:
		DirVector = st_Vector2Int::Left() * Distance;
		break;
	case en_MoveDir::RIGHT:
		DirVector = st_Vector2Int::Right() * Distance;
		break;
	}

	FrontPosition = FrontPosition + DirVector;

	return FrontPosition;
}

vector<st_Vector2Int> CGameObject::GetAroundCellPositions(st_Vector2Int CellPosition, int8 Distance)
{
	vector<st_Vector2Int> AroundPosition;

	st_Vector2Int LeftTop(Distance * -1, Distance);
	st_Vector2Int RightDown(Distance, Distance * -1);

	st_Vector2Int LeftTopPosition = CellPosition + LeftTop;
	st_Vector2Int RightDownPosition = CellPosition + RightDown;

	for (int32 Y = LeftTopPosition._Y; Y >= RightDownPosition._Y; Y--)
	{
		for (int32 X = LeftTopPosition._X; X <= RightDownPosition._X; X++)
		{
			if (X == CellPosition._X && Y == CellPosition._Y)
			{
				continue;
			}

			AroundPosition.push_back(st_Vector2Int(X, Y));
		}
	}

	return AroundPosition;
}

void CGameObject::SetOwner(CGameObject* Target)
{
	_Owner = Target;
}

CGameObject* CGameObject::GetTarget()
{
	return _Owner;
}

void CGameObject::AddBuf(CSkill* Buf)
{	
	Buf->SetOwner(this);		

	_Bufs.insert(pair<en_SkillType, CSkill*>(Buf->GetSkillInfo()->SkillType, Buf));
}

void CGameObject::DeleteBuf(en_SkillType DeleteBufSkillType)
{
	_Bufs.erase(DeleteBufSkillType);	
}

void CGameObject::AddDebuf(CSkill* DeBuf)
{
	DeBuf->SetOwner(this);		

	_DeBufs.insert(pair<en_SkillType, CSkill*>(DeBuf->GetSkillInfo()->SkillType, DeBuf));
}

void CGameObject::DeleteDebuf(en_SkillType DeleteDebufSkillType)
{
	_DeBufs.erase(DeleteDebufSkillType);		
}

void CGameObject::SetStatusAbnormal(int8 StatusAbnormalValue)
{
	_StatusAbnormal |= StatusAbnormalValue;	
}

void CGameObject::ReleaseStatusAbnormal(int8 StatusAbnormalValue)
{
	_StatusAbnormal &= StatusAbnormalValue;
}

int8 CGameObject::CheckStatusAbnormal()
{
	int8 StatusAbnormalCount = 0;

	if (_StatusAbnormal & STATUS_ABNORMAL_WARRIOR_CHOHONE)
	{		
		StatusAbnormalCount++;		
	}

	if(_StatusAbnormal & STATUS_ABNORMAL_WARRIOR_SHAEHONE)
	{
		StatusAbnormalCount++;		
	}
	
	if ((_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ROOT)
		|| (_StatusAbnormal & STATUS_ABNORMAL_TAIOIST_ROOT))
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ICE_WAVE)
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE)
	{
		StatusAbnormalCount++;		
	}		

	return StatusAbnormalCount;
}

CChannel* CGameObject::GetChannel()
{
	return _Channel;
}

void CGameObject::SetChannel(CChannel* Channel)
{
	_Channel = Channel;
}

void CGameObject::BroadCastPacket(en_GAME_SERVER_PACKET_TYPE PacketType, bool CanMove)
{
	CMessage* ResPacket = nullptr;

	switch (PacketType)
	{								
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_DIE:			
		ResPacket = G_ObjectManager->GameServer->MakePacketObjectDie(this->_GameObjectInfo.ObjectId);
		break;		
	default:
		CRASH("Monster BroadCast PacketType Error");
		break;
	}

	G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResPacket);
	ResPacket->Free();
}