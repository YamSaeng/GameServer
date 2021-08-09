#include "pch.h"
#include "Channel.h"
#include "Player.h"
#include "Message.h"

CChannel::~CChannel()
{
	for (int i = 0; i < _SectorCountY; i++)
	{
		delete _Sectors[i];
		_Sectors[i] = nullptr;
	}

	delete _Sectors;
}

void CChannel::Init(int MapId, int SectorSize)
{
	_Map = new CMap(MapId);

	_SectorSize = SectorSize;

	// �� ũ�⸦ ���� ���Ͱ� ���� ���� ��� �ִ��� Ȯ��
	_SectorCountY = (_Map->_SizeY + _SectorSize - 1) / _SectorSize;
	_SectorCountX = (_Map->_SizeX + _SectorSize - 1) / _SectorSize;

	// �ռ� ���� ���� ���� ������ ���� �����Ҵ� 
	_Sectors = new CSector * [_SectorCountY];

	for (int i = 0; i < _SectorCountY; i++)
	{
		_Sectors[i] = new CSector[_SectorCountX];
	}

	for (int Y = 0; Y < _SectorCountY; Y++)
	{
		for (int X = 0; X < _SectorCountX; X++)
		{
			// ���� ����
			_Sectors[Y][X] = CSector(Y, X);
		}
	}
}

CSector* CChannel::GetSector(st_Vector2Int CellPosition)
{	
	int X = (CellPosition._X - _Map->_Left) / _SectorSize;
	int Y = (_Map->_Down - CellPosition._Y) / _SectorSize;

	return GetSector(Y, X);	
}

CSector* CChannel::GetSector(int32 IndexY, int32 IndexX)
{
	if (IndexX < 0 || IndexX >= _SectorCountX)
	{
		return nullptr;
	}

	if (IndexY < 0 || IndexY >= _SectorCountY)
	{
		return nullptr;
	}	

	return &_Sectors[IndexY][IndexX];
}

vector<CSector*> CChannel::GetAroundSectors(CGameObject* Object, int32 Range)
{
	vector<CSector*> Sectors;

	int MaxY = Object->GetCellPosition()._Y + Range;
	int MinY = Object->GetCellPosition()._Y - Range;
	int MaxX = Object->GetCellPosition()._X + Range;
	int MinX = Object->GetCellPosition()._X - Range;

	// ���� ��� ���� ���
	st_Vector2Int LeftTop;
	LeftTop._X = MinX;
	LeftTop._Y = MaxY;

	int MinIndexY = (_Map->_Down - LeftTop._Y) / _SectorSize;
	int MinIndexX = (LeftTop._X - _Map->_Left) / _SectorSize;

	// ���� �ϴ� ���� ���
	st_Vector2Int RightBottom;
	RightBottom._X = MaxX;
	RightBottom._Y = MinY;

	int MaxIndexY = (_Map->_Down - RightBottom._Y) / _SectorSize;
	int MaxIndexX = (RightBottom._X - _Map->_Left) / _SectorSize;

	CSector* NowSector = GetSector(Object->GetCellPosition());

	for (int X = MinIndexX; X <= MaxIndexX; X++)
	{
		for (int Y = MinIndexY; Y <= MaxIndexY; Y++)
		{
			CSector* Sector = GetSector(Y, X);
			if (Sector == nullptr)
			{
				continue;
			}

			Sectors.push_back(Sector);
		}
	}	

	return Sectors;
}

vector<CGameObject*> CChannel::GetAroundObjects(CGameObject* Object, int32 Range)
{
	vector<CGameObject*> GameObjects;

	vector<CSector*> Sectors = GetAroundSectors(Object, Range);	

	return GameObjects;
}

vector<CPlayer*> CChannel::GetAroundPlayer(CGameObject* Object, int32 Range, bool ExceptMe)
{
	vector<CSector*> Sectors = GetAroundSectors(Object, Range);
	vector<CPlayer*> Players;

	for (CSector* Sector : Sectors)
	{
		for (CPlayer* Player : Sector->GetPlayers())
		{
			if (ExceptMe == true)
			{
				if (Object->_GameObjectInfo.ObjectId != Player->_GameObjectInfo.ObjectId)
				{
					Players.push_back(Player);
				}
			}
			else
			{
				Players.push_back(Player);
			}			
		}
	}

	return Players;
}

void CChannel::Update()
{

}

void CChannel::EnterChannel(CGameObject* EnterChannelGameObject)
{
	if (EnterChannelGameObject == nullptr)
	{
		CRASH("GameObject�� nullptr");
		return;
	}	

	st_Vector2Int SpawnPosition(0, 0);

	switch (EnterChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::PLAYER:
		{
			CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;

			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._Y;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._X;

			_Players.insert(pair<int64, CPlayer*>(EnterChannelPlayer->_GameObjectInfo.ObjectId, EnterChannelPlayer));
			
			EnterChannelPlayer->_Channel = this;

			st_Vector2Int EnterChannelPlayerPosition;
			EnterChannelPlayerPosition._X = EnterChannelPlayer->GetPositionInfo().PositionX;
			EnterChannelPlayerPosition._Y = EnterChannelPlayer->GetPositionInfo().PositionY;

			_Map->ApplyMove(EnterChannelPlayer, EnterChannelPlayerPosition);

			CSector* EnterSector = GetSector(EnterChannelPlayerPosition);
			//G_Logger->WriteStdOut(en_Color::GREEN, L"EnterSector Y : (%d) X : (%d) \n", EnterSector->_SectorY, EnterSector->_SectorX);
			EnterSector->Insert(EnterChannelPlayer);
			
		}
		break;
	case en_GameObjectType::MONSTER:
		break;
	}
}

void CChannel::LeaveChannel(CGameObject* LeaveChannelGameObject)
{	
	switch (LeaveChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::PLAYER:
		// �����̳ʿ��� ����
		_Players.erase(LeaveChannelGameObject->_GameObjectInfo.ObjectId);

		// �ʿ����� ����
		_Map->ApplyLeave(LeaveChannelGameObject);
		break;
	case en_GameObjectType::MONSTER:
		break;	
	}
}
