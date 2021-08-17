#include "pch.h"
#include "Monster.h"
#include "Player.h"
#include "DataManager.h"
#include "ObjectManager.h"

CMonster::CMonster()
{
	_NextSearchTick = GetTickCount64();
	_NextMoveTick = GetTickCount64();

	_GameObjectInfo.ObjectType = en_GameObjectType::MONSTER;
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

CMonster::~CMonster()
{

}

void CMonster::Init(int32 DataSheetId)
{
	_DataSheetId = DataSheetId;


}

void CMonster::Update()
{
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
		break;
	default:
		break;
	}
}

void CMonster::UpdateIdle()
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

void CMonster::UpdateMoving()
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
		_AttackTick = 0;
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::ATTACK;
		_GameObjectInfo.ObjectPositionInfo.MoveDir = GetDirectionFromVector(Direction);	
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}
	
	_GameObjectInfo.ObjectPositionInfo.MoveDir = GetDirectionFromVector(Path[1] - MonsterPosition);
	_Channel->_Map->ApplyMove(this, Path[1]);

	BroadCastPacket(en_PACKET_S2C_MOVE);	
}

void CMonster::UpdateAttack()
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
		// ���� �÷��̾�鿡�� ������ ���� ��� ����
		BroadCastPacket(en_PACKET_S2C_CHANGE_HP);

		// 0.5�ʸ��� ����
		_AttackTick = GetTickCount64() + 500;
	}	

	if (_AttackTick > GetTickCount64())
	{
		return;
	}

	_AttackTick = 0;
}

void CMonster::UpdateDead()
{

}

void CMonster::OnDamaged(CGameObject* Attacker, int32 Damage)
{

}

void CMonster::OnDead(CGameObject* Killer)
{
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
		ResPacket = G_ObjectManager->GameServer->MakePacketResObjectState(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.MoveDir, _GameObjectInfo.ObjectType, _GameObjectInfo.ObjectPositionInfo.State);
		break;
	case en_PACKET_S2C_CHANGE_HP:
		ResPacket = G_ObjectManager->GameServer->MakePacketResChangeHP(_Target->_GameObjectInfo.ObjectId, _Target->_GameObjectInfo.ObjectStatInfo.HP, _Target->_GameObjectInfo.ObjectStatInfo.MaxHP);
		break;
	default:
		break;
	}

	G_ObjectManager->GameServer->SendPacketAroundSector(this->GetCellPosition(), ResPacket);
	ResPacket->Free();
}
