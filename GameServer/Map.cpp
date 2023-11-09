#include "pch.h"
#include "Map.h"
#include "GameObject.h"
#include "Player.h"
#include "NonPlayer.h"
#include "Monster.h"
#include "Heap.h"
#include "ObjectManager.h"
#include "Item.h"
#include "CraftingTable.h"
#include "Crop.h"
#include "DBConnectionPool.h"
#include "DBStoreProcedure.h"
#include "RectCollision.h"

CMap::CMap()
{
	_MapID = 0;

	_Left = 0;
	_Right = 0;
	_Up = 0;
	_Down = 0;

	_SizeX = 0;
	_SizeY = 0;

	_ObjectsInfos = nullptr;
	_SeedObjectInfos = nullptr;
	_Items = nullptr;

	_ChannelManager = nullptr;

	_Sectors = nullptr;
	_SectorSize = 0;
	_SectorCountX = 0;
	_SectorCountY = 0;
}

CMap::~CMap()
{
	if (_ChannelManager != nullptr)
	{
		delete _ChannelManager;
		_ChannelManager = nullptr;
	}

	if (_Sectors != nullptr)
	{
		for (int8 i = 0; i < _SectorCountY; i++)
		{
			delete _Sectors[i];
		}

		delete _Sectors;
		_Sectors = nullptr;
	}
}

void CMap::MapInit()
{	
	int XCount = _Right - _Left + 1;
	int YCount = _Up - _Down  + 1;			

	for (int i = 0; i < YCount; i++)
	{
		vector<st_TileInfo> TileInfos;		

		_TileInfos.push_back(TileInfos);		
	}	

	_ObjectsInfos = new CGameObject * *[YCount];

	for (int i = 0; i < YCount; i++)
	{
		_ObjectsInfos[i] = new CGameObject * [XCount];
		for (int j = 0; j < XCount; j++)
		{
			_ObjectsInfos[i][j] = nullptr;
		}
	}

	_SeedObjectInfos = new CGameObject * *[YCount];

	for (int i = 0; i < YCount; i++)
	{
		_SeedObjectInfos[i] = new CGameObject * [XCount];
		for (int j = 0; j < XCount; j++)
		{
			_SeedObjectInfos[i][j] = nullptr;
		}
	}

	_Items = new CItem * **[YCount];

	for (int i = 0; i < YCount; i++)
	{
		_Items[i] = new CItem * *[XCount];
		for (int j = 0; j < XCount; j++)
		{
			_Items[i][j] = new CItem * [(int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX];

			for (int8 k = 0; k < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; k++)
			{
				_Items[i][j][k] = nullptr;
			}
		}
	}

	_SizeX = _Right - _Left + 1;
	_SizeY = _Up - _Down + 1;
	_ChannelManager = new CChannelManager();
	_ChannelManager->Init(this, _ChannelCount);	

	// �� ũ�⸦ ���� ���Ͱ� ���� ���� ��� �ִ��� Ȯ��
	_SectorCountY = (_SizeY + _SectorSize - 1) / _SectorSize;
	_SectorCountX = (_SizeX + _SectorSize - 1) / _SectorSize;

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

CSector* CMap::GetSector(Vector2Int CellPosition)
{
	int X = (CellPosition.X - _Left) / _SectorSize;
	int Y = (_Up - CellPosition.Y) / _SectorSize;

	return GetSector(Y, X);
}

CSector* CMap::GetSector(int32 IndexY, int32 IndexX)
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

vector<CSector*> CMap::GetAroundSectors(Vector2Int CellPosition, int32 Range)
{
	CSector* Sector = GetSector(CellPosition);

	if (Sector != nullptr)
	{
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
}

vector<st_FieldOfViewInfo> CMap::GetFieldOfViewObjects(CPlayer* Object)
{
	vector<CGameObject*> FieldOfViewGameObjectInfos;
	vector<st_FieldOfViewInfo> FieldOfViewGameObjects;

	// ������Ʈ �������� ���� ���͸� ������
	vector<CSector*> Sectors = GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, 1);

	st_FieldOfViewInfo FieldOfViewInfo;

	// ������Ʈ �þ� ���� �ȿ� �ִ� ������Ʈ���� ����
	for (CSector* Sector : Sectors)
	{
		Sector->AcquireSectorLock();

		for (CPlayer* Player : Sector->GetPlayers())
		{			
			if (Player->_GameObjectInfo.ObjectId != Object->_GameObjectInfo.ObjectId)				
			{
				float Distance = Vector2::Distance(Player->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
				if (Distance < Object->_FieldOfViewDistance)
				{
					FieldOfViewGameObjectInfos.push_back(Player);					
				}
			}			
		}

		for (CNonPlayer* NonPlayer : Sector->GetNonPlayers())
		{
			float Distance = Vector2::Distance(NonPlayer->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
			if (Distance < Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjectInfos.push_back(NonPlayer);
			}
		}
		
		for (CMonster* Monster : Sector->GetMonsters())
		{
			float Distance = Vector2::Distance(Monster->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
			if (Distance < Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjectInfos.push_back(Monster);
			}			
		}

		for (CEnvironment* Enviroment : Sector->GetEnvironment())
		{
			if (Enviroment->_GameObjectInfo.ObjectType != en_GameObjectType::OBJECT_WALL)
			{
				float Distance = Vector2::Distance(Enviroment->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
				if (Distance < Object->_FieldOfViewDistance)
				{
					FieldOfViewGameObjectInfos.push_back(Enviroment);
				}
			}			
		}		

		// SkillObject�� ��� �� �ڿ� ������ �ִ��� �߰������� �˻������� ����
		// _ObjectsInfos�� �������� �ʱ� ������ ���� ���Ϳ��� �þ� ���� �ȿ� ���� ��� ����ش�.
		for (CGameObject* SkillObject : Sector->GetSkillObject())
		{
			if (SkillObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
			{
				float Distance = Vector2::Distance(SkillObject->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
				if (Distance < Object->_FieldOfViewDistance)
				{					
					FieldOfViewInfo.ObjectID = SkillObject->_GameObjectInfo.ObjectId;
					FieldOfViewInfo.ObjectType = SkillObject->_GameObjectInfo.ObjectType;

					FieldOfViewGameObjects.push_back(FieldOfViewInfo);
				}
			}			
		}

		for (CItem* Item : Sector->GetItems())
		{
			if (Item->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
			{
				FieldOfViewInfo.ObjectID = Item->_GameObjectInfo.ObjectId;
				FieldOfViewInfo.ObjectType = Item->_GameObjectInfo.ObjectType;

				FieldOfViewGameObjects.push_back(FieldOfViewInfo);
			}
		}

		for (CCraftingTable* CraftingTable : Sector->GetCraftingTable())
		{
			float Distance = Vector2::Distance(CraftingTable->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
			if (Distance < Object->_FieldOfViewDistance)
			{
				FieldOfViewGameObjectInfos.push_back(CraftingTable);
			}
		}	

		Sector->ReleaseSectorLock();
	}

	// �þ� ���� �ȿ� �ִ� ������Ʈ���� ���� ������ �ִ��� Ȯ��
	for (CGameObject* FieldOfViewObject : FieldOfViewGameObjectInfos)
	{
		Vector2 FieldOfViewObjectDir = FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.Position - Object->_GameObjectInfo.ObjectPositionInfo.Position;
		Vector2 FieldOfViewRay = FieldOfViewObjectDir.Normalize();

		// ����ĳ���� �˻��Ҷ� ������ ���� x, y ��
		Vector2 RayUnitStepSize;
		RayUnitStepSize.X = sqrt(1 + (FieldOfViewRay.Y / FieldOfViewRay.X) * (FieldOfViewRay.Y / FieldOfViewRay.X));
		RayUnitStepSize.Y = sqrt(1 + (FieldOfViewRay.X / FieldOfViewRay.Y) * (FieldOfViewRay.X / FieldOfViewRay.Y));

		// �� ��ǥ ��ġ 
		Vector2Int MapCheck;
		MapCheck.X = Object->_GameObjectInfo.ObjectPositionInfo.Position.X;
		MapCheck.Y = Object->_GameObjectInfo.ObjectPositionInfo.Position.Y;

		// ���� ��ġ���� ���� ��ġ�� Ray ����
		Vector2 RayLength1D;

		// Ž�� ����
		Vector2Int Step;

		// Ž�� ���� ���ϰ� ���� ��ġ Ray ���� �� ���ϱ�
		if (FieldOfViewRay.X < 0)
		{
			Step.X = -1;
			RayLength1D.X = (Object->_GameObjectInfo.ObjectPositionInfo.Position.X - float(MapCheck.X)) * RayUnitStepSize.X;
		}
		else
		{
			Step.X = 1;
			RayLength1D.X = (float(MapCheck.X + 1) - Object->_GameObjectInfo.ObjectPositionInfo.Position.X) * RayUnitStepSize.X;
		}

		if (FieldOfViewRay.Y < 0)
		{
			Step.Y = -1;
			RayLength1D.Y = (Object->_GameObjectInfo.ObjectPositionInfo.Position.Y - float(MapCheck.Y)) * RayUnitStepSize.Y;
		}
		else
		{
			Step.Y = 1;
			RayLength1D.Y = (float(MapCheck.Y + 1) - Object->_GameObjectInfo.ObjectPositionInfo.Position.Y) * RayUnitStepSize.Y;
		}

		// �˻�
		bool WallFound = false;
		bool TargetFound = false;
		float MaxDistance = Object->_FieldOfViewDistance;
		float Distance = 0.0f;

		// ���� ã�ų� ��ǥ���� ã�ų� �ִ� ���� �Ÿ��� �����ϸ� ����
		while (!WallFound && !TargetFound && Distance < MaxDistance)
		{
			// ���� Ÿ�� ��ġ�� �ű�
			if (RayLength1D.X < RayLength1D.Y)
			{
				MapCheck.X += Step.X;
				Distance = RayLength1D.X;
				RayLength1D.X += RayUnitStepSize.X;
			}
			else
			{
				MapCheck.Y += Step.Y;
				Distance = RayLength1D.Y;
				RayLength1D.Y += RayUnitStepSize.Y;
			}

			// �ʿ��� ����� Ÿ�� ��ġ ����			
			if (MapCheck.X >= 0 && MapCheck.X < _Right && MapCheck.Y >= 0 && MapCheck.Y < _Down)
			{
				int X = MapCheck.X - _Left;
				int Y = _Down - MapCheck.Y;

				CGameObject* GameObject = _ObjectsInfos[Y][X];
				if (GameObject != nullptr)
				{
					if (GameObject->_GameObjectInfo.ObjectId == FieldOfViewObject->_GameObjectInfo.ObjectId)
					{
						TargetFound = true;
					}
					else
					{
						switch (GameObject->_GameObjectInfo.ObjectType)
						{
						case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
						case en_GameObjectType::OBJECT_WALL:
							WallFound = true;
							break;
						default:							
							break;
						}
					}
				}
			}
		}

		// ��ǥ���� ���� ã�� ���
		if (TargetFound == true)
		{
			if (FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD
				&& FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_READY)
			{
				FieldOfViewInfo.ObjectID = FieldOfViewObject->_GameObjectInfo.ObjectId;
				FieldOfViewInfo.ObjectType = FieldOfViewObject->_GameObjectInfo.ObjectType;

				FieldOfViewGameObjects.push_back(FieldOfViewInfo);
			}			
		}	
	}

	return FieldOfViewGameObjects;
}

vector<st_FieldOfViewInfo> CMap::GetFieldAroundPlayers(CGameObject* Object, bool ExceptMe)
{
	vector<st_FieldOfViewInfo> FieldOfViewGamePlayers;

	vector<CSector*> Sectors = GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, 1);

	st_FieldOfViewInfo FieldOfViewInfo;

	for (CSector* Sector : Sectors)
	{
		Sector->AcquireSectorLock();

		// �ֺ� ���� �÷��̾� ����
		for (CPlayer* Player : Sector->GetPlayers())
		{
			if (Player->_NetworkState == en_ObjectNetworkState::OBJECT_NETWORK_STATE_LIVE)
			{
				FieldOfViewInfo.ObjectID = Player->_GameObjectInfo.ObjectId;
				FieldOfViewInfo.SessionID = Player->_SessionId;
				FieldOfViewInfo.ObjectType = Player->_GameObjectInfo.ObjectType;

				float Distance = Vector2::Distance(Object->_GameObjectInfo.ObjectPositionInfo.Position, Player->_GameObjectInfo.ObjectPositionInfo.Position);
				// �Լ� ȣ���� ������Ʈ�� ������ �������� ���� ���� true�� ���� false�� ����
				if (ExceptMe == true && Distance <= Object->_FieldOfViewDistance)
				{
					if (Object->_GameObjectInfo.ObjectId != Player->_GameObjectInfo.ObjectId)
					{
						FieldOfViewGamePlayers.push_back(FieldOfViewInfo);
					}
				}
				else if (ExceptMe == false && Distance <= Object->_FieldOfViewDistance)
				{
					FieldOfViewGamePlayers.push_back(FieldOfViewInfo);
				}
			}
		}

		Sector->ReleaseSectorLock();
	}

	return FieldOfViewGamePlayers;
}

vector<CPlayer*> CMap::GetAroundPlayers(CGameObject* Object, bool ExceptMe)
{
	vector<CPlayer*> FieldOfViewPlayers;

	vector<CSector*> Sectors = GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, 1);

	for (CSector* Sector : Sectors)
	{
		// �ֺ� ���� �÷��̾� ����
		for (CPlayer* Player : Sector->GetPlayers())
		{
			int16 Distance = Vector2Int::Distance(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

			// �Լ� ȣ���� ������Ʈ�� ������ �������� ���� ���� true�� ���� false�� ����
			if (ExceptMe == true && Distance <= Object->_FieldOfViewDistance)
			{
				if (Object->_GameObjectInfo.ObjectId != Player->_GameObjectInfo.ObjectId)
				{
					FieldOfViewPlayers.push_back(Player);
				}
			}
			else if (ExceptMe == false && Distance <= Object->_FieldOfViewDistance)
			{
				FieldOfViewPlayers.push_back(Player);
			}
		}
	}

	return FieldOfViewPlayers;
}

vector<CMonster*> CMap::GetAroundMonster(CGameObject* Object, int16 Range, bool ExceptMe)
{
	vector<CMonster*> Monsters;

	vector<CSector*> Sectors = GetAroundSectors(Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Range);

	for (CSector* Sector : Sectors)
	{
		Sector->AcquireSectorLock();

		for (CMonster* Monster : Sector->GetMonsters())
		{
			Monsters.push_back(Monster);
		}

		Sector->ReleaseSectorLock();
	}

	return Monsters;
}

CGameObject* CMap::MonsterReqFindNearPlayer(CMonster* Monster, en_MonsterAggroType* AggroType, bool* CollisionCango)
{
	// ���� �þ� ���� �ȿ� �ִ� �÷��̾���� �޾ƿ´�.
	vector<CPlayer*> Players = GetAroundPlayers(Monster, 1);

	// �޾ƿ� �÷��̾� ������ ���� �Ÿ��� ���ؼ� �켱���� ť�� ��´�.
	CHeap<float, CPlayer*> Distances((int32)Players.size()); // ����� ������� 
	for (CPlayer* Player : Players)
	{
		if (Player->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			float Distance = Vector2::Distance(Player->_GameObjectInfo.ObjectPositionInfo.Position, Monster->_GameObjectInfo.ObjectPositionInfo.Position);
			if (Distance > Monster->_PlayerSearchDistance)
			{
				continue;
			}

			Distances.InsertHeap(Distance, Player);
		}
	}

	CPlayer* Player = nullptr;
	// �Ÿ� ����� �ֺ��� �����ؼ� �ش� �÷��̾�� �� �� �ִ��� Ȯ���ϰ�
	// �� �� �ִ� ����̸� �ش� �÷��̾ ��ȯ���ش�.
	if (Distances.GetUseSize() != 0)
	{
		Player = Distances.PopHeap();

		*AggroType = en_MonsterAggroType::MONSTER_AGGRO_FIRST_TARGET;

		vector<Vector2Int> FirstPaths = FindPath(Monster, Monster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
		// Ÿ���� ������ ������ ���� ���� ( ������ ������Ʈ��� ������ )
		if (FirstPaths.size() < 2)
		{
			if (Player != nullptr)
			{
				// ���� �ٸ� ����߿��� �� �� �ִ� ����� �ִ��� �߰��� �Ǵ�
				while (Distances.GetUseSize() != 0)
				{
					Player = Distances.PopHeap();
					vector<Vector2Int> SecondPaths = FindPath(Monster, Monster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition, Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
					if (SecondPaths.size() < 2)
					{
						continue;
					}

					*AggroType = en_MonsterAggroType::MONSTER_AGGRO_SECOND_TARGET;
					*CollisionCango = true;

					break;
				}

				*CollisionCango = false;
			}
		}
		else
		{
			*CollisionCango = true;
		}
	}
	else
	{
		*CollisionCango = false;
	}

	// �÷��̾ ã������ ���°��� SPAWN_IDLE�� ��쿡�� �����Ѵ�.
	if (Player != nullptr && Player->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::SPAWN_IDLE)
	{
		Player = nullptr;
	}

	return Player;
}

CGameObject* CMap::Find(Vector2Int& CellPosition)
{
	// �¿� ��ǥ �˻�
	if (CellPosition.X < _Left || CellPosition.X > _Right)
	{
		return nullptr;
	}

	// ���� ��ǥ �˻�
	if (CellPosition.Y < _Down || CellPosition.Y > _Up)
	{
		return nullptr;
	}

	int X = CellPosition.X - _Left;
	int Y = _Up - CellPosition.Y;

	return _ObjectsInfos[Y][X];
}

CGameObject* CMap::FindPlant(Vector2Int& PlantPosition)
{
	// �¿� ��ǥ �˻�
	if (PlantPosition.X < _Left || PlantPosition.X > _Right)
	{
		return nullptr;
	}

	// ���� ��ǥ �˻�
	if (PlantPosition.Y < _Down || PlantPosition.Y > _Up)
	{
		return nullptr;
	}

	int X = PlantPosition.X - _Left;
	int Y = _Up - PlantPosition.Y;

	return _SeedObjectInfos[Y][X];
}

CItem** CMap::FindItem(Vector2Int& ItemCellPosition)
{
	int32 X = ItemCellPosition.X - _Left;
	int32 Y = _Down - ItemCellPosition.Y;

	// �¿� ��ǥ �˻�
	if (ItemCellPosition.X < _Left || ItemCellPosition.X > _Right)
	{
		return nullptr;
	}

	// ���� ��ǥ �˻�
	if (ItemCellPosition.X < _Left || ItemCellPosition.X > _Right)
	{
		return nullptr;
	}

	int XCount = _Right - _Left + 1;
	int YCount = _Up - _Down + 1;

	return _Items[Y][X];
}

bool CMap::Cango(CGameObject* Object)
{
	Vector2Int CollisionPosition;
	CollisionPosition.X = (int32)Object->_GameObjectInfo.ObjectPositionInfo.Position.X;
	CollisionPosition.Y = (int32)Object->_GameObjectInfo.ObjectPositionInfo.Position.Y;

	Vector2 CheckPosition; 
	CheckPosition.X = Object->_GameObjectInfo.ObjectPositionInfo.Position.X;
	CheckPosition.Y = Object->_GameObjectInfo.ObjectPositionInfo.Position.Y;	
	
	Vector2 DirectionNormal = Object->_GameObjectInfo.ObjectPositionInfo.MoveDirection.Normalize();

	CheckPosition.Y += (DirectionNormal.Y * Object->_GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
	CheckPosition.X += (DirectionNormal.X * Object->_GameObjectInfo.ObjectStatInfo.Speed * 0.02f);	

	if (CheckPosition.X < _Left
		|| CheckPosition.X > _Right)
	{
		return false;
	}

	if (CheckPosition.Y < _Down
		|| CheckPosition.Y > _Up)
	{
		return false;
	}

	CRectCollision CheckRectCollision = *Object->GetRectCollision();
	CheckRectCollision._Position = CheckPosition;
	CheckRectCollision.NotSetPositionUpdate();

	bool NextPositionMoveCheck = MoveCollisionCango(Object, CollisionPosition, &CheckRectCollision);

	if (NextPositionMoveCheck == true)
	{
		// �̵��� ��ǥ ��ġ�� �ٸ� ������Ʈ ����� ���� ��� �������� �ʴ´�.
		// ��� ��ġ�� �ű�� ���� ĳ���� �˻縦 �� �� ����
		Vector2Int CollisionPosition;
		CollisionPosition.X = (int32)CheckPosition.X;
		CollisionPosition.Y = (int32)CheckPosition.Y;

		if (CollisionPosition.X != Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X
			|| CollisionPosition.Y != Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y)
		{
			int X = CollisionPosition.X - _Left;
			int Y = _Up - CollisionPosition.Y;

			if (_ObjectsInfos[Y][X] != nullptr)
			{
				return false;
			}
		}

		Object->_GameObjectInfo.ObjectPositionInfo.Position = CheckPosition;		

		if (Object->GetRectCollision() != nullptr)
		{
			Object->GetRectCollision()->Update();
		}		
	}		

	return NextPositionMoveCheck;
}

bool CMap::MoveCollisionCango(CGameObject* Object, Vector2Int& CellPosition, CRectCollision* CheckRectCollsion, bool CheckObjects, CGameObject* CollisionObject)
{
	// ä���� �Ҵ�Ǿ� �ִ��� Ȯ��
	if (Object->GetChannel() == nullptr)
	{
		CRASH("ApplyMove GameObject Channel nullptr")
			return false;
	}

	// ���ӿ�����Ʈ�� ���� ä���� ��� �ִ� �ʰ� ���� ���� ������ Ȯ��
	if (Object->GetChannel()->GetMap() != this)
	{
		CRASH("ApplyMove GameObject�� ä���� ������ �ִ� �ʰ� ���� ���� �ٸ�")
			return false;
	}	

	bool ObjectCheck = Object->GetChannel()->ChannelColliderSingleCheck(Object, CheckRectCollsion, &CollisionObject);

	return (!CheckObjects || ObjectCheck);
}

bool CMap::ApplyMove(CGameObject* GameObject, Vector2Int& DestPosition, bool CheckObject, bool Applycollision)
{
	// ä���� �Ҵ�Ǿ� �ִ��� Ȯ��
	if (GameObject->GetChannel() == nullptr)
	{
		CRASH("ApplyMove GameObject Channel nullptr")
			return false;
	}

	// ���ӿ�����Ʈ�� ���� ä���� ��� �ִ� �ʰ� ���� ���� ������ Ȯ��
	if (GameObject->GetChannel()->GetMap() != this)
	{
		CRASH("ApplyMove GameObject�� ä���� ������ �ִ� �ʰ� ���� ���� �ٸ�")
			return false;
	}
	// ��ġ ���� ������ �´�.
	st_PositionInfo PositionInfo = GameObject->_GameObjectInfo.ObjectPositionInfo;

	switch (GameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:	
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	case en_GameObjectType::OBJECT_NON_PLAYER_GENERAL_MERCHANT:
	case en_GameObjectType::OBJECT_GOBLIN:	
	case en_GameObjectType::OBJECT_WALL:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		{
			// �������� �� �� �ִ��� �˻��Ѵ�.
			//if (MoveCollisionCango(GameObject, DestPosition, CheckObject) == false)
			//{
			//	//G_Logger->WriteStdOut(en_Color::RED, L"Cant Go ApplyMove Y (%d) X (%d) ",DestPosition._Y,DestPosition._X);
			//	return false;
			//}

			// ȣ������ ����� �浹ü�� ����ٸ�
			if (Applycollision == true)
			{
				int X = PositionInfo.CollisionPosition.X - _Left;
				int Y = _Up - PositionInfo.CollisionPosition.Y;

				// ������ġ �����ʹ� ������
				if (_ObjectsInfos[Y][X] == GameObject)
				{
					_ObjectsInfos[Y][X] = nullptr;
				}

				// ������ ��ġ �����ְ�
				X = DestPosition.X - _Left;
				Y = _Up - DestPosition.Y;

				// �������� �־��ش�.
				_ObjectsInfos[Y][X] = GameObject;

				int16 Width = GameObject->_GameObjectInfo.ObjectWidth;
				int16 Height = GameObject->_GameObjectInfo.ObjectHeight;

				for (int16 WidthX = 0; WidthX < Width; WidthX++)
				{
					for (int16 HeightY = 0; HeightY < Height; HeightY++)
					{
						_ObjectsInfos[Y + HeightY][X + WidthX] = GameObject;
					}
				}
			}
		}
		break;				
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_CORN:
		{
			CItem* ItemObject = (CItem*)GameObject;

			// �̵� �ϱ��� �־��� ��ǥ ���
			int32 PreviousX = ItemObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X - _Left;
			int32 PreviousY = _Up - ItemObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y;

			// �̵��ϰ��� �ϴ� ��ǥ ���
			int32 X = DestPosition.X - _Left;
			int32 Y = _Up - DestPosition.Y;

			// ���� ��ġ���� �����̴� ����� �����ϰ�
			for (int8 i = 0; i < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; i++)
			{
				if (_Items[PreviousY][PreviousX][i] == ItemObject)
				{
					_Items[PreviousY][PreviousX][i] = nullptr;
				}
			}

			// �켱 �ش� ��ġ�� �����۵�� ���� ���� �������� ������ ���Ѵ�.
			// ���� ���� �������� ������ �̹� �ش� ��ġ�� ���� ��쿡
			// ī��Ʈ�� ������Ű�� ���� ���� �������� �ʿ� ������Ű�� �ʴ´�.
			for (int8 i = 0; i < (int8)en_MapItemInfo::MAP_ITEM_COUNT_MAX; i++)
			{
				if (_Items[Y][X][i] != nullptr && _Items[Y][X][i]->_ItemInfo.ItemSmallCategory == ItemObject->_ItemInfo.ItemSmallCategory)
				{
					_Items[Y][X][i]->_ItemInfo.ItemCount += ItemObject->_ItemInfo.ItemCount;

					CItem* FindItem = (CItem*)(ItemObject->GetChannel()->FindChannelObject(_Items[Y][X][i]->_ItemInfo.ItemDBId, en_GameObjectType::OBJECT_ITEM));
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
		}
		break;
	case en_GameObjectType::OBJECT_CROP_POTATO:
	case en_GameObjectType::OBJECT_CROP_CORN:
		{
			CCrop* CropObject = (CCrop*)GameObject;

			int32 X = DestPosition.X - _Left;
			int32 Y = _Up - DestPosition.Y;

			_SeedObjectInfos[Y][X] = GameObject;
		}
		break;
	}

	CSector* CurrentSector = GetSector(GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
	CSector* NextSector = GetSector(DestPosition);
	if (CurrentSector != NextSector)
	{
		CurrentSector->Remove(GameObject);
		NextSector->Insert(GameObject);
	}

	GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X = DestPosition.X;
	GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y = DestPosition.Y;

	return true;
}

bool CMap::ApplyLeave(CGameObject* GameObject)
{
	if (GameObject->GetChannel() == nullptr)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyLeave Channel is nullptr");
		return false;
	}

	if (GameObject->GetChannel()->GetMap() != this)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyLeave Channel _Map Error");
		return false;
	}

	// �¿� ��ǥ �˻�
	if (GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X < _Left || GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X > _Right)
	{
		return false;
	}

	// ���� ��ǥ �˻�
	if (GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y < _Down || GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y > _Up)
	{
		return false;
	}

	// ���Ϳ��� ������Ʈ ����
	CSector* Sector = GetSector(GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
	Sector->Remove(GameObject);

	int X = GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X - _Left;
	int Y = _Up - GameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y;

	int Width = GameObject->_GameObjectInfo.ObjectWidth;
	int Height = GameObject->_GameObjectInfo.ObjectHeight;

	switch (GameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:	
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	case en_GameObjectType::OBJECT_GOBLIN:	
	case en_GameObjectType::OBJECT_WALL:
		{
			// �ʿ��� ����
			if (_ObjectsInfos[Y][X] == GameObject)
			{
				for (int16 WidthX = 0; WidthX < Width; WidthX++)
				{
					for (int16 HeightY = 0; HeightY < Height; HeightY++)
					{
						_ObjectsInfos[Y][X] = nullptr;
					}
				}
			}
			else
			{
				if (GameObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
				{
					//CRASH("ApplyLeave �����Ϸ��� ������Ʈ�� ����Ǿ� �ִ� ������Ʈ�� �ٸ�");
				}
			}
		}
		break;			
	case en_GameObjectType::OBJECT_CROP_CORN:
	case en_GameObjectType::OBJECT_CROP_POTATO:
		{
			if (_SeedObjectInfos[Y][X] == GameObject)
			{
				for (int16 WidthX = 0; WidthX < Width; WidthX++)
				{
					for (int16 HeightY = 0; HeightY < Height; HeightY++)
					{
						_SeedObjectInfos[Y][X] = nullptr;
					}
				}
			}
		}
		break;	
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_CORN:
		{
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
		}
		break;
	}

	return true;
}

bool CMap::CanMoveSkillGo(CGameObject* SkillObject, OUT Vector2* NextPosition, int64 ExceptionID, OUT CGameObject** CollisionObject)
{
	Vector2Int CollisionPosition;
	CollisionPosition.X = (int32)SkillObject->_GameObjectInfo.ObjectPositionInfo.Position.X;
	CollisionPosition.Y = (int32)SkillObject->_GameObjectInfo.ObjectPositionInfo.Position.Y;

	Vector2 CheckPosition;
	CheckPosition.X = SkillObject->_GameObjectInfo.ObjectPositionInfo.Position.X;
	CheckPosition.Y = SkillObject->_GameObjectInfo.ObjectPositionInfo.Position.Y;

	Vector2 DirectionNormal = SkillObject->_GameObjectInfo.ObjectPositionInfo.MoveDirection.Normalize();

	CheckPosition.Y += (DirectionNormal.Y * SkillObject->_GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
	CheckPosition.X += (DirectionNormal.X * SkillObject->_GameObjectInfo.ObjectStatInfo.Speed * 0.02f);

	if (CheckPosition.X < _Left
		|| CheckPosition.X > _Right)
	{
		return false;
	}

	if (CheckPosition.Y < _Down
		|| CheckPosition.Y > _Up)
	{
		return false;
	}

	bool NextPositionMoveCheck = ApplySkillObjectMove(SkillObject, CollisionPosition, ExceptionID, CollisionObject);
	NextPosition->X = CheckPosition.X;
	NextPosition->Y = CheckPosition.Y;

	return NextPositionMoveCheck;
}

bool CMap::ApplySkillObjectMove(CGameObject* SkillObject, Vector2Int& DestPosition, int64 ExceptionID, CGameObject** CollisionObject)
{
	if (SkillObject->GetChannel() == nullptr)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyLeave Channel is nullptr");
		return false;
	}

	if (SkillObject->GetChannel()->GetMap() != this)
	{
		G_Logger->WriteStdOut(en_Color::RED, L"ApplyLeave Channel _Map Error");
		return false;
	}	

	CSector* CurrentSector = GetSector(SkillObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
	CSector* NextSector = GetSector(DestPosition);
	if (CurrentSector != NextSector)
	{
		CurrentSector->Remove(SkillObject);
		NextSector->Insert(SkillObject);
	}

	SkillObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X = DestPosition.X;
	SkillObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y = DestPosition.Y;
	
	CGameObject* ColObject = nullptr;

	bool IsCollision = SkillObject->GetChannel()->ChannelColliderOBBCheck(SkillObject, ExceptionID, &ColObject);
	
	*CollisionObject = ColObject;

	return IsCollision;
}

bool CMap::MonsterCango(CGameObject* Object)
{
	Vector2 CheckPosition;
	CheckPosition.X = Object->_GameObjectInfo.ObjectPositionInfo.Position.X;
	CheckPosition.Y = Object->_GameObjectInfo.ObjectPositionInfo.Position.Y;

	Vector2 DirectionNormal = Object->_GameObjectInfo.ObjectPositionInfo.MoveDirection.Normalize();

	CheckPosition.Y += (DirectionNormal.Y * Object->_GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
	CheckPosition.X += (DirectionNormal.X * Object->_GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
	
	if (CheckPosition.X < _Left
		|| CheckPosition.X > _Right)
	{
		return false;
	}

	if (CheckPosition.Y < _Down
		|| CheckPosition.Y > _Up)
	{
		return false;
	}

	Vector2Int CheckCollisionPosition;
	CheckCollisionPosition.X = (int32)CheckPosition.X;
	CheckCollisionPosition.Y = (int32)CheckPosition.Y;
		
	bool NextPositionMoveCheck = FindPathNextPositionCango(Object, CheckCollisionPosition);
	if (NextPositionMoveCheck == true)
	{
		Object->_GameObjectInfo.ObjectPositionInfo.Position = CheckPosition;

		if (Object->GetRectCollision() != nullptr)
		{
			Object->GetRectCollision()->Update();
		}
	}

	return NextPositionMoveCheck;
}

vector<Vector2Int> CMap::FindPath(CGameObject* Object, Vector2Int StartCellPosition, Vector2Int DestCellPostion, bool CheckObjects, int32 MaxDistance)
{
	// �� �� �� �� 
	int32 DeltaY[4] = { 1, -1, 0, 0};
	int32 DeltaX[4] = { 0, 0, -1, 1};
	int32 Cost[4] = { 10,10,10,10};

	// ������ ������ ��ǥ��ȯ
	Vector2Int StartPosition = StartCellPosition;
	Vector2Int DestPosition = DestCellPostion;

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
	map<Vector2Int, Vector2Int> Parents;

	// �켱���� ť ����
	CHeap<int32, st_AStarNodeInt> OpenListQue(_SizeY * _SizeX);

	// ������忡 ó�� F �� ���
	OpenList[StartPosition.Y][StartPosition.X] = abs(DestPosition.Y - StartPosition.Y) + abs(DestPosition.X - StartPosition.X);

	// AStar Node ����
	st_AStarNodeInt StartNode(abs(DestPosition.Y - StartPosition.Y) + abs(DestPosition.X - StartPosition.X), 0, StartPosition.X, StartPosition.Y);
	// ť�� ����
	OpenListQue.InsertHeap(StartNode._F, StartNode);	

	// ó�� ��ġ ù �θ�� ����
	Parents.insert(pair<Vector2Int, Vector2Int>(StartPosition, StartPosition));

	// ���¸���Ʈ ť�� ����������� �ݺ�
	while (OpenListQue.GetUseSize() > 0)
	{
		// ���¸���Ʈ ť���� ��� �ϳ� ����
		st_AStarNodeInt AStarNode = OpenListQue.PopHeap();
		// �ش� ��ġ�� �湮�߾����� Ȯ��
		if (CloseList[AStarNode._Position.Y][AStarNode._Position.X] == true)
		{
			continue;
		}

		// �ش� ��ġ�� �湮�ߴٰ� ���
		CloseList[AStarNode._Position.Y][AStarNode._Position.X] = true;

		// ���� ����� ��ġ�� ��������� ������
		if (AStarNode._Position.Y == DestPosition.Y && AStarNode._Position.X == DestPosition.X)
		{
			break;
		}

		// ���� ��带 �������� �ؼ� �� �� �� �� ��带 �˻��ϰ� ���¸���Ʈ ť�� �ִ´�.
		for (int32 i = 0; i < 4; i++)
		{
			// ���� ��ġ�� �˾Ƴ���.
			Vector2Int NextPosition;
			NextPosition.Y = AStarNode._Position.Y + DeltaY[i];
			NextPosition.X = AStarNode._Position.X + DeltaX[i];

			// �������� �̾Ƴ� ��ġ�� ���������� �������� ������ �ʹ� �ִٸ� �ش� ��ǥ�� �����Ѵ�.
			if (abs(StartPosition.Y - NextPosition.Y) + abs(StartPosition.X - NextPosition.X) > MaxDistance)
			{
				continue;
			}

			// ���� ��ġ�� ������ ��ǥ�� �ƴ� ���
			if (NextPosition.Y != DestPosition.Y || NextPosition.X != DestPosition.X)
			{
				Vector2Int NextPositionVector = NextPosition;
				
				// �ش� ��ġ�� �� �� �ִ��� �˻��Ѵ�.
				if (FindPathNextPositionCango(Object, NextPositionVector, CheckObjects) == false	)
				{
					continue;
				}
			}

			// �̵��ϰ��� �ϴ� ���� ��ġ�� �̹� �湮 �ߴ��� Ȯ���Ѵ�.
			if (CloseList[NextPosition.Y][NextPosition.X] == true)
			{
				continue;
			}

			// G ���� ����Ѵ�.
			int G = AStarNode._G + Cost[i];
			// ���� ��ġ�� ������ ��ġ�� ����Ѵ�.
			int H = abs(DestPosition.Y - NextPosition.Y) + abs(DestPosition.X - NextPosition.X);

			// ���� ����� F ���� ���� �־��� F ������ ���� ��� ���� ����Ѵ�.
			if (G + H > OpenList[NextPosition.Y][NextPosition.X])
			{
				continue;
			}

			// ������ ��带 �غ��ϰ�
			st_AStarNodeInt InsertAStartNode;
			// �ռ� ���� ������ �̿��� F���� ������ �����Ѵ�.
			OpenList[NextPosition.Y][NextPosition.X] = G + H;

			// ������ ��带 �ʱ�ȭ �� �Ŀ�
			InsertAStartNode._F = G + H;
			InsertAStartNode._G = G;
			InsertAStartNode._Position.X = NextPosition.X;
			InsertAStartNode._Position.Y = NextPosition.Y;

			// �켱���� ť�� �ִ´�.
			OpenListQue.InsertHeap(InsertAStartNode._F, InsertAStartNode);
			// �θ� ��Ͽ� ���� ��ġ�� �ִ��� Ȯ��
			auto ChildFind = Parents.find(NextPosition);
			if (ChildFind == Parents.end())
			{
				//ã�� �������� �־��ְ�
				Parents.insert(pair<Vector2Int, Vector2Int>(NextPosition, AStarNode._Position));
			}
			else
			{
				//�̹� �־����� �θ� ��带 �ٲ��ش�.
				(*ChildFind).second = AStarNode._Position;
			}
		}
	}

	// CloseList, OpenList ����
	for (int32 i = 0; i < _SizeY; i++)
	{
		free(CloseList[i]);
		free(OpenList[i]);
	}

	free(CloseList);
	free(OpenList);

	vector<Vector2Int> CompletePathCells;

	// �ϼ��� �� ��� �߿��� ��ǥ ������ �ִ��� Ȯ��
	if(Parents.find(DestPosition) == Parents.end())
	{
		// �ϼ� �� �� �� ��ǥ ������ ���ٸ�
		// 
		Vector2Int BestPosition;
		int32 BestDistance; memset(&BestDistance, 1, sizeof(int32));

		// ������ �ִ� ��ġ �߿��� ���� ����� ��ġ�� ã�´�.		
		for (auto Start = Parents.begin(); Start != Parents.end(); ++Start)
		{
			int32 Distance = Vector2Int::Distance((*Start).first, DestPosition);
			if (BestDistance > Distance)
			{
				BestPosition = (*Start).first;
				BestDistance = Distance;
			}
		}

		// �ش� ��ġ�� �������� ��´�.
		DestPosition = BestPosition;

		// ���ο� �������� �������� �ٽ� ���� ã�´�.
		return FindPath(Object, StartCellPosition, DestPosition);
	}	
	else
	{
		Vector2Int Position = DestPosition;
		while ((*Parents.find(Position)).second != Position)
		{
			CompletePathCells.push_back(Position);
			Position = (*Parents.find(Position)).second;
		}

		// ������ ���
		CompletePathCells.push_back(Position);
		// �θ� ��ġ ���� �迭 �Ųٷ� ����� ��ȯ
		reverse(CompletePathCells.begin(), CompletePathCells.end());

		return CompletePathCells;
	}	
}

bool CMap::FindPathNextPositionCango(CGameObject* Object, Vector2Int& NextPosition, bool CheckObjects)
{
	// �¿� ��ǥ �˻�
	if (NextPosition.X < _Left || NextPosition.X > _Right)
	{
		return false;
	}

	// ���� ��ǥ �˻�
	if (NextPosition.Y < _Down || NextPosition.Y > _Up)
	{
		return false;
	}

	int X = NextPosition.X - _Left;
	int Y = _Up - NextPosition.Y;

	//G_Logger->WriteStdOut(en_Color::RED, L"X : %d Y : %d \n", X, Y);

	bool IsCollisionMapInfo = true;

	if (_ObjectsInfos[Y][X] != nullptr)
	{
		switch (_ObjectsInfos[Y][X]->_GameObjectInfo.ObjectType)
		{
		case en_GameObjectType::OBJECT_WALL:
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
			IsCollisionMapInfo = false;
			break;
		}
	}

	bool ObjectCheck = false;

	switch (Object->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_GOBLIN:	
		// ������Ʈ ��ġ �迭�� ��� ���� ��� true ��ȯ
		if (_ObjectsInfos[Y][X] == nullptr)
		{			
			ObjectCheck = true;			
		}
		else
		{
			// ��� ������ ������ �ȿ� �ִ� ������Ʈ�� �̵��ϰ��� �ϴ� ������Ʈ�� ���
			if (_ObjectsInfos[Y][X]->_GameObjectInfo.ObjectId == Object->_GameObjectInfo.ObjectId)
			{
				ObjectCheck = true;
			}
			else // �ȿ� �ִ� ������Ʈ�� �ٸ� ������Ʈ�� ���
			{
				if (_ObjectsInfos[Y][X]->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::DEAD)
				{
					ObjectCheck = true;
				}
				else
				{
					ObjectCheck = false;
				}
			}			
		}
		break;
	}

	return IsCollisionMapInfo && (!CheckObjects || ObjectCheck);
}

CChannelManager* CMap::GetChannelManager()
{
	return _ChannelManager;
}

Vector2 CMap::GetMovePositionNearTarget(CGameObject* Object, CGameObject* Target)
{
	Vector2Int TargetDir = Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition - Object->_GameObjectInfo.ObjectPositionInfo.CollisionPosition;
	Vector2Int NearDir = TargetDir.Direction();
	NearDir *= -1;

	Vector2Int NearCollisionPosition;
	NearCollisionPosition.X = Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X + NearDir.X;
	NearCollisionPosition.Y = Target->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y + NearDir.Y;

	return Vector2(NearCollisionPosition.X + 0.5f, NearCollisionPosition.Y + 0.5f);
}

st_TileInfo CMap::GetTileInfo(Vector2Int Position)
{
	return _TileInfos[Position.Y][Position.X];
}

vector<st_TileInfo> CMap::GetTileInfos(Vector2Int CenterPosition, int16 RangeX, int16 RangeY)
{
	vector<st_TileInfo> AroundTileInfos;

	Vector2Int LeftTopPosition;
	LeftTopPosition.X = CenterPosition.X - RangeX / 2;
	LeftTopPosition.Y = CenterPosition.Y + RangeY / 2;

	if (LeftTopPosition.X < 0)
	{
		LeftTopPosition.X = 0;
	}

	if (LeftTopPosition.Y > _Up)
	{
		LeftTopPosition.Y = _Up;
	}

	for (int16 HeightY = LeftTopPosition.Y; HeightY > LeftTopPosition.Y - RangeY; HeightY--)
	{
		for (int16 WidthX = LeftTopPosition.X; WidthX < LeftTopPosition.X + RangeX; WidthX++)
		{		
			AroundTileInfos.push_back(_TileInfos[HeightY][WidthX]);
		}
	}

	return AroundTileInfos;
}

void CMap::SetTileInfos(vector<st_TileInfo> TileInfos)
{
	for (st_TileInfo const TileInfo : TileInfos)
	{
		_TileInfos[TileInfo.Position.Y].push_back(TileInfo);			
	}
}

bool CMap::BuildingInstall(vector<st_TileInfo> BuildingTileInfos)
{
	bool IsBuildingSuccess = false;	

	for (st_TileInfo BuildingTileInfo : BuildingTileInfos)
	{
		vector<st_TileInfo> MapTileInfos = _TileInfos[BuildingTileInfo.Position.Y];

		for (st_TileInfo MapTileInfo : MapTileInfos)
		{
			if (BuildingTileInfo.Position == MapTileInfo.Position)
			{
				IsBuildingSuccess &= BuildingTileInfo.IsOccupation;
				break;
			}
		}
	}

	// �Ű������� ���� Ÿ�� ��ġ�� ��� �˻��� ��ġ�� �����ϸ� ���� ����
	if (IsBuildingSuccess == false)
	{
		for (st_TileInfo BuildingTileInfo : BuildingTileInfos)
		{
			vector<st_TileInfo> MapTileInfos = _TileInfos[BuildingTileInfo.Position.Y];

			for (st_TileInfo MapTileInfo : MapTileInfos)
			{
				if (BuildingTileInfo.Position == MapTileInfo.Position)
				{
					_TileInfos[BuildingTileInfo.Position.Y][BuildingTileInfo.Position.X].IsOccupation = BuildingTileInfo.IsOccupation;
					_TileInfos[BuildingTileInfo.Position.Y][BuildingTileInfo.Position.X].OwnerObjectID = BuildingTileInfo.OwnerObjectID;					
					break;
				}
			}
		}
	}

	return IsBuildingSuccess;
}

void CMap::SetTileInfo(int8 XRange, int8 YRange, bool IsOccupdation, int64 OwnerObjectID)
{
	int32 TilePositionX;
	int32 TilePositionY;

	Vector2Int TileStartPosition;

	while (1)
	{
		TilePositionX = Math::RandomNumberInt(0, 90);
		TilePositionY = Math::RandomNumberInt(0, 74);
		
		TileStartPosition.Y = TilePositionY + 10;
		TileStartPosition.X = TilePositionX - 10;

		if (TileStartPosition.X > _Left && TileStartPosition.X + XRange < _Right
			&& TileStartPosition.Y  > _Down && TileStartPosition.Y + YRange < _Up)
		{
			break;
		}
	}	

	for (int Y = TileStartPosition.Y; Y < TileStartPosition.Y + YRange; Y++)
	{
		for (int X = TileStartPosition.X; X < TileStartPosition.X + XRange; X++)
		{
			_TileInfos[Y][X].IsOccupation = IsOccupdation;
			_TileInfos[Y][X].OwnerObjectID = OwnerObjectID;
		}
	}
}

void CMap::SetTileInfo(bool IsOccupdation, int64 OwnerObjectID, Vector2Int Position)
{
	_TileInfos[Position.X][Position.Y].IsOccupation = IsOccupdation;
	_TileInfos[Position.X][Position.Y].OwnerObjectID = OwnerObjectID;
}
