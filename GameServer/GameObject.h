#pragma once
#include "Map.h"
#include "Channel.h"
#include "CommonProtocol.h"
#include "GameObjectInfo.h"
#include "Math.h"

class CSkill;
class CRectCollision;

class CGameObject
{
public:
	int64 _StatusAbnormal;	

	int32 _ObjectManagerArrayIndex;
	int32 _ChannelArrayIndex;

	en_ObjectNetworkState _NetworkState;
	st_GameObjectInfo _GameObjectInfo;

	// 선택한 대상
	CGameObject* _SelectTarget;	

	//----------------------------
	// 채집하고 있는 대상
	//----------------------------
	CGameObject* _GatheringTarget;

	//---------------------------
	// 오브젝트가 스폰될 위치
	//---------------------------
	Vector2Int _SpawnPosition;

	// 시야 각
	float _FieldOfAngle;
	// 바라보는 방향
	Vector2 _FieldOfDirection;
	// 시야 거리
	float _FieldOfViewDistance;				

	// 플레이어 탐색 거리
	float _PlayerSearchDistance;

	//--------------------------------------------
	// SpawnIdle 상태에서 Idle 상태로 돌아갈 Tick
	//--------------------------------------------
	uint64 _SpawnIdleTick;	

	// 타겟
	CGameObject* _Owner;
	
	// 강화효과
	map<en_SkillType, CSkill*> _Bufs;
	// 약화효과
	map<en_SkillType, CSkill*> _DeBufs;

	// 게임오브젝트가 처리해야할 Job 구조체
	CLockFreeQue<st_GameObjectJob*> _GameObjectJobQue;

	CGameObject();
	CGameObject(st_GameObjectInfo GameObjectInfo);
	virtual ~CGameObject();

	// 밀려남 상태이상에 걸려 있는지 확인
	void PushedOutStatusAbnormalCheck();

	virtual void Update();
	virtual bool OnDamaged(CGameObject* Attacker, int32 DamagePoint);
	virtual void OnHeal(CGameObject* Healer, int32 HealPoint);	
	
	Vector2 PositionCheck(Vector2Int& CheckPosition);
	virtual void PositionReset();

	st_PositionInfo GetPositionInfo();		
	
	//------------------------------------------------------------------------------------
	// 내 주위 Distance안에 있는 위치들의 값을 반환한다.
	//------------------------------------------------------------------------------------
	vector<Vector2Int> GetAroundCellPositions(Vector2Int CellPosition, int8 Distance);

	void SetOwner(CGameObject* Target);
	CGameObject* GetTarget();

	//---------------------------------------------
	// 강화효과 스킬 추가 및 삭제
	//---------------------------------------------
	void AddBuf(CSkill* Buf);
	void DeleteBuf(en_SkillType DeleteBufSkillType);

	//-------------------------------------------------
	// 약화효과 스킬 추가 및 삭제
	//-------------------------------------------------
	void AddDebuf(CSkill* DeBuf);
	void DeleteDebuf(en_SkillType DeleteDebufSkillType);

	//--------------------------------------------------
	// 상태이상 값 설정 및 해제
	//--------------------------------------------------
	void SetStatusAbnormal(int64 StatusAbnormalValue);
	void ReleaseStatusAbnormal(int64 StatusAbnormalValue);
		
	//--------------------------------------------
	// 제어 할 수 없는 상태이상 체크
	//--------------------------------------------
	int64 CheckCantControlStatusAbnormal();
	//--------------------------------------------
	// 제어 할 수 있는 상태이상 체크
	//--------------------------------------------
	int64 CheckCanControlStatusAbnormal();

	//--------------------------------------------
	// 채널 가져오기 및 설정
	//--------------------------------------------
	CChannel* GetChannel();
	void SetChannel(CChannel* Channel);
		
	CRectCollision* GetRectCollision();	
	
	virtual void Init(en_GameObjectType GameObjectType);

	virtual void Start();
	virtual void End();

	vector<CGameObject*> GetFieldOfViewObjects();		

	CSkill* GetSkillCastingSkill();
	void SetSkillCastingSkill(CSkill* CastingSkill);	

	vector<st_FieldOfViewInfo> GetFieldOfViewInfo();
protected:			
	// 시야 범위 오브젝트
	vector<st_FieldOfViewInfo> _FieldOfViewInfos;

	CRectCollision* _RectCollision;

	map<int64, st_Aggro> _AggroTargetList;

	int64 _ReSpawnTime;

	// 게임오브젝트가 속한 채널	
	CChannel* _Channel;
	
	// 시전 중인 기술	
	CSkill* _CastingSkill;

	//-------------------------
	// 재생력 Tick
	//-------------------------
	uint64 _NatureRecoveryTick;

	//--------------------------------------
	// 공격 상태에서 기본공격을 실행할 Tick
	//--------------------------------------
	uint64 _DefaultAttackTick;
	//--------------------------------------
	// 마법 시전 상태에서 마법 시전 완료 Tick
	//--------------------------------------
	uint64 _SpellTick;
	//--------------------------------------
	// 채집 완료 Tick
	//--------------------------------------
	uint64 _GatheringTick;
	//--------------------------------------
	// 제작 Tick
	//--------------------------------------
	uint64 _CraftingTick;

	//---------------------------
	// 주위 시야 오브젝트 탐색 틱
	//---------------------------
	uint64 _FieldOfViewUpdateTick;

	//------------------------------------
	// 죽음 틱
	//------------------------------------
	uint64 _DeadTick;	

	//------------------------------------
	// 재소환 틱
	//------------------------------------
	uint64 _ReSpawnTick;	

	virtual void UpdateSpawnReady();
	virtual bool UpdateSpawnIdle();
	virtual void UpdateIdle();
	virtual void UpdatePatrol();
	virtual void UpdateMoving();
	virtual void UpdateReturnSpawnPosition();
	virtual void UpdateAttack();
	virtual void UpdateSpell();
	virtual void UpdateGathering();
	virtual void UpdateCrafting();	
	virtual void UpdateRooting();
	virtual void UpdateDead();	

	void CheckBufDeBufSkill();
	
private:	
};