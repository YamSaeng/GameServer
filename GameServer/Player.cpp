#include "pch.h"
#include "Player.h"
#include "ObjectManager.h"
#include "NetworkManager.h"
#include "DataManager.h"
#include "Skill.h"
#include "SkillBox.h"
#include "MapManager.h"
#include "FlameBolt.h"
#include "DivineBolt.h"
#include "RectCollision.h"

CPlayer::CPlayer()
{
	_AccountId = 0;
	_SessionId = 0;
		
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
	_DefaultAttackTick = 0;

	_FieldOfDirection = Vector2::Left;

	_FieldOfViewDistance = 12;		

	_FieldOfAngle = 210;

	_NatureRecoveryTick = GetTickCount64() + 5000;
	_FieldOfViewUpdateTick = GetTickCount64() + 100;

	_GameObjectInfo.ObjectWidth = 1;
	_GameObjectInfo.ObjectHeight = 1;	

	_CastingSkill = nullptr;			

	_SkillBox.SetOwner(this);
}

CPlayer::~CPlayer()
{
	
}

void CPlayer::Update()
{
	CGameObject::Update();

	if (_NetworkState == en_ObjectNetworkState::OBJECT_NETWORK_STATE_LEAVE)
	{
		return;
	}	

	// ���� �þ� ������Ʈ ����
	CheckFieldOfViewObject();	

	// ��ų��� ������Ʈ
	_SkillBox.Update();
	
	// ����, ����� ������Ʈ
	CheckBufDeBufSkill();	

	if (_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
	{
		if (_NatureRecoveryTick < GetTickCount64())
		{
			if (_GameObjectInfo.ObjectStatInfo.HP != _GameObjectInfo.ObjectStatInfo.MaxHP
				|| _GameObjectInfo.ObjectStatInfo.MP != _GameObjectInfo.ObjectStatInfo.MaxMP)
			{
				int32 AutoHPRecoveryPoint = 0;
				int32 AutoMPRecoveryPoint = 0;

				AutoHPRecoveryPoint = (_GameObjectInfo.ObjectStatInfo.MaxHP / 100) * _GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent;
				AutoMPRecoveryPoint = (_GameObjectInfo.ObjectStatInfo.MaxMP / 100) * _GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent;

				_GameObjectInfo.ObjectStatInfo.HP += AutoHPRecoveryPoint;
				_GameObjectInfo.ObjectStatInfo.MP += AutoMPRecoveryPoint;

				if (_GameObjectInfo.ObjectStatInfo.HP > _GameObjectInfo.ObjectStatInfo.MaxHP)
				{
					_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
				}

				if (_GameObjectInfo.ObjectStatInfo.MP > _GameObjectInfo.ObjectStatInfo.MaxMP)
				{
					_GameObjectInfo.ObjectStatInfo.MP = _GameObjectInfo.ObjectStatInfo.MaxMP;
				}

				_NatureRecoveryTick = GetTickCount64() + 5000;

				vector<st_FieldOfViewInfo> AroundPlayers = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

				CMessage* ResObjectStatPacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId,
					_GameObjectInfo.ObjectStatInfo);
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, ResObjectStatPacket);
				ResObjectStatPacket->Free();
			}						
		}
	}		

	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::SPAWN_IDLE:
		UpdateSpawnIdle();
		break;
	case en_CreatureState::IDLE:
		UpdateIdle();
		break;
	case en_CreatureState::MOVING:
		UpdateMoving();		
		break;	
	case en_CreatureState::ATTACK:
		UpdateAttack();
		break;
	case en_CreatureState::SPELL:
		UpdateSpell();
		break;
	case en_CreatureState::GATHERING:
		UpdateGathering();
		break;	
	case en_CreatureState::DEAD:
		UpdateDead();
		break;
	}	

	/*G_Logger->WriteStdOut(en_Color::RED, L"Dir X %0.1f Y %0.1f PositionX %0.1f PositionY %0.1f\n", _GameObjectInfo.ObjectPositionInfo.Direction._X,
		_GameObjectInfo.ObjectPositionInfo.Direction._Y, _GameObjectInfo.ObjectPositionInfo.Position._X, _GameObjectInfo.ObjectPositionInfo.Position._Y);*/	
}

bool CPlayer::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	if (_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE
		|| _GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::RETURN_SPAWN_POSITION)
	{
		CGameObject::OnDamaged(Attacker, Damage);

		if (_GameObjectInfo.ObjectStatInfo.HP == 0)
		{
			_RectCollision->SetActive(false);

			_DeadTick = GetTickCount64() + 1000;

			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::DEAD;

			_GameObjectInfo.ObjectPositionInfo.MoveDirection = Vector2::Zero;			

			vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

			CGameServerMessage* ResDeadStateChangePacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId, 
				_GameObjectInfo.ObjectPositionInfo.State);
			G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResDeadStateChangePacket);
			ResDeadStateChangePacket->Free();

			return true;
		}
	}

	return false;
}

void CPlayer::Start()
{
	
}

void CPlayer::End()
{
	CGameObject::End();

	_FieldOfViewInfos.clear();

	// ��ų ��� ����
	_SkillBox.Empty();
	// ������ ����
	_QuickSlotManager.Empty();

	// ���� �ִ� �÷��̾� �� ť ó��
	CGameObject::Update();
	
	_GameObjectInfo.ObjectName = L"";	
}

bool CPlayer::UpdateSpawnIdle()
{
	bool ChangeToIdle = CGameObject::UpdateSpawnIdle();

	if (ChangeToIdle)
	{
		CGameServerMessage* ChangeToIdlePacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,						
			_GameObjectInfo.ObjectPositionInfo.State);
		G_NetworkManager->GetGameServer()->SendPacketFieldOfView(this, ChangeToIdlePacket);
		ChangeToIdlePacket->Free();
	}

	return ChangeToIdle;	
}

void CPlayer::UpdateIdle()
{

}

void CPlayer::UpdateMoving()
{	
	bool CanMove = _Channel->GetMap()->Cango(this);
	if (CanMove == true)
	{
		Vector2Int CollisionPosition;
		CollisionPosition.X = (int32)_GameObjectInfo.ObjectPositionInfo.Position.X;
		CollisionPosition.Y = (int32)_GameObjectInfo.ObjectPositionInfo.Position.Y;

		if (CollisionPosition.X != _GameObjectInfo.ObjectPositionInfo.CollisionPosition.X
			|| CollisionPosition.Y != _GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y)
		{
			_Channel->GetMap()->ApplyMove(this, CollisionPosition);
		}
	}
	else
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;		

		vector<st_FieldOfViewInfo> AroundPlayers = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

		CMessage* ResMoveStopPacket = G_NetworkManager->GetGameServer()->MakePacketResMoveStop(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectPositionInfo.Position.X,
			_GameObjectInfo.ObjectPositionInfo.Position.Y);			
		G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, ResMoveStopPacket);
		ResMoveStopPacket->Free();
	}
}

void CPlayer::UpdateAttack()
{
	
}

void CPlayer::UpdateSpell()
{
	if (_SpellTick < GetTickCount64())
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

		vector<st_FieldOfViewInfo> AroundPlayers = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

		CMessage* ResObjectStateChangePacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,			
			_GameObjectInfo.ObjectPositionInfo.State);
		G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, ResObjectStateChangePacket);
		ResObjectStateChangePacket->Free();

		if (_CastingSkill != nullptr)
		{
			_SkillBox.SkillProcess(this, _CastingSkill);	

			_CastingSkill = nullptr;
		}
	}
}

void CPlayer::UpdateGathering()
{
	if (_GatheringTick < GetTickCount64())
	{
		if (_GatheringTarget == nullptr)
		{
			CRASH("ä���� ����� ���µ� ä�� ��û��");
			return;
		}		

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

		_GatheringTarget->OnDamaged(this, 1);		

		CMessage* ResObjectStateChangePacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,						
			_GameObjectInfo.ObjectPositionInfo.State);
		G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStateChangePacket);

		CMessage* ResObjectStatChangePacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectStat(_GatheringTarget->_GameObjectInfo.ObjectId,
			_GatheringTarget->_GameObjectInfo.ObjectStatInfo);
		G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStatChangePacket);
		ResObjectStatChangePacket->Free();

		CMessage* ResGatheringDamagePacket = G_NetworkManager->GetGameServer()->MakePacketResGatheringDamage(_GatheringTarget->_GameObjectInfo.ObjectId);
		G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_FieldOfViewInfos, ResGatheringDamagePacket);
		ResGatheringDamagePacket->Free();

		// ä��â ��
		CMessage* ResGatheringPacket = G_NetworkManager->GetGameServer()->MakePacketResGathering(_GameObjectInfo.ObjectId, false, L"");
		G_NetworkManager->GetGameServer()->SendPacketFieldOfView(_FieldOfViewInfos, ResGatheringPacket);
		ResGatheringPacket->Free();		
	}
}

void CPlayer::UpdateDead()
{
	if (_DeadTick < GetTickCount64())
	{
		st_GameObjectJob* DeSpawnPlayerJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobObjectDeSpawnObjectChannel(this);
		_Channel->_ChannelJobQue.Enqueue(DeSpawnPlayerJob);
	}
}

void CPlayer::CheckFieldOfViewObject()
{
	// �þ߹��� ��ü ����
	if (_FieldOfViewUpdateTick < GetTickCount64() && _Channel != nullptr)
	{
		_FieldOfViewUpdateTick = GetTickCount64() + 100;

		// �þ߹��� ������Ʈ�� �����ؼ� ����		
		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldOfViewObjects(this);
		vector<st_FieldOfViewInfo> SpawnObjectIds;
		vector<st_FieldOfViewInfo> DeSpawnObjectIds;
				
		if (CurrentFieldOfViewObjectIds.size() > 1)
		{
			sort(CurrentFieldOfViewObjectIds.begin(), CurrentFieldOfViewObjectIds.end());
		}

		if (_FieldOfViewInfos.size() > 1)
		{
			sort(_FieldOfViewInfos.begin(), _FieldOfViewInfos.end());
		}

		SpawnObjectIds.resize(CurrentFieldOfViewObjectIds.size());

		set_difference(CurrentFieldOfViewObjectIds.begin(), CurrentFieldOfViewObjectIds.end(),
			_FieldOfViewInfos.begin(), _FieldOfViewInfos.end(),
			SpawnObjectIds.begin());

		DeSpawnObjectIds.resize(_FieldOfViewInfos.size());

		set_difference(_FieldOfViewInfos.begin(), _FieldOfViewInfos.end(),
			CurrentFieldOfViewObjectIds.begin(), CurrentFieldOfViewObjectIds.end(),
			DeSpawnObjectIds.begin());

		vector<CGameObject*> SpawnObjectInfos;
		vector<CGameObject*> DeSpawnObjectInfos;

		// �ѹ� �� �˻�
		if (SpawnObjectIds.size() > 0)
		{
			// ���� �ؾ��� ������ ����			
			for (st_FieldOfViewInfo SpawnObject : SpawnObjectIds)
			{
				if (SpawnObject.ObjectID != 0 && SpawnObject.ObjectType != en_GameObjectType::OBJECT_NON_TYPE)
				{
					CGameObject* FindObject = _Channel->FindChannelObject(SpawnObject.ObjectID, SpawnObject.ObjectType);
					if (FindObject != nullptr
						&& FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD
						&& FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_READY)
					{
						SpawnObjectInfos.push_back(FindObject);
					}
				}
			}			
		}

		// �ѹ� �� �˻�
		if (DeSpawnObjectIds.size() > 0)
		{
			for (st_FieldOfViewInfo DeSpawnObject : DeSpawnObjectIds)
			{
				if (DeSpawnObject.ObjectID != 0 && DeSpawnObject.ObjectType != en_GameObjectType::OBJECT_NON_TYPE)
				{
					CGameObject* FindObject = _Channel->FindChannelObject(DeSpawnObject.ObjectID, DeSpawnObject.ObjectType);

					// �߰������� �˻�
					// ��ȯ���� �ؾ��� ����� ���� �غ� ���� �Ǵ� ���� ������ ��쿡�� �˾Ƽ� ��ȯ���� �Ǳ� ������
					// ���� �غ� ���� �׸��� ���� ���°� �ƴ� ��쿡�� ��ȯ���� �ϵ��� ����					
					
					// ������ �÷��̾��� �þ߸� ����� ����ٸ� �ٷ� ���� �������
					if (FindObject != nullptr						
						&& FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD
						&& FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_READY)
					{
						if (_SelectTarget != nullptr && FindObject->_GameObjectInfo.ObjectId == _SelectTarget->_GameObjectInfo.ObjectId)
						{
							_SelectTarget = nullptr;
						}

						DeSpawnObjectInfos.push_back(FindObject);
					}
				}
			}			
		}

		_FieldOfViewInfos = CurrentFieldOfViewObjectIds;

		_FieldOfViewObjects.clear();

		for (st_FieldOfViewInfo FieldOfViewInfo : _FieldOfViewInfos)
		{
			CGameObject* FindObject = _Channel->FindChannelObject(FieldOfViewInfo.ObjectID, FieldOfViewInfo.ObjectType);
			if (FindObject != nullptr)
			{
				_FieldOfViewObjects.push_back(FindObject);
			}			
		}

		RayCastingToFieldOfViewObjects(&SpawnObjectInfos, &DeSpawnObjectInfos);

		if (SpawnObjectInfos.size() > 0)
		{
			// �����ؾ� �� ������ ������ �����϶�� �˸�
			CMessage* ResOtherObjectSpawnPacket = G_NetworkManager->GetGameServer()->MakePacketResObjectSpawn((int32)SpawnObjectInfos.size(), SpawnObjectInfos);
			G_NetworkManager->GetGameServer()->SendPacket(_SessionId, ResOtherObjectSpawnPacket);
			ResOtherObjectSpawnPacket->Free();
		}

		// ��ȯ�����ؾ� �� ����� ������ ���� �����϶�� �˸�
		if (DeSpawnObjectInfos.size() > 0)
		{
			CMessage* ResOtherObjectDeSpawnPacket = G_NetworkManager->GetGameServer()->MakePacketResObjectDeSpawn((int32)DeSpawnObjectInfos.size(), DeSpawnObjectInfos);
			G_NetworkManager->GetGameServer()->SendPacket(_SessionId, ResOtherObjectDeSpawnPacket);
			ResOtherObjectDeSpawnPacket->Free();
		}
	}	
}

void CPlayer::RayCastingToFieldOfViewObjects(vector<CGameObject*>* SpawnObjects, vector<CGameObject*>* DespawnObjects)
{
	for (CGameObject* FieldOfViewObject : _FieldOfViewObjects)
	{
		Vector2 FieldOfViewObjectDir = FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.Position - _GameObjectInfo.ObjectPositionInfo.Position;
		Vector2 FieldOfViewRay = FieldOfViewObjectDir.Normalize();

		// ����ĳ���� �˻��Ҷ� ������ ���� x, y ��
		Vector2 RayUnitStepSize;
		RayUnitStepSize.X = sqrt(1 + (FieldOfViewRay.Y / FieldOfViewRay.X) * (FieldOfViewRay.Y / FieldOfViewRay.X));
		RayUnitStepSize.Y = sqrt(1 + (FieldOfViewRay.X / FieldOfViewRay.Y) * (FieldOfViewRay.X / FieldOfViewRay.Y));

		// �� ��ǥ ��ġ 
		Vector2Int MapCheck;
		MapCheck.X = _GameObjectInfo.ObjectPositionInfo.Position.X;
		MapCheck.Y = _GameObjectInfo.ObjectPositionInfo.Position.Y;

		// ���� ��ġ���� ���� ��ġ�� Ray ����
		Vector2 RayLength1D;

		// Ž�� ����
		Vector2Int Step;

		// Ž�� ���� ���ϰ� ���� ��ġ Ray ���� �� ���ϱ�
		if (FieldOfViewRay.X < 0)
		{
			Step.X = -1;
			RayLength1D.X = (_GameObjectInfo.ObjectPositionInfo.Position.X - float(MapCheck.X)) * RayUnitStepSize.X;
		}
		else
		{
			Step.X = 1;
			RayLength1D.X = (float(MapCheck.X + 1) - _GameObjectInfo.ObjectPositionInfo.Position.X) * RayUnitStepSize.X;
		}

		if (FieldOfViewRay.Y < 0)
		{
			Step.Y = -1;
			RayLength1D.Y = (_GameObjectInfo.ObjectPositionInfo.Position.Y - float(MapCheck.Y)) * RayUnitStepSize.Y;
		}
		else
		{
			Step.Y = 1;
			RayLength1D.Y = (float(MapCheck.Y + 1) - _GameObjectInfo.ObjectPositionInfo.Position.Y) * RayUnitStepSize.Y;
		}

		// �˻�
		bool WallFound = false;
		bool TargetFound = false;
		float MaxDistance = _FieldOfViewDistance;
		float Distance = 0.0f;
		// ���� ã�ų� ��ǥ���� ã�ų� �ִ� ���� �Ÿ��� �����ϸ� ����
		while (!WallFound && !TargetFound && Distance < MaxDistance)
		{
			// ���� Ÿ�� ��ġ�� �ű�
			if (RayLength1D.X < RayLength1D.Y)
			{
				MapCheck.X += Step.X;
				Distance = RayLength1D.X;
				RayLength1D.X += RayUnitStepSize.X;
			}
			else
			{
				MapCheck.Y += Step.Y;
				Distance = RayLength1D.Y;
				RayLength1D.Y += RayUnitStepSize.Y;
			}

			// �ʿ��� ����� Ÿ�� ��ġ ����
			CMap* Map = _Channel->GetMap();
			if (MapCheck.X >= 0 && MapCheck.X < Map->_Right && MapCheck.Y >= 0 && MapCheck.Y < Map->_Down)
			{
				int X = MapCheck.X - Map->_Left;
				int Y = Map->_Down - MapCheck.Y;

				CGameObject* GameObject = Map->_ObjectsInfos[Y][X];
				if (GameObject != nullptr)
				{
					if (GameObject->_GameObjectInfo.ObjectId == FieldOfViewObject->_GameObjectInfo.ObjectId)
					{
						TargetFound = true;
					}
					else
					{
						switch (GameObject->_GameObjectInfo.ObjectType)
						{
						case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
						case en_GameObjectType::OBJECT_WALL:
							WallFound = true;
							break;
						}
					}
				}
			}
		}

		// ��ǥ���� ���� ã�� ���
		if (TargetFound == true)
		{
			if (FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD
				&& FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_READY)
			{
				SpawnObjects->push_back(FieldOfViewObject);
			}			
		}

		// ���� ���� �þ߿� �������� ��ü�� ���� ã�� ���
		if (WallFound == true)
		{
			DespawnObjects->push_back(FieldOfViewObject);
		}		
	}	
}