#include "pch.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "Skill.h"
#include "Furnace.h"

CGameObject::CGameObject()
{
	_StatusAbnormal = 0;

	_ObjectManagerArrayIndex = -1;
	_ChannelArrayIndex = -1;
	_NetworkState = en_ObjectNetworkState::READY;
	_GameObjectInfo.OwnerObjectId = 0;
	
	_Channel = nullptr;

	_MeleeSkill = nullptr;
	_SpellSkill = nullptr;

	_Owner = nullptr;
	_SelectTarget = nullptr;	

	_NatureRecoveryTick = 0;
	_FieldOfViewUpdateTick = 0;	

	_RectCollision = nullptr;	
}

CGameObject::CGameObject(st_GameObjectInfo GameObjectInfo)
{
	_GameObjectInfo = GameObjectInfo;
	_NetworkState = en_ObjectNetworkState::READY;
	_GameObjectInfo.OwnerObjectId = 0;
}

CGameObject::~CGameObject()
{
	if (_RectCollision != nullptr)
	{
		delete _RectCollision;
		_RectCollision = nullptr;
	}
}

void CGameObject::StatusAbnormalCheck()
{
	bool IsShamanIceWave = _StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ICE_WAVE;
	if (IsShamanIceWave == true)
	{
		switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
		{
		case en_MoveDir::UP:
			_GameObjectInfo.ObjectPositionInfo.Position._Y +=
				(st_Vector2::Down()._Y * 4.0f * 0.02f);
			break;
		case en_MoveDir::DOWN:
			_GameObjectInfo.ObjectPositionInfo.Position._Y +=
				(st_Vector2::Up()._Y * 4.0f * 0.02f);
			break;
		case en_MoveDir::LEFT:
			_GameObjectInfo.ObjectPositionInfo.Position._X +=
				(st_Vector2::Right()._X * 4.0f * 0.02f);
			break;
		case en_MoveDir::RIGHT:
			_GameObjectInfo.ObjectPositionInfo.Position._X +=
				(st_Vector2::Left()._X * 4.0f * 0.02f);
			break;
		}

		bool CanMove = _Channel->GetMap()->Cango(this, _GameObjectInfo.ObjectPositionInfo.Position._X, _GameObjectInfo.ObjectPositionInfo.Position._Y);
		if (CanMove == true)
		{
			st_Vector2Int CollisionPosition;
			CollisionPosition._X = _GameObjectInfo.ObjectPositionInfo.Position._X;
			CollisionPosition._Y = _GameObjectInfo.ObjectPositionInfo.Position._Y;

			if (CollisionPosition._X != _GameObjectInfo.ObjectPositionInfo.CollisionPosition._X
				|| CollisionPosition._Y != _GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y)
			{
				_Channel->GetMap()->ApplyMove(this, CollisionPosition);
			}
		}
		else
		{
			_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

			PositionReset();

			// �ٲ� ��ǥ �� �þ߹��� ������Ʈ�鿡�� ����
			CMessage* ResMovePacket = G_ObjectManager->GameServer->MakePacketResMove(
				_GameObjectInfo.ObjectId,
				CanMove,
				_GameObjectInfo.ObjectPositionInfo);

			G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResMovePacket);
			ResMovePacket->Free();
		}
	}
}

