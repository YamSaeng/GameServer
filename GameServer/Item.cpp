#include "pch.h"
#include "Item.h"
#include "ObjectManager.h"

CItem::CItem()
{
	
}

void CItem::Update()
{
	if (_DestroyTime < GetTickCount64())
	{
	}

	// 타겟의 상태가 LEAVE면 타겟을 없애준다.
	if (_Target && _Target->_NetworkState == en_ObjectNetworkState::LEAVE)
	{
		_Target = nullptr;
	}

	// 상태에 따라 Update문 따로 호출
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
		G_Logger->WriteStdOut(en_Color::GREEN, L"플레이어와 부딪힘");
				
		CPlayer* Player = nullptr;

		switch (_Target->_GameObjectInfo.ObjectType)
		{
		case en_GameObjectType::MELEE_PLAYER:
		case en_GameObjectType::MAGIC_PLAYER:
			Player = (CPlayer*)_Target;
			break;
		default:
			break;
		}

		// Item 인벤토리 저장 요청 Job 생성
		st_Job* ReqItemToInventoryJob = G_ObjectManager->GameServer->_JobMemoryPool->Alloc();
		ReqItemToInventoryJob->Type = en_MESSAGE_TYPE::NETWORK_MESSAGE;
		ReqItemToInventoryJob->SessionId = Player->_SessionId;
		ReqItemToInventoryJob->Session = nullptr;

		// Item 정보 담기
		CMessage* ReqItemToInventoryMessage = CMessage::Alloc();
		ReqItemToInventoryMessage->Clear();

		*ReqItemToInventoryMessage << (int16)(en_PACKET_TYPE::en_PACKET_C2S_ITEM_TO_INVENTORY);
		*ReqItemToInventoryMessage << Player->_AccountId;
		*ReqItemToInventoryMessage << _GameObjectInfo.ObjectId;
		*ReqItemToInventoryMessage << (int8)_GameObjectInfo.ObjectType;
		*ReqItemToInventoryMessage << _Target->_GameObjectInfo.ObjectId;
		*ReqItemToInventoryMessage << (int8)_Target->_GameObjectInfo.ObjectType;

		ReqItemToInventoryJob->Message = ReqItemToInventoryMessage;

		G_ObjectManager->GameServer->_GameServerNetworkThreadMessageQue.Enqueue(ReqItemToInventoryJob);
	}
}

CMaterial::CMaterial()
{

}