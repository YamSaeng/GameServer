#include "pch.h"
#include "Player.h"

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

void CPlayer::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	//CGameObject::OnDamaged(Attacker, Damage);

}

void CPlayer::OnDead(CGameObject* Killer)
{
}
