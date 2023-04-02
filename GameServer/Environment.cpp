#include "pch.h"
#include "Environment.h"
#include "DataManager.h"
#include "ObjectManager.h"
#include "MapManager.h"
#include "NetworkManager.h"
#include "ChannelManager.h"
#include "RectCollision.h"
#include <atlbase.h>

CEnvironment::CEnvironment()
{
	_FieldOfViewDistance = 10;	
}

void CEnvironment::Start()
{
	_SpawnPosition = _GameObjectInfo.ObjectPositionInfo.CollisionPosition;

	_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
}

void CEnvironment::Update()
{
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::IDLE:
		UpdateIdle();
		break;	
	case en_CreatureState::DEAD:
		UpdateDead();
		break;
	default:
		break;
	}

}

bool CEnvironment::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	return CGameObject::OnDamaged(Attacker, Damage);	
}

void CEnvironment::UpdateIdle()
{

}

void CEnvironment::UpdateDead()
{
	if (_DeadTick < GetTickCount64())
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

		if (_Channel == nullptr)
		{
			CRASH("퇴장하려는 채널이 존재하지 않음");
		}

		st_GameObjectJob* LeaveChannelEnvironmentJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobLeaveChannel(this);
		_Channel->_ChannelJobQue.Enqueue(LeaveChannelEnvironmentJob);
	}

	if (_ReSpawnTick < GetTickCount64())
	{
		CMap* Map = G_MapManager->GetMap(1);
		if (Map != nullptr)
		{
			CGameObject* FindObject = Map->Find(_SpawnPosition);
			if (FindObject == nullptr)
			{
				st_GameObjectJob* ObjectEnterChannelJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobObjectEnterChannel(this);
				Map->GetChannelManager()->Find(1)->_ChannelJobQue.Enqueue(ObjectEnterChannelJob);
			}
			else
			{
				_ReSpawnTick = GetTickCount64() + 5000;
			}
		}
	}	
}

CStone::CStone()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_STONE;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	auto FindEnvironmentData = G_Datamanager->_Environments.find(en_GameObjectType::OBJECT_STONE);
	st_EnvironmentData EnvironmentData = *(*FindEnvironmentData).second;

	_GameObjectInfo.ObjectName = (LPWSTR)CA2W(EnvironmentData.EnvironmentName.c_str());
	_GameObjectInfo.ObjectStatInfo.MaxHP = EnvironmentData.MaxHP;
	_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.Level = EnvironmentData.Level;

	_GameObjectInfo.ObjectWidth = 1;
	_GameObjectInfo.ObjectHeight = 1;
}

void CStone::Start()
{
	CEnvironment::Start();
}

bool CStone::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	bool IsDead = CEnvironment::OnDamaged(Attacker, Damage);

	if (IsDead == true)
	{
		_DeadTick = 0;

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

		G_ObjectManager->ObjectItemSpawn(_Channel, Attacker->_GameObjectInfo.ObjectId,
			Attacker->_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.CollisionPosition,
			_GameObjectInfo.ObjectPositionInfo.Position,
			_GameObjectInfo.ObjectType);
	}

	return IsDead;
}

void CStone::UpdateIdle()
{
}

CTree::CTree()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_TREE;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	auto FindEnvironmentData = G_Datamanager->_Environments.find(en_GameObjectType::OBJECT_TREE);
	st_EnvironmentData EnvironmentData = *(*FindEnvironmentData).second;

	_GameObjectInfo.ObjectName = (LPWSTR)CA2W(EnvironmentData.EnvironmentName.c_str());
	_GameObjectInfo.ObjectStatInfo.MaxHP = EnvironmentData.MaxHP;
	_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
	_GameObjectInfo.ObjectStatInfo.Level = EnvironmentData.Level;

	_GameObjectInfo.ObjectWidth = 1;
	_GameObjectInfo.ObjectHeight = 1;
}

void CTree::Start()
{
	CEnvironment::Start();
}

bool CTree::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	bool IsDead = CEnvironment::OnDamaged(Attacker, Damage);

	if (IsDead == true)
	{
		_DeadTick = GetTickCount64() + 1500;

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

		CMessage* ResChangeStatePacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.State);
		G_NetworkManager->GetGameServer()->SendPacketFieldOfView(this, ResChangeStatePacket);
		ResChangeStatePacket->Free();
		
		G_ObjectManager->ObjectItemSpawn(_Channel, Attacker->_GameObjectInfo.ObjectId,
			Attacker->_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.CollisionPosition,
			_GameObjectInfo.ObjectPositionInfo.Position,
			_GameObjectInfo.ObjectType);
	}

	return IsDead;
}

void CTree::UpdateIdle()
{	
	
}