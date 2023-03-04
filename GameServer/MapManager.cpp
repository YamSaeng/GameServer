#include "pch.h"
#include "Map.h"
#include "MapManager.h"
#include "ChannelManager.h"
#include "DataManager.h"
#include "ObjectManager.h"
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

		NewMap->_MapID = MapInfoIterator.second->MapID;
		NewMap->_MapName = (LPWSTR)CA2W(MapInfoIterator.second->MapName.c_str());		
		NewMap->_SectorSize = MapInfoIterator.second->MapSectorSize;		
		NewMap->_ChannelCount = MapInfoIterator.second->ChannelCount;
		NewMap->_Left = MapInfoIterator.second->Left;
		NewMap->_Right = MapInfoIterator.second->Right;
		NewMap->_Up = MapInfoIterator.second->Up;
		NewMap->_Down = MapInfoIterator.second->Down;

		NewMap->MapInit();
		
		for (auto MapGameObjectListIter : MapInfoIterator.second->GameObjectList)
		{						
			for (st_Vector2Int Position : MapGameObjectListIter.second)
			{
				CGameObject* NewObject = G_ObjectManager->ObjectCreate(MapGameObjectListIter.first);
				if (NewObject != nullptr)
				{
					NewObject->_SpawnPosition = Position;
					NewObject->_NetworkState = en_ObjectNetworkState::LIVE;

					CChannel* Channel = NewMap->GetChannelManager()->Find(1);
					Channel->EnterChannel(NewObject, &NewObject->_SpawnPosition);
				}
			}
		}		

		_Maps.insert(pair<int64, CMap*>(NewMap->_MapID, NewMap));
	}
}

CMap* CMapManager::GetMap(int64 MapID)
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
