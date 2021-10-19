#include "pch.h"
#include "Player.h"
#include "ObjectManager.h"

CPlayer::CPlayer()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_PLAYER;	
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
	_AttackTick = 0;
	_SpellTick = 0;
	_SkillJob = nullptr;
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
	case en_CreatureState::SPELL:
		UpdateSpell();
		break;
	default:
		break;
	}
}

void CPlayer::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	CGameObject::OnDamaged(Attacker, Damage);

}

void CPlayer::OnDead(CGameObject* Killer)
{

}

void CPlayer::UpdateAttack()
{

}

void CPlayer::UpdateSpell()
{	

}
