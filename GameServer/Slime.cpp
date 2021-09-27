#include "pch.h"
#include "Slime.h"
#include "DataManager.h"
#include "Player.h"
#include "ObjectManager.h"
#include <atlbase.h>

CSlime::CSlime()
{	
	_GameObjectInfo.ObjectType = en_GameObjectType::SLIME;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	auto FindMonsterStat = G_Datamanager->_Monsters.find(1);
	st_MonsterData MonsterData = *(*FindMonsterStat).second;

	// ���� ����	
	_GameObjectInfo.ObjectName = (LPWSTR)CA2W(MonsterData.MonsterName.c_str());
	_GameObjectInfo.ObjectStatInfo.MinAttackDamage = MonsterData.MonsterStatInfo.MinAttackDamage;
	_GameObjectInfo.ObjectStatInfo.MaxAttackDamage = MonsterData.MonsterStatInfo.MaxAttackDamage;
	_GameObjectInfo.ObjectStatInfo.CriticalPoint = MonsterData.MonsterStatInfo.CriticalPoint;
	_GameObjectInfo.ObjectStatInfo.MaxHP = MonsterData.MonsterStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.HP = MonsterData.MonsterStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.Level = MonsterData.MonsterStatInfo.Level;
	_GameObjectInfo.ObjectStatInfo.Speed = MonsterData.MonsterStatInfo.Speed;

	_SearchCellDistance = MonsterData.MonsterStatInfo.SearchCellDistance;
	_ChaseCellDistance = MonsterData.MonsterStatInfo.ChaseCellDistance;
	_AttackRange = MonsterData.MonsterStatInfo.AttackRange;

	_GetDPPoint = MonsterData.GetDPPoint;
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
	int32 Distance = st_Vector2Int::Distance(TargetPosition, MonsterPosition);
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
		_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetDirectionFromVector(Direction);
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
		return;
	}

	_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetDirectionFromVector(Path[1] - MonsterPosition);
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

		_GameObjectInfo.ObjectPositionInfo.MoveDir = st_Vector2Int::GetDirectionFromVector(Direction);

		int32 Distance = Direction.CellDistanceFromZero();
		// Ÿ�ٰ��� �Ÿ��� ���� ���� �ȿ� ���ϰ� X==0 || Y ==0 �϶�( �밢���� ����) ����
		bool CanUseAttack = (Distance <= _AttackRange && (Direction._X == 0 || Direction._Y == 0));
		if (CanUseAttack == false)
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;
			return;
		}

		// ũ��Ƽ�� �Ǵ�		
		random_device Seed;
		default_random_engine Eng(Seed());

		float CriticalPoint = _GameObjectInfo.ObjectStatInfo.CriticalPoint / 1000.0f;
		bernoulli_distribution CriticalCheck(CriticalPoint);
		bool IsCritical = CriticalCheck(Eng);

		// ������ �Ǵ�
		mt19937 Gen(Seed());
		uniform_int_distribution<int> DamageChoiceRandom(_GameObjectInfo.ObjectStatInfo.MinAttackDamage, _GameObjectInfo.ObjectStatInfo.MaxAttackDamage);
		int32 ChoiceDamage = DamageChoiceRandom(Gen);
		int32 FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

		_Target->OnDamaged(this, FinalDamage);

		CMessage* ResSlimeAttackPacket = G_ObjectManager->GameServer->MakePacketResAttack(_GameObjectInfo.ObjectId, _Target->_GameObjectInfo.ObjectId, en_SkillType::SKILL_SLIME_NORMAL, FinalDamage, IsCritical);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResSlimeAttackPacket);
		ResSlimeAttackPacket->Free();

		// ���� �÷��̾�鿡�� ������ ���� ��� ����
		BroadCastPacket(en_PACKET_S2C_CHANGE_OBJECT_STAT);

		// 0.8�ʸ��� ����
		_AttackTick = GetTickCount64() + 800;
		
		wchar_t SlimeAttackMessage[64] = L"0";
		wsprintf(SlimeAttackMessage, L"%s�� �Ϲ� ������ ����� %s���� %d�� �������� ����ϴ�", _GameObjectInfo.ObjectName.c_str(), _Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
		
		wstring SlimeAttackString = SlimeAttackMessage;

		CMessage* ResSlimeSystemMessage = G_ObjectManager->GameServer->MakePacketResChattingMessage(_Target->_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, IsCritical ? st_Color::Red() : st_Color::White(), IsCritical ? L"ġ��Ÿ! " + SlimeAttackString : SlimeAttackString);
		G_ObjectManager->GameServer->SendPacketAroundSector(GetCellPosition(), ResSlimeSystemMessage);
		ResSlimeSystemMessage->Free();
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

void CSlime::OnDead(CGameObject* Killer)
{
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

	G_ObjectManager->ItemSpawn(Killer->_GameObjectInfo.ObjectId, Killer->_GameObjectInfo.ObjectType, GetCellPosition(), en_MonsterDataType::SLIME_DATA);
	
	Killer->_GameObjectInfo.ObjectStatInfo.DP += _GetDPPoint;

	if (Killer->_GameObjectInfo.ObjectStatInfo.DP >= Killer->_GameObjectInfo.ObjectStatInfo.MaxDP)
	{
		Killer->_GameObjectInfo.ObjectStatInfo.DP = Killer->_GameObjectInfo.ObjectStatInfo.MaxDP;
	}

	BroadCastPacket(en_PACKET_S2C_CHANGE_OBJECT_STAT);
	BroadCastPacket(en_PACKET_S2C_DIE);		

	G_ObjectManager->Remove(this, 1);	

	/*_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
	_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;	

	Channel->EnterChannel(this);
	BroadCastPacket(en_PACKET_S2C_SPAWN);*/
}

