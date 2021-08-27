#include "pch.h"
#include "Monster.h"
#include "Player.h"
#include "DataManager.h"
#include "ObjectManager.h"

CMonster::CMonster()
{
	_NextSearchTick = GetTickCount64();
	_NextMoveTick = GetTickCount64();	
}

CMonster::~CMonster()
{

}

void CMonster::Init(int32 DataSheetId)
{
	_DataSheetId = DataSheetId;
}

void CMonster::GetRandomDropItem(CGameObject* Killer, en_MonsterDataType MonsterDataType)
{
	bool Find = false;
	int64 KillerId = Killer->_GameObjectInfo.ObjectId;

	auto FindMonsterDropItem = G_Datamanager->_Monsters.find(MonsterDataType);
	st_MonsterData MonsterData = *(*FindMonsterDropItem).second;

	random_device RD;
	mt19937 Gen(RD());

	uniform_int_distribution<int> RandomDropPoint(0, 60);
	int32 RandomPoint = RandomDropPoint(Gen);

	int32 Sum = 0;

	st_ItemData DropItemData;
	for (st_DropData DropItem : MonsterData._DropItems)
	{
		Sum += DropItem.Probability;

		if (Sum >= RandomPoint)
		{
			Find = true;
			// 드랍 확정 되면 해당 아이템 읽어오기
			auto FindDropItemInfo = G_Datamanager->_Items.find(DropItem.ItemDataSheetId);
			if (FindDropItemInfo == G_Datamanager->_Items.end())
			{
				CRASH("DropItemInfo를 찾지 못함");
			}

			DropItemData = *(*FindDropItemInfo).second;

			uniform_int_distribution<int> RandomDropItemCount(DropItem.MinCount, DropItem.MaxCount);
			DropItemData.Count = RandomDropItemCount(Gen);
			break;
		}
	}

	if (Find == true)
	{
		st_ItemInfo NewItemInfo;

		NewItemInfo.ItemDBId = -1;
		NewItemInfo.Count = DropItemData.Count;
		NewItemInfo.SlotNumber = -1;
		NewItemInfo.IsEquipped = DropItemData.IsEquipped;
		NewItemInfo.ItemType = DropItemData._ItemType;
		NewItemInfo.ThumbnailImagePath.assign(DropItemData._ImagePath.begin(), DropItemData._ImagePath.end());
		NewItemInfo.ItemName.assign(DropItemData._Name.begin(), DropItemData._Name.end());

		en_GameObjectType GameObjectType;
		switch (NewItemInfo.ItemType)
		{
		case en_ItemType::ITEM_TYPE_SLIMEGEL:
			GameObjectType = en_GameObjectType::SLIME_GEL;
			break;
		case en_ItemType::ITEM_TYPE_LEATHER:
			GameObjectType = en_GameObjectType::LEATHER;
			break;
		case en_ItemType::ITEM_TYPE_BRONZE_COIN:
			GameObjectType = en_GameObjectType::BRONZE_COIN;
			break;		
		default:
			break;
		}

		G_ObjectManager->ItemSpawn(1, GetCellPosition(), KillerId, NewItemInfo, GameObjectType);
	}
}

void CMonster::Update()
{
	if (_Target && _Target->_NetworkState == en_ObjectNetworkState::LEAVE)
	{
		_Target = nullptr;		
	}

	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::IDLE:
		UpdateIdle();
		break;
	case en_CreatureState::MOVING:
		UpdateMoving();
		break;
	case en_CreatureState::ATTACK:
		UpdateAttack();
		break;
	case en_CreatureState::DEAD:
		UpdateDead();
		break;
	default:
		break;
	}
}

void CMonster::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	CGameObject::OnDamaged(Attacker, Damage);
	
	_Target = (CPlayer*)Attacker;	

	if (_GameObjectInfo.ObjectStatInfo.HP == 0)
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;
		OnDead(Attacker);
	}
}

void CMonster::BroadCastPacket(en_PACKET_TYPE PacketType)
{	
	CMessage* ResPacket = nullptr;

	switch (PacketType)
	{
	case en_PACKET_S2C_MOVE:
		ResPacket = G_ObjectManager->GameServer->MakePacketResMove(-1, _GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType, _GameObjectInfo.ObjectPositionInfo);
		break;
	case en_PACKET_S2C_OBJECT_STATE_CHANGE:		
		ResPacket = G_ObjectManager->GameServer->MakePacketResObjectState(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.MoveDir,
			_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.State);
		break;
	case en_PACKET_S2C_ATTACK:
		ResPacket = G_ObjectManager->GameServer->MakePacketResAttack(-1, _GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.MoveDir, en_AttackType::BEAR_NORMAL_ATTACK, false);
		break;
	case en_PACKET_S2C_CHANGE_HP:
		ResPacket = G_ObjectManager->GameServer->MakePacketResChangeHP(_Target->_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectStatInfo.Attack, 
			_Target->_GameObjectInfo.ObjectStatInfo.HP,
			_Target->_GameObjectInfo.ObjectStatInfo.MaxHP, 
			false,
			_Target->GetCellPosition()._X,
			_Target->GetCellPosition()._Y);
		break;
	case en_PACKET_S2C_DIE:
		ResPacket = G_ObjectManager->GameServer->MakePacketResDie(this->_GameObjectInfo.ObjectId);
		break;
	case en_PACKET_S2C_SPAWN:
	{
		vector<st_GameObjectInfo> SpawnObjectIds;
		SpawnObjectIds.push_back(_GameObjectInfo);
		ResPacket = G_ObjectManager->GameServer->MakePacketResSpawn(1, SpawnObjectIds);
	}		
		break;
	case en_PACKET_S2C_DESPAWN:
	{
		vector<int64> DeSpawnObjectIds;
		DeSpawnObjectIds.push_back(_GameObjectInfo.ObjectId);
		ResPacket = G_ObjectManager->GameServer->MakePacketResDeSpawn(1, DeSpawnObjectIds);
	}		
		break;	
	case en_PACKET_S2C_MESSAGE:
	{
		wchar_t AttackMessage[256] = L"0";
		wsprintf(AttackMessage, L"%s이 %s을 %d의 공격력으로 공격", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), _GameObjectInfo.ObjectStatInfo.Attack);

		wstring AttackSystemMessage = AttackMessage;
		ResPacket = G_ObjectManager->GameServer->MakePacketResChattingMessage(_Target->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, AttackSystemMessage);
	}		
		break;
	default:
		break;
	}

	G_ObjectManager->GameServer->SendPacketAroundSector(this->GetCellPosition(), ResPacket);
	ResPacket->Free();
}