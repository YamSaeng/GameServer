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
public:	
	int32 _ChannelId;		
	
	CMap* _Map;	
	
	//----------------
	// ���� ũ��
	//----------------
	int32 _SectorSize;

	//------------------
	// ���� X, Y ����
	//------------------
	int32 _SectorCountX;
	int32 _SectorCountY;

	CChannel();
	~CChannel();	

	//---------------------------------------
	// ä�� �ʱ�ȭ
	//---------------------------------------
	void Init(int32 MapId, int32 SectorSize);	

	//---------------------------------------------
	// ��ǥ ���� ���� ������
	//---------------------------------------------
	CSector* GetSector(st_Vector2Int CellPosition);
	CSector* GetSector(int32 IndexY, int32 IndexX);

	//-----------------------------------------------------------------
	// �� ���� ���� ��ȯ
	//-----------------------------------------------------------------
	vector<CSector*> GetAroundSectors(st_Vector2Int CellPosition, int32 Range);
	//-----------------------------------------------------------------
	// �� ���� ���� �ȿ� �ִ� ������Ʈ ��ȯ
	//-----------------------------------------------------------------
	vector<CGameObject*> GetAroundSectorObjects(CGameObject* Object, int32 Range, bool ExceptMe = true);
	//-----------------------------------------------------------------
	// ������Ʈ �þ� ���� �ȿ� �ִ� ������Ʈ ���̵� ��� ��ȯ
	//-----------------------------------------------------------------
	vector<st_FieldOfViewInfo> GetFieldOfViewObjects(CGameObject* Object, int16 Range, bool ExceptMe = true);
	//----------------------------------------------------------------------------------------
	// ������Ʈ ���� ���� ��� ��ȯ
	//----------------------------------------------------------------------------------------
	vector<CMonster*> GetAroundMonster(CGameObject* Object, int16 Range, bool ExceptMe = true);
	//--------------------------------------------------------------------------------------
	// �� ���� �÷��̾� ��ȯ
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetAroundPlayer(CGameObject* Object, int32 Range);
	//--------------------------------------------------------------------------------------
	// �� �þ� ���� �÷��̾� ��ȯ
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetFieldOfViewPlayer(CGameObject* Object, int16 Range, bool ExceptMe = true);

	//-------------------------------------------------------
	// �� ��ó �÷��̾� ��ȯ
	//-------------------------------------------------------
	CGameObject* FindNearPlayer(CGameObject* Object, int32 Range, bool* CollisionCango);

	//------------------------------
	// �����ϰ� �ִ� ���� ������Ʈ
	//------------------------------
	void Update();
	
	//----------------------------------------------------
	// ä�� ����
	// - Object�� ä�ο� �����Ű�鼭 �ڷᱸ���� ������ ��
	// - Map���� �ش� ������Ʈ�� ��ġ�� ����Ѵ�.
	//----------------------------------------------------
	bool EnterChannel(CGameObject* EnterChannelGameObject,st_Vector2Int* ObjectSpawnPosition = nullptr);
	//----------------------------------------------------
	// ä�� ������
	// - Object�� ä�ο� �����Ű�鼭 �ڷᱸ���� ������ ��
	// - Map���� �ش� ������Ʈ�� ��ġ�� �����Ѵ�.
	//----------------------------------------------------
	void LeaveChannel(CGameObject* LeaveChannelGameObject);			
};

