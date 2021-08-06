#include "pch.h"
#include "Map.h"
#include "GameObject.h"

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

	_CollisionMapInfo = new bool*[YCount];

	for (int i = 0; i < YCount; i++)
	{
		_CollisionMapInfo[i] = new bool[XCount];
	}

	_ObjectsInfo = new CGameObject*[YCount];

	for (int i = 0; i < YCount; i++)
	{
		_ObjectsInfo[i] = new CGameObject[XCount];	
	}

	for (int Y = 0; Y < YCount; Y++)
	{
		for (int X = 0; X < XCount; X++)
		{
			if (X == XCount - 1)
			{
				int a = 0;
			}
			if (*ConvertP == '\r')
			{
				
				break;
			}		

			_CollisionMapInfo[Y][X] = *ConvertP - 48;		

			ConvertP++;
		}
		ConvertP += 2;
	}
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

	return &_ObjectsInfo[Y][X];
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

	return !_CollisionMapInfo[Y][X];
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
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyMove (%d) (%d)로 갈 수 없습니다.",DestPosition._Y,DestPosition._X);
		return false;
	}

	// 호출해준 대상을 충돌체로 여긴다면
	if (Applycollision == true)
	{
		int X = PositionInfo.PositionX = _Left;
		int Y = _Down = PositionInfo.PositionY;

		// 기존위치 데이터는 날리고
		if (&_ObjectsInfo[Y][X] == GameObject)
		{
			memset(&_ObjectsInfo[Y][X], 0, sizeof(CGameObject));
		}

		// 목적지 위치 구해주고
		X = DestPosition._X - _Left;
		Y = _Down - DestPosition._Y;
		// 목적지에 넣어준다.
		_ObjectsInfo[Y][X] = *GameObject;
	}	

	// 섹터 작업
	switch (GameObject->_GameObjectInfo.ObjectType)
	{
	case NORMAL:
		break;
	case PLAYER:		
		break;
	case MONSTER:
		break;
	default:
		CRASH("ApplyMove GameObject Type 이상한 값")
		break;
	}
}
