#include "pch.h"
#include "Map.h"
#include "GameObject.h"
#include "Player.h"
#include "Monster.h"
#include "Heap.h"
#include "ObjectManager.h"
#include "Item.h"
#include "CraftingTable.h"
#include "Crop.h"
#include "DBConnectionPool.h"
#include "DBStoreProcedure.h"
#include "RectCollision.h"

CMap::CMap()
{

}

CMap::~CMap()
{
	if (_ChannelManager != nullptr)
	{
		delete _ChannelManager;
		_ChannelManager = nullptr;
	}

	if (_Sectors != nullptr)
	{
		for (int8 i = 0; i < _SectorCountY; i++)
		{
			delete _Sectors[i];
		}

		delete _Sectors;
		_Sectors = nullptr;
	}
}

void CMap::MapInit(int16 MapID, wstring MapName, int32 SectorSize, int8 ChannelCount)
{
	_MapID = MapID;
	_MapName = MapName;

	// �� ������Ʈ ���� �б�
#pragma region �� ������Ʈ ���� �б�
	wstring ObjectInfoData = MapName + L"ObjectInfoData.txt";

	char* MapObjectInfoDataFileStr = FileUtils::LoadFile(ObjectInfoData.c_str());
	char* MapObjectInfoDataMovingFileP = MapObjectInfoDataFileStr;
	char* MapObjectInfoDataConvertP = MapObjectInfoDataFileStr;

	int MapObjectInfoDataCount = 0;
	while (true)
	{
		if (*MapObjectInfoDataMovingFileP == '\n')
		{
			if (MapObjectInfoDataCount == 0)
			{
				_Left = atoi(MapObjectInfoDataConvertP);
				MapObjectInfoDataConvertP = MapObjectInfoDataMovingFileP;
				MapObjectInfoDataCount++;
			}
			else if (MapObjectInfoDataCount == 1)
			{
				_Right = atoi(MapObjectInfoDataConvertP);
				MapObjectInfoDataConvertP = MapObjectInfoDataMovingFileP;
				MapObjectInfoDataCount++;
			}
			else if (MapObjectInfoDataCount == 2)
			{
				_Up = atoi(MapObjectInfoDataConvertP);
				MapObjectInfoDataConvertP = MapObjectInfoDataMovingFileP;
				MapObjectInfoDataCount++;
			}
			else if (MapObjectInfoDataCount == 3)
			{
				_Down = atoi(MapObjectInfoDataConvertP);
				MapObjectInfoDataConvertP = MapObjectInfoDataMovingFileP + 1;
				MapObjectInfoDataCount++;
				break;
			}
		}

		MapObjectInfoDataMovingFileP++;
	}

	int XCount = _Right - _Left + 1;
	int YCount = _Down - _Up + 1;

	int YCountXCount = YCount * XCount;

	_CollisionMapInfos = new en_MapObjectInfo * [YCount];

	for (int i = 0; i < YCount; i++)
	{
		_CollisionMapInfos[i] = new en_MapObjectInfo[XCount];
	}

	for (int Y = 0; Y < YCount; Y++)
	{
		for (int X = 0; X < XCount; X++)
		{
			if (*MapObjectInfoDataConvertP == '\r')
			{
				break;
			}

			_CollisionMapInfos[Y][X] = (en_MapObjectInfo)(*MapObjectInfoDataConvertP - 48);

			MapObjectInfoDataConvertP++;
		}
		MapObjectInfoDataConvertP += 2;
	}
#pragma endregion	

#pragma region �� Ÿ�� ���� �б�
	// DB���� �� Ÿ�� ������ �о��
	CDBConnection* GetTileMapInfoAllocFreeDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
	SP::CDBGameServerGetTileMapInfoAllocFree GetTileMapInfoAllocFree(*GetTileMapInfoAllocFreeDBConnection);

	GetTileMapInfoAllocFree.InMapID(_MapID);

	bool MapTileAllocFree;
	int64 MapTileAccountID;
	int64 MapTilePlayerID;
	int32 MapTilePositionX;
	int32 MapTilePositionY;

	GetTileMapInfoAllocFree.OutMapTileAllocFree(MapTileAllocFree);
	GetTileMapInfoAllocFree.OutMapTileAccountID(MapTileAccountID);
	GetTileMapInfoAllocFree.OutMapTilePlayerID(MapTilePlayerID);
	GetTileMapInfoAllocFree.OutMapTilePositionX(MapTilePositionX);
	GetTileMapInfoAllocFree.OutMapTilePositionY(MapTilePositionY);

	GetTileMapInfoAllocFree.Execute();

	vector<st_TileMapInfo> TileMapInfos;

	while (GetTileMapInfoAllocFree.Fetch())
	{
		st_TileMapInfo TileMapInfo;

		if (MapTileAllocFree == true)
		{
			TileMapInfo.MapTileType = en_MapTileInfo::MAP_TILE_USER_ALLOC;

			TileMapInfo.AccountID = MapTileAccountID;
			TileMapInfo.PlayerID = MapTilePlayerID;
			TileMapInfo.TilePosition._X = MapTilePositionX;
			TileMapInfo.TilePosition._Y = MapTilePositionY;

			TileMapInfos.push_back(TileMapInfo);
		}
	}

	G_DBConnectionPool->Push(en_DBConnect::GAME, GetTileMapInfoAllocFreeDBConnection);

	_TileMapInfos = new st_TileMapInfo * [YCount];

	for (int i = 0; i < YCount; i++)
	{
		_TileMapInfos[i] = new st_TileMapInfo[XCount];
	}

	// DB TileMap ������ ���� Ÿ�� ��� �о��
	wstring TileInfoInfoData = MapName + L"TileInfoData.txt";

	char* TileInfoDataFileStr = FileUtils::LoadFile(TileInfoInfoData.c_str());
	char* TileInfoDataMovingFileP = TileInfoDataFileStr;
	char* TileInfoDataConvertP = TileInfoDataFileStr;

	for (int Y = 0; Y < YCount; Y++)
	{
		for (int X = 0; X < XCount; X++)
		{
			if (*TileInfoDataConvertP == '\r')
			{
				break;
			}

			_TileMapInfos[Y][X].MapTileType = (en_MapTileInfo)(*TileInfoDataConvertP - 48);
			_TileMapInfos[Y][X].AccountID = 0;
			_TileMapInfos[Y][X].PlayerID = 0;

			_TileMapInfos[Y][X].TilePosition._Y = _Down - Y;
			_TileMapInfos[Y][X].TilePosition._X = X + _Left;

			TileInfoDataConvertP++;
		}

		TileInfoDataConvertP += 2;
	}

	for (st_TileMapInfo TileMapInfo : TileMapInfos)
	{
		_TileMapInfos[TileMapInfo.TilePosition._Y][TileMapInfo.TilePosition._X].MapTileType = TileMapInfo.MapTileType;
		_TileMapInfos[TileMapInfo.TilePosition._Y][TileMapInfo.TilePosition._X].AccountID = TileMapInfo.AccountID;
		_TileMapInfos[TileMapInfo.TilePosition._Y][TileMapInfo.TilePosition._X].PlayerID = TileMapInfo.PlayerID;
	}
#pragma endregion

	_ObjectsInfos = new CGameObject * *[YCount];

	for (int i = 0; i < YCount; i++)
	{
		_ObjectsInfos[i] = new CGameObject * [XCount];
		for (int j = 0; j < XCount; j++)
		{
			_ObjectsInfos[i][j] = nullptr;
		}
	}

	_SeedObjectInfos = new CGameObject * *[YCount];

	for (int i = 0; i < YCount; i++)
	{
		_SeedObjectInfos[i] = new CGameObject * [XCount];
		for (int j = 0; j < XCount; j++)
		{
			_SeedObjectInfos[i][j] = nullptr;
		}
	}

	_Items = new CItem * **[YCount];

	for (int i = 0; i < YCount; i++)
	{
		_Items[i] = new CItem * *[XCount];
		for (int j = 0; j < XCount; j++)
		{
			_Items[i][j] = new CItem * [(int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX];

			for (int8 k = 0; k < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; k++)
			{
				_Items[i][j][k] = nullptr;
			}
		}
	}

	_SizeX = _Right - _Left + 1;
	_SizeY = _Down - _Up + 1;

	_ChannelManager = new CChannelManager();
	_ChannelManager->Init(this, ChannelCount);

	_SectorSize = SectorSize;

	// �� ũ�⸦ ���� ���Ͱ� ���� ���� ��� �ִ��� Ȯ��
	_SectorCountY = (_SizeY + _SectorSize - 1) / _SectorSize;
	_SectorCountX = (_SizeX + _SectorSize - 1) / _SectorSize;

	// �ռ� ���� ���� ���� ������ ���� �����Ҵ� 
	_Sectors = new CSector * [_SectorCountY];

	for (int32 i = 0; i < _SectorCountY; i++)
	{
		_Sectors[i] = new CSector[_SectorCountX];
	}

	for (int32 Y = 0; Y < _SectorCountY; Y++)
	{
		for (int32 X = 0; X < _SectorCountX; X++)
		{
			// ���� ����
			_Sectors[Y][X] = CSector(Y, X);
		}
	}
}

CSector* CMap::GetSector(st_Vector2Int CellPosition)
{
	int X = (CellPosition._X - _Left) / _SectorSize;
	int Y = (_Down - CellPosition._Y) / _SectorSize;

	return GetSector(Y, X);
}

CSector* CMap::GetSector(int32 IndexY, int32 IndexX)
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

vector<CSector*> CMap::GetAroundSectors(st_Vector2Int CellPosition, int32 Range)
{
	CSector* Sector = GetSector(CellPosition);

	if (Sector != nullptr)
	{
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
}

vector<st_FieldOfViewInfo> CMap::GetFieldOfViewObjects(CGameObject* Object, int16 Range, bool ExceptMe)
{
	vector<st_FieldOfViewInfo> FieldOfViewGameObjects;

	vector<CSector*> Sectors = GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Range);

	for (CSector* Sector : Sectors)
	{
		Sector->AcquireSectorLock();

		st_FieldOfViewInfo FieldOfViewInfo;
		// �ֺ� ���� �÷��̾� ����
		for (CPlayer* Player : Sector->GetPlayers())
		{
			if (Player->_NetworkState == en_ObjectNetworkState::LIVE)
			{
				FieldOfViewInfo.ObjectID = Player->_GameObjectInfo.ObjectId;
				FieldOfViewInfo.SessionID = Player->_SessionId;
				FieldOfViewInfo.ObjectType = Player->_GameObjectInfo.ObjectType;

				float Distance = st_Vector2::Distance(Object->_GameObjectInfo.ObjectPositionInfo.Position, Player->_GameObjectInfo.ObjectPositionInfo.Position);

				// �Լ� ȣ���� ������Ʈ�� ������ �������� ���� ���� true�� ���� false�� ����
				if (ExceptMe == true && Distance <= Object->_FieldOfViewDistance)
				{
					if (Object->_GameObjectInfo.ObjectId != Player->_GameObjectInfo.ObjectId)
					{
						FieldOfViewGameObjects.push_back(FieldOfViewInfo);
					}
				}
				else if (ExceptMe == false && Distance <= Object->_FieldOfViewDistance)
				{
					FieldOfViewGameObjects.push_back(FieldOfViewInfo);
				}
			}
		}

		// �ֺ� ���� ���� ����
		for (CMonster* Monster : Sector->GetMonsters())
		{
			FieldOfViewInfo.ObjectID = Monster->_GameObjectInfo.ObjectId;
			FieldOfViewInfo.SessionID = 0;
			FieldOfViewInfo.ObjectType = Monster->_GameObjectInfo.ObjectType;

			int16 Distance = st_Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Monster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

			if (Distance <= Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjects.push_back(FieldOfViewInfo);
			}
		}

		for (CEnvironment* Environment : Sector->GetEnvironment())
		{
			FieldOfViewInfo.ObjectID = Environment->_GameObjectInfo.ObjectId;
			FieldOfViewInfo.SessionID = 0;
			FieldOfViewInfo.ObjectType = Environment->_GameObjectInfo.ObjectType;

			int16 Distance = st_Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Environment->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

			if (Distance <= Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjects.push_back(FieldOfViewInfo);
			}
		}

		for (CCraftingTable* CraftingTable : Sector->GetCraftingTable())
		{
			FieldOfViewInfo.ObjectID = CraftingTable->_GameObjectInfo.ObjectId;
			FieldOfViewInfo.SessionID = 0;
			FieldOfViewInfo.ObjectType = CraftingTable->_GameObjectInfo.ObjectType;

			int16 Distance = st_Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, CraftingTable->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

			if (Distance <= Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjects.push_back(FieldOfViewInfo);
			}
		}

		for (CItem* Item : Sector->GetItems())
		{
			FieldOfViewInfo.ObjectID = Item->_GameObjectInfo.ObjectId;
			FieldOfViewInfo.SessionID = 0;
			FieldOfViewInfo.ObjectType = Item->_GameObjectInfo.ObjectType;

			int16 Distance = st_Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Item->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

			if (Distance <= Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjects.push_back(FieldOfViewInfo);
			}
		}

		for (CCrop* Crop : Sector->GetCrop())
		{
			FieldOfViewInfo.ObjectID = Crop->_GameObjectInfo.ObjectId;
			FieldOfViewInfo.SessionID = 0;
			FieldOfViewInfo.ObjectType = Crop->_GameObjectInfo.ObjectType;

			int16 Distance = st_Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Crop->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

			if (Distance <= Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjects.push_back(FieldOfViewInfo);
			}
		}

		Sector->ReleaseSectorLock();
	}

	return FieldOfViewGameObjects;
}

vector<st_FieldOfViewInfo> CMap::GetFieldOfViewPlayers(CGameObject* Object, int16 Range, bool ExceptMe)
{
	vector<st_FieldOfViewInfo> FieldOfViewGamePlayers;

	vector<CSector*> Sectors = GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Range);

	st_FieldOfViewInfo FieldOfViewInfo;

	for (CSector* Sector : Sectors)
	{
		Sector->AcquireSectorLock();

		// �ֺ� ���� �÷��̾� ����
		for (CPlayer* Player : Sector->GetPlayers())
		{
			if (Player->_NetworkState == en_ObjectNetworkState::LIVE)
			{
				FieldOfViewInfo.ObjectID = Player->_GameObjectInfo.ObjectId;
				FieldOfViewInfo.SessionID = Player->_SessionId;
				FieldOfViewInfo.ObjectType = Player->_GameObjectInfo.ObjectType;

				int16 Distance = st_Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

				// �Լ� ȣ���� ������Ʈ�� ������ �������� ���� ���� true�� ���� false�� ����
				if (ExceptMe == true && Distance <= Object->_FieldOfViewDistance)
				{
					if (Object->_GameObjectInfo.ObjectId != Player->_GameObjectInfo.ObjectId)
					{
						FieldOfViewGamePlayers.push_back(FieldOfViewInfo);
					}
				}
				else if (ExceptMe == false && Distance <= Object->_FieldOfViewDistance)
				{
					FieldOfViewGamePlayers.push_back(FieldOfViewInfo);
				}
			}
		}

		Sector->ReleaseSectorLock();
	}

	return FieldOfViewGamePlayers;
}

vector<st_FieldOfViewInfo> CMap::GetFieldOfViewAttackObjects(CGameObject* Object, int16 Distance)
{
	vector<st_FieldOfViewInfo> FieldOfViewGameObjects;

	vector<CSector*> Sectors = GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Distance);

	for (CSector* Sector : Sectors)
	{
		Sector->AcquireSectorLock();

		st_FieldOfViewInfo FieldOfViewInfo;
		for (CPlayer* Player : Sector->GetPlayers())
		{
			if (Player->_NetworkState == en_ObjectNetworkState::LIVE)
			{
				FieldOfViewInfo.ObjectID = Player->_GameObjectInfo.ObjectId;
				FieldOfViewInfo.SessionID = Player->_SessionId;
				FieldOfViewInfo.ObjectType = Player->_GameObjectInfo.ObjectType;

				float Distance = st_Vector2::Distance(Object->_GameObjectInfo.ObjectPositionInfo.Position, Player->_GameObjectInfo.ObjectPositionInfo.Position);

				if (Distance <= Object->_FieldOfViewDistance)
				{
					if (Object->_GameObjectInfo.ObjectId != Player->_GameObjectInfo.ObjectId)
					{
						FieldOfViewGameObjects.push_back(FieldOfViewInfo);
					}
				}
			}
		}

		for (CMonster* Monster : Sector->GetMonsters())
		{
			FieldOfViewInfo.ObjectID = Monster->_GameObjectInfo.ObjectId;
			FieldOfViewInfo.SessionID = 0;
			FieldOfViewInfo.ObjectType = Monster->_GameObjectInfo.ObjectType;

			int16 Distance = st_Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Monster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

			if (Distance <= Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjects.push_back(FieldOfViewInfo);
			}
		}

		Sector->ReleaseSectorLock();
	}

	return FieldOfViewGameObjects;
}

vector<CMonster*> CMap::GetAroundMonster(CGameObject* Object, int16 Range, bool ExceptMe)
{
	vector<CMonster*> Monsters;

	vector<CSector*> Sectors = GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Range);

	for (CSector* Sector : Sectors)
	{
		Sector->AcquireSectorLock();

		for (CMonster* Monster : Sector->GetMonsters())
		{
			Monsters.push_back(Monster);
		}

		Sector->ReleaseSectorLock();
	}

	return Monsters;
}

vector<CPlayer*> CMap::GetFieldOfViewPlayer(CGameObject* Object, int16 Range, bool ExceptMe)
{
	vector<CPlayer*> FieldOfViewPlayers;

	vector<CSector*> Sectors = GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Range);

	for (CSector* Sector : Sectors)
	{
		// �ֺ� ���� �÷��̾� ����
		for (CPlayer* Player : Sector->GetPlayers())
		{
			int16 Distance = st_Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

			// �Լ� ȣ���� ������Ʈ�� ������ �������� ���� ���� true�� ���� false�� ����
			if (ExceptMe == true && Distance <= Object->_FieldOfViewDistance)
			{
				if (Object->_GameObjectInfo.ObjectId != Player->_GameObjectInfo.ObjectId)
				{
					FieldOfViewPlayers.push_back(Player);
				}
			}
			else if (ExceptMe == false && Distance <= Object->_FieldOfViewDistance)
			{
				FieldOfViewPlayers.push_back(Player);
			}
		}
	}

	return FieldOfViewPlayers;
}

