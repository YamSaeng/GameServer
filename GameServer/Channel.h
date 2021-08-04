#pragma once
#include "Sector.h"

class CPlayer;

class CChannel
{
public:
	int32 _ChannelId;

	map<int32, CPlayer*> _Players;
	
	CSector* Sectors;
	int32 _SectorSize;

	void Init(int MapId, int SectorSize);

};

