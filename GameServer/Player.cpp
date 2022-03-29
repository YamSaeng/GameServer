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

	// �þ߹��� ��ü ����
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
		
		// �ѹ� �� �˻�
		if (SpawnObjectIds.size() > 0)
		{
			// ���� �ؾ��� ������ ����
			vector<CGameObject*> SpawnObjectInfos;
			for (st_FieldOfViewInfo SpawnObject : SpawnObjectIds)
			{
				if (SpawnObject.ObjectId != 0 && SpawnObject.ObjectType != en_GameObjectType::NORMAL)
				{
					CGameObject* FindObject = G_ObjectManager->Find(SpawnObject.ObjectId, SpawnObject.ObjectType);
					if (FindObject != nullptr) 
					{
						// �þ� ���� �ȿ� ������ ��� �������� ����
						int16 Distance = st_Vector2Int::Distance(FindObject->GetCellPosition(), GetCellPosition());
						if (Distance <= _FieldOfViewDistance)
						{
							SpawnObjectInfos.push_back(FindObject);
						}						
					}					
				}								
			}

			// �����ؾ� �� ����� ������ �����϶�� �˸�
			if (SpawnObjectInfos.size() > 0)
			{
				CMessage* ResOtherObjectSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn((int32)SpawnObjectInfos.size(), SpawnObjectInfos);
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResOtherObjectSpawnPacket);
				ResOtherObjectSpawnPacket->Free();

				// ���� �ؾ��� ���� ���ؿ��� ������ �Ÿ��� ���� �ؾ��� ���� �þ߹����ȿ� ���� ��� ���� �����϶�� �˷���
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

		// �ѹ� �� �˻�
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

			// �����ؾ� �� ����� ������ ���� �����϶�� �˸�
			if (DeSpawnObjectInfos.size() > 0)
			{
				CMessage* ResOtherObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn((int32)DeSpawnObjectInfos.size(), DeSpawnObjectInfos);
				G_ObjectManager->GameServer->SendPacket(_SessionId, ResOtherObjectDeSpawnPacket);
				ResOtherObjectDeSpawnPacket->Free();

				// ������ ���� ������ �Ÿ��� ���� �þ߹��� �ȿ� ���� ��� ���� ���������� �ʴ´�.
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

	// ��ų��� ������Ʈ
	_SkillBox.Update();

	// ��ȭȿ�� ��ų ����Ʈ ��ȸ
	for (auto BufSkillIterator : _Bufs)
	{
		// ���ӽð� ���� ��ȭȿ�� ����
		bool DeleteBufSkill = BufSkillIterator.second->Update();
		if (DeleteBufSkill)
		{
			DeleteBuf(BufSkillIterator.first);
			// ��ȭȿ�� ��ų ���� �޸� �ݳ�
			G_ObjectManager->SkillInfoReturn(BufSkillIterator.second->GetSkillInfo()->SkillMediumCategory, BufSkillIterator.second->GetSkillInfo());
			// ��ȭȿ�� ��ų �޸� �ݳ�
			G_ObjectManager->SkillReturn(BufSkillIterator.second);
		}
	}

	// ��ȭȿ�� ��ų ����Ʈ ��ȸ
	for (auto DebufSkillIterator : _DeBufs)
	{
		// ���ӽð� ���� ��ȭȿ�� ����
		bool DeleteDebufSkill = DebufSkillIterator.second->Update();
		if (DeleteDebufSkill)
		{
			DeleteDebuf(DebufSkillIterator.first);
			// ��ȭȿ�� ��ų ���� �޸� �ݳ�
			G_ObjectManager->SkillInfoReturn(DebufSkillIterator.second->GetSkillInfo()->SkillMediumCategory, DebufSkillIterator.second->GetSkillInfo());
			// ��ȭȿ�� ��ų �޸� �ݳ�
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
	// ��ų ��� ����
	_SkillBox.Empty();
	// ������ ����
	_QuickSlotManager.Empty();
	
	// ���� �ִ� �÷��̾� �� ť ó��
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

		// �ٲ� ��ǥ �� �þ߹��� ������Ʈ�鿡�� ����
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

			// ũ��Ƽ�� �Ǵ�
			float CriticalPoint = _GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint / 1000.0f;
			bernoulli_distribution CriticalCheck(CriticalPoint);
			bool IsCritical = CriticalCheck(Eng);

			// ������ �Ǵ�
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

			// �ý��� �޼��� ����
			wsprintf(SkillTypeMessage, L"%s�� �Ϲݰ����� ����� %s���� %d�� �������� ����ϴ�.", _GameObjectInfo.ObjectName.c_str(), Target->_GameObjectInfo.ObjectName.c_str(), FinalDamage);
			HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;

			SkillTypeString = SkillTypeMessage;
			SkillTypeString = IsCritical ? L"ġ��Ÿ! " + SkillTypeString : SkillTypeString;

			// ������ �ý��� �޼��� ����
			CMessage* ResSkillSystemMessagePacket = G_ObjectManager->GameServer->MakePacketResChattingBoxMessage(_GameObjectInfo.ObjectId,
				en_MessageType::SYSTEM,
				IsCritical ? st_Color::Red() : st_Color::White(),
				SkillTypeString);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResSkillSystemMessagePacket, this);
			ResSkillSystemMessagePacket->Free();

			// ���� ���� �޼��� ����
			CMessage* ResMyAttackOtherPacket = G_ObjectManager->GameServer->MakePacketResAttack(_GameObjectInfo.ObjectId,
				Target->_GameObjectInfo.ObjectId,				
				_GameObjectInfo.ObjectPositionInfo.MoveDir,
				en_SkillType::SKILL_DEFAULT_ATTACK,
				FinalDamage,
				IsCritical);
			G_ObjectManager->GameServer->SendPacketFieldOfView(_FieldOfViewInfos, ResMyAttackOtherPacket, this);
			ResMyAttackOtherPacket->Free();
			
			// ���� ���� �޼��� ����
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