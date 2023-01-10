#pragma once
#include "Sector.h"
#include "LockFreeQue.h"
#include "LockFreeStack.h"
#include "GameObjectInfo.h"

class CMonster;
class CItem;
class CEnvironment;
class CPlayer;
class CNonPlayer;
class CMap;
struct st_Vector2Int;
struct st_GameObjectJob;

class CChannel
{
public:
	// 채널 ID
	int32 _ChannelId;
	// 채널이 처리해야할 Job 구조체
	CLockFreeQue<st_GameObjectJob*> _ChannelJobQue;

	CChannel();
	~CChannel();

	//---------------------------------------
	// 채널 초기화
	//---------------------------------------
	void Init();

	//------------------------------
	// 소유하고 있는 몬스터 업데이트
	//------------------------------
	void Update();

	//----------------------------
	// 채널에 맵 할당
	//----------------------------
	void SetMap(CMap* Map);
	//----------------------------
	// 채널에 할당된 맵 가져오기
	//----------------------------
	CMap* GetMap();	

	//------------------------------------------------------------------------------
	// 채널에 있는 오브젝트 찾기 ( 단일 )
	//------------------------------------------------------------------------------
	CGameObject* FindChannelObject(int64 ObjectID, en_GameObjectType GameObjectType);
	//------------------------------------------------------------------------------
	// 채널에 있는 오브젝트 찾기 ( 복수 )
	//------------------------------------------------------------------------------
	vector<CGameObject*> FindChannelObjects(en_GameObjectType GameObjectType);	
	//------------------------------------------------------------------------------
	// 채널에 있는 오브젝트들 찾기
	//------------------------------------------------------------------------------
	vector<CGameObject*> FindChannelObjects(vector<st_FieldOfViewInfo>& FindObjectIDs);
	//-----------------------------------------------------
	// 채널에 있는 공격 가능한 오브젝트들 찾기 ( 거리 기준 )
	//-----------------------------------------------------
	vector<CGameObject*> FindAttackChannelObjects(vector<st_FieldOfViewInfo>& FindObjectIDs, CGameObject* Object, int16 Distance);	
	//---------------------------------------------------------
	// 채널에 있는 공격 가능한 오브젝트 찾기 ( 광역 범위 )
	//---------------------------------------------------------
	vector<CGameObject*> FindRangeAttackChannelObjects(CGameObject* Object, int16 Distance);

	//-----------------------------------------------------
	// 채널에 있는 오브젝트들과 검사해서 충돌 판단
	//-----------------------------------------------------
	bool ChannelColliderCheck(CGameObject* Object);

	//----------------------------------------------------
	// 채널 입장
	// - Object를 채널에 입장시키면서 자료구조에 저장한 후
	// - Map에도 해당 오브젝트의 위치를 기록한다.
	//----------------------------------------------------
	bool EnterChannel(CGameObject* EnterChannelGameObject, st_Vector2Int* ObjectSpawnPosition = nullptr);
	//----------------------------------------------------
	// 채널 나가기
	// - Object가 채널에 속해 있는 Index를 반납
	//----------------------------------------------------
	void LeaveChannel(CGameObject* LeaveChannelGameObject);	

	//-------------------------------------------------------------------------
	// 경험치 계산
	//-------------------------------------------------------------------------
	void ExperienceCalculate(CPlayer* TargetPlayer, en_GameObjectType TargetMonsterObjectType, int32 ExperiencePoint);
private:
	enum en_Channel
	{
		PLAYER_MAX = 100,
		DUMMY_PLAYER_MAX = 500,
		NON_PLAYER_MAX = 50,
		MONSTER_MAX = 100,
		ENVIRONMENT_MAX = 100,
		CRAFTING_TABLE_MAX = 100,
		CROP_MAX = 100,
		ITEM_MAX = 200
	};
	
	// 채널에서 관리중인 PC, NPC, Monster, Item	
	CPlayer* _ChannelPlayerArray[PLAYER_MAX];
	CPlayer* _ChannelDummyPlayerArray[DUMMY_PLAYER_MAX];
	CNonPlayer* _ChannelNonPlayerArray[NON_PLAYER_MAX];
	CMonster* _ChannelMonsterArray[MONSTER_MAX];
	CEnvironment* _ChannelEnvironmentArray[ENVIRONMENT_MAX];
	CCraftingTable* _ChannelCraftingTableArray[CRAFTING_TABLE_MAX];
	CCrop* _ChannelCropArray[CROP_MAX];
	CItem* _ChannelItemArray[ITEM_MAX];

	CLockFreeStack<int32> _ChannelPlayerArrayIndexs;
	CLockFreeStack<int32> _ChannelDummyPlayerArrayIndexs;
	CLockFreeStack<int32> _ChannelNonPlayerArrayIndexs;
	CLockFreeStack<int32> _ChannelMonsterArrayIndexs;
	CLockFreeStack<int32> _ChannelEnvironmentArrayIndexs;
	CLockFreeStack<int32> _ChannelCraftingTableArrayIndexs;
	CLockFreeStack<int32> _ChannelCropArrayIndexs;
	CLockFreeStack<int32> _ChannelItemArrayIndexs;	
	
	// 채널에 할당된 Map
	CMap* _Map;
};