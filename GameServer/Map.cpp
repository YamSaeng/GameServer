#include "pch.h"
#include "Map.h"
#include "GameObject.h"
#include "Player.h"
#include "Monster.h"
#include "Heap.h"
#include "ObjectManager.h"
#include "Item.h"

CMap::CMap(int MapId)
{
	char* FileStr = FileUtils::LoadFile(L"MapData.txt");	
	char* MovingFileP = FileStr;
	char* ConvertP = FileStr;

	int i = 0;
	while (true)
	{
		if (*MovingFileP == '\n')
		{
			if (i == 0)
			{
				_Left = atoi(ConvertP);
				ConvertP = MovingFileP;
				i++;
			}
			else if (i == 1)
			{
				_Right = atoi(ConvertP);
				ConvertP = MovingFileP;
				i++;
			}
			else if (i == 2)
			{
				_Up = atoi(ConvertP);
				ConvertP = MovingFileP;
				i++;
			}
			else if (i == 3)
			{
				_Down = atoi(ConvertP);
				ConvertP = MovingFileP + 1;
				i++;
				break;
			}		
		}

		MovingFileP++;
	}

	int XCount = _Right - _Left + 1;
	int YCount = _Down - _Up + 1;	

	_CollisionMapInfos = new en_TileMapEnvironment*[YCount];

	for (int i = 0; i < YCount; i++)
	{
		_CollisionMapInfos[i] = new en_TileMapEnvironment[XCount];
	}

	_ObjectsInfos = new CGameObject**[YCount];

	for (int i = 0; i < YCount; i++)
	{
		_ObjectsInfos[i] = new CGameObject*[XCount];	
		for (int j = 0; j < XCount; j++)
		{
			_ObjectsInfos[i][j] = nullptr;
		}
	}

	_Items = new CItem***[YCount];
	
	for (int i = 0; i < YCount; i++)
	{
		_Items[i] = new CItem**[XCount];
		for (int j = 0; j < XCount; j++)
		{
			_Items[i][j] = new CItem*[(int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX];

			for (int8 k = 0; k < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; k++)
			{
				_Items[i][j][k] = nullptr;				
			}
		}
	}

	for (int Y = 0; Y < YCount; Y++)
	{
		for (int X = 0; X < XCount; X++)
		{		
			if (*ConvertP == '\r')
			{				
				break;
			}		

			_CollisionMapInfos[Y][X] = (en_TileMapEnvironment)(*ConvertP - 48);		

			ConvertP++;
		}
		ConvertP += 2;
	}		

	_SizeX = _Right - _Left + 1;
	_SizeY = _Down - _Up + 1;
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

bool CMap::Cango(st_Vector2Int& CellPosition, bool CheckObjects)
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
	
	//G_Logger->WriteStdOut(en_Color::RED, L"Y : %d X : %d\n", Y, X);
	bool IsCollisionMapInfo = false;
	switch (_CollisionMapInfos[Y][X])
	{
	case en_TileMapEnvironment::TILE_MAP_NONE:
	case en_TileMapEnvironment::TILE_MAP_TREE:
	case en_TileMapEnvironment::TILE_MAP_STONE:
	case en_TileMapEnvironment::TILE_MAP_SLIME:
	case en_TileMapEnvironment::TILE_MAP_BEAR:
		IsCollisionMapInfo = true;
		break;
	case en_TileMapEnvironment::TILE_MAP_WALL:
		IsCollisionMapInfo = false;
		break;	
	}

	return IsCollisionMapInfo && (!CheckObjects || _ObjectsInfos[Y][X] == nullptr);
}

bool CMap::ApplyMove(CGameObject* GameObject, st_Vector2Int& DestPosition, bool CheckObject, bool Applycollision)
{
	// ä���� �Ҵ�Ǿ� �ִ��� Ȯ��
	if (GameObject->_Channel == nullptr)
	{
		CRASH("ApplyMove GameObject Channel nullptr")
		return false;
	}

	// ���ӿ�����Ʈ�� ���� ä���� ��� �ִ� �ʰ� ���� ���� ������ Ȯ��
	if (GameObject->_Channel->_Map != this)
	{
		CRASH("ApplyMove GameObject�� ä���� ������ �ִ� �ʰ� ���� ���� �ٸ�")
		return false;
	}

	// ��ġ ���� ������ �´�.
	st_PositionInfo PositionInfo = GameObject->_GameObjectInfo.ObjectPositionInfo;

	// �������� �� �� �ִ��� �˻��Ѵ�.
	if (Cango(DestPosition, CheckObject) == false)
	{
		//G_Logger->WriteStdOut(en_Color::RED, L"Cant Go ApplyMove Y (%d) X (%d) ",DestPosition._Y,DestPosition._X);
		return false;
	}

	// ȣ������ ����� �浹ü�� ����ٸ�
	if (Applycollision == true)
	{
		int X = PositionInfo.PositionX - _Left;
		int Y = _Down - PositionInfo.PositionY;

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
	}		

	// ���� �۾�
	switch (GameObject->_GameObjectInfo.ObjectType)
	{	
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:		
	case en_GameObjectType::OBJECT_MAGIC_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	{
		CPlayer* MovePlayer = (CPlayer*)GameObject;

		CSector* CurrentSector = GameObject->_Channel->GetSector(MovePlayer->GetCellPosition());
		CSector* NextSector = GameObject->_Channel->GetSector(DestPosition);		

		if (CurrentSector != NextSector)
		{			
			// ���� ���Ϳ��� �÷��̾� ����
			CurrentSector->Remove(MovePlayer);		

			vector<CSector*> CurrentSectors =  GameObject->_Channel->GetAroundSectors(MovePlayer->GetCellPosition(), 1);
			vector<CSector*> NextSectors =  GameObject->_Channel->GetAroundSectors(DestPosition, 1);			

			// ���� ������ ���͸� ã�� �۾�
			// Current - Next;
			// ���� ���͵鿡�� ���� �̵��� ���͸� ������ ������ ���͸� ��´�.
			vector<CSector*> DeSpawnSectors = CurrentSectors;
			for (int32 i = 0; i < DeSpawnSectors.size(); i++)
			{
				for (int32 j = 0; j < NextSectors.size(); j++)
				{
					if (DeSpawnSectors[i]->_SectorY == NextSectors[j]->_SectorY && DeSpawnSectors[i]->_SectorX == NextSectors[j]->_SectorX)
					{
						DeSpawnSectors.erase(DeSpawnSectors.begin() + i);
					}
				}
			}
			
			vector<int64> DeSpawnSectorObjectIds;
			DeSpawnSectorObjectIds.push_back(MovePlayer->_GameObjectInfo.ObjectId);
			// ���� �����϶�� �޽����� ���� �� 
			CMessage* ResSectorDespawnPlayer = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, DeSpawnSectorObjectIds);
			// �ش� ���� �÷��̾�鿡�� �����Ѵ�.
			for (int32 i = 0; i < DeSpawnSectors.size(); i++)
			{
				for (CPlayer* Player : DeSpawnSectors[i]->GetPlayers())
				{
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSectorDespawnPlayer);
				}				
			}
			ResSectorDespawnPlayer->Free();			
			
			DeSpawnSectorObjectIds.clear();

			// DeSpawnSector�� �ִ� ������Ʈ���� ��Ƽ� �����׼� �����Ѵ�.
			for (CSector* Sector : DeSpawnSectors)
			{
				for (CPlayer* DeSpawnPlayer : Sector->GetPlayers())
				{
					DeSpawnSectorObjectIds.push_back(DeSpawnPlayer->_GameObjectInfo.ObjectId);
				}

				for (CMonster* DeSpawnMonster : Sector->GetMonsters())
				{
					DeSpawnSectorObjectIds.push_back(DeSpawnMonster->_GameObjectInfo.ObjectId);
				}

				for (CItem* DeSpawnItem : Sector->GetItems())
				{
					DeSpawnSectorObjectIds.push_back(DeSpawnItem->_GameObjectInfo.ObjectId);
				}

				for (CEnvironment* DeSpawnEnvironment : Sector->GetEnvironment())
				{
					DeSpawnSectorObjectIds.push_back(DeSpawnEnvironment->_GameObjectInfo.ObjectId);
				}
			}	
			
			if (DeSpawnSectorObjectIds.size() > 0)
			{
				CMessage* ResSectorDespawnOtherPlayer = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn((int32)DeSpawnSectorObjectIds.size(), DeSpawnSectorObjectIds);
				G_ObjectManager->GameServer->SendPacket(MovePlayer->_SessionId, ResSectorDespawnOtherPlayer);
				ResSectorDespawnOtherPlayer->Free();
			}			

			// �̵��� ���Ϳ� �÷��̾� �߰�
			NextSector->Insert(MovePlayer);
			// ���� ������ ���͸� ã�� �۾�
			// Next - Current;
			// �̵��� ���Ϳ��� ���� ���͸� ������ ������ ���͸� ã�´�.
			vector<CSector*> SpawnSectors = NextSectors;
			for (int32 i = 0; i < SpawnSectors.size(); i++)
			{
				for (int32 j = 0; j < CurrentSectors.size(); j++)
				{
					if (SpawnSectors[i]->_SectorY == CurrentSectors[j]->_SectorY && SpawnSectors[i]->_SectorX == CurrentSectors[j]->_SectorX)
					{
						SpawnSectors.erase(SpawnSectors.begin() + i);
					}
				}
			}					
		
			// ������ ���迭
			vector<st_GameObjectInfo> SpawnObjectInfos;
			// ���� ������ ���
			SpawnObjectInfos.push_back(MovePlayer->_GameObjectInfo);

			// ���� ��ȯ�϶�� �ռ� ����� ���� �÷��̾�鿡�� ��Ŷ�� �����Ѵ�.
			CMessage* ResSectorSpawnPlayer = G_ObjectManager->GameServer->MakePacketResObjectSpawn(1, SpawnObjectInfos);
			for (int32 i = 0; i < SpawnSectors.size(); i++)
			{
				for (CPlayer* Player : SpawnSectors[i]->GetPlayers())
				{
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSectorSpawnPlayer);					
				}
			}			
			ResSectorSpawnPlayer->Free();			

			SpawnObjectInfos.clear();
						
			// �ݴ�� ���� ���Ϳ� �ִ� ������Ʈ���� ��´�.
			for (CSector* Sector : SpawnSectors)
			{
				for (CPlayer* ExistPlayer : Sector->GetPlayers())
				{
					SpawnObjectInfos.push_back(ExistPlayer->_GameObjectInfo);
				}

				for (CMonster* ExistMonster : Sector->GetMonsters())
				{
					SpawnObjectInfos.push_back(ExistMonster->_GameObjectInfo);
				}

				for (CItem* ExistItem : Sector->GetItems())
				{
					SpawnObjectInfos.push_back(ExistItem->_GameObjectInfo);
				}

				for (CEnvironment* ExistEnvironment : Sector->GetEnvironment())
				{
					SpawnObjectInfos.push_back(ExistEnvironment->_GameObjectInfo);
				}
			}			
	
			// ������ �����Ͽ� ���� ���Ϳ� �ִ� ������Ʈ���� ���� ��Ų��.
			if (SpawnObjectInfos.size() > 0)
			{
				CMessage* ResOtherObjectSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn((int32)SpawnObjectInfos.size(), SpawnObjectInfos);
				G_ObjectManager->GameServer->SendPacket(MovePlayer->_SessionId, ResOtherObjectSpawnPacket);
				ResOtherObjectSpawnPacket->Free();
			}			
		}
	}
		break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
	{
		CMonster* MoveMonster = (CMonster*)GameObject;

		CSector* CurrentSector = GameObject->_Channel->GetSector(MoveMonster->GetCellPosition());
		CSector* NextSector = GameObject->_Channel->GetSector(DestPosition);

		if (CurrentSector != NextSector)
		{
			// ���� ���Ϳ��� ���� ����
			CurrentSector->Remove(MoveMonster);		

			vector<CSector*> CurrentSectors = GameObject->_Channel->GetAroundSectors(MoveMonster->GetCellPosition(), 1);
			vector<CSector*> NextSectors = GameObject->_Channel->GetAroundSectors(DestPosition, 1);

			// ���� ������ ���͸� ã�� �۾�
			// Current - Next;
			// ���� ���͵鿡�� ���� �̵��� ���͸� ������ ������ ���͸� ��´�.
			vector<CSector*> DeSpawnSectors = CurrentSectors;
			for (int32 i = 0; i < DeSpawnSectors.size(); i++)
			{
				for (int32 j = 0; j < NextSectors.size(); j++)
				{
					if (DeSpawnSectors[i]->_SectorY == NextSectors[j]->_SectorY && DeSpawnSectors[i]->_SectorX == NextSectors[j]->_SectorX)
					{
						DeSpawnSectors.erase(DeSpawnSectors.begin() + i);
					}
				}
			}

			vector<int64> DeSpawnSectorObjectIds;
			DeSpawnSectorObjectIds.push_back(MoveMonster->_GameObjectInfo.ObjectId);
			// ���� �����϶�� �޽����� ���� �� 
			CMessage* ResSectorDespawnPlayer = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, DeSpawnSectorObjectIds);
			// �ش� ���� �÷��̾�鿡�� �����Ѵ�.
			for (int32 i = 0; i < DeSpawnSectors.size(); i++)
			{
				for (CPlayer* Player : DeSpawnSectors[i]->GetPlayers())
				{
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSectorDespawnPlayer);
				}
			}
			ResSectorDespawnPlayer->Free();


			// �̵��� ���Ϳ� ���� �߰�
			NextSector->Insert(MoveMonster);
			// ���� ������ ���͸� ã�� �۾�
			// Next - Current;
			// �̵��� ���Ϳ��� ���� ���͸� ������ ������ ���͸� ã�´�.
			vector<CSector*> SpawnSectors = NextSectors;
			for (int32 i = 0; i < SpawnSectors.size(); i++)
			{
				for (int32 j = 0; j < CurrentSectors.size(); j++)
				{
					if (SpawnSectors[i]->_SectorY == CurrentSectors[j]->_SectorY && SpawnSectors[i]->_SectorX == CurrentSectors[j]->_SectorX)
					{
						SpawnSectors.erase(SpawnSectors.begin() + i);
					}
				}
			}

			// ������ ���迭
			vector<st_GameObjectInfo> SpawnObjectInfos;
			// ���� ������ ���
			SpawnObjectInfos.push_back(MoveMonster->_GameObjectInfo);

			// ���� ��ȯ�϶�� �ռ� ����� ���� �÷��̾�鿡�� ��Ŷ�� �����Ѵ�.
			CMessage* ResSectorSpawnPlayer = G_ObjectManager->GameServer->MakePacketResObjectSpawn(1, SpawnObjectInfos);
			for (int32 i = 0; i < SpawnSectors.size(); i++)
			{
				for (CPlayer* Player : SpawnSectors[i]->GetPlayers())
				{
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSectorSpawnPlayer);
				}
			}
			ResSectorSpawnPlayer->Free();
		}
	}
		break;	
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
	{
		CEnvironment* MoveEnvironment = (CEnvironment*)GameObject;

		CSector* CurrentSector = GameObject->_Channel->GetSector(MoveEnvironment->GetCellPosition());
		CSector* NextSector = GameObject->_Channel->GetSector(DestPosition);

		if (CurrentSector != NextSector)
		{
			CurrentSector->Remove(MoveEnvironment);

			// ȯ�� ���� �ű�� �� �ֺ� ���� 
			vector<CSector*> CurrentSectors = GameObject->_Channel->GetAroundSectors(MoveEnvironment->GetCellPosition(), 1);
			// ���� �ű�� �� �� �ֺ� ����
			vector<CSector*> NextSectors = GameObject->_Channel->GetAroundSectors(DestPosition, 1);

			// ���� ������ ���͸� ã�� �۾�
			// Current - Next;
			// ���� ���͵鿡�� ���� �̵��� ���͸� ������ ������ ���͸� ��´�.
			vector<CSector*> DeSpawnSectors = CurrentSectors;
			for (int32 i = 0; i < DeSpawnSectors.size(); i++)
			{
				for (int32 j = 0; j < NextSectors.size(); j++)
				{
					if (DeSpawnSectors[i]->_SectorY == NextSectors[j]->_SectorY && DeSpawnSectors[i]->_SectorX == NextSectors[j]->_SectorX)
					{
						DeSpawnSectors.erase(DeSpawnSectors.begin() + i);
					}
				}
			}

			vector<int64> DeSpawnSectorObjectIds;
			DeSpawnSectorObjectIds.push_back(MoveEnvironment->_GameObjectInfo.ObjectId);
			// ���� �����϶�� �޽����� ���� �� 
			CMessage* ResSectorDespawnPlayer = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, DeSpawnSectorObjectIds);
			// �ش� ���� �÷��̾�鿡�� �����Ѵ�.
			for (int32 i = 0; i < DeSpawnSectors.size(); i++)
			{
				for (CPlayer* Player : DeSpawnSectors[i]->GetPlayers())
				{
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSectorDespawnPlayer);
				}
			}
			ResSectorDespawnPlayer->Free();

			NextSector->Insert(MoveEnvironment);

			// ���� ������ ���͸� ã�� �۾�
			// Next - Current;
			// �̵��� ���Ϳ��� ���� ���͸� ������ ������ ���͸� ã�´�.
			vector<CSector*> SpawnSectors = NextSectors;
			for (int32 i = 0; i < SpawnSectors.size(); i++)
			{
				for (int32 j = 0; j < CurrentSectors.size(); j++)
				{
					if (SpawnSectors[i]->_SectorY == CurrentSectors[j]->_SectorY && SpawnSectors[i]->_SectorX == CurrentSectors[j]->_SectorX)
					{
						SpawnSectors.erase(SpawnSectors.begin() + i);
					}
				}
			}

			// ������ ���迭
			vector<st_GameObjectInfo> SpawnObjectInfos;
			// ���� ������ ���
			SpawnObjectInfos.push_back(MoveEnvironment->_GameObjectInfo);

			// ���� ��ȯ�϶�� �ռ� ����� ���� �÷��̾�鿡�� ��Ŷ�� �����Ѵ�.
			CMessage* ResSectorSpawnPlayer = G_ObjectManager->GameServer->MakePacketResObjectSpawn(1, SpawnObjectInfos);
			for (int32 i = 0; i < SpawnSectors.size(); i++)
			{
				for (CPlayer* Player : SpawnSectors[i]->GetPlayers())
				{
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSectorSpawnPlayer);
				}
			}
			ResSectorSpawnPlayer->Free();
		}
	}
		break;
	default:
		CRASH("ApplyMove GameObject Type �̻��� ��")
		break;
	}

	GameObject->_GameObjectInfo.ObjectPositionInfo.PositionX = DestPosition._X;
	GameObject->_GameObjectInfo.ObjectPositionInfo.PositionY = DestPosition._Y;	

	return true;
}

