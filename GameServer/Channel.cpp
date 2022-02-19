#include "pch.h"
#include "Channel.h"
#include "Player.h"
#include "Message.h"
#include "Monster.h"
#include "Item.h"
#include "Heap.h"
#include "Environment.h"

CChannel::CChannel()
{
	InitializeSRWLock(&_ChannelLock);

	for (int32 PlayerCount = PLAYER_MAX - 1; PlayerCount >= 0; --PlayerCount)
	{
		_ChannelPlayerArray[PlayerCount] = nullptr;
		_ChannelPlayerArrayIndexs.Push(PlayerCount);
	}

	for (int32 MonsterCount = MONSTER_MAX - 1; MonsterCount >= 0; --MonsterCount)
	{
		_ChannelMonsterArray[MonsterCount] = nullptr;
		_ChannelMonsterArrayIndexs.Push(MonsterCount);
	}

	for (int32 ItemCount = ITEM_MAX - 1; ItemCount >= 0; --ItemCount)
	{
		_ChannelItemArray[ItemCount] = nullptr;
		_ChannelItemArrayIndexs.Push(ItemCount);
	}

	for (int32 Environment = ENVIRONMENT_MAX - 1; Environment >= 0; --Environment)
	{
		_ChannelEnvironmentArray[Environment] = nullptr;
		_ChannelEnvironmentArrayIndexs.Push(Environment);
	}
}

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

vector<CGameObject*> CChannel::GetAroundSectorObjects(CGameObject* Object, int32 Range, bool ExceptMe)
{
	vector<CGameObject*> SectorGameObjects;

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
					SectorGameObjects.push_back(Player);
				}
			}
			else
			{
				SectorGameObjects.push_back(Player);
			}		
		}

		// 주변 섹터 몬스터 정보
		for (CMonster* Monster : Sector->GetMonsters())
		{
			SectorGameObjects.push_back(Monster);
		}

		for (CEnvironment* Environment : Sector->GetEnvironment())
		{
			SectorGameObjects.push_back(Environment);
		}

		for (CItem* Item : Sector->GetItems())
		{
			SectorGameObjects.push_back(Item);
		}
	}	
	
	return SectorGameObjects;
}

vector<CGameObject*> CChannel::GetFieldOfViewObjects(CGameObject* Object, int16 Range, bool ExceptMe)
{
	vector<CGameObject*> FieldOfViewGameObjects;

	vector<CSector*> Sectors = GetAroundSectors(Object->GetCellPosition(), Range);

	for (CSector* Sector : Sectors)
	{
		// 주변 섹터 플레이어 정보
		for (CPlayer* Player : Sector->GetPlayers())
		{
			int16 Distance = st_Vector2Int::Distance(Object->GetCellPosition(), Player->GetCellPosition());

			// 함수 호출한 오브젝트를 포함할 것인지에 대한 여부 true면 제외 false면 포함
			if (ExceptMe == true && Distance <= Object->_FieldOfViewDistance)
			{
				if (Object->_GameObjectInfo.ObjectId != Player->_GameObjectInfo.ObjectId)
				{
					FieldOfViewGameObjects.push_back(Player);
				}
			}
			else if(ExceptMe == false && Distance <= Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjects.push_back(Player);
			}
		}

		// 주변 섹터 몬스터 정보
		for (CMonster* Monster : Sector->GetMonsters())
		{
			int16 Distance = st_Vector2Int::Distance(Object->GetCellPosition(), Monster->GetCellPosition());

			if (Distance <= Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjects.push_back(Monster);
			}			
		}

		for (CEnvironment* Environment : Sector->GetEnvironment())
		{
			int16 Distance = st_Vector2Int::Distance(Object->GetCellPosition(), Environment->GetCellPosition());

			if (Distance <= Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjects.push_back(Environment);
			}			
		}

		for (CItem* Item : Sector->GetItems())
		{
			int16 Distance = st_Vector2Int::Distance(Object->GetCellPosition(), Item->GetCellPosition());

			if (Distance <= Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjects.push_back(Item);
			}			
		}		
	}

	return FieldOfViewGameObjects;
}

vector<CPlayer*> CChannel::GetAroundPlayer(CGameObject* Object, int32 Range, bool ExceptMe)
{
	vector<CPlayer*> Players;	
	// 주위 섹터 얻어오고
	vector<CSector*> Sectors = GetAroundSectors(Object->GetCellPosition(), Range);

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

vector<CPlayer*> CChannel::GetFieldOfViewPlayer(CGameObject* Object, int16 Range, bool ExceptMe)
{
	vector<CPlayer*> FieldOfViewPlayers;

	vector<CSector*> Sectors = GetAroundSectors(Object->GetCellPosition(), Range);

	for (CSector* Sector : Sectors)
	{
		// 주변 섹터 플레이어 정보
		for (CPlayer* Player : Sector->GetPlayers())
		{
			int16 Distance = st_Vector2Int::Distance(Object->GetCellPosition(), Player->GetCellPosition());

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

CGameObject* CChannel::FindNearPlayer(CGameObject* Object, int32 Range, bool* CollisionCango)
{
	// 주위 플레이어 정보 받아와서
	vector<CPlayer*> Players = GetAroundPlayer(Object, Range);	
	
	// 받아온 플레이어 정보를 토대로 거리를 구해서 우선순위 큐에 담는다.
	CHeap<int16, CPlayer*> Distances((int32)Players.size());		
	for (CPlayer* Player : Players)
	{		
		Distances.InsertHeap(st_Vector2Int::Distance(Player->GetCellPosition(), Object->GetCellPosition()), Player);
	}
	
	CPlayer* Player = nullptr;
	// 거리 가까운 애부터 접근해서 해당 플레이어로 갈 수 있는지 확인하고
	// 갈 수 있는 대상이면 해당 플레이어를 반환해준다.
	if (Distances.GetUseSize() != 0)
	{
		Player = Distances.PopHeap();
		vector<st_Vector2Int> FirstPaths = _Map->FindPath(Object, Object->GetCellPosition(), Player->GetCellPosition());
		if (FirstPaths.size() < 2)
		{
			// 타겟은 있지만 갈수는 없는 상태 ( 주위에 오브젝트들로 막혀서 )
			if (Player != nullptr)
			{				
				// 주위 다른 대상중에서 갈 수 있는 대상이 있는지 추가로 판단
				while (Distances.GetUseSize() != 0)
				{
					Player = Distances.PopHeap();
					vector<st_Vector2Int> SecondPaths = _Map->FindPath(Object, Object->GetCellPosition(), Player->GetCellPosition());
					if (SecondPaths.size() < 2)
					{
						continue;
					}

					*CollisionCango = true;
					return Player;
				}

				*CollisionCango = false;
				return Player;
			}			
		}
		
		*CollisionCango = true;
		return Player;
	}
	else
	{
		*CollisionCango = false;
		return nullptr;
	}
}

void CChannel::Update()
{	
	for (int16 i = 0; i < PLAYER_MAX; i++)
	{
		if (_ChannelPlayerArray[i] 
			&& _ChannelPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE
			&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			_ChannelPlayerArray[i]->Update();
		}
	}

	for (int16 i = 0; i < MONSTER_MAX; i++)
	{
		if (_ChannelMonsterArray[i]
			&& _ChannelMonsterArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			_ChannelMonsterArray[i]->Update();
		}
	}

	for (int16 i = 0; i < ITEM_MAX; i++)
	{
		if (_ChannelItemArray[i]
			&& _ChannelItemArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			_ChannelItemArray[i]->Update();
		}
	}

	for (int16 i = 0; i < ENVIRONMENT_MAX; i++)
	{
		if (_ChannelEnvironmentArray[i]
			&& _ChannelEnvironmentArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			_ChannelEnvironmentArray[i]->Update();
		}
	}	
}

bool CChannel::EnterChannel(CGameObject* EnterChannelGameObject, st_Vector2Int* ObjectSpawnPosition)
{	
	bool IsEnterChannel = false;
	// 채널 입장
	if (EnterChannelGameObject == nullptr)
	{
		CRASH("GameObject가 nullptr");
		return false;
	}	

	random_device RD;
	mt19937 Gen(RD());	

	st_Vector2Int SpawnPosition;		

	if (ObjectSpawnPosition != nullptr)
	{
		SpawnPosition = *ObjectSpawnPosition;
	}
	else
	{
		// 더미를 대상으로 랜덤 좌표 받아서 채널에 입장
		while (true)
		{
			uniform_int_distribution<int> RandomXPosition(-26, 63);
			uniform_int_distribution<int> RandomYPosition(-13, 56);		

			SpawnPosition._X =  RandomXPosition(Gen);
			SpawnPosition._Y =  RandomYPosition(Gen);
						
			if (_Map->CollisionCango(EnterChannelGameObject, SpawnPosition) == true)
			{
				break;
			}
		}
	}
	// 입장한 오브젝트의 타입에 따라
	switch ((en_GameObjectType)EnterChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_MAGIC_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		{
			// 플레이어로 형변환
			CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;
			EnterChannelPlayer->_SpawnPosition._Y = SpawnPosition._Y;
			EnterChannelPlayer->_SpawnPosition._X = SpawnPosition._X;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = SpawnPosition._Y;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = SpawnPosition._X;
						
			EnterChannelPlayer->PositionReset();			
						
			// 플레이어 저장
			_ChannelPlayerArrayIndexs.Pop(&EnterChannelPlayer->_ChannelArrayIndex);
			_ChannelPlayerArray[EnterChannelPlayer->_ChannelArrayIndex] = EnterChannelPlayer;

			//_Players.insert(pair<int64, CPlayer*>(EnterChannelPlayer->_GameObjectInfo.ObjectId, EnterChannelPlayer));			
			
			// 채널 저장
			EnterChannelPlayer->_Channel = this;		

			// 맵에 적용
			IsEnterChannel = _Map->ApplyMove(EnterChannelPlayer, SpawnPosition);

			// 섹터 얻어서 해당 섹터에도 저장
			CSector* EnterSector = GetSector(SpawnPosition);			
			EnterSector->Insert(EnterChannelPlayer);			
		}
		break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		{
			// 몬스터로 형변환
			CMonster* EnterChannelMonster = (CMonster*)EnterChannelGameObject;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = SpawnPosition._Y;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = SpawnPosition._X;

			EnterChannelMonster->Init(SpawnPosition);
						
			// 몬스터 저장
			_ChannelMonsterArrayIndexs.Pop(&EnterChannelMonster->_ChannelArrayIndex);
			_ChannelMonsterArray[EnterChannelMonster->_ChannelArrayIndex] = EnterChannelMonster;
			
			// 채널 저장
			EnterChannelMonster->_Channel = this;		

			// 맵에 적용
			IsEnterChannel = _Map->ApplyMove(EnterChannelMonster, SpawnPosition);			

			// 섹터 얻어서 해당 섹터에도 저장
			CSector* EnterSector = GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelMonster);												
		}
		break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:	
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
		{
			// 아이템으로 형변환
			CItem* EnterChannelItem = (CItem*)EnterChannelGameObject;
			EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = SpawnPosition._Y;
			EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = SpawnPosition._X;
			
			EnterChannelItem->_Channel = this;			
			
			// 맵 정보에 보관			
			IsEnterChannel = _Map->ApplyPositionUpdateItem(EnterChannelItem, SpawnPosition);
			
			// 중복되지 않는 아이템의 경우에만 채널에 해당 아이템을 채널과 섹터에 저장
			if (IsEnterChannel == true)
			{
				// 몬스터 저장
				_ChannelItemArrayIndexs.Pop(&EnterChannelItem->_ChannelArrayIndex);
				_ChannelItemArray[EnterChannelItem->_ChannelArrayIndex] = EnterChannelItem;				
				
				// 섹터 얻어서 해당 섹터에도 저장
				CSector* EnterSector = GetSector(SpawnPosition);
				EnterSector->Insert(EnterChannelItem);
			}		
		}
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		{			
			CEnvironment* EnterChannelEnvironment = (CEnvironment*)EnterChannelGameObject;
			EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = SpawnPosition._Y;
			EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = SpawnPosition._X;
			
			if (EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY > 0)
			{
				EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.PositionY = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY;
			}
			else if (EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY == 0)
			{
				EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.PositionY = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY - 0.5f;
			}
			else
			{
				EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.PositionY = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY - 1.0f;
			}
			
			if (EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX > 0)
			{
				EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.PositionX = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX + 0.5f;
			}
			else
			{
				EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.PositionX = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX - 0.5f;
			}		

			// 환경 오브젝트 저장
			_ChannelEnvironmentArrayIndexs.Pop(&EnterChannelEnvironment->_ChannelArrayIndex);
			_ChannelEnvironmentArray[EnterChannelEnvironment->_ChannelArrayIndex] = EnterChannelEnvironment;

			EnterChannelEnvironment->_Channel = this;		
						
			IsEnterChannel = _Map->ApplyMove(EnterChannelEnvironment, SpawnPosition);

			// 섹터 얻어서 해당 섹터에도 저장
			CSector* EnterSector = GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelEnvironment);
		}
		break;
	}	
	
	return IsEnterChannel;
}

void CChannel::LeaveChannel(CGameObject* LeaveChannelGameObject)
{	
	// 채널 퇴장
	// 컨테이너에서 제거한 후 맵에서도 제거
	switch ((en_GameObjectType)LeaveChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_MAGIC_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:		
		_ChannelPlayerArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);		

		_Map->ApplyLeave(LeaveChannelGameObject);		
		break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		_ChannelMonsterArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);		

		_Map->ApplyLeave(LeaveChannelGameObject);
		break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
		_ChannelItemArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);		

		_Map->ApplyPositionLeaveItem(LeaveChannelGameObject);
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		_ChannelEnvironmentArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);		

		_Map->ApplyLeave(LeaveChannelGameObject);
		break;
	}	
}