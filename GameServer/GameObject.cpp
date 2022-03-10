#include "pch.h"
#include "GameObject.h"
#include "ObjectManager.h"

CGameObject::CGameObject()
{
	_ObjectManagerArrayIndex = -1;
	_ChannelArrayIndex = -1;
	_NetworkState = en_ObjectNetworkState::READY;
	_GameObjectInfo.OwnerObjectId = 0;	
	_Channel = nullptr;
	_Target = nullptr;
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

void CGameObject::SetTarget(CGameObject* Target)
{
	_Target = Target;
}

CGameObject* CGameObject::GetTarget()
{
	return _Target;
}

void CGameObject::BroadCastPacket(en_GAME_SERVER_PACKET_TYPE PacketType, bool CanMove)
{
	CMessage* ResPacket = nullptr;

	switch (PacketType)
	{							
	case en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_OBJECT_STAT_CHANGE:
		ResPacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_Target->_GameObjectInfo.ObjectId,
			_Target->_GameObjectInfo.ObjectStatInfo);
		break;
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