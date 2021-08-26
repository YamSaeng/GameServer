#include "pch.h"
#include "Slime.h"
#include "DataManager.h"
#include "Player.h"
#include "ObjectManager.h"

CSlime::CSlime()
{	
	_GameObjectInfo.ObjectType = en_GameObjectType::SLIME;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	auto FindMonsterStat = G_Datamanager->_Monsters.find(1);
	st_MonsterData MonsterData = *(*FindMonsterStat).second;

	// ���� ����
	_GameObjectInfo.ObjectName.assign(MonsterData._MonsterName.begin(), MonsterData._MonsterName.end());
	_GameObjectInfo.ObjectStatInfo.Attack = MonsterData._MonsterStatInfo.Attack;
	_GameObjectInfo.ObjectStatInfo.MaxHP = MonsterData._MonsterStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.HP = MonsterData._MonsterStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.Level = MonsterData._MonsterStatInfo.Level;
	_GameObjectInfo.ObjectStatInfo.Speed = MonsterData._MonsterStatInfo.Speed;

	_SearchCellDistance = MonsterData._MonsterStatInfo.SearchCellDistance;
	_ChaseCellDistance = MonsterData._MonsterStatInfo.ChaseCellDistance;
	_AttackRange = MonsterData._MonsterStatInfo.AttackRange;
}

CSlime::~CSlime()
{
}

void CSlime::Init(int32 DataSheetId)
{
	_DataSheetId = DataSheetId;
}

void CSlime::UpdateIdle()
{
	if (_NextSearchTick > GetTickCount64())
	{
		return;
	}

	_NextSearchTick = GetTickCount64() + 1000;

	CPlayer* Target = _Channel->FindNearPlayer(this, _SearchCellDistance);
	if (Target == nullptr)
	{
		return;
	}

	_Target = Target;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
}

void CSlime::UpdateMoving()
{
	if (_NextMoveTick > GetTickCount64())
	{
		return;
	}

	int MoveTick = (int)(1000 / _GameObjectInfo.ObjectStatInfo.Speed);
	_NextMoveTick = GetTickCount64() + MoveTick;

	//G_Logger->WriteStdOut(en_Color::RED, L"UpdateMoving Func CAll\n");

	// Ÿ���� ���ų� ���� �ٸ� ä�ο� ���� ��� Idle ���·� ��ȯ�Ѵ�.
	if (_Target == nullptr || _Target->_Channel != _Channel)
	{
		_Target = nullptr;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	st_Vector2Int TargetPosition = _Target->GetCellPosition();
	st_Vector2Int MonsterPosition = GetCellPosition();

	// ���Ⱚ ���Ѵ�.
	st_Vector2Int Direction = TargetPosition - MonsterPosition;
	// Ÿ�ٰ� ������ �Ÿ��� ���.
	int32 Distance = Direction.CellDistanceFromZero();
	// Ÿ�ٰ��� �Ÿ��� 0 �Ǵ� �߰ݰŸ� ���� �־�����
	if (Distance == 0 || Distance > _ChaseCellDistance)
	{
		_Target = nullptr;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	vector<st_Vector2Int> Path = _Channel->_Map->FindPath(MonsterPosition, TargetPosition);
	// �߰��߿� �÷��̾����� �ٰ����� ���ų� �߰ݰŸ��� ����� �����.
	if (Path.size() < 2 || Path.size() > _ChaseCellDistance)
	{
		_Target = nullptr;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	if (Distance <= _AttackRange && (Direction._X == 0 || Direction._Y == 0))
	{
		_AttackTick = GetTickCount64() + 500;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
		_GameObjectInfo.ObjectPositionInfo.MoveDir = GetDirectionFromVector(Direction);
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	_GameObjectInfo.ObjectPositionInfo.MoveDir = GetDirectionFromVector(Path[1] - MonsterPosition);
	_Channel->_Map->ApplyMove(this, Path[1]);

	BroadCastPacket(en_PACKET_S2C_MOVE);
}

void CSlime::UpdateAttack()
{
	if (_AttackTick == 0)
	{
		// Ÿ���� ������ų� ä���� �޶��� ��� Ÿ���� ����
		if (_Target == nullptr || _Target->_Channel != _Channel)
		{
			_Target = nullptr;
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			return;
		}

		// ������ �������� Ȯ��
		st_Vector2Int TargetCellPosition = _Target->GetCellPosition();
		st_Vector2Int MyCellPosition = GetCellPosition();
		st_Vector2Int Direction = TargetCellPosition - MyCellPosition;

		_GameObjectInfo.ObjectPositionInfo.MoveDir = GetDirectionFromVector(Direction);

		int32 Distance = Direction.CellDistanceFromZero();
		// Ÿ�ٰ��� �Ÿ��� ���� ���� �ȿ� ���ϰ� X==0 || Y ==0 �϶�( �밢���� ����) ����
		bool CanUseAttack = (Distance <= _AttackRange && (Direction._X == 0 || Direction._Y == 0));
		if (CanUseAttack == false)
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			return;
		}

		// ������ ����
		_Target->OnDamaged(this, _GameObjectInfo.ObjectStatInfo.Attack);
		BroadCastPacket(en_PACKET_S2C_ATTACK);
		// ���� �÷��̾�鿡�� ������ ���� ��� ����
		BroadCastPacket(en_PACKET_S2C_CHANGE_HP);

		// 0.8�ʸ��� ����
		_AttackTick = GetTickCount64() + 800;
		BroadCastPacket(en_PACKET_S2C_MESSAGE);
	}

	if (_AttackTick > GetTickCount64())
	{
		return;
	}

	_AttackTick = 0;
}

void CSlime::UpdateDead()
{

}

void CSlime::GetRandomDropItem(CGameObject* Killer)
{
	bool Find = false;
	int64 KillerId = Killer->_GameObjectInfo.ObjectId;
	// ��� ������ ���� �о����
	auto FindMonsterDropItem = G_Datamanager->_Monsters.find(1);
	st_MonsterData MonsterData = *(*FindMonsterDropItem).second;

	random_device RD;
	mt19937 Gen(RD());

	uniform_int_distribution<int> RandomXPosition(0, 60);
	int32 Random = RandomXPosition(Gen);

	int32 Sum = 0;

	st_ItemData DropItemData;	
	for (st_DropData DropItem : MonsterData._DropItems)
	{
		Sum += DropItem._Probability;

		if (Sum >= Random)
		{
			Find = true;
			// ��� Ȯ�� �Ǹ� �ش� ������ �о����
			auto FindDropItemInfo = G_Datamanager->_Items.find(DropItem._ItemDataSheetId);
			if (FindDropItemInfo == G_Datamanager->_Items.end())
			{
				CRASH("DropItemInfo�� ã�� ����");
			}

			DropItemData = *(*FindDropItemInfo).second;
			DropItemData.Count = DropItem._Count;			
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
		case en_ItemType::ITEM_TYPE_BRONZE_COIN:
			GameObjectType = en_GameObjectType::BRONZE_COIN;
			break;
		default:
			break;
		}
		
		G_ObjectManager->ItemSpawn(1, GetCellPosition(), KillerId, NewItemInfo, GameObjectType);
	}	
}

void CSlime::OnDead(CGameObject* Killer)
{
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

	GetRandomDropItem(Killer);

	BroadCastPacket(en_PACKET_S2C_DIE);		

	G_ObjectManager->Remove(this, 1);	

	/*_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
	_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;	

	Channel->EnterChannel(this);
	BroadCastPacket(en_PACKET_S2C_SPAWN);*/
}