void CGameObject::Update()
{
	if (_NetworkState == en_ObjectNetworkState::LEAVE)
	{
		return;
	}

	while (!_GameObjectJobQue.IsEmpty())
	{
		st_GameObjectJob* GameObjectJob = nullptr;

		if (!_GameObjectJobQue.Dequeue(&GameObjectJob))
		{
			break;
		}

		switch ((en_GameObjectJobType)GameObjectJob->GameObjectJobType)
		{
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SHOCK_RELEASE:
			{
				for (auto DebufSkillIter : _DeBufs)
				{
					if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_KNIGHT_CHOHONE
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SHAMAN_ICE_WAVE)
					{
						DebufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
					}
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_BACK_TELEPORT:
			{
				for (auto DebufSkillIter : _DeBufs)
				{
					if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SHAMAN_ICE_CHAIN
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SHAMAN_ROOT
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_TAIOIST_ROOT)
					{
						DebufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
					}
				}
			}
			break;		
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_DEAFLUT_ATTACK:
			{
				CPlayer* Player = (CPlayer*)this;

				if (_SelectTarget != nullptr)
				{
					float Distance = st_Vector2::Distance(_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position, Player->_GameObjectInfo.ObjectPositionInfo.Position);
					if (Distance < 2.0f)
					{
						CSkill* DefaultAttackSkill = Player->_SkillBox.FindSkill(en_SkillType::SKILL_DEFAULT_ATTACK);
						if (DefaultAttackSkill != nullptr)
						{
							if (DefaultAttackSkill->GetSkillInfo()->CanSkillUse == true)
							{								
								Player->_OnPlayerDefaultAttack = true;
							}
						}
						else
						{
							CRASH("�⺻ ���� ��ų�� ��ų â���� �߰� ����");
						}
					}
					else
					{
						CMessage* FarDistanceErrorPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_FAR_DISTANCE);
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, FarDistanceErrorPacket);
						FarDistanceErrorPacket->Free();
					}
				}
				else
				{
					CMessage* NonSelectTargetPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT);
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, NonSelectTargetPacket);
					NonSelectTargetPacket->Free();
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SKILL_MELEE_ATTACK:
			{
				int16 MeleeSkillType;
				*GameObjectJob->GameObjectJobMessage >> MeleeSkillType;

				CPlayer* Player = (CPlayer*)this;

				if (Player->_ComboSkill != nullptr)
				{
					st_GameObjectJob* ComboAttackOffJob = G_ObjectManager->GameServer->MakeGameObjectJobComboSkillOff();
					_GameObjectJobQue.Enqueue(ComboAttackOffJob);
				}

				CSkill* FindMeleeSkill = Player->_SkillBox.FindSkill((en_SkillType)MeleeSkillType);
				if (FindMeleeSkill != nullptr)
				{
					st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)FindMeleeSkill->GetSkillInfo();

					if (FindMeleeSkill->GetSkillInfo()->CanSkillUse == true)
					{
						if (_SelectTarget != nullptr)
						{
							_MeleeSkill = FindMeleeSkill;

							vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldOfViewPlayers(this, 1, false);

							bool IsCritical = true;
							// ������ �Ǵ�
							int32 Damage = CMath::CalculateMeleeDamage(
								_SelectTarget->_GameObjectInfo.ObjectStatInfo.Defence,
								_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage + Player->_Equipment._WeaponMinDamage + AttackSkillInfo->SkillMinDamage,
								_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage + Player->_Equipment._WeaponMaxDamage + AttackSkillInfo->SkillMaxDamage,
								_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint, &IsCritical);

							st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(this, IsCritical, Damage, FindMeleeSkill->GetSkillInfo()->SkillType);
							_SelectTarget->_GameObjectJobQue.Enqueue(DamageJob);

							switch (FindMeleeSkill->GetSkillInfo()->SkillType)
							{
							case en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK:
							case en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK:							
								{
									if (AttackSkillInfo->NextComboSkill != en_SkillType::SKILL_TYPE_NONE)
									{
										CSkill* FindNextComboSkill = Player->_SkillBox.FindSkill(AttackSkillInfo->NextComboSkill);
										if (FindNextComboSkill->GetSkillInfo()->CanSkillUse == true)
										{
											st_GameObjectJob* ComboAttackCreateJob = G_ObjectManager->GameServer->MakeGameObjectJobComboSkillCreate(FindMeleeSkill);
											_GameObjectJobQue.Enqueue(ComboAttackCreateJob);
										}
									}
								}
								break;
							case en_SkillType::SKILL_KNIGHT_SHIELD_SMASH:								
								break;
							case en_SkillType::SKILL_KNIGHT_CHOHONE:
								{
									st_Vector2 TargetPosition = _SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position;
									st_Vector2 MyPosition = _GameObjectInfo.ObjectPositionInfo.Position;									

									float Distance = st_Vector2::Distance(TargetPosition, MyPosition);
									if (Distance <= 4.0f)
									{
										st_Vector2Int MyFrontCellPosition = GetFrontCellPosition(_GameObjectInfo.ObjectPositionInfo.MoveDir, 1);
										if (_Channel != nullptr)
										{
											if (_Channel->GetMap()->ApplyMove(_SelectTarget, MyFrontCellPosition))
											{
												CSkill* NewDeBufSkill = G_ObjectManager->SkillCreate();

												if (NewDeBufSkill != nullptr)
												{
													st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(FindMeleeSkill->GetSkillInfo()->SkillMediumCategory);
													*NewAttackSkillInfo = *((st_AttackSkillInfo*)FindMeleeSkill->GetSkillInfo());

													NewDeBufSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
													NewDeBufSkill->StatusAbnormalDurationTimeStart();

													_SelectTarget->AddDebuf(NewDeBufSkill);
													_SelectTarget->SetStatusAbnormal(STATUS_ABNORMAL_WARRIOR_CHOHONE);

													_SelectTarget->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

													CMessage* SelectTargetMoveStopMessage = G_ObjectManager->GameServer->MakePacketResMoveStop(_SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectPositionInfo);
													G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, SelectTargetMoveStopMessage);
													SelectTargetMoveStopMessage->Free();

													CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
														_SelectTarget->_GameObjectInfo.ObjectType,
														_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
														FindMeleeSkill->GetSkillInfo()->SkillType,
														true, STATUS_ABNORMAL_WARRIOR_CHOHONE);
													G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
													ResStatusAbnormalPacket->Free();

													CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewDeBufSkill->GetSkillInfo());
													G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
													ResBufDeBufSkillPacket->Free();

													CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(_SelectTarget->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_DEBUF_STUN, NewDeBufSkill->GetSkillInfo()->SkillDurationTime / 1000.0f);
													G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
													ResEffectPacket->Free();

													switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
													{
													case en_MoveDir::UP:
														_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._X = _GameObjectInfo.ObjectPositionInfo.Position._X;
														_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._Y = MyFrontCellPosition._Y + 0.5f;
														break;
													case en_MoveDir::DOWN:
														_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._X = _GameObjectInfo.ObjectPositionInfo.Position._X;
														_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._Y = MyFrontCellPosition._Y + 0.5f;
														break;
													case en_MoveDir::LEFT:
														_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._X = MyFrontCellPosition._X + 0.5f;
														_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._Y = _GameObjectInfo.ObjectPositionInfo.Position._Y;
														break;
													case en_MoveDir::RIGHT:
														_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._X = MyFrontCellPosition._X + 0.5f;
														_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._Y = _GameObjectInfo.ObjectPositionInfo.Position._Y;
														break;
													}

													CMessage* ResSyncPositionPacket = G_ObjectManager->GameServer->MakePacketResSyncPosition(_SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectPositionInfo);
													G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSyncPositionPacket);
													ResSyncPositionPacket->Free();
												}
											}
											else
											{
												CMessage* ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_PLACE_BLOCK, FindMeleeSkill->GetSkillInfo()->SkillName.c_str());
												G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResErrorPacket);
												ResErrorPacket->Free();
											}
										}
									}
								}
								break;
							case en_SkillType::SKILL_KNIGHT_SHAEHONE:
								{
									st_Vector2 TargetPosition = _SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position;
									st_Vector2 MyPosition = _GameObjectInfo.ObjectPositionInfo.Position;
									st_Vector2 Direction = TargetPosition - MyPosition;

									en_MoveDir Dir = st_Vector2::GetMoveDir(Direction);

									float Distance = st_Vector2::Distance(TargetPosition, MyPosition);
									if (Distance <= 4.0f)
									{
										st_Vector2Int MovePosition;

										switch (Dir)
										{
										case en_MoveDir::UP:
											MovePosition = _SelectTarget->GetFrontCellPosition(en_MoveDir::DOWN, 1);
											break;
										case en_MoveDir::DOWN:
											MovePosition = _SelectTarget->GetFrontCellPosition(en_MoveDir::UP, 1);
											break;
										case en_MoveDir::LEFT:
											MovePosition = _SelectTarget->GetFrontCellPosition(en_MoveDir::RIGHT, 1);
											break;
										case en_MoveDir::RIGHT:
											MovePosition = _SelectTarget->GetFrontCellPosition(en_MoveDir::LEFT, 1);
											break;										
										}

										if (_Channel->GetMap()->ApplyMove(this, MovePosition))
										{
											CSkill* NewDeBufSkill = G_ObjectManager->SkillCreate();

											st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(FindMeleeSkill->GetSkillInfo()->SkillMediumCategory);
											*AttackSkillInfo = *((st_AttackSkillInfo*)FindMeleeSkill->GetSkillInfo());

											NewDeBufSkill->SetSkillInfo(en_SkillCategory::STATUS_ABNORMAL_SKILL, AttackSkillInfo);
											NewDeBufSkill->StatusAbnormalDurationTimeStart();

											_SelectTarget->AddDebuf(NewDeBufSkill);
											_SelectTarget->SetStatusAbnormal(STATUS_ABNORMAL_WARRIOR_SHAEHONE);

											_SelectTarget->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

											CMessage* SelectTargetMoveStopMessage = G_ObjectManager->GameServer->MakePacketResMoveStop(_SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectPositionInfo);
											G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, SelectTargetMoveStopMessage);
											SelectTargetMoveStopMessage->Free();

											CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
												_SelectTarget->_GameObjectInfo.ObjectType,
												_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
												FindMeleeSkill->GetSkillInfo()->SkillType,
												true, STATUS_ABNORMAL_WARRIOR_SHAEHONE);
											G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
											ResStatusAbnormalPacket->Free();

											CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewDeBufSkill->GetSkillInfo());
											G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
											ResBufDeBufSkillPacket->Free();

											float EffectPrintTime = FindMeleeSkill->GetSkillInfo()->SkillDurationTime / 1000.0f;

											// ����Ʈ ���
											CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(_SelectTarget->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_DEBUF_ROOT, EffectPrintTime);
											G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
											ResEffectPacket->Free();											

											switch (Dir)
											{
											case en_MoveDir::UP:
												_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::UP;
												_GameObjectInfo.ObjectPositionInfo.Position._X = _SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._X;
												_GameObjectInfo.ObjectPositionInfo.Position._Y = MovePosition._Y + 0.5f;
												break;
											case en_MoveDir::DOWN:
												_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::DOWN;
												_GameObjectInfo.ObjectPositionInfo.Position._X = _SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._X;
												_GameObjectInfo.ObjectPositionInfo.Position._Y = MovePosition._Y + 0.5f;
												break;
											case en_MoveDir::LEFT:
												_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::LEFT;
												_GameObjectInfo.ObjectPositionInfo.Position._X = MovePosition._X + 0.5f;
												_GameObjectInfo.ObjectPositionInfo.Position._Y = _SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._Y;
												break;
											case en_MoveDir::RIGHT:
												_GameObjectInfo.ObjectPositionInfo.MoveDir = en_MoveDir::RIGHT;
												_GameObjectInfo.ObjectPositionInfo.Position._X = MovePosition._X + 0.5f;
												_GameObjectInfo.ObjectPositionInfo.Position._Y = _SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position._Y;
												break;
											}

											CMessage* ResSyncPositionPacket = G_ObjectManager->GameServer->MakePacketResSyncPosition(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo);
											G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSyncPositionPacket);
											ResSyncPositionPacket->Free();
										}
										else
										{
											CMessage* ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_PLACE_BLOCK, FindMeleeSkill->GetSkillInfo()->SkillName.c_str());
											G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResErrorPacket);
											ResErrorPacket->Free();
										}
									}
									else
									{
										CMessage* ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_PLACE_DISTANCE, FindMeleeSkill->GetSkillInfo()->SkillName.c_str(), Distance);
										G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResErrorPacket);
										ResErrorPacket->Free();
									}
								}
								break;
							case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
								{
									
								}
								break;
							}							

							CMessage* AnimationPlayPacket = G_ObjectManager->GameServer->MakePacketResAnimationPlay(_GameObjectInfo.ObjectId,_GameObjectInfo.ObjectPositionInfo.MoveDir,
								(*FindMeleeSkill->GetSkillInfo()->SkillAnimations.find(_GameObjectInfo.ObjectPositionInfo.MoveDir)).second);
							G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, AnimationPlayPacket);
							AnimationPlayPacket->Free();

							// ��Ÿ�� ����
							FindMeleeSkill->CoolTimeStart();

							// ��Ÿ�� ǥ�� ( ������ �ٿ� ��ϵǾ� �ִ� ���� ������ ��ų�� ��� ��Ÿ�� ǥ�� ���� �ش� )
							for (auto QuickSlotBarPosition : Player->_QuickSlotManager.FindQuickSlotBar(FindMeleeSkill->GetSkillInfo()->SkillType))
							{
								// Ŭ�󿡰� ��Ÿ�� ǥ��
								CMessage* ResCoolTimeStartPacket = G_ObjectManager->GameServer->MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
									QuickSlotBarPosition.QuickSlotBarSlotIndex,
									1.0f, FindMeleeSkill);
								G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCoolTimeStartPacket);
								ResCoolTimeStartPacket->Free();
							}

							// ��û�� ��ų�� �⺻ ���� ��ų�� �����ϰ� ��ų â���� ������
							vector<CSkill*> GlobalSkills = Player->_SkillBox.GetGlobalSkills(FindMeleeSkill->GetSkillInfo()->SkillType, FindMeleeSkill->GetSkillKind());
							
							// ���� ��Ÿ�� ����
							for (CSkill* GlobalSkill : GlobalSkills)
							{
								GlobalSkill->GlobalCoolTimeStart(FindMeleeSkill->GetSkillInfo()->SkillMotionTime);

								for (st_Vector2Int QuickSlotPosition : GlobalSkill->_QuickSlotBarPosition)
								{
									CMessage* ResCoolTimeStartPacket = G_ObjectManager->GameServer->MakePacketCoolTime((int8)QuickSlotPosition._Y,
										(int8)QuickSlotPosition._X,
										1.0f, nullptr, FindMeleeSkill->GetSkillInfo()->SkillMotionTime);
									G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCoolTimeStartPacket);
									ResCoolTimeStartPacket->Free();
								}
							}						
						}	
						else
						{
							CMessage* NonSelectTargetErrorPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT);
							G_ObjectManager->GameServer->SendPacket(Player->_SessionId, NonSelectTargetErrorPacket);
							NonSelectTargetErrorPacket->Free();
						}
					}
					else
					{
						G_Logger->WriteStdOut(en_Color::RED, L"%s ��ų�� ���� ��� �� �� ����\n",FindMeleeSkill->GetSkillInfo()->SkillName.c_str());
					}
				}
				else
				{
					CRASH("���� ���� ��ų�� ��ųâ���� �߰� ����");
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_COMBO_ATTACK_CREATE:
			{				
				CSkill* ReqMeleeSkill;
				*GameObjectJob->GameObjectJobMessage >> &ReqMeleeSkill;

				CPlayer* MyPlayer = ((CPlayer*)this);

				// ���ӱ� ��ų�� ��ųâ���� ã�´�.
				CSkill* FindComboSkill = MyPlayer->_SkillBox.FindSkill(ReqMeleeSkill->GetSkillInfo()->NextComboSkill);
				if (FindComboSkill != nullptr)
				{
					// ���ӱ� ��ų ����
					CSkill* NewComboSkill = G_ObjectManager->SkillCreate();				
					// ������ ����
					NewComboSkill->SetSkillInfo(en_SkillCategory::COMBO_SKILL, FindComboSkill->GetSkillInfo(), ReqMeleeSkill->GetSkillInfo());
					NewComboSkill->SetOwner(MyPlayer);

					// ���ӱ� ��ų ���� �Է�
					MyPlayer->_ComboSkill = NewComboSkill;
					
					NewComboSkill->ComboSkillStart(ReqMeleeSkill->_QuickSlotBarPosition, FindComboSkill->GetSkillInfo()->SkillType);

					CMessage* ResNextComboSkill = G_ObjectManager->GameServer->MakePacketComboSkillOn(ReqMeleeSkill->_QuickSlotBarPosition,
						*FindComboSkill->GetSkillInfo());
					G_ObjectManager->GameServer->SendPacket(MyPlayer->_SessionId, ResNextComboSkill);
					ResNextComboSkill->Free();
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_COMBO_ATTACK_OFF:
			{
				CPlayer* MyPlayer = ((CPlayer*)this);

				if (MyPlayer->_ComboSkill != nullptr)
				{
					MyPlayer->_ComboSkill->_ComboSkillTick = 0;
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SPELL_START:
			{
				CSkill* ReqSpellSkill;
				*GameObjectJob->GameObjectJobMessage >> &ReqSpellSkill;
						
				_SpellSkill = ReqSpellSkill;

				_SpellTick = GetTickCount64() + ReqSpellSkill->GetSkillInfo()->SkillCastingTime;

				_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPELL;

				CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
					_GameObjectInfo.ObjectPositionInfo.MoveDir,
					_GameObjectInfo.ObjectType,
					_GameObjectInfo.ObjectPositionInfo.State);
				G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResObjectStateChangePacket);
				ResObjectStateChangePacket->Free();
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SPELL_CANCEL:
			{
				CMessage* ResMagicCancelPacket = G_ObjectManager->GameServer->MakePacketMagicCancel(_GameObjectInfo.ObjectId);
				G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResMagicCancelPacket);
				ResMagicCancelPacket->Free();
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_GATHERING_START:
			{
				if (_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::GATHERING)
				{
					CGameObject* GatheringTarget;
					*GameObjectJob->GameObjectJobMessage >> &GatheringTarget;

					_GatheringTarget = GatheringTarget;

					_GatheringTick = GetTickCount64() + 1000;

					_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::GATHERING;

					CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectPositionInfo.MoveDir,
						_GameObjectInfo.ObjectType,
						_GameObjectInfo.ObjectPositionInfo.State);
					G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResObjectStateChangePacket);
					ResObjectStateChangePacket->Free();

					CMessage* ResGatheringPacket = nullptr;

					switch (GatheringTarget->_GameObjectInfo.ObjectType)
					{
					case en_GameObjectType::OBJECT_STONE:
						ResGatheringPacket = G_ObjectManager->GameServer->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"�� ä��");
						break;
					case en_GameObjectType::OBJECT_TREE:
						ResGatheringPacket = G_ObjectManager->GameServer->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"���� ����");
						break;
					case en_GameObjectType::OBJECT_CROP_POTATO:
						ResGatheringPacket = G_ObjectManager->GameServer->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"���� ��Ȯ");
						break;
					default:
						CRASH("ä���� �� ���� ä���� ä�� ��û");
						break;
					}

					G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResGatheringPacket);
					ResGatheringPacket->Free();
				}				
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_GATHERING_CANCEL:
			{
				CMessage* ResGatheringCancelPacket = G_ObjectManager->GameServer->MakePacketGatheringCancel(_GameObjectInfo.ObjectId);
				G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResGatheringCancelPacket);
				ResGatheringCancelPacket->Free();
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_AGGRO_LIST_INSERT_OR_UPDATE:
			{
				int8 AggroCategory;
				*GameObjectJob->GameObjectJobMessage >> AggroCategory;
				CGameObject* Target;
				*GameObjectJob->GameObjectJobMessage >> &Target;

				auto FindAggroTargetIterator = _AggroTargetList.find(Target->_GameObjectInfo.ObjectId);
				if (FindAggroTargetIterator != _AggroTargetList.end())
				{
					switch ((en_AggroCategory)AggroCategory)
					{
					case en_AggroCategory::AGGRO_CATEGORY_DAMAGE:
					{
						int32 Damage;
						*GameObjectJob->GameObjectJobMessage >> Damage;

						FindAggroTargetIterator->second.AggroPoint += (Damage * (0.8 + G_Datamanager->_MonsterAggroData.MonsterAggroAttacker));
					}
					break;
					case en_AggroCategory::AGGRO_CATEGORY_HEAL:
					{
						int32 HealPoint;
						*GameObjectJob->GameObjectJobMessage >> HealPoint;

						FindAggroTargetIterator->second.AggroPoint += (HealPoint * G_Datamanager->_MonsterAggroData.MonsterAggroHeal);
					}
					break;
					default:
						break;
					}
				}
				else
				{
					st_Aggro NewAggroTarget;
					NewAggroTarget.AggroTarget = Target;
					NewAggroTarget.AggroPoint = _GameObjectInfo.ObjectStatInfo.MaxHP * G_Datamanager->_MonsterAggroData.MonsterAggroFirstAttacker;

					_AggroTargetList.insert(pair<int64, st_Aggro>(Target->_GameObjectInfo.ObjectId, NewAggroTarget));
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_AGGRO_LIST_REMOVE:
			{
				int64 RemoveAggroListGameObjectId;
				*GameObjectJob->GameObjectJobMessage >> RemoveAggroListGameObjectId;

				auto FindAggroTargetIterator = _AggroTargetList.find(RemoveAggroListGameObjectId);
				if (FindAggroTargetIterator != _AggroTargetList.end())
				{
					_AggroTargetList.erase(RemoveAggroListGameObjectId);
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_DAMAGE:
			{
				CGameObject* Attacker;
				*GameObjectJob->GameObjectJobMessage >> &Attacker;

				bool IsCritical;
				*GameObjectJob->GameObjectJobMessage >> IsCritical;

				int32 Damage;
				*GameObjectJob->GameObjectJobMessage >> Damage;

				int16 SkillType;
				*GameObjectJob->GameObjectJobMessage >> SkillType;

				if (OnDamaged(Attacker, Damage))
				{
					End();
					
					ExperienceCalculate((CPlayer*)Attacker, this);
					// ĳ���� ������ ���� ����� â �����ؾ���
					Attacker->_SelectTarget = nullptr;	
				}

				en_EffectType HitEffectType = en_EffectType::EFFECT_TYPE_NONE;

				wstring SkillTypeString;
				wchar_t SkillTypeMessage[64] = L"0";
				wchar_t SkillDamageMessage[64] = L"0";

				switch ((en_SkillType)SkillType)
				{
				case en_SkillType::SKILL_TYPE_NONE:
					CRASH("SkillType None");
					break;
				case en_SkillType::SKILL_DEFAULT_ATTACK:
					wsprintf(SkillTypeMessage, L"%s�� �Ϲݰ����� ����� %s���� %d�� �������� ����ϴ�.", Attacker->_GameObjectInfo.ObjectName.c_str(), _GameObjectInfo.ObjectName.c_str(), Damage);
					HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
					break;
				case en_SkillType::SKILL_KNIGHT_FIERCE_ATTACK:
					wsprintf(SkillTypeMessage, L"%s�� �ͷ��� �ϰ��� ����� %s���� %d�� �������� ����ϴ�.", Attacker->_GameObjectInfo.ObjectName.c_str(), _GameObjectInfo.ObjectName.c_str(), Damage);
					HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
					break;
				case en_SkillType::SKILL_KNIGHT_CONVERSION_ATTACK:
					wsprintf(SkillTypeMessage, L"%s�� ȸ���� �ϰ��� ����� %s���� %d�� �������� ����ϴ�.", Attacker->_GameObjectInfo.ObjectName.c_str(), _GameObjectInfo.ObjectName.c_str(), Damage);
					HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
					break;
				case en_SkillType::SKILL_KNIGHT_CHOHONE:
					wsprintf(SkillTypeMessage, L"%s�� ��ȥ�񹫸� ����� %s���� %d�� �������� ����ϴ�.", Attacker->_GameObjectInfo.ObjectName.c_str(), _GameObjectInfo.ObjectName.c_str(), Damage);
					HitEffectType = en_EffectType::EFFECT_CHOHONE_TARGET_HIT;
					break;
				case en_SkillType::SKILL_KNIGHT_SHAEHONE:
					wsprintf(SkillTypeMessage, L"%s�� ��ȥ�񹫸� ����� %s���� %d�� �������� ����ϴ�.", Attacker->_GameObjectInfo.ObjectName.c_str(), _GameObjectInfo.ObjectName.c_str(), Damage);
					HitEffectType = en_EffectType::EFFECT_SHAHONE_TARGET_HIT;
					break;
				case en_SkillType::SKILL_KNIGHT_SMASH_WAVE:
					wsprintf(SkillTypeMessage, L"%s�� �м��ĵ��� ����� %s���� %d�� �������� ����ϴ�.", Attacker->_GameObjectInfo.ObjectName.c_str(), _GameObjectInfo.ObjectName.c_str(), Damage);
					HitEffectType = en_EffectType::EFFECT_NORMAL_ATTACK_TARGET_HIT;
					break;
				case en_SkillType::SKILL_KNIGHT_SHIELD_SMASH:
					wsprintf(SkillTypeMessage, L"%s�� ���а�Ÿ�� ����� %s���� %d�� �������� ����ϴ�.", Attacker->_GameObjectInfo.ObjectName.c_str(), _GameObjectInfo.ObjectName.c_str(), Damage);
					break;
				default:
					break;
				}

				SkillTypeString = SkillTypeMessage;

				vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldOfViewPlayers(this, 1, false);

				// ������ �ý��� �޼��� ����
				CMessage* ResSkillSystemMessagePacket = G_ObjectManager->GameServer->MakePacketResChattingBoxMessage(Attacker->_GameObjectInfo.ObjectId,
					en_MessageType::SYSTEM,
					IsCritical ? st_Color::Red() : st_Color::White(),
					SkillTypeString);
				G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSkillSystemMessagePacket);
				ResSkillSystemMessagePacket->Free();

				// ������ ���
				CMessage* ResDamagePacket = G_ObjectManager->GameServer->MakePacketResDamage(Attacker->_GameObjectInfo.ObjectId,
					_GameObjectInfo.ObjectId,
					(en_SkillType)SkillType,
					Damage,
					IsCritical);
				G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResDamagePacket);
				ResDamagePacket->Free();

				// ����� ü�� ����
				CMessage* StatChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectStatInfo);
				G_ObjectManager->GameServer->SendPacketFieldOfView(this, StatChangePacket);
				StatChangePacket->Free();
			}
			break;
		case en_GameObjectJobType::GAMEOJBECT_JOB_TYPE_HP_HEAL:
			{
				CGameObject* Healer;
				*GameObjectJob->GameObjectJobMessage >> &Healer;

				int32 HealPoint;
				*GameObjectJob->GameObjectJobMessage >> HealPoint;

				OnHeal(Healer, HealPoint);

				CMessage* StatChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectStatInfo);
				G_ObjectManager->GameServer->SendPacketFieldOfView(this, StatChangePacket);
				StatChangePacket->Free();
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_ON_EQUIPMENT:
			{
				// ������ ��� 
				CItem* EquipmentItem;
				*GameObjectJob->GameObjectJobMessage >> &EquipmentItem;				

				// ��� ����
				CPlayer* Player = (CPlayer*)this;
				CItem* ReturnEquipmentItem = Player->_Equipment.ItemOnEquipment(EquipmentItem);

				CMessage* EquipmentUpdateMessage = G_ObjectManager->GameServer->MakePacketOnEquipment(_GameObjectInfo.ObjectId, EquipmentItem->_ItemInfo);
				G_ObjectManager->GameServer->SendPacket(Player->_SessionId, EquipmentUpdateMessage);
				EquipmentUpdateMessage->Free();

				// ���濡�� ������ ��� ���ֱ�
				Player->_InventoryManager.InitItem(0, EquipmentItem->_ItemInfo.ItemTileGridPositionX, EquipmentItem->_ItemInfo.ItemTileGridPositionY);

				// ��� ������ �������� ���� ��� ���濡 �ش� �������� ���� ����
				if (ReturnEquipmentItem != nullptr)
				{
					Player->_InventoryManager.InsertItem(0, ReturnEquipmentItem);

					CMessage* ResItemToInventoryPacket = G_ObjectManager->GameServer->MakePacketResItemToInventory(Player->_GameObjectInfo.ObjectId,
						ReturnEquipmentItem->_ItemInfo, false, ReturnEquipmentItem->_ItemInfo.ItemCount);
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResItemToInventoryPacket);
					ResItemToInventoryPacket->Free();
				}
			}
			break;	
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_OFF_EQUIPMENT:
			{
				int8 EquipmentParts;
				*GameObjectJob->GameObjectJobMessage >> EquipmentParts;

				CPlayer* Player = (CPlayer*)this;
				CItem* ReturnEquipmentItem = Player->_Equipment.ItemOffEquipment((en_EquipmentParts)EquipmentParts);
				
				if (ReturnEquipmentItem != nullptr)
				{
					// Ŭ�� ���â���� ��� ����
					CMessage* OffEquipmentMessage = G_ObjectManager->GameServer->MakePacketOffEquipment(Player->_GameObjectInfo.ObjectId, ReturnEquipmentItem->_ItemInfo.ItemEquipmentPart);
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, OffEquipmentMessage);
					OffEquipmentMessage->Free();

					// Ŭ�� �κ��丮�� ��� ������ ������ ����
					Player->_InventoryManager.InsertItem(0, ReturnEquipmentItem);

					// Ŭ�󿡰� �˷���
					CMessage* ResItemToInventoryPacket = G_ObjectManager->GameServer->MakePacketResItemToInventory(Player->_GameObjectInfo.ObjectId,
						ReturnEquipmentItem->_ItemInfo, false, ReturnEquipmentItem->_ItemInfo.ItemCount);
					G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResItemToInventoryPacket);
					ResItemToInventoryPacket->Free();
				}
				else
				{
					CRASH("������ ��� ���µ� ������� ��û");
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_FULL_RECOVERY:
			{
				for (auto DebufSkillIter : _DeBufs)
				{
					if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_KNIGHT_CHOHONE
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SHAMAN_LIGHTNING_STRIKE
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SHAMAN_ICE_WAVE
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SHAMAN_ICE_CHAIN
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SHAMAN_ROOT
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_TAIOIST_ROOT)
					{
						DebufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
					}
				}

				_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
				_GameObjectInfo.ObjectStatInfo.MP = _GameObjectInfo.ObjectStatInfo.MaxMP;

				CMessage* StatChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectStatInfo);
				G_ObjectManager->GameServer->SendPacketFieldOfView(this, StatChangePacket);
				StatChangePacket->Free();
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_ITEM_DROP:
			{
				int16 DropItemType;
				*GameObjectJob->GameObjectJobMessage >> DropItemType;

				int32 DropItemCount;
				*GameObjectJob->GameObjectJobMessage >> DropItemCount;

				CPlayer* Player = (CPlayer*)this;

				// ���濡 �������� �ϴ� �������� �ִ��� Ȯ��
				CItem* FindDropItem = Player->_InventoryManager.FindInventoryItem(0, (en_SmallItemCategory)DropItemType);
				if (FindDropItem != nullptr)
				{				
					// ������ ������ �´��� Ȯ��
					if (FindDropItem->_ItemInfo.ItemCount >= DropItemCount)
					{
						G_ObjectManager->ObjectItemDropToSpawn(Player, Player->GetChannel(), (en_SmallItemCategory)DropItemType,
							DropItemCount);

						FindDropItem->_ItemInfo.ItemCount -= DropItemCount;

						if (FindDropItem->_ItemInfo.ItemCount < 0)
						{
							FindDropItem->_ItemInfo.ItemCount = 0;
						}

						CMessage* DropItemUpdatePacket = G_ObjectManager->GameServer->MakePacketInventoryItemUpdate(Player->_GameObjectInfo.ObjectId, FindDropItem->_ItemInfo);
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, DropItemUpdatePacket);
						DropItemUpdatePacket->Free();

						if (FindDropItem->_ItemInfo.ItemCount == 0)
						{
							Player->_InventoryManager.InitItem(0, FindDropItem->_ItemInfo.ItemTileGridPositionX, FindDropItem->_ItemInfo.ItemTileGridPositionY);
						}						
					}
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_ITEM_INVENTORY_SAVE:
			{
				CGameObject* Item;
				*GameObjectJob->GameObjectJobMessage >> &Item;
								
				CItem* InsertItem = (CItem*)Item;
				CPlayer* Player = (CPlayer*)this;

				int16 ItemEach = InsertItem->_ItemInfo.ItemCount;

				bool IsExistItem = false;

				switch (((CItem*)Item)->_ItemInfo.ItemSmallCategory)
				{
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_BRONZE_COIN:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_SLIVER_COIN:
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_GOLD_COIN:	
					{
						Player->_InventoryManager.InsertMoney(0, InsertItem);

						CMessage* ResMoneyToInventoryPacket = G_ObjectManager->GameServer->MakePacketResMoneyToInventory(Player->_GameObjectInfo.ObjectId,
							Player->_InventoryManager._GoldCoinCount,
							Player->_InventoryManager._SliverCoinCount,
							Player->_InventoryManager._BronzeCoinCount,
							InsertItem->_ItemInfo,
							ItemEach);
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResMoneyToInventoryPacket);
						ResMoneyToInventoryPacket->Free();
					}
					break;
				default:
					{
						CItem* FindItem = Player->_InventoryManager.FindInventoryItem(0, InsertItem->_ItemInfo.ItemSmallCategory);
						if (FindItem == nullptr)
						{
							CItem* NewItem = G_ObjectManager->GameServer->NewItemCrate(InsertItem->_ItemInfo);
							Player->_InventoryManager.InsertItem(0, NewItem);

							FindItem = Player->_InventoryManager.GetItem(0, NewItem->_ItemInfo.ItemTileGridPositionX, NewItem->_ItemInfo.ItemTileGridPositionY);
						}
						else
						{
							IsExistItem = true;
							FindItem->_ItemInfo.ItemCount += ItemEach;
						}

						CMessage* ResItemToInventoryPacket = G_ObjectManager->GameServer->MakePacketResItemToInventory(Player->_GameObjectInfo.ObjectId,
							FindItem->_ItemInfo, IsExistItem, ItemEach);
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResItemToInventoryPacket);
						ResItemToInventoryPacket->Free();
					}
					break;
				}		

				st_GameObjectJob* DeSpawnMonsterChannelJob = G_ObjectManager->GameServer->MakeGameObjectJobObjectDeSpawnObjectChannel(InsertItem);
				_Channel->_ChannelJobQue.Enqueue(DeSpawnMonsterChannelJob);

				st_GameObjectJob* LeaveChannerMonsterJob = G_ObjectManager->GameServer->MakeGameObjectJobLeaveChannel(InsertItem);
				_Channel->_ChannelJobQue.Enqueue(LeaveChannerMonsterJob);				
			}
			break;		
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_ITEM_ADD:
			{
				CGameObject* CraftingTableItemAddPlayerGO;
				*GameObjectJob->GameObjectJobMessage >> &CraftingTableItemAddPlayerGO;

				int16 AddItemSmallCategory;
				*GameObjectJob->GameObjectJobMessage >> AddItemSmallCategory;

				int16 AddItemCount;
				*GameObjectJob->GameObjectJobMessage >> AddItemCount;

				CCraftingTable* CraftingTable = (CCraftingTable*)this;

				CPlayer* CraftingTableItemAddPlayer = (CPlayer*)CraftingTableItemAddPlayerGO;

				// ���� �������� �ִ� ����� �κ��丮�� �ִ��� Ȯ��
				CItem* FindAddItem = CraftingTableItemAddPlayer->_InventoryManager.FindInventoryItem(0, (en_SmallItemCategory)AddItemSmallCategory);
				if (FindAddItem != nullptr && FindAddItem->_ItemInfo.ItemCount > 0)
				{
					// ���۹��� ������ ��
					st_CraftingTableRecipe CraftingTableRecipe = CraftingTable->GetCraftingTableRecipe();

					bool IsMaterial = false;

					// ���۴� ���۹� ������ ��
					for (CItem* Item : CraftingTableRecipe.CraftingTableCompleteItems)
					{
						// �����ϰ��� �ϴ� ���� �������� ã��
						if (Item->_ItemInfo.ItemSmallCategory == CraftingTable->_SelectCraftingItemType)
						{
							// ��û�� ������� ���� �������� ���� ������ ����Ȯ��
							for (st_CraftingMaterialItemInfo MaterialItemInfo : Item->_ItemInfo.Materials)
							{
								if (MaterialItemInfo.MaterialItemType == (en_SmallItemCategory)AddItemSmallCategory)
								{
									IsMaterial = true;
								}
							}

							break;
						}
					}

					if (IsMaterial == true)
					{
						CraftingTable->InputMaterialItem(FindAddItem, AddItemCount);

						FindAddItem->_ItemInfo.ItemCount -= AddItemCount;

						if (FindAddItem->_ItemInfo.ItemCount < 0)
						{
							FindAddItem->_ItemInfo.ItemCount = 0;							
						}

						CMessage* ResInventoryItemUpdatePacket = G_ObjectManager->GameServer->MakePacketInventoryItemUpdate(CraftingTableItemAddPlayer->_GameObjectInfo.ObjectId,
							FindAddItem->_ItemInfo);
						G_ObjectManager->GameServer->SendPacket(CraftingTableItemAddPlayer->_SessionId, ResInventoryItemUpdatePacket);
						ResInventoryItemUpdatePacket->Free();
						
						if (FindAddItem->_ItemInfo.ItemCount == 0)
						{
							CraftingTableItemAddPlayer->_InventoryManager.InitItem(0, FindAddItem->_ItemInfo.ItemTileGridPositionX, FindAddItem->_ItemInfo.ItemTileGridPositionY);
						}

						CMessage* ResCraftingTableAddItemPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableInput(_GameObjectInfo.ObjectId, CraftingTable->GetMaterialItems());
						G_ObjectManager->GameServer->SendPacket(CraftingTableItemAddPlayer->_SessionId, ResCraftingTableAddItemPacket);
						ResCraftingTableAddItemPacket->Free();
					}
					else
					{
						CMessage* WrongItemInputMessage = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSOANL_MESSAGE_CRAFTING_TABLE_MATERIAL_WRONG_ITEM_ADD);
						G_ObjectManager->GameServer->SendPacket(CraftingTableItemAddPlayer->_SessionId, WrongItemInputMessage);
						WrongItemInputMessage->Free();
					}
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_MATERIAL_ITEM_SUBTRACT:
			{
				CGameObject* CraftingTableMaterialItemSubtractPlayerGO;
				*GameObjectJob->GameObjectJobMessage >> &CraftingTableMaterialItemSubtractPlayerGO;

				int16 SubtractItemSmallCategory;
				*GameObjectJob->GameObjectJobMessage >> SubtractItemSmallCategory;

				int16 SubtractItemCount;
				*GameObjectJob->GameObjectJobMessage >> SubtractItemCount;

				CCraftingTable* CraftingTable = (CCraftingTable*)this;

				CPlayer* CraftingTableItemSubtractPlayer = (CPlayer*)CraftingTableMaterialItemSubtractPlayerGO;

				bool SubtractItemFind = false;

				for (CItem* CraftingTableCompleteItem : CraftingTable->GetCraftingTableRecipe().CraftingTableCompleteItems)
				{
					if (CraftingTableCompleteItem->_ItemInfo.ItemSmallCategory == CraftingTable->_SelectCraftingItemType)
					{
						for (auto MaterialItemIter : CraftingTable->GetMaterialItems())
						{
							if (MaterialItemIter.second->_ItemInfo.ItemSmallCategory == (en_SmallItemCategory)SubtractItemSmallCategory)
							{
								if (MaterialItemIter.second->_ItemInfo.ItemCount != 0)
								{
									SubtractItemFind = true;
									MaterialItemIter.second->_ItemInfo.ItemCount -= SubtractItemCount;
								}
								break;
							}
						}

						break;
					}
				}

				if (SubtractItemFind == true)
				{
					bool IsExistItem;
					CItem* InsertItem = CraftingTableItemSubtractPlayer->_InventoryManager.InsertItem(0, (en_SmallItemCategory)SubtractItemSmallCategory, SubtractItemCount, &IsExistItem);

					CMessage* InsertItemToInventoryPacket = G_ObjectManager->GameServer->MakePacketResItemToInventory(CraftingTableItemSubtractPlayer->_GameObjectInfo.ObjectId,
						InsertItem->_ItemInfo,
						IsExistItem,
						SubtractItemCount);
					G_ObjectManager->GameServer->SendPacket(CraftingTableItemSubtractPlayer->_SessionId, InsertItemToInventoryPacket);
					InsertItemToInventoryPacket->Free();

					CMessage* ResCraftingTableMaterialItemListPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableMaterialItemList(
						_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectType,
						CraftingTable->_SelectCraftingItemType,
						CraftingTable->GetMaterialItems());
					G_ObjectManager->GameServer->SendPacket(CraftingTableItemSubtractPlayer->_SessionId, ResCraftingTableMaterialItemListPacket);
					ResCraftingTableMaterialItemListPacket->Free();
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_COMPLETE_ITEM_SUBTRACT:
			{
				CGameObject* CraftingTableCompleteItemSubtractPlayerGO;
				*GameObjectJob->GameObjectJobMessage >> &CraftingTableCompleteItemSubtractPlayerGO;

				int16 SubtractItemSmallCategory;
				*GameObjectJob->GameObjectJobMessage >> SubtractItemSmallCategory;

				int16 SubtractItemCount;
				*GameObjectJob->GameObjectJobMessage >> SubtractItemCount;

				CCraftingTable* CraftingTable = (CCraftingTable*)this;

				CPlayer* CraftingTableItemSubtractPlayer = (CPlayer*)CraftingTableCompleteItemSubtractPlayerGO;

				bool SubtractItemFind = false;

				for (CItem* CraftingTableCompleteItem : CraftingTable->GetCraftingTableRecipe().CraftingTableCompleteItems)
				{
					if (CraftingTableCompleteItem->_ItemInfo.ItemSmallCategory == CraftingTable->_SelectCraftingItemType)
					{
						for (auto CompleteItemIter : CraftingTable->GetCompleteItems())
						{
							if (CompleteItemIter.second->_ItemInfo.ItemSmallCategory == (en_SmallItemCategory)SubtractItemSmallCategory)
							{
								if (CompleteItemIter.second->_ItemInfo.ItemCount != 0)
								{
									SubtractItemFind = true;
									CompleteItemIter.second->_ItemInfo.ItemCount -= SubtractItemCount;

									if (CompleteItemIter.second->_ItemInfo.ItemCount <= 0)
									{
										G_ObjectManager->ItemReturn(CompleteItemIter.second);
										map<en_SmallItemCategory, CItem*> CraftingTableCompleteItems = CraftingTable->GetCompleteItems();
										
										CraftingTableCompleteItems.erase(CompleteItemIter.first);
									}
								}
								break;
							}
						}

						break;
					}
				}

				if (SubtractItemFind == true)
				{
					bool IsExistItem;
					CItem* InsertItem = CraftingTableItemSubtractPlayer->_InventoryManager.InsertItem(0, (en_SmallItemCategory)SubtractItemSmallCategory, SubtractItemCount, &IsExistItem);

					CMessage* InsertItemToInventoryPacket = G_ObjectManager->GameServer->MakePacketResItemToInventory(CraftingTableItemSubtractPlayer->_GameObjectInfo.ObjectId,
						InsertItem->_ItemInfo,
						IsExistItem,
						SubtractItemCount);
					G_ObjectManager->GameServer->SendPacket(CraftingTableItemSubtractPlayer->_SessionId, InsertItemToInventoryPacket);
					InsertItemToInventoryPacket->Free();

					CMessage* ResCraftingTableMaterialItemListPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableCompleteItemList(
						_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectType,
						CraftingTable->GetCompleteItems());
					G_ObjectManager->GameServer->SendPacket(CraftingTableItemSubtractPlayer->_SessionId, ResCraftingTableMaterialItemListPacket);
					ResCraftingTableMaterialItemListPacket->Free();
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_CRAFTING_START:
			{
				CGameObject* CraftingStartObject;
				*GameObjectJob->GameObjectJobMessage >> &CraftingStartObject;

				if (_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::CRAFTING)
				{
					int16 CraftingCompleteItemType;
					*GameObjectJob->GameObjectJobMessage >> CraftingCompleteItemType;

					int16 CraftingCount;
					*GameObjectJob->GameObjectJobMessage >> CraftingCount;

					switch (_GameObjectInfo.ObjectType)
					{
					case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
					case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
						{
							CCraftingTable* CraftingTable = (CCraftingTable*)this;

							// �뱤�ΰ� ������ �ִ� ���۹��� �������
							st_CraftingTableRecipe CraftingTableRecipe = CraftingTable->GetCraftingTableRecipe();

							// ���۹� �߿��� ��û�� ������ Ÿ���� ���۹��� �ִ��� ã��
							for (CItem* CraftingCompleteItem : CraftingTableRecipe.CraftingTableCompleteItems)
							{
								if (CraftingCompleteItem->_ItemInfo.ItemSmallCategory == (en_SmallItemCategory)CraftingCompleteItemType)
								{
									bool IsCrafting = true;

									// ã���� ���濡 ���۹� ��ᰡ �ִ��� Ȯ����
									for (st_CraftingMaterialItemInfo CraftingMaterialItemInfo : CraftingCompleteItem->_ItemInfo.Materials)
									{
										// �������� �Ѱ� ���鋚 �ʿ��� ����� ������ ��´�.
										int16 OneReqMaterialcount = CraftingMaterialItemInfo.ItemCount;
										// Ŭ�� ��û�� ������ ���۰����� ������ ���� ������ ���ؼ� �� �ʿ䰳���� ���Ѵ�.
										int16 ReqCraftingItemTotalCount = CraftingCount * OneReqMaterialcount;

										// �κ��丮�� �������� �ּ� ��� �ʿ� ������ŭ �뱤�� ����ۿ� �ִ��� Ȯ���Ѵ�.
										if (!CraftingTable->FindMaterialItem(CraftingMaterialItemInfo.MaterialItemType, OneReqMaterialcount))
										{
											IsCrafting = false;
										}
									}

									// ��ᰡ ��� ���� ���
									if (IsCrafting == true)
									{
										CraftingTable->_CraftingStartCompleteItem = CraftingCompleteItem->_ItemInfo.ItemSmallCategory;										

										// ������ ���� ����
										CraftingCompleteItem->CraftingStart();

										CPlayer* Player = (CPlayer*)CraftingTable->_SelectedObject;
										CMessage* ResCraftingstartPacket = G_ObjectManager->GameServer->MakePacketResCraftingStart(
											_GameObjectInfo.ObjectId,
											CraftingCompleteItem->_ItemInfo);
										G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCraftingstartPacket);
										
										// �뱤�� ���� ���� ����
										CraftingTable->CraftingStart();
									}
									else
									{
										// ��� ���� ���� �޼��� ����
										CMessage* CraftingStartErrorPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_CRAFTING_TABLE_MATERIAL_COUNT_NOT_ENOUGH);
										G_ObjectManager->GameServer->SendPacket(((CPlayer*)CraftingStartObject)->_SessionId, CraftingStartErrorPacket);
										CraftingStartErrorPacket->Free();
									}

									break;
								}
							}							
						}
						break;					
					}
				}				
				else
				{
					CMessage* CraftingStartErrorPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_CRAFTING_TABLE_OVERLAP_CRAFTING_START);
					G_ObjectManager->GameServer->SendPacket(((CPlayer*)CraftingStartObject)->_SessionId, CraftingStartErrorPacket);
					CraftingStartErrorPacket->Free();
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CRAFTING_TABLE_CRAFTING_STOP:
			{
				CGameObject* CraftingStoptObject;
				*GameObjectJob->GameObjectJobMessage >> &CraftingStoptObject;

				CCraftingTable* CraftingTable = (CCraftingTable*)this;
				CraftingTable->CraftingStop();
				
				for (CItem* CraftingStopItem : CraftingTable->GetCraftingTableRecipe().CraftingTableCompleteItems)
				{
					CraftingStopItem->CraftingStop();

					CMessage* CraftingTableCraftingStopPacket = G_ObjectManager->GameServer->MakePacketResCraftingStop(
						_GameObjectInfo.ObjectId,
						CraftingStopItem->_ItemInfo);
					G_ObjectManager->GameServer->SendPacket(((CPlayer*)CraftingStoptObject)->_SessionId, CraftingTableCraftingStopPacket);
					CraftingTableCraftingStopPacket->Free();
				}
			}
			break;
		}

		if (GameObjectJob->GameObjectJobMessage != nullptr)
		{
			GameObjectJob->GameObjectJobMessage->Free();
		}

		G_ObjectManager->GameObjectJobReturn(GameObjectJob);
	}

	StatusAbnormalCheck();
}

bool CGameObject::OnDamaged(CGameObject* Attacker, int32 DamagePoint)
{
	_GameObjectInfo.ObjectStatInfo.HP -= DamagePoint;

	if (_GameObjectInfo.ObjectStatInfo.HP <= 0)
	{
		_GameObjectInfo.ObjectStatInfo.HP = 0;

		return true;
	}

	return false;
}

void CGameObject::OnHeal(CGameObject* Healer, int32 HealPoint)
{
	_GameObjectInfo.ObjectStatInfo.HP += HealPoint;

	if (_GameObjectInfo.ObjectStatInfo.HP >= _GameObjectInfo.ObjectStatInfo.MaxHP)
	{
		_GameObjectInfo.ObjectStatInfo.HP = _GameObjectInfo.ObjectStatInfo.MaxHP;
	}

	// ���� ��󿡰� ������ ���� ���͵鿡�� �� ��׷θ� ����ϱ� ���� �˷��ش�.
	vector<CMonster*> AroundMonsters = _Channel->GetMap()->GetAroundMonster(Healer, 1);
	for (CMonster* AroundMonster : AroundMonsters)
	{
		st_GameObjectJob* AggroUpdateJob = G_ObjectManager->GameObjectJobCreate();
		AggroUpdateJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_AGGRO_LIST_INSERT_OR_UPDATE;

		CGameServerMessage* AggroUpdateJobMessage = CGameServerMessage::GameServerMessageAlloc();
		AggroUpdateJobMessage->Clear();

		*AggroUpdateJobMessage << (int8)en_AggroCategory::AGGRO_CATEGORY_HEAL;
		*AggroUpdateJobMessage << &Healer;
		*AggroUpdateJobMessage << HealPoint;

		AggroUpdateJob->GameObjectJobMessage = AggroUpdateJobMessage;

		AroundMonster->_GameObjectJobQue.Enqueue(AggroUpdateJob);
	}
}

st_Vector2 CGameObject::PositionCheck(st_Vector2Int& CheckPosition)
{
	st_Vector2 ResultPosition;

	if (CheckPosition._Y > 0)
	{
		ResultPosition._Y =
			CheckPosition._Y + 0.5f;
	}
	else if (CheckPosition._Y == 0)
	{
		ResultPosition._Y =
			CheckPosition._Y;
	}
	else if (CheckPosition._Y < 0)
	{
		ResultPosition._Y =
			CheckPosition._Y - 0.5f;
	}

	if (CheckPosition._X > 0)
	{
		ResultPosition._X =
			CheckPosition._X + 0.5f;
	}
	else if (CheckPosition._X == 0)
	{
		ResultPosition._X =
			CheckPosition._X;
	}
	else if (CheckPosition._X < 0)
	{
		ResultPosition._X =
			CheckPosition._X - 0.5f;
	}

	return ResultPosition;
}

void CGameObject::PositionReset()
{
}

st_PositionInfo CGameObject::GetPositionInfo()
{
	return _GameObjectInfo.ObjectPositionInfo;
}

st_Vector2Int CGameObject::GetFrontCellPosition(en_MoveDir Dir, int8 Distance)
{
	st_Vector2Int FrontPosition = _GameObjectInfo.ObjectPositionInfo.CollisionPosition;

	st_Vector2Int DirVector;
	switch (Dir)
	{
	case en_MoveDir::UP:
		DirVector = st_Vector2Int::Up() * Distance;
		break;
	case en_MoveDir::DOWN:
		DirVector = st_Vector2Int::Down() * Distance;
		break;
	case en_MoveDir::LEFT:
		DirVector = st_Vector2Int::Left() * Distance;
		break;
	case en_MoveDir::RIGHT:
		DirVector = st_Vector2Int::Right() * Distance;
		break;
	}

	FrontPosition = FrontPosition + DirVector;

	return FrontPosition;
}

vector<st_Vector2Int> CGameObject::GetAroundCellPositions(st_Vector2Int CellPosition, int8 Distance)
{
	vector<st_Vector2Int> AroundPosition;

	st_Vector2Int LeftTop(Distance * -1, Distance);
	st_Vector2Int RightDown(Distance, Distance * -1);

	st_Vector2Int LeftTopPosition = CellPosition + LeftTop;
	st_Vector2Int RightDownPosition = CellPosition + RightDown;

	for (int32 Y = LeftTopPosition._Y; Y >= RightDownPosition._Y; Y--)
	{
		for (int32 X = LeftTopPosition._X; X <= RightDownPosition._X; X++)
		{
			if (X == CellPosition._X && Y == CellPosition._Y)
			{
				continue;
			}

			AroundPosition.push_back(st_Vector2Int(X, Y));
		}
	}

	return AroundPosition;
}

void CGameObject::SetOwner(CGameObject* Target)
{
	_Owner = Target;
}

CGameObject* CGameObject::GetTarget()
{
	return _Owner;
}

void CGameObject::AddBuf(CSkill* Buf)
{
	Buf->SetOwner(this);

	_Bufs.insert(pair<en_SkillType, CSkill*>(Buf->GetSkillInfo()->SkillType, Buf));
}

void CGameObject::DeleteBuf(en_SkillType DeleteBufSkillType)
{
	_Bufs.erase(DeleteBufSkillType);
}

void CGameObject::AddDebuf(CSkill* DeBuf)
{
	DeBuf->SetOwner(this);

	_DeBufs.insert(pair<en_SkillType, CSkill*>(DeBuf->GetSkillInfo()->SkillType, DeBuf));
}

void CGameObject::DeleteDebuf(en_SkillType DeleteDebufSkillType)
{
	_DeBufs.erase(DeleteDebufSkillType);
}

void CGameObject::SetStatusAbnormal(int8 StatusAbnormalValue)
{
	_StatusAbnormal |= StatusAbnormalValue;
}

void CGameObject::ReleaseStatusAbnormal(int8 StatusAbnormalValue)
{
	_StatusAbnormal &= StatusAbnormalValue;
}

int8 CGameObject::CheckStatusAbnormal()
{
	int8 StatusAbnormalCount = 0;

	if (_StatusAbnormal & STATUS_ABNORMAL_WARRIOR_CHOHONE)
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & STATUS_ABNORMAL_WARRIOR_SHAEHONE)
	{
		StatusAbnormalCount++;
	}

	if ((_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ROOT)
		|| (_StatusAbnormal & STATUS_ABNORMAL_TAIOIST_ROOT))
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_ICE_WAVE)
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & STATUS_ABNORMAL_SHAMAN_LIGHTNING_STRIKE)
	{
		StatusAbnormalCount++;
	}

	return StatusAbnormalCount;
}

