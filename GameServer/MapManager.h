#pragma once

class CMap;

class CMapManager
{
public:
	CMapManager();
	~CMapManager();

	void MapSave(vector<st_TileInfo>& TileInfos);

	CMap* GetMap(int64 MapID);

	void Update();
private:
	map<int64, CMap*> _Maps;

	SRWLOCK _MapManagerLock;
};

