#include "pch.h"
#include "NonPlayer.h"
#include "DataManager.h"
#include "ObjectManager.h"

CNonPlayer::CNonPlayer()
{
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
}

CNonPlayer::~CNonPlayer()
{

}

void CNonPlayer::Update()
{
}

bool CNonPlayer::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	return false;
}