#include "pch.h"
#include "Channel.h"
#include "Player.h"
#include "Message.h"
#include "Monster.h"
#include "Item.h"

CChannel::~CChannel()
{
	for (int i = 0; i < _SectorCountY; i++)
	{
		delete _Sectors[i];
		_Sectors[i] = nullptr;
	}

	delete _Map;
	delete _Sectors;
}

void CChannel::Init(int32 MapId, int32 SectorSize)
{
	_Map = new CMap(MapId);

	_SectorSize = SectorSize;

	// 맵 크기를 토대로 섹터가 가로 세로 몇개씩 있는지 확인
	_SectorCountY = (_Map->_SizeY + _SectorSize - 1) / _SectorSize;
	_SectorCountX = (_Map->_SizeX + _SectorSize - 1) / _SectorSize;

	// 앞서 구한 가로 세로 개수를 토대로 동적할당 
	_Sectors = new CSector * [_SectorCountY];

	for (int32 i = 0; i < _SectorCountY; i++)
	{
		_Sectors[i] = new CSector[_SectorCountX];
	}

	for (int32 Y = 0; Y < _SectorCountY; Y++)
	{
		for (int32 X = 0; X < _SectorCountX; X++)
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
	CSector* Sector = GetSector(CellPosition);

	int MaxY = Sector->_SectorY + Range;
	int MinY = Sector->_SectorY - Range;
	int MaxX = Sector->_SectorX + Range;
	int MinX = Sector->_SectorX - Range;
	
	vector<CSector*> Sectors;

	for (int X = MinX; X <= MaxX; X++)
	{
		for (int Y = MinY; Y <= MaxY; Y++)
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
			// 함수 호출한 오브젝트를 포함할 것인지에 대한 여부 true면 제외 false면 포함
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
	// 주위 섹터 얻어오고
	vector<CSector*> Sectors = GetAroundSectors(Object->GetCellPosition(), Range);
	vector<CPlayer*> Players;

	// 섹터에 있는 플레이어를 담아서 반환
	// ExceptMe가 true면 제외 false면 포함
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
	// 주위 플레이어 정보 받아와서
	vector<CPlayer*> Players = GetAroundPlayer(Object, Range);	

	// 플레이어 목록 돌면서 길찾기로 찾아보고 갈 수 있으면 해당 플레이어반환
	for (int32 i = 0; i < Players.size(); i++)
	{
		CPlayer* Player = Players[i];
		vector<st_Vector2Int> Path = _Map->FindPath(Object->GetCellPosition(), Player->GetCellPosition());
		if (Path.size() < 2)
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

	for (auto PlayerIterator : _Players)
	{
		CPlayer* Player = PlayerIterator.second;

		Player->Update();
	}

	for (auto ItemIterator : _Items)
	{
		CItem* Item = ItemIterator.second;

		Item->Update();
	}
}

void CChannel::EnterChannel(CGameObject* EnterChannelGameObject, st_Vector2Int* ObjectSpawnPosition)
{
	// 채널 입장
	if (EnterChannelGameObject == nullptr)
	{
		CRASH("GameObject가 nullptr");
		return;
	}	

	random_device RD;
	mt19937 Gen(RD());
		
	st_Vector2Int SpawnPosition;

	// 스폰 포지션을 따로 지정해 주지 않았으면 랜덤 스폰 좌표 얻음
	if (ObjectSpawnPosition == nullptr)
	{
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
	}
	else
	{
		SpawnPosition = *ObjectSpawnPosition;
	}
	
	G_Logger->WriteStdOut(en_Color::RED, L"SpawnPosition Y : %d X : %d \n", SpawnPosition._Y, SpawnPosition._X);	

	// 입장한 오브젝트의 타입에 따라
	switch ((en_GameObjectType)EnterChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::MELEE_PLAYER:
	case en_GameObjectType::MAGIC_PLAYER:
		{
			// 플레이어로 형변환
			CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._Y;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._X;

			// 플레이어 자료구조에 저장
			_Players.insert(pair<int64, CPlayer*>(EnterChannelPlayer->_GameObjectInfo.ObjectId, EnterChannelPlayer));
			
			// 채널 저장
			EnterChannelPlayer->_Channel = this;		

			// 맵에 적용
			_Map->ApplyMove(EnterChannelPlayer, SpawnPosition);

			// 섹터 얻어서 해당 섹터에도 저장
			CSector* EnterSector = GetSector(SpawnPosition);			
			EnterSector->Insert(EnterChannelPlayer);			
		}
		break;
	case en_GameObjectType::SLIME:
	case en_GameObjectType::BEAR:
		{
			// 몬스터로 형변환
			CMonster* EnterChannelMonster = (CMonster*)EnterChannelGameObject;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._Y;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._X;

			// 몬스터 자료구조에 저장
			_Monsters.insert(pair<int64,CMonster*>(EnterChannelMonster->_GameObjectInfo.ObjectId,EnterChannelMonster));

			// 채널 저장
			EnterChannelMonster->_Channel = this;		

			// 맵에 적용
			_Map->ApplyMove(EnterChannelMonster, SpawnPosition);
			EnterChannelMonster->Init(SpawnPosition);

			// 섹터 얻어서 해당 섹터에도 저장
			CSector* EnterSector = GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelMonster);												
		}
		break;
	case en_GameObjectType::SLIME_GEL:
	case en_GameObjectType::BRONZE_COIN:
	case en_GameObjectType::LEATHER:
	case en_GameObjectType::SKILL_BOOK:
		{
			// 아이템으로 형변환
			CItem* EnterChannelItem = (CItem*)EnterChannelGameObject;
			EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._Y;
			EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._X;

			_Items.insert(pair<int64, CItem*>(EnterChannelItem->_GameObjectInfo.ObjectId,EnterChannelItem));

			EnterChannelItem->_Channel = this;			
			
			// 맵에 적용 아이템의 경우는 충돌체로 인식하지 않게 한다.
			_Map->ApplyMove(EnterChannelItem, SpawnPosition, false, false);

			// 섹터 얻어서 해당 섹터에도 저장
			CSector* EnterSector = GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelItem);
		}
		break;
	}
}

void CChannel::LeaveChannel(CGameObject* LeaveChannelGameObject)
{	
	// 채널 퇴장
	switch ((en_GameObjectType)LeaveChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::MELEE_PLAYER:
	case en_GameObjectType::MAGIC_PLAYER:
		// 컨테이너에서 제거
		_Players.erase(LeaveChannelGameObject->_GameObjectInfo.ObjectId);
		
		_Map->ApplyLeave(LeaveChannelGameObject);		
		break;
	case en_GameObjectType::SLIME:
	case en_GameObjectType::BEAR:
		_Monsters.erase(LeaveChannelGameObject->_GameObjectInfo.ObjectId);

		_Map->ApplyLeave(LeaveChannelGameObject);		
		break;	
	case en_GameObjectType::SLIME_GEL:
	case en_GameObjectType::BRONZE_COIN:
	case en_GameObjectType::LEATHER:
	case en_GameObjectType::SKILL_BOOK:
		_Items.erase(LeaveChannelGameObject->_GameObjectInfo.ObjectId);

		_Map->ApplyLeave(LeaveChannelGameObject);
		break;
	}	
}
