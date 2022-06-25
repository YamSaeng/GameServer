#include "pch.h"
#include "Crop.h"

CCrop::CCrop()
{

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
		break;
	default:
		break;
	}
}
