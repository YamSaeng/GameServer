#include "pch.h"
#include "Map.h"
#include "GameObject.h"
#include "Player.h"

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

	_ObjectsInfo = new CGameObject**[YCount];

	for (int i = 0; i < YCount; i++)
	{
		_ObjectsInfo[i] = new CGameObject*[XCount];			
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

	return _ObjectsInfo[Y][X];
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
	
	return !_CollisionMapInfo[Y][X] && (!CheckObjects || _ObjectsInfo[Y][X] == nullptr);
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
		G_Logger->WriteStdOut(en_Color::RED, L"Cant Go ApplyMove Y (%d) X (%d) ",DestPosition._Y,DestPosition._X);
		return false;
	}

	// ȣ������ ����� �浹ü�� ����ٸ�
	if (Applycollision == true)
	{
		int X = PositionInfo.PositionX - _Left;
		int Y = _Down - PositionInfo.PositionY;

		// ������ġ �����ʹ� ������
		if (_ObjectsInfo[Y][X] == GameObject)
		{
			_ObjectsInfo[Y][X] = nullptr;
		}

		// ������ ��ġ �����ְ�
		X = DestPosition._X - _Left;
		Y = _Down - DestPosition._Y;
		// �������� �־��ش�.
		_ObjectsInfo[Y][X] = GameObject;
	}	

	// ���� �۾�
	switch (GameObject->_GameObjectInfo.ObjectType)
	{	
	case PLAYER:		
	{
		CPlayer* Player = (CPlayer*)GameObject;		

		CSector* CurrentSector = GameObject->_Channel->GetSector(Player->GetCellPosition());		
		CSector* NextSector = GameObject->_Channel->GetSector(DestPosition);		

		if (CurrentSector != NextSector)
		{
			//G_Logger->WriteStdOut(en_Color::GREEN, L"LeaveSector Y (%d) X (%d) EnterSector Y (%d) X (%d) \n", CurrentSector->_SectorY, CurrentSector->_SectorX, NextSector->_SectorY, NextSector->_SectorX);			

			CurrentSector->Remove(Player);	
			NextSector->Insert(Player);			
		}
	}
		break;
	case MONSTER:
		break;
	default:
		CRASH("ApplyMove GameObject Type �̻��� ��")
		break;
	}

	GameObject->_GameObjectInfo.ObjectPositionInfo.PositionX = DestPosition._X;
	GameObject->_GameObjectInfo.ObjectPositionInfo.PositionY = DestPosition._Y;	

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
	if (_ObjectsInfo[Y][X] == GameObject)
	{		
		_ObjectsInfo[Y][X] = nullptr;
	}
	else
	{
		CRASH("ApplyLeave �����Ϸ��� ������Ʈ�� ����Ǿ� �ִ� ������Ʈ�� �ٸ�");
	}

	return true;
}
