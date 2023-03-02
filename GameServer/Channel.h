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
	// ä�� ID
	int32 _ChannelId;
	// ä���� ó���ؾ��� Job ����ü
	CLockFreeQue<st_GameObjectJob*> _ChannelJobQue;

	CChannel();
	~CChannel();

	//---------------------------------------
	// ä�� �ʱ�ȭ
	//---------------------------------------
	void Init();

	//------------------------------
	// �����ϰ� �ִ� ���� ������Ʈ
	//------------------------------
	void Update();

	//----------------------------
	// ä�ο� �� �Ҵ�
	//----------------------------
	void SetMap(CMap* Map);
	//----------------------------
	// ä�ο� �Ҵ�� �� ��������
	//----------------------------
	CMap* GetMap();	

	//------------------------------------------------------------------------------
	// ä�ο� �ִ� ������Ʈ ã�� ( ���� )
	//------------------------------------------------------------------------------
	CGameObject* FindChannelObject(int64 ObjectID, en_GameObjectType GameObjectType);
	//------------------------------------------------------------------------------
	// ä�ο� �ִ� ������Ʈ ã�� ( ���� )
	//------------------------------------------------------------------------------
	vector<CGameObject*> FindChannelObjects(en_GameObjectType GameObjectType);	
	//------------------------------------------------------------------------------
	// ä�ο� �ִ� ������Ʈ�� ã��
	//------------------------------------------------------------------------------
	vector<CGameObject*> FindChannelObjects(vector<st_FieldOfViewInfo>& FindObjectIDs);
	//-----------------------------------------------------
	// ä�ο� �ִ� ���� ������ ������Ʈ�� ã�� ( �Ÿ� ���� )
	//-----------------------------------------------------
	vector<CGameObject*> FindAttackChannelObjects(vector<st_FieldOfViewInfo>& FindObjectIDs, CGameObject* Object, st_Vector2 Direction, int16 Distance);	
	//---------------------------------------------------------
	// ä�ο� �ִ� ���� ������ ������Ʈ ã�� ( ���� ���� )
	//---------------------------------------------------------
	vector<CGameObject*> FindRangeAttackChannelObjects(CGameObject* Object, st_Vector2 Direciton, int16 Distance);

	//-----------------------------------------------------
	// ä�ο� �ִ� ������Ʈ��� �˻��ؼ� �浹 �Ǵ�
	//-----------------------------------------------------
	bool ChannelColliderCheck(CGameObject* Object);
	//-----------------------------------------------------
	// ä�ο� �ִ� ������Ʈ��� �˻��ؼ� �浹 �Ǵ� ( �Ű� ������ ���� ��ǥ�� �������� )
	//-----------------------------------------------------
	bool ChannelColliderCheck(CGameObject* CheckObject, st_Vector2 CheckPosition);

	//----------------------------------------------------
	// ä�� ����
	// - Object�� ä�ο� �����Ű�鼭 �ڷᱸ���� ������ ��
	// - Map���� �ش� ������Ʈ�� ��ġ�� ����Ѵ�.
	//----------------------------------------------------
	bool EnterChannel(CGameObject* EnterChannelGameObject, st_Vector2Int* ObjectSpawnPosition = nullptr);
	//----------------------------------------------------
	// ä�� ������
	// - Object�� ä�ο� ���� �ִ� Index�� �ݳ�
	//----------------------------------------------------
	void LeaveChannel(CGameObject* LeaveChannelGameObject);	

	//-------------------------------------------------------------------------
	// ����ġ ���
	//-------------------------------------------------------------------------
	void ExperienceCalculate(CPlayer* TargetPlayer, en_GameObjectType TargetMonsterObjectType, int32 ExperiencePoint);
private:
	enum en_Channel
	{
		CHANNEL_PLAYER_MAX = 100,
		CHANNEL_DUMMY_PLAYER_MAX = 500,
		CHANNEL_NON_PLAYER_MAX = 50,
		CHANNEL_MONSTER_MAX = 100,
		CHANNEL_ENVIRONMENT_MAX = 100,
		CHANNEL_CRAFTING_TABLE_MAX = 100,
		CHANNEL_CROP_MAX = 100,
		CHANNEL_ITEM_MAX = 200
	};
	
	// ä�ο��� �������� PC, NPC, Monster, Item		
	CPlayer* _ChannelPlayerArray[CHANNEL_PLAYER_MAX];
	CPlayer* _ChannelDummyPlayerArray[CHANNEL_DUMMY_PLAYER_MAX];
	CNonPlayer* _ChannelNonPlayerArray[CHANNEL_NON_PLAYER_MAX];
	CMonster* _ChannelMonsterArray[CHANNEL_MONSTER_MAX];
	CEnvironment* _ChannelEnvironmentArray[CHANNEL_ENVIRONMENT_MAX];
	CCraftingTable* _ChannelCraftingTableArray[CHANNEL_CRAFTING_TABLE_MAX];
	CCrop* _ChannelCropArray[CHANNEL_CROP_MAX];
	CItem* _ChannelItemArray[CHANNEL_ITEM_MAX];

	CLockFreeStack<int32> _ChannelPlayerArrayIndexs;
	CLockFreeStack<int32> _ChannelDummyPlayerArrayIndexs;
	CLockFreeStack<int32> _ChannelNonPlayerArrayIndexs;
	CLockFreeStack<int32> _ChannelMonsterArrayIndexs;
	CLockFreeStack<int32> _ChannelEnvironmentArrayIndexs;
	CLockFreeStack<int32> _ChannelCraftingTableArrayIndexs;
	CLockFreeStack<int32> _ChannelCropArrayIndexs;
	CLockFreeStack<int32> _ChannelItemArrayIndexs;	
	
	// ä�ο� �Ҵ�� Map
	CMap* _Map;
};