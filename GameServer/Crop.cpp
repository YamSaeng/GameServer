#include "pch.h"
#include "Crop.h"
#include "ObjectManager.h"

CCrop::CCrop()
{
	_FieldOfViewDistance = 10;
}

CCrop::~CCrop()
{

}

void CCrop::Update()
{
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::SPAWN_IDLE:
		break;
	case en_CreatureState::IDLE:	
		UpdateIdle();
		break;
	case en_CreatureState::READY_DEAD:
		UpdateReadyDead();
		break;
	case en_CreatureState::DEAD:
		UpdateDead();
		break;
	default:
		break;
	}
}

void CCrop::CropStart(int8 CropStep)
{
}

bool CCrop::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	return CGameObject::OnDamaged(Attacker, Damage);	
}

void CCrop::UpdateIdle()
{
}

void CCrop::UpdateReadyDead()
{
	if (_DeadReadyTick < GetTickCount64())
	{
		_DeadTick = GetTickCount64() + 2000;

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

		if (_Channel == nullptr)
		{
			CRASH("퇴장하려는 채널이 존재하지 않음");
		}

		st_GameObjectJob* LeaveChannelEnvironmentJob = G_ObjectManager->GameServer->MakeGameObjectJobLeaveChannel(this);
		_Channel->_ChannelJobQue.Enqueue(LeaveChannelEnvironmentJob);
	}
}

void CCrop::UpdateDead()
{

}
