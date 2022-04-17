#pragma once
#include "Sector.h"
#include "Map.h"
#include "LockFreeStack.h"

class CMonster;
class CItem;
class CEnvironment;
class CPlayer;

class CChannel
{
private:
	enum en_Channel
	{
		PLAYER_MAX = 800,
		MONSTER_MAX = 100,
		ITEM_MAX = 200,
		ENVIRONMENT_MAX = 100
	};

	//-------------------------------------------
	// 채널에서 관리중인 플레이어, 몬스터, 아이템
	//-------------------------------------------
	CPlayer* _ChannelPlayerArray[PLAYER_MAX];
	CMonster* _ChannelMonsterArray[MONSTER_MAX];
	CItem* _ChannelItemArray[ITEM_MAX];
	CEnvironment* _ChannelEnvironmentArray[ENVIRONMENT_MAX];

	CLockFreeStack<int32> _ChannelPlayerArrayIndexs;
	CLockFreeStack<int32> _ChannelMonsterArrayIndexs;
	CLockFreeStack<int32> _ChannelItemArrayIndexs;
	CLockFreeStack<int32> _ChannelEnvironmentArrayIndexs;

	SRWLOCK _ChannelLock;

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

	CChannel();
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
	// 오브젝트 시야 범위 안에 있는 오브젝트 아이디 목록 반환
	//-----------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewObjects(CGameObject* Object, int16 Range, bool ExceptMe = true);
	//----------------------------------------------------------------------------------------
	// 오브젝트 주위 몬스터 목록 반환
	//----------------------------------------------------------------------------------------
	vector<CMonster*> GetAroundMonster(CGameObject* Object, int16 Range, bool ExceptMe = true);
	//--------------------------------------------------------------------------------------
	// 내 주위 플레이어 반환
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetAroundPlayer(CGameObject* Object, int32 Range);
	//--------------------------------------------------------------------------------------
	// 내 시야 범위 플레이어 반환
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetFieldOfViewPlayer(CGameObject* Object, int16 Range, bool ExceptMe = true);

	//-------------------------------------------------------
	// 내 근처 플레이어 반환
	//-------------------------------------------------------
	CGameObject* FindNearPlayer(CGameObject* Object, int32 Range, bool* CollisionCango);

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

