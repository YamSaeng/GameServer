#pragma once
#include "Sector.h"
#include "Map.h"

class CChannel
{
public:	
	int32 _ChannelId;	
	map<int64, CPlayer*> _Players;
	
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
	vector<CSector*> GetAroundSectors(CGameObject* Object, int32 Range);
	vector<CGameObject*> GetAroundObjects(CGameObject* Object, int32 Range);
	
	//--------------------------------------------------------------------------------------
	// 내 주위 플레이어 반환
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetAroundPlayer(CGameObject* Object, int32 Range, bool ExceptMe = true);

	void Update();

	//----------------------------------------------------
	// 채널 입장
	//----------------------------------------------------
	void EnterChannel(CGameObject* EnterChannelGameObject);
	//----------------------------------------------------
	// 채널 나가기
	//----------------------------------------------------
	void LeaveChannel(CGameObject* LeaveChannelGameObject);	
};

