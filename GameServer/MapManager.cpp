#include "pch.h"
#include "Map.h"
#include "MapManager.h"
#include "ChannelManager.h"
#include "DataManager.h"
#include <atlbase.h>

CMapManager::CMapManager()
{
	InitializeSRWLock(&_MapManagerLock);
}

CMapManager::~CMapManager()
{

}

void CMapManager::MapSave()
{
	for (auto MapInfoIterator : G_Datamanager->_MapInfoDatas)
	{
		CMap* NewMap = new CMap();

		st_MapInfo NewMapInfo;
		NewMapInfo.MapID = MapInfoIterator.second->MapID;
		NewMapInfo.MapName = (LPWSTR)CA2W(MapInfoIterator.second->MapName.c_str());
		NewMapInfo.MapSectorSize = MapInfoIterator.second->MapSectorSize;
		NewMapInfo.ChannelCount = MapInfoIterator.second->ChannelCount;

		NewMap->MapInit(NewMapInfo.MapID, NewMapInfo.MapName, NewMapInfo.MapSectorSize, NewMapInfo.ChannelCount);

		_Maps.insert(pair<int64, CMap*>(NewMap->_MapID, NewMap));
	}
}

CMap* CMapManager::GetMap(int64& MapID)
{
	CMap* Map = nullptr;

	auto MapFindIterator = _Maps.find(MapID);
	if (MapFindIterator != _Maps.end())
	{
		Map = (*MapFindIterator).second;
	}
	else
	{

	}

	return Map;
}

void CMapManager::Update()
{
	for (auto MapIterator : _Maps)
	{
		MapIterator.second->GetChannelManager()->Update();
	}
}
