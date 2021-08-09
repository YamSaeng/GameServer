#pragma once
#include "GameObject.h"

class CPlayer : public CGameObject
{
public:	
	wstring _PlayerName;
	int64 _SessionId;

	CPlayer();
	CPlayer(st_GameObjectInfo _PlayerInfo);
	~CPlayer();
};

