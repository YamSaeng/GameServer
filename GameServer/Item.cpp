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
		BroadCastPacket(en_PACKET_TYPE::en_PACKET_S2C_DESPAWN);		
		G_ObjectManager->Remove(this, 1);		
	}
}

void CItem::SetDestoryTime(int32 DestoryTime)
{
	_DestroyTime += (GetTickCount64() + DestoryTime);
}

CMaterial::CMaterial()
{	
	
}