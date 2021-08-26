#pragma once
#include "Sector.h"
#include "Map.h"

class CMonster;
class CItem;

class CChannel
{
private:
	//-------------------------------------------
	// ä�ο��� �������� �÷��̾�, ����, ������
	//-------------------------------------------
	map<int64, CPlayer*> _Players;
	map<int64, CMonster*> _Monsters;
	map<int64, CItem*> _Items;

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

	CChannel() {};
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
	vector<CGameObject*> GetAroundObjects(CGameObject* Object, int32 Range, bool ExceptMe = true);
	
	//--------------------------------------------------------------------------------------
	// �� ���� �÷��̾� ��ȯ
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetAroundPlayer(CGameObject* Object, int32 Range, bool ExceptMe = true);

	//-------------------------------------------------------
	// �� ��ó �÷��̾� ��ȯ
	//-------------------------------------------------------
	CPlayer* FindNearPlayer(CGameObject* Object, int32 Range);

	//------------------------------
	// �����ϰ� �ִ� ���� ������Ʈ
	//------------------------------
	void Update();
	
	//----------------------------------------------------
	// ä�� ����
	// - Object�� ä�ο� �����Ű�鼭 �ڷᱸ���� ������ ��
	// - Map���� �ش� ������Ʈ�� ��ġ�� ����Ѵ�.
	//----------------------------------------------------
	void EnterChannel(CGameObject* EnterChannelGameObject,st_Vector2Int* ObjectSpawnPosition = nullptr);
	//----------------------------------------------------
	// ä�� ������
	// - Object�� ä�ο� �����Ű�鼭 �ڷᱸ���� ������ ��
	// - Map���� �ش� ������Ʈ�� ��ġ�� �����Ѵ�.
	//----------------------------------------------------
	void LeaveChannel(CGameObject* LeaveChannelGameObject);	
};

