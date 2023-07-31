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
	// 맵 타일에 존재하는 게임 오브젝트 정보
	//-------------------------------------
	CGameObject*** _ObjectsInfos;

	//-------------------------------------
	// 맵 타일에 존재하는 씨앗 오브젝트 정보
	//-------------------------------------
	CGameObject*** _SeedObjectInfos;

	//-----------------------------------------------------------
	// 맵 타일에 존재하는 아이템 정보
	// 한 타일에 존재 할 수 있는 아이템의 종류는 20개로 제한한다.
	//-----------------------------------------------------------
	CItem**** _Items;

	CMap();
	~CMap();

	void MapInit();

	CSector* GetSector(Vector2Int CellPosition);
	CSector* GetSector(int32 IndexY, int32 IndexX);

	//-----------------------------------------------------------------
	// 내 주위 섹터 반환
	//-----------------------------------------------------------------
	vector<CSector*> GetAroundSectors(Vector2Int CellPosition, int32 Range = 1);
	//------------------------------------------------------------------------------------------------------
	// 오브젝트 시야 범위 안에 있는 모든 오브젝트 아이디 목록을 반환
	//------------------------------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewObjects(CPlayer* Object);
	//---------------------------------------------------------------------------------
	// 오브젝트를 기준으로 일정 거리 안에 있는 플레이어 아이디 목록을 반환
	//---------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldAroundPlayers(CGameObject* Object, bool ExceptMe = true);
	//--------------------------------------------------------------------------------------
	// 내 시야 범위 플레이어 반환
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetAroundPlayers(CGameObject* Object, bool ExceptMe = true);	
	//----------------------------------------------------------------------------------------
	// 오브젝트 주위 몬스터 목록 반환
	//----------------------------------------------------------------------------------------
	vector<CMonster*> GetAroundMonster(CGameObject* Object, int16 Range, bool ExceptMe = true);	

	//-------------------------------------------------------
	// 내 근처 플레이어 반환
	//-------------------------------------------------------
	CGameObject* MonsterReqFindNearPlayer(CMonster* Monster, en_MonsterAggroType* AggroType, bool* CollisionCango);

	//-------------------------------------------
	// 좌표 위치에 있는 오브젝트 반환
	//-------------------------------------------
	CGameObject* Find(Vector2Int& CellPosition);

	//--------------------------------------------------
	// 좌표 위치에 있는 식물 오브젝트 반환
	//--------------------------------------------------
	CGameObject* FindPlant(Vector2Int& PlantPosition);

	//-------------------------------------------
	// 좌표 위치에 있는 아이템들을 반환
	//-------------------------------------------
	CItem** FindItem(Vector2Int& ItemCellPosition);

	//----------------------------------------------------------------------------
	// 다음 번 위치로 갈 수 있는지 확인 ( 위치 계산 후 MoveCollisionCango로 전달 )
	//----------------------------------------------------------------------------
	bool Cango(CGameObject* Object);
	//----------------------------------------------------------------------------
	// 위치로 갈 수 있는지 확인
	// CheckObjects = 벽을 제외한 오브젝트를 충돌 대상으로 여길 것인지에 대한 판단
	// ( true : 해당위치에 오브젝트가 있는지 확인해서 있으면 충돌체로 판단한다. )
	//----------------------------------------------------------------------------
	bool MoveCollisionCango(CGameObject* Object, Vector2Int& CellPosition, CRectCollision* CheckRectCollsion = nullptr, bool CheckObjects = true, CGameObject* CollisionObject = nullptr);

	//------------------------------------------------------------------------------------------------------------------------
	// 목적지 좌표값을 받아서 해당 좌표로 갈 수 있는지 없는지 판단
	// CheckObject = 벽을 제외한 해당 위치에 있는 오브젝트를 충돌 대상으로 여길 것인지에 대한 여부
	// ApplyCollision = 해당 함수를 호출 해준 오브젝트를 충돌 대상으로 여길 것인지에 대한 여부
	//------------------------------------------------------------------------------------------------------------------------
	bool ApplyMove(CGameObject* GameObject, Vector2Int& DestPosition, bool CheckObject = true, bool Applycollision = true);
	//---------------------------------------
	// 맵에서 오브젝트 퇴장
	//---------------------------------------
	bool ApplyLeave(CGameObject* GameObject);		

	//-------------------------------------------------------------------------------------------------------------------
	// SkillObject가 다음 번 위치로 갈 수 있는지 확인 ( 충돌할 경우 충돌한 대상 가져옴 )
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