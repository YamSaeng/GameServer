#pragma once

class CGameObject;
class CPlayer;
class CMonster;

class CSector
{
private:
	set<CPlayer*> _Players;
	set<CMonster*> _Monsters;
public:
	int32 _SectorY;
	int32 _SectorX;	

	CSector() {};
	CSector(int32 SectorY,int32 SectorX);

	void Insert(CGameObject* InsertGameObject);
	void Remove(CGameObject* RemoveGameObject);
	
	set<CPlayer*> GetPlayers();	
	set<CMonster*> GetMonsters();
};

