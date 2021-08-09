#pragma once

class CPlayer;
class CGameObject;

class CSector
{
private:
	set<CPlayer*> _Players;
public:
	int32 _SectorY;
	int32 _SectorX;	

	CSector() {};
	CSector(int32 SectorY,int32 SectorX);

	void Insert(CGameObject* InsertGameObject);
	void Remove(CGameObject* RemoveGameObject);
	
	set<CPlayer*> GetPlayers();	
};

