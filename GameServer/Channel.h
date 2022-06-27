#pragma once
#include "Sector.h"
#include "LockFreeQue.h"
#include "LockFreeStack.h"
#include "GameObjectInfo.h"

class CMonster;
class CItem;
class CEnvironment;
class CPlayer;
class CMap;
struct st_Vector2Int;
struct st_GameObjectJob;

class CChannel
{
private:
	enum en_Channel
	{
		PLAYER_MAX = 100,
		DUMMY_PLAYER_MAX = 500,
		MONSTER_MAX = 100,
		ENVIRONMENT_MAX = 100,
		CRAFTING_TABLE_MAX = 100,
		CROP_MAX = 100,
		ITEM_MAX = 200		
	};

	//-------------------------------------------
	// ä�ο��� �������� �÷��̾�, ����, ������
	//-------------------------------------------
	CPlayer* _ChannelPlayerArray[PLAYER_MAX];
	CPlayer* _ChannelDummyPlayerArray[DUMMY_PLAYER_MAX];
	CMonster* _ChannelMonsterArray[MONSTER_MAX];
	CEnvironment* _ChannelEnvironmentArray[ENVIRONMENT_MAX];
	CCraftingTable* _ChannelCraftingTableArray[CRAFTING_TABLE_MAX];
	CCrop* _ChannelCropArray[CROP_MAX];
	CItem* _ChannelItemArray[ITEM_MAX];		

	CLockFreeStack<int32> _ChannelPlayerArrayIndexs;
	CLockFreeStack<int32> _ChannelDummyPlayerArrayIndexs;
	CLockFreeStack<int32> _ChannelMonsterArrayIndexs;
	CLockFreeStack<int32> _ChannelEnvironmentArrayIndexs;
	CLockFreeStack<int32> _ChannelCraftingTableArrayIndexs;
	CLockFreeStack<int32> _ChannelCropArrayIndexs;
	CLockFreeStack<int32> _ChannelItemArrayIndexs;	

	SRWLOCK _ChannelLock;

	//-----------------
	// ���� ���
	//-----------------
	CSector** _Sectors;

	CMap* _Map;
public:
	int32 _ChannelId;	

	//----------------
	// ���� ũ��
	//----------------
	int32 _SectorSize;

	//------------------
	// ���� X, Y ����
	//------------------
	int32 _SectorCountX;
	int32 _SectorCountY;

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

	CMap* GetMap();
	void SetMap(CMap* Map);

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
	// ä�ο� �ִ� ������Ʈ�� ã�� ( �Ÿ� ���� )
	//-----------------------------------------------------
	vector<CGameObject*> FindChannelObjects(vector<st_FieldOfViewInfo>& FindObjectIDs, CGameObject* Object, int16 Distance);

	//----------------------------------------------------
	// ä�� ����
	// - Object�� ä�ο� �����Ű�鼭 �ڷᱸ���� ������ ��
	// - Map���� �ش� ������Ʈ�� ��ġ�� ����Ѵ�.
	//----------------------------------------------------
	bool EnterChannel(CGameObject* EnterChannelGameObject, st_Vector2Int* ObjectSpawnPosition = nullptr);
	//----------------------------------------------------
	// ä�� ������
	// - Object�� ä�ο� �����Ű�鼭 �ڷᱸ���� ������ ��
	// - Map���� �ش� ������Ʈ�� ��ġ�� �����Ѵ�.
	//----------------------------------------------------
	void LeaveChannel(CGameObject* LeaveChannelGameObject);
};