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
	int32 _StatusAbnormal;	

	int32 _ObjectManagerArrayIndex;
	int32 _ChannelArrayIndex;

	en_ObjectNetworkState _NetworkState;
	st_GameObjectInfo _GameObjectInfo;

	// ������ ���
	CGameObject* _SelectTarget;	

	//----------------------------
	// ä���ϰ� �ִ� ���
	//----------------------------
	CGameObject* _GatheringTarget;

	//---------------------------
	// ������Ʈ�� ������ ��ġ
	//---------------------------
	st_Vector2Int _SpawnPosition;

	// �þ� ��
	float _FieldOfAngle;
	// �ٶ󺸴� ����
	st_Vector2 _FieldOfDirection;	
	// �þ� �Ÿ�
	float _FieldOfViewDistance;			

	//--------------------------------------------
	// SpawnIdle ���¿��� Idle ���·� ���ư� Tick
	//--------------------------------------------
	uint64 _SpawnIdleTick;	

	// Ÿ��
	CGameObject* _Owner;
	
	// ��ȭȿ��
	map<en_SkillType, CSkill*> _Bufs;
	// ��ȭȿ��
	map<en_SkillType, CSkill*> _DeBufs;

	// ���ӿ�����Ʈ�� ó���ؾ��� Job ����ü
	CLockFreeQue<st_GameObjectJob*> _GameObjectJobQue;

	CGameObject();
	CGameObject(st_GameObjectInfo GameObjectInfo);
	virtual ~CGameObject();

	// �з��� �����̻� �ɷ� �ִ��� Ȯ��
	void PushedOutStatusAbnormalCheck();

	virtual void Update();
	virtual bool OnDamaged(CGameObject* Attacker, int32 DamagePoint);
	virtual void OnHeal(CGameObject* Healer, int32 HealPoint);	

	st_Vector2 PositionCheck(st_Vector2Int& CheckPosition);
	virtual void PositionReset();

	st_PositionInfo GetPositionInfo();		

	//------------------------------------------------
	// ���Ⱚ�� �޾Ƽ� ���ʿ� �ִ� ��ġ�� ��ȯ�Ѵ�.
	//------------------------------------------------
	st_Vector2 GetFrontPosition(int8 Distance);

	//------------------------------------------------------------------------------------
	// �� ���� Distance�ȿ� �ִ� ��ġ���� ���� ��ȯ�Ѵ�.
	//------------------------------------------------------------------------------------
	vector<st_Vector2Int> GetAroundCellPositions(st_Vector2Int CellPosition, int8 Distance);

	void SetOwner(CGameObject* Target);
	CGameObject* GetTarget();

	//---------------------------------------------
	// ��ȭȿ�� ��ų �߰� �� ����
	//---------------------------------------------
	void AddBuf(CSkill* Buf);
	void DeleteBuf(en_SkillType DeleteBufSkillType);

	//-------------------------------------------------
	// ��ȭȿ�� ��ų �߰� �� ����
	//-------------------------------------------------
	void AddDebuf(CSkill* DeBuf);
	void DeleteDebuf(en_SkillType DeleteDebufSkillType);

	//--------------------------------------------------
	// �����̻� �� ���� �� ����
	//--------------------------------------------------
	void SetStatusAbnormal(int32 StatusAbnormalValue);
	void ReleaseStatusAbnormal(int32 StatusAbnormalValue);
		
	//--------------------------------------------
	// ���� �� �� ���� �����̻� üũ
	//--------------------------------------------
	int32 CheckCantControlStatusAbnormal();
	//--------------------------------------------
	// ���� �� �� �ִ� �����̻� üũ
	//--------------------------------------------
	int32 CheckCanControlStatusAbnormal();

	//--------------------------------------------
	// ä�� �������� �� ����
	//--------------------------------------------
	CChannel* GetChannel();
	void SetChannel(CChannel* Channel);

	CRectCollision* GetRectCollision();
	
	bool IsPlayer();
	
	virtual void Init(en_GameObjectType GameObjectType);

	virtual void Start();
	virtual void End();

	vector<CGameObject*> GetFieldOfViewObjects();

	void SetMeleeSkill(CSkill* MeleeSkill);	
protected:	

	// �þ� ���� ������Ʈ ��ü
	vector<CGameObject*> _FieldOfViewObjects;
	// �þ� ���� ������Ʈ
	vector<st_FieldOfViewInfo> _FieldOfViewInfos;

	CRectCollision* _RectCollision;

	map<int64, st_Aggro> _AggroTargetList;

	int64 _ReSpawnTime;

	// ���ӿ�����Ʈ�� ���� ä��	
	CChannel* _Channel;

	// ���� ���� ���� ���
	CSkill* _MeleeSkill;
	
	// ���� ���� ������ų	
	CSkill* _SpellSkill;

	//-------------------------
	// ����� Tick
	//-------------------------
	uint64 _NatureRecoveryTick;

	//--------------------------------------
	// ���� ���¿��� �⺻������ ������ Tick
	//--------------------------------------
	uint64 _DefaultAttackTick;
	//--------------------------------------
	// ���� ���� ���¿��� ���� ���� �Ϸ� Tick
	//--------------------------------------
	uint64 _SpellTick;
	//--------------------------------------
	// ä�� �Ϸ� Tick
	//--------------------------------------
	uint64 _GatheringTick;
	//--------------------------------------
	// ���� Tick
	//--------------------------------------
	uint64 _CraftingTick;

	//---------------------------
	// ���� �þ� ������Ʈ Ž�� ƽ
	//---------------------------
	uint64 _FieldOfViewUpdateTick;

	//------------------------------------
	// ���� ƽ
	//------------------------------------
	uint64 _DeadTick;	

	//------------------------------------
	// ���ȯ ƽ
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
	virtual void UpdateDead();	

	void CheckBufDeBufSkill();
	
private:
	st_Vector2 _MousePosition;
};