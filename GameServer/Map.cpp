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

bool CMap::Cango(CGameObject* Object, float X, float Y)
{
	st_Vector2Int CollisionPosition;	
	CollisionPosition._X = X;
	CollisionPosition._Y = Y;
	
	return CollisionCango(Object, CollisionPosition);
}

bool CMap::CollisionCango(CGameObject* Object, st_Vector2Int& CellPosition, bool CheckObjects)
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
	
	//G_Logger->WriteStdOut(en_Color::RED, L"X : %d Y : %d \n", X, Y);

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
	// 채널이 할당되어 있는지 확인
	if (GameObject->GetChannel() == nullptr)
	{
		CRASH("ApplyMove GameObject Channel nullptr")
		return false;
	}

	// 게임오브젝트가 속한 채널이 들고 있는 맵과 현재 맵이 같은지 확인
	if (GameObject->GetChannel()->_Map != this)
	{
		CRASH("ApplyMove GameObject의 채널이 가지고 있는 맵과 지금 맵이 다름")
		return false;
	}

	// 위치 정보 가지고 온다.
	st_PositionInfo PositionInfo = GameObject->_GameObjectInfo.ObjectPositionInfo;

	// 목적지로 갈 수 있는지 검사한다.
	if (CollisionCango(GameObject, DestPosition, CheckObject) == false)
	{
		//G_Logger->WriteStdOut(en_Color::RED, L"Cant Go ApplyMove Y (%d) X (%d) ",DestPosition._Y,DestPosition._X);
		return false;
	}

	// 호출해준 대상을 충돌체로 여긴다면
	if (Applycollision == true)
	{
		int X = PositionInfo.CollisionPositionX - _Left;
		int Y = _Down - PositionInfo.CollisionPositionY;

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
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	{
		CPlayer* MovePlayer = (CPlayer*)GameObject;

		CSector* CurrentSector = GameObject->GetChannel()->GetSector(MovePlayer->GetCellPosition());
		CSector* NextSector = GameObject->GetChannel()->GetSector(DestPosition);
				
		if (CurrentSector != NextSector)
		{			
			// 현재 섹터에서 플레이어 제거
			CurrentSector->Remove(MovePlayer);	
			// 이동한 섹터에 플레이어 추가
			NextSector->Insert(MovePlayer);			
		}				
	}
		break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
	{
		CMonster* MoveMonster = (CMonster*)GameObject;

		CSector* CurrentSector = GameObject->GetChannel()->GetSector(MoveMonster->GetCellPosition());
		CSector* NextSector = GameObject->GetChannel()->GetSector(DestPosition);

		if (CurrentSector != NextSector)
		{
			// 현재 섹터에서 몬스터 제거
			CurrentSector->Remove(MoveMonster);	
			// 이동한 섹터에 몬스터 추가
			NextSector->Insert(MoveMonster);			
		}		
	}
		break;	
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
	{
		CEnvironment* MoveEnvironment = (CEnvironment*)GameObject;

		CSector* CurrentSector = GameObject->GetChannel()->GetSector(MoveEnvironment->GetCellPosition());
		CSector* NextSector = GameObject->GetChannel()->GetSector(DestPosition);

		if (CurrentSector != NextSector)
		{
			CurrentSector->Remove(MoveEnvironment);
			NextSector->Insert(MoveEnvironment);			
		}
	}
		break;
	default:
		CRASH("ApplyMove GameObject Type 이상한 값")
		break;
	}

	// 시야 작업 ( 시야 안에 들어오는 오브젝트와 시야 밖으로 벗어나는 오브젝트 작업 )

	GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = DestPosition._X;
	GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = DestPosition._Y;	

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
	CSector* CurrentSector = ItemObject->GetChannel()->GetSector(ItemObject->GetCellPosition());
	CSector* NextSector = ItemObject->GetChannel()->GetSector(NewPosition);
	
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
	if (GameObject->GetChannel() == nullptr)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyLeave Channel is nullptr");
		return false;
	}

	if (GameObject->GetChannel()->_Map != this)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyLeave Channel _Map Error");
		return false;
	}
		
	st_PositionInfo PositionInfo = GameObject->GetPositionInfo();
	// 좌우 좌표 검사
	if (PositionInfo.CollisionPositionX < _Left || PositionInfo.CollisionPositionX > _Right)
	{
		return false;
	}

	// 상하 좌표 검사
	if (PositionInfo.CollisionPositionY < _Up || PositionInfo.CollisionPositionY > _Down)
	{
		return false;
	}	

	// 섹터에서 오브젝트 제거
	CSector* Sector = GameObject->GetChannel()->GetSector(GameObject->GetCellPosition());
	Sector->Remove(GameObject);

	int X = PositionInfo.CollisionPositionX - _Left;
	int Y = _Down - PositionInfo.CollisionPositionY;	

	// 맵에서 제거
	if (_ObjectsInfos[Y][X] == GameObject)
	{		
		_ObjectsInfos[Y][X] = nullptr;
	}
	else
	{	
		if (GameObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
		{
			//CRASH("ApplyLeave 삭제하려는 오브젝트가 저장되어 있는 오브젝트와 다름");
		}		
	}

	return true;
}

bool CMap::ApplyPositionLeaveItem(CGameObject* GameObject)
{
	int32 X = GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX - _Left;
	int32 Y = _Down - GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY;

	// 맵에서 정보 삭제
	if (GameObject->GetChannel() == nullptr)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyPositionLeaveItem Channel is nullptr");
		return false;
	}

	if (GameObject->GetChannel()->_Map != this)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyPositionLeaveItem Channel _Map Error");
		return false;
	}

	st_PositionInfo PositionInfo = GameObject->GetPositionInfo();
	// 좌우 좌표 검사
	if (PositionInfo.CollisionPositionX < _Left || PositionInfo.CollisionPositionX > _Right)
	{
		return false;
	}

	// 상하 좌표 검사
	if (PositionInfo.CollisionPositionY < _Up || PositionInfo.CollisionPositionY > _Down)
	{
		return false;
	}

	CSector* Sector = GameObject->GetChannel()->GetSector(GameObject->GetCellPosition());
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
	
	// 시작점 도착점 좌표변환
	st_PositionInt StartPosition = CellToPositionInt(StartCellPosition);
	st_PositionInt DestPosition = CellToPositionInt(DestCellPostion);

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
	map<st_PositionInt, st_PositionInt> Parents;	

	// 우선순위 큐 생성
	CHeap<int32, st_AStarNodeInt> OpenListQue(_SizeY * _SizeX);

	// 열린노드에 처음 F 값 기록
	OpenList[StartPosition._Y][StartPosition._X] = abs(DestPosition._Y - StartPosition._Y) + abs(DestPosition._X - StartPosition._X);

	// AStar Node 생성
	st_AStarNodeInt StartNode(abs(DestPosition._Y - StartPosition._Y) + abs(DestPosition._X - StartPosition._X), 0, StartPosition._X, StartPosition._Y);
	// 큐에 삽입
	OpenListQue.InsertHeap(StartNode._F, StartNode);
	
	// 처음 위치 첫 부모로 설정
	Parents.insert(pair<st_PositionInt, st_PositionInt>(StartPosition, StartPosition));

	// 오픈리스트 큐가 비워질때까지 반복
	while (OpenListQue.GetUseSize() > 0)
	{
		// 오픈리스트 큐에서 노드 하나 뽑음
		st_AStarNodeInt AStarNode = OpenListQue.PopHeap();
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
			st_PositionInt NextPosition(AStarNode._Position._Y + DeltaY[i], AStarNode._Position._X + DeltaX[i]);			
			
			// 다음으로 뽑아낸 위치가 시작점으로 지정해준 값보다 너무 멀다면 해당 좌표는 무시한다.
			if (abs(StartPosition._Y - NextPosition._Y) + abs(StartPosition._X - NextPosition._X) > MaxDistance)
			{
				continue;
			}

			// 다음 위치가 목적지 좌표가 아닐 경우, 해당 위치로 갈 수 있는지 검사한다.
			if (NextPosition._Y != DestPosition._Y || NextPosition._X != DestPosition._X)
			{
				st_Vector2Int NextPositionVector = PositionToCellInt(NextPosition);

				// 갈 수 없으면 다음 위치를 검사한다.
				if (CollisionCango(Object, NextPositionVector, CheckObjects) == false)
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
			st_AStarNodeInt InsertAStartNode;
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
				Parents.insert(pair<st_PositionInt, st_PositionInt>(NextPosition, AStarNode._Position));
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

vector<st_Vector2Int> CMap::CompletePath(map<st_PositionInt, st_PositionInt> Parents, st_PositionInt DestPosition)
{
	// 반환해줄 배열
	vector<st_Vector2Int> Cells;	

	int32 X = DestPosition._X;
	int32 Y = DestPosition._Y;

	st_PositionInt Point;			

	// 부모 목록 중에서 목적지가 없으면
	if (Parents.find(DestPosition) == Parents.end())
	{			
		st_PositionInt BestPosition;
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

	st_PositionInt Position = DestPosition;
	while ((*Parents.find(Position)).second != Position)
	{
		Cells.push_back(PositionToCellInt(Position));
		Position = (*Parents.find(Position)).second;
	}

	// 시작점 담기
	Cells.push_back(PositionToCellInt(Position));
	// 부모 위치 담은 배열 거꾸로 뒤집어서 반환
	reverse(Cells.begin(), Cells.end());	

	return Cells;
}