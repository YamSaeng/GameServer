#include "pch.h"
#include "SkillBox.h"
#include "Skill.h"
#include "ObjectManager.h"
#include "NetworkManager.h"
#include "SwordBlade.h"
#include "FlameBolt.h"
#include "DivineBolt.h"
#include "RectCollision.h"

CSkillBox::CSkillBox()
{
	_ComboSkill = nullptr;

	_GlobalCoolTimeSkill = nullptr;	

	_OwnerGameObject = nullptr;
}

CSkillBox::~CSkillBox()
{
	
}

void CSkillBox::Init()
{
	_GlobalCoolTimeSkill = G_ObjectManager->SkillCreate();
	if (_GlobalCoolTimeSkill != nullptr)
	{	
		_GlobalCoolTimeSkill->SetTarget(_OwnerGameObject);

		st_SkillInfo* GlobalCoolTimeSkillInfo = G_ObjectManager->SkillInfoCreate(en_SkillType::SKILL_GLOBAL_SKILL, 1);
		if (GlobalCoolTimeSkillInfo != nullptr)
		{
			_GlobalCoolTimeSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_GLOBAL, GlobalCoolTimeSkillInfo);
		}		
	}	
}

void CSkillBox::SetOwner(CGameObject* Owner)
{
	_OwnerGameObject = Owner;
}

CSkillCharacteristic* CSkillBox::FindCharacteristic(int8 FindCharacteristicType)
{	
	if (_SkillCharacteristic._SkillCharacteristic == (en_SkillCharacteristic)FindCharacteristicType)
	{
		return &_SkillCharacteristic;
	}
	else
	{
		return nullptr;
	}	
}

void CSkillBox::CreateChracteristic(int8 CharacteristicType)
{
	_SkillCharacteristic.SkillCharacteristicInit((en_SkillCharacteristic)CharacteristicType);	
}

void CSkillBox::SkillLearn(bool IsSkillLearn, en_SkillType LearnSkillType)
{
	_SkillCharacteristic.SkillCharacteristicActive(IsSkillLearn, LearnSkillType, 1);
}

CSkill* CSkillBox::FindSkill(en_SkillCharacteristic CharacteristicType, en_SkillType SkillType)
{
	CSkill* FindSkill = nullptr;

	switch (CharacteristicType)
	{	
	case en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC:
		FindSkill = _SkillCharacteristicPublic.FindSkill(SkillType);
		break;
	case en_SkillCharacteristic::SKILL_CATEGORY_FIGHT:		
	case en_SkillCharacteristic::SKILL_CATEGORY_PROTECTION:		
	case en_SkillCharacteristic::SKILL_CATEGORY_SPELL:		
	case en_SkillCharacteristic::SKILL_CATEGORY_SHOOTING:		
	case en_SkillCharacteristic::SKILL_CATEGORY_DISCIPLINE:		
	case en_SkillCharacteristic::SKILL_CATEGORY_ASSASSINATION:
		if (_SkillCharacteristic._SkillCharacteristic == CharacteristicType)
		{
			FindSkill = _SkillCharacteristic.FindSkill(SkillType);
		}
		else
		{
			FindSkill = nullptr;
		}
		break;	
	}	

	return FindSkill;
}

void CSkillBox::Update()
{
	_GlobalCoolTimeSkill->Update();

	if (_ComboSkill != nullptr)
	{
		if (_ComboSkill->Update() == true)
		{
			G_ObjectManager->SkillReturn(_ComboSkill);

			_ComboSkill = nullptr;
		}
	}

	_SkillCharacteristicPublic.CharacteristicUpdate();
	_SkillCharacteristic.CharacteristicUpdate();	
}

void CSkillBox::Empty()
{
	_SkillCharacteristicPublic.CharacteristicEmpty();
	_SkillCharacteristic.CharacteristicEmpty();	
}

vector<CSkill*> CSkillBox::GetGlobalSkills(en_SkillType ExceptSkillType, en_SkillKinds SkillKind)
{
	vector<CSkill*> GlobalSkills;

	// 기본 공격 스킬은 제외함	
	vector<CSkill*> PubliChracteristicActiveSkill = _SkillCharacteristicPublic.GetActiveSkill();
	for (CSkill* PublicActiveSkill : PubliChracteristicActiveSkill)
	{
		if (PublicActiveSkill->GetSkillInfo()->SkillType != en_SkillType::SKILL_DEFAULT_ATTACK
			&& PublicActiveSkill->GetSkillInfo()->SkillType != ExceptSkillType
			&& PublicActiveSkill->GetSkillInfo()->SkillKind == SkillKind
			&& PublicActiveSkill->GetSkillInfo()->CanSkillUse == true)
		{
			GlobalSkills.push_back(PublicActiveSkill);
		}
	}	

	// 요청한 스킬과 같은 종류의 스킬을 스킬특성 창에서 찾음
	vector<CSkill*> SkillCharacteristicActiveSkill = _SkillCharacteristic.GetActiveSkill();
	for (CSkill* ActiveSkill : SkillCharacteristicActiveSkill)
	{
		if (ActiveSkill->GetSkillInfo()->IsSkillLearn == true
			&& ActiveSkill->GetSkillInfo()->SkillType != ExceptSkillType			
			&& ActiveSkill->GetSkillInfo()->CanSkillUse == true
			&& ActiveSkill->GetSkillInfo()->SkillKind == SkillKind)
		{
			GlobalSkills.push_back(ActiveSkill);
		}
	}

	return GlobalSkills;
}

CSkillCharacteristic* CSkillBox::GetSkillCharacteristicPublic()
{
	return &_SkillCharacteristicPublic;
}

CSkillCharacteristic* CSkillBox::GetSkillCharacteristics()
{
	return &_SkillCharacteristic;
}