CChannel* CGameObject::GetChannel()
{
	return _Channel;
}

void CGameObject::SetChannel(CChannel* Channel)
{
	_Channel = Channel;
}

CRectCollision* CGameObject::GetRectCollision()
{
	return _RectCollision;
}

void CGameObject::Start()
{
}

void CGameObject::End()
{
	for (auto BufSkillIter : _Bufs)
	{
		BufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
	}

	for (auto DebufSkillIter : _DeBufs)
	{
		DebufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
	}
}

void CGameObject::ExperienceCalculate(CPlayer* TargetPlayer, CGameObject* TargetObject)
{
	CMonster* TargetMonster = nullptr;	

	switch (TargetObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_SLIME:
		TargetMonster = (CMonster*)TargetObject;

		TargetPlayer->_Experience.CurrentExperience += TargetMonster->_GetExpPoint;
		TargetPlayer->_Experience.CurrentExpRatio = ((float)TargetPlayer->_Experience.CurrentExperience) / TargetPlayer->_Experience.RequireExperience;

		if (TargetPlayer->_Experience.CurrentExpRatio >= 1.0f)
		{
			// ���� ����
			TargetPlayer->_GameObjectInfo.ObjectStatInfo.Level += 1;

			// ������ ������ �ش��ϴ� �ɷ�ġ ������ �о�� �� �����Ѵ�.
			st_ObjectStatusData NewCharacterStatus;
			st_LevelData LevelData;

			switch (TargetPlayer->_GameObjectInfo.ObjectType)
			{
			case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
			{
				auto FindStatus = G_Datamanager->_WarriorStatus.find(TargetPlayer->_GameObjectInfo.ObjectStatInfo.Level);
				if (FindStatus == G_Datamanager->_WarriorStatus.end())
				{
					CRASH("���� �������ͽ� ã�� ����");
				}

				NewCharacterStatus = *(*FindStatus).second;

				TargetPlayer->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxSpeed = NewCharacterStatus.Speed;
			}
			break;
			case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
			{
				auto FindStatus = G_Datamanager->_ShamanStatus.find(TargetPlayer->_GameObjectInfo.ObjectStatInfo.Level);
				if (FindStatus == G_Datamanager->_WarriorStatus.end())
				{
					CRASH("���� ������ ã�� ����");
				}

				NewCharacterStatus = *(*FindStatus).second;

				TargetPlayer->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxSpeed = NewCharacterStatus.Speed;
			}
			break;
			case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
			{
				auto FindStatus = G_Datamanager->_TaioistStatus.find(TargetPlayer->_GameObjectInfo.ObjectStatInfo.Level);
				if (FindStatus == G_Datamanager->_TaioistStatus.end())
				{
					CRASH("���� ������ ã�� ����");
				}

				NewCharacterStatus = *(*FindStatus).second;

				TargetPlayer->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxSpeed = NewCharacterStatus.Speed;
			}
			break;
			case en_GameObjectType::OBJECT_THIEF_PLAYER:
			{
				auto FindStatus = G_Datamanager->_ThiefStatus.find(TargetPlayer->_GameObjectInfo.ObjectStatInfo.Level);
				if (FindStatus == G_Datamanager->_ThiefStatus.end())
				{
					CRASH("���� ������ ã�� ����");
				}

				NewCharacterStatus = *(*FindStatus).second;

				TargetPlayer->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxSpeed = NewCharacterStatus.Speed;
			}
			break;
			case en_GameObjectType::OBJECT_ARCHER_PLAYER:
			{
				auto FindStatus = G_Datamanager->_ArcherStatus.find(TargetPlayer->_GameObjectInfo.ObjectStatInfo.Level);
				if (FindStatus == G_Datamanager->_ArcherStatus.end())
				{
					CRASH("���� ������ ã�� ����");
				}

				NewCharacterStatus = *(*FindStatus).second;

				TargetPlayer->_GameObjectInfo.ObjectStatInfo.HP = NewCharacterStatus.MaxHP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxHP = NewCharacterStatus.MaxHP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MP = NewCharacterStatus.MaxMP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxMP = NewCharacterStatus.MaxMP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.DP = NewCharacterStatus.DP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxDP = NewCharacterStatus.MaxDP;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent = NewCharacterStatus.AutoRecoveryHPPercent;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent = NewCharacterStatus.AutoRecoveryMPPercent;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage = NewCharacterStatus.MinMeleeAttackDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage = NewCharacterStatus.MaxMeleeAttackDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate = NewCharacterStatus.MeleeAttackHitRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicDamage = NewCharacterStatus.MagicDamage;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicHitRate = NewCharacterStatus.MagicHitRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.Defence = NewCharacterStatus.Defence;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.EvasionRate = NewCharacterStatus.EvasionRate;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint = NewCharacterStatus.MeleeCriticalPoint;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MagicCriticalPoint = NewCharacterStatus.MagicCriticalPoint;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.Speed = NewCharacterStatus.Speed;
				TargetPlayer->_GameObjectInfo.ObjectStatInfo.MaxSpeed = NewCharacterStatus.Speed;
			}
			break;
			}

			CGameServerMessage* ResObjectStatChangeMessage = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(TargetPlayer->_GameObjectInfo.ObjectId, TargetPlayer->_GameObjectInfo.ObjectStatInfo);
			G_ObjectManager->GameServer->SendPacket(TargetPlayer->_SessionId, ResObjectStatChangeMessage);
			ResObjectStatChangeMessage->Free();

			auto FindLevelData = G_Datamanager->_LevelDatas.find(TargetPlayer->_GameObjectInfo.ObjectStatInfo.Level);
			if (FindLevelData == G_Datamanager->_LevelDatas.end())
			{
				CRASH("���� ������ ã�� ����");
			}

			LevelData = *(*FindLevelData).second;

			TargetPlayer->_Experience.CurrentExperience = 0;
			TargetPlayer->_Experience.RequireExperience = LevelData.RequireExperience;
			TargetPlayer->_Experience.TotalExperience = LevelData.TotalExperience;
		}

		{
			CGameServerMessage* ResMonsterGetExpMessage = G_ObjectManager->GameServer->MakePacketExperience(TargetPlayer->_AccountId, TargetPlayer->_GameObjectInfo.ObjectId, TargetMonster->_GetExpPoint,
				TargetPlayer->_Experience.CurrentExperience,
				TargetPlayer->_Experience.RequireExperience,
				TargetPlayer->_Experience.TotalExperience);
			G_ObjectManager->GameServer->SendPacket(TargetPlayer->_SessionId, ResMonsterGetExpMessage);
			ResMonsterGetExpMessage->Free();
		}
		break;
	case en_GameObjectType::OBJECT_TREE:
		break;
	case en_GameObjectType::OBJECT_STONE:
		break;
	}
}

bool CGameObject::UpdateSpawnIdle()
{
	if (_SpawnIdleTick > GetTickCount64())
	{
		return false;
	}

	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	return true;
}

void CGameObject::UpdateIdle()
{
}

void CGameObject::UpdatePatrol()
{
}

void CGameObject::UpdateMoving()
{
}

void CGameObject::UpdateReturnSpawnPosition()
{
}

void CGameObject::UpdateAttack()
{
}

void CGameObject::UpdateSpell()
{
}

void CGameObject::UpdateGathering()
{
}

void CGameObject::UpdateCrafting()
{
}

void CGameObject::UpdateReadyDead()
{
}

void CGameObject::UpdateDead()
{
}
