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
	// 맵 환경 오브젝트의 정보를 보관
	//-----------------------------------------
	en_MapObjectInfo** _CollisionMapInfos;

	//-----------------------------------------
	// 맵의 타일 정보를 보관 ( 할당되었는지 아닌지 )
	//-----------------------------------------
	st_TileMapInfo** _TileMapInfos;

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

	void MapInit(int16 MapID, wstring MapName, int32 SectorSize, int8 ChannelCount);

	CSector* GetSector(st_Vector2Int CellPosition);
	CSector* GetSector(int32 IndexY, int32 IndexX);

	//-----------------------------------------------------------------
	// 내 주위 섹터 반환
	//-----------------------------------------------------------------
	vector<CSector*> GetAroundSectors(st_Vector2Int CellPosition, int32 Range);	
	//------------------------------------------------------------------------------------------------------
	// 오브젝트 시야 범위 안에 있는 오브젝트 아이디 목록 반환
	//------------------------------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewObjects(CGameObject* Object, int16 Range, bool ExceptMe = true);
	//---------------------------------------------------------------------------------
	// 오브젝트 시야 범위 안에 있는 플레이어 아이디 목록 반환
	//---------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewPlayers(CGameObject* Object, int16 Range, bool ExceptMe = true);	
	//------------------------------------------------------------------------------------------------------------
	// 오브젝트 시야 범위 안에 있는 공격 가능한 오브젝트 아이디 목록 반환
	//------------------------------------------------------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewAttackObjects(CGameObject* Object, int16 Distance);
	//----------------------------------------------------------------------------------------
	// 오브젝트 주위 몬스터 목록 반환
	//----------------------------------------------------------------------------------------
	vector<CMonster*> GetAroundMonster(CGameObject* Object, int16 Range, bool ExceptMe = true);	
	//--------------------------------------------------------------------------------------
	// 내 시야 범위 플레이어 반환
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetFieldOfViewPlayer(CGameObject* Object, int16 Range, bool ExceptMe = true);

	//-------------------------------------------------------
	// 내 근처 플레이어 반환
	//-------------------------------------------------------
	CGameObject* MonsterReqFindNearPlayer(CMonster* Monster, en_MonsterAggroType* AggroType, int32 Range, bool* CollisionCango);

	//-------------------------------------------
	// 좌표 위치에 있는 오브젝트 반환
	//-------------------------------------------
	CGameObject* Find(st_Vector2Int& CellPosition);

	//--------------------------------------------------
	// 좌표 위치에 있는 식물 오브젝트 반환
	//--------------------------------------------------
	CGameObject* FindPlant(st_Vector2Int& PlantPosition);

	//-------------------------------------------
	// 좌표 위치에 있는 아이템들을 반환
	//-------------------------------------------
	CItem** FindItem(st_Vector2Int& ItemCellPosition);

	//----------------------------------------------------------------------
	// 요청한 오브젝트 시야 범위 안에 있는 타일 정보 반환
	//----------------------------------------------------------------------
	vector<st_TileMapInfo> FindMapTileInfo(CGameObject* Player);
		
	bool Cango(CGameObject* Object, float X, float Y);
	//----------------------------------------------------------------------------
	// 위치로 갈 수 있는지 확인
	// CheckObjects = 벽을 제외한 오브젝트를 충돌 대상으로 여길 것인지에 대한 판단
	// ( true : 해당위치에 오브젝트가 있는지 확인해서 있으면 충돌체로 판단한다. )
	//----------------------------------------------------------------------------
	bool CollisionCango(CGameObject* Object, st_Vector2Int& CellPosition, bool CheckObjects = true);	

	//------------------------------------------------------------------------------------------------------------------------
	// 목적지 좌표값을 받아서 해당 좌표로 갈 수 있는지 없는지 판단
	// CheckObject = 벽을 제외한 해당 위치에 있는 오브젝트를 충돌 대상으로 여길 것인지에 대한 여부
	// ApplyCollision = 해당 함수를 호출 해준 오브젝트를 충돌 대상으로 여길 것인지에 대한 여부
	//------------------------------------------------------------------------------------------------------------------------
	bool ApplyMove(CGameObject* GameObject, st_Vector2Int& DestPosition, bool CheckObject = true, bool Applycollision = true);
	//---------------------------------------
	// 맵에서 오브젝트 퇴장
	//---------------------------------------
	bool ApplyLeave(CGameObject* GameObject);	

	//-----------------------------------------------------------------------------------------------
	// 등록하고자 하는 사유지의 좌표값을 받아서 등록
	//-----------------------------------------------------------------------------------------------
	bool ApplyTileUserAlloc(CGameObject* ReqTileUserAllocObject, st_Vector2Int TileUserAllocPosition);
	//-----------------------------------------------------------------------------------------------
	// 해제하고자 하는 사유지의 좌표값을 받아서 해제
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