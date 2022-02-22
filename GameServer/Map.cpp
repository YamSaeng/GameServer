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
	}

	bool ObjectCheck = false;

	if (_ObjectsInfos[Y][X] == nullptr)
	{
		ObjectCheck = true;
	}
	else
	{
		if (_ObjectsInfos[Y][X]->_GameObjectInfo.ObjectId == Object->_GameObjectInfo.ObjectId)
		{
			ObjectCheck = true;
		}		
		else
		{				
			ObjectCheck = false;
		}
	}	

	return IsCollisionMapInfo && (!CheckObjects || ObjectCheck);
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
	if (CollisionCango(GameObject, DestPosition, CheckObject) == false)
	{
		//G_Logger->WriteStdOut(en_Color::RED, L"Cant Go ApplyMove Y (%d) X (%d) ",DestPosition._Y,DestPosition._X);
		return false;
	}

	// ȣ������ ����� �浹ü�� ����ٸ�
	if (Applycollision == true)
	{
		int X = PositionInfo.CollisionPositionX - _Left;
		int Y = _Down - PositionInfo.CollisionPositionY;

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
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	{
		CPlayer* MovePlayer = (CPlayer*)GameObject;

		CSector* CurrentSector = GameObject->_Channel->GetSector(MovePlayer->GetCellPosition());		
		CSector* NextSector = GameObject->_Channel->GetSector(DestPosition);				

		if (CurrentSector != NextSector)
		{			
			// ���� ���Ϳ��� �÷��̾� ����
			CurrentSector->Remove(MovePlayer);	
			// �̵��� ���Ϳ� �÷��̾� �߰�
			NextSector->Insert(MovePlayer);			
		}
		
		// ���� ������
		vector<CSector*> AroundSectors = GameObject->_Channel->GetAroundSectors(MovePlayer->GetCellPosition(), 1);
		
		// ���� ���Ϳ��� �� �þ� ���� �ȿ� ���ϴ� ������Ʈ ��������
		vector<CGameObject*> CurrentFieldOfViewObjects;

		for (CSector* AroundSector : AroundSectors)
		{
			AroundSector->GetSectorLock();

			// �÷��̾� ���
			for (CPlayer* Player : AroundSector->GetPlayers())
			{
				int16 Distance = st_Vector2Int::Distance(MovePlayer->GetCellPosition(), Player->GetCellPosition());				

				if (MovePlayer->_GameObjectInfo.ObjectId != Player->_GameObjectInfo.ObjectId 
					&& Distance <= MovePlayer->_FieldOfViewDistance)
				{
					CurrentFieldOfViewObjects.push_back(Player);
				}								
			}

			// ���� ���
			for (CMonster* Monster : AroundSector->GetMonsters())
			{
				int16 Distance = st_Vector2Int::Distance(MovePlayer->GetCellPosition(), Monster->GetCellPosition());

				if (Distance <= MovePlayer->_FieldOfViewDistance)
				{
					CurrentFieldOfViewObjects.push_back(Monster);
				}
			}

			// ȯ�� ���
			for (CEnvironment* Environment : AroundSector->GetEnvironment())
			{
				int16 Distance = st_Vector2Int::Distance(MovePlayer->GetCellPosition(), Environment->GetCellPosition());

				if (Distance <= MovePlayer->_FieldOfViewDistance)
				{
					CurrentFieldOfViewObjects.push_back(Environment);
				}
			}

			// ������ ���
			for (CItem* Item : AroundSector->GetItems())
			{
				int16 Distance = st_Vector2Int::Distance(MovePlayer->GetCellPosition(), Item->GetCellPosition());

				if (Distance <= MovePlayer->_FieldOfViewDistance)
				{
					CurrentFieldOfViewObjects.push_back(Item);
				}
			}
		}

		// �÷��̾ �����̱��� ���� �þ� ���� ������Ʈ��
		vector<CGameObject*> PreviousFieldOfViewObjects = MovePlayer->_FieldOfViewObjects;	
		
		vector<CGameObject*> SpawnFieldOfViewObjects = CurrentFieldOfViewObjects;
		vector<CGameObject*> DeSpawnFieldOfViewObjects = PreviousFieldOfViewObjects;

		// ���� �þ� ������Ʈ��� ���� �þ� ������Ʈ���� ���Ѵ�.
		// ���� �þ� ������Ʈ���� ���� �þ� ������Ʈ�� ���� �ϸ� ���� �����ؾ��� ������Ʈ��� �� �� �ִ�.
		for (int16 i = 0; i < CurrentFieldOfViewObjects.size(); i++)
		{
			for (int16 j = 0; j < PreviousFieldOfViewObjects.size(); j++)
			{
				if (CurrentFieldOfViewObjects[i]->_GameObjectInfo.ObjectId == PreviousFieldOfViewObjects[j]->_GameObjectInfo.ObjectId)
				{
					for (int16 k = 0; k < SpawnFieldOfViewObjects.size(); k++)
					{
						if (CurrentFieldOfViewObjects[i]->_GameObjectInfo.ObjectId == SpawnFieldOfViewObjects[k]->_GameObjectInfo.ObjectId)
						{
							int16 Distance = st_Vector2Int::Distance(MovePlayer->GetCellPosition(), SpawnFieldOfViewObjects[k]->GetCellPosition());

							if (Distance > MovePlayer->_FieldOfViewDistance)
							{
								SpawnFieldOfViewObjects.erase(SpawnFieldOfViewObjects.begin() + k);
							}

							break;
						}
					}					
					break;
				}
			}
		}
		
		if (SpawnFieldOfViewObjects.size() > 0)
		{
			// ���� �ؾ��� ������ ����
			vector<st_GameObjectInfo> OtherSpawnObjectInfos;
			for (CGameObject* SpawnObject : SpawnFieldOfViewObjects)
			{
				OtherSpawnObjectInfos.push_back(SpawnObject->_GameObjectInfo);
			}
						
			CMessage* ResOtherObjectSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn((int32)SpawnFieldOfViewObjects.size(), OtherSpawnObjectInfos);
			G_ObjectManager->GameServer->SendPacket(MovePlayer->_SessionId, ResOtherObjectSpawnPacket);
			ResOtherObjectSpawnPacket->Free();

			// ���� �ؾ��� ���鿡�� ���� �����϶�� �˸�
			vector<st_GameObjectInfo> MyCharacterSpawnObjectInfos;
			MyCharacterSpawnObjectInfos.push_back(MovePlayer->_GameObjectInfo);
						
			CMessage* ResMyObjectSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn(1, MyCharacterSpawnObjectInfos);
			for (CGameObject* SpawnObject : SpawnFieldOfViewObjects)
			{
				if (SpawnObject->_IsSendPacketTarget == true)
				{
					G_ObjectManager->GameServer->SendPacket(((CPlayer*)SpawnObject)->_SessionId, ResMyObjectSpawnPacket);
				}
			}
			ResMyObjectSpawnPacket->Free();
		}		
		

		// ���� �þ� ������Ʈ��� ���� �þ� ������Ʈ���� ���Ѵ�.
		// ���� �þ� ������Ʈ�鿡�� ���� �þ� ������Ʈ���� �����ϸ� ���� �ؾ��� ������Ʈ�� Ȯ�� �� �� �ִ�.
		for (int16 i = 0; i < PreviousFieldOfViewObjects.size(); i++)
		{
			for (int16 j = 0; j < CurrentFieldOfViewObjects.size(); j++)
			{
				if (PreviousFieldOfViewObjects[i]->_GameObjectInfo.ObjectId == CurrentFieldOfViewObjects[j]->_GameObjectInfo.ObjectId)
				{
					for (int16 k = 0; k < DeSpawnFieldOfViewObjects.size(); k++)
					{
						if (PreviousFieldOfViewObjects[i]->_GameObjectInfo.ObjectId == DeSpawnFieldOfViewObjects[k]->_GameObjectInfo.ObjectId)
						{
							int16 Distance = st_Vector2Int::Distance(MovePlayer->GetCellPosition(), DeSpawnFieldOfViewObjects[k]->GetCellPosition());

							if (Distance < MovePlayer->_FieldOfViewDistance)
							{
								DeSpawnFieldOfViewObjects.erase(DeSpawnFieldOfViewObjects.begin() + k);
							}						
							
							break;
						}
					}
					break;					
				}
			}
		}

		if (DeSpawnFieldOfViewObjects.size() > 0)
		{
			// ���� �ؾ��� ������ ����
			vector<int64> DeSpawnObjectInfos;
			for (CGameObject* DeSpawnObject : DeSpawnFieldOfViewObjects)
			{
				if (DeSpawnObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
				{
					DeSpawnObjectInfos.push_back(DeSpawnObject->_GameObjectInfo.ObjectId);
				}				
			}

			CMessage* ResOtherCharacterDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn((int32)DeSpawnObjectInfos.size(), DeSpawnObjectInfos);
			G_ObjectManager->GameServer->SendPacket(MovePlayer->_SessionId, ResOtherCharacterDeSpawnPacket);
			ResOtherCharacterDeSpawnPacket->Free();

			// ���� �ؾ��� ���鿡�� ���� ���� �϶�� �˸�
			vector<int64> MyCharacterDeSpawnObjectInfos;
			MyCharacterDeSpawnObjectInfos.push_back(MovePlayer->_GameObjectInfo.ObjectId);
			
			CMessage* ResMyObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, MyCharacterDeSpawnObjectInfos);
			for (CGameObject* DeSpawnObject : DeSpawnFieldOfViewObjects)
			{
				if (DeSpawnObject->_IsSendPacketTarget == true)
				{
					G_ObjectManager->GameServer->SendPacket(((CPlayer*)DeSpawnObject)->_SessionId, ResMyObjectDeSpawnPacket);
				}
			}
			ResMyObjectDeSpawnPacket->Free();
		}		

		MovePlayer->_FieldOfViewObjects = CurrentFieldOfViewObjects;	

		for (CSector* AroundSector : AroundSectors)
		{
			AroundSector->GetSectorUnLock();
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
			// �̵��� ���Ϳ� ���� �߰�
			NextSector->Insert(MoveMonster);			
		}
		
		// ���� ������
		vector<CSector*> AroundSectors = GameObject->_Channel->GetAroundSectors(MoveMonster->GetCellPosition(), 1);

		// ���� ���� �þ� ���� ������Ʈ ��������
		vector<CPlayer*> CurrentFieldOfViewObjects;

		for (CSector* AroundSector : AroundSectors)
		{
			AroundSector->GetSectorLock();

			// �÷��̾� ���
			for (CPlayer* Player : AroundSector->GetPlayers())
			{
				int16 Distance = st_Vector2Int::Distance(MoveMonster->GetCellPosition(), Player->GetCellPosition());

				if (Distance <= MoveMonster->_FieldOfViewDistance)
				{
					CurrentFieldOfViewObjects.push_back(Player);
				}
			}			
		}

		vector<CPlayer*> PreviousFieldOfViewObjects = MoveMonster->_FieldOfViewPlayers;

		vector<CPlayer*> SpawnFieldOfViewObjects = CurrentFieldOfViewObjects;		
		vector<CPlayer*> DeSpawnFieldOfViewObjects = PreviousFieldOfViewObjects;
		
		// ���� �þ� ������Ʈ��� ���� �þ� ������Ʈ���� ���Ѵ�.
		// ���� �þ� ������Ʈ���� ���� �þ� ������Ʈ�� ���� �ϸ� ���� �����ؾ��� ������Ʈ��� �� �� �ִ�.
		for (int16 i = 0; i < CurrentFieldOfViewObjects.size(); i++)
		{
			for (int16 j = 0; j < PreviousFieldOfViewObjects.size(); j++)
			{
				if (CurrentFieldOfViewObjects[i]->_GameObjectInfo.ObjectId == PreviousFieldOfViewObjects[j]->_GameObjectInfo.ObjectId)
				{
					for (int16 k = 0; k < SpawnFieldOfViewObjects.size(); k++)
					{
						if (CurrentFieldOfViewObjects[i]->_GameObjectInfo.ObjectId == SpawnFieldOfViewObjects[k]->_GameObjectInfo.ObjectId)
						{
							int16 Distance = st_Vector2Int::Distance(SpawnFieldOfViewObjects[k]->GetCellPosition(), MoveMonster->GetCellPosition());

							if (Distance > SpawnFieldOfViewObjects[k]->_FieldOfViewDistance)
							{
								SpawnFieldOfViewObjects.erase(SpawnFieldOfViewObjects.begin() + k);
							}							
							break;
						}
					}
					break;
				}
			}
		}
				
		if (SpawnFieldOfViewObjects.size() > 0)
		{
			// ���� �ؾ��� ���鿡�� ���� �����϶�� �˸�
			vector<st_GameObjectInfo> MyCharacterSpawnObjectInfos;
			MyCharacterSpawnObjectInfos.push_back(MoveMonster->_GameObjectInfo);

			CMessage* ResMyObjectSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn(1, MyCharacterSpawnObjectInfos);
			for (CGameObject* SpawnObject : SpawnFieldOfViewObjects)
			{
				if (SpawnObject->_IsSendPacketTarget == true)
				{
					G_ObjectManager->GameServer->SendPacket(((CPlayer*)SpawnObject)->_SessionId, ResMyObjectSpawnPacket);
				}
			}
			ResMyObjectSpawnPacket->Free();
		}		
		
		// ���� �þ� ������Ʈ��� ���� �þ� ������Ʈ���� ���Ѵ�.
		// ���� �þ� ������Ʈ�鿡�� ���� �þ� ������Ʈ���� �����ϸ� ���� �ؾ��� ������Ʈ�� Ȯ�� �� �� �ִ�.
		for (int16 i = 0; i < PreviousFieldOfViewObjects.size(); i++)
		{
			for (int16 j = 0; j < CurrentFieldOfViewObjects.size(); j++)
			{
				if (PreviousFieldOfViewObjects[i]->_GameObjectInfo.ObjectId == CurrentFieldOfViewObjects[j]->_GameObjectInfo.ObjectId)
				{
					for (int16 k = 0; k < DeSpawnFieldOfViewObjects.size(); k++)
					{
						if (PreviousFieldOfViewObjects[i]->_GameObjectInfo.ObjectId == DeSpawnFieldOfViewObjects[k]->_GameObjectInfo.ObjectId)
						{
							int16 Distance = st_Vector2Int::Distance(DeSpawnFieldOfViewObjects[k]->GetCellPosition(), MoveMonster->GetCellPosition());
							
							if (Distance < DeSpawnFieldOfViewObjects[k]->_FieldOfViewDistance)
							{
								DeSpawnFieldOfViewObjects.erase(DeSpawnFieldOfViewObjects.begin() + k);
							}							
							break;
						}
					}
					break;
				}
			}
		}

		if (DeSpawnFieldOfViewObjects.size() > 0)
		{
			// ���� �ؾ��� ���鿡�� ���� ���� �϶�� �˸�
			vector<int64> MyCharacterDeSpawnObjectInfos;
			MyCharacterDeSpawnObjectInfos.push_back(MoveMonster->_GameObjectInfo.ObjectId);

			CMessage* ResMyObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, MyCharacterDeSpawnObjectInfos);
			for (CGameObject* DeSpawnObject : DeSpawnFieldOfViewObjects)
			{
				if (DeSpawnObject->_IsSendPacketTarget == true)
				{
					G_ObjectManager->GameServer->SendPacket(((CPlayer*)DeSpawnObject)->_SessionId, ResMyObjectDeSpawnPacket);
				}
			}
			ResMyObjectDeSpawnPacket->Free();
		}		

		MoveMonster->_FieldOfViewPlayers = CurrentFieldOfViewObjects;		

		for (CSector* AroundSector : AroundSectors)
		{
			AroundSector->GetSectorUnLock();
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
			NextSector->Insert(MoveEnvironment);			
		}
	}
		break;
	default:
		CRASH("ApplyMove GameObject Type �̻��� ��")
		break;
	}

	// �þ� �۾� ( �þ� �ȿ� ������ ������Ʈ�� �þ� ������ ����� ������Ʈ �۾� )

	GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = DestPosition._X;
	GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = DestPosition._Y;	

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
		NextSector->Insert(ItemObject);		
	}

	ItemObject->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = NewPosition._X;
	ItemObject->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = NewPosition._Y;

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
	if (PositionInfo.CollisionPositionX < _Left || PositionInfo.CollisionPositionX > _Right)
	{
		return false;
	}

	// ���� ��ǥ �˻�
	if (PositionInfo.CollisionPositionY < _Up || PositionInfo.CollisionPositionY > _Down)
	{
		return false;
	}	

	// ���Ϳ��� ������Ʈ ����
	CSector* Sector = GameObject->_Channel->GetSector(GameObject->GetCellPosition());	
	Sector->Remove(GameObject);

	int X = PositionInfo.CollisionPositionX - _Left;
	int Y = _Down - PositionInfo.CollisionPositionY;	

	// �ʿ��� ����
	if (_ObjectsInfos[Y][X] == GameObject)
	{		
		_ObjectsInfos[Y][X] = nullptr;
	}
	else
	{	
		if (GameObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
		{
			//CRASH("ApplyLeave �����Ϸ��� ������Ʈ�� ����Ǿ� �ִ� ������Ʈ�� �ٸ�");
		}		
	}

	return true;
}

bool CMap::ApplyPositionLeaveItem(CGameObject* GameObject)
{
	int32 X = GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX - _Left;
	int32 Y = _Down - GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY;

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
	if (PositionInfo.CollisionPositionX < _Left || PositionInfo.CollisionPositionX > _Right)
	{
		return false;
	}

	// ���� ��ǥ �˻�
	if (PositionInfo.CollisionPositionY < _Up || PositionInfo.CollisionPositionY > _Down)
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

st_PositionInt CMap::CellToPositionInt(st_Vector2Int CellPosition)
{
	return st_PositionInt(_Down - CellPosition._Y, CellPosition._X - _Left);
}

st_Vector2Int CMap::PositionToCellInt(st_PositionInt Position)
{
	return st_Vector2Int(Position._X + _Left, _Down - Position._Y);
}

vector<st_Vector2Int> CMap::FindPath(CGameObject* Object, st_Vector2Int StartCellPosition, st_Vector2Int DestCellPostion, bool CheckObjects , int32 MaxDistance)
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