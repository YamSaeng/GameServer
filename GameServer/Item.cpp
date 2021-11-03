#include "pch.h"
#include "Item.h"
#include "ObjectManager.h"
#include "GameServerMessage.h"

CItem::CItem()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
}

CItem::~CItem()
{
	
}

void CItem::Update()
{
	if (_DestroyTime < GetTickCount64())
	{
	}

	// Ÿ���� ���°� LEAVE�� Ÿ���� �����ش�.
	if (_Target && _Target->_NetworkState == en_ObjectNetworkState::LEAVE)
	{
		_Target = nullptr;
	}

	// ���¿� ���� Update�� ���� ȣ��
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::IDLE:
		UpdateIdle();
		break;
	default:
		break;
	}
}

void CItem::SetDestoryTime(int32 DestoryTime)
{
	_DestroyTime += (GetTickCount64() + DestoryTime);
}

void CItem::ItemSetTarget(en_GameObjectType TargetType, int64 TargetDBId)
{
	_Target = G_ObjectManager->Find(TargetDBId, TargetType);
}

void CItem::UpdateIdle()
{
	if (_Target && _Target->GetCellPosition() == GetCellPosition())
	{
		G_Logger->WriteStdOut(en_Color::GREEN, L"�÷��̾�� �ε��� \n");

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;		
				
		CPlayer* Player = nullptr;

		switch (_Target->_GameObjectInfo.ObjectType)
		{
		case en_GameObjectType::OBJECT_MELEE_PLAYER:
		case en_GameObjectType::OBJECT_MAGIC_PLAYER:
		case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
			Player = (CPlayer*)_Target;
			break;
		default:
			break;
		}

		// Item �κ��丮 ���� ��û Job ����
		st_Job* ReqItemToInventoryJob = G_ObjectManager->GameServer->_JobMemoryPool->Alloc();
		ReqItemToInventoryJob->Type = en_JobType::NETWORK_MESSAGE;
		ReqItemToInventoryJob->SessionId = Player->_SessionId;
		ReqItemToInventoryJob->Session = nullptr;

		// Item ���� ���
		CGameServerMessage* ReqItemToInventoryMessage = CGameServerMessage::GameServerMessageAlloc();
		ReqItemToInventoryMessage->Clear();

		*ReqItemToInventoryMessage << (int16)(en_PACKET_TYPE::en_PACKET_C2S_ITEM_TO_INVENTORY);
		*ReqItemToInventoryMessage << Player->_AccountId;
		*ReqItemToInventoryMessage << _GameObjectInfo.ObjectId;						
		*ReqItemToInventoryMessage << _Target->_GameObjectInfo.ObjectId;				

		ReqItemToInventoryJob->Message = ReqItemToInventoryMessage;

		G_ObjectManager->GameServer->_GameServerNetworkThreadMessageQue.Enqueue(ReqItemToInventoryJob);
	}
}

CWeapon::CWeapon()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM_WEAPON;
}

CWeapon::~CWeapon()
{
}

void CWeapon::UpdateIdle()
{
	CItem::UpdateIdle();
}

CArmor::CArmor()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM_ARMOR;
}

CArmor::~CArmor()
{
}

void CArmor::UpdateIdle()
{
	CItem::UpdateIdle();
}

CMaterial::CMaterial()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ITEM_MATERIAL;
}

CMaterial::~CMaterial()
{
}

void CMaterial::UpdateIdle()
{
	CItem::UpdateIdle();	
}