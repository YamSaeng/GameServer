#include "pch.h"
#include "Channel.h"
#include "Player.h"
#include "Message.h"

void CChannel::Init(int MapId, int SectorSize)
{
	_Map = new CMap(MapId);

	_SectorSize = SectorSize;

	int SectorCountY = (_Map->_SizeY + _SectorSize - 1) / _SectorSize;
	int SectorCountX = (_Map->_SizeX + _SectorSize - 1) / _SectorSize;

	_Sectors = new CSector * [SectorCountY];

	for (int i = 0; i < SectorCountY; i++)
	{
		_Sectors[i] = new CSector[SectorCountX];
	}

	for (int Y = 0; Y < SectorCountY; Y++)
	{
		for (int X = 0; X < SectorCountX; X++)
		{
			_Sectors[Y][X] = CSector(Y, X);
		}
	}
}

void CChannel::EnterChannel(CGameObject* GameObject)
{
	if (GameObject == nullptr)
	{
		CRASH("GameObject°¡ nullptr");
		return;
	}	

	st_Vector2Int SpawnPosition(0, 0);

	switch (GameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::PLAYER:
		{
			CPlayer* EnterChannelPlayer = (CPlayer*)GameObject;

			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._Y;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._X;

			_Players.insert(pair<int64, CPlayer*>(EnterChannelPlayer->_GameObjectInfo.ObjectId, EnterChannelPlayer));
			
			EnterChannelPlayer->_Channel = this;

			st_Vector2Int EnterChannelPlayerPosition;
			EnterChannelPlayerPosition._X = EnterChannelPlayer->GetPositionInfo().PositionX;
			EnterChannelPlayerPosition._Y = EnterChannelPlayer->GetPositionInfo().PositionY;

			_Map->ApplyMove(EnterChannelPlayer, EnterChannelPlayerPosition);
		}
		break;
	case en_GameObjectType::MONSTER:
		break;
	}
}
