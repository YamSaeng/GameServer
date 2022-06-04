#include "pch.h"
#include "Furnace.h"

CFurnace::CFurnace()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_FURNACE;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
}
