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

	// ������ ���
	CGameObject* _SelectTarget;

	//---------------------------
	// ������Ʈ�� ������ ��ġ
	//---------------------------
	st_Vector2Int _SpawnPosition;

	// �þ� ����
	int8 _FieldOfViewDistance;		
	
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

	virtual void Update();
	virtual bool OnDamaged(CGameObject* Attacker, int32 DamagePoint);
	virtual void OnHeal(CGameObject* Healer, int32 HealPoint);
	virtual void OnDead(CGameObject* Killer);

	st_Vector2 PositionCheck(st_Vector2Int& CheckPosition);
	virtual void PositionReset();

	st_PositionInfo GetPositionInfo();

	//--------------------------------------------
	// ���� ��ǥ �������� st_Vector2Int�� ��ȯ�Ѵ�.
	//--------------------------------------------
	st_Vector2Int GetCellPosition();

	//-----------------------
	// float ��ǥ ��ȯ
	//-----------------------
	st_Vector2 GetPosition();

	//------------------------------------------------
	// ���Ⱚ�� �޾Ƽ� ���ʿ� �ִ� ��ġ�� ��ȯ�Ѵ�.
	//------------------------------------------------
	st_Vector2Int GetFrontCellPosition(en_MoveDir Dir,int8 Distance);
	
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
	void SetStatusAbnormal(int8 StatusAbnormalValue);
	void ReleaseStatusAbnormal(int8 StatusAbnormalValue);

	int8 CheckStatusAbnormal();
protected:
	//-------------------------
	// ����� Tick
	//-------------------------
	uint64 _NatureRecoveryTick;	

	//---------------------------
	// ���� �þ� ������Ʈ Ž�� ƽ
	//---------------------------
	uint64 _FieldOfViewUpdateTick;

	void BroadCastPacket(en_GAME_SERVER_PACKET_TYPE PacketType, bool CanMove = true);
};

