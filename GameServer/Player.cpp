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

	_FieldOfViewDistance = 10;

	_IsSendPacketTarget = true;

	_NatureRecoveryTick = GetTickCount64() + 5000;
	_FieldOfViewUpdateTick = GetTickCount64() + 50;

	_ComboSkill = nullptr;
	_CurrentSkill = nullptr;

	_IsReqAttack = false;
	_IsReqMagic = false;

	_ReqMeleeSkillInit = nullptr;
	_ReqMagicSkillInit = nullptr;
}

CPlayer::~CPlayer()
{

}

void CPlayer::Update()
{
	CGameObject::Update();

	if (_NetworkState == en_ObjectNetworkState::LEAVE)
	{
		return;
	}

	// 시야범위 객체 조사
	if (_FieldOfViewUpdateTick < GetTickCount64() && _Channel != nullptr)
	{
		vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldOfViewObjects(this, 1, false);
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
				if (SpawnObject.ObjectID != 0 && SpawnObject.ObjectType != en_GameObjectType::NORMAL)
				{
					CGameObject* FindObject = _Channel->FindChannelObject(SpawnObject.ObjectID, SpawnObject.ObjectType);
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
			}
		}

		// 한번 더 검사
		if (DeSpawnObjectIds.size() > 0)
		{
			vector<CGameObject*> DeSpawnObjectInfos;
			for (st_FieldOfViewInfo DeSpawnObject : DeSpawnObjectIds)
			{
				if (DeSpawnObject.ObjectID != 0 && DeSpawnObject.ObjectType != en_GameObjectType::NORMAL)
				{
					CGameObject* FindObject = _Channel->FindChannelObject(DeSpawnObject.ObjectID, DeSpawnObject.ObjectType);

					if (FindObject != nullptr && FindObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
					{
						if (_SelectTarget != nullptr && FindObject->_GameObjectInfo.ObjectId == _SelectTarget->_GameObjectInfo.ObjectId)
						{
							_SelectTarget = nullptr;
						}

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
			}
		}

		_FieldOfViewInfos = CurrentFieldOfViewObjectIds;
	}

	// 스킬목록 업데이트
	_SkillBox.Update();

	if (_ComboSkill != nullptr)
	{
		bool ReturnComboSkill = _ComboSkill->Update();
		if (ReturnComboSkill)
		{
			G_ObjectManager->SkillReturn(_ComboSkill);
			_ComboSkill = nullptr;
		}
	}

	if (_ReqMeleeSkillInit != nullptr)
	{
		bool ReturnReqMeleeSkillInit = _ReqMeleeSkillInit->Update();
		if (ReturnReqMeleeSkillInit)
		{
			G_ObjectManager->SkillInfoReturn(_ReqMeleeSkillInit->GetSkillInfo()->SkillMediumCategory, _ReqMeleeSkillInit->GetSkillInfo());
			G_ObjectManager->SkillReturn(_ReqMeleeSkillInit);
			_ReqMeleeSkillInit = nullptr;
		}
	}

	if (_ReqMagicSkillInit != nullptr)
	{
		bool ReturnReqMagicSkillInit = _ReqMagicSkillInit->Update();
		if (ReturnReqMagicSkillInit)
		{
			G_ObjectManager->SkillInfoReturn(_ReqMagicSkillInit->GetSkillInfo()->SkillMediumCategory, _ReqMagicSkillInit->GetSkillInfo());
			G_ObjectManager->SkillReturn(_ReqMagicSkillInit);
			_ReqMagicSkillInit = nullptr;
		}
	}

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

		if (_GameObjectInfo.ObjectStatInfo.MP > _GameObjectInfo.ObjectStatInfo.MaxMP)
		{
			_GameObjectInfo.ObjectStatInfo.MP = _GameObjectInfo.ObjectStatInfo.MaxMP;
		}

		_NatureRecoveryTick = GetTickCount64() + 5000;

		CMessage* ResObjectStatPacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId,
			_GameObjectInfo.ObjectStatInfo);
		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStatPacket);
		ResObjectStatPacket->Free();
	}

	// 냉기파동 상태이상 검사
	bool IsShmanIceWave = _StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ICE_WAVE;

	// 냉기파동 상태이상일 경우
	// 대상이 바라보고 있는 반대방향으로 캐릭터를 밀어버림
	if (IsShmanIceWave == true)
	{
		switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
		{
		case en_MoveDir::UP:
			_GameObjectInfo.ObjectPositionInfo.PositionY +=
				(st_Vector2::Down()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 2.0f * 0.02f);
			break;
		case en_MoveDir::DOWN:
			_GameObjectInfo.ObjectPositionInfo.PositionY +=
				(st_Vector2::Up()._Y * _GameObjectInfo.ObjectStatInfo.Speed * 2.0f * 0.02f);
			break;
		case en_MoveDir::LEFT:
			_GameObjectInfo.ObjectPositionInfo.PositionX +=
				(st_Vector2::Right()._X * _GameObjectInfo.ObjectStatInfo.Speed * 2.0f * 0.02f);
			break;
		case en_MoveDir::RIGHT:
			_GameObjectInfo.ObjectPositionInfo.PositionX +=
				(st_Vector2::Left()._X * _GameObjectInfo.ObjectStatInfo.Speed * 2.0f * 0.02f);
			break;
		}

		bool CanMove = _Channel->GetMap()->Cango(this, _GameObjectInfo.ObjectPositionInfo.PositionX, _GameObjectInfo.ObjectPositionInfo.PositionY);
		if (CanMove == true)
		{
			st_Vector2Int CollisionPosition;
			CollisionPosition._X = _GameObjectInfo.ObjectPositionInfo.PositionX;
			CollisionPosition._Y = _GameObjectInfo.ObjectPositionInfo.PositionY;

			if (CollisionPosition._X != _GameObjectInfo.ObjectPositionInfo.CollisionPositionX
				|| CollisionPosition._Y != _GameObjectInfo.ObjectPositionInfo.CollisionPositionY)
			{
				_Channel->GetMap()->ApplyMove(this, CollisionPosition);
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

			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMovePacket);
			ResMovePacket->Free();
		}
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
	_FieldOfViewInfos.clear();

	// 스킬 목록 정리
	_SkillBox.Empty();
	// 퀵슬롯 정리
	_QuickSlotManager.Empty();

	// 남아 있는 플레이어 잡 큐 처리
	CGameObject::Update();

	_GameObjectInfo.ObjectId = 0;
	_GameObjectInfo.ObjectName = L"";

	_NetworkState = en_ObjectNetworkState::READY;
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

	bool CanMove = _Channel->GetMap()->Cango(this, _GameObjectInfo.ObjectPositionInfo.PositionX, _GameObjectInfo.ObjectPositionInfo.PositionY);
	if (CanMove == true)
	{
		st_Vector2Int CollisionPosition;
		CollisionPosition._X = _GameObjectInfo.ObjectPositionInfo.PositionX;
		CollisionPosition._Y = _GameObjectInfo.ObjectPositionInfo.PositionY;

		if (CollisionPosition._X != _GameObjectInfo.ObjectPositionInfo.CollisionPositionX
			|| CollisionPosition._Y != _GameObjectInfo.ObjectPositionInfo.CollisionPositionY)
		{
			_Channel->GetMap()->ApplyMove(this, CollisionPosition);
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

		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMovePacket);
		ResMovePacket->Free();
	}
}

void CPlayer::UpdateAttack()
{

}

void CPlayer::UpdateSpell()
{
	if (_SpellTick < GetTickCount64())
	{
		// 마법 스킬을 사용한 캐릭터의 시전 속도에 비례해 다음 마법 스킬을 사용 할 수 있도록 한다.
		st_GameObjectJob* GameObjectJob = G_ObjectManager->GameObjectJobCreate();
		GameObjectJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_REQ_MAGIC;

		CGameServerMessage* ReqMeleeSkillMessage = CGameServerMessage::GameServerMessageAlloc();
		ReqMeleeSkillMessage->Clear();

		*ReqMeleeSkillMessage << &_CurrentSkill;

		GameObjectJob->GameObjectJobMessage = ReqMeleeSkillMessage;

		_GameObjectJobQue.Enqueue(GameObjectJob);

		_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

		CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.MoveDir,
			_GameObjectInfo.ObjectType,
			_GameObjectInfo.ObjectPositionInfo.State);
		G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStateChangePacket);
		ResObjectStateChangePacket->Free();

		if (_CurrentSkill != nullptr)
		{
			_CurrentSkill->CoolTimeStart();

			for (auto QuickSlotBarPosition : _QuickSlotManager.FindQuickSlotBar(_CurrentSkill->GetSkillInfo()->SkillType))
			{
				// 클라에게 쿨타임 표시
				CMessage* ResCoolTimeStartPacket = G_ObjectManager->GameServer->MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
					QuickSlotBarPosition.QuickSlotBarSlotIndex,
					1.0f, _CurrentSkill);
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResCoolTimeStartPacket);
				ResCoolTimeStartPacket->Free();
			}

			// 전역 쿨타임 시간 표시
			for (auto QuickSlotBarPosition : _QuickSlotManager.ExceptionFindQuickSlotBar(_CurrentSkill->_QuickSlotBarIndex, _CurrentSkill->_QuickSlotBarSlotIndex, _CurrentSkill->GetSkillKind()))
			{
				CMessage* ResCoolTimeStartPacket = G_ObjectManager->GameServer->MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
					QuickSlotBarPosition.QuickSlotBarSlotIndex,
					1.0f, nullptr, (int32)(500 * _GameObjectInfo.ObjectStatInfo.MagicHitRate));
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResCoolTimeStartPacket);
				ResCoolTimeStartPacket->Free();
			}

			en_EffectType HitEffectType = en_EffectType::EFFECT_TYPE_NONE;

			wstring MagicSystemString;

			wchar_t SpellMessage[64] = L"0";

			// 크리티컬 판단
			random_device Seed;
			default_random_engine Eng(Seed());

			float CriticalPoint = _GameObjectInfo.ObjectStatInfo.MagicCriticalPoint / 1000.0f;
			bernoulli_distribution CriticalCheck(CriticalPoint);
			bool IsCritical = CriticalCheck(Eng);

			int32 FinalDamage = 0;

			mt19937 Gen(Seed());	

			bool TargetIsDead = false;

			switch (_CurrentSkill->GetSkillInfo()->SkillType)
			{
			case en_SkillType::SKILL_SHAMAN_FLAME_HARPOON:
			{
				HitEffectType = en_EffectType::EFFECT_FLAME_HARPOON_TARGET;

				int32 MagicDamage = _GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6;

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_CurrentSkill->GetSkillInfo();

				uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
				int32 ChoiceDamage = DamageChoiceRandom(Gen);
				FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

				wsprintf(SpellMessage, L"%s가 %s을 사용해 %s에게 %d의 데미지를 줬습니다.", this->_GameObjectInfo.ObjectName.c_str(), _CurrentSkill->GetSkillInfo()->SkillName.c_str(), _SelectTarget->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

				// 데미지 처리
				TargetIsDead = _SelectTarget->OnDamaged(this, FinalDamage);

				MagicSystemString = SpellMessage;
			}
			break;
			case en_SkillType::SKILL_SHAMAN_ROOT:
			{
				HitEffectType = en_EffectType::EFFECT_DEBUF_ROOT;

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_CurrentSkill->GetSkillInfo();

				CSkill* NewSkill = G_ObjectManager->SkillCreate();

				st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(_CurrentSkill->GetSkillInfo()->SkillMediumCategory);
				*NewAttackSkillInfo = *((st_AttackSkillInfo*)_CurrentSkill->GetSkillInfo());
				NewSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
				NewSkill->StatusAbnormalDurationTimeStart();

				_SelectTarget->AddDebuf(NewSkill);
				_SelectTarget->SetStatusAbnormal(STATUS_ABNORMAL_SHAMAN_ROOT);

				CMessage* SelectTargetMoveStopMessage = G_ObjectManager->GameServer->MakePacketResMoveStop(_AccountId, _SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectPositionInfo);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, SelectTargetMoveStopMessage);
				SelectTargetMoveStopMessage->Free();

				CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
					_SelectTarget->_GameObjectInfo.ObjectType,
					_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
					_CurrentSkill->GetSkillInfo()->SkillType,
					true, STATUS_ABNORMAL_SHAMAN_ROOT);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResStatusAbnormalPacket);
				ResStatusAbnormalPacket->Free();

				CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResBufDeBufSkillPacket);
				ResBufDeBufSkillPacket->Free();
			}
			break;
			case en_SkillType::SKILL_SHAMAN_ICE_CHAIN:
			{
				HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;

				int32 MagicDamage = _GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6;

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_CurrentSkill->GetSkillInfo();

				uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
				int32 ChoiceDamage = DamageChoiceRandom(Gen);
				FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

				// 데미지 처리
				TargetIsDead = _SelectTarget->OnDamaged(this, FinalDamage);				

				if (AttackSkillInfo->NextComboSkill != en_SkillType::SKILL_TYPE_NONE)
				{
					CSkill* FindNextComboSkill = _SkillBox.FindSkill(AttackSkillInfo->NextComboSkill);
					if (FindNextComboSkill->GetSkillInfo()->CanSkillUse == true)
					{
						st_GameObjectJob* ComboAttackCreateJob = G_ObjectManager->GameObjectJobCreate();
						ComboAttackCreateJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_COMBO_ATTACK_CREATE;

						CGameServerMessage* ComboAttackCreateMessage = CGameServerMessage::GameServerMessageAlloc();
						ComboAttackCreateMessage->Clear();

						*ComboAttackCreateMessage << _CurrentSkill->_QuickSlotBarIndex;
						*ComboAttackCreateMessage << _CurrentSkill->_QuickSlotBarSlotIndex;
						*ComboAttackCreateMessage << &_CurrentSkill;

						ComboAttackCreateJob->GameObjectJobMessage = ComboAttackCreateMessage;

						_GameObjectJobQue.Enqueue(ComboAttackCreateJob);
					}					
				}

				_SelectTarget->_GameObjectInfo.ObjectStatInfo.Speed -= AttackSkillInfo->SkillDebufMovingSpeed;

				CMessage* ResObjectStatChange = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId,
					_GameObjectInfo.ObjectStatInfo);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResObjectStatChange);
				ResObjectStatChange->Free();

				CSkill* NewSkill = G_ObjectManager->SkillCreate();

				st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(_CurrentSkill->GetSkillInfo()->SkillMediumCategory);
				*NewAttackSkillInfo = *((st_AttackSkillInfo*)_CurrentSkill->GetSkillInfo());
				NewSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
				NewSkill->StatusAbnormalDurationTimeStart();

				_SelectTarget->AddDebuf(NewSkill);
				_SelectTarget->SetStatusAbnormal(STATUS_ABNORMAL_SHAMAN_ICE_CHAIN);

				CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
					_SelectTarget->_GameObjectInfo.ObjectType,
					_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
					_CurrentSkill->GetSkillInfo()->SkillType, true, STATUS_ABNORMAL_SHAMAN_ICE_CHAIN);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResStatusAbnormalPacket);
				ResStatusAbnormalPacket->Free();

				CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResBufDeBufSkillPacket);
				ResBufDeBufSkillPacket->Free();
			}
			break;
			case en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE:
			{
				HitEffectType = en_EffectType::EFFECT_LIGHTNING;

				int32 MagicDamage = _GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6;

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_CurrentSkill->GetSkillInfo();

				uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
				int32 ChoiceDamage = DamageChoiceRandom(Gen);
				FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

				// 데미지 처리
				TargetIsDead = _SelectTarget->OnDamaged(this, FinalDamage);

				CSkill* NewSkill = G_ObjectManager->SkillCreate();

				st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(_CurrentSkill->GetSkillInfo()->SkillMediumCategory);
				*NewAttackSkillInfo = *((st_AttackSkillInfo*)_CurrentSkill->GetSkillInfo());
				NewSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
				NewSkill->StatusAbnormalDurationTimeStart();

				_SelectTarget->AddDebuf(NewSkill);
				_SelectTarget->SetStatusAbnormal(STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE);

				CMessage* SelectTargetMoveStopMessage = G_ObjectManager->GameServer->MakePacketResMoveStop(_AccountId, _SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectPositionInfo);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, SelectTargetMoveStopMessage);
				SelectTargetMoveStopMessage->Free();

				CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
					_SelectTarget->_GameObjectInfo.ObjectType,
					_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
					_CurrentSkill->GetSkillInfo()->SkillType,
					true, STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResStatusAbnormalPacket);
				ResStatusAbnormalPacket->Free();

				CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResBufDeBufSkillPacket);
				ResBufDeBufSkillPacket->Free();

				float EffectPrintTime = _CurrentSkill->GetSkillInfo()->SkillDurationTime / 1000.0f;

				// 이펙트 출력
				CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(_SelectTarget->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_DEBUF_STUN, EffectPrintTime);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResEffectPacket);
				ResEffectPacket->Free();
			}
			break;
			case en_SkillType::SKILL_SHAMAN_HELL_FIRE:
			{
				HitEffectType = en_EffectType::EFFECT_FLAME_HARPOON_TARGET;

				int32 MagicDamage = _GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6;

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_CurrentSkill->GetSkillInfo();

				uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
				int32 ChoiceDamage = DamageChoiceRandom(Gen);
				FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

				wsprintf(SpellMessage, L"%s가 %s을 사용해 %s에게 %d의 데미지를 줬습니다.", _GameObjectInfo.ObjectName.c_str(), _CurrentSkill->GetSkillInfo()->SkillName.c_str(), _SelectTarget->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

				// 데미지 처리
				TargetIsDead = _SelectTarget->OnDamaged(this, FinalDamage);

				MagicSystemString = SpellMessage;
			}
			break;
			case en_SkillType::SKILL_TAIOIST_DIVINE_STRIKE:
			{
				HitEffectType = en_EffectType::EFFECT_FLAME_HARPOON_TARGET;

				int32 MagicDamage = (int32)(_GameObjectInfo.ObjectStatInfo.MagicDamage * 0.6);

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_CurrentSkill->GetSkillInfo();

				uniform_int_distribution<int> DamageChoiceRandom(AttackSkillInfo->SkillMinDamage + MagicDamage, AttackSkillInfo->SkillMaxDamage + MagicDamage);
				int32 ChoiceDamage = DamageChoiceRandom(Gen);
				FinalDamage = IsCritical ? ChoiceDamage * 2 : ChoiceDamage;

				wsprintf(SpellMessage, L"%s가 %s을 사용해 %s에게 %d의 데미지를 줬습니다.", _GameObjectInfo.ObjectName.c_str(), _CurrentSkill->GetSkillInfo()->SkillName.c_str(), _SelectTarget->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

				// 데미지 처리
				TargetIsDead = _SelectTarget->OnDamaged(this, FinalDamage);

				MagicSystemString = SpellMessage;
			}
			break;
			case en_SkillType::SKILL_TAIOIST_ROOT:
			{
				HitEffectType = en_EffectType::EFFECT_DEBUF_ROOT;

				st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)_CurrentSkill->GetSkillInfo();

				CSkill* NewSkill = G_ObjectManager->SkillCreate();

				st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(_CurrentSkill->GetSkillInfo()->SkillMediumCategory);
				*NewAttackSkillInfo = *((st_AttackSkillInfo*)_CurrentSkill->GetSkillInfo());
				NewSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
				NewSkill->StatusAbnormalDurationTimeStart();

				CMessage* SelectTargetMoveStopMessage = G_ObjectManager->GameServer->MakePacketResMoveStop(_AccountId, _SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectPositionInfo);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, SelectTargetMoveStopMessage);
				SelectTargetMoveStopMessage->Free();

				_SelectTarget->AddDebuf(NewSkill);
				_SelectTarget->SetStatusAbnormal(STATUS_ABNORMAL_TAIOIST_ROOT);

				CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
					_SelectTarget->_GameObjectInfo.ObjectType,
					_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
					_CurrentSkill->GetSkillInfo()->SkillType,
					true, STATUS_ABNORMAL_TAIOIST_ROOT);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResStatusAbnormalPacket);
				ResStatusAbnormalPacket->Free();

				CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResBufDeBufSkillPacket);
				ResBufDeBufSkillPacket->Free();
			}
			break;
			case en_SkillType::SKILL_TAIOIST_HEALING_LIGHT:
			{
				HitEffectType = en_EffectType::EFFECT_HEALING_LIGHT_TARGET;

				st_HealSkillInfo* HealSkillInfo = (st_HealSkillInfo*)_CurrentSkill->GetSkillInfo();

				uniform_int_distribution<int> HealChoiceRandom(HealSkillInfo->SkillMinHealPoint, HealSkillInfo->SkillMaxHealPoint);
				FinalDamage = HealChoiceRandom(Gen);

				wsprintf(SpellMessage, L"%s가 치유의빛을 사용해 %s를 %d만큼 회복했습니다.", _GameObjectInfo.ObjectName.c_str(), _SelectTarget->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

				_SelectTarget->OnHeal(this, FinalDamage);
				
				MagicSystemString = SpellMessage;

			}
			break;
			case en_SkillType::SKILL_TAIOIST_HEALING_WIND:
			{
				HitEffectType = en_EffectType::EFFECT_HEALING_WIND_TARGET;

				st_HealSkillInfo* HealSkillInfo = (st_HealSkillInfo*)_CurrentSkill->GetSkillInfo();

				uniform_int_distribution<int> HealChoiceRandom(HealSkillInfo->SkillMinHealPoint, HealSkillInfo->SkillMaxHealPoint);
				FinalDamage = HealChoiceRandom(Gen);

				wsprintf(SpellMessage, L"%s가 치유의바람을 사용해 %s를 %d만큼 회복했습니다.", _GameObjectInfo.ObjectName.c_str(), _SelectTarget->_GameObjectInfo.ObjectName.c_str(), FinalDamage);

				_SelectTarget->OnHeal(this, FinalDamage);				

				MagicSystemString = SpellMessage;
			}
			break;
			}					

			// 공격 응답
			CMessage* ResAttackMagicPacket = G_ObjectManager->GameServer->MakePacketResAttack(
				_GameObjectInfo.ObjectId,
				_SelectTarget->_GameObjectInfo.ObjectId,
				_CurrentSkill->GetSkillInfo()->SkillType,
				FinalDamage,
				false);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResAttackMagicPacket);
			ResAttackMagicPacket->Free();

			// 이펙트 출력
			CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(_SelectTarget->_GameObjectInfo.ObjectId, HitEffectType, _CurrentSkill->GetSkillInfo()->SkillTargetEffectTime);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResEffectPacket);
			ResEffectPacket->Free();

			// HP 변경 전송
			CMessage* ResChangeObjectStat = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_SelectTarget->_GameObjectInfo.ObjectId,
				_SelectTarget->_GameObjectInfo.ObjectStatInfo);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResChangeObjectStat);
			ResChangeObjectStat->Free();

			if (TargetIsDead == true)
			{
				CMessage* SelectTargetDeadPacket = G_ObjectManager->GameServer->MakePacketObjectDie(_SelectTarget->_GameObjectInfo.ObjectId);
				G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, SelectTargetDeadPacket);
				SelectTargetDeadPacket->Free();

				_SelectTarget = nullptr;
			}

			// 시스템 메세지 전송
			CMessage* ResAttackMagicSystemMessagePacket = G_ObjectManager->GameServer->MakePacketResChattingBoxMessage(_GameObjectInfo.ObjectId, en_MessageType::SYSTEM, st_Color::White(), MagicSystemString);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResAttackMagicSystemMessagePacket);
			ResAttackMagicSystemMessagePacket->Free();			

			// 스펠창 끝
			CMessage* ResMagicPacket = G_ObjectManager->GameServer->MakePacketResMagic(_GameObjectInfo.ObjectId, false);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMagicPacket);
			ResMagicPacket->Free();			
		}
	}
}