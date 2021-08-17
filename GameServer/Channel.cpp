#include "pch.h"
#include "Channel.h"
#include "Player.h"
#include "Message.h"
#include "Monster.h"

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

	// 맵 크기를 토대로 섹터가 가로 세로 몇개씩 있는지 확인
	_SectorCountY = (_Map->_SizeY + _SectorSize - 1) / _SectorSize;
	_SectorCountX = (_Map->_SizeX + _SectorSize - 1) / _SectorSize;

	// 앞서 구한 가로 세로 개수를 토대로 동적할당 
	_Sectors = new CSector * [_SectorCountY];

	for (int i = 0; i < _SectorCountY; i++)
	{
		_Sectors[i] = new CSector[_SectorCountX];
	}

	for (int Y = 0; Y < _SectorCountY; Y++)
	{
		for (int X = 0; X < _SectorCountX; X++)
		{
			// 섹터 저장
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

vector<CSector*> CChannel::GetAroundSectors(st_Vector2Int CellPosition, int32 Range)
{
	vector<CSector*> Sectors;

	int MaxY = CellPosition._Y + Range;
	int MinY = CellPosition._Y - Range;
	int MaxX = CellPosition._X + Range;
	int MinX = CellPosition._X - Range;

	// 좌측 상단 섹터 얻기
	st_Vector2Int LeftTop;
	LeftTop._X = MinX;
	LeftTop._Y = MaxY;

	int MinIndexY = (_Map->_Down - LeftTop._Y) / _SectorSize;
	int MinIndexX = (LeftTop._X - _Map->_Left) / _SectorSize;

	// 우측 하단 섹터 얻기
	st_Vector2Int RightBottom;
	RightBottom._X = MaxX;
	RightBottom._Y = MinY;

	int MaxIndexY = (_Map->_Down - RightBottom._Y) / _SectorSize;
	int MaxIndexX = (RightBottom._X - _Map->_Left) / _SectorSize;

	CSector* NowSector = GetSector(CellPosition);

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

vector<CGameObject*> CChannel::GetAroundObjects(CGameObject* Object, int32 Range, bool ExceptMe)
{
	vector<CGameObject*> GameObjects;

	vector<CSector*> Sectors = GetAroundSectors(Object->GetCellPosition(), Range);
		
	for (CSector* Sector : Sectors)
	{
		// 주변 섹터 플레이어 정보
		for (CPlayer* Player : Sector->GetPlayers())
		{
			if (ExceptMe == true)
			{
				if (Object->_GameObjectInfo.ObjectId != Player->_GameObjectInfo.ObjectId)
				{
					GameObjects.push_back(Player);
				}
			}
			else
			{
				GameObjects.push_back(Player);
			}		
		}

		// 주변 섹터 몬스터 정보
		for (CMonster* Monster : Sector->GetMonsters())
		{
			GameObjects.push_back(Monster);
		}
	}	
	
	return GameObjects;
}

vector<CPlayer*> CChannel::GetAroundPlayer(CGameObject* Object, int32 Range, bool ExceptMe)
{
	vector<CSector*> Sectors = GetAroundSectors(Object->GetCellPosition(), Range);
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

CPlayer* CChannel::FindNearPlayer(CGameObject* Object, int32 Range)
{
	vector<CPlayer*> Players = GetAroundPlayer(Object, Range);	

	for (int32 i = 0; i < Players.size(); i++)
	{
		CPlayer* Player = Players[i];
		vector<st_Vector2Int> Path = _Map->FindPath(Object->GetCellPosition(), Player->GetCellPosition());
		if (Path.size() < 2 || Path.size() > Range)
		{
			continue;
		}

		return Player;
	}

	return nullptr;
}

void CChannel::Update()
{
	// 소유하고 있는 몬스터 업데이트
	for (auto MonsterIteraotr : _Monsters)
	{
		CMonster* Monster = MonsterIteraotr.second;

		Monster->Update();
	}
}

void CChannel::EnterChannel(CGameObject* EnterChannelGameObject)
{
	if (EnterChannelGameObject == nullptr)
	{
		CRASH("GameObject가 nullptr");
		return;
	}	

	random_device RD;
	mt19937 Gen(RD());

	st_Vector2Int SpawnPosition;

	while (true)
	{
		uniform_int_distribution<int> RandomXPosition(-10, 54);
		uniform_int_distribution<int> RandomYPosition(-4, 40);

		SpawnPosition._X = RandomXPosition(Gen);
		SpawnPosition._Y = RandomYPosition(Gen);

		if (_Map->Find(SpawnPosition) == nullptr)
		{
			break;
		}
	}
	
	G_Logger->WriteStdOut(en_Color::RED, L"SpawnPosition Y : %d X : %d \n", SpawnPosition._Y, SpawnPosition._X);
	int SpawnX = rand() % 100;

	switch (EnterChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::PLAYER:
		{
			CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._X;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._Y;

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
		{
			CMonster* EnterChannelMonster = (CMonster*)EnterChannelGameObject;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._X;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._Y;

			_Monsters.insert(pair<int64,CMonster*>(EnterChannelMonster->_GameObjectInfo.ObjectId,EnterChannelMonster));

			EnterChannelMonster->_Channel = this;

			st_Vector2Int EnterChannelMonsterPosition;
			EnterChannelMonsterPosition._X = EnterChannelMonster->GetPositionInfo().PositionX;
			EnterChannelMonsterPosition._Y = EnterChannelMonster->GetPositionInfo().PositionY;

			_Map->ApplyMove(EnterChannelMonster, EnterChannelMonsterPosition);

			CSector* EnterSector = GetSector(EnterChannelMonsterPosition);
			EnterSector->Insert(EnterChannelMonster);												
		}
		break;
	}
}

void CChannel::LeaveChannel(CGameObject* LeaveChannelGameObject)
{	
	switch (LeaveChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::PLAYER:
		// 컨테이너에서 제거
		_Players.erase(LeaveChannelGameObject->_GameObjectInfo.ObjectId);
		
		_Map->ApplyLeave(LeaveChannelGameObject);
		
		LeaveChannelGameObject->_Channel = nullptr;
		break;
	case en_GameObjectType::MONSTER:
		_Monsters.erase(LeaveChannelGameObject->_GameObjectInfo.ObjectId);

		_Map->ApplyLeave(LeaveChannelGameObject);
		
		LeaveChannelGameObject->_Channel = nullptr;
		break;	
	}	
}
