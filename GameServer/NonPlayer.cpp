#include "pch.h"
#include "NonPlayer.h"
#include "DataManager.h"
#include "ObjectManager.h"

CNonPlayer::CNonPlayer()
{
	
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