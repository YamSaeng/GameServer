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
class CRectCollision;
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
	// 채널에 있는 오브젝트들 찾기 ( 시야 범위 안에서 )
	//------------------------------------------------------------------------------
	vector<CGameObject*> FindChannelObjects(vector<st_FieldOfViewInfo>& FindObjectIDs);
	//-----------------------------------------------------
	// 채널에 있는 공격 가능한 오브젝트들 찾기 ( 거리 기준 )
	//-----------------------------------------------------
	vector<CGameObject*> FindAttackChannelObjects(vector<st_FieldOfViewInfo>& FindObjectIDs, CGameObject* Object, Vector2 Direction, int16 Distance);
	//---------------------------------------------------------
	// 채널에 있는 공격 가능한 오브젝트 찾기 ( 광역 범위 )
	//---------------------------------------------------------
	vector<CGameObject*> FindRangeAttackChannelObjects(CGameObject* Object, Vector2 Direciton, int16 Distance);
	
	//-----------------------------------------------------
	// 채널에 있는 오브젝트들과 충돌하는지 판단
	//-----------------------------------------------------	
	bool ChannelColliderCheck(CGameObject* CheckObject, Vector2 CheckPosition);
	// 충돌 대상을 하나 반환
	bool ChannelColliderSingleCheck(CGameObject* CheckObject, CRectCollision* CheckRectCollision, CGameObject** CollisionObject = nullptr);
	// 충돌한 모든 대상을 반환
	bool ChannelColliderMultipleCheck(CGameObject* CheckObject, Vector2 CheckPosition, vector<CGameObject*>* CollisionObjects);
	// OBB 알고리즘로 충돌대상 확인
	bool ChannelColliderOBBCheck(CGameObject* CheckObject, int64 ExceptionID = 0, CGameObject** CollisionObject = nullptr);
	// 매개변수로 받는 AorundObjects에서 충돌하는 대상이 있는지 확인
	bool ChannelColliderOBBCheckAroundObject(CRectCollision* CheckCollision,  vector<CGameObject*> AroundObjects, vector<CGameObject*>& CollisionObjects, int64 ExceptionObjectID);

	//----------------------------------------------------
	// 채널 입장
	// - Object를 채널에 입장시키면서 자료구조에 저장한 후
	// - Map에도 해당 오브젝트의 위치를 기록한다.
	//----------------------------------------------------
	bool EnterChannel(CGameObject* EnterChannelGameObject, Vector2Int* ObjectSpawnPosition = nullptr);
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
		CHANNEL_PLAYER_MAX = 100,
		CHANNEL_DUMMY_PLAYER_MAX = 500,
		CHANNEL_NON_PLAYER_MAX = 50,
		CHANNEL_MONSTER_MAX = 100,
		CHANNEL_ENVIRONMENT_MAX = 300,
		CHANNEL_CRAFTING_TABLE_MAX = 100,
		CHANNEL_SKILL_OBJECT_MAX = 100,
		CHANNEL_CROP_MAX = 100,
		CHANNEL_ITEM_MAX = 200
	};
	
	// 채널에서 관리중인 PC, NPC, Monster, Item		
	CPlayer* _ChannelPlayerArray[CHANNEL_PLAYER_MAX];
	CPlayer* _ChannelDummyPlayerArray[CHANNEL_DUMMY_PLAYER_MAX];
	CNonPlayer* _ChannelNonPlayerArray[CHANNEL_NON_PLAYER_MAX];
	CMonster* _ChannelMonsterArray[CHANNEL_MONSTER_MAX];
	CEnvironment* _ChannelEnvironmentArray[CHANNEL_ENVIRONMENT_MAX];
	CCraftingTable* _ChannelCraftingTableArray[CHANNEL_CRAFTING_TABLE_MAX];
	CGameObject* _ChannelSkillObjectArray[CHANNEL_SKILL_OBJECT_MAX];
	CCrop* _ChannelCropArray[CHANNEL_CROP_MAX];
	CItem* _ChannelItemArray[CHANNEL_ITEM_MAX];	

	CLockFreeStack<int32> _ChannelPlayerArrayIndexs;
	CLockFreeStack<int32> _ChannelDummyPlayerArrayIndexs;
	CLockFreeStack<int32> _ChannelNonPlayerArrayIndexs;
	CLockFreeStack<int32> _ChannelMonsterArrayIndexs;
	CLockFreeStack<int32> _ChannelEnvironmentArrayIndexs;
	CLockFreeStack<int32> _ChannelCraftingTableArrayIndexs;
	CLockFreeStack<int32> _ChannelSkillObjectArrayIndexs;
	CLockFreeStack<int32> _ChannelCropArrayIndexs;
	CLockFreeStack<int32> _ChannelItemArrayIndexs;	
	
	// 채널에 할당된 Map
	CMap* _Map;
};