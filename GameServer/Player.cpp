#include "pch.h"
#include "Player.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "Skill.h"
#include "SkillBox.h"

CPlayer::CPlayer()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_PLAYER;	
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
	_DefaultAttackTick = 0;	
	_SkillJob = nullptr;

	_FieldOfViewDistance = 10;

	_IsSendPacketTarget = true;

	_NatureRecoveryTick = GetTickCount64() + 5000;
	_FieldOfViewUpdateTick = GetTickCount64() + 50;
}

CPlayer::~CPlayer()
{
}

void CPlayer::Update()
{	
	CGameObject::Update();

	// 시야범위 객체 조사
	if (_FieldOfViewUpdateTick < GetTickCount64())
	{	
		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetFieldOfViewObjects(this, 1);		
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
		
		// 한번 더 검사
		if (SpawnObjectIds.size() > 0)
		{
			// 스폰 해야할 대상들을 스폰
			vector<CGameObject*> SpawnObjectInfos;
			for (st_FieldOfViewInfo SpawnObject : SpawnObjectIds)
			{
				if (SpawnObject.ObjectId != 0 && SpawnObject.ObjectType != en_GameObjectType::NORMAL)
				{
					CGameObject* FindObject = G_ObjectManager->Find(SpawnObject.ObjectId, SpawnObject.ObjectType);
					if (FindObject != nullptr) 
					{
						// 시야 범위 안에 존재할 경우 스폰정보 담음
						int16 Distance = st_Vector2Int::Distance(FindObject->GetCellPosition(), GetCellPosition());
						if (Distance <= _FieldOfViewDistance)
						{
							SpawnObjectInfos.push_back(FindObject);
						}						
					}					
				}								
			}

			// 스폰해야 할 대상을 나에게 스폰하라고 알림
			if (SpawnObjectInfos.size() > 0)
			{
				CMessage* ResOtherObjectSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn((int32)SpawnObjectInfos.size(), SpawnObjectInfos);
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResOtherObjectSpawnPacket);
				ResOtherObjectSpawnPacket->Free();

				// 스폰 해야할 대상들 기준에서 나와의 거리가 스폰 해야할 대상들 시야범위안에 있을 경우 나를 스폰하라고 알려줌
				SpawnObjectInfos.clear();
				SpawnObjectInfos.push_back(this);

				CMessage* ResMyObjectSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn(1, SpawnObjectInfos);
				for (CGameObject* SpawnObject : SpawnObjectInfos)
				{
					if (SpawnObject->_IsSendPacketTarget == true)
					{
						int16 Distance = st_Vector2Int::Distance(GetCellPosition(), SpawnObject->GetCellPosition());

						if (Distance <= SpawnObject->_FieldOfViewDistance)
						{
							G_ObjectManager->GameServer->SendPacket(((CPlayer*)SpawnObject)->_SessionId, ResMyObjectSpawnPacket);
						}
					}
				}
				ResMyObjectSpawnPacket->Free();
			}						
		}

		// 한번 더 검사
		if (DeSpawnObjectIds.size() > 0)
		{
			vector<CGameObject*> DeSpawnObjectInfos;
			for (st_FieldOfViewInfo DeSpawnObject : DeSpawnObjectIds)
			{
				if (DeSpawnObject.ObjectId != 0 && DeSpawnObject.ObjectType != en_GameObjectType::NORMAL)
				{
					CGameObject* FindObject = G_ObjectManager->Find(DeSpawnObject.ObjectId, DeSpawnObject.ObjectType);

					if (FindObject != nullptr && FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
					{
						DeSpawnObjectInfos.push_back(FindObject);
					}
				}				
			}

			// 디스폰해야 할 대상을 나에게 스폰 해제하라고 알림
			if (DeSpawnObjectInfos.size() > 0)
			{
				CMessage* ResOtherObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn((int32)DeSpawnObjectInfos.size(), DeSpawnObjectInfos);
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResOtherObjectDeSpawnPacket);
				ResOtherObjectDeSpawnPacket->Free();

				// 하지만 나와 대상들의 거리가 대상들 시야범위 안에 속할 경우 나를 디스폰하지는 않는다.
				DeSpawnObjectInfos.clear();
				DeSpawnObjectInfos.push_back(this);

				CMessage* ResMyObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(1, DeSpawnObjectInfos);
				for (CGameObject* DeSpawnObject : DeSpawnObjectInfos)
				{
					if (DeSpawnObject->_IsSendPacketTarget)
					{
						int16 Distance = st_Vector2Int::Distance(GetCellPosition(), DeSpawnObject->GetCellPosition());
						if (Distance > DeSpawnObject->_FieldOfViewDistance)
						{							
							G_ObjectManager->GameServer->SendPacket(((CPlayer*)DeSpawnObject)->_SessionId, ResMyObjectDeSpawnPacket);
						}
					}
				}
				ResMyObjectDeSpawnPacket->Free();
			}			
		}

		_FieldOfViewInfos = CurrentFieldOfViewObjectIds;				
	}			

	// 스킬목록 업데이트
	_SkillBox.Update();

	// 강화효과 스킬 리스트 순회
	for (auto BufSkillIterator : _Bufs)
	{
		// 지속시간 끝난 강화효과 삭제
		bool DeleteBufSkill = BufSkillIterator.second->Update();
		if (DeleteBufSkill)
		{
			DeleteBuf(BufSkillIterator.first);
			// 강화효과 스킬 정보 메모리 반납
			G_ObjectManager->SkillInfoReturn(BufSkillIterator.second->GetSkillInfo()->SkillMediumCategory, BufSkillIterator.second->GetSkillInfo());
			// 강화효과 스킬 메모리 반납
			G_ObjectManager->SkillReturn(BufSkillIterator.second);
		}
	}

	// 약화효과 스킬 리스트 순회
	for (auto DebufSkillIterator : _DeBufs)
	{
		// 지속시간 끝난 약화효과 삭제
		bool DeleteDebufSkill = DebufSkillIterator.second->Update();
		if (DeleteDebufSkill)
		{
			DeleteDebuf(DebufSkillIterator.first);
			// 약화효과 스킬 정보 메모리 반납
			G_ObjectManager->SkillInfoReturn(DebufSkillIterator.second->GetSkillInfo()->SkillMediumCategory, DebufSkillIterator.second->GetSkillInfo());
			// 약화효과 스킬 메모리 반납
			G_ObjectManager->SkillReturn(DebufSkillIterator.second);
		}		
	}

	if (_NatureRecoveryTick < GetTickCount64())
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

		if (_GameObjectInfo.ObjectStatInfo.HP < 0)
		{
			_GameObjectInfo.ObjectStatInfo.HP = 0;
		}

		_NatureRecoveryTick = GetTickCount64() + 5000;		
				
		CMessage* ResObjectStatPacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId,			
			_GameObjectInfo.ObjectStatInfo);
		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStatPacket, this);		
		ResObjectStatPacket->Free();
	}
	
	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::MOVING:
		UpdateMove();
		break;
	case en_CreatureState::ATTACK:
		UpdateAttack();
		break;
	case en_CreatureState::SPELL:
		UpdateSpell();
		break;
	}
}

bool CPlayer::OnDamaged(CGameObject* Attacker, int32 Damage)
{
	return CGameObject::OnDamaged(Attacker, Damage);
}

void CPlayer::OnDead(CGameObject* Killer)
{

}

void CPlayer::Init()
{
	// 스킬 목록 정리
	_SkillBox.Empty();
	// 퀵슬롯 정리
	_QuickSlotManager.Empty();
	
	// 남아 있는 플레이어 잡 큐 처리
	CGameObject::Update();

	_GameObjectInfo.ObjectId = 0;
	_GameObjectInfo.ObjectName = L"";	
}

void CPlayer::PositionReset()
{
	switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
	{
	case en_MoveDir::LEFT:
		_GameObjectInfo.ObjectPositionInfo.PositionX =
			_GameObjectInfo.ObjectPositionInfo.CollisionPositionX + 0.3f;
		break;
	case en_MoveDir::RIGHT:
		_GameObjectInfo.ObjectPositionInfo.PositionX =
			_GameObjectInfo.ObjectPositionInfo.CollisionPositionX + 0.7f;
		break;
	case en_MoveDir::UP:
		_GameObjectInfo.ObjectPositionInfo.PositionY =
			_GameObjectInfo.ObjectPositionInfo.CollisionPositionY + 0.7f;
		break;
	case en_MoveDir::DOWN:
		_GameObjectInfo.ObjectPositionInfo.PositionY =
			_GameObjectInfo.ObjectPositionInfo.CollisionPositionY + 0.3f;
		break;	
	}	
}

void CPlayer::UpdateMove()
{	
	switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
	{
	case en_MoveDir::UP:		
		_GameObjectInfo.ObjectPositionInfo.PositionY +=
			(st_Vector2::Up()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::DOWN:		
		_GameObjectInfo.ObjectPositionInfo.PositionY +=
			(st_Vector2::Down()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::LEFT:		
		_GameObjectInfo.ObjectPositionInfo.PositionX += 
			(st_Vector2::Left()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	case en_MoveDir::RIGHT:		
		_GameObjectInfo.ObjectPositionInfo.PositionX += 
			(st_Vector2::Right()._X * _GameObjectInfo.ObjectStatInfo.Speed * 0.02f);
		break;
	}	
		
	bool CanMove = _Channel->_Map->Cango(this, _GameObjectInfo.ObjectPositionInfo.PositionX, _GameObjectInfo.ObjectPositionInfo.PositionY);
	if (CanMove == true)
	{
		st_Vector2Int CollisionPosition;
		CollisionPosition._X = _GameObjectInfo.ObjectPositionInfo.PositionX;
		CollisionPosition._Y = _GameObjectInfo.ObjectPositionInfo.PositionY;
		
		if (CollisionPosition._X != _GameObjectInfo.ObjectPositionInfo.CollisionPositionX
			|| CollisionPosition._Y != _GameObjectInfo.ObjectPositionInfo.CollisionPositionY)
		{
			_Channel->_Map->ApplyMove(this, CollisionPosition);					
		}				
	}
	else
	{
		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

		PositionReset();

		// 바뀐 좌표 값 시야범위 오브젝트들에게 전송
		CMessage* ResMovePacket = G_ObjectManager->GameServer->MakePacketResMove(_AccountId,
			_GameObjectInfo.ObjectId,
			CanMove,
			_GameObjectInfo.ObjectPositionInfo);

		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMovePacket, this);
		ResMovePacket->Free();
	}		
}

void CPlayer::UpdateAttack()
{
	if (_DefaultAttackTick < GetTickCount64())
	{
		_DefaultAttackTick = GetTickCount64() + 800;		

		st_Vector2Int FrontCell = GetFrontCellPosition(_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);
		CGameObject* Target = _Channel->_Map->Find(FrontCell);

		if (Target != nullptr && Target->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::SPAWN_IDLE)
		{
			st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_SkillBox.FindSkill(en_SkillType::SKILL_DEFAULT_ATTACK)->GetSkillInfo();

			random_device Seed;
			default_random_engine Eng(Seed());

			// 크리티컬 판단
			float CriticalPoint = _GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint / 1000.0f;
			bernoulli_distribution CriticalCheck(CriticalPoint);
			bool IsCritical = CriticalCheck(Eng);

			// 데미지 판단
			mt19937 Gen(Seed());
			uniform_int_distribution<int> DamageChoiceRandom(
				_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage + _Equipment._WeaponMinDamage + AttackSkillInfo->SkillMinDamage,
				_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage + _Equipment._WeaponMaxDamage + AttackSkillInfo->SkillMaxDamage);
			int32 ChoiceDamage = DamageChoiceRandom(Gen);

			int32 CriticalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

			float DefenceRate = (float)pow(((float)(200 - Target->_GameObjectInfo.ObjectStatInfo.Defence)) / 20, 2) * 0.01f;

			int32 FinalDamage = CriticalDamage * DefenceRate;

			bool TargetIsDead = Target->OnDamaged(this, FinalDamage);
			if (TargetIsDead == true)
			{
				//ExperienceCalculate(MyPlayer, Target);
			}

			en_EffectType HitEffectType;

			wstring SkillTypeString;
			wchar_t SkillTypeMessage[64] = L"0";
			wchar_t SkillDamageMessage[64] = L"0";

			// 시스템 메세지 생성
			wsprintf(SkillTypeMessage, L"%s가 일반공격을 사용해 %s에게 %d의 데미지를 줬습니다.", _GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
			HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;

			SkillTypeString = SkillTypeMessage;
			SkillTypeString = IsCritical ? L"치명타! " + SkillTypeString : SkillTypeString;

			// 데미지 시스템 메세지 전송
			CMessage* ResSkillSystemMessagePacket = G_ObjectManager->GameServer->MakePacketResChattingBoxMessage(_GameObjectInfo.ObjectId,
				en_MessageType::SYSTEM,
				IsCritical ? st_Color::Red() : st_Color::White(),
				SkillTypeString);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResSkillSystemMessagePacket, this);
			ResSkillSystemMessagePacket->Free();

			// 공격 응답 메세지 전송
			CMessage* ResMyAttackOtherPacket = G_ObjectManager->GameServer->MakePacketResAttack(_GameObjectInfo.ObjectId,
				Target->_GameObjectInfo.ObjectId,				
				_GameObjectInfo.ObjectPositionInfo.MoveDir,
				en_SkillType::SKILL_DEFAULT_ATTACK,
				FinalDamage,
				IsCritical);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMyAttackOtherPacket, this);
			ResMyAttackOtherPacket->Free();
			
			// 스탯 변경 메세지 전송
			CMessage* ResChangeObjectStat = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(Target->_GameObjectInfo.ObjectId,
				Target->_GameObjectInfo.ObjectStatInfo);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResChangeObjectStat, this);
			ResChangeObjectStat->Free();

			CSkill* DefaultAttackSkill = _SkillBox.FindSkill(en_SkillType::SKILL_DEFAULT_ATTACK);
			DefaultAttackSkill->CoolTimeStart();
						
			CMessage* ResCoolTimeStartPacket = G_ObjectManager->GameServer->MakePacketCoolTime(0,
				0,
				1.0f, DefaultAttackSkill);
			G_ObjectManager->GameServer->SendPacket(this->_SessionId, ResCoolTimeStartPacket);
			ResCoolTimeStartPacket->Free();
		}
		else
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

			CMessage* ResChageObjectStatePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
				_GameObjectInfo.ObjectPositionInfo.MoveDir,
				_GameObjectInfo.ObjectType,
				_GameObjectInfo.ObjectPositionInfo.State);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResChageObjectStatePacket, this);
			ResChageObjectStatePacket->Free();
		}
	}	
}

void CPlayer::UpdateSpell()
{	

}