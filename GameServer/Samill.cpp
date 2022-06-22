#include "pch.h"
#include "Samill.h"
#include "DataManager.h"

CSamill::CSamill()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	_GameObjectInfo.ObjectWidth = 2;
	_GameObjectInfo.ObjectHeight = 1;

	_GameObjectInfo.ObjectName = L"Á¦Àç¼Ò";

	auto FindSamillData = G_Datamanager->_CraftingTableData.find((int16)_GameObjectInfo.ObjectType);
	_CraftingTableRecipe = *(*FindSamillData).second;
}

void CSamill::Update()
{
}

void CSamill::UpdateCrafting()
{
}
