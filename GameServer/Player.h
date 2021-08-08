#pragma once
#include "GameObject.h"

class CPlayer : public CGameObject
{
public:	
	wstring _PlayerName;

	CPlayer();
	CPlayer(st_GameObjectInfo _PlayerInfo);
	~CPlayer();
};