bool CMap::ApplyPositionUpdateItem(CItem* ItemObject, st_Vector2Int& NewPosition)
{	
	int32 X = NewPosition._X - _Left;
	int32 Y = _Down - NewPosition._Y;

	// �켱 �ش� ��ġ�� �����۵�� ���� ���� �������� ������ ���Ѵ�.
	// ���� ���� �������� ������ �̹� �ش� ��ġ�� ���� ��쿡
	// ī��Ʈ�� ������Ű�� ���� ���� �������� �ʿ� ������Ű�� �ʴ´�.
	for (int8 i = 0; i < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; i++)
	{
		if (_Items[Y][X][i] != nullptr && _Items[Y][X][i]->_ItemInfo.ItemSmallCategory == ItemObject->_ItemInfo.ItemSmallCategory)
		{
			_Items[Y][X][i]->_ItemInfo.ItemCount += ItemObject->_ItemInfo.ItemCount;

			CItem* FindItem = (CItem*)(G_ObjectManager->Find(_Items[Y][X][i]->_ItemInfo.ItemDBId, en_GameObjectType::OBJECT_ITEM));
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

	// ������ ����ó��
	CSector* CurrentSector = ItemObject->_Channel->GetSector(ItemObject->GetCellPosition());
	CSector* NextSector = ItemObject->_Channel->GetSector(NewPosition);
	
	if (CurrentSector != NextSector)
	{
		CurrentSector->Remove(ItemObject);

		// ������ ���� �ű�� �� �ֺ� ���� 
		vector<CSector*> CurrentSectors = ItemObject->_Channel->GetAroundSectors(ItemObject->GetCellPosition(), 1);
		// ���� �ű�� �� �� �ֺ� ����
		vector<CSector*> NextSectors = ItemObject->_Channel->GetAroundSectors(NewPosition, 1);

		// ���� ������ ���͸� ã�� �۾�
		// Current - Next;
		// ���� ���͵鿡�� ���� �̵��� ���͸� ������ ������ ���͸� ��´�.
		vector<CSector*> DeSpawnSectors = CurrentSectors;
		for (int32 i = 0; i < DeSpawnSectors.size(); i++)
		{
			for (int32 j = 0; j < NextSectors.size(); j++)
			{
				if (DeSpawnSectors[i]->_SectorY == NextSectors[j]->_SectorY && DeSpawnSectors[i]->_SectorX == NextSectors[j]->_SectorX)
				{
					DeSpawnSectors.erase(DeSpawnSectors.begin() + i);
				}
			}
		}

		vector<int64> DeSpawnSectorObjectIds;
		DeSpawnSectorObjectIds.push_back(ItemObject->_GameObjectInfo.ObjectId);
		// ���� �����϶�� �޽����� ���� �� 
		CMessage* ResSectorDespawnPlayer = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, DeSpawnSectorObjectIds);
		// �ش� ���� �÷��̾�鿡�� �����Ѵ�.
		for (int32 i = 0; i < DeSpawnSectors.size(); i++)
		{
			for (CPlayer* Player : DeSpawnSectors[i]->GetPlayers())
			{
				G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSectorDespawnPlayer);
			}
		}
		ResSectorDespawnPlayer->Free();

		NextSector->Insert(ItemObject);

		// ���� ������ ���͸� ã�� �۾�
		// Next - Current;
		// �̵��� ���Ϳ��� ���� ���͸� ������ ������ ���͸� ã�´�.
		vector<CSector*> SpawnSectors = NextSectors;
		for (int32 i = 0; i < SpawnSectors.size(); i++)
		{
			for (int32 j = 0; j < CurrentSectors.size(); j++)
			{
				if (SpawnSectors[i]->_SectorY == CurrentSectors[j]->_SectorY && SpawnSectors[i]->_SectorX == CurrentSectors[j]->_SectorX)
				{
					SpawnSectors.erase(SpawnSectors.begin() + i);
				}
			}
		}

		// ������ ���迭
		vector<st_GameObjectInfo> SpawnObjectInfos;
		// ���� ������ ���
		SpawnObjectInfos.push_back(ItemObject->_GameObjectInfo);

		// ���� ��ȯ�϶�� �ռ� ����� ���� �÷��̾�鿡�� ��Ŷ�� �����Ѵ�.
		CMessage* ResSectorSpawnPlayer = G_ObjectManager->GameServer->MakePacketResObjectSpawn(1, SpawnObjectInfos);
		for (int32 i = 0; i < SpawnSectors.size(); i++)
		{
			for (CPlayer* Player : SpawnSectors[i]->GetPlayers())
			{
				G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSectorSpawnPlayer);
			}
		}
		ResSectorSpawnPlayer->Free();
	}

	ItemObject->_GameObjectInfo.ObjectPositionInfo.PositionX = NewPosition._X;
	ItemObject->_GameObjectInfo.ObjectPositionInfo.PositionY = NewPosition._Y;

	return true;
}

