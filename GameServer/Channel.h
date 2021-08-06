#pragma once
#include "Sector.h"
#include "Map.h"

class CGameObject;
class CPlayer;

class CChannel
{
public:
	int32 _ChannelId;

	map<int64, CPlayer*> _Players;
	
	CMap* _Map;

	CSector** _Sectors;
	int32 _SectorSize;

	void Init(int MapId, int SectorSize);
	void EnterChannel(CGameObject* GameObject);
};

