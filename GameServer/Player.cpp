#include "pch.h"
#include "Player.h"
#include "ObjectManager.h"

CPlayer::CPlayer()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_PLAYER;	
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
	_AttackTick = 0;
	_SpellTick = 0;
	_SkillJob = nullptr;

	_IsSendPacketTarget = true;
}

CPlayer::~CPlayer()
{
}

void CPlayer::Update()
{	
	if (_NatureRecoveryTick < GetTickCount64())
	{
		int32 AutoHPRecoveryPoint = 0;
		int32 AutoMPRecoveryPoint = 0;	

		AutoHPRecoveryPoint = (_GameObjectInfo.ObjectStatInfo.MaxHP / 100) * _GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent;
		AutoMPRecoveryPoint = (_GameObjectInfo.ObjectStatInfo.MaxMP / 100) * _GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent;

		_NatureRecoveryTick = GetTickCount64() + 5000;
	}

	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{	
	case en_CreatureState::MOVING:
		UpdateMove();
		break;
	case en_CreatureState::ATTACK:		
		UpdateAttack();
		break;
	case en_CreatureState::SPELL:
		UpdateSpell();
		break;
	default:
		break;
	}
}

bool CPlayer::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	return CGameObject::OnDamaged(Attacker, Damage);
}

void CPlayer::OnDead(CGameObject* Killer)
{

}

void CPlayer::Init()
{
	wstring InitString = L"";

	_GameObjectInfo.ObjectId = 0;
	_GameObjectInfo.ObjectName = InitString;
	_GameObjectInfo.OwnerObjectType = en_GameObjectType::NORMAL;
	memset(&_GameObjectInfo.ObjectStatInfo, 0, sizeof(st_StatInfo));
	_SpawnPosition._X = 0;
	_SpawnPosition._Y = 0;	
}

void CPlayer::PositionReset()
{
	// 이동 방향에 따라 좌표값 재 조정
	switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
	{
	case en_MoveDir::UP:
	case en_MoveDir::DOWN:
		if (_GameObjectInfo.ObjectPositionInfo.CollisionPositionY >= 0)
		{
			_GameObjectInfo.ObjectPositionInfo.PositionY =
				_GameObjectInfo.ObjectPositionInfo.CollisionPositionY + 0.5f;
		}
		else if (_GameObjectInfo.ObjectPositionInfo.CollisionPositionY == 0)
		{
			_GameObjectInfo.ObjectPositionInfo.PositionY =
				_GameObjectInfo.ObjectPositionInfo.CollisionPositionY;
		}
		else if (_GameObjectInfo.ObjectPositionInfo.CollisionPositionY < 0)
		{
			_GameObjectInfo.ObjectPositionInfo.PositionY =
				_GameObjectInfo.ObjectPositionInfo.CollisionPositionY - 0.5f;
		}
		break;
	case en_MoveDir::LEFT:
	case en_MoveDir::RIGHT:
		if (_GameObjectInfo.ObjectPositionInfo.CollisionPositionX >= 0)
		{
			_GameObjectInfo.ObjectPositionInfo.PositionX =
				_GameObjectInfo.ObjectPositionInfo.CollisionPositionX + 0.5f;
		}
		else if (_GameObjectInfo.ObjectPositionInfo.CollisionPositionX == 0)
		{
			_GameObjectInfo.ObjectPositionInfo.PositionX =
				_GameObjectInfo.ObjectPositionInfo.CollisionPositionX;
		}
		else if (_GameObjectInfo.ObjectPositionInfo.CollisionPositionX < 0)
		{
			_GameObjectInfo.ObjectPositionInfo.PositionX =
				_GameObjectInfo.ObjectPositionInfo.CollisionPositionX - 0.5f;
		}
		break;
	}
}

void CPlayer::UpdateMove()
{		
	st_Vector2 DirVector;	

	switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
	{
	case en_MoveDir::UP:
		DirVector = st_Vector2::Up();
		_GameObjectInfo.ObjectPositionInfo.PositionY += (DirVector._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::DOWN:
		DirVector = st_Vector2::Down();
		_GameObjectInfo.ObjectPositionInfo.PositionY += (DirVector._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::LEFT:
		DirVector = st_Vector2::Left();
		_GameObjectInfo.ObjectPositionInfo.PositionX += (DirVector._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::RIGHT:
		DirVector = st_Vector2::Right();
		_GameObjectInfo.ObjectPositionInfo.PositionX += (DirVector._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	}

	bool CanMove = _Channel->_Map->Cango(this, _GameObjectInfo.ObjectPositionInfo.PositionX, _GameObjectInfo.ObjectPositionInfo.PositionY);
	if (CanMove == true)
	{
		st_Vector2Int CollisionPosition;
		CollisionPosition._X = _GameObjectInfo.ObjectPositionInfo.PositionX;
		CollisionPosition._Y = _GameObjectInfo.ObjectPositionInfo.PositionY;
		
		if (CollisionPosition._X != _GameObjectInfo.ObjectPositionInfo.CollisionPositionX
			|| CollisionPosition._Y != _GameObjectInfo.ObjectPositionInfo.CollisionPositionY)
		{
			_Channel->_Map->ApplyMove(this, CollisionPosition);					
		}				
	}
	else
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

		PositionReset();

		// 바뀐 좌표 값 시야범위 오브젝트들에게 전송
		CMessage* ResMovePacket = G_ObjectManager->GameServer->MakePacketResMove(_AccountId,
			_GameObjectInfo.ObjectId,
			CanMove,
			_GameObjectInfo.ObjectPositionInfo);

		G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResMovePacket);
		ResMovePacket->Free();
	}		
}

void CPlayer::UpdateAttack()
{

}

void CPlayer::UpdateSpell()
{	

}