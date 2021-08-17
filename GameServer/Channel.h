#pragma once
#include "Sector.h"
#include "Map.h"

class CMonster;

class CChannel
{
public:	
	int32 _ChannelId;	
	map<int64, CPlayer*> _Players;
	map<int64, CMonster*> _Monsters;
	
	CMap* _Map;

	CSector** _Sectors;
	
	int32 _SectorSize;
	int32 _SectorCountX;
	int32 _SectorCountY;

	CChannel() {};
	~CChannel();	

	void Init(int MapId, int SectorSize);

	//---------------------------------------------
	// 좌표 기준 섹터 얻어오기
	//---------------------------------------------
	CSector* GetSector(st_Vector2Int CellPosition);
	CSector* GetSector(int32 IndexY, int32 IndexX);

	//-----------------------------------------------------------------
	// 내 주위 섹터 반환
	//-----------------------------------------------------------------
	vector<CSector*> GetAroundSectors(st_Vector2Int CellPosition, int32 Range);
	vector<CGameObject*> GetAroundObjects(CGameObject* Object, int32 Range, bool ExceptMe = true);
	
	//--------------------------------------------------------------------------------------
	// 내 주위 플레이어 반환
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetAroundPlayer(CGameObject* Object, int32 Range, bool ExceptMe = true);
	CPlayer* FindNearPlayer(CGameObject* Object, int32 Range);

	void Update();

	//----------------------------------------------------
	// 채널 입장
	// - Object를 채널에 입장시키면서 자료구조에 저장한 후
	// - Map에도 해당 오브젝트의 위치를 기록한다.
	//----------------------------------------------------
	void EnterChannel(CGameObject* EnterChannelGameObject);
	//----------------------------------------------------
	// 채널 나가기
	// - Object를 채널에 퇴장시키면서 자료구조에 제거한 후
	// - Map에도 해당 오브젝트의 위치를 제거한다.
	//----------------------------------------------------
	void LeaveChannel(CGameObject* LeaveChannelGameObject);	
};

