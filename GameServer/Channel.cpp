#include "pch.h"
#include "Channel.h"
#include "Player.h"
#include "Message.h"
#include "Monster.h"
#include "Item.h"
#include "Heap.h"
#include "Environment.h"

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
		// �ֺ� ���� �÷��̾� ����
		for (CPlayer* Player : Sector->GetPlayers())
		{
			// �Լ� ȣ���� ������Ʈ�� ������ �������� ���� ���� true�� ���� false�� ����
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

		// �ֺ� ���� ���� ����
		for (CMonster* Monster : Sector->GetMonsters())
		{
			GameObjects.push_back(Monster);
		}

		for (CEnvironment* Environment : Sector->GetEnvironment())
		{
			GameObjects.push_back(Environment);
		}

		for (CItem* Item : Sector->GetItems())
		{
			GameObjects.push_back(Item);
		}
	}	
	
	return GameObjects;
}

vector<CPlayer*> CChannel::GetAroundPlayer(CGameObject* Object, int32 Range, bool ExceptMe)
{
	// ���� ���� ������
	vector<CSector*> Sectors = GetAroundSectors(Object->GetCellPosition(), Range);
	vector<CPlayer*> Players;

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

CGameObject* CChannel::FindNearPlayer(CGameObject* Object, int32 Range, bool* Cango)
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
		vector<st_Vector2Int> FirstPaths = _Map->FindPath(Object->GetCellPosition(), Player->GetCellPosition());
		if (FirstPaths.size() < 2)
		{
			// Ÿ���� ������ ������ ���� ���� ( ������ ������Ʈ��� ������ )
			if (Player != nullptr)
			{				
				// ���� �ٸ� ����߿��� �� �� �ִ� ����� �ִ��� �߰��� �Ǵ�
				while (Distances.GetUseSize() != 0)
				{
					Player = Distances.PopHeap();
					vector<st_Vector2Int> SecondPaths = _Map->FindPath(Object->GetCellPosition(), Player->GetCellPosition());
					if (SecondPaths.size() < 2)
					{
						continue;
					}

					*Cango = true;
					return Player;
				}

				*Cango = false;
				return Player;
			}			
		}
		
		*Cango = true;
		return Player;
	}
	else
	{
		*Cango = false;
		return nullptr;
	}
}

void CChannel::Update()
{	
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

	for (auto EnvironmentIterator : _Environments)
	{
		CEnvironment* Environment = EnvironmentIterator.second;

		Environment->Update();
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

			SpawnPosition._X = RandomXPosition(Gen);
			SpawnPosition._Y = RandomYPosition(Gen);
						
			if (_Map->Cango(SpawnPosition) == true)
			{
				break;
			}
		}
	}
	
	G_Logger->WriteStdOut(en_Color::RED, L"SpawnPosition Y : %d X : %d \n", SpawnPosition._Y, SpawnPosition._X);	

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
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._Y;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._X;

			// �÷��̾� �ڷᱸ���� ����
			_Players.insert(pair<int64, CPlayer*>(EnterChannelPlayer->_GameObjectInfo.ObjectId, EnterChannelPlayer));
			
			// ä�� ����
			EnterChannelPlayer->_Channel = this;		

			// �ʿ� ����
			IsEnterChannel = _Map->ApplyMove(EnterChannelPlayer, SpawnPosition, true, false);

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
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._Y;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._X;

			EnterChannelMonster->Init(SpawnPosition);

			// ���� �ڷᱸ���� ����
			_Monsters.insert(pair<int64,CMonster*>(EnterChannelMonster->_GameObjectInfo.ObjectId,EnterChannelMonster));

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
			EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._Y;
			EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._X;
			
			EnterChannelItem->_Channel = this;			
			
			// �� ������ ����			
			IsEnterChannel = _Map->ApplyPositionUpdateItem(EnterChannelItem, SpawnPosition);
			
			// �ߺ����� �ʴ� �������� ��쿡�� ä�ο� �ش� �������� ä�ΰ� ���Ϳ� ����
			if (IsEnterChannel == true)
			{
				_Items.insert(pair<int64, CItem*>(EnterChannelItem->_GameObjectInfo.ObjectId, EnterChannelItem));
				
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
			EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._Y;
			EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._X;

			_Environments.insert(pair<int64, CEnvironment*>(EnterChannelEnvironment->_GameObjectInfo.ObjectId, EnterChannelEnvironment));

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
		_Players.erase(LeaveChannelGameObject->_GameObjectInfo.ObjectId);
		
		_Map->ApplyLeave(LeaveChannelGameObject);		
		break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		_Monsters.erase(LeaveChannelGameObject->_GameObjectInfo.ObjectId);

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
		_Items.erase(LeaveChannelGameObject->_GameObjectInfo.ObjectId);

		_Map->ApplyPositionLeaveItem(LeaveChannelGameObject);
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		_Environments.erase(LeaveChannelGameObject->_GameObjectInfo.ObjectId);

		_Map->ApplyLeave(LeaveChannelGameObject);
		break;
	}	
}
