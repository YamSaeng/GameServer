#pragma once
#include "FileUtils.h"
#include "GameObjectInfo.h"

class CItem;
class CGameObject;
class CSector;
class CPlayer;
class CMonster;
class CChannelManager;
class CRectCollision;

struct st_AStarNodeInt
{
	int32 _F;
	int32 _G;
	Vector2Int _Position;	

	st_AStarNodeInt() {}

	st_AStarNodeInt(int32 F, int32 G, int32 X, int32 Y)
	{
		_F = F;
		_G = G;
		_Position.X = X;
		_Position.Y = Y;
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

	int32 _SectorSize;

	int8 _ChannelCount;	

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

	void MapInit();

	CSector* GetSector(Vector2Int CellPosition);
	CSector* GetSector(int32 IndexY, int32 IndexX);

	//-----------------------------------------------------------------
	// �� ���� ���� ��ȯ
	//-----------------------------------------------------------------
	vector<CSector*> GetAroundSectors(Vector2Int CellPosition, int32 Range = 1);
	//------------------------------------------------------------------------------------------------------
	// ������Ʈ �þ� ���� �ȿ� �ִ� ��� ������Ʈ ���̵� ����� ��ȯ
	//------------------------------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewObjects(CPlayer* Object);
	//---------------------------------------------------------------------------------
	// ������Ʈ�� �������� ���� �Ÿ� �ȿ� �ִ� �÷��̾� ���̵� ����� ��ȯ
	//---------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldAroundPlayers(CGameObject* Object, bool ExceptMe = true);
	//--------------------------------------------------------------------------------------
	// �� �þ� ���� �÷��̾� ��ȯ
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetAroundPlayers(CGameObject* Object, bool ExceptMe = true);	
	//----------------------------------------------------------------------------------------
	// ������Ʈ ���� ���� ��� ��ȯ
	//----------------------------------------------------------------------------------------
	vector<CMonster*> GetAroundMonster(CGameObject* Object, int16 Range, bool ExceptMe = true);	

	//-------------------------------------------------------
	// �� ��ó �÷��̾� ��ȯ
	//-------------------------------------------------------
	CGameObject* MonsterReqFindNearPlayer(CMonster* Monster, en_MonsterAggroType* AggroType, bool* CollisionCango);

	//-------------------------------------------
	// ��ǥ ��ġ�� �ִ� ������Ʈ ��ȯ
	//-------------------------------------------
	CGameObject* Find(Vector2Int& CellPosition);

	//--------------------------------------------------
	// ��ǥ ��ġ�� �ִ� �Ĺ� ������Ʈ ��ȯ
	//--------------------------------------------------
	CGameObject* FindPlant(Vector2Int& PlantPosition);

	//-------------------------------------------
	// ��ǥ ��ġ�� �ִ� �����۵��� ��ȯ
	//-------------------------------------------
	CItem** FindItem(Vector2Int& ItemCellPosition);

	//----------------------------------------------------------------------------
	// ���� �� ��ġ�� �� �� �ִ��� Ȯ�� ( ��ġ ��� �� MoveCollisionCango�� ���� )
	//----------------------------------------------------------------------------
	bool Cango(CGameObject* Object);
	//----------------------------------------------------------------------------
	// ��ġ�� �� �� �ִ��� Ȯ��
	// CheckObjects = ���� ������ ������Ʈ�� �浹 ������� ���� �������� ���� �Ǵ�
	// ( true : �ش���ġ�� ������Ʈ�� �ִ��� Ȯ���ؼ� ������ �浹ü�� �Ǵ��Ѵ�. )
	//----------------------------------------------------------------------------
	bool MoveCollisionCango(CGameObject* Object, Vector2Int& CellPosition, CRectCollision* CheckRectCollsion = nullptr, bool CheckObjects = true, CGameObject* CollisionObject = nullptr);

	//------------------------------------------------------------------------------------------------------------------------
	// ������ ��ǥ���� �޾Ƽ� �ش� ��ǥ�� �� �� �ִ��� ������ �Ǵ�
	// CheckObject = ���� ������ �ش� ��ġ�� �ִ� ������Ʈ�� �浹 ������� ���� �������� ���� ����
	// ApplyCollision = �ش� �Լ��� ȣ�� ���� ������Ʈ�� �浹 ������� ���� �������� ���� ����
	//------------------------------------------------------------------------------------------------------------------------
	bool ApplyMove(CGameObject* GameObject, Vector2Int& DestPosition, bool CheckObject = true, bool Applycollision = true);
	//---------------------------------------
	// �ʿ��� ������Ʈ ����
	//---------------------------------------
	bool ApplyLeave(CGameObject* GameObject);		

	//-------------------------------------------------------------------------------------------------------------------
	// SkillObject�� ���� �� ��ġ�� �� �� �ִ��� Ȯ�� ( �浹�� ��� �浹�� ��� ������ )
	//-------------------------------------------------------------------------------------------------------------------
	bool CanMoveSkillGo(CGameObject* SkillObject, OUT Vector2* NextPosition, int64 ExceptionID = 0, OUT CGameObject** CollisionObject = nullptr);
	bool ApplySkillObjectMove(CGameObject* SkillObject, Vector2Int& DestPosition, int64 ExceptionID = 0, CGameObject** CollisionObject = nullptr);

	bool MonsterCango(CGameObject* Object);

	vector<Vector2Int> FindPath(CGameObject* Object, Vector2Int StartCellPosition, Vector2Int DestCellPostion, bool CheckObjects = true, int32 MaxDistance = 8);		
	bool FindPathNextPositionCango(CGameObject* Object, Vector2Int& NextPosition, bool CheckObjects = true);

	CChannelManager* GetChannelManager();

	Vector2 GetMovePositionNearTarget(CGameObject* Object, CGameObject* Target);
private:
	CChannelManager* _ChannelManager;
	CSector** _Sectors;	

	int32 _SectorCountX;
	int32 _SectorCountY;
};