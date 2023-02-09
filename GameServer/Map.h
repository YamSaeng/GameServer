#pragma once
#include "FileUtils.h"
#include "GameObjectInfo.h"

class CItem;
class CGameObject;
class CSector;
class CPlayer;
class CMonster;
class CChannelManager;

struct st_AStarNodeInt
{
	int32 _F;
	int32 _G;
	st_Vector2Int _Position;	

	st_AStarNodeInt() {}

	st_AStarNodeInt(int32 F, int32 G, int32 X, int32 Y)
	{
		_F = F;
		_G = G;
		_Position._X = X;
		_Position._Y = Y;
	}	
};

class CMap
{
public:
	int16 _MapID;

	wstring _MapName;

	int32 _Left;
	int32 _Right;
	int32 _Up;
	int32 _Down;

	int32 _SizeX;
	int32 _SizeY;

	//-----------------------------------------
	// �� ȯ�� ������Ʈ�� ������ ����
	//-----------------------------------------
	en_MapObjectInfo** _CollisionMapInfos;	

	//-------------------------------------
	// �� Ÿ�Ͽ� �����ϴ� ���� ������Ʈ ����
	//-------------------------------------
	CGameObject*** _ObjectsInfos;

	//-------------------------------------
	// �� Ÿ�Ͽ� �����ϴ� ���� ������Ʈ ����
	//-------------------------------------
	CGameObject*** _SeedObjectInfos;

	//-----------------------------------------------------------
	// �� Ÿ�Ͽ� �����ϴ� ������ ����
	// �� Ÿ�Ͽ� ���� �� �� �ִ� �������� ������ 20���� �����Ѵ�.
	//-----------------------------------------------------------
	CItem**** _Items;

	CMap();
	~CMap();

	void MapInit(int16 MapID, wstring MapName, int32 SectorSize, int8 ChannelCount);

	CSector* GetSector(st_Vector2Int CellPosition);
	CSector* GetSector(int32 IndexY, int32 IndexX);

	//-----------------------------------------------------------------
	// �� ���� ���� ��ȯ
	//-----------------------------------------------------------------
	vector<CSector*> GetAroundSectors(st_Vector2Int CellPosition, int32 Range = 1);
	//------------------------------------------------------------------------------------------------------
	// ������Ʈ �þ� ���� �ȿ� �ִ� ������Ʈ ���̵� ��� ��ȯ
	//------------------------------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewObjects(CGameObject* Object, bool ExceptMe = true, int16 Range = 1);
	//---------------------------------------------------------------------------------
	// ������Ʈ �þ� ���� �ȿ� �ִ� �÷��̾� ���̵� ��� ��ȯ
	//---------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewPlayers(CGameObject* Object, int16 Range, bool ExceptMe = true);
	//------------------------------------------------------------------------------------------------------------
	// ������Ʈ �þ� ���� �ȿ� �ִ� ���� ������ ������Ʈ ���̵� ��� ��ȯ
	//------------------------------------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewAttackObjects(CGameObject* Object, int16 Distance);
	//----------------------------------------------------------------------------------------
	// ������Ʈ ���� ���� ��� ��ȯ
	//----------------------------------------------------------------------------------------
	vector<CMonster*> GetAroundMonster(CGameObject* Object, int16 Range, bool ExceptMe = true);
	//--------------------------------------------------------------------------------------
	// �� �þ� ���� �÷��̾� ��ȯ
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetFieldOfViewPlayer(CGameObject* Object, int16 Range, bool ExceptMe = true);

	//-------------------------------------------------------
	// �� ��ó �÷��̾� ��ȯ
	//-------------------------------------------------------
	CGameObject* MonsterReqFindNearPlayer(CMonster* Monster, en_MonsterAggroType* AggroType, int32 Range, bool* CollisionCango);

	//-------------------------------------------
	// ��ǥ ��ġ�� �ִ� ������Ʈ ��ȯ
	//-------------------------------------------
	CGameObject* Find(st_Vector2Int& CellPosition);

	//--------------------------------------------------
	// ��ǥ ��ġ�� �ִ� �Ĺ� ������Ʈ ��ȯ
	//--------------------------------------------------
	CGameObject* FindPlant(st_Vector2Int& PlantPosition);

	//-------------------------------------------
	// ��ǥ ��ġ�� �ִ� �����۵��� ��ȯ
	//-------------------------------------------
	CItem** FindItem(st_Vector2Int& ItemCellPosition);

	//----------------------------------------------------------------------------
	// ���� �� ��ġ�� �� �� �ִ��� Ȯ�� ( ��ġ ��� �� MoveCollisionCango�� ���� )
	//----------------------------------------------------------------------------
	bool Cango(CGameObject* Object, OUT st_Vector2* NextPosition);
	//----------------------------------------------------------------------------
	// ��ġ�� �� �� �ִ��� Ȯ��
	// CheckObjects = ���� ������ ������Ʈ�� �浹 ������� ���� �������� ���� �Ǵ�
	// ( true : �ش���ġ�� ������Ʈ�� �ִ��� Ȯ���ؼ� ������ �浹ü�� �Ǵ��Ѵ�. )
	//----------------------------------------------------------------------------
	bool MoveCollisionCango(CGameObject* Object, st_Vector2Int& CellPosition, st_Vector2& NextPosition, bool CheckObjects = true);	

	//------------------------------------------------------------------------------------------------------------------------
	// ������ ��ǥ���� �޾Ƽ� �ش� ��ǥ�� �� �� �ִ��� ������ �Ǵ�
	// CheckObject = ���� ������ �ش� ��ġ�� �ִ� ������Ʈ�� �浹 ������� ���� �������� ���� ����
	// ApplyCollision = �ش� �Լ��� ȣ�� ���� ������Ʈ�� �浹 ������� ���� �������� ���� ����
	//------------------------------------------------------------------------------------------------------------------------
	bool ApplyMove(CGameObject* GameObject, st_Vector2Int& DestPosition, bool CheckObject = true, bool Applycollision = true);
	//---------------------------------------
	// �ʿ��� ������Ʈ ����
	//---------------------------------------
	bool ApplyLeave(CGameObject* GameObject);	

	vector<st_Vector2Int> FindPath(CGameObject* Object, st_Vector2Int StartCellPosition, st_Vector2Int DestCellPostion, bool CheckObjects = true, int32 MaxDistance = 8);	
	bool FindPathNextPositionCango(CGameObject* Object, st_Vector2Int& CellPosition, bool CheckObjects = true);

	CChannelManager* GetChannelManager();
private:
	CChannelManager* _ChannelManager;
	CSector** _Sectors;
	int32 _SectorSize;

	int32 _SectorCountX;
	int32 _SectorCountY;
};