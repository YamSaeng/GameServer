#include "pch.h"
#include "Crop.h"
#include "ObjectManager.h"
#include "DataManager.h"

CCrop::CCrop()
{
	_FieldOfViewDistance = 10;

	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;	

	_CropState = en_CropState::CROP_IDLE;
}

CCrop::~CCrop()
{

}

void CCrop::Init(en_SmallItemCategory CropItemCategory)
{
	_CropItemInfo = *G_Datamanager->FindItemData(CropItemCategory);

	_CropIdleTick = GetTickCount64() + 1000;	

	_GameObjectInfo.ObjectCropMaxStep = _CropItemInfo.ItemMaxstep;
	_GameObjectInfo.ObjectCropStep = 1;	

	_CropRatio = 0;

	_CropTime = 0;	

	_CropState = en_CropState::CROP_GROWING;
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

bool CCrop::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	return CGameObject::OnDamaged(Attacker, Damage);	
}

void CCrop::UpdateIdle()
{
	if (_CropIdleTick < GetTickCount64() && _GameObjectInfo.ObjectCropStep < _GameObjectInfo.ObjectCropMaxStep)
	{
		_CropIdleTick = GetTickCount64() + 1000;

		_CropTime += 1;
		
		float Ratio = 1 / (float)_CropItemInfo.ItemGrowTime;

		_CropRatio += Ratio;

		if (_CropRatio > (1 / (float)_GameObjectInfo.ObjectCropMaxStep))
		{			
			_GameObjectInfo.ObjectCropStep += 1;			
			
			_CropRatio = 0;
		}				

		switch (_CropState)
		{
		case en_CropState::CROP_IDLE:
			break;
		case en_CropState::CROP_GROWING:			
			if (_CropTime >= _CropItemInfo.ItemGrowTime)
			{
				_CropState = en_CropState::CROP_GROW_END;
			}
			break;
		case en_CropState::CROP_GROW_END:
			break;
		default:
			break;
		}
	}
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

		st_GameObjectJob* DeSpawnCropChanelJob = G_ObjectManager->GameServer->MakeGameObjectJobObjectDeSpawnObjectChannel(this);
		_Channel->_ChannelJobQue.Enqueue(DeSpawnCropChanelJob);		
	}
}

void CCrop::UpdateDead()
{
	if (_DeadTick < GetTickCount64())
	{
		st_GameObjectJob* LeaveChannelEnvironmentJob = G_ObjectManager->GameServer->MakeGameObjectJobLeaveChannel(this);
		_Channel->_ChannelJobQue.Enqueue(LeaveChannelEnvironmentJob);
	}
}