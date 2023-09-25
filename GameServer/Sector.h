#pragma once

class CGameObject;
class CPlayer;
class CNonPlayer;
class CMonster;
class CItem;
class CEnvironment;
class CCraftingTable;
class CCrop;

class CSector
{
private:
	// ���Ϳ��� �����ϴ� �÷��̾�, ����, ������ 
	set<CPlayer*> _Players;
	set<CNonPlayer*> _NonPlayers;
	set<CMonster*> _Monsters;
	set<CItem*> _Items;
	set<CEnvironment*> _Environment;	
	set<CGameObject*> _Buildings;
	set<CGameObject*> _SkillObjects;
	set<CCraftingTable*> _CraftingTables;
	set<CCrop*> _Crops;

	SRWLOCK _SectorLock;
public:
	// ���� ��ǥ
	int32 _SectorY;
	int32 _SectorX;

	CSector() {};
	CSector(int32 SectorY, int32 SectorX);

	// ���Ϳ� ������Ʈ �ֱ�
	void Insert(CGameObject* InsertGameObject);
	// ���Ϳ� ������Ʈ ���� 
	void Remove(CGameObject* RemoveGameObject);

	set<CPlayer*> GetPlayers();
	set<CNonPlayer*> GetNonPlayers();
	set<CMonster*> GetMonsters();
	set<CItem*> GetItems();
	set<CEnvironment*> GetEnvironment();
	set<CGameObject*> GetBuildings();
	set<CGameObject*> GetSkillObject();
	set<CCraftingTable*> GetCraftingTable();
	set<CCrop*> GetCrop();

	void AcquireSectorLock();
	void ReleaseSectorLock();
};