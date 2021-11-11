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
	// 좌우 좌표 검사
	if (CellPosition._X < _Left || CellPosition._X > _Right)
	{
		return nullptr;
	}

	// 상하 좌표 검사
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

	// 좌우 좌표 검사
	if (ItemCellPosition._X < _Left || ItemCellPosition._X > _Right)
	{
		return nullptr;
	}

	// 상하 좌표 검사
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
	// 좌우 좌표 검사
	if (CellPosition._X < _Left || CellPosition._X > _Right)
	{
		return false;
	}

	// 상하 좌표 검사
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
	// 채널이 할당되어 있는지 확인
	if (GameObject->_Channel == nullptr)
	{
		CRASH("ApplyMove GameObject Channel nullptr")
		return false;
	}

	// 게임오브젝트가 속한 채널이 들고 있는 맵과 현재 맵이 같은지 확인
	if (GameObject->_Channel->_Map != this)
	{
		CRASH("ApplyMove GameObject의 채널이 가지고 있는 맵과 지금 맵이 다름")
		return false;
	}

	// 위치 정보 가지고 온다.
	st_PositionInfo PositionInfo = GameObject->_GameObjectInfo.ObjectPositionInfo;

	// 목적지로 갈 수 있는지 검사한다.
	if (Cango(DestPosition, CheckObject) == false)
	{
		//G_Logger->WriteStdOut(en_Color::RED, L"Cant Go ApplyMove Y (%d) X (%d) ",DestPosition._Y,DestPosition._X);
		return false;
	}

	// 호출해준 대상을 충돌체로 여긴다면
	if (Applycollision == true)
	{
		int X = PositionInfo.PositionX - _Left;
		int Y = _Down - PositionInfo.PositionY;

		// 기존위치 데이터는 날리고
		if (_ObjectsInfos[Y][X] == GameObject)
		{
			_ObjectsInfos[Y][X] = nullptr;
		}

		// 목적지 위치 구해주고
		X = DestPosition._X - _Left;
		Y = _Down - DestPosition._Y;
		// 목적지에 넣어준다.
		_ObjectsInfos[Y][X] = GameObject;
	}		

	// 섹터 작업
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
			// 현재 섹터에서 플레이어 제거
			CurrentSector->Remove(MovePlayer);		

			vector<CSector*> CurrentSectors =  GameObject->_Channel->GetAroundSectors(MovePlayer->GetCellPosition(), 1);
			vector<CSector*> NextSectors =  GameObject->_Channel->GetAroundSectors(DestPosition, 1);			

			// 나를 제거할 섹터를 찾는 작업
			// Current - Next;
			// 현재 섹터들에서 내가 이동할 섹터를 제거한 차집합 섹터를 얻는다.
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
			// 나를 제외하라는 메시지를 생성 후 
			CMessage* ResSectorDespawnPlayer = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, DeSpawnSectorObjectIds);
			// 해당 섹터 플레이어들에게 전송한다.
			for (int32 i = 0; i < DeSpawnSectors.size(); i++)
			{
				for (CPlayer* Player : DeSpawnSectors[i]->GetPlayers())
				{
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSectorDespawnPlayer);
				}				
			}
			ResSectorDespawnPlayer->Free();			
			
			DeSpawnSectorObjectIds.clear();

			// DeSpawnSector에 있는 오브젝트들을 담아서 나한테서 제거한다.
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

			// 이동한 섹터에 플레이어 추가
			NextSector->Insert(MovePlayer);
			// 나를 스폰할 섹터를 찾는 작업
			// Next - Current;
			// 이동할 섹터에서 현재 섹터를 제거한 차집합 섹터를 찾는다.
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
		
			// 스폰할 대상배열
			vector<st_GameObjectInfo> SpawnObjectInfos;
			// 나의 정보를 담고
			SpawnObjectInfos.push_back(MovePlayer->_GameObjectInfo);

			// 나를 소환하라고 앞서 얻어준 섹터 플레이어들에게 패킷을 전송한다.
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
						
			// 반대로 스폰 섹터에 있는 오브젝트들을 담는다.
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
	
			// 나에게 전송하여 스폰 섹터에 있는 오브젝트들을 스폰 시킨다.
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
			// 현재 섹터에서 몬스터 제거
			CurrentSector->Remove(MoveMonster);		

			vector<CSector*> CurrentSectors = GameObject->_Channel->GetAroundSectors(MoveMonster->GetCellPosition(), 1);
			vector<CSector*> NextSectors = GameObject->_Channel->GetAroundSectors(DestPosition, 1);

			// 나를 제거할 섹터를 찾는 작업
			// Current - Next;
			// 현재 섹터들에서 내가 이동할 섹터를 제거한 차집합 섹터를 얻는다.
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
			// 나를 제외하라는 메시지를 생성 후 
			CMessage* ResSectorDespawnPlayer = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, DeSpawnSectorObjectIds);
			// 해당 섹터 플레이어들에게 전송한다.
			for (int32 i = 0; i < DeSpawnSectors.size(); i++)
			{
				for (CPlayer* Player : DeSpawnSectors[i]->GetPlayers())
				{
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSectorDespawnPlayer);
				}
			}
			ResSectorDespawnPlayer->Free();


			// 이동한 섹터에 몬스터 추가
			NextSector->Insert(MoveMonster);
			// 나를 스폰할 섹터를 찾는 작업
			// Next - Current;
			// 이동할 섹터에서 현재 섹터를 제거한 차집합 섹터를 찾는다.
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

			// 스폰할 대상배열
			vector<st_GameObjectInfo> SpawnObjectInfos;
			// 나의 정보를 담고
			SpawnObjectInfos.push_back(MoveMonster->_GameObjectInfo);

			// 나를 소환하라고 앞서 얻어준 섹터 플레이어들에게 패킷을 전송한다.
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

			// 환경 섹터 옮기기 전 주변 섹터 
			vector<CSector*> CurrentSectors = GameObject->_Channel->GetAroundSectors(MoveEnvironment->GetCellPosition(), 1);
			// 섹터 옮기고 난 후 주변 섹터
			vector<CSector*> NextSectors = GameObject->_Channel->GetAroundSectors(DestPosition, 1);

			// 나를 제거할 섹터를 찾는 작업
			// Current - Next;
			// 현재 섹터들에서 내가 이동할 섹터를 제거한 차집합 섹터를 얻는다.
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
			// 나를 제외하라는 메시지를 생성 후 
			CMessage* ResSectorDespawnPlayer = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, DeSpawnSectorObjectIds);
			// 해당 섹터 플레이어들에게 전송한다.
			for (int32 i = 0; i < DeSpawnSectors.size(); i++)
			{
				for (CPlayer* Player : DeSpawnSectors[i]->GetPlayers())
				{
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSectorDespawnPlayer);
				}
			}
			ResSectorDespawnPlayer->Free();

			NextSector->Insert(MoveEnvironment);

			// 나를 스폰할 섹터를 찾는 작업
			// Next - Current;
			// 이동할 섹터에서 현재 섹터를 제거한 차집합 섹터를 찾는다.
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

			// 스폰할 대상배열
			vector<st_GameObjectInfo> SpawnObjectInfos;
			// 나의 정보를 담고
			SpawnObjectInfos.push_back(MoveEnvironment->_GameObjectInfo);

			// 나를 소환하라고 앞서 얻어준 섹터 플레이어들에게 패킷을 전송한다.
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
		CRASH("ApplyMove GameObject Type 이상한 값")
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

	// 우선 해당 위치에 아이템들과 새로 얻은 아이템의 종류를 비교한다.
	// 새로 얻은 아이템의 종류가 이미 해당 위치에 있을 경우에
	// 카운트를 증가시키고 새로 얻은 아이템은 맵에 스폰시키지 않는다.
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

	// 아이템 섹터처리
	CSector* CurrentSector = ItemObject->_Channel->GetSector(ItemObject->GetCellPosition());
	CSector* NextSector = ItemObject->_Channel->GetSector(NewPosition);
	
	if (CurrentSector != NextSector)
	{
		CurrentSector->Remove(ItemObject);

		// 아이템 섹터 옮기기 전 주변 섹터 
		vector<CSector*> CurrentSectors = ItemObject->_Channel->GetAroundSectors(ItemObject->GetCellPosition(), 1);
		// 섹터 옮기고 난 후 주변 섹터
		vector<CSector*> NextSectors = ItemObject->_Channel->GetAroundSectors(NewPosition, 1);

		// 나를 제거할 섹터를 찾는 작업
		// Current - Next;
		// 현재 섹터들에서 내가 이동할 섹터를 제거한 차집합 섹터를 얻는다.
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
		// 나를 제외하라는 메시지를 생성 후 
		CMessage* ResSectorDespawnPlayer = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, DeSpawnSectorObjectIds);
		// 해당 섹터 플레이어들에게 전송한다.
		for (int32 i = 0; i < DeSpawnSectors.size(); i++)
		{
			for (CPlayer* Player : DeSpawnSectors[i]->GetPlayers())
			{
				G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSectorDespawnPlayer);
			}
		}
		ResSectorDespawnPlayer->Free();

		NextSector->Insert(ItemObject);

		// 나를 스폰할 섹터를 찾는 작업
		// Next - Current;
		// 이동할 섹터에서 현재 섹터를 제거한 차집합 섹터를 찾는다.
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

		// 스폰할 대상배열
		vector<st_GameObjectInfo> SpawnObjectInfos;
		// 나의 정보를 담고
		SpawnObjectInfos.push_back(ItemObject->_GameObjectInfo);

		// 나를 소환하라고 앞서 얻어준 섹터 플레이어들에게 패킷을 전송한다.
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
	// 좌우 좌표 검사
	if (PositionInfo.PositionX < _Left || PositionInfo.PositionX > _Right)
	{
		return false;
	}

	// 상하 좌표 검사
	if (PositionInfo.PositionY < _Up || PositionInfo.PositionY > _Down)
	{
		return false;
	}	

	// 섹터에서 오브젝트 제거
	CSector* Sector = GameObject->_Channel->GetSector(GameObject->GetCellPosition());	
	Sector->Remove(GameObject);

	int X = PositionInfo.PositionX - _Left;
	int Y = _Down - PositionInfo.PositionY;

	// 맵에서 제거
	if (_ObjectsInfos[Y][X] == GameObject)
	{		
		_ObjectsInfos[Y][X] = nullptr;
	}
	else
	{
		if (GameObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
		{
			CRASH("ApplyLeave 삭제하려는 오브젝트가 저장되어 있는 오브젝트와 다름");
		}		
	}

	return true;
}

bool CMap::ApplyPositionLeaveItem(CGameObject* GameObject)
{
	int32 X = GameObject->_GameObjectInfo.ObjectPositionInfo.PositionX - _Left;
	int32 Y = _Down - GameObject->_GameObjectInfo.ObjectPositionInfo.PositionY;

	// 맵에서 정보 삭제
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
	// 좌우 좌표 검사
	if (PositionInfo.PositionX < _Left || PositionInfo.PositionX > _Right)
	{
		return false;
	}

	// 상하 좌표 검사
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
	
	// 시작점 도착점 좌표변환
	st_Position StartPosition = CellToPosition(StartCellPosition);
	st_Position DestPosition = CellToPosition(DestCellPostion);

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
	map<st_Position, st_Position> Parents;	

	// 우선순위 큐 생성
	CHeap<int32, st_AStarNode> OpenListQue(_SizeY * _SizeX);

	// 열린노드에 처음 F 값 기록
	OpenList[StartPosition._Y][StartPosition._X] = abs(DestPosition._Y - StartPosition._Y) + abs(DestPosition._X - StartPosition._X);

	// AStar Node 생성
	st_AStarNode StartNode(abs(DestPosition._Y - StartPosition._Y) + abs(DestPosition._X - StartPosition._X), 0, StartPosition._X, StartPosition._Y);
	// 큐에 삽입
	OpenListQue.InsertHeap(StartNode._F, StartNode);
	
	// 처음 위치 첫 부모로 설정
	Parents.insert(pair<st_Position, st_Position>(StartPosition, StartPosition));

	// 오픈리스트 큐가 비워질때까지 반복
	while (OpenListQue.GetUseSize() > 0)
	{
		// 오픈리스트 큐에서 노드 하나 뽑음
		st_AStarNode AStarNode = OpenListQue.PopHeap();
		// 해당 위치를 방문했엇는지 확인
		if (CloseList[AStarNode._Position._Y][AStarNode._Position._X] == true)
		{
			continue;
		}

		// 해당 위치를 방문했다고 기록
		CloseList[AStarNode._Position._Y][AStarNode._Position._X] = true;

		// 뽑은 노드의 위치가 목적지라면 나간다
		if (AStarNode._Position._Y == DestPosition._Y && AStarNode._Position._X == DestPosition._X)
		{
			break;
		}

		// 뽑은 노드를 기준으로 해서 상 하 좌 우 노드를 검사하고 오픈리스트 큐에 넣는다.
		for (int32 i = 0; i < 4; i++)
		{
			// 다음 위치를 알아낸다.
			st_Position NextPosition(AStarNode._Position._Y + DeltaY[i], AStarNode._Position._X + DeltaX[i]);			
			
			// 다음으로 뽑아낸 위치가 시작점으로 지정해준 값보다 너무 멀다면 해당 좌표는 무시한다.
			if (abs(StartPosition._Y - NextPosition._Y) + abs(StartPosition._X - NextPosition._X) > MaxDistance)
			{
				continue;
			}

			// 다음 위치가 목적지 좌표가 아닐 경우, 해당 위치로 갈 수 있는지 검사한다.
			if (NextPosition._Y != DestPosition._Y || NextPosition._X != DestPosition._X)
			{
				st_Vector2Int NextPositionVector = PositionToCell(NextPosition);

				// 갈 수 없으면 다음 위치를 검사한다.
				if (Cango(NextPositionVector, CheckObjects) == false)
				{
					continue;
				}
			}

			// 상 하 좌 우 위치가 이미 방문 했었는지 확인한다.
			if (CloseList[NextPosition._Y][NextPosition._X] == true)
			{
				continue;
			}

			// G 값을 계산한다.
			int G = AStarNode._G + Cost[i];
			// 다음 위치와 목적지 위치를 계산한다.
			int H = abs(DestPosition._Y - NextPosition._Y) + abs(DestPosition._X - NextPosition._X);

			// 새로 계산한 F 값이 원래 있었던 F 값보다 작을 경우 새로 계산한다.
			if (G + H > OpenList[NextPosition._Y][NextPosition._X])
			{
				continue;
			}

			// 삽입할 노드를 준비하고
			st_AStarNode InsertAStartNode;
			// 앞서 구한 값들을 이용해 F값을 구한후 저장한다.
			OpenList[NextPosition._Y][NextPosition._X] = G + H;

			// 삽입할 노드를 초기화 한 후에
			InsertAStartNode._F = G + H;
			InsertAStartNode._G = G;
			InsertAStartNode._Position._X = NextPosition._X;
			InsertAStartNode._Position._Y = NextPosition._Y;

			// 우선순위 큐에 넣는다.
			OpenListQue.InsertHeap(InsertAStartNode._F, InsertAStartNode);
			// 부모 목록에 다음 위치가 있는지 확인
			auto ChildFind = Parents.find(NextPosition);
			if (ChildFind == Parents.end())
			{
				//찾지 못햇으면 넣어주고
				Parents.insert(pair<st_Position, st_Position>(NextPosition, AStarNode._Position));
			}
			else
			{
				//이미 있었으면 부모 노드를 바꿔준다.
				(*ChildFind).second = AStarNode._Position;
			}
		}
	}	

	// 위에서 할당한 메모리를 삭제한다.
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
	// 반환해줄 배열
	vector<st_Vector2Int> Cells;	

	int32 X = DestPosition._X;
	int32 Y = DestPosition._Y;

	st_Position Point;			

	// 부모 목록 중에서 목적지가 없으면
	if (Parents.find(DestPosition) == Parents.end())
	{			
		st_Position BestPosition;
		int32 BestDistance;
		memset(&BestDistance, 1, sizeof(int32));

		// 가지고 있는 위치 중에서 가장 가까운 위치를 찾고
		// 해당 위치를 목적지로 삼는다.
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

	// 시작점 담기
	Cells.push_back(PositionToCell(Position));
	// 부모 위치 담은 배열 거꾸로 뒤집어서 반환
	reverse(Cells.begin(), Cells.end());	

	return Cells;
}