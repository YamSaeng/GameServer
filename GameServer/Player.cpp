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

	// 주위 시야 오브젝트 점검
	CheckFieldOfViewObject();	

	// 스킬목록 업데이트
	_SkillBox.Update();
	
	// 버프, 디버프 업데이트
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

	// 스킬 목록 정리
	_SkillBox.Empty();
	// 퀵슬롯 정리
	_QuickSlotManager.Empty();

	// 남아 있는 플레이어 잡 큐 처리
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
			CRASH("채집할 대상이 없는데 채집 요청함");
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

		// 채집창 끝
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
	// 시야범위 객체 조사
	if (_FieldOfViewUpdateTick < GetTickCount64() && _Channel != nullptr)
	{
		_FieldOfViewUpdateTick = GetTickCount64() + 100;

		// 시야범위 오브젝트를 조사해서 저장		
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

		// 한번 더 검사
		if (SpawnObjectIds.size() > 0)
		{
			// 스폰 해야할 대상들을 스폰			
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

		// 한번 더 검사
		if (DeSpawnObjectIds.size() > 0)
		{
			for (st_FieldOfViewInfo DeSpawnObject : DeSpawnObjectIds)
			{
				if (DeSpawnObject.ObjectID != 0 && DeSpawnObject.ObjectType != en_GameObjectType::OBJECT_NON_TYPE)
				{
					CGameObject* FindObject = _Channel->FindChannelObject(DeSpawnObject.ObjectID, DeSpawnObject.ObjectType);

					// 추가적으로 검사
					// 소환해제 해야할 대상이 죽음 준비 상태 또는 죽음 상태일 경우에는 알아서 소환해제 되기 때문에
					// 죽음 준비 상태 그리고 죽음 상태가 아닐 경우에만 소환해제 하도록 설정					
					
					// 하지만 플레이어의 시야를 대상이 벗어난다면 바로 디스폰 해줘야함
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
			// 스폰해야 할 대상들을 나에게 스폰하라고 알림
			CMessage* ResOtherObjectSpawnPacket = G_NetworkManager->GetGameServer()->MakePacketResObjectSpawn((int32)SpawnObjectInfos.size(), SpawnObjectInfos);
			G_NetworkManager->GetGameServer()->SendPacket(_SessionId, ResOtherObjectSpawnPacket);
			ResOtherObjectSpawnPacket->Free();
		}

		// 소환해제해야 할 대상을 나에게 스폰 해제하라고 알림
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

		// 레이캐스팅 검사할때 움직일 단위 x, y 값
		Vector2 RayUnitStepSize;
		RayUnitStepSize.X = sqrt(1 + (FieldOfViewRay.Y / FieldOfViewRay.X) * (FieldOfViewRay.Y / FieldOfViewRay.X));
		RayUnitStepSize.Y = sqrt(1 + (FieldOfViewRay.X / FieldOfViewRay.Y) * (FieldOfViewRay.X / FieldOfViewRay.Y));

		// 맵 좌표 위치 
		Vector2Int MapCheck;
		MapCheck.X = _GameObjectInfo.ObjectPositionInfo.Position.X;
		MapCheck.Y = _GameObjectInfo.ObjectPositionInfo.Position.Y;

		// 현재 위치에서 다음 위치의 Ray 길이
		Vector2 RayLength1D;

		// 탐색 방향
		Vector2Int Step;

		// 탐색 방향 정하고 다음 위치 Ray 길이 값 정하기
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

		// 검사
		bool WallFound = false;
		bool TargetFound = false;
		float MaxDistance = _FieldOfViewDistance;
		float Distance = 0.0f;
		// 벽을 찾거나 목표물을 찾거나 최대 광선 거리에 도착하면 나옴
		while (!WallFound && !TargetFound && Distance < MaxDistance)
		{
			// 다음 타일 위치로 옮김
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

			// 맵에서 계산한 타일 위치 조사
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

		// 목표물을 먼저 찾은 경우
		if (TargetFound == true)
		{
			if (FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD
				&& FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_READY)
			{
				SpawnObjects->push_back(FieldOfViewObject);
			}			
		}

		// 벽과 같은 시야에 가려지는 물체를 먼저 찾은 경우
		if (WallFound == true)
		{
			DespawnObjects->push_back(FieldOfViewObject);
		}		
	}	
}