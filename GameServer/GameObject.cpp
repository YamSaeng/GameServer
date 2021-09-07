#include "pch.h"
#include "GameObject.h"
#include "ObjectManager.h"

CGameObject::CGameObject()
{
	_NetworkState = en_ObjectNetworkState::READY;
	_GameObjectInfo.OwnerObjectId = 0;
	_Channel = nullptr;
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

void CGameObject::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	_GameObjectInfo.ObjectStatInfo.HP -= Damage;

	if (_GameObjectInfo.ObjectStatInfo.HP <= 0)
	{
		_GameObjectInfo.ObjectStatInfo.HP = 0;
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

vector<st_Vector2Int> CGameObject::GetAroundCellPosition(st_Vector2Int CellPosition, int8 Distance)
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

en_MoveDir CGameObject::GetDirectionFromVector(st_Vector2Int DirectionVector)
{
	if (DirectionVector._X > 0)
	{
		return en_MoveDir::RIGHT;
	}
	else if (DirectionVector._X < 0)
	{
		return en_MoveDir::LEFT;
	}
	else if (DirectionVector._Y > 0)
	{
		return en_MoveDir::UP;
	}
	else
	{
		return en_MoveDir::DOWN;
	}
}

void CGameObject::BroadCastPacket(en_PACKET_TYPE PacketType)
{
	CMessage* ResPacket = nullptr;

	switch (PacketType)
	{
	case en_PACKET_TYPE::en_PACKET_S2C_MOVE:
		ResPacket = G_ObjectManager->GameServer->MakePacketResMove((int64)-1, _GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType, _GameObjectInfo.ObjectPositionInfo);
		break;
	case en_PACKET_TYPE::en_PACKET_S2C_OBJECT_STATE_CHANGE:
		ResPacket = G_ObjectManager->GameServer->MakePacketResObjectState(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.MoveDir,
			_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.State);
		break;
	case en_PACKET_TYPE::en_PACKET_S2C_MAGIC_ATTACK:
		ResPacket = G_ObjectManager->GameServer->MakePacketResMagic(_GameObjectInfo.ObjectId);
		break;
	case en_PACKET_TYPE::en_PACKET_S2C_CHANGE_HP:
		ResPacket = G_ObjectManager->GameServer->MakePacketResChangeHP(_Target->_GameObjectInfo.ObjectId,
			_Target->_GameObjectInfo.ObjectStatInfo.HP,
			_Target->_GameObjectInfo.ObjectStatInfo.MaxHP);
		break;
	case en_PACKET_TYPE::en_PACKET_S2C_DIE:
		ResPacket = G_ObjectManager->GameServer->MakePacketResDie(this->_GameObjectInfo.ObjectId);
		break;
	case en_PACKET_TYPE::en_PACKET_S2C_SPAWN:
	{
		vector<st_GameObjectInfo> SpawnObjectIds;
		SpawnObjectIds.push_back(_GameObjectInfo);
		ResPacket = G_ObjectManager->GameServer->MakePacketResSpawn(1, SpawnObjectIds);
	}
	break;
	case en_PACKET_TYPE::en_PACKET_S2C_DESPAWN:
	{
		vector<int64> DeSpawnObjectIds;
		DeSpawnObjectIds.push_back(_GameObjectInfo.ObjectId);
		ResPacket = G_ObjectManager->GameServer->MakePacketResDeSpawn(1, DeSpawnObjectIds);
	}
	break;	
	default:
		CRASH("Monster BroadCast PacketType Error");
		break;
	}

	G_ObjectManager->GameServer->SendPacketAroundSector(this->GetCellPosition(), ResPacket);
	ResPacket->Free();
}