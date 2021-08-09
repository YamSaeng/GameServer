#pragma once
#include "Sector.h"
#include "Map.h"

class CChannel
{
public:	
	int32 _ChannelId;	
	map<int64, CPlayer*> _Players;
	
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
	vector<CSector*> GetAroundSectors(CGameObject* Object, int32 Range);
	vector<CGameObject*> GetAroundObjects(CGameObject* Object, int32 Range);
	
	//--------------------------------------------------------------------------------------
	// �� ���� �÷��̾� ��ȯ
	//--------------------------------------------------------------------------------------
	vector<CPlayer*> GetAroundPlayer(CGameObject* Object, int32 Range, bool ExceptMe = true);

	void Update();

	//----------------------------------------------------
	// ä�� ����
	//----------------------------------------------------
	void EnterChannel(CGameObject* EnterChannelGameObject);
	//----------------------------------------------------
	// ä�� ������
	//----------------------------------------------------
	void LeaveChannel(CGameObject* LeaveChannelGameObject);	
};

