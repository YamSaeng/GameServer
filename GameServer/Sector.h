#pragma once

class CGameObject;
class CPlayer;
class CMonster;
class CItem;
class CEnvironment;

class CSector
{
private:
	// ���Ϳ��� �����ϴ� �÷��̾�, ����, ������ 
	set<CPlayer*> _Players;
	set<CMonster*> _Monsters;
	set<CItem*> _Items;
	set<CEnvironment*> _Environment;

	SRWLOCK _SectorLock;	
public:
	// ���� ��ǥ
	int32 _SectorY;
	int32 _SectorX;	

	CSector() {};
	CSector(int32 SectorY,int32 SectorX);

	// ���Ϳ� ������Ʈ �ֱ�
	void Insert(CGameObject* InsertGameObject);
	// ���Ϳ� ������Ʈ ���� 
	void Remove(CGameObject* RemoveGameObject);
	
	set<CPlayer*> GetPlayers();	
	set<CMonster*> GetMonsters();
	set<CItem*> GetItems();
	set<CEnvironment*> GetEnvironment();

	void GetSectorLock();
	void GetSectorUnLock();
};

