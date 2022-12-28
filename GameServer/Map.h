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

	//-----------------------------------------
	// ���� Ÿ�� ������ ���� ( �Ҵ�Ǿ����� �ƴ��� )
	//-----------------------------------------
	st_TileMapInfo** _TileMapInfos;

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
	vector<CSector*> GetAroundSectors(st_Vector2Int CellPosition, int32 Range);	
	//------------------------------------------------------------------------------------------------------
	// ������Ʈ �þ� ���� �ȿ� �ִ� ������Ʈ ���̵� ��� ��ȯ
	//------------------------------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewObjects(CGameObject* Object, int16 Range, bool ExceptMe = true);
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

	//----------------------------------------------------------------------
	// ��û�� ������Ʈ �þ� ���� �ȿ� �ִ� Ÿ�� ���� ��ȯ
	//----------------------------------------------------------------------
	vector<st_TileMapInfo> FindMapTileInfo(CGameObject* Player);
		
	bool Cango(CGameObject* Object, float X, float Y);
	//----------------------------------------------------------------------------
	// ��ġ�� �� �� �ִ��� Ȯ��
	// CheckObjects = ���� ������ ������Ʈ�� �浹 ������� ���� �������� ���� �Ǵ�
	// ( true : �ش���ġ�� ������Ʈ�� �ִ��� Ȯ���ؼ� ������ �浹ü�� �Ǵ��Ѵ�. )
	//----------------------------------------------------------------------------
	bool CollisionCango(CGameObject* Object, st_Vector2Int& CellPosition, bool CheckObjects = true);	

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

	//-----------------------------------------------------------------------------------------------
	// ����ϰ��� �ϴ� �������� ��ǥ���� �޾Ƽ� ���
	//-----------------------------------------------------------------------------------------------
	bool ApplyTileUserAlloc(CGameObject* ReqTileUserAllocObject, st_Vector2Int TileUserAllocPosition);
	//-----------------------------------------------------------------------------------------------
	// �����ϰ��� �ϴ� �������� ��ǥ���� �޾Ƽ� ����
	//-----------------------------------------------------------------------------------------------
	bool ApplyTileUseFree(CGameObject* ReqTileUserFreeObject, st_Vector2Int TileUserFreePosition);	

	vector<st_Vector2Int> FindPath(CGameObject* Object, st_Vector2Int StartCellPosition, st_Vector2Int DestCellPostion, bool CheckObjects = true, int32 MaxDistance = 10);
	vector<st_Vector2Int> CompletePath(map<st_Vector2Int, st_Vector2Int> Parents, st_Vector2Int DestPosition);

	CChannelManager* GetChannelManager();
private:
	CChannelManager* _ChannelManager;
	CSector** _Sectors;
	int32 _SectorSize;

	int32 _SectorCountX;
	int32 _SectorCountY;
};