#pragma once

class CGameObject;
class CPlayer;
class CMonster;
class CItem;
class CEnvironment;

class CSector
{
private:
	// 섹터에서 관리하는 플레이어, 몬스터, 아이템 
	set<CPlayer*> _Players;
	set<CMonster*> _Monsters;
	set<CItem*> _Items;
	set<CEnvironment*> _Environment;

	SRWLOCK _SectorLock;	
public:
	// 섹터 좌표
	int32 _SectorY;
	int32 _SectorX;	

	CSector() {};
	CSector(int32 SectorY,int32 SectorX);

	// 섹터에 오브젝트 넣기
	void Insert(CGameObject* InsertGameObject);
	// 섹터에 오브젝트 빼기 
	void Remove(CGameObject* RemoveGameObject);
	
	set<CPlayer*> GetPlayers();	
	set<CMonster*> GetMonsters();
	set<CItem*> GetItems();
	set<CEnvironment*> GetEnvironment();

	void GetSectorLock();
	void GetSectorUnLock();
};

