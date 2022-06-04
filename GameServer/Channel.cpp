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
	
	for (int32 DummyPlayerCount = DUMMY_PLAYER_MAX - 1; DummyPlayerCount >= 0; --DummyPlayerCount)
	{
		_ChannelDummyPlayerArray[DummyPlayerCount] = nullptr;
		_ChannelDummyPlayerArrayIndexs.Push(DummyPlayerCount);
	}

	for (int32 MonsterCount = MONSTER_MAX - 1; MonsterCount >= 0; --MonsterCount)
	{
		_ChannelMonsterArray[MonsterCount] = nullptr;
		_ChannelMonsterArrayIndexs.Push(MonsterCount);
	}

	for (int32 Environment = ENVIRONMENT_MAX - 1; Environment >= 0; --Environment)
	{
		_ChannelEnvironmentArray[Environment] = nullptr;
		_ChannelEnvironmentArrayIndexs.Push(Environment);
	}

	for (int32 ItemCount = ITEM_MAX - 1; ItemCount >= 0; --ItemCount)
	{
		_ChannelItemArray[ItemCount] = nullptr;
		_ChannelItemArrayIndexs.Push(ItemCount);
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

		if (GameObjectJob != nullptr)
		{
			switch ((en_GameObjectJobType)GameObjectJob->GameObjectJobType)
			{
			case en_GameObjectJobType::GAMEOBJECT_JOB_PLAYER_ENTER_CHANNEL:
				{				
					CPlayer* EnterPlayer;
					*GameObjectJob->GameObjectJobMessage >> &EnterPlayer;
					
					EnterPlayer->_SpawnIdleTick = GetTickCount64() + 5000;
					EnterPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;				

					EnterChannel(EnterPlayer, &EnterPlayer->_SpawnPosition);

					// ������ �� �����϶�� �˷���
					CMessage* ResEnterGamePacket = G_ObjectManager->GameServer->MakePacketResEnterGame(true, &EnterPlayer->_GameObjectInfo, &EnterPlayer->_SpawnPosition);
					G_ObjectManager->GameServer->SendPacket(EnterPlayer->_SessionId, ResEnterGamePacket);
					ResEnterGamePacket->Free();

					if (EnterPlayer->_GameObjectInfo.ObjectType != en_GameObjectType::OBJECT_PLAYER_DUMMY)
					{
						st_GameServerJob* DBCharacterInfoSendJob = G_ObjectManager->GameServer->_GameServerJobMemoryPool->Alloc();
						DBCharacterInfoSendJob->Type = en_GameServerJobType::DATA_BASE_CHARACTER_INFO_SEND;

						CGameServerMessage* ReqDBCharacterInfoMessage = CGameServerMessage::GameServerMessageAlloc();
						ReqDBCharacterInfoMessage->Clear();

						*ReqDBCharacterInfoMessage << EnterPlayer->_SessionId;

						DBCharacterInfoSendJob->Message = ReqDBCharacterInfoMessage;

						G_ObjectManager->GameServer->_GameServerUserDBThreadMessageQue.Enqueue(DBCharacterInfoSendJob);
						SetEvent(G_ObjectManager->GameServer->_UserDataBaseWakeEvent);
					}	
					else
					{
						EnterPlayer->_NetworkState = en_ObjectNetworkState::LIVE;
					}
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_OBJECT_ENTER_CHANNEL:
				{
					CGameObject* EnterObject;
					*GameObjectJob->GameObjectJobMessage >> &EnterObject;

					EnterObject->_SpawnIdleTick = GetTickCount64() + 5000;
					EnterObject->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;

					EnterChannel(EnterObject, &EnterObject->_SpawnPosition);

					CMessage* SpawnObjectPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn(EnterObject);
					G_ObjectManager->GameServer->SendPacketFieldOfView(EnterObject, SpawnObjectPacket);
					SpawnObjectPacket->Free();
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_LEAVE_CHANNEL:
				{
					CGameObject* LeaveGameObject;
					*GameObjectJob->GameObjectJobMessage >> &LeaveGameObject;

					LeaveChannel(LeaveGameObject);

					// �� �����ؼ� ���� �þ߹��� �÷��̾� ����
					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Map->GetFieldOfViewPlayers(LeaveGameObject, 1, false);

					CMessage* ResObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(LeaveGameObject->_GameObjectInfo.ObjectId);
					G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResObjectDeSpawnPacket);
					ResObjectDeSpawnPacket->Free();
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_PLAYER_LEAVE_CHANNEL:
				{
					CGameObject* LeaveGameObject;
					*GameObjectJob->GameObjectJobMessage >> &LeaveGameObject;

					LeaveChannel(LeaveGameObject);

					// �� �����ؼ� ���� �þ߹��� �÷��̾� ����
					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Map->GetFieldOfViewPlayers(LeaveGameObject, 1, false);

					CMessage* ResObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(LeaveGameObject->_GameObjectInfo.ObjectId);
					G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResObjectDeSpawnPacket);
					ResObjectDeSpawnPacket->Free();

					LeaveGameObject->_NetworkState = en_ObjectNetworkState::LEAVE;

					LeaveGameObject->End();

					for (int8 i = 0; i < SESSION_CHARACTER_MAX; i++)
					{
						int32 PlayerIndex;
						*GameObjectJob->GameObjectJobMessage >> PlayerIndex;

						G_ObjectManager->PlayerIndexReturn(PlayerIndex);
					}					
				}
				break;
			}

			if (GameObjectJob->GameObjectJobMessage != nullptr)
			{
				GameObjectJob->GameObjectJobMessage->Free();
			}

			G_ObjectManager->GameObjectJobReturn(GameObjectJob);
		}	
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

	for (int16 i = 0; i < DUMMY_PLAYER_MAX; i++)
	{
		if (_ChannelDummyPlayerArray[i]
			&& _ChannelDummyPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE
			&& _ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			_ChannelDummyPlayerArray[i]->Update();
		}
	}

	for (int16 i = 0; i < MONSTER_MAX; i++)
	{
		if (_ChannelMonsterArray[i] != nullptr)
		{
			_ChannelMonsterArray[i]->Update();
		}
	}

	for (int16 i = 0; i < ITEM_MAX; i++)
	{
		if (_ChannelItemArray[i] != nullptr)
		{
			_ChannelItemArray[i]->Update();
		}
	}

	for (int16 i = 0; i < ENVIRONMENT_MAX; i++)
	{
		if (_ChannelEnvironmentArray[i] != nullptr)
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

CGameObject* CChannel::FindChannelObject(int64 ObjectID, en_GameObjectType GameObjectType)
{
	CGameObject* FindObject = nullptr;	
		
	switch (GameObjectType)
	{	
	case en_GameObjectType::OBJECT_PLAYER:
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
		{
			for (int32 i = 0; i < en_Channel::PLAYER_MAX; i++)
			{
				if (_ChannelPlayerArray[i] != nullptr 
					&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectId == ObjectID
					&& _ChannelPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE)
				{
					FindObject = _ChannelPlayerArray[i];
				}				
			}
		}
		break;
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		{
			for (int32 i = 0; i < en_Channel::DUMMY_PLAYER_MAX; i++)
			{
				if (_ChannelDummyPlayerArray[i] != nullptr && _ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectId == ObjectID)
				{
					FindObject = _ChannelDummyPlayerArray[i];
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_MONSTER:
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		{
			for (int32 i = 0; i < en_Channel::MONSTER_MAX; i++)
			{
				if (_ChannelMonsterArray[i] != nullptr && _ChannelMonsterArray[i]->_GameObjectInfo.ObjectId == ObjectID)
				{
					FindObject = _ChannelMonsterArray[i];
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_ENVIRONMENT:
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		{
			for (int32 i = 0; i < en_Channel::ENVIRONMENT_MAX; i++)
			{
				if (_ChannelEnvironmentArray[i] != nullptr && _ChannelEnvironmentArray[i]->_GameObjectInfo.ObjectId == ObjectID)
				{
					FindObject = _ChannelEnvironmentArray[i];
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_ITEM:
	case en_GameObjectType::OBJECT_ITEM_WEAPON:
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
	case en_GameObjectType::OBJECT_ITEM_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_HEAL_POTION_SMALL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIVER_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_GOLD_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
		{
			for (int32 i = 0; i < en_Channel::ENVIRONMENT_MAX; i++)
			{
				if (_ChannelItemArray[i] != nullptr && _ChannelItemArray[i]->_GameObjectInfo.ObjectId == ObjectID)
				{
					FindObject = _ChannelItemArray[i];
				}
			}
		}
		break;		
	}

	return FindObject;
}

vector<CGameObject*> CChannel::FindChannelObjects(vector<st_FieldOfViewInfo>& FindObjectIDs)
{
	vector<CGameObject*> FindObjects;

	for (st_FieldOfViewInfo FieldOfViewInfo : FindObjectIDs)
	{
		switch (FieldOfViewInfo.ObjectType)
		{			
		case en_GameObjectType::OBJECT_PLAYER:
		case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
		case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
		case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
		case en_GameObjectType::OBJECT_THIEF_PLAYER:
		case en_GameObjectType::OBJECT_ARCHER_PLAYER:
		{
			for (int32 i = 0; i < en_Channel::PLAYER_MAX; i++)
			{
				if (_ChannelPlayerArray[i] != nullptr
					&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID
					&& _ChannelPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE)
				{
					FindObjects.push_back(_ChannelPlayerArray[i]);
				}
			}
		}
		break;
		case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		{
			for (int32 i = 0; i < en_Channel::DUMMY_PLAYER_MAX; i++)
			{
				if (_ChannelDummyPlayerArray[i] != nullptr && _ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
				{
					FindObjects.push_back(_ChannelDummyPlayerArray[i]);
				}
			}
		}
		break;
		case en_GameObjectType::OBJECT_MONSTER:
		case en_GameObjectType::OBJECT_SLIME:
		case en_GameObjectType::OBJECT_BEAR:
		{
			for (int32 i = 0; i < en_Channel::MONSTER_MAX; i++)
			{
				if (_ChannelMonsterArray[i] != nullptr && _ChannelMonsterArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
				{
					FindObjects.push_back(_ChannelMonsterArray[i]);
				}
			}
		}
		break;
		case en_GameObjectType::OBJECT_ENVIRONMENT:
		case en_GameObjectType::OBJECT_STONE:
		case en_GameObjectType::OBJECT_TREE:
		{
			for (int32 i = 0; i < en_Channel::ENVIRONMENT_MAX; i++)
			{
				if (_ChannelEnvironmentArray[i] != nullptr && _ChannelEnvironmentArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
				{
					FindObjects.push_back(_ChannelEnvironmentArray[i]);
				}
			}
		}
		break;
		case en_GameObjectType::OBJECT_ITEM:
		case en_GameObjectType::OBJECT_ITEM_WEAPON:
		case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
		case en_GameObjectType::OBJECT_ITEM_ARMOR:
		case en_GameObjectType::OBJECT_ITEM_ARMOR_WOOD_ARMOR:
		case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
		case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
		case en_GameObjectType::OBJECT_ITEM_CONSUMABLE:
		case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
		case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_HEAL_POTION_SMALL:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIVER_COIN:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_GOLD_COIN:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
		{
			for (int32 i = 0; i < en_Channel::ENVIRONMENT_MAX; i++)
			{
				if (_ChannelItemArray[i] != nullptr && _ChannelItemArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
				{
					FindObjects.push_back(_ChannelItemArray[i]);
				}
			}
		}
		break;
		}
	}	

	return FindObjects;
}

vector<CGameObject*> CChannel::FindChannelObjects(vector<st_FieldOfViewInfo>& FindObjectIDs, CGameObject* Object, int16 Distance)
{
	vector<CGameObject*> FindObjects;

	for (st_FieldOfViewInfo FieldOfViewInfo : FindObjectIDs)
	{
		switch (FieldOfViewInfo.ObjectType)
		{
		case en_GameObjectType::OBJECT_PLAYER:
		case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
		case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
		case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
		case en_GameObjectType::OBJECT_THIEF_PLAYER:
		case en_GameObjectType::OBJECT_ARCHER_PLAYER:
		{
			for (int32 i = 0; i < en_Channel::PLAYER_MAX; i++)
			{
				if (_ChannelPlayerArray[i] != nullptr
					&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID
					&& _ChannelPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE)
				{	
					if (st_Vector2::CheckFieldOfView(_ChannelPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position, Object->GetPositionInfo().MoveDir, 80))
					{
						// �þ߰� �ȿ� ������Ʈ�� ����
						float TargetDistance = st_Vector2::Distance(_ChannelPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
						if (TargetDistance <= Distance)
						{
							FindObjects.push_back(_ChannelPlayerArray[i]);
						}
					}				
				}
			}
		}
		break;
		case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		{
			for (int32 i = 0; i < en_Channel::DUMMY_PLAYER_MAX; i++)
			{
				if (_ChannelDummyPlayerArray[i] != nullptr && _ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
				{
					if (st_Vector2::CheckFieldOfView(_ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position, Object->GetPositionInfo().MoveDir, 80))
					{
						// �þ߰� �ȿ� ������Ʈ�� ����
						float TargetDistance = st_Vector2::Distance(_ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
						if (TargetDistance <= Distance)
						{
							FindObjects.push_back(_ChannelDummyPlayerArray[i]);
						}
					}					
				}
			}
		}
		break;
		case en_GameObjectType::OBJECT_MONSTER:
		case en_GameObjectType::OBJECT_SLIME:
		case en_GameObjectType::OBJECT_BEAR:
		{
			for (int32 i = 0; i < en_Channel::MONSTER_MAX; i++)
			{
				if (_ChannelMonsterArray[i] != nullptr && _ChannelMonsterArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
				{
					if (st_Vector2::CheckFieldOfView(_ChannelMonsterArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position, Object->GetPositionInfo().MoveDir, 80))
					{
						// �þ߰� �ȿ� ������Ʈ�� ����
						float TargetDistance = st_Vector2::Distance(_ChannelMonsterArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
						if (TargetDistance <= Distance)
						{
							FindObjects.push_back(_ChannelMonsterArray[i]);
						}
					}
				}
			}
		}
		break;
		case en_GameObjectType::OBJECT_ENVIRONMENT:
		case en_GameObjectType::OBJECT_STONE:
		case en_GameObjectType::OBJECT_TREE:
		{
			for (int32 i = 0; i < en_Channel::ENVIRONMENT_MAX; i++)
			{
				if (_ChannelEnvironmentArray[i] != nullptr && _ChannelEnvironmentArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
				{
					if (st_Vector2::CheckFieldOfView(_ChannelEnvironmentArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position, Object->GetPositionInfo().MoveDir, 80))
					{
						// �þ߰� �ȿ� ������Ʈ�� ����
						float TargetDistance = st_Vector2::Distance(_ChannelEnvironmentArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
						if (TargetDistance <= Distance)
						{
							FindObjects.push_back(_ChannelEnvironmentArray[i]);
						}
					}
				}
			}
		}
		break;		
		}
	}

	return FindObjects;
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
		
	if (EnterChannelGameObject->_GameObjectInfo.ObjectType == en_GameObjectType::OBJECT_PLAYER_DUMMY)
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
	else
	{
		SpawnPosition = *ObjectSpawnPosition;
	}
	
	// ������ ������Ʈ�� Ÿ�Կ� ����
	switch ((en_GameObjectType)EnterChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:	
		{
			// �÷��̾�� ����ȯ
			CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;
			EnterChannelPlayer->_SpawnPosition = SpawnPosition;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;

			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;

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
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		{
			// �÷��̾�� ����ȯ
			CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;
			EnterChannelPlayer->_SpawnPosition = SpawnPosition;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;

			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;

			// �÷��̾� ����
			_ChannelDummyPlayerArrayIndexs.Pop(&EnterChannelPlayer->_ChannelArrayIndex);
			_ChannelDummyPlayerArray[EnterChannelPlayer->_ChannelArrayIndex] = EnterChannelPlayer;

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
		EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;

		EnterChannelMonster->Start();

		EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
		EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;

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
		EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;

		EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
		EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;

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
		EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;

		EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
		EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;		

		EnterChannelEnvironment->Init(EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);

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
		_ChannelPlayerArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);

		_Map->ApplyLeave(LeaveChannelGameObject);
		break;
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		_ChannelDummyPlayerArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);

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