bool CMap::ApplyLeave(CGameObject* GameObject)
{
	if (GameObject->_Channel == nullptr)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyLeave Channel is nullptr");
		return false;
	}

	if (GameObject->_Channel->_Map != this)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyLeave Channel _Map Error");
		return false;
	}
		
	st_PositionInfo PositionInfo = GameObject->GetPositionInfo();
	// �¿� ��ǥ �˻�
	if (PositionInfo.PositionX < _Left || PositionInfo.PositionX > _Right)
	{
		return false;
	}

	// ���� ��ǥ �˻�
	if (PositionInfo.PositionY < _Up || PositionInfo.PositionY > _Down)
	{
		return false;
	}	

	// ���Ϳ��� ������Ʈ ����
	CSector* Sector = GameObject->_Channel->GetSector(GameObject->GetCellPosition());	
	Sector->Remove(GameObject);

	int X = PositionInfo.PositionX - _Left;
	int Y = _Down - PositionInfo.PositionY;

	// �ʿ��� ����
	if (_ObjectsInfos[Y][X] == GameObject)
	{		
		_ObjectsInfos[Y][X] = nullptr;
	}
	else
	{
		if (GameObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
		{
			CRASH("ApplyLeave �����Ϸ��� ������Ʈ�� ����Ǿ� �ִ� ������Ʈ�� �ٸ�");
		}		
	}

	return true;
}

bool CMap::ApplyPositionLeaveItem(CGameObject* GameObject)
{
	int32 X = GameObject->_GameObjectInfo.ObjectPositionInfo.PositionX - _Left;
	int32 Y = _Down - GameObject->_GameObjectInfo.ObjectPositionInfo.PositionY;

	// �ʿ��� ���� ����
	if (GameObject->_Channel == nullptr)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyPositionLeaveItem Channel is nullptr");
		return false;
	}

	if (GameObject->_Channel->_Map != this)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyPositionLeaveItem Channel _Map Error");
		return false;
	}

	st_PositionInfo PositionInfo = GameObject->GetPositionInfo();
	// �¿� ��ǥ �˻�
	if (PositionInfo.PositionX < _Left || PositionInfo.PositionX > _Right)
	{
		return false;
	}

	// ���� ��ǥ �˻�
	if (PositionInfo.PositionY < _Up || PositionInfo.PositionY > _Down)
	{
		return false;
	}

	CSector* Sector = GameObject->_Channel->GetSector(GameObject->GetCellPosition());
	Sector->Remove(GameObject);	

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

	return true;
}

st_Position CMap::CellToPosition(st_Vector2Int CellPosition)
{
	return st_Position(_Down - CellPosition._Y, CellPosition._X - _Left);
}

st_Vector2Int CMap::PositionToCell(st_Position Position)
{
	return st_Vector2Int(Position._X + _Left, _Down - Position._Y);
}

vector<st_Vector2Int> CMap::FindPath(st_Vector2Int StartCellPosition, st_Vector2Int DestCellPostion, bool CheckObjects , int32 MaxDistance)
{
	int32 DeltaY[4] = { 1, -1, 0, 0 };
	int32 DeltaX[4] = { 0, 0, -1, 1 };
	int32 Cost[4] = { 10,10,10,10 };
	
	// ������ ������ ��ǥ��ȯ
	st_Position StartPosition = CellToPosition(StartCellPosition);
	st_Position DestPosition = CellToPosition(DestCellPostion);

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
	map<st_Position, st_Position> Parents;	

	// �켱���� ť ����
	CHeap<int32, st_AStarNode> OpenListQue(_SizeY * _SizeX);

	// ������忡 ó�� F �� ���
	OpenList[StartPosition._Y][StartPosition._X] = abs(DestPosition._Y - StartPosition._Y) + abs(DestPosition._X - StartPosition._X);

	// AStar Node ����
	st_AStarNode StartNode(abs(DestPosition._Y - StartPosition._Y) + abs(DestPosition._X - StartPosition._X), 0, StartPosition._X, StartPosition._Y);
	// ť�� ����
	OpenListQue.InsertHeap(StartNode._F, StartNode);
	
	// ó�� ��ġ ù �θ�� ����
	Parents.insert(pair<st_Position, st_Position>(StartPosition, StartPosition));

	// ���¸���Ʈ ť�� ����������� �ݺ�
	while (OpenListQue.GetUseSize() > 0)
	{
		// ���¸���Ʈ ť���� ��� �ϳ� ����
		st_AStarNode AStarNode = OpenListQue.PopHeap();
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
			st_Position NextPosition(AStarNode._Position._Y + DeltaY[i], AStarNode._Position._X + DeltaX[i]);			
			
			// �������� �̾Ƴ� ��ġ�� ���������� �������� ������ �ʹ� �ִٸ� �ش� ��ǥ�� �����Ѵ�.
			if (abs(StartPosition._Y - NextPosition._Y) + abs(StartPosition._X - NextPosition._X) > MaxDistance)
			{
				continue;
			}

			// ���� ��ġ�� ������ ��ǥ�� �ƴ� ���, �ش� ��ġ�� �� �� �ִ��� �˻��Ѵ�.
			if (NextPosition._Y != DestPosition._Y || NextPosition._X != DestPosition._X)
			{
				st_Vector2Int NextPositionVector = PositionToCell(NextPosition);

				// �� �� ������ ���� ��ġ�� �˻��Ѵ�.
				if (Cango(NextPositionVector, CheckObjects) == false)
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
			st_AStarNode InsertAStartNode;
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
				Parents.insert(pair<st_Position, st_Position>(NextPosition, AStarNode._Position));
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

vector<st_Vector2Int> CMap::CompletePath(map<st_Position, st_Position> Parents, st_Position DestPosition)
{
	// ��ȯ���� �迭
	vector<st_Vector2Int> Cells;	

	int32 X = DestPosition._X;
	int32 Y = DestPosition._Y;

	st_Position Point;			

	// �θ� ��� �߿��� �������� ������
	if (Parents.find(DestPosition) == Parents.end())
	{			
		st_Position BestPosition;
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

	st_Position Position = DestPosition;
	while ((*Parents.find(Position)).second != Position)
	{
		Cells.push_back(PositionToCell(Position));
		Position = (*Parents.find(Position)).second;
	}

	// ������ ���
	Cells.push_back(PositionToCell(Position));
	// �θ� ��ġ ���� �迭 �Ųٷ� ����� ��ȯ
	reverse(Cells.begin(), Cells.end());	

	return Cells;
}