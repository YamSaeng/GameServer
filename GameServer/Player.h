#pragma once
#include "GameObject.h"

class CPlayer : public CGameObject
{
public:
	int32 _PlayerDBId;
	wstring _PlayerName;

	CPlayer();
	CPlayer(st_GameObjectInfo _PlayerInfo);
	~CPlayer();
};

