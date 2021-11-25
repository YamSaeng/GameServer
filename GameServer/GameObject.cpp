#include "pch.h"
#include "GameObject.h"
#include "ObjectManager.h"

CGameObject::CGameObject()
{
	_ObjectManagerIndex = -1;
	_NetworkState = en_ObjectNetworkState::READY;
	_GameObjectInfo.OwnerObjectId = 0;	
	_Channel = nullptr;
	_Target = nullptr;
	_SelectTarget = nullptr;

	_NatureRecoveryTick = 0;
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

st_PositionInfo CGameObject::GetPositionInfo()
{
	return _GameObjectInfo.ObjectPositionInfo;
}

st_Vector2Int CGameObject::GetCellPosition()
{
	return st_Vector2Int(_GameObjectInfo.ObjectPositionInfo.PositionX, _GameObjectInfo.ObjectPositionInfo.PositionY);
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

void CGameObject::BroadCastPacket(en_PACKET_TYPE PacketType)
{
	CMessage* ResPacket = nullptr;

	switch (PacketType)
	{
	case en_PACKET_TYPE::en_PACKET_S2C_MOVE:
		ResPacket = G_ObjectManager->GameServer->MakePacketResMove((int64)-1, _GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType, _GameObjectInfo.ObjectPositionInfo);
		break;
	case en_PACKET_TYPE::en_PACKET_S2C_PATROL:
		ResPacket = G_ObjectManager->GameServer->MakePacketPatrol(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo);
		break;
	case en_PACKET_TYPE::en_PACKET_S2C_OBJECT_STATE_CHANGE:
		ResPacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.MoveDir,
			_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.State);
		break;	
	case en_PACKET_TYPE::en_PACKET_S2C_OBJECT_STAT_CHANGE:
		ResPacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_Target->_GameObjectInfo.ObjectId,
			_Target->_GameObjectInfo.ObjectStatInfo);
		break;
	case en_PACKET_TYPE::en_PACKET_S2C_DIE:
		ResPacket = G_ObjectManager->GameServer->MakePacketObjectDie(this->_GameObjectInfo.ObjectId);
		break;
	case en_PACKET_TYPE::en_PACKET_S2C_SPAWN:
	{
		vector<st_GameObjectInfo> SpawnObjectIds;
		SpawnObjectIds.push_back(_GameObjectInfo);
		ResPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn(1, SpawnObjectIds);
	}
	break;
	case en_PACKET_TYPE::en_PACKET_S2C_DESPAWN:
	{
		vector<int64> DeSpawnObjectIds;
		DeSpawnObjectIds.push_back(_GameObjectInfo.ObjectId);
		ResPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, DeSpawnObjectIds);
	}
	break;	
	default:
		CRASH("Monster BroadCast PacketType Error");
		break;
	}

	G_ObjectManager->GameServer->SendPacketAroundSector(this->GetCellPosition(), ResPacket);
	ResPacket->Free();
}