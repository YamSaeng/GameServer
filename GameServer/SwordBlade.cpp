#include "pch.h"
#include "SwordBlade.h"
#include "RectCollision.h"
#include "NetworkManager.h"

CSwordBlade::CSwordBlade()
{
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	_GameObjectInfo.ObjectName = L"Ä®³¯";

	_GameObjectInfo.ObjectStatInfo.Speed = 10.0f;

	_FieldOfViewDistance = 10.0f;
}

CSwordBlade::~CSwordBlade()
{
}

void CSwordBlade::Update()
{
	if (_RectCollision != nullptr)
	{
		_RectCollision->Update();
	}	

	Move();
}

void CSwordBlade::Move()
{	
	Vector2Int CollisionPosition;
	CollisionPosition.X = (int32)_GameObjectInfo.ObjectPositionInfo.Position.X;
	CollisionPosition.Y = (int32)_GameObjectInfo.ObjectPositionInfo.Position.Y;

	Vector2 DirectionNormal = _GameObjectInfo.ObjectPositionInfo.MoveDirection.Normalize();

	CGameObject* CollisionObject = nullptr;
	Vector2 NextPosition;	

	bool IsCollision = _Channel->GetMap()->CanMoveSkillGo(this, &NextPosition, &CollisionObject);
	if (IsCollision == true)
	{
		_GameObjectInfo.ObjectPositionInfo.Position = NextPosition;
	}
	else
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

		if (CollisionObject != nullptr)
		{			
			st_GameObjectJob* DamageJob = G_NetworkManager->GetGameServer()->MakeGameObjectDamage(_Owner->_GameObjectInfo.ObjectId,
				_Owner->_GameObjectInfo.ObjectType,
				en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FLY_KNIFE,
				_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage,
				_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage);
			CollisionObject->_GameObjectJobQue.Enqueue(DamageJob);			
		}

		st_GameObjectJob* LeaveChannelJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobLeaveChannel(this);
		_Channel->_ChannelJobQue.Enqueue(LeaveChannelJob);

		vector<st_FieldOfViewInfo> AroundPlayers = _Channel->GetMap()->GetFieldAroundPlayers(this);

		CMessage* ResOtherObjectDeSpawnPacket = G_NetworkManager->GetGameServer()->MakePacketResObjectDeSpawn(_GameObjectInfo.ObjectId);
		G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, ResOtherObjectDeSpawnPacket);
		ResOtherObjectDeSpawnPacket->Free();
	}
}
