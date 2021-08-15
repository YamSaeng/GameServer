#pragma once
#include "GameObject.h"

class CPlayer : public CGameObject
{
public:	
	int64 _SessionId;

	CPlayer();
	CPlayer(st_GameObjectInfo _PlayerInfo);
	~CPlayer();
};

