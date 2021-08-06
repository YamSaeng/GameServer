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

	return &_ObjectsInfo[Y][X];
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

	return !_CollisionMapInfo[Y][X];
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
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyMove (%d) (%d)�� �� �� �����ϴ�.",DestPosition._Y,DestPosition._X);
		return false;
	}

	// ȣ������ ����� �浹ü�� ����ٸ�
	if (Applycollision == true)
	{
		int X = PositionInfo.PositionX = _Left;
		int Y = _Down = PositionInfo.PositionY;

		// ������ġ �����ʹ� ������
		if (&_ObjectsInfo[Y][X] == GameObject)
		{
			memset(&_ObjectsInfo[Y][X], 0, sizeof(CGameObject));
		}

		// ������ ��ġ �����ְ�
		X = DestPosition._X - _Left;
		Y = _Down - DestPosition._Y;
		// �������� �־��ش�.
		_ObjectsInfo[Y][X] = *GameObject;
	}	

	// ���� �۾�
	switch (GameObject->_GameObjectInfo.ObjectType)
	{
	case NORMAL:
		break;
	case PLAYER:		
		break;
	case MONSTER:
		break;
	default:
		CRASH("ApplyMove GameObject Type �̻��� ��")
		break;
	}
}
