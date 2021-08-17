#pragma once
#include "Sector.h"
#include "Map.h"

class CMonster;

class CChannel
{
public:	
	int32 _ChannelId;	
	map<int64, CPlayer*> _Players;
	map<int64, CMonster*> _Monsters;
	
	CMap* _Map;

	CSector** _Sectors;
	
	int32 _SectorSize;
	int32 _SectorCountX;
	int32 _SectorCountY;

	CChannel() {};
	~CChannel();	

	void Init(int MapId, int SectorSize);

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
	CPlayer* FindNearPlayer(CGameObject* Object, int32 Range);

	void Update();

	//----------------------------------------------------
	// ä�� ����
	// - Object�� ä�ο� �����Ű�鼭 �ڷᱸ���� ������ ��
	// - Map���� �ش� ������Ʈ�� ��ġ�� ����Ѵ�.
	//----------------------------------------------------
	void EnterChannel(CGameObject* EnterChannelGameObject);
	//----------------------------------------------------
	// ä�� ������
	// - Object�� ä�ο� �����Ű�鼭 �ڷᱸ���� ������ ��
	// - Map���� �ش� ������Ʈ�� ��ġ�� �����Ѵ�.
	//----------------------------------------------------
	void LeaveChannel(CGameObject* LeaveChannelGameObject);	
};

