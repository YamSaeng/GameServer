#include "pch.h"
#include "Player.h"
#include "ObjectManager.h"

CPlayer::CPlayer()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::MELEE_PLAYER;	
}

CPlayer::CPlayer(st_GameObjectInfo _PlayerInfo)
{
	_GameObjectInfo = _PlayerInfo;		
	_GameObjectInfo.ObjectType = en_GameObjectType::MELEE_PLAYER;		
}

CPlayer::~CPlayer()
{
}

void CPlayer::Update()
{	
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::ATTACK:
		UpdateAttack();
		break;
	default:
		break;
	}
}

void CPlayer::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	//CGameObject::OnDamaged(Attacker, Damage);

}

void CPlayer::OnDead(CGameObject* Killer)
{

}

void CPlayer::UpdateAttack()
{
	// ������ AttackTick�ð��� �Ǹ�
	if (_AttackTick < GetTickCount64())
	{		
		// Idle ���·� �ٲٰ� ���� ���� �÷��̾�鿡�� �˷��ش�.
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
		BroadCastPacket(en_PACKET_S2C_OBJECT_STATE_CHANGE);
	}
}

void CPlayer::BroadCastPacket(en_PACKET_TYPE PacketType)
{
	CMessage* ResPacket = nullptr;

	switch (PacketType)
	{	
	case en_PACKET_S2C_OBJECT_STATE_CHANGE:
		ResPacket = G_ObjectManager->GameServer->MakePacketResObjectState(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.MoveDir, _GameObjectInfo.ObjectType, _GameObjectInfo.ObjectPositionInfo.State);
		break;	
	default:
		CRASH("Player BroadCast PacketType Error");
		break;
	}

	G_ObjectManager->GameServer->SendPacketAroundSector(this->GetCellPosition(), ResPacket);
	ResPacket->Free();
}
