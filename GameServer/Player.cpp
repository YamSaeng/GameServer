#include "pch.h"
#include "Player.h"

CPlayer::CPlayer()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::PLAYER;
}

CPlayer::CPlayer(st_GameObjectInfo _PlayerInfo)
{
	_GameObjectInfo = _PlayerInfo;		
	_GameObjectInfo.ObjectType = en_GameObjectType::PLAYER;	
}

CPlayer::~CPlayer()
{
}
