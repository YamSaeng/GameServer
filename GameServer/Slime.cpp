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

	// 스탯 셋팅
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

	// 타겟이 없거나 나와 다른 채널에 있을 경우 Idle 상태로 전환한다.
	if (_Target == nullptr || _Target->_Channel != _Channel)
	{
		_Target = nullptr;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	st_Vector2Int TargetPosition = _Target->GetCellPosition();
	st_Vector2Int MonsterPosition = GetCellPosition();

	// 방향값 구한다.
	st_Vector2Int Direction = TargetPosition - MonsterPosition;
	// 타겟과 몬스터의 거리를 잰다.
	int32 Distance = Direction.CellDistanceFromZero();
	// 타겟과의 거리가 0 또는 추격거리 보다 멀어지면
	if (Distance == 0 || Distance > _ChaseCellDistance)
	{
		_Target = nullptr;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	vector<st_Vector2Int> Path = _Channel->_Map->FindPath(MonsterPosition, TargetPosition);
	// 추격중에 플레이어한테 다가갈수 없거나 추격거리를 벗어나면 멈춘다.
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
		// 타겟이 사라지거나 채널이 달라질 경우 타겟을 해제
		if (_Target == nullptr || _Target->_Channel != _Channel)
		{
			_Target = nullptr;
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			return;
		}

		// 공격이 가능한지 확인
		st_Vector2Int TargetCellPosition = _Target->GetCellPosition();
		st_Vector2Int MyCellPosition = GetCellPosition();
		st_Vector2Int Direction = TargetCellPosition - MyCellPosition;

		_GameObjectInfo.ObjectPositionInfo.MoveDir = GetDirectionFromVector(Direction);

		int32 Distance = Direction.CellDistanceFromZero();
		// 타겟과의 거리가 공격 범위 안에 속하고 X==0 || Y ==0 일때( 대각선은 제한) 공격
		bool CanUseAttack = (Distance <= _AttackRange && (Direction._X == 0 || Direction._Y == 0));
		if (CanUseAttack == false)
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			return;
		}

		// 데미지 적용
		_Target->OnDamaged(this, _GameObjectInfo.ObjectStatInfo.Attack);
		BroadCastPacket(en_PACKET_S2C_ATTACK);
		// 주위 플레이어들에게 데미지 적용 결과 전송
		BroadCastPacket(en_PACKET_S2C_CHANGE_HP);

		// 0.8초마다 공격
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
	// 드랍 아이템 정보 읽어오기
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
			// 드랍 확정 되면 해당 아이템 읽어오기
			auto FindDropItemInfo = G_Datamanager->_Items.find(DropItem._ItemDataSheetId);
			if (FindDropItemInfo == G_Datamanager->_Items.end())
			{
				CRASH("DropItemInfo를 찾지 못함");
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

