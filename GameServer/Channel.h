#pragma once
#include "Sector.h"
#include "LockFreeQue.h"
#include "LockFreeStack.h"

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
		PLAYER_MAX = 800,
		MONSTER_MAX = 100,
		ITEM_MAX = 200,
		ENVIRONMENT_MAX = 100
	};

	//-------------------------------------------
	// ä�ο��� �������� �÷��̾�, ����, ������
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