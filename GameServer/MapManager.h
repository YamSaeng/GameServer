#pragma once

class CMap;

class CMapManager
{
public:
	CMapManager();
	~CMapManager();

	void MapSave();

	CMap* GetMap(int64 MapID);

	void Update();
private:
	map<int64, CMap*> _Maps;

	SRWLOCK _MapManagerLock;
};

