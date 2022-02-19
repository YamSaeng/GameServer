#include "Map.h"
#include "ClientInfo.h"

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

	_CollisionMapInfos = new en_TileMapEnvironment * [YCount];

	for (int i = 0; i < YCount; i++)
	{
		_CollisionMapInfos[i] = new en_TileMapEnvironment[XCount];
	}

	_ObjectsInfos = new st_Client**[YCount];

	for (int i = 0; i < YCount; i++)
	{
		_ObjectsInfos[i] = new st_Client*[XCount];
		for (int j = 0; j < XCount; j++)
		{
			_ObjectsInfos[i][j] = nullptr;
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

st_Client* CMap::Find(st_Vector2Int& CellPosition)
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

bool CMap::Cango(st_Client* Object, float X, float Y)
{
	st_Vector2Int CollisionPosition;
	CollisionPosition._X = X;
	CollisionPosition._Y = Y;

	return CollisionCango(Object, CollisionPosition);
}

bool CMap::CollisionCango(st_Client* Object, st_Vector2Int& CellPosition, bool CheckObjects)
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
	}

	bool ObjectCheck = false;

	if (_ObjectsInfos[Y][X] == nullptr)
	{
		ObjectCheck = true;
	}
	else
	{
		if (_ObjectsInfos[Y][X]->MyCharacterGameObjectInfo.ObjectId == Object->MyCharacterGameObjectInfo.ObjectId)
		{
			ObjectCheck = true;
		}
		else
		{
			// Y 때문인지 X 때문인지 확인해야함
			ObjectCheck = false;
		}
	}

	return IsCollisionMapInfo && (!CheckObjects || ObjectCheck);
}

bool CMap::ApplyMove(st_Client* GameObject, st_Vector2Int& DestPosition, bool CheckObject, bool Applycollision)
{
	// 위치 정보 가지고 온다.
	st_PositionInfo PositionInfo = GameObject->MyCharacterGameObjectInfo.ObjectPositionInfo;

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

	// 시야 작업 ( 시야 안에 들어오는 오브젝트와 시야 밖으로 벗어나는 오브젝트 작업 )
	GameObject->MyCharacterGameObjectInfo.ObjectPositionInfo.CollisionPositionX = DestPosition._X;
	GameObject->MyCharacterGameObjectInfo.ObjectPositionInfo.CollisionPositionY = DestPosition._Y;

	return true;
}

bool CMap::ApplyLeave(st_Client* GameObject)
{	
	st_PositionInfo PositionInfo = GameObject->MyCharacterGameObjectInfo.ObjectPositionInfo;
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

	int X = PositionInfo.CollisionPositionX - _Left;
	int Y = _Down - PositionInfo.CollisionPositionY;

	// 맵에서 제거
	if (_ObjectsInfos[Y][X] == GameObject)
	{
		_ObjectsInfos[Y][X] = nullptr;
	}
	else
	{
		if (GameObject->MyCharacterGameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
		{
			//CRASH("ApplyLeave 삭제하려는 오브젝트가 저장되어 있는 오브젝트와 다름");
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