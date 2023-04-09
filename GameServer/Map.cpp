#include "pch.h"
#include "Map.h"
#include "GameObject.h"
#include "Player.h"
#include "NonPlayer.h"
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
	_MapID = 0;

	_Left = 0;
	_Right = 0;
	_Up = 0;
	_Down = 0;

	_SizeX = 0;
	_SizeY = 0;

	_ObjectsInfos = nullptr;
	_SeedObjectInfos = nullptr;
	_Items = nullptr;

	_ChannelManager = nullptr;

	_Sectors = nullptr;
	_SectorSize = 0;
	_SectorCountX = 0;
	_SectorCountY = 0;
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

void CMap::MapInit()
{	
	int XCount = _Right - _Left + 1;
	int YCount = _Down - _Up + 1;

	int YCountXCount = YCount * XCount;	

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
	_ChannelManager->Init(this, _ChannelCount);	

	// 맵 크기를 토대로 섹터가 가로 세로 몇개씩 있는지 확인
	_SectorCountY = (_SizeY + _SectorSize - 1) / _SectorSize;
	_SectorCountX = (_SizeX + _SectorSize - 1) / _SectorSize;

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

CSector* CMap::GetSector(Vector2Int CellPosition)
{
	int X = (CellPosition.X - _Left) / _SectorSize;
	int Y = (_Down - CellPosition.Y) / _SectorSize;

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

vector<CSector*> CMap::GetAroundSectors(Vector2Int CellPosition, int32 Range)
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

vector<st_FieldOfViewInfo> CMap::GetFieldOfViewObjects(CGameObject* Object)
{
	vector<st_FieldOfViewInfo> FieldOfViewGameObjects;

	// 오브젝트 기준으로 주위 섹터를 가져옴
	vector<CSector*> Sectors = GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, 1);

	st_FieldOfViewInfo FieldOfViewInfo;

	for (CSector* Sector : Sectors)
	{
		Sector->AcquireSectorLock();

		for (CPlayer* Player : Sector->GetPlayers())
		{			
			if (Vector2::CheckFieldOfView(Player->_GameObjectInfo.ObjectPositionInfo.Position,
				Object->_GameObjectInfo.ObjectPositionInfo.Position, Object->_FieldOfDirection, Object->_FieldOfAngle, Object->_FieldOfViewDistance)
				&& Player->_GameObjectInfo.ObjectId != Object->_GameObjectInfo.ObjectId)
			{
				FieldOfViewInfo.ObjectID = Player->_GameObjectInfo.ObjectId;
				FieldOfViewInfo.SessionID = 0;
				FieldOfViewInfo.ObjectType = Player->_GameObjectInfo.ObjectType;

				FieldOfViewGameObjects.push_back(FieldOfViewInfo);				
			}
		}

		for (CNonPlayer* NonPlayer : Sector->GetNonPlayers())
		{
			if (Vector2::CheckFieldOfView(NonPlayer->_GameObjectInfo.ObjectPositionInfo.Position,
				Object->_GameObjectInfo.ObjectPositionInfo.Position, Object->_FieldOfDirection, Object->_FieldOfAngle, Object->_FieldOfViewDistance)
				&& NonPlayer->_GameObjectInfo.ObjectId != Object->_GameObjectInfo.ObjectId)
			{
				FieldOfViewInfo.ObjectID = NonPlayer->_GameObjectInfo.ObjectId;
				FieldOfViewInfo.SessionID = 0;
				FieldOfViewInfo.ObjectType = NonPlayer->_GameObjectInfo.ObjectType;

				FieldOfViewGameObjects.push_back(FieldOfViewInfo);
			}
		}
		
		for (CMonster* Monster : Sector->GetMonsters())
		{
			if (Vector2::CheckFieldOfView(Monster->_GameObjectInfo.ObjectPositionInfo.Position, 
				Object->_GameObjectInfo.ObjectPositionInfo.Position,
				Object->_FieldOfDirection, Object->_FieldOfAngle, Object->_FieldOfViewDistance))
			{
				FieldOfViewInfo.ObjectID = Monster->_GameObjectInfo.ObjectId;
				FieldOfViewInfo.SessionID = 0;
				FieldOfViewInfo.ObjectType = Monster->_GameObjectInfo.ObjectType;

				FieldOfViewGameObjects.push_back(FieldOfViewInfo);
			}
		}

		for (CEnvironment* Enviroment : Sector->GetEnvironment())
		{
			if (Enviroment->_GameObjectInfo.ObjectType != en_GameObjectType::OBJECT_WALL)
			{
				if (Vector2::CheckFieldOfView(Enviroment->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position, Object->_FieldOfDirection, Object->_FieldOfAngle, Object->_FieldOfViewDistance))
				{
					FieldOfViewInfo.ObjectID = Enviroment->_GameObjectInfo.ObjectId;
					FieldOfViewInfo.SessionID = 0;
					FieldOfViewInfo.ObjectType = Enviroment->_GameObjectInfo.ObjectType;

					FieldOfViewGameObjects.push_back(FieldOfViewInfo);
				}
			}			
		}		

		for (CGameObject* SkillObject : Sector->GetSkillObject())
		{
			if (Vector2::CheckFieldOfView(SkillObject->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position, Object->_FieldOfDirection, Object->_FieldOfAngle, Object->_FieldOfViewDistance))
			{
				FieldOfViewInfo.ObjectID = SkillObject->_GameObjectInfo.ObjectId;
				FieldOfViewInfo.SessionID = 0;
				FieldOfViewInfo.ObjectType = SkillObject->_GameObjectInfo.ObjectType;

				FieldOfViewGameObjects.push_back(FieldOfViewInfo);
			}
		}

		for (CCraftingTable* CraftingTable : Sector->GetCraftingTable())
		{
			if (Vector2::CheckFieldOfView(CraftingTable->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position, Object->_FieldOfDirection, Object->_FieldOfAngle, Object->_FieldOfViewDistance))
			{
				FieldOfViewInfo.ObjectID = CraftingTable->_GameObjectInfo.ObjectId;
				FieldOfViewInfo.SessionID = 0;
				FieldOfViewInfo.ObjectType = CraftingTable->_GameObjectInfo.ObjectType;

				FieldOfViewGameObjects.push_back(FieldOfViewInfo);
			}
		}	

		Sector->ReleaseSectorLock();
	}

	return FieldOfViewGameObjects;
}

vector<st_FieldOfViewInfo> CMap::GetFieldAroundPlayers(CGameObject* Object, bool ExceptMe)
{
	vector<st_FieldOfViewInfo> FieldOfViewGamePlayers;

	vector<CSector*> Sectors = GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, 1);

	st_FieldOfViewInfo FieldOfViewInfo;

	for (CSector* Sector : Sectors)
	{
		Sector->AcquireSectorLock();

		// 주변 섹터 플레이어 정보
		for (CPlayer* Player : Sector->GetPlayers())
		{
			if (Player->_NetworkState == en_ObjectNetworkState::LIVE)
			{
				FieldOfViewInfo.ObjectID = Player->_GameObjectInfo.ObjectId;
				FieldOfViewInfo.SessionID = Player->_SessionId;
				FieldOfViewInfo.ObjectType = Player->_GameObjectInfo.ObjectType;

				float Distance = Vector2::Distance(Object->_GameObjectInfo.ObjectPositionInfo.Position, Player->_GameObjectInfo.ObjectPositionInfo.Position);
				// 함수 호출한 오브젝트를 포함할 것인지에 대한 여부 true면 제외 false면 포함
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

vector<CPlayer*> CMap::GetAroundPlayers(CGameObject* Object, bool ExceptMe)
{
	vector<CPlayer*> FieldOfViewPlayers;

	vector<CSector*> Sectors = GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, 1);

	for (CSector* Sector : Sectors)
	{
		// 주변 섹터 플레이어 정보
		for (CPlayer* Player : Sector->GetPlayers())
		{
			int16 Distance = Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

			// 함수 호출한 오브젝트를 포함할 것인지에 대한 여부 true면 제외 false면 포함
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

				float Distance = Vector2::Distance(Object->_GameObjectInfo.ObjectPositionInfo.Position, Player->_GameObjectInfo.ObjectPositionInfo.Position);

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

			int16 Distance = Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Monster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

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

CGameObject* CMap::MonsterReqFindNearPlayer(CMonster* Monster, en_MonsterAggroType* AggroType, bool* CollisionCango)
{
	// 주위 시야 범위 안에 있는 플레이어들을 받아온다.
	vector<CPlayer*> Players = GetAroundPlayers(Monster, 1);

	// 받아온 플레이어 정보를 토대로 거리를 구해서 우선순위 큐에 담는다.
	CHeap<float, CPlayer*> Distances((int32)Players.size()); // 가까운 순서대로 
	for (CPlayer* Player : Players)
	{
		if (Player->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			float Distance = Vector2::Distance(Player->_GameObjectInfo.ObjectPositionInfo.Position, Monster->_GameObjectInfo.ObjectPositionInfo.Position);
			if (Distance > Monster->_PlayerSearchDistance)
			{
				continue;
			}

			Distances.InsertHeap(Distance, Player);
		}
	}

	CPlayer* Player = nullptr;
	// 거리 가까운 애부터 접근해서 해당 플레이어로 갈 수 있는지 확인하고
	// 갈 수 있는 대상이면 해당 플레이어를 반환해준다.
	if (Distances.GetUseSize() != 0)
	{
		Player = Distances.PopHeap();

		*AggroType = en_MonsterAggroType::MONSTER_AGGRO_FIRST_TARGET;

		vector<Vector2Int> FirstPaths = FindPath(Monster, Monster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
		// 타겟은 있지만 갈수는 없는 상태 ( 주위에 오브젝트들로 막혀서 )
		if (FirstPaths.size() < 2)
		{
			if (Player != nullptr)
			{
				// 주위 다른 대상중에서 갈 수 있는 대상이 있는지 추가로 판단
				while (Distances.GetUseSize() != 0)
				{
					Player = Distances.PopHeap();
					vector<Vector2Int> SecondPaths = FindPath(Monster, Monster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
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

	// 플레이어를 찾앗으나 상태값이 SPAWN_IDLE일 경우에는 무시한다.
	if (Player != nullptr && Player->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::SPAWN_IDLE)
	{
		Player = nullptr;
	}

	return Player;
}

CGameObject* CMap::Find(Vector2Int& CellPosition)
{
	// 좌우 좌표 검사
	if (CellPosition.X < _Left || CellPosition.X > _Right)
	{
		return nullptr;
	}

	// 상하 좌표 검사
	if (CellPosition.Y < _Up || CellPosition.Y > _Down)
	{
		return nullptr;
	}

	int X = CellPosition.X - _Left;
	int Y = _Down - CellPosition.Y;

	return _ObjectsInfos[Y][X];
}

CGameObject* CMap::FindPlant(Vector2Int& PlantPosition)
{
	// 좌우 좌표 검사
	if (PlantPosition.X < _Left || PlantPosition.X > _Right)
	{
		return nullptr;
	}

	// 상하 좌표 검사
	if (PlantPosition.Y < _Up || PlantPosition.Y > _Down)
	{
		return nullptr;
	}

	int X = PlantPosition.X - _Left;
	int Y = _Down - PlantPosition.Y;

	return _SeedObjectInfos[Y][X];
}

CItem** CMap::FindItem(Vector2Int& ItemCellPosition)
{
	int32 X = ItemCellPosition.X - _Left;
	int32 Y = _Down - ItemCellPosition.Y;

	// 좌우 좌표 검사
	if (ItemCellPosition.X < _Left || ItemCellPosition.X > _Right)
	{
		return nullptr;
	}

	// 상하 좌표 검사
	if (ItemCellPosition.X < _Left || ItemCellPosition.X > _Right)
	{
		return nullptr;
	}

	int XCount = _Right - _Left + 1;
	int YCount = _Down - _Up + 1;

	return _Items[Y][X];
}

bool CMap::Cango(CGameObject* Object, OUT Vector2* NextPosition)
{
	Vector2Int CollisionPosition;
	CollisionPosition.X = (int32)Object->_GameObjectInfo.ObjectPositionInfo.Position.X;
	CollisionPosition.Y = (int32)Object->_GameObjectInfo.ObjectPositionInfo.Position.Y;

	Vector2 CheckPosition; 
	CheckPosition.X = Object->_GameObjectInfo.ObjectPositionInfo.Position.X;
	CheckPosition.Y = Object->_GameObjectInfo.ObjectPositionInfo.Position.Y;
	
	Vector2 DirectionNormal = Object->_GameObjectInfo.ObjectPositionInfo.MoveDirection.Normalize();

	CheckPosition.Y += (DirectionNormal.Y * Object->_GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
	CheckPosition.X += (DirectionNormal.X * Object->_GameObjectInfo.ObjectStatInfo.Speed * 0.02f);

	bool NextPositionMoveCheck = MoveCollisionCango(Object, CollisionPosition, CheckPosition);
	NextPosition->X = CheckPosition.X;
	NextPosition->Y = CheckPosition.Y;

	return NextPositionMoveCheck;
}

bool CMap::MoveCollisionCango(CGameObject* Object, Vector2Int& CellPosition, Vector2& NextPosition, bool CheckObjects, CGameObject* CollisionObject)
{
	// 채널이 할당되어 있는지 확인
	if (Object->GetChannel() == nullptr)
	{
		CRASH("ApplyMove GameObject Channel nullptr")
			return false;
	}

	// 게임오브젝트가 속한 채널이 들고 있는 맵과 현재 맵이 같은지 확인
	if (Object->GetChannel()->GetMap() != this)
	{
		CRASH("ApplyMove GameObject의 채널이 가지고 있는 맵과 지금 맵이 다름")
			return false;
	}

	// 좌우 좌표 검사
	if (CellPosition.X < _Left 
		|| CellPosition.X > _Right)
	{
		return false;
	}

	// 상하 좌표 검사
	if (CellPosition.Y < _Up
		|| CellPosition.Y > _Down)
	{
		return false;
	}	

	bool ObjectCheck = Object->GetChannel()->ChannelColliderCheck(Object, NextPosition, &CollisionObject);

	return (!CheckObjects || ObjectCheck);
}



bool CMap::ApplyMove(CGameObject* GameObject, Vector2Int& DestPosition, bool CheckObject, bool Applycollision)
{
	// 채널이 할당되어 있는지 확인
	if (GameObject->GetChannel() == nullptr)
	{
		CRASH("ApplyMove GameObject Channel nullptr")
			return false;
	}

	// 게임오브젝트가 속한 채널이 들고 있는 맵과 현재 맵이 같은지 확인
	if (GameObject->GetChannel()->GetMap() != this)
	{
		CRASH("ApplyMove GameObject의 채널이 가지고 있는 맵과 지금 맵이 다름")
			return false;
	}

	// 좌우 좌표 검사
	if (GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X < _Left
		|| GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X > _Right)
	{
		return false;
	}

	// 상하 좌표 검사
	if (GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y < _Up
		|| GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y > _Down)
	{
		return false;
	}

	// 위치 정보 가지고 온다.
	st_PositionInfo PositionInfo = GameObject->_GameObjectInfo.ObjectPositionInfo;

	switch (GameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:	
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	case en_GameObjectType::OBJECT_NON_PLAYER_GENERAL_MERCHANT:
	case en_GameObjectType::OBJECT_GOBLIN:	
	case en_GameObjectType::OBJECT_WALL:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		{
			// 목적지로 갈 수 있는지 검사한다.
			//if (MoveCollisionCango(GameObject, DestPosition, CheckObject) == false)
			//{
			//	//G_Logger->WriteStdOut(en_Color::RED, L"Cant Go ApplyMove Y (%d) X (%d) ",DestPosition._Y,DestPosition._X);
			//	return false;
			//}

			// 호출해준 대상을 충돌체로 여긴다면
			if (Applycollision == true)
			{
				int X = PositionInfo.CollisionPosition.X - _Left;
				int Y = _Down - PositionInfo.CollisionPosition.Y;

				// 기존위치 데이터는 날리고
				if (_ObjectsInfos[Y][X] == GameObject)
				{
					_ObjectsInfos[Y][X] = nullptr;
				}

				// 목적지 위치 구해주고
				X = DestPosition.X - _Left;
				Y = _Down - DestPosition.Y;

				// 목적지에 넣어준다.
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
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_CORN:
		{
			CItem* ItemObject = (CItem*)GameObject;

			// 이동 하기전 있었던 좌표 계산
			int32 PreviousX = ItemObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X - _Left;
			int32 PreviousY = _Down - ItemObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y;

			// 이동하고자 하는 좌표 계산
			int32 X = DestPosition.X - _Left;
			int32 Y = _Down - DestPosition.Y;

			// 기존 위치에서 움직이는 대상은 제거하고
			for (int8 i = 0; i < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; i++)
			{
				if (_Items[PreviousY][PreviousX][i] == ItemObject)
				{
					_Items[PreviousY][PreviousX][i] = nullptr;
				}
			}

			// 우선 해당 위치에 아이템들과 새로 얻은 아이템의 종류를 비교한다.
			// 새로 얻은 아이템의 종류가 이미 해당 위치에 있을 경우에
			// 카운트를 증가시키고 새로 얻은 아이템은 맵에 스폰시키지 않는다.
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

			// 새로 얻은 아이템의 종류가 해당 위치에 없다면
			// 빈 자리(Index)를 찾는다.
			int NewItemInfoIndex = -1;
			for (int8 i = 0; i < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; i++)
			{
				if (_Items[Y][X][i] == nullptr)
				{
					NewItemInfoIndex = i;
					break;
				}
			}

			// 아이템을 저장한다.
			_Items[Y][X][NewItemInfoIndex] = ItemObject;
		}
		break;
	case en_GameObjectType::OBJECT_CROP_POTATO:
	case en_GameObjectType::OBJECT_CROP_CORN:
		{
			CCrop* CropObject = (CCrop*)GameObject;

			int32 X = DestPosition.X - _Left;
			int32 Y = _Down - DestPosition.Y;

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

	GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X = DestPosition.X;
	GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y = DestPosition.Y;

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

	// 좌우 좌표 검사
	if (GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X < _Left || GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X > _Right)
	{
		return false;
	}

	// 상하 좌표 검사
	if (GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y < _Up || GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y > _Down)
	{
		return false;
	}

	// 섹터에서 오브젝트 제거
	CSector* Sector = GetSector(GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
	Sector->Remove(GameObject);

	int X = GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X - _Left;
	int Y = _Down - GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y;

	int Width = GameObject->_GameObjectInfo.ObjectWidth;
	int Height = GameObject->_GameObjectInfo.ObjectHeight;

	switch (GameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:	
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	case en_GameObjectType::OBJECT_GOBLIN:	
	case en_GameObjectType::OBJECT_WALL:
		{
			// 맵에서 제거
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
					//CRASH("ApplyLeave 삭제하려는 오브젝트가 저장되어 있는 오브젝트와 다름");
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
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
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

bool CMap::ApplySkillObjectMove(CGameObject* SkillObject, Vector2Int& DestPosition, CGameObject** CollisionObject)
{
	if (SkillObject->GetChannel() == nullptr)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyLeave Channel is nullptr");
		return false;
	}

	if (SkillObject->GetChannel()->GetMap() != this)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyLeave Channel _Map Error");
		return false;
	}

	// 좌우 좌표 검사
	if (SkillObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X < _Left || SkillObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X > _Right)
	{
		return false;
	}

	// 상하 좌표 검사
	if (SkillObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y < _Up || SkillObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y > _Down)
	{
		return false;
	}	

	CSector* CurrentSector = GetSector(SkillObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
	CSector* NextSector = GetSector(DestPosition);
	if (CurrentSector != NextSector)
	{
		CurrentSector->Remove(SkillObject);
		NextSector->Insert(SkillObject);
	}

	SkillObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X = DestPosition.X;
	SkillObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y = DestPosition.Y;
	
	CGameObject* ColObject = nullptr;

	bool IsCollision = SkillObject->GetChannel()->ChannelColliderCheck(SkillObject, SkillObject->_GameObjectInfo.ObjectPositionInfo.Position, &ColObject);
	
	*CollisionObject = ColObject;

	return IsCollision;
}

bool CMap::MonsterCango(CGameObject* Object, OUT Vector2* NextPosition)
{
	Vector2 CheckPosition;
	CheckPosition.X = Object->_GameObjectInfo.ObjectPositionInfo.Position.X;
	CheckPosition.Y = Object->_GameObjectInfo.ObjectPositionInfo.Position.Y;

	Vector2 DirectionNormal = Object->_GameObjectInfo.ObjectPositionInfo.MoveDirection.Normalize();

	CheckPosition.Y += (DirectionNormal.Y * Object->_GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
	CheckPosition.X += (DirectionNormal.X * Object->_GameObjectInfo.ObjectStatInfo.Speed * 0.02f);

	Vector2Int CheckCollisionPosition;
	CheckCollisionPosition.X = (int32)CheckPosition.X;
	CheckCollisionPosition.Y = (int32)CheckPosition.Y;
		
	bool NextPositionMoveCheck = FindPathNextPositionCango(Object, CheckCollisionPosition);
	NextPosition->X = CheckPosition.X;
	NextPosition->Y = CheckPosition.Y;

	return NextPositionMoveCheck;
}

vector<Vector2Int> CMap::FindPath(CGameObject* Object, Vector2Int StartCellPosition, Vector2Int DestCellPostion, bool CheckObjects, int32 MaxDistance)
{
	// 상 하 좌 우 
	int32 DeltaY[4] = { 1, -1, 0, 0};
	int32 DeltaX[4] = { 0, 0, -1, 1};
	int32 Cost[4] = { 10,10,10,10};

	// 시작점 도착점 좌표변환
	Vector2Int StartPosition = StartCellPosition;
	Vector2Int DestPosition = DestCellPostion;

	// 닫힌노드 검사배열
	bool** CloseList = (bool**)malloc(sizeof(bool) * _SizeY * _SizeX);
	for (int32 i = 0; i < _SizeY; i++)
	{
		CloseList[i] = (bool*)malloc(sizeof(bool) * _SizeX);
		memset(CloseList[i], false, sizeof(bool) * _SizeX);
	}

	// 열린노드 검사배열
	int** OpenList = (int**)malloc(sizeof(int) * _SizeY * _SizeX);
	for (int32 i = 0; i < _SizeY; i++)
	{
		OpenList[i] = (int*)malloc(sizeof(int) * _SizeX);
		memset(OpenList[i], 1, sizeof(int) * _SizeX);
	}

	// 자식, 부모 맵
	map<Vector2Int, Vector2Int> Parents;

	// 우선순위 큐 생성
	CHeap<int32, st_AStarNodeInt> OpenListQue(_SizeY * _SizeX);

	// 열린노드에 처음 F 값 기록
	OpenList[StartPosition.Y][StartPosition.X] = abs(DestPosition.Y - StartPosition.Y) + abs(DestPosition.X - StartPosition.X);

	// AStar Node 생성
	st_AStarNodeInt StartNode(abs(DestPosition.Y - StartPosition.Y) + abs(DestPosition.X - StartPosition.X), 0, StartPosition.X, StartPosition.Y);
	// 큐에 삽입
	OpenListQue.InsertHeap(StartNode._F, StartNode);	

	// 처음 위치 첫 부모로 설정
	Parents.insert(pair<Vector2Int, Vector2Int>(StartPosition, StartPosition));

	// 오픈리스트 큐가 비워질때까지 반복
	while (OpenListQue.GetUseSize() > 0)
	{
		// 오픈리스트 큐에서 노드 하나 뽑음
		st_AStarNodeInt AStarNode = OpenListQue.PopHeap();
		// 해당 위치를 방문했엇는지 확인
		if (CloseList[AStarNode._Position.Y][AStarNode._Position.X] == true)
		{
			continue;
		}

		// 해당 위치를 방문했다고 기록
		CloseList[AStarNode._Position.Y][AStarNode._Position.X] = true;

		// 뽑은 노드의 위치가 목적지라면 나간다
		if (AStarNode._Position.Y == DestPosition.Y && AStarNode._Position.X == DestPosition.X)
		{
			break;
		}

		// 뽑은 노드를 기준으로 해서 상 하 좌 우 노드를 검사하고 오픈리스트 큐에 넣는다.
		for (int32 i = 0; i < 4; i++)
		{
			// 다음 위치를 알아낸다.
			Vector2Int NextPosition;
			NextPosition.Y = AStarNode._Position.Y + DeltaY[i];
			NextPosition.X = AStarNode._Position.X + DeltaX[i];

			// 다음으로 뽑아낸 위치가 시작점으로 지정해준 값보다 너무 멀다면 해당 좌표는 무시한다.
			if (abs(StartPosition.Y - NextPosition.Y) + abs(StartPosition.X - NextPosition.X) > MaxDistance)
			{
				continue;
			}

			// 다음 위치가 목적지 좌표가 아닐 경우
			if (NextPosition.Y != DestPosition.Y || NextPosition.X != DestPosition.X)
			{
				Vector2Int NextPositionVector = NextPosition;
				
				// 해당 위치로 갈 수 있는지 검사한다.
				if (FindPathNextPositionCango(Object, NextPositionVector, CheckObjects) == false	)
				{
					continue;
				}
			}

			// 이동하고자 하는 다음 위치를 이미 방문 했는지 확인한다.
			if (CloseList[NextPosition.Y][NextPosition.X] == true)
			{
				continue;
			}

			// G 값을 계산한다.
			int G = AStarNode._G + Cost[i];
			// 다음 위치와 목적지 위치를 계산한다.
			int H = abs(DestPosition.Y - NextPosition.Y) + abs(DestPosition.X - NextPosition.X);

			// 새로 계산한 F 값이 원래 있었던 F 값보다 작을 경우 새로 계산한다.
			if (G + H > OpenList[NextPosition.Y][NextPosition.X])
			{
				continue;
			}

			// 삽입할 노드를 준비하고
			st_AStarNodeInt InsertAStartNode;
			// 앞서 구한 값들을 이용해 F값을 구한후 저장한다.
			OpenList[NextPosition.Y][NextPosition.X] = G + H;

			// 삽입할 노드를 초기화 한 후에
			InsertAStartNode._F = G + H;
			InsertAStartNode._G = G;
			InsertAStartNode._Position.X = NextPosition.X;
			InsertAStartNode._Position.Y = NextPosition.Y;

			// 우선순위 큐에 넣는다.
			OpenListQue.InsertHeap(InsertAStartNode._F, InsertAStartNode);
			// 부모 목록에 다음 위치가 있는지 확인
			auto ChildFind = Parents.find(NextPosition);
			if (ChildFind == Parents.end())
			{
				//찾지 못햇으면 넣어주고
				Parents.insert(pair<Vector2Int, Vector2Int>(NextPosition, AStarNode._Position));
			}
			else
			{
				//이미 있었으면 부모 노드를 바꿔준다.
				(*ChildFind).second = AStarNode._Position;
			}
		}
	}

	// CloseList, OpenList 정리
	for (int32 i = 0; i < _SizeY; i++)
	{
		free(CloseList[i]);
		free(OpenList[i]);
	}

	free(CloseList);
	free(OpenList);

	vector<Vector2Int> CompletePathCells;

	// 완성된 길 목록 중에서 목표 지점이 있는지 확인
	if(Parents.find(DestPosition) == Parents.end())
	{
		// 완성 된 길 중 목표 지점이 없다면
		// 
		Vector2Int BestPosition;
		int32 BestDistance; memset(&BestDistance, 1, sizeof(int32));

		// 가지고 있는 위치 중에서 가장 가까운 위치를 찾는다.		
		for (auto Start = Parents.begin(); Start != Parents.end(); ++Start)
		{
			int32 Distance = Vector2Int::Distance((*Start).first, DestPosition);
			if (BestDistance > Distance)
			{
				BestPosition = (*Start).first;
				BestDistance = Distance;
			}
		}

		// 해당 위치를 목적지로 삼는다.
		DestPosition = BestPosition;

		// 새로운 목적지를 기준으로 다시 길을 찾는다.
		return FindPath(Object, StartCellPosition, DestPosition);
	}	
	else
	{
		Vector2Int Position = DestPosition;
		while ((*Parents.find(Position)).second != Position)
		{
			CompletePathCells.push_back(Position);
			Position = (*Parents.find(Position)).second;
		}

		// 시작점 담기
		CompletePathCells.push_back(Position);
		// 부모 위치 담은 배열 거꾸로 뒤집어서 반환
		reverse(CompletePathCells.begin(), CompletePathCells.end());

		return CompletePathCells;
	}	
}

bool CMap::FindPathNextPositionCango(CGameObject* Object, Vector2Int& NextPosition, bool CheckObjects)
{
	// 좌우 좌표 검사
	if (NextPosition.X < _Left || NextPosition.X > _Right)
	{
		return false;
	}

	// 상하 좌표 검사
	if (NextPosition.Y < _Up || NextPosition.Y > _Down)
	{
		return false;
	}

	int X = NextPosition.X - _Left;
	int Y = _Down - NextPosition.Y;

	//G_Logger->WriteStdOut(en_Color::RED, L"X : %d Y : %d \n", X, Y);

	bool IsCollisionMapInfo = true;

	if (_ObjectsInfos[Y][X] != nullptr)
	{
		switch (_ObjectsInfos[Y][X]->_GameObjectInfo.ObjectType)
		{
		case en_GameObjectType::OBJECT_WALL:
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
			IsCollisionMapInfo = false;
			break;
		}
	}

	bool ObjectCheck = false;

	switch (Object->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_GOBLIN:	
		// 오브젝트 위치 배열이 비워 있을 경우 true 반환
		if (_ObjectsInfos[Y][X] == nullptr)
		{			
			ObjectCheck = true;			
		}
		else
		{
			// 비워 있지는 않지만 안에 있는 오브젝트가 이동하고자 하는 오브젝트일 경우
			if (_ObjectsInfos[Y][X]->_GameObjectInfo.ObjectId == Object->_GameObjectInfo.ObjectId)
			{
				ObjectCheck = true;
			}
			else // 안에 있는 오브젝트가 다른 오브젝트일 경우
			{
				if (_ObjectsInfos[Y][X]->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::DEAD)
				{
					ObjectCheck = true;
				}
				else
				{
					ObjectCheck = false;
				}
			}			
		}
		break;
	}

	return IsCollisionMapInfo && (!CheckObjects || ObjectCheck);
}

CChannelManager* CMap::GetChannelManager()
{
	return _ChannelManager;
}