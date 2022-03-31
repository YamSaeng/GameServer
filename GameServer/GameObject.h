#pragma once

#include "Channel.h"
#include "CommonProtocol.h"
#include "GameObjectInfo.h"
#include "LockFreeQue.h"

class CSkill;

#define STATUS_ABNORMAL_WARRIOR_CHOHONE         0b00000001
#define STATUS_ABNORMAL_WARRIOR_SHAEHONE        0b00000010
#define STATUS_ABNORMAL_SHAMAN_ROOT             0b00000100
#define STATUS_ABNORMAL_SHAMAN_ICE_CHAIN        0b00001000
#define STATUS_ABNORMAL_SHAMAN_ICE_WAVE         0b00010000
#define STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE 0b00100000
#define STATUS_ABNORMAL_TAIOIST_ROOT			0b01000000

#define STATUS_ABNORMAL_WARRIOR_CHOHONE_MASK         0b11111110
#define STATUS_ABNORMAL_WARRIOR_SHAEHONE_MASK        0b11111101
#define STATUS_ABNORMAL_SHAMAN_ROOT_MASK             0b11111011
#define STATUS_ABNORMAL_SHAMAN_ICE_CHAIN_MASK        0b11110111
#define STATUS_ABNORMAL_SHAMAN_ICE_WAVE_MASK         0b11101111
#define STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE_MASK 0b11011111
#define STATUS_ABNORMAL_TAIOIST_ROOT_MASK            0b10111111

class CGameObject
{
private:
public:
	int8 _StatusAbnormal;
	bool _IsSendPacketTarget;

	int32 _ObjectManagerArrayIndex;
	int32 _ChannelArrayIndex;

	en_ObjectNetworkState _NetworkState;
	st_GameObjectInfo _GameObjectInfo;		
	CChannel* _Channel;	

	// 선택한 대상
	CGameObject* _SelectTarget;

	//---------------------------
	// 오브젝트가 스폰될 위치
	//---------------------------
	st_Vector2Int _SpawnPosition;

	// 시야 범위
	int8 _FieldOfViewDistance;		
	
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

	virtual void Update();
	virtual bool OnDamaged(CGameObject* Attacker, int32 DamagePoint);
	virtual void OnHeal(CGameObject* Healer, int32 HealPoint);
	virtual void OnDead(CGameObject* Killer);

	st_Vector2 PositionCheck(st_Vector2Int& CheckPosition);
	virtual void PositionReset();

	st_PositionInfo GetPositionInfo();

	//--------------------------------------------
	// 현재 좌표 기준으로 st_Vector2Int를 반환한다.
	//--------------------------------------------
	st_Vector2Int GetCellPosition();

	//-----------------------
	// float 좌표 반환
	//-----------------------
	st_Vector2 GetPosition();

	//------------------------------------------------
	// 방향값을 받아서 앞쪽에 있는 위치를 반환한다.
	//------------------------------------------------
	st_Vector2Int GetFrontCellPosition(en_MoveDir Dir,int8 Distance);
	
	//------------------------------------------------------------------------------------
	// 내 주위 Distance안에 있는 위치들의 값을 반환한다.
	//------------------------------------------------------------------------------------
	vector<st_Vector2Int> GetAroundCellPositions(st_Vector2Int CellPosition, int8 Distance);
	
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
	void SetStatusAbnormal(int8 StatusAbnormalValue);
	void ReleaseStatusAbnormal(int8 StatusAbnormalValue);

	int8 CheckStatusAbnormal();
protected:
	//-------------------------
	// 재생력 Tick
	//-------------------------
	uint64 _NatureRecoveryTick;	

	//---------------------------
	// 주위 시야 오브젝트 탐색 틱
	//---------------------------
	uint64 _FieldOfViewUpdateTick;

	void BroadCastPacket(en_GAME_SERVER_PACKET_TYPE PacketType, bool CanMove = true);
};