CGameObject* CMap::MonsterReqFindNearPlayer(CMonster* Monster, en_MonsterAggroType* AggroType, int32 Range, bool* CollisionCango)
{
	// ���� �þ� ���� �ȿ� �ִ� �÷��̾���� �޾ƿ´�.
	vector<CPlayer*> Players = GetFieldOfViewPlayer(Monster, Range);

	// �޾ƿ� �÷��̾� ������ ���� �Ÿ��� ���ؼ� �켱���� ť�� ��´�.
	CHeap<int16, CPlayer*> Distances((int32)Players.size()); // ����� ������� 
	for (CPlayer* Player : Players)
	{
		if (Player->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::READY_DEAD && Player->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			Distances.InsertHeap(st_Vector2Int::Distance(Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Monster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition), Player);
		}
	}

	CPlayer* Player = nullptr;
	// �Ÿ� ����� �ֺ��� �����ؼ� �ش� �÷��̾�� �� �� �ִ��� Ȯ���ϰ�
	// �� �� �ִ� ����̸� �ش� �÷��̾ ��ȯ���ش�.
	if (Distances.GetUseSize() != 0)
	{
		Player = Distances.PopHeap();

		*AggroType = en_MonsterAggroType::MONSTER_AGGRO_FIRST_TARGET;

		vector<st_Vector2Int> FirstPaths = FindPath(Monster, Monster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
		// Ÿ���� ������ ������ ���� ���� ( ������ ������Ʈ��� ������ )
		if (FirstPaths.size() < 2)
		{
			if (Player != nullptr)
			{
				// ���� �ٸ� ����߿��� �� �� �ִ� ����� �ִ��� �߰��� �Ǵ�
				while (Distances.GetUseSize() != 0)
				{
					Player = Distances.PopHeap();
					vector<st_Vector2Int> SecondPaths = FindPath(Monster, Monster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
					if (SecondPaths.size() < 2)
					{
						continue;
					}

					*AggroType = en_MonsterAggroType::MONSTER_AGGRO_SECOND_TARGET;
					*CollisionCango = true;

					break;
				}

				*CollisionCango = false;
			}
		}
		else
		{
			*CollisionCango = true;
		}
	}
	else
	{
		*CollisionCango = false;
	}

	// �÷��̾ ã������ ���°��� SPAWN_IDLE�� ��쿡�� �����Ѵ�.
	if (Player != nullptr && Player->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::SPAWN_IDLE)
	{
		Player = nullptr;
	}

	return Player;
}

CGameObject* CMap::Find(st_Vector2Int& CellPosition)
{
	// �¿� ��ǥ �˻�
	if (CellPosition._X < _Left || CellPosition._X > _Right)
	{
		return nullptr;
	}

	// ���� ��ǥ �˻�
	if (CellPosition._Y < _Up || CellPosition._Y > _Down)
	{
		return nullptr;
	}

	int X = CellPosition._X - _Left;
	int Y = _Down - CellPosition._Y;

	return _ObjectsInfos[Y][X];
}

CGameObject* CMap::FindPlant(st_Vector2Int& PlantPosition)
{
	// �¿� ��ǥ �˻�
	if (PlantPosition._X < _Left || PlantPosition._X > _Right)
	{
		return nullptr;
	}

	// ���� ��ǥ �˻�
	if (PlantPosition._Y < _Up || PlantPosition._Y > _Down)
	{
		return nullptr;
	}

	int X = PlantPosition._X - _Left;
	int Y = _Down - PlantPosition._Y;

	return _SeedObjectInfos[Y][X];
}

CItem** CMap::FindItem(st_Vector2Int& ItemCellPosition)
{
	int32 X = ItemCellPosition._X - _Left;
	int32 Y = _Down - ItemCellPosition._Y;

	// �¿� ��ǥ �˻�
	if (ItemCellPosition._X < _Left || ItemCellPosition._X > _Right)
	{
		return nullptr;
	}

	// ���� ��ǥ �˻�
	if (ItemCellPosition._X < _Left || ItemCellPosition._X > _Right)
	{
		return nullptr;
	}

	int XCount = _Right - _Left + 1;
	int YCount = _Down - _Up + 1;

	return _Items[Y][X];
}

vector<st_TileMapInfo> CMap::FindMapTileInfo(CGameObject* Player)
{
	vector<st_TileMapInfo> TileInfos;

	int X = Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X - _Left;
	int Y = _Down - Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y;
		
	// ĳ���� ��ġ���� �����¿� 20 ũ�� ��ŭ Ÿ�� ��w�� ��ȯ
	int LeftX = X - 20;
	int UpY = Y + 20;
	int RightX = X + 20;
	int DownY = Y - 20;	

	for (; LeftX < RightX; LeftX++)
	{
		UpY = Y + 10;

		for (; UpY > DownY; UpY--)
		{
			if (_TileMapInfos[UpY][LeftX].TilePosition._X >= 0 && _TileMapInfos[UpY][LeftX].TilePosition._Y >= 0)
			{
				TileInfos.push_back(_TileMapInfos[UpY][LeftX]);
			}			
		}
	}

	return TileInfos;
}

bool CMap::Cango(CGameObject* Object, float X, float Y)
{
	st_Vector2Int CollisionPosition;
	CollisionPosition._X = X;
	CollisionPosition._Y = Y;

	return CollisionCango(Object, CollisionPosition);
}

bool CMap::CollisionCango(CGameObject* Object, st_Vector2Int& CellPosition, bool CheckObjects)
{
	// �¿� ��ǥ �˻�
	if (CellPosition._X < _Left || CellPosition._X > _Right)
	{
		return false;
	}

	// ���� ��ǥ �˻�
	if (CellPosition._Y < _Up || CellPosition._Y > _Down)
	{
		return false;
	}

	int X = CellPosition._X - _Left;
	int Y = _Down - CellPosition._Y;

	//G_Logger->WriteStdOut(en_Color::RED, L"X : %d Y : %d \n", X, Y);

	bool IsCollisionMapInfo = false;
	switch (_CollisionMapInfos[Y][X])
	{
	case en_MapObjectInfo::TILE_MAP_NONE:
	case en_MapObjectInfo::TILE_MAP_TREE:
	case en_MapObjectInfo::TILE_MAP_STONE:
	case en_MapObjectInfo::TILE_MAP_SLIME:
	case en_MapObjectInfo::TILE_MAP_BEAR:
	case en_MapObjectInfo::TILE_MAP_FURNACE:
	case en_MapObjectInfo::TILE_MAP_SAMILL:
	case en_MapObjectInfo::TILE_MAP_POTATO:
		IsCollisionMapInfo = true;
		break;
	case en_MapObjectInfo::TILE_MAP_WALL:
		IsCollisionMapInfo = false;
	}

	bool ObjectCheck = false;

	switch (Object->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
		ObjectCheck = Object->GetChannel()->ChannelColliderCheck(Object);
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
	case en_GameObjectType::OBJECT_SLIME:
		ObjectCheck = false;

		// ������Ʈ ��ġ �迭�� ��� ���� ��� true ��ȯ
		if (_ObjectsInfos[Y][X] == nullptr)
		{
			ObjectCheck = true;
		}
		else
		{
			// ��� ������ ������ �ȿ� �ִ� ������Ʈ�� �̵��ϰ��� �ϴ� ������Ʈ�� ���
			if (_ObjectsInfos[Y][X]->_GameObjectInfo.ObjectId == Object->_GameObjectInfo.ObjectId)
			{
				ObjectCheck = true;
			}
			else // �ȿ� �ִ� ������Ʈ�� �ٸ� ������Ʈ�� ���
			{
				ObjectCheck = false;
			}
		}
		break;
	}

	return IsCollisionMapInfo && (!CheckObjects || ObjectCheck);
}

bool CMap::ApplyMove(CGameObject* GameObject, st_Vector2Int& DestPosition, bool CheckObject, bool Applycollision)
{
	// ä���� �Ҵ�Ǿ� �ִ��� Ȯ��
	if (GameObject->GetChannel() == nullptr)
	{
		CRASH("ApplyMove GameObject Channel nullptr")
			return false;
	}

	// ���ӿ�����Ʈ�� ���� ä���� ��� �ִ� �ʰ� ���� ���� ������ Ȯ��
	if (GameObject->GetChannel()->GetMap() != this)
	{
		CRASH("ApplyMove GameObject�� ä���� ������ �ִ� �ʰ� ���� ���� �ٸ�")
			return false;
	}

	// ��ġ ���� ������ �´�.
	st_PositionInfo PositionInfo = GameObject->_GameObjectInfo.ObjectPositionInfo;

	switch (GameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		{
			// �������� �� �� �ִ��� �˻��Ѵ�.
			if (CollisionCango(GameObject, DestPosition, CheckObject) == false)
			{
				//G_Logger->WriteStdOut(en_Color::RED, L"Cant Go ApplyMove Y (%d) X (%d) ",DestPosition._Y,DestPosition._X);
				return false;
			}

			// ȣ������ ����� �浹ü�� ����ٸ�
			if (Applycollision == true)
			{
				int X = PositionInfo.CollisionPosition._X - _Left;
				int Y = _Down - PositionInfo.CollisionPosition._Y;

				// ������ġ �����ʹ� ������
				if (_ObjectsInfos[Y][X] == GameObject)
				{
					_ObjectsInfos[Y][X] = nullptr;
				}

				// ������ ��ġ �����ְ�
				X = DestPosition._X - _Left;
				Y = _Down - DestPosition._Y;

				// �������� �־��ش�.
				_ObjectsInfos[Y][X] = GameObject;

				int16 Width = GameObject->_GameObjectInfo.ObjectWidth;
				int16 Height = GameObject->_GameObjectInfo.ObjectHeight;

				for (int16 WidthX = 0; WidthX < Width; WidthX++)
				{
					for (int16 HeightY = 0; HeightY < Height; HeightY++)
					{
						_ObjectsInfos[Y + HeightY][X + WidthX] = GameObject;
					}
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_CORN:
		{
			CItem* ItemObject = (CItem*)GameObject;

			// �̵� �ϱ��� �־��� ��ǥ ���
			int32 PreviousX = ItemObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X - _Left;
			int32 PreviousY = _Down - ItemObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y;

			// �̵��ϰ��� �ϴ� ��ǥ ���
			int32 X = DestPosition._X - _Left;
			int32 Y = _Down - DestPosition._Y;

			// ���� ��ġ���� �����̴� ����� �����ϰ�
			for (int8 i = 0; i < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; i++)
			{
				if (_Items[PreviousY][PreviousX][i] == ItemObject)
				{
					_Items[PreviousY][PreviousX][i] = nullptr;
				}
			}

			// �켱 �ش� ��ġ�� �����۵�� ���� ���� �������� ������ ���Ѵ�.
			// ���� ���� �������� ������ �̹� �ش� ��ġ�� ���� ��쿡
			// ī��Ʈ�� ������Ű�� ���� ���� �������� �ʿ� ������Ű�� �ʴ´�.
			for (int8 i = 0; i < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; i++)
			{
				if (_Items[Y][X][i] != nullptr && _Items[Y][X][i]->_ItemInfo.ItemSmallCategory == ItemObject->_ItemInfo.ItemSmallCategory)
				{
					_Items[Y][X][i]->_ItemInfo.ItemCount += ItemObject->_ItemInfo.ItemCount;

					CItem* FindItem = (CItem*)(ItemObject->GetChannel()->FindChannelObject(_Items[Y][X][i]->_ItemInfo.ItemDBId, en_GameObjectType::OBJECT_ITEM));
					if (FindItem != nullptr)
					{
						FindItem->_ItemInfo.ItemCount = _Items[Y][X][i]->_ItemInfo.ItemCount;
					}

					return false;
				}
			}

			// ���� ���� �������� ������ �ش� ��ġ�� ���ٸ�
			// �� �ڸ�(Index)�� ã�´�.
			int NewItemInfoIndex = -1;
			for (int8 i = 0; i < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; i++)
			{
				if (_Items[Y][X][i] == nullptr)
				{
					NewItemInfoIndex = i;
					break;
				}
			}

			// �������� �����Ѵ�.
			_Items[Y][X][NewItemInfoIndex] = ItemObject;
		}
		break;
	case en_GameObjectType::OBJECT_CROP_POTATO:	
	case en_GameObjectType::OBJECT_CROP_CORN:
		{
			CCrop* CropObject = (CCrop*)GameObject;

			int32 X = DestPosition._X - _Left;
			int32 Y = _Down - DestPosition._Y;

			_SeedObjectInfos[Y][X] = GameObject;
		}
		break;
	}

	CSector* CurrentSector = GetSector(GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
	CSector* NextSector = GetSector(DestPosition);
	if (CurrentSector != NextSector)
	{
		CurrentSector->Remove(GameObject);
		NextSector->Insert(GameObject);
	}

	GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X = DestPosition._X;
	GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y = DestPosition._Y;

	return true;
}

bool CMap::ApplyLeave(CGameObject* GameObject)
{
	if (GameObject->GetChannel() == nullptr)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyLeave Channel is nullptr");
		return false;
	}

	if (GameObject->GetChannel()->GetMap() != this)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyLeave Channel _Map Error");
		return false;
	}

	// �¿� ��ǥ �˻�
	if (GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X < _Left || GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X > _Right)
	{
		return false;
	}

	// ���� ��ǥ �˻�
	if (GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y < _Up || GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y > _Down)
	{
		return false;
	}

	// ���Ϳ��� ������Ʈ ����
	CSector* Sector = GetSector(GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
	Sector->Remove(GameObject);

	int X = GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X - _Left;
	int Y = _Down - GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y;

	int Width = GameObject->_GameObjectInfo.ObjectWidth;
	int Height = GameObject->_GameObjectInfo.ObjectHeight;

	switch (GameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	case en_GameObjectType::OBJECT_SLIME:
		{
			// �ʿ��� ����
			if (_ObjectsInfos[Y][X] == GameObject)
			{
				for (int16 WidthX = 0; WidthX < Width; WidthX++)
				{
					for (int16 HeightY = 0; HeightY < Height; HeightY++)
					{
						_ObjectsInfos[Y][X] = nullptr;
					}
				}
			}
			else
			{
				if (GameObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
				{
					//CRASH("ApplyLeave �����Ϸ��� ������Ʈ�� ����Ǿ� �ִ� ������Ʈ�� �ٸ�");
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_CROP_CORN:
	case en_GameObjectType::OBJECT_CROP_POTATO:
		{
			if (_SeedObjectInfos[Y][X] == GameObject)
			{
				for (int16 WidthX = 0; WidthX < Width; WidthX++)
				{
					for (int16 HeightY = 0; HeightY < Height; HeightY++)
					{
						_SeedObjectInfos[Y][X] = nullptr;
					}
				}				
			}
		}
		break;
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_CORN:
		{
			CItem* Item = (CItem*)GameObject;

			for (int8 i = 0; i < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; i++)
			{
				if (_Items[Y][X][i] != nullptr &&
					_Items[Y][X][i]->_ItemInfo.ItemSmallCategory == Item->_ItemInfo.ItemSmallCategory)
				{
					_Items[Y][X][i] = nullptr;
					break;
				}
			}
		}
		break;	
	}

	return true;
}

bool CMap::ApplyTileUserAlloc(CGameObject* ReqTileUserAllocObject, st_Vector2Int TileUserAllocPosition)
{
	// �¿� ��ǥ �˻�
	if (TileUserAllocPosition._X < _Left || TileUserAllocPosition._X > _Right)
	{
		return false;
	}

	// ���� ��ǥ �˻�
	if (TileUserAllocPosition._Y < _Up || TileUserAllocPosition._Y > _Down)
	{
		return false;
	}

	int X = TileUserAllocPosition._X - _Left;
	int Y = _Down - TileUserAllocPosition._Y;

	if (_TileMapInfos[Y][X].MapTileType == en_MapTileInfo::MAP_TILE_USER_ALLOC
		|| _TileMapInfos[Y][X].MapTileType == en_MapTileInfo::MAP_TILE_SYSTEM_ALLOC)
	{
		return false;
	}

	_TileMapInfos[Y][X].MapTileType = en_MapTileInfo::MAP_TILE_USER_ALLOC;

	return true;
}

bool CMap::ApplyTileUseFree(CGameObject* ReqTileUserFreeObject, st_Vector2Int TileUserFreePosition)
{
	// �¿� ��ǥ �˻�
	if (TileUserFreePosition._X < _Left || TileUserFreePosition._X > _Right)
	{
		return false;
	}

	// ���� ��ǥ �˻�
	if (TileUserFreePosition._Y < _Up || TileUserFreePosition._Y > _Down)
	{
		return false;
	}

	int X = TileUserFreePosition._X - _Left;
	int Y = _Down - TileUserFreePosition._Y;

	_TileMapInfos[Y][X].MapTileType = en_MapTileInfo::MAP_TILE_USER_FREE;

	return true;
}

st_PositionInt CMap::CellToPositionInt(st_Vector2Int CellPosition)
{
	return st_PositionInt(_Down - CellPosition._Y, CellPosition._X - _Left);
}

st_Vector2Int CMap::PositionToCellInt(st_PositionInt Position)
{
	return st_Vector2Int(Position._X + _Left, _Down - Position._Y);
}

vector<st_Vector2Int> CMap::FindPath(CGameObject* Object, st_Vector2Int StartCellPosition, st_Vector2Int DestCellPostion, bool CheckObjects, int32 MaxDistance)
{
	int32 DeltaY[4] = { 1, -1, 0, 0 };
	int32 DeltaX[4] = { 0, 0, -1, 1 };
	int32 Cost[4] = { 10,10,10,10 };

	// ������ ������ ��ǥ��ȯ
	st_PositionInt StartPosition = CellToPositionInt(StartCellPosition);
	st_PositionInt DestPosition = CellToPositionInt(DestCellPostion);

	// ������� �˻�迭
	bool** CloseList = (bool**)malloc(sizeof(bool) * _SizeY * _SizeX);
	for (int32 i = 0; i < _SizeY; i++)
	{
		CloseList[i] = (bool*)malloc(sizeof(bool) * _SizeX);
		memset(CloseList[i], false, sizeof(bool) * _SizeX);
	}

	// ������� �˻�迭
	int** OpenList = (int**)malloc(sizeof(int) * _SizeY * _SizeX);
	for (int32 i = 0; i < _SizeY; i++)
	{
		OpenList[i] = (int*)malloc(sizeof(int) * _SizeX);
		memset(OpenList[i], 1, sizeof(int) * _SizeX);
	}

	// �ڽ�, �θ� ��
	map<st_PositionInt, st_PositionInt> Parents;

	// �켱���� ť ����
	CHeap<int32, st_AStarNodeInt> OpenListQue(_SizeY * _SizeX);

	// ������忡 ó�� F �� ���
	OpenList[StartPosition._Y][StartPosition._X] = abs(DestPosition._Y - StartPosition._Y) + abs(DestPosition._X - StartPosition._X);

	// AStar Node ����
	st_AStarNodeInt StartNode(abs(DestPosition._Y - StartPosition._Y) + abs(DestPosition._X - StartPosition._X), 0, StartPosition._X, StartPosition._Y);
	// ť�� ����
	OpenListQue.InsertHeap(StartNode._F, StartNode);

	// ó�� ��ġ ù �θ�� ����
	Parents.insert(pair<st_PositionInt, st_PositionInt>(StartPosition, StartPosition));

	// ���¸���Ʈ ť�� ����������� �ݺ�
	while (OpenListQue.GetUseSize() > 0)
	{
		// ���¸���Ʈ ť���� ��� �ϳ� ����
		st_AStarNodeInt AStarNode = OpenListQue.PopHeap();
		// �ش� ��ġ�� �湮�߾����� Ȯ��
		if (CloseList[AStarNode._Position._Y][AStarNode._Position._X] == true)
		{
			continue;
		}

		// �ش� ��ġ�� �湮�ߴٰ� ���
		CloseList[AStarNode._Position._Y][AStarNode._Position._X] = true;

		// ���� ����� ��ġ�� ��������� ������
		if (AStarNode._Position._Y == DestPosition._Y && AStarNode._Position._X == DestPosition._X)
		{
			break;
		}

		// ���� ��带 �������� �ؼ� �� �� �� �� ��带 �˻��ϰ� ���¸���Ʈ ť�� �ִ´�.
		for (int32 i = 0; i < 4; i++)
		{
			// ���� ��ġ�� �˾Ƴ���.
			st_PositionInt NextPosition(AStarNode._Position._Y + DeltaY[i], AStarNode._Position._X + DeltaX[i]);

			// �������� �̾Ƴ� ��ġ�� ���������� �������� ������ �ʹ� �ִٸ� �ش� ��ǥ�� �����Ѵ�.
			if (abs(StartPosition._Y - NextPosition._Y) + abs(StartPosition._X - NextPosition._X) > MaxDistance)
			{
				continue;
			}

			// ���� ��ġ�� ������ ��ǥ�� �ƴ� ���, �ش� ��ġ�� �� �� �ִ��� �˻��Ѵ�.
			if (NextPosition._Y != DestPosition._Y || NextPosition._X != DestPosition._X)
			{
				st_Vector2Int NextPositionVector = PositionToCellInt(NextPosition);

				// �� �� ������ ���� ��ġ�� �˻��Ѵ�.
				if (CollisionCango(Object, NextPositionVector, CheckObjects) == false)
				{
					continue;
				}
			}

			// �� �� �� �� ��ġ�� �̹� �湮 �߾����� Ȯ���Ѵ�.
			if (CloseList[NextPosition._Y][NextPosition._X] == true)
			{
				continue;
			}

			// G ���� ����Ѵ�.
			int G = AStarNode._G + Cost[i];
			// ���� ��ġ�� ������ ��ġ�� ����Ѵ�.
			int H = abs(DestPosition._Y - NextPosition._Y) + abs(DestPosition._X - NextPosition._X);

			// ���� ����� F ���� ���� �־��� F ������ ���� ��� ���� ����Ѵ�.
			if (G + H > OpenList[NextPosition._Y][NextPosition._X])
			{
				continue;
			}

			// ������ ��带 �غ��ϰ�
			st_AStarNodeInt InsertAStartNode;
			// �ռ� ���� ������ �̿��� F���� ������ �����Ѵ�.
			OpenList[NextPosition._Y][NextPosition._X] = G + H;

			// ������ ��带 �ʱ�ȭ �� �Ŀ�
			InsertAStartNode._F = G + H;
			InsertAStartNode._G = G;
			InsertAStartNode._Position._X = NextPosition._X;
			InsertAStartNode._Position._Y = NextPosition._Y;

			// �켱���� ť�� �ִ´�.
			OpenListQue.InsertHeap(InsertAStartNode._F, InsertAStartNode);
			// �θ� ��Ͽ� ���� ��ġ�� �ִ��� Ȯ��
			auto ChildFind = Parents.find(NextPosition);
			if (ChildFind == Parents.end())
			{
				//ã�� �������� �־��ְ�
				Parents.insert(pair<st_PositionInt, st_PositionInt>(NextPosition, AStarNode._Position));
			}
			else
			{
				//�̹� �־����� �θ� ��带 �ٲ��ش�.
				(*ChildFind).second = AStarNode._Position;
			}
		}
	}

	// ������ �Ҵ��� �޸𸮸� �����Ѵ�.
	for (int32 i = 0; i < _SizeY; i++)
	{
		free(CloseList[i]);
		free(OpenList[i]);
	}

	free(CloseList);
	free(OpenList);

	return CompletePath(Parents, DestPosition);
}

vector<st_Vector2Int> CMap::CompletePath(map<st_PositionInt, st_PositionInt> Parents, st_PositionInt DestPosition)
{
	// ��ȯ���� �迭
	vector<st_Vector2Int> Cells;

	int32 X = DestPosition._X;
	int32 Y = DestPosition._Y;

	st_PositionInt Point;

	// �θ� ��� �߿��� �������� ������
	if (Parents.find(DestPosition) == Parents.end())
	{
		st_PositionInt BestPosition;
		int32 BestDistance;
		memset(&BestDistance, 1, sizeof(int32));

		// ������ �ִ� ��ġ �߿��� ���� ����� ��ġ�� ã��
		// �ش� ��ġ�� �������� ��´�.
		for (auto Start = Parents.begin(); Start != Parents.end(); ++Start)
		{
			int32 Distance = abs(DestPosition._X - (*Start).first._X) + abs(DestPosition._Y - (*Start).first._Y);
			if (BestDistance > Distance)
			{
				BestPosition = (*Start).first;
				BestDistance = Distance;
			}
		}

		DestPosition = BestPosition;
	}

	st_PositionInt Position = DestPosition;
	while ((*Parents.find(Position)).second != Position)
	{
		Cells.push_back(PositionToCellInt(Position));
		Position = (*Parents.find(Position)).second;
	}

	// ������ ���
	Cells.push_back(PositionToCellInt(Position));
	// �θ� ��ġ ���� �迭 �Ųٷ� ����� ��ȯ
	reverse(Cells.begin(), Cells.end());

	return Cells;
}

CChannelManager* CMap::GetChannelManager()
{
	return _ChannelManager;
}