bool CSkillBox::CheckCharacteristic(en_SkillCharacteristic SkillCharacteristic)
{	
	return _SkillCharacteristic._SkillCharacteristic == SkillCharacteristic;	
}

bool CSkillBox::SetStatusAbnormal(CGameObject* SkillUser, CGameObject* Target, en_GameObjectStatusType StatusType, en_SkillType SkillType, int8 SkillLevel)
{
	vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = SkillUser->GetChannel()->GetMap()->GetFieldAroundPlayers(SkillUser, false);

	CSkill* StatusAbnormalSkill = G_ObjectManager->SkillCreate();
	if (StatusAbnormalSkill != nullptr)
	{
		// 새로운 상태이상 스킬 적용
		auto StatusAbnormalSkillIter = Target->_DeBufs.find(SkillType);
		if (StatusAbnormalSkillIter == Target->_DeBufs.end())
		{
			st_SkillInfo* NewSkillInfo = G_ObjectManager->SkillInfoCreate(SkillType);
			if (NewSkillInfo != nullptr)
			{
				StatusAbnormalSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewSkillInfo);
				StatusAbnormalSkill->SetTarget(Target);								

				Target->AddDebuf(StatusAbnormalSkill);

				StatusAbnormalSkill->StatusAbnormalDurationTimeStart();

				Target->SetStatusAbnormal((int64)StatusType);

				CMessage* ResObjectStatChange = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectStat(Target->_GameObjectInfo.ObjectId,
					Target->_GameObjectInfo.ObjectStatInfo);
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResObjectStatChange);
				ResObjectStatChange->Free();

				CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(Target->_GameObjectInfo.ObjectId,
					Target->_GameObjectInfo.ObjectPositionInfo.Position.X,
					Target->_GameObjectInfo.ObjectPositionInfo.Position.Y,
					StatusAbnormalSkill->GetSkillInfo(),
					true, (int64)StatusType);
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
				ResStatusAbnormalPacket->Free();

				CMessage* ResBufDebufSkillPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBuf(Target->_GameObjectInfo.ObjectId, false, StatusAbnormalSkill->GetSkillInfo());
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDebufSkillPacket);
				ResBufDebufSkillPacket->Free();
			}			
		}
		else
		{
			// 상대방 약화효과에 적용할 상태이상 스킬이 있을 경우
			CSkill* FindDebufSkill = (*StatusAbnormalSkillIter).second;
			if (FindDebufSkill != nullptr)
			{
				switch (FindDebufSkill->GetSkillInfo()->SkillType)
				{
				case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_POISON_INJECTION:
					{
						FindDebufSkill->GetSkillInfo()->SkillOverlapStep++;
						FindDebufSkill->StatusAbnormalDurationTimeStart();

						CMessage* ResBufDebufSkillPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBuf(Target->_GameObjectInfo.ObjectId, false, FindDebufSkill->GetSkillInfo());
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDebufSkillPacket);
						ResBufDebufSkillPacket->Free();
					}					
					break;
				default:
					{
						FindDebufSkill->StatusAbnormalDurationTimeStart();

						CMessage* ResBufDebufSkillPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBuf(Target->_GameObjectInfo.ObjectId, false, FindDebufSkill->GetSkillInfo());
						G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDebufSkillPacket);
						ResBufDebufSkillPacket->Free();
					}					
					break;
				}
			}
		}

		return true;
	}

	return false;	
}

bool CSkillBox::SelectTargetSkillUse(CGameObject* SkillUser, CSkill* Skill)
{
	if (SkillUser != nullptr)
	{
		if (SkillUser->_SelectTarget != nullptr)
		{
			float Distance = Vector2::Distance(SkillUser->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position,
				SkillUser->_GameObjectInfo.ObjectPositionInfo.Position);

			bool IsBehind = Vector2::IsBehind(SkillUser->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position,
				SkillUser->_GameObjectInfo.ObjectPositionInfo.Position, SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton);
			if (IsBehind == false && Distance < Skill->GetSkillInfo()->SkillDistance)
			{
				return true;
			}
			else if (IsBehind == false && Distance > Skill->GetSkillInfo()->SkillDistance)
			{
				CPlayer* Player = dynamic_cast<CPlayer*>(SkillUser);
				if (Player != nullptr)
				{
					CMessage* DistanceFarPacket = G_NetworkManager->GetGameServer()->MakePacketCommonError(en_GlobalMessageType::GLOBAL_MESSAGE_FAR_DISTANCE);
					G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, DistanceFarPacket);
					DistanceFarPacket->Free();
				}

				return false;
			}
			else if (IsBehind == true)
			{
				CPlayer* Player = dynamic_cast<CPlayer*>(SkillUser);
				if (Player != nullptr)
				{
					CMessage* DistanceFarPacket = G_NetworkManager->GetGameServer()->MakePacketCommonError(en_GlobalMessageType::GLOBAL_MESSAGE_ATTACK_ANGLE);
					G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, DistanceFarPacket);
					DistanceFarPacket->Free();
				}

				return false;
			}						
		}
		else
		{
			CPlayer* Player = dynamic_cast<CPlayer*>(SkillUser);
			if (Player != nullptr)
			{
				CMessage* NoneSelectTargetPacket = G_NetworkManager->GetGameServer()->MakePacketCommonError(en_GlobalMessageType::GLOBAL_MESSAGE_NON_SELECT_OBJECT);
				G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, NoneSelectTargetPacket);
				NoneSelectTargetPacket->Free();
			}

			return false;
		}
	}
	else
	{
		return false;
	}
}

vector<CGameObject*> CSkillBox::CollisionSkillUse(CGameObject* SkillUser, CSkill* Skill, en_CollisionPosition CollisionPositionType, Vector2 CollisionCreatePosition, Vector2 CollisionCreateDir)
{	
	vector<CGameObject*> CollisionObjects;

	if (SkillUser != nullptr)
	{
		if (Skill != nullptr)
		{						
			CRectCollision* SkillCollision = G_ObjectManager->RectCollisionCreate();
			if (SkillCollision != nullptr)
			{	
				switch (CollisionPositionType)
				{
				case en_CollisionPosition::COLLISION_POSITION_OBJECT:
					SkillCollision->SkillRectInit(en_CollisionPosition::COLLISION_POSITION_OBJECT, Skill,
						CollisionCreatePosition,
						CollisionCreateDir);
					break;
				case en_CollisionPosition::COLLISION_POSITION_SKILL_MIDDLE:
					SkillCollision->SkillRectInit(en_CollisionPosition::COLLISION_POSITION_SKILL_MIDDLE, Skill,
						CollisionCreatePosition,
						Vector2::Zero,
						SkillUser);
					break;				
				case en_CollisionPosition::COLLISION_POSITION_SKILL_FRONT:
					SkillCollision->SkillRectInit(en_CollisionPosition::COLLISION_POSITION_SKILL_FRONT, Skill,
						CollisionCreatePosition,
						CollisionCreateDir,
						SkillUser);
					break;
				}							

				SkillUser->GetChannel()->ChannelColliderOBBCheckAroundObject(SkillCollision, SkillUser->GetFieldOfViewInfo(), CollisionObjects, SkillUser->_GameObjectInfo.ObjectId);

				CPlayer* Player = dynamic_cast<CPlayer*>(SkillUser);
				if (Player != nullptr)
				{
					CMessage* CollisionSpawnPacket = G_NetworkManager->GetGameServer()->MakePacketRectCollisionSpawn(SkillCollision);
					G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, CollisionSpawnPacket);
					CollisionSpawnPacket->Free();
				}				
			}	

			G_ObjectManager->RectCollisionReturn(SkillCollision);
		}		
	}

	return CollisionObjects;
}

void CSkillBox::ShockReleaseUse(CGameObject* User, CSkill* ShockReleaseSkill)
{
	for (auto DebufSkillIter : User->_DeBufs)
	{
		// 기절 상태이상 해제
		if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_WRATH_ATTACK
			|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_LAST_ATTACK
			|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH
			|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_COUNTER
			|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SWORD_STORM			
			|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE
			|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE
			|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_POISON_STUN
			|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP)
		{
			DebufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
		}
	}

	// 충격해제 버프 생성
	CSkill* NewBufSkill = G_ObjectManager->SkillCreate();
	st_SkillInfo* NewShockReleaseSkillInfo = G_ObjectManager->SkillInfoCreate(ShockReleaseSkill->GetSkillInfo()->SkillType, ShockReleaseSkill->GetSkillInfo()->SkillLevel);
	NewBufSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewShockReleaseSkillInfo);
	NewBufSkill->StatusAbnormalDurationTimeStart();

	// 충격해제 버프 등록
	User->AddBuf(NewBufSkill);

	vector<st_FieldOfViewInfo> AroundPlayers = User->GetChannel()->GetMap()->GetFieldAroundPlayers(User, false);

	// 클라에게 버프 등록 알림
	CMessage* ResBufDebufSkillPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBuf(User->_GameObjectInfo.ObjectId, true, NewBufSkill->GetSkillInfo());
	G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, ResBufDebufSkillPacket);
	ResBufDebufSkillPacket->Free();

	// 상태이상 해제 알림
	CMessage* ResObjectStateChangePacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectState(User->_GameObjectInfo.ObjectId,
		User->_GameObjectInfo.ObjectPositionInfo.State);
	G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, ResObjectStateChangePacket);
	ResObjectStateChangePacket->Free();
}

void CSkillBox::MoveStatusAbnormalRelease(CGameObject* User)
{
	// 이동 불가와 이동 속도 감소 효과를 모두 삭제
	for (auto DebufSkillIter : User->_DeBufs)
	{
		if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_JUMPING_ATTACK
			|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE
			|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN
			|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT
			|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT)
		{
			DebufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
		}
	}
}

void CSkillBox::SkillIsCasting(CGameObject* SkillUser, en_SkillCharacteristic SkillCharacteristic, en_SkillType SkillType, int8 QuickSlotBarIndex, int8 QuickSlotBarSlotIndex, float AttackDirectionX, float AttackDirectionY)
{
	CSkill* Skill = FindSkill(SkillCharacteristic, SkillType);
	if (Skill != nullptr)
	{
		// 전역 재사용 시간이 완료 되었는지 확인
		if (_GlobalCoolTimeSkill->GetSkillInfo()->CanSkillUse == true)
		{
			_SkillUseQuickSlotPosition.QuickSlotBarIndex = QuickSlotBarIndex;
			_SkillUseQuickSlotPosition.QuickSlotBarSlotIndex = QuickSlotBarSlotIndex;			

			SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton.X = AttackDirectionX;
			SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton.Y = AttackDirectionY;

			// 기술을 사용 할 수 있는지 확인			
			if (Skill->GetSkillInfo()->CanSkillUse == true)
			{
				int32 SkillCastingTime = Skill->GetSkillInfo()->SkillCastingTime;
				if (SkillCastingTime == 0)
				{
					SkillProcess(SkillUser, Skill);
				}
				else
				{
					if (SkillUser->GetSkillCastingSkill() != nullptr)
					{
						return;
					}

					// 시전 중인 기술 저장 및	시전 시간 설정
					SkillUser->SetSkillCastingSkill(Skill);						

					// 기술 시전 바 시작
					CMessage* ResMagicPacket = G_NetworkManager->GetGameServer()->MakePacketSkillCastingStart(SkillUser->_GameObjectInfo.ObjectId,
						true, Skill->GetSkillInfo()->SkillType, SkillCastingTime / 1000.0f);
					G_NetworkManager->GetGameServer()->SendPacketFieldOfView(SkillUser, ResMagicPacket);
					ResMagicPacket->Free();
				}
			}
		}
	}
}

void CSkillBox::SkillProcess(CGameObject* SkillUser, CSkill* Skill)
{
	bool IsNextCombo = false;
	bool IsGlobalCoolTime = true;

	CPlayer* Player = dynamic_cast<CPlayer*>(SkillUser);
	if (Player != nullptr)
	{
		CRectCollision* SkillCollision = nullptr;

		// 전역 재사용 시간이 완료 되었는지 확인
		if (_GlobalCoolTimeSkill->GetSkillInfo()->CanSkillUse == true)
		{
			// 기술을 사용 할 수 있는지 확인			
			if (Skill->GetSkillInfo()->CanSkillUse == true)
			{
				bool IsDamage = false;
				bool IsBackAttack = false;

				vector<st_FieldOfViewInfo> AroundPlayers = SkillUser->GetChannel()->GetMap()->GetFieldAroundPlayers(SkillUser, false);

				CMessage* ResAnimationPlayPacket = G_NetworkManager->GetGameServer()->MakePacketResAnimationPlay(SkillUser->_GameObjectInfo.ObjectId,
					SkillUser->_GameObjectInfo.ObjectType, en_AnimationType::ANIMATION_TYPE_SWORD_MELEE_ATTACK);
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, ResAnimationPlayPacket);
				ResAnimationPlayPacket->Free();				

				vector<CGameObject*> CollisionObjects;

				bool IsSkillProcessSuccess = true;

				switch (Skill->GetSkillInfo()->SkillKind)
				{
				case en_SkillKinds::SKILL_KIND_MELEE_SKILL:
					{
						switch (Skill->GetSkillInfo()->SkillType)
						{
						case en_SkillType::SKILL_DEFAULT_ATTACK:
							IsGlobalCoolTime = false;

							CollisionObjects = CollisionSkillUse(SkillUser, Skill,
								en_CollisionPosition::COLLISION_POSITION_SKILL_FRONT,
								SkillUser->_GameObjectInfo.ObjectPositionInfo.Position, Player->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton);
							break;
						case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:												
						case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT:
						case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT:
							IsNextCombo = true;

							CollisionObjects = CollisionSkillUse(SkillUser, Skill,
								en_CollisionPosition::COLLISION_POSITION_SKILL_FRONT,
								SkillUser->_GameObjectInfo.ObjectPositionInfo.Position, Player->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton);
							break;						
						case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
							if (_ComboSkill != nullptr && _ComboSkill->GetSkillInfo()->SkillType == en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK)
							{
								IsNextCombo = true;

								CollisionObjects = CollisionSkillUse(SkillUser, Skill,
									en_CollisionPosition::COLLISION_POSITION_SKILL_FRONT,
									SkillUser->_GameObjectInfo.ObjectPositionInfo.Position, Player->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton);
							}			
							else
							{
								IsSkillProcessSuccess = false;
							}
							break;
						case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_WRATH_ATTACK:
							if (_ComboSkill != nullptr && _ComboSkill->GetSkillInfo()->SkillType == en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_WRATH_ATTACK)
							{
								if (SelectTargetSkillUse(SkillUser, Skill))
								{
									CollisionObjects.push_back(SkillUser->_SelectTarget);
								}
							}	
							else
							{
								IsSkillProcessSuccess = false;
							}
							break;
						case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE:
							CollisionObjects = CollisionSkillUse(SkillUser, Skill,
								en_CollisionPosition::COLLISION_POSITION_SKILL_MIDDLE,
								SkillUser->_GameObjectInfo.ObjectPositionInfo.Position, Vector2::Zero);
							break;
						case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FLY_KNIFE:
							{
								CSwordBlade* NewSwordBlade = dynamic_cast<CSwordBlade*>(G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_SKILL_SWORD_BLADE));
								if (NewSwordBlade != nullptr)
								{
									NewSwordBlade->_Owner = SkillUser;

									NewSwordBlade->_SpawnPosition = SkillUser->_GameObjectInfo.ObjectPositionInfo.CollisionPosition;
									NewSwordBlade->_GameObjectInfo.ObjectPositionInfo.Position = SkillUser->_GameObjectInfo.ObjectPositionInfo.Position + SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton;
									NewSwordBlade->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton = Player->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton;
									NewSwordBlade->_GameObjectInfo.ObjectPositionInfo.MoveDirection = Player->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton;

									NewSwordBlade->_GameObjectInfo.ObjectStatInfo.MinAttackPoint = Skill->GetSkillInfo()->SkillMinDamage;
									NewSwordBlade->_GameObjectInfo.ObjectStatInfo.MaxAttackPoint = Skill->GetSkillInfo()->SkillMaxDamage;

									st_GameObjectJob* EnterChannelSwordBladeJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobObjectEnterChannel(NewSwordBlade);
									SkillUser->GetChannel()->_ChannelJobQue.Enqueue(EnterChannelSwordBladeJob);
								}
							}
							break;
						case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_JUMPING_ATTACK:
							{
								CPlayer* Player = dynamic_cast<CPlayer*>(SkillUser);
								if (Player != nullptr)
								{
									if (Player->_SelectTarget != nullptr)
									{
										float Distance = Vector2::Distance(Player->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position,
											Player->_GameObjectInfo.ObjectPositionInfo.Position);
										if (Distance < Skill->GetSkillInfo()->SkillDistance)
										{
											// 이동할 위치를 구하고 적용
											Vector2 NewPosition = Player->GetChannel()->GetMap()->GetMovePositionNearTarget(Player, Player->_SelectTarget);
											Player->_GameObjectInfo.ObjectPositionInfo.Position = NewPosition;

											Vector2Int CollisionPosition;
											CollisionPosition.X = (int32)Player->_GameObjectInfo.ObjectPositionInfo.Position.X;
											CollisionPosition.Y = (int32)Player->_GameObjectInfo.ObjectPositionInfo.Position.Y;

											if (CollisionPosition.X != Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X
												|| CollisionPosition.Y != Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y)
											{
												Player->GetChannel()->GetMap()->ApplyMove(Player, CollisionPosition);
											}

											// 주위 유저들에게 새로 이동한 위치를 알려줌
											CMessage* SyncPositionPacket = G_NetworkManager->GetGameServer()->MakePacketResSyncPosition(Player->_GameObjectInfo.ObjectId, Player->_GameObjectInfo.ObjectPositionInfo);
											G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, SyncPositionPacket);
											SyncPositionPacket->Free();

											// 데미지 판정을 위해 충돌체 생성
											CollisionObjects = CollisionSkillUse(SkillUser, Skill, en_CollisionPosition::COLLISION_POSITION_SKILL_MIDDLE,
												Player->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position, Vector2::Zero);
										}
										else
										{
											CMessage* DistanceFarPacket = G_NetworkManager->GetGameServer()->MakePacketCommonError(en_GlobalMessageType::GLOBAL_MESSAGE_FAR_DISTANCE);
											G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, DistanceFarPacket);
											DistanceFarPacket->Free();
										}
									}
									else
									{
										CMessage* NonSelectTargetPacket = G_NetworkManager->GetGameServer()->MakePacketCommonError(en_GlobalMessageType::GLOBAL_MESSAGE_NON_SELECT_OBJECT);
										G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, NonSelectTargetPacket);
										NonSelectTargetPacket->Free();
									}
								}
							}
							break;
						case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_PIERCING_WAVE:
							CollisionObjects = CollisionSkillUse(SkillUser, Skill,
								en_CollisionPosition::COLLISION_POSITION_SKILL_MIDDLE,
								SkillUser->_GameObjectInfo.ObjectPositionInfo.Position, Vector2::Zero);
							break;
						case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_LAST_ATTACK:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_COUNTER:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SWORD_STORM:
							CollisionObjects = CollisionSkillUse(SkillUser, Skill,
								en_CollisionPosition::COLLISION_POSITION_SKILL_FRONT,
								SkillUser->_GameObjectInfo.ObjectPositionInfo.Position, Player->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton);
							break;
						case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);

								Vector2 MyFrontPosition =
									SkillUser->_GameObjectInfo.ObjectPositionInfo.Position + SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton;

								Vector2Int MyFrontIntPosition;
								MyFrontIntPosition.X = MyFrontPosition.X;
								MyFrontIntPosition.Y = MyFrontPosition.Y;

								bool IsCollision = Player->GetChannel()->ChannelColliderCheck(Player->_SelectTarget, MyFrontPosition);
								if (IsCollision == true)
								{
									Player->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position = MyFrontPosition;

									Vector2Int CollisionPosition;
									CollisionPosition.X = (int32)Player->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.X;
									CollisionPosition.Y = (int32)Player->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.Y;

									if (CollisionPosition.X != Player->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X
										|| CollisionPosition.Y != Player->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y)
									{
										Player->GetChannel()->GetMap()->ApplyMove(Player->_SelectTarget, CollisionPosition);
									}

									CMessage* SyncPositionPacket = G_NetworkManager->GetGameServer()->MakePacketResSyncPosition(Player->_SelectTarget->_GameObjectInfo.ObjectId, Player->_SelectTarget->_GameObjectInfo.ObjectPositionInfo);
									G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, SyncPositionPacket);
									SyncPositionPacket->Free();
								}
							}
							break;
						case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK:
						case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_CUT:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);

								IsBackAttack = Vector2::IsBehind(SkillUser->_GameObjectInfo.ObjectPositionInfo.Position,
									SkillUser->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position, SkillUser->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton);
							}
							break;
						case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_ADVANCE_CUT:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);

								Vector2 NewPosition = Player->GetChannel()->GetMap()->GetMovePositionNearTarget(Player, Player->_SelectTarget);

								bool IsCollision = Player->GetChannel()->ChannelColliderCheck(Player, NewPosition);
								if (IsCollision == true)
								{
									Player->_GameObjectInfo.ObjectPositionInfo.Position = NewPosition;

									Vector2Int CollisionPosition;
									CollisionPosition.X = (int32)Player->_GameObjectInfo.ObjectPositionInfo.Position.X;
									CollisionPosition.Y = (int32)Player->_GameObjectInfo.ObjectPositionInfo.Position.Y;

									if (CollisionPosition.X != Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X
										|| CollisionPosition.Y != Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y)
									{
										Player->GetChannel()->GetMap()->ApplyMove(Player, CollisionPosition);
									}

									CMessage* SyncPositionPacket = G_NetworkManager->GetGameServer()->MakePacketResSyncPosition(Player->_GameObjectInfo.ObjectId, Player->_GameObjectInfo.ObjectPositionInfo);
									G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, SyncPositionPacket);
									SyncPositionPacket->Free();
								}
							}
							break;
						case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_POISON_INJECTION:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_POISON_STUN:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_ASSASSINATION:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						}
					}
					break;
				case en_SkillKinds::SKILL_KIND_SPELL_SKILL:
					{
						st_SkillInfo* MeleeSkillInfo = Skill->GetSkillInfo();

						switch (Skill->GetSkillInfo()->SkillType)
						{
						case en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE:
							ShockReleaseUse(Player, Skill);
							break;
						case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_BOLT:
							{
								CFlameBolt* FlameBolt = dynamic_cast<CFlameBolt*>(G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_SKILL_FLAME_BOLT));
								if (FlameBolt != nullptr)
								{
									Vector2 FlameBoltDirection(SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton.X, SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton.Y);

									FlameBolt->_Owner = SkillUser;

									FlameBolt->_SpawnPosition = SkillUser->_GameObjectInfo.ObjectPositionInfo.CollisionPosition;
									FlameBolt->_GameObjectInfo.ObjectPositionInfo.Position = SkillUser->_GameObjectInfo.ObjectPositionInfo.Position + SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton;
									FlameBolt->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton = FlameBoltDirection;
									FlameBolt->_GameObjectInfo.ObjectPositionInfo.MoveDirection = FlameBoltDirection;

									FlameBolt->_GameObjectInfo.ObjectStatInfo.MinAttackPoint = Skill->GetSkillInfo()->SkillMinDamage;
									FlameBolt->_GameObjectInfo.ObjectStatInfo.MaxAttackPoint = Skill->GetSkillInfo()->SkillMaxDamage;

									st_GameObjectJob* EnterChannelFlameBoltJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobObjectEnterChannel(FlameBolt);
									SkillUser->GetChannel()->_ChannelJobQue.Enqueue(EnterChannelFlameBoltJob);
								}
							}
							break;
						case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_BLAZE:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_SLEEP:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_WINTER_BINDING:
							CollisionObjects = CollisionSkillUse(SkillUser, Skill,
								en_CollisionPosition::COLLISION_POSITION_SKILL_MIDDLE,
								SkillUser->_GameObjectInfo.ObjectPositionInfo.Position, Vector2::Zero);
							break;
						case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_SPELL_ACTIVE_BUF_BACK_TELEPORT:
							{				
								Vector2 MovePosition;
								MovePosition = SkillUser->_GameObjectInfo.ObjectPositionInfo.Position + (-SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton * Skill->GetSkillInfo()->SkillDistance);
								
								Vector2Int CollisionPosition;
								CollisionPosition.X = (int32)MovePosition.X;
								CollisionPosition.Y = (int32)MovePosition.Y;

								CChannel* Channel = SkillUser->GetChannel();
								if (Channel != nullptr)
								{
									CRectCollision CheckRectCollision = *SkillUser->GetRectCollision();
									CheckRectCollision._Position = MovePosition;
									CheckRectCollision.NotSetPositionUpdate();

									bool MoveCheck = Channel->GetMap()->MoveCollisionCango(SkillUser, CollisionPosition, &CheckRectCollision);
									if (MoveCheck == true)
									{
										SkillUser->_GameObjectInfo.ObjectPositionInfo.Position = MovePosition;
										SkillUser->GetRectCollision()->Update();

										// 시공 뒤틀림 위치 재조정
										CMessage* ResSyncPositionPacket = G_NetworkManager->GetGameServer()->MakePacketResSyncPosition(SkillUser->_GameObjectInfo.ObjectId, SkillUser->_GameObjectInfo.ObjectPositionInfo);
										G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, ResSyncPositionPacket);
										ResSyncPositionPacket->Free();
									}
								}												
							}
							break;
						case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:
							{
								CDivineBolt* DivineBolt = dynamic_cast<CDivineBolt*>(G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_SKILL_DIVINE_BOLT));
								if (DivineBolt != nullptr)
								{
									Vector2 DivineBoltDirection(SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton.X, SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton.Y);

									DivineBolt->_Owner = SkillUser;

									DivineBolt->_SpawnPosition = SkillUser->_GameObjectInfo.ObjectPositionInfo.CollisionPosition;
									DivineBolt->_GameObjectInfo.ObjectPositionInfo.Position = SkillUser->_GameObjectInfo.ObjectPositionInfo.Position + SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton;
									DivineBolt->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton = DivineBoltDirection;
									DivineBolt->_GameObjectInfo.ObjectPositionInfo.MoveDirection = DivineBoltDirection;

									DivineBolt->_GameObjectInfo.ObjectStatInfo.MinAttackPoint = Skill->GetSkillInfo()->SkillMinDamage;
									DivineBolt->_GameObjectInfo.ObjectStatInfo.MaxAttackPoint = Skill->GetSkillInfo()->SkillMaxDamage;

									st_GameObjectJob* EnterChannelDivineBoltJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobObjectEnterChannel(DivineBolt);
									SkillUser->GetChannel()->_ChannelJobQue.Enqueue(EnterChannelDivineBoltJob);
								}
							}
							break;
						case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_THUNDER_BOLT:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_JUDGMENT:
							if (SelectTargetSkillUse(SkillUser, Skill))
							{
								CollisionObjects.push_back(SkillUser->_SelectTarget);
							}
							break;
						case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:
						case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_COUNTER_ARMOR:
						case en_SkillType::SKILL_PROTECTION_ACTIVE_BUF_FURY:
						case en_SkillType::SKILL_PROTECTION_ACTIVE_DOUBLE_ARMOR:
						case en_SkillType::SKILL_SPELL_ACTIVE_BUF_ILLUSION:
						case en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_STEALTH:
						case en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_SIXTH_SENSE_MAXIMIZE:
							{
								CSkill* BufSkill = G_ObjectManager->SkillCreate();

								st_SkillInfo* BufSkillInfo = G_ObjectManager->SkillInfoCreate(Skill->GetSkillInfo()->SkillType, Skill->GetSkillInfo()->SkillLevel);
								BufSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_BUF_SKILL, BufSkillInfo);
								BufSkill->SetTarget(Player);

								Player->AddBuf(BufSkill);

								BufSkill->BufTimeStart();								

								CMessage* ResBufDeBufSkillPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBuf(SkillUser->_GameObjectInfo.ObjectId, true, BufSkill->GetSkillInfo());
								G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, ResBufDeBufSkillPacket);
								ResBufDeBufSkillPacket->Free();
							}
							break;
						}
					}
					break;
				case en_SkillKinds::SKILL_KIND_HEAL_SKILL:
					switch (Skill->GetSkillInfo()->SkillType)
					{
					case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT:
						break;
					case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_RECOVERY_LIGHT:
						break;
					case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_GRACE:
						break;
					case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:
						break;
					case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_RECOVERY_WIND:
						break;
					case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_VITALITY_LIGHT:
						break;
					}
					break;
				}


				if (IsSkillProcessSuccess == true)
				{
					st_GameObjectJob* DamageJob = nullptr;

					if (CollisionObjects.size() > 0)
					{
						for (CGameObject* CollisionObject : CollisionObjects)
						{
							if (Skill->GetSkillInfo()->SkillIsDamage == true)
							{
								DamageJob = G_NetworkManager->GetGameServer()->MakeGameObjectDamage(SkillUser->_GameObjectInfo.ObjectId,
									SkillUser->_GameObjectInfo.ObjectType,
									Skill->GetSkillInfo()->SkillKind,
									Skill->GetSkillInfo()->SkillMinDamage,
									Skill->GetSkillInfo()->SkillMaxDamage,
									IsBackAttack);

								if (DamageJob != nullptr
									&& CollisionObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD
									&& CollisionObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::ROOTING)
								{
									CollisionObject->_GameObjectJobQue.Enqueue(DamageJob);
								}
							}

							if (Skill->GetSkillInfo()->SkillStatusAbnormal != en_GameObjectStatusType::STATUS_ABNORMAL_NONE)
							{
								SetStatusAbnormal(SkillUser, CollisionObject, Skill->GetSkillInfo()->SkillStatusAbnormal, Skill->GetSkillInfo()->SkillType, Skill->GetSkillInfo()->SkillLevel);
							}
						}
					}

					Skill->CoolTimeStart();

					// 클라에게 쿨타임 표시
					CMessage* ResCoolTimeStartPacket = G_NetworkManager->GetGameServer()->MakePacketCoolTime(Player->_QuickSlotManager.FindQuickSlotBar(Skill->GetSkillInfo()->SkillType),
						1.0f, Skill);
					G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResCoolTimeStartPacket);
					ResCoolTimeStartPacket->Free();

					// 연속기 기술 비활성화
					if (_ComboSkill != nullptr)
					{
						ComboSkillOff();
					}

					if (IsNextCombo == true && Skill->GetSkillInfo()->NextComboSkill != en_SkillType::SKILL_TYPE_NONE)
					{
						CSkill* FindNextComboSkill = FindSkill(Skill->GetSkillInfo()->SkillCharacteristic, Skill->GetSkillInfo()->NextComboSkill);
						if (FindNextComboSkill->GetSkillInfo()->CanSkillUse == true)
						{
							st_GameObjectJob* ComboSkillCreateJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobComboSkillCreate(
								Skill->GetSkillInfo()->SkillCharacteristic,
								Skill->GetSkillInfo()->SkillType,
								FindNextComboSkill->GetSkillInfo()->SkillCharacteristic,
								FindNextComboSkill->GetSkillInfo()->SkillType,
								_SkillUseQuickSlotPosition.QuickSlotBarIndex,
								_SkillUseQuickSlotPosition.QuickSlotBarSlotIndex);
							_OwnerGameObject->_GameObjectJobQue.Enqueue(ComboSkillCreateJob);
						}
					}

					if (IsGlobalCoolTime == true)
					{
						// 전역 쿨타임 시작
						_GlobalCoolTimeSkill->CoolTimeStart();

						// 요청한 스킬과 기본 공격 스킬을 제외하고 스킬 창에서 가져옴
						vector<CSkill*> GlobalSkills = Player->_SkillBox.GetGlobalSkills(Skill->GetSkillInfo()->SkillType, Skill->GetSkillInfo()->SkillKind);

						// 전역 쿨타임 적용
						for (CSkill* GlobalSkill : GlobalSkills)
						{
							GlobalSkill->GlobalCoolTimeStart(Skill->GetSkillInfo()->SkillMotionTime);

							CMessage* ResCoolTimeStartPacket = G_NetworkManager->GetGameServer()->MakePacketCoolTime(GlobalSkill->_QuickSlotBarPosition,
								1.0f, nullptr, _GlobalCoolTimeSkill->GetSkillInfo()->SkillCoolTime);
							G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResCoolTimeStartPacket);
							ResCoolTimeStartPacket->Free();
						}
					}
				}				
			}
			else
			{
				CMessage* ResErrorPacket = G_NetworkManager->GetGameServer()->MakePacketSkillError(en_GlobalMessageType::GLOBAL_MESSAGE_SKILL_COOLTIME, Skill->GetSkillInfo()->SkillName.c_str());
				G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResErrorPacket);
				ResErrorPacket->Free();
			}
		}
	}
}

CSkill* CSkillBox::ComboSkillCreate(CSkill* ComboSkill)
{
	if (ComboSkill != nullptr)
	{	
		// 연속기 기술 생성
		CSkill* NewComboSkill = G_ObjectManager->SkillCreate();
		if (NewComboSkill != nullptr)
		{
			NewComboSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_COMBO_SKILL, ComboSkill->GetSkillInfo());

			_ComboSkill = NewComboSkill;

			NewComboSkill->ComboSkillStart(ComboSkill->_ComboSkillQuickSlotBarPosition);
			NewComboSkill->SetTarget(_OwnerGameObject);			
		}		

		return NewComboSkill;
	}

	return nullptr;
}

void CSkillBox::ComboSkillOff()
{
	if (_ComboSkill != nullptr)
	{
		_ComboSkill->ComboSkillOff();

		vector<st_QuickSlotOffInfo> ComboSkillQuickSlotOffPositions;				

		CPlayer* Player = dynamic_cast<CPlayer*>(_OwnerGameObject);
		if (Player != nullptr)
		{
			for (auto QuickSlotBarPosition : _ComboSkill->_ComboSkillQuickSlotBarPosition)
			{
				st_QuickSlotBarSlotInfo* QuickSlotBarInfo = Player->_QuickSlotManager.FindQuickSlotBar(QuickSlotBarPosition.QuickSlotBarIndex, QuickSlotBarPosition.QuickSlotBarSlotIndex);
				if (QuickSlotBarInfo != nullptr)
				{
					st_QuickSlotOffInfo ComboSkillQuickSlotOffPosition;

					switch (QuickSlotBarInfo->QuickBarSkill->GetSkillInfo()->SkillType)
					{
					case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
						switch (_ComboSkill->GetSkillInfo()->SkillType)
						{
						case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
							ComboSkillQuickSlotOffPosition.QuickSlotBarIndex = QuickSlotBarInfo->QuickSlotBarIndex;
							ComboSkillQuickSlotOffPosition.QuickSlotBarSlotIndex = QuickSlotBarInfo->QuickSlotBarSlotIndex;
							ComboSkillQuickSlotOffPosition.RollBackSkillType = _ComboSkill->GetSkillInfo()->RollBackSkill;

							ComboSkillQuickSlotOffPositions.push_back(ComboSkillQuickSlotOffPosition);
							break;
						case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_WRATH_ATTACK:
							ComboSkillQuickSlotOffPosition.QuickSlotBarIndex = QuickSlotBarInfo->QuickSlotBarIndex;
							ComboSkillQuickSlotOffPosition.QuickSlotBarSlotIndex = QuickSlotBarInfo->QuickSlotBarSlotIndex;
							ComboSkillQuickSlotOffPosition.RollBackSkillType = _ComboSkill->GetSkillInfo()->RollBackSkill;

							ComboSkillQuickSlotOffPositions.push_back(ComboSkillQuickSlotOffPosition);
							break;	
						}
						break;											
					case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
						ComboSkillQuickSlotOffPosition.QuickSlotBarIndex = QuickSlotBarInfo->QuickSlotBarIndex;
						ComboSkillQuickSlotOffPosition.QuickSlotBarSlotIndex = QuickSlotBarInfo->QuickSlotBarSlotIndex;
						ComboSkillQuickSlotOffPosition.RollBackSkillType = QuickSlotBarInfo->QuickBarSkill->GetSkillInfo()->SkillType;

						ComboSkillQuickSlotOffPositions.push_back(ComboSkillQuickSlotOffPosition);						
						break;
					}
				}
			}
			
			if (ComboSkillQuickSlotOffPositions.size() > 0)
			{				
				CMessage* ResNextComboSkillOff = G_NetworkManager->GetGameServer()->MakePacketComboSkillOff(_ComboSkill->GetSkillInfo()->SkillType, ComboSkillQuickSlotOffPositions);
				G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResNextComboSkillOff);
				ResNextComboSkillOff->Free();
			}
			else
			{
				CMessage* ResNextComboSkillOff = G_NetworkManager->GetGameServer()->MakePacketComboSkillOff(en_SkillType::SKILL_TYPE_NONE, ComboSkillQuickSlotOffPositions);
				G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResNextComboSkillOff);
				ResNextComboSkillOff->Free();
			}
		}		

		_ComboSkill->_ComboSkillQuickSlotBarPosition.clear();		

		G_ObjectManager->SkillReturn(_ComboSkill);

		_ComboSkill = nullptr;
	}
}

int32 CSkillBox::CalculateDamage(int8 SkillKind, int32& Str, int32& Dex, int32& Int, int32& Luck, bool* InOutCritical, bool IsBackAttack, int32 TargetDefence, int32 MinDamage, int32 MaxDamage, int16 CriticalPoint)
{	
	int32 ChoiceRandomDamage = Math::RandomNumberInt(MinDamage, MaxDamage);

	int32 CriticalDamage = 0;

	if (*InOutCritical == true)
	{
		// 크리티컬 판단
		float CriticalPointCheck = CriticalPoint / 1000.0f;		
		bool IsCritical = Math::IsSuccess(CriticalPointCheck);

		*InOutCritical = IsCritical;

		CriticalDamage = IsCritical ? ChoiceRandomDamage * 2 : ChoiceRandomDamage;
	}
	else
	{
		CriticalDamage = ChoiceRandomDamage;
	}

	int32 FinalDamage = 0;

	switch ((en_SkillKinds)SkillKind)
	{
	case en_SkillKinds::SKILL_KIND_MELEE_SKILL:
		FinalDamage = (int32)((CriticalDamage + Str / 2) * (1 - ((float)TargetDefence / (100.0f + (float)TargetDefence))));
		break;
	case en_SkillKinds::SKILL_KIND_SPELL_SKILL:
		FinalDamage = (int32)((CriticalDamage + Int / 2) * (1 - ((float)TargetDefence / (100.0f + (float)TargetDefence))));
		break;
	case en_SkillKinds::SKILL_KIND_RANGE_SKILL:
		break;	
	}

	float DefenceRate = (float)pow(((float)(200 - 1)) / 20, 2) * 0.01f;
	FinalDamage = (int32)(CriticalDamage * DefenceRate);

	if (IsBackAttack == true)
	{
		FinalDamage *= 2.0f;
	}

	return FinalDamage;
}
