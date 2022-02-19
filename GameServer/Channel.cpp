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

	// �� ũ�⸦ ���� ���Ͱ� ���� ���� ��� �ִ��� Ȯ��
	_SectorCountY = (_Map->_SizeY + _SectorSize - 1) / _SectorSize;
	_SectorCountX = (_Map->_SizeX + _SectorSize - 1) / _SectorSize;

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
		// �ֺ� ���� �÷��̾� ����
		for (CPlayer* Player : Sector->GetPlayers())
		{			
			// �Լ� ȣ���� ������Ʈ�� ������ �������� ���� ���� true�� ���� false�� ����
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

		// �ֺ� ���� ���� ����
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
		// �ֺ� ���� �÷��̾� ����
		for (CPlayer* Player : Sector->GetPlayers())
		{
			int16 Distance = st_Vector2Int::Distance(Object->GetCellPosition(), Player->GetCellPosition());

			// �Լ� ȣ���� ������Ʈ�� ������ �������� ���� ���� true�� ���� false�� ����
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

		// �ֺ� ���� ���� ����
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
	// ���� ���� ������
	vector<CSector*> Sectors = GetAroundSectors(Object->GetCellPosition(), Range);

	// ���Ϳ� �ִ� �÷��̾ ��Ƽ� ��ȯ
	// ExceptMe�� true�� ���� false�� ����
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
		// �ֺ� ���� �÷��̾� ����
		for (CPlayer* Player : Sector->GetPlayers())
		{
			int16 Distance = st_Vector2Int::Distance(Object->GetCellPosition(), Player->GetCellPosition());

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

CGameObject* CChannel::FindNearPlayer(CGameObject* Object, int32 Range, bool* CollisionCango)
{
	// ���� �÷��̾� ���� �޾ƿͼ�
	vector<CPlayer*> Players = GetAroundPlayer(Object, Range);	
	
	// �޾ƿ� �÷��̾� ������ ���� �Ÿ��� ���ؼ� �켱���� ť�� ��´�.
	CHeap<int16, CPlayer*> Distances((int32)Players.size());		
	for (CPlayer* Player : Players)
	{		
		Distances.InsertHeap(st_Vector2Int::Distance(Player->GetCellPosition(), Object->GetCellPosition()), Player);
	}
	
	CPlayer* Player = nullptr;
	// �Ÿ� ����� �ֺ��� �����ؼ� �ش� �÷��̾�� �� �� �ִ��� Ȯ���ϰ�
	// �� �� �ִ� ����̸� �ش� �÷��̾ ��ȯ���ش�.
	if (Distances.GetUseSize() != 0)
	{
		Player = Distances.PopHeap();
		vector<st_Vector2Int> FirstPaths = _Map->FindPath(Object, Object->GetCellPosition(), Player->GetCellPosition());
		if (FirstPaths.size() < 2)
		{
			// Ÿ���� ������ ������ ���� ���� ( ������ ������Ʈ��� ������ )
			if (Player != nullptr)
			{				
				// ���� �ٸ� ����߿��� �� �� �ִ� ����� �ִ��� �߰��� �Ǵ�
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
	// ä�� ����
	if (EnterChannelGameObject == nullptr)
	{
		CRASH("GameObject�� nullptr");
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
		// ���̸� ������� ���� ��ǥ �޾Ƽ� ä�ο� ����
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
	// ������ ������Ʈ�� Ÿ�Կ� ����
	switch ((en_GameObjectType)EnterChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_MAGIC_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		{
			// �÷��̾�� ����ȯ
			CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;
			EnterChannelPlayer->_SpawnPosition._Y = SpawnPosition._Y;
			EnterChannelPlayer->_SpawnPosition._X = SpawnPosition._X;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = SpawnPosition._Y;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = SpawnPosition._X;
						
			EnterChannelPlayer->PositionReset();			
						
			// �÷��̾� ����
			_ChannelPlayerArrayIndexs.Pop(&EnterChannelPlayer->_ChannelArrayIndex);
			_ChannelPlayerArray[EnterChannelPlayer->_ChannelArrayIndex] = EnterChannelPlayer;

			//_Players.insert(pair<int64, CPlayer*>(EnterChannelPlayer->_GameObjectInfo.ObjectId, EnterChannelPlayer));			
			
			// ä�� ����
			EnterChannelPlayer->_Channel = this;		

			// �ʿ� ����
			IsEnterChannel = _Map->ApplyMove(EnterChannelPlayer, SpawnPosition);

			// ���� �� �ش� ���Ϳ��� ����
			CSector* EnterSector = GetSector(SpawnPosition);			
			EnterSector->Insert(EnterChannelPlayer);			
		}
		break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		{
			// ���ͷ� ����ȯ
			CMonster* EnterChannelMonster = (CMonster*)EnterChannelGameObject;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = SpawnPosition._Y;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = SpawnPosition._X;

			EnterChannelMonster->Init(SpawnPosition);
						
			// ���� ����
			_ChannelMonsterArrayIndexs.Pop(&EnterChannelMonster->_ChannelArrayIndex);
			_ChannelMonsterArray[EnterChannelMonster->_ChannelArrayIndex] = EnterChannelMonster;
			
			// ä�� ����
			EnterChannelMonster->_Channel = this;		

			// �ʿ� ����
			IsEnterChannel = _Map->ApplyMove(EnterChannelMonster, SpawnPosition);			

			// ���� �� �ش� ���Ϳ��� ����
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
			// ���������� ����ȯ
			CItem* EnterChannelItem = (CItem*)EnterChannelGameObject;
			EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = SpawnPosition._Y;
			EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = SpawnPosition._X;
			
			EnterChannelItem->_Channel = this;			
			
			// �� ������ ����			
			IsEnterChannel = _Map->ApplyPositionUpdateItem(EnterChannelItem, SpawnPosition);
			
			// �ߺ����� �ʴ� �������� ��쿡�� ä�ο� �ش� �������� ä�ΰ� ���Ϳ� ����
			if (IsEnterChannel == true)
			{
				// ���� ����
				_ChannelItemArrayIndexs.Pop(&EnterChannelItem->_ChannelArrayIndex);
				_ChannelItemArray[EnterChannelItem->_ChannelArrayIndex] = EnterChannelItem;				
				
				// ���� �� �ش� ���Ϳ��� ����
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

			// ȯ�� ������Ʈ ����
			_ChannelEnvironmentArrayIndexs.Pop(&EnterChannelEnvironment->_ChannelArrayIndex);
			_ChannelEnvironmentArray[EnterChannelEnvironment->_ChannelArrayIndex] = EnterChannelEnvironment;

			EnterChannelEnvironment->_Channel = this;		
						
			IsEnterChannel = _Map->ApplyMove(EnterChannelEnvironment, SpawnPosition);

			// ���� �� �ش� ���Ϳ��� ����
			CSector* EnterSector = GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelEnvironment);
		}
		break;
	}	
	
	return IsEnterChannel;
}

void CChannel::LeaveChannel(CGameObject* LeaveChannelGameObject)
{	
	// ä�� ����
	// �����̳ʿ��� ������ �� �ʿ����� ����
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