#include "pch.h"
#include "Channel.h"
#include "Player.h"
#include "Message.h"
#include "Monster.h"
#include "Item.h"
#include "Heap.h"
#include "Environment.h"
#include "Map.h"
#include "ObjectManager.h"

CChannel::CChannel()
{
	InitializeSRWLock(&_ChannelLock);

	for (int32 PlayerCount = PLAYER_MAX - 1; PlayerCount >= 0; --PlayerCount)
	{
		_ChannelPlayerArray[PlayerCount] = nullptr;
		_ChannelPlayerArrayIndexs.Push(PlayerCount);
	}

	for (int32 MonsterCount = MONSTER_MAX - 1; MonsterCount >= 0; --MonsterCount)
	{
		_ChannelMonsterArray[MonsterCount] = nullptr;
		_ChannelMonsterArrayIndexs.Push(MonsterCount);
	}

	for (int32 ItemCount = ITEM_MAX - 1; ItemCount >= 0; --ItemCount)
	{
		_ChannelItemArray[ItemCount] = nullptr;
		_ChannelItemArrayIndexs.Push(ItemCount);
	}

	for (int32 Environment = ENVIRONMENT_MAX - 1; Environment >= 0; --Environment)
	{
		_ChannelEnvironmentArray[Environment] = nullptr;
		_ChannelEnvironmentArrayIndexs.Push(Environment);
	}
}

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

void CChannel::Init()
{
	
}

void CChannel::Update()
{
	while (!_ChannelJobQue.IsEmpty())
	{
		st_GameObjectJob* GameObjectJob = nullptr;

		if (!_ChannelJobQue.Dequeue(&GameObjectJob))
		{
			break;
		}

		switch ((en_GameObjectJobType)GameObjectJob->GameObjectJobType)
		{
		case en_GameObjectJobType::GAMEOBJECT_JOB_ENTER_CHANNEL:
			{
				CPlayer* EnterPlayer;
				*GameObjectJob->GameObjectJobMessage >> &EnterPlayer;

				G_ObjectManager->ObjectEnterGame(EnterPlayer, 1);

				// ������ �÷��̾�� ���� ����
				CMessage* ResEnterGamePacket = G_ObjectManager->GameServer->MakePacketResEnterGame(true, &EnterPlayer->_GameObjectInfo);
				G_ObjectManager->GameServer->SendPacket(EnterPlayer->_SessionId, ResEnterGamePacket);
				ResEnterGamePacket->Free();
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_LEAVE_CHANNEL:
			{	
				CPlayer* LeavePlayer;
				*GameObjectJob->GameObjectJobMessage >> &LeavePlayer;
				
				LeaveChannel(LeavePlayer);

				vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Map->GetFieldOfViewPlayers(LeavePlayer, 1);
				
				CMessage* ResObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(LeavePlayer->_GameObjectInfo.ObjectId);
				G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResObjectDeSpawnPacket);
				ResObjectDeSpawnPacket->Free();

				LeavePlayer->_NetworkState = en_ObjectNetworkState::LEAVE;

				LeavePlayer->Init();

				G_ObjectManager->PlayerIndexReturn(LeavePlayer->_ObjectManagerArrayIndex);
			}
			break;		
		}

		if (GameObjectJob->GameObjectJobMessage != nullptr)
		{
			GameObjectJob->GameObjectJobMessage->Free();
		}

		G_ObjectManager->GameObjectJobReturn(GameObjectJob);
	}

	for (int16 i = 0; i < PLAYER_MAX; i++)
	{
		if (_ChannelPlayerArray[i]
			&& _ChannelPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE
			&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			_ChannelPlayerArray[i]->Update();
		}
	}

	for (int16 i = 0; i < MONSTER_MAX; i++)
	{
		if (_ChannelMonsterArray[i]
			&& _ChannelMonsterArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			_ChannelMonsterArray[i]->Update();
		}
	}

	for (int16 i = 0; i < ITEM_MAX; i++)
	{
		if (_ChannelItemArray[i]
			&& _ChannelItemArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			_ChannelItemArray[i]->Update();
		}
	}

	for (int16 i = 0; i < ENVIRONMENT_MAX; i++)
	{
		if (_ChannelEnvironmentArray[i]
			&& _ChannelEnvironmentArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			_ChannelEnvironmentArray[i]->Update();
		}
	}
}

CMap* CChannel::GetMap()
{
	return _Map;
}

void CChannel::SetMap(CMap* Map)
{
	_Map = Map;
}

bool CChannel::EnterChannel(CGameObject* EnterChannelGameObject, st_Vector2Int* ObjectSpawnPosition)
{
	bool IsEnterChannel = false;
	// ä�� ����
	if (EnterChannelGameObject == nullptr)
	{
		CRASH("GameObject�� nullptr");
		return false;
	}

	random_device RD;
	mt19937 Gen(RD());

	st_Vector2Int SpawnPosition;

	if (ObjectSpawnPosition != nullptr)
	{
		SpawnPosition = *ObjectSpawnPosition;
	}
	else
	{
		// ���̸� ������� ���� ��ǥ �޾Ƽ� ä�ο� ����
		while (true)
		{
			uniform_int_distribution<int> RandomXPosition(0, 89);
			uniform_int_distribution<int> RandomYPosition(0, 73);

			SpawnPosition._X = RandomXPosition(Gen);
			SpawnPosition._Y = RandomYPosition(Gen);

			if (_Map->CollisionCango(EnterChannelGameObject, SpawnPosition) == true)
			{
				break;
			}
		}
	}
	// ������ ������Ʈ�� Ÿ�Կ� ����
	switch ((en_GameObjectType)EnterChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
	{
		// �÷��̾�� ����ȯ
		CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;
		EnterChannelPlayer->_SpawnPosition._Y = SpawnPosition._Y;
		EnterChannelPlayer->_SpawnPosition._X = SpawnPosition._X;
		EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = SpawnPosition._Y;
		EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = SpawnPosition._X;

		EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionX = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX + 0.5f;
		EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.PositionY = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY + 0.5f;

		// �÷��̾� ����
		_ChannelPlayerArrayIndexs.Pop(&EnterChannelPlayer->_ChannelArrayIndex);
		_ChannelPlayerArray[EnterChannelPlayer->_ChannelArrayIndex] = EnterChannelPlayer;

		//_Players.insert(pair<int64, CPlayer*>(EnterChannelPlayer->_GameObjectInfo.ObjectId, EnterChannelPlayer));			

		// ä�� ����
		EnterChannelPlayer->SetChannel(this);

		// �ʿ� ����
		IsEnterChannel = _Map->ApplyMove(EnterChannelPlayer, SpawnPosition);

		// ���Ϳ� ����
		CSector* EnterSector = _Map->GetSector(SpawnPosition);
		EnterSector->Insert(EnterChannelPlayer);
	}
	break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
	{
		// ���ͷ� ����ȯ
		CMonster* EnterChannelMonster = (CMonster*)EnterChannelGameObject;
		EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = SpawnPosition._Y;
		EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = SpawnPosition._X;

		EnterChannelMonster->Init(SpawnPosition);

		EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.PositionX = EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX + 0.5f;
		EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.PositionY = EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY + 0.5f;

		// ���� ����
		_ChannelMonsterArrayIndexs.Pop(&EnterChannelMonster->_ChannelArrayIndex);
		_ChannelMonsterArray[EnterChannelMonster->_ChannelArrayIndex] = EnterChannelMonster;

		// ä�� ����
		EnterChannelMonster->SetChannel(this);

		// �ʿ� ����
		IsEnterChannel = _Map->ApplyMove(EnterChannelMonster, SpawnPosition);

		// ���� �� �ش� ���Ϳ��� ����
		CSector* EnterSector = _Map->GetSector(SpawnPosition);
		EnterSector->Insert(EnterChannelMonster);
	}
	break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	{
		// ���������� ����ȯ
		CItem* EnterChannelItem = (CItem*)EnterChannelGameObject;
		EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = SpawnPosition._Y;
		EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = SpawnPosition._X;

		EnterChannelItem->SetChannel(this);

		// �� ������ ����			
		IsEnterChannel = _Map->ApplyPositionUpdateItem(EnterChannelItem, SpawnPosition);

		// �ߺ����� �ʴ� �������� ��쿡�� ä�ο� �ش� �������� ä�ΰ� ���Ϳ� ����
		if (IsEnterChannel == true)
		{
			// ���� ����
			_ChannelItemArrayIndexs.Pop(&EnterChannelItem->_ChannelArrayIndex);
			_ChannelItemArray[EnterChannelItem->_ChannelArrayIndex] = EnterChannelItem;

			// ���� �� �ش� ���Ϳ��� ����
			CSector* EnterSector = _Map->GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelItem);
		}
	}
	break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
	{
		CEnvironment* EnterChannelEnvironment = (CEnvironment*)EnterChannelGameObject;
		EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY = SpawnPosition._Y;
		EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX = SpawnPosition._X;

		if (EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY > 0)
		{
			EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.PositionY = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY;
		}
		else if (EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY == 0)
		{
			EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.PositionY = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY - 0.5f;
		}
		else
		{
			EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.PositionY = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionY - 1.0f;
		}

		if (EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX > 0)
		{
			EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.PositionX = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX + 0.5f;
		}
		else
		{
			EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.PositionX = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPositionX - 0.5f;
		}

		// ȯ�� ������Ʈ ����
		_ChannelEnvironmentArrayIndexs.Pop(&EnterChannelEnvironment->_ChannelArrayIndex);
		_ChannelEnvironmentArray[EnterChannelEnvironment->_ChannelArrayIndex] = EnterChannelEnvironment;

		EnterChannelEnvironment->SetChannel(this);

		IsEnterChannel = _Map->ApplyMove(EnterChannelEnvironment, SpawnPosition);

		// ���� �� �ش� ���Ϳ��� ����
		CSector* EnterSector = _Map->GetSector(SpawnPosition);
		EnterSector->Insert(EnterChannelEnvironment);
	}
	break;
	}

	return IsEnterChannel;
}

void CChannel::LeaveChannel(CGameObject* LeaveChannelGameObject)
{
	// ä�� ����
	// �����̳ʿ��� ������ �� �ʿ����� ����
	switch ((en_GameObjectType)LeaveChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		_ChannelPlayerArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);

		_Map->ApplyLeave(LeaveChannelGameObject);
		break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		_ChannelMonsterArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);

		_Map->ApplyLeave(LeaveChannelGameObject);
		break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
		_ChannelItemArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);

		_Map->ApplyPositionLeaveItem(LeaveChannelGameObject);
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		_ChannelEnvironmentArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);

		_Map->ApplyLeave(LeaveChannelGameObject);
		break;
	}
}