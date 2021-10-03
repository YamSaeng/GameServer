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

CPlayer* CChannel::FindNearPlayer(CGameObject* Object, int32 Range)
{
	// ���� �÷��̾� ���� �޾ƿͼ�
	vector<CPlayer*> Players = GetAroundPlayer(Object, Range);	

	// �÷��̾� ��� ���鼭 ��ã��� ã�ƺ��� �� �� ������ �ش� �÷��̾��ȯ
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
	// �����ϰ� �ִ� ���� ������Ʈ
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
	// ä�� ����
	if (EnterChannelGameObject == nullptr)
	{
		CRASH("GameObject�� nullptr");
		return;
	}	

	random_device RD;
	mt19937 Gen(RD());
		
	st_Vector2Int SpawnPosition;

	// ���� �������� ���� ������ ���� �ʾ����� ���� ���� ��ǥ ����
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

	// ������ ������Ʈ�� Ÿ�Կ� ����
	switch ((en_GameObjectType)EnterChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::MELEE_PLAYER:
	case en_GameObjectType::MAGIC_PLAYER:
		{
			// �÷��̾�� ����ȯ
			CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._Y;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._X;

			// �÷��̾� �ڷᱸ���� ����
			_Players.insert(pair<int64, CPlayer*>(EnterChannelPlayer->_GameObjectInfo.ObjectId, EnterChannelPlayer));
			
			// ä�� ����
			EnterChannelPlayer->_Channel = this;		

			// �ʿ� ����
			_Map->ApplyMove(EnterChannelPlayer, SpawnPosition);

			// ���� �� �ش� ���Ϳ��� ����
			CSector* EnterSector = GetSector(SpawnPosition);			
			EnterSector->Insert(EnterChannelPlayer);			
		}
		break;
	case en_GameObjectType::SLIME:
	case en_GameObjectType::BEAR:
		{
			// ���ͷ� ����ȯ
			CMonster* EnterChannelMonster = (CMonster*)EnterChannelGameObject;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._Y;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._X;

			// ���� �ڷᱸ���� ����
			_Monsters.insert(pair<int64,CMonster*>(EnterChannelMonster->_GameObjectInfo.ObjectId,EnterChannelMonster));

			// ä�� ����
			EnterChannelMonster->_Channel = this;		

			// �ʿ� ����
			_Map->ApplyMove(EnterChannelMonster, SpawnPosition);
			EnterChannelMonster->Init(SpawnPosition);

			// ���� �� �ش� ���Ϳ��� ����
			CSector* EnterSector = GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelMonster);												
		}
		break;
	case en_GameObjectType::SLIME_GEL:
	case en_GameObjectType::BRONZE_COIN:
	case en_GameObjectType::LEATHER:
	case en_GameObjectType::SKILL_BOOK:
		{
			// ���������� ����ȯ
			CItem* EnterChannelItem = (CItem*)EnterChannelGameObject;
			EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.PositionY = SpawnPosition._Y;
			EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.PositionX = SpawnPosition._X;

			_Items.insert(pair<int64, CItem*>(EnterChannelItem->_GameObjectInfo.ObjectId,EnterChannelItem));

			EnterChannelItem->_Channel = this;			
			
			// �ʿ� ���� �������� ���� �浹ü�� �ν����� �ʰ� �Ѵ�.
			_Map->ApplyMove(EnterChannelItem, SpawnPosition, false, false);

			// ���� �� �ش� ���Ϳ��� ����
			CSector* EnterSector = GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelItem);
		}
		break;
	}
}

void CChannel::LeaveChannel(CGameObject* LeaveChannelGameObject)
{	
	// ä�� ����
	switch ((en_GameObjectType)LeaveChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::MELEE_PLAYER:
	case en_GameObjectType::MAGIC_PLAYER:
		// �����̳ʿ��� ����
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
