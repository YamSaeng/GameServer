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
			DropItemData.DataSheetId = DropItem.ItemDataSheetId;
			break;
		}
	}

	if (Find == true)
	{
		st_ItemInfo NewItemInfo;

		NewItemInfo.ItemDBId = -1;
		NewItemInfo.DataSheetId = DropItemData.DataSheetId;
		NewItemInfo.ItemCount = DropItemData.Count;
		NewItemInfo.SlotIndex = -1;
		NewItemInfo.IsEquipped = DropItemData.IsEquipped;
		NewItemInfo.ItemType = DropItemData.ItemType;
		NewItemInfo.ThumbnailImagePath.assign(DropItemData.ThumbnailImagePath.begin(), DropItemData.ThumbnailImagePath.end());
		NewItemInfo.ItemName.assign(DropItemData.ItemName.begin(), DropItemData.ItemName.end());

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