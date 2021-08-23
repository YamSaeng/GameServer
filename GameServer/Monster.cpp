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
	
	if (_GameObjectInfo.ObjectStatInfo.HP == 0)
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;
		OnDead(Attacker);
	}
}

void CMonster::OnDead(CGameObject* Killer)
{
	BroadCastPacket(en_PACKET_S2C_DIE);
	BroadCastPacket(en_PACKET_S2C_DESPAWN);

	CChannel* Channel = _Channel;
	Channel->LeaveChannel(this);

	_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
	_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;

	Channel->EnterChannel(this);
	BroadCastPacket(en_PACKET_S2C_SPAWN);
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
		ResPacket = G_ObjectManager->GameServer->MakePacketResChangeHP(_Target->_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectStatInfo.Attack, _Target->_GameObjectInfo.ObjectStatInfo.HP, _Target->_GameObjectInfo.ObjectStatInfo.MaxHP);
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