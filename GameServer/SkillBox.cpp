#include "pch.h"
#include "SkillBox.h"
#include "Skill.h"
#include "ObjectManager.h"
#include "NetworkManager.h"
#include "SwordBlade.h"

CSkillBox::CSkillBox()
{
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
		_GlobalCoolTimeSkill->SetOwner(_OwnerGameObject);

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
			&& PublicActiveSkill->GetSkillKind() == SkillKind
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
			&& ActiveSkill->GetSkillKind() == SkillKind
			&& ActiveSkill->GetSkillInfo()->CanSkillUse == true)
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

void CSkillBox::SkillProcess(CGameObject* SkillUser, CGameObject* SkillUserd, en_SkillCharacteristic SkillCharacteristic, en_SkillType SkillType)
{
	bool IsNextCombo = false;

	CSkill* Skill = FindSkill(SkillCharacteristic, SkillType);
	if (Skill != nullptr)
	{
		CPlayer* Player = dynamic_cast<CPlayer*>(SkillUser);
		if (Player != nullptr)
		{			
			// 전역 재사용 시간이 완료 되었는지 확인
			if (_GlobalCoolTimeSkill->GetSkillInfo()->CanSkillUse == true)
			{
				// 기술을 사용 할 수 있는지 확인			
				if (Skill->GetSkillInfo()->CanSkillUse == true)
				{
					vector<st_FieldOfViewInfo> AroundPlayers = SkillUser->GetChannel()->GetMap()->GetFieldAroundPlayers(SkillUser, false);

					CMessage* ResAnimationPlayPacket = G_NetworkManager->GetGameServer()->MakePacketResAnimationPlay(SkillUser->_GameObjectInfo.ObjectId,
						SkillUser->_GameObjectInfo.ObjectType, en_AnimationType::ANIMATION_TYPE_SWORD_MELEE_ATTACK);
					G_NetworkManager->GetGameServer()->SendPacketFieldOfView(AroundPlayers, ResAnimationPlayPacket);
					ResAnimationPlayPacket->Free();

					SkillUser->SetMeleeSkill(Skill);

					switch (Skill->GetSkillKind())
					{
					case en_SkillKinds::SKILL_KIND_MELEE_SKILL:
						{
							st_SkillInfo* MeleeSkillInfo = Skill->GetSkillInfo();

							switch (Skill->GetSkillInfo()->SkillType)
							{
							case en_SkillType::SKILL_DEFAULT_ATTACK:
							case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
							case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
								{
									vector<CGameObject*> FieldOfViewObjects = SkillUser->GetFieldOfViewObjects();
									for (CGameObject* FieldOfViewObject : FieldOfViewObjects)
									{
										if (FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD
											&& FieldOfViewObject ->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::ROOTING)
										{
											if (Vector2::CheckFieldOfView(FieldOfViewObject->_GameObjectInfo.ObjectPositionInfo.Position,
												SkillUser->_GameObjectInfo.ObjectPositionInfo.Position,
												SkillUser->_FieldOfDirection, SkillUser->_FieldOfAngle, Skill->GetSkillInfo()->SkillDistance))
											{
												IsNextCombo = true;
												st_GameObjectJob* DamageJob = G_NetworkManager->GetGameServer()->MakeGameObjectDamage(SkillUser->_GameObjectInfo.ObjectId, SkillUser->_GameObjectInfo.ObjectType,
													MeleeSkillInfo->SkillType,
													MeleeSkillInfo->SkillMinDamage,
													MeleeSkillInfo->SkillMaxDamage);
												FieldOfViewObject->_GameObjectJobQue.Enqueue(DamageJob);
											}
										}										
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
												Vector2Int JumpingPositionDir = Player->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.CollisionPosition
													- Player->_GameObjectInfo.ObjectPositionInfo.CollisionPosition;

												Vector2Int JumpingPosition = JumpingPositionDir.Direction();
												JumpingPosition *= -1;												

												Vector2Int NewCollisionPosition;
												NewCollisionPosition.X = Player->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.X + JumpingPosition.X;
												NewCollisionPosition.Y = Player->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y + JumpingPosition.Y;

												Vector2 NewPosition;
												NewPosition.X = NewCollisionPosition.X + 0.5f;
												NewPosition.Y = NewCollisionPosition.Y + 0.5f;

												bool IsCollision = Player->GetChannel()->ChannelColliderCheck(Player, NewPosition);
												if (IsCollision == true)
												{
													st_GameObjectJob* DamageJob = G_NetworkManager->GetGameServer()->MakeGameObjectDamage(SkillUser->_GameObjectInfo.ObjectId, 
														SkillUser->_GameObjectInfo.ObjectType,
														MeleeSkillInfo->SkillType,
														MeleeSkillInfo->SkillMinDamage,
														MeleeSkillInfo->SkillMaxDamage);
													Player->_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);

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
												else
												{
													G_Logger->WriteStdOut(en_Color::RED, L"충돌해서 해당 좌표로 이동 못함\n");
												}
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
								{									
									vector<st_FieldOfViewInfo> FieldOfViewInfos = SkillUser->GetChannel()->GetMap()->GetFieldOfViewAttackObjects(SkillUser, 1);
									for (auto FieldOfViewInfo : FieldOfViewInfos)
									{
										CGameObject* FindObject = SkillUser->GetChannel()->FindChannelObject(FieldOfViewInfo.ObjectID, FieldOfViewInfo.ObjectType);
										
										float Distance = Vector2::Distance(FindObject->_GameObjectInfo.ObjectPositionInfo.Position, SkillUser->_GameObjectInfo.ObjectPositionInfo.Position);
										if (Distance < Skill->GetSkillInfo()->SkillDistance)
										{
											st_GameObjectJob* DamageJob = G_NetworkManager->GetGameServer()->MakeGameObjectDamage(SkillUser->_GameObjectInfo.ObjectId,
												SkillUser->_GameObjectInfo.ObjectType,
												MeleeSkillInfo->SkillType,
												MeleeSkillInfo->SkillMinDamage,
												MeleeSkillInfo->SkillMaxDamage);
											FindObject->_GameObjectJobQue.Enqueue(DamageJob);
										}
									}									
								}
								break;
							case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FLY_KNIFE:
								{
									CSwordBlade* NewSwordBlade = dynamic_cast<CSwordBlade*>(G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_SKILL_SWORD_BLADE));
									if (NewSwordBlade != nullptr)
									{
										NewSwordBlade->_Owner = SkillUser;

										NewSwordBlade->_SpawnPosition = SkillUser->_GameObjectInfo.ObjectPositionInfo.CollisionPosition;
										NewSwordBlade->_GameObjectInfo.ObjectPositionInfo.Position = SkillUser->_GameObjectInfo.ObjectPositionInfo.Position + SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton;
										NewSwordBlade->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton = SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton;
										NewSwordBlade->_GameObjectInfo.ObjectPositionInfo.MoveDirection = SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton;						

										NewSwordBlade->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = Skill->GetSkillInfo()->SkillMinDamage;
										NewSwordBlade->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = Skill->GetSkillInfo()->SkillMaxDamage;

										st_GameObjectJob* EnterChannelSwordBladeJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobObjectEnterChannel(NewSwordBlade);
										SkillUser->GetChannel()->_ChannelJobQue.Enqueue(EnterChannelSwordBladeJob);
									}
								}
								break;
							case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE:
								{
									if (SkillUser->_SelectTarget != nullptr)
									{
										Vector2 TargetPosition = SkillUser->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position;
										float ChoHoneDistance = Vector2::Distance(TargetPosition, SkillUser->_GameObjectInfo.ObjectPositionInfo.Position);

										if (Skill->GetSkillInfo()->SkillDistance >= ChoHoneDistance)
										{
											Vector2 MyFrontPosition = SkillUser->_GameObjectInfo.ObjectPositionInfo.Position + SkillUser->_GameObjectInfo.ObjectPositionInfo.LookAtDireciton;

											Vector2Int MyFrontIntPosition;
											MyFrontIntPosition.X = MyFrontPosition.X;
											MyFrontIntPosition.Y = MyFrontPosition.Y;

											if (SkillUser->GetChannel() != nullptr)
											{
												if (SkillUser->GetChannel()->GetMap() != nullptr)
												{
													vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = SkillUser->GetChannel()->GetMap()->GetFieldAroundPlayers(SkillUser, false);

													if (SkillUser->GetChannel()->GetMap()->ApplyMove(SkillUser->_SelectTarget, MyFrontIntPosition))
													{
														SkillUser->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.X = MyFrontPosition.X;
														SkillUser->_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.Y = MyFrontPosition.Y;

														CMessage* SelectTargetStopPacket = G_NetworkManager->GetGameServer()->MakePacketResMoveStop(SkillUser->_SelectTarget->_GameObjectInfo.ObjectId,
															MyFrontPosition.X,
															MyFrontPosition.Y);
														G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, SelectTargetStopPacket);
														SelectTargetStopPacket->Free();
													}
												}
											}
										}
									}
									else
									{

									}
								}
								break;															
							}
						}
						break;
					case en_SkillKinds::SKILL_KIND_SPELL_SKILL:
						break;					
					}

					if (IsNextCombo == true && Skill->GetSkillInfo()->NextComboSkill != en_SkillType::SKILL_TYPE_NONE)
					{
						CSkill* FindNextComboSkill = FindSkill(Skill->GetSkillInfo()->SkillCharacteristic, Skill->GetSkillInfo()->NextComboSkill);
						if (FindNextComboSkill->GetSkillInfo()->CanSkillUse == true)
						{
							st_GameObjectJob* ComboSkillJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobComboSkillCreate(Skill);
							SkillUser->_GameObjectJobQue.Enqueue(ComboSkillJob);
						}
					}

					CCreature* Creature = dynamic_cast<CCreature*>(SkillUser);
					if (Creature != nullptr)
					{
						if (Creature->_ComboSkill != nullptr)
						{
							st_GameObjectJob* ComboSkillOffJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobComboSkillOff();
							SkillUser->_GameObjectJobQue.Enqueue(ComboSkillOffJob);
						}
					}

					Skill->CoolTimeStart();

					// 쿨타임 표시 ( 퀵술롯 바에 등록되어 있는 같은 종류의 스킬을 모두 쿨타임 표시 시켜 준다 )
					for (auto QuickSlotBarPosition : Player->_QuickSlotManager.FindQuickSlotBar(Skill->GetSkillInfo()->SkillType))
					{
						// 클라에게 쿨타임 표시
						CMessage* ResCoolTimeStartPacket = G_NetworkManager->GetGameServer()->MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
							QuickSlotBarPosition.QuickSlotBarSlotIndex,
							1.0f, Skill);
						G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResCoolTimeStartPacket);
						ResCoolTimeStartPacket->Free();
					}

					// 전역 쿨타임 시작
					_GlobalCoolTimeSkill->CoolTimeStart();

					// 요청한 스킬과 기본 공격 스킬을 제외하고 스킬 창에서 가져옴
					vector<CSkill*> GlobalSkills = Player->_SkillBox.GetGlobalSkills(Skill->GetSkillInfo()->SkillType, Skill->GetSkillKind());

					// 전역 쿨타임 적용
					for (CSkill* GlobalSkill : GlobalSkills)
					{
						GlobalSkill->GlobalCoolTimeStart(Skill->GetSkillInfo()->SkillMotionTime);

						for (Vector2Int QuickSlotPosition : GlobalSkill->_QuickSlotBarPosition)
						{
							CMessage* ResCoolTimeStartPacket = G_NetworkManager->GetGameServer()->MakePacketCoolTime((int8)QuickSlotPosition.Y,
								(int8)QuickSlotPosition.X,
								1.0f, nullptr, _GlobalCoolTimeSkill->GetSkillInfo()->SkillCoolTime);
							G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResCoolTimeStartPacket);
							ResCoolTimeStartPacket->Free();
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
}

int32 CSkillBox::CalculateDamage(en_SkillType SkillType, int32& Str, int32& Dex, int32& Int, int32& Luck, bool* InOutCritical, int32 TargetDefence, int32 MinDamage, int32 MaxDamage, int16 CriticalPoint)
{
	random_device Seed;
	default_random_engine Eng(Seed());

	mt19937 Gen(Seed());
	uniform_int_distribution<int> DamageChoiceRandom(MinDamage, MaxDamage);

	int32 ChoiceRandomDamage = DamageChoiceRandom(Gen);

	int32 CriticalDamage = 0;

	if (*InOutCritical == true)
	{
		// 크리티컬 판단
		float CriticalPointCheck = CriticalPoint / 1000.0f;
		bernoulli_distribution CriticalCheck(CriticalPointCheck);
		bool IsCritical = CriticalCheck(Eng);

		*InOutCritical = IsCritical;

		CriticalDamage = IsCritical ? ChoiceRandomDamage * 2 : ChoiceRandomDamage;
	}
	else
	{
		CriticalDamage = ChoiceRandomDamage;
	}

	int32 FinalDamage = 0;

	switch (SkillType)
	{
	case en_SkillType::SKILL_DEFAULT_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_JUMPING_ATTACK:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_PIERCING_WAVE:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FLY_KNIFE:
	case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_COMBO_FLY_KNIFE:
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
	case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_QUICK_CUT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_FAST_CUT:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_ATTACK:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_ATTACK_BACK_STEP:
	case en_SkillType::SKILL_ASSASSINATION_ACTIVE_BUF_WEAPON_POISON:
	case en_SkillType::SKILL_GOBLIN_ACTIVE_MELEE_DEFAULT_ATTACK:

		FinalDamage = (int32)((CriticalDamage + Str / 2) * (1 - ((float)TargetDefence / (100.0f + (float)TargetDefence))));

		break;
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
	case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:
	case en_SkillType::SKILL_SPELL_ACTIVE_BUF_TELEPORT:
	case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:

		FinalDamage = (int32)((CriticalDamage + Int / 2) * (1 - ((float)TargetDefence / (100.0f + (float)TargetDefence))));

		break;
	case en_SkillType::SKILL_SHOOTING_ACTIVE_ATTACK_SNIFING:
		break;
	}

	float DefenceRate = (float)pow(((float)(200 - 1)) / 20, 2) * 0.01f;
	int32 FinalDamage = (int32)(CriticalDamage * DefenceRate);

	return FinalDamage;
}
