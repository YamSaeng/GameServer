#pragma once
#include "Sector.h"
#include "Map.h"

class CMonster;
class CItem;
class CEnvironment;

class CChannel
{
private:
	//-------------------------------------------
	// 채널에서 관리중인 플레이어, 몬스터, 아이템
	//-------------------------------------------
	map<int64, CPlayer*> _Players;
	map<int64, CMonster*> _Monsters;
	map<int64, CItem*> _Items;
	map<int64, CEnvironment*> _Environments;

	//-----------------
	// 섹터 목록
	//-----------------
	CSector** _Sectors;
public:	
	int32 _ChannelId;		
	
	CMap* _Map;	
	
	//----------------
	// 섹터 크기
	//----------------
	int32 _SectorSize;

	//------------------
	// 섹터 X, Y 개수
	//------------------
	int32 _SectorCountX;
	int32 _SectorCountY;

	CChannel() {};
	~CChannel();	

	//---------------------------------------
	// 채널 초기화
	//---------------------------------------
	void Init(int32 MapId, int32 SectorSize);	

	//---------------------------------------------
	// 좌표 기준 섹터 얻어오기
	//---------------------------------------------
	CSector* GetSector(st_Vector2Int CellPosition);
	CSector* GetSector(int32 IndexY, int32 IndexX);

	//-----------------------------------------------------------------
	// 내 주위 섹터 반환
	//-----------------------------------------------------------------
	vector<CSector*> GetAroundSectors(st_Vector2Int CellPosition, int32 Range);
	//-----------------------------------------------------------------
	// 내 주위 섹터 안에 있는 오브젝트 반환
	//-----------------------------------------------------------------
	vector<CGameObject*> GetAroundSectorObjects(CGameObject* Object, int32 Range, bool ExceptMe = true);
	//-----------------------------------------------------------------
	// 내 시야 범위 안에 있는 오브젝트 반환
	//-----------------------------------------------------------------
	vector<CGameObject*> GetFieldOfViewObjects(CPlayer* MyPlayer, int16 Range, bool ExceptMe = true);
	
	//--------------------------------------------------------------------------------------
	// 내 주위 플레이어 반환
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetAroundPlayer(CGameObject* Object, int32 Range, bool ExceptMe = true);

	//-------------------------------------------------------
	// 내 근처 플레이어 반환
	//-------------------------------------------------------
	CGameObject* FindNearPlayer(CGameObject* Object, int32 Range, bool* Cango);

	//------------------------------
	// 소유하고 있는 몬스터 업데이트
	//------------------------------
	void Update();
	
	//----------------------------------------------------
	// 채널 입장
	// - Object를 채널에 입장시키면서 자료구조에 저장한 후
	// - Map에도 해당 오브젝트의 위치를 기록한다.
	//----------------------------------------------------
	bool EnterChannel(CGameObject* EnterChannelGameObject,st_Vector2Int* ObjectSpawnPosition = nullptr);
	//----------------------------------------------------
	// 채널 나가기
	// - Object를 채널에 퇴장시키면서 자료구조에 제거한 후
	// - Map에도 해당 오브젝트의 위치를 제거한다.
	//----------------------------------------------------
	void LeaveChannel(CGameObject* LeaveChannelGameObject);	
};

