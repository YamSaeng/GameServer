#include "pch.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "Skill.h"
#include "Furnace.h"
#include "Crop.h"

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

void CGameObject::PushedOutStatusAbnormalCheck()
{
	bool IsShamanIceWave = _StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE;
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
		case en_MoveDir::LEFT_UP:
			_GameObjectInfo.ObjectPositionInfo.Position._Y +=
				(st_Vector2::Down()._Y * 4.0f * 0.02f);
			_GameObjectInfo.ObjectPositionInfo.Position._X +=
				(st_Vector2::Right()._X * 4.0f * 0.02f);
			break;
		case en_MoveDir::LEFT_DOWN:
			_GameObjectInfo.ObjectPositionInfo.Position._Y +=
				(st_Vector2::Up()._Y * 4.0f * 0.02f);
			_GameObjectInfo.ObjectPositionInfo.Position._X +=
				(st_Vector2::Right()._X * 4.0f * 0.02f);
			break;
		case en_MoveDir::RIGHT_UP:
			_GameObjectInfo.ObjectPositionInfo.Position._Y +=
				(st_Vector2::Down()._Y * 4.0f * 0.02f);
			_GameObjectInfo.ObjectPositionInfo.Position._X +=
				(st_Vector2::Left()._X * 4.0f * 0.02f);
			break;
		case en_MoveDir::RIGHT_DOWN:
			_GameObjectInfo.ObjectPositionInfo.Position._Y +=
				(st_Vector2::Up()._Y * 4.0f * 0.02f);
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SELECT_SKILL_CHARACTERISTIC:
			{
				int8 SelectSkillCharacterIndex;
				*GameObjectJob->GameObjectJobMessage >> SelectSkillCharacterIndex;

				int8 SkillCharacteristicType;
				*GameObjectJob->GameObjectJobMessage >> SkillCharacteristicType;

				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if (Player != nullptr)
				{
					if (Player->_SkillBox.CheckCharacteristic((en_SkillCharacteristic)SkillCharacteristicType) == true)
					{
						CMessage* ResReqCancel = G_ObjectManager->GameServer->MakePacketReqCancel(en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_SELECT_SKILL_CHARACTERISTIC);
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResReqCancel);
						ResReqCancel->Free();
					}
					else
					{
						Player->_SkillBox.CreateChracteristic(SelectSkillCharacterIndex, SkillCharacteristicType);

						CSkillCharacteristic* SkillCharacteristic = Player->_SkillBox.FindCharacteristic(SelectSkillCharacterIndex, SkillCharacteristicType);

						CMessage* ResSkillCharacteristicPacket = G_ObjectManager->GameServer->MakePacketResSelectSkillCharacteristic(true, SelectSkillCharacterIndex,
							SkillCharacteristicType,
							SkillCharacteristic->GetPassiveSkill(), SkillCharacteristic->GetActiveSkill());
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSkillCharacteristicPacket);
						ResSkillCharacteristicPacket->Free();
					}
				}			
				else
				{
					CRASH("Player Casting Fail");
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SKILL_LEARN:
			{
				bool IsSkillLearn;
				*GameObjectJob->GameObjectJobMessage >> IsSkillLearn;

				int8 SkillCharacteristicIndex;
				*GameObjectJob->GameObjectJobMessage >> SkillCharacteristicIndex;

				int8 LearnSkillChracteristicType;
				*GameObjectJob->GameObjectJobMessage >> LearnSkillChracteristicType;

				int8 LearnSkillType;
				*GameObjectJob->GameObjectJobMessage >> LearnSkillType;

				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if (Player != nullptr)
				{
					CSkill* Skill = Player->_SkillBox.FindSkill((en_SkillCharacteristic)LearnSkillChracteristicType, (en_SkillType)LearnSkillType);
					if (Skill != nullptr)
					{
						if (IsSkillLearn == true)
						{
							if (Skill->GetSkillInfo()->IsSkillLearn == false)
							{
								if (Player->_GameObjectInfo.ObjectSkillPoint > 0)
								{
									Player->_SkillBox.SkillLearn(IsSkillLearn, SkillCharacteristicIndex, LearnSkillType);
									Player->_GameObjectInfo.ObjectSkillPoint--;
								}
								else
								{
									IsSkillLearn = false;
								}
							}

							CMessage* ResSkillLearnPacket = G_ObjectManager->GameServer->MakePacketResSkillLearn(IsSkillLearn, (en_SkillType)LearnSkillType, Player->_GameObjectInfo.ObjectSkillMaxPoint, Player->_GameObjectInfo.ObjectSkillPoint);
							G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSkillLearnPacket);
							ResSkillLearnPacket->Free();
						}
						else
						{
							if (Skill->GetSkillInfo()->CanSkillUse == true)
							{
								if (Skill->GetSkillInfo()->IsSkillLearn == true)
								{
									if (Player->_GameObjectInfo.ObjectSkillPoint < Player->_GameObjectInfo.ObjectSkillMaxPoint)
									{
										// �����Կ��� ����ϰ��� �ϴ½�ų�� ã�� ������ ��� ����
										vector<st_QuickSlotBarSlotInfo*> QuickSlotBars = Player->_QuickSlotManager.FindQuickSlotBarInfo((en_SkillType)LearnSkillType);
										for (auto QuickSlotBar : QuickSlotBars)
										{
											for (auto QuickSlotPositionIter = Skill->_QuickSlotBarPosition.begin();
												QuickSlotPositionIter != Skill->_QuickSlotBarPosition.end();
												++QuickSlotPositionIter)
											{
												st_Vector2Int QuickSlotPosition = *QuickSlotPositionIter;

												if (QuickSlotPosition._Y == QuickSlotBar->QuickSlotBarIndex && QuickSlotPosition._X == QuickSlotBar->QuickSlotBarSlotIndex)
												{
													QuickSlotBar->QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_NONE;
													QuickSlotBar->QuickBarSkill = nullptr;
													QuickSlotBar->QuickBarItem = nullptr;

													Skill->_QuickSlotBarPosition.erase(QuickSlotPositionIter);

													CMessage* ResQuickSlotInitMessage = G_ObjectManager->GameServer->MakePacketResQuickSlotInit(QuickSlotBar->QuickSlotBarIndex,
														QuickSlotBar->QuickSlotBarSlotIndex);
													G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResQuickSlotInitMessage);
													ResQuickSlotInitMessage->Free();

													break;
												}
											}
										}

										Player->_SkillBox.SkillLearn(IsSkillLearn, SkillCharacteristicIndex, LearnSkillType);
										Player->_GameObjectInfo.ObjectSkillPoint++;
									}
									else
									{
										IsSkillLearn = false;
									}
								}

								CMessage* ResSkillLearnPacket = G_ObjectManager->GameServer->MakePacketResSkillLearn(IsSkillLearn, (en_SkillType)LearnSkillType, Player->_GameObjectInfo.ObjectSkillMaxPoint, Player->_GameObjectInfo.ObjectSkillPoint);
								G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSkillLearnPacket);
								ResSkillLearnPacket->Free();
							}
							else
							{
								CMessage* ResSkillCancelFailtPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_SKILL_CANCEL_FAIL_COOLTIME, Skill->GetSkillInfo()->SkillName.c_str());
								G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSkillCancelFailtPacket);
								ResSkillCancelFailtPacket->Free();
							}							
						}
					}					
				}	
				else
				{
					CRASH("Player Casting Fail");
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_DEAFLUT_ATTACK:
			{
				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if (Player != nullptr)
				{
					if (_SelectTarget != nullptr)
					{
						float Distance = st_Vector2::Distance(_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position, Player->_GameObjectInfo.ObjectPositionInfo.Position);
						if (Distance < 2.0f)
						{
							CSkill* DefaultAttackSkill = Player->_SkillBox.FindSkill(en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC, en_SkillType::SKILL_DEFAULT_ATTACK);
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
				else
				{
					CRASH("Player Casting Fail");
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SKILL_MELEE_ATTACK:
			{
				int8 MeleeChracteristicType;
				*GameObjectJob->GameObjectJobMessage >> MeleeChracteristicType;

				int16 MeleeSkillType;
				*GameObjectJob->GameObjectJobMessage >> MeleeSkillType;

				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if(Player != nullptr)
				{
					CSkill* FindMeleeSkill = Player->_SkillBox.FindSkill((en_SkillCharacteristic)MeleeChracteristicType, (en_SkillType)MeleeSkillType);
					if (FindMeleeSkill != nullptr)
					{						
						st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)FindMeleeSkill->GetSkillInfo();

						if (FindMeleeSkill->GetSkillInfo()->CanSkillUse == true)
						{
							_MeleeSkill = FindMeleeSkill;

							vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldOfViewPlayers(this, 1, false);						

							// ������ ������ ����
							vector<CGameObject*> DamageTargets;

							switch (FindMeleeSkill->GetSkillInfo()->SkillType)
							{
							case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_FIERCE_ATTACK:
							case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CONVERSION_ATTACK:
								{			
									if (_SelectTarget != nullptr)
									{
										if (AttackSkillInfo->NextComboSkill != en_SkillType::SKILL_TYPE_NONE)
										{
											CSkill* FindNextComboSkill = Player->_SkillBox.FindSkill(AttackSkillInfo->SkillCharacteristic, AttackSkillInfo->NextComboSkill);
											if (FindNextComboSkill->GetSkillInfo()->CanSkillUse == true)
											{
												st_GameObjectJob* ComboAttackCreateJob = G_ObjectManager->GameServer->MakeGameObjectJobComboSkillCreate(FindMeleeSkill);
												_GameObjectJobQue.Enqueue(ComboAttackCreateJob);
											}
										}

										DamageTargets.push_back(_SelectTarget);
									}
									else
									{
										CMessage* NonSelectTargetErrorPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT);
										G_ObjectManager->GameServer->SendPacket(Player->_SessionId, NonSelectTargetErrorPacket);
										NonSelectTargetErrorPacket->Free();
									}									
								}
								break;
							case en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_SHIELD_SMASH:
								{
									if (_SelectTarget != nullptr)
									{
										CSkill* NewDeBufSkill = G_ObjectManager->SkillCreate();
										if (NewDeBufSkill != nullptr)
										{
											st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(FindMeleeSkill->GetSkillInfo()->SkillType, FindMeleeSkill->GetSkillInfo()->SkillLevel);
											NewDeBufSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
											NewDeBufSkill->StatusAbnormalDurationTimeStart();

											_SelectTarget->AddDebuf(NewDeBufSkill);
											_SelectTarget->SetStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_SHIELD_SMASH);

											CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
												_SelectTarget->_GameObjectInfo.ObjectType,
												_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
												FindMeleeSkill->GetSkillInfo()->SkillType,
												true, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_SHIELD_SMASH);
											G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
											ResStatusAbnormalPacket->Free();

											CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewDeBufSkill->GetSkillInfo());
											G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
											ResBufDeBufSkillPacket->Free();
										}

										DamageTargets.push_back(_SelectTarget);
									}
									else
									{
										CMessage* NonSelectTargetErrorPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT);
										G_ObjectManager->GameServer->SendPacket(Player->_SessionId, NonSelectTargetErrorPacket);
										NonSelectTargetErrorPacket->Free();
									}									
								}
								break;
							case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE:
								{
									if (_SelectTarget != nullptr)
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
													DamageTargets.push_back(_SelectTarget);

													CSkill* NewDeBufSkill = G_ObjectManager->SkillCreate();

													if (NewDeBufSkill != nullptr)
													{
														st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(FindMeleeSkill->GetSkillInfo()->SkillType, FindMeleeSkill->GetSkillInfo()->SkillLevel);
														NewDeBufSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
														NewDeBufSkill->StatusAbnormalDurationTimeStart();

														_SelectTarget->AddDebuf(NewDeBufSkill);
														_SelectTarget->SetStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_CHOHONE);

														_SelectTarget->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

														CMessage* SelectTargetMoveStopMessage = G_ObjectManager->GameServer->MakePacketResMoveStop(_SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectPositionInfo);
														G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, SelectTargetMoveStopMessage);
														SelectTargetMoveStopMessage->Free();

														CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
															_SelectTarget->_GameObjectInfo.ObjectType,
															_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
															FindMeleeSkill->GetSkillInfo()->SkillType,
															true, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_CHOHONE);
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

											DamageTargets.push_back(_SelectTarget);
										}
									}	
									else
									{
										CMessage* NonSelectTargetErrorPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT);
										G_ObjectManager->GameServer->SendPacket(Player->_SessionId, NonSelectTargetErrorPacket);
										NonSelectTargetErrorPacket->Free();
									}
								}
								break;
							case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SHAHONE:
								{
									if (_SelectTarget != nullptr)
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
												DamageTargets.push_back(_SelectTarget);

												CSkill* NewDeBufSkill = G_ObjectManager->SkillCreate();

												st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(FindMeleeSkill->GetSkillInfo()->SkillType, FindMeleeSkill->GetSkillInfo()->SkillLevel);
												NewDeBufSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, AttackSkillInfo);
												NewDeBufSkill->StatusAbnormalDurationTimeStart();

												_SelectTarget->AddDebuf(NewDeBufSkill);
												_SelectTarget->SetStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_SHAEHONE);

												_SelectTarget->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

												CMessage* SelectTargetMoveStopMessage = G_ObjectManager->GameServer->MakePacketResMoveStop(_SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectPositionInfo);
												G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, SelectTargetMoveStopMessage);
												SelectTargetMoveStopMessage->Free();

												CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
													_SelectTarget->_GameObjectInfo.ObjectType,
													_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
													FindMeleeSkill->GetSkillInfo()->SkillType,
													true, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_SHAEHONE);
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
									else
									{
										CMessage* NonSelectTargetErrorPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT);
										G_ObjectManager->GameServer->SendPacket(Player->_SessionId, NonSelectTargetErrorPacket);
										NonSelectTargetErrorPacket->Free();
									}
								}
								break;
							case en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_SMASH_WAVE:
								{
									CChannel* Channel = Player->GetChannel();
									if (Channel != nullptr)
									{
										vector<CGameObject*> RangeAttackObjects = Channel->FindRangeAttackChannelObjects(Player, 2);
										for (CGameObject* RangeAttackObject : RangeAttackObjects)
										{
											DamageTargets.push_back(RangeAttackObject);
										}
									}
								}
								break;
							}

							for (CGameObject* DamageTarget : DamageTargets)
							{
								bool IsCritical = true;

								// ������ �Ǵ�
								int32 Damage = CMath::CalculateMeleeDamage(&IsCritical,
									DamageTarget->_GameObjectInfo.ObjectStatInfo.Defence,
									_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage + Player->_Equipment._WeaponMinDamage + AttackSkillInfo->SkillMinDamage,
									_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage + Player->_Equipment._WeaponMaxDamage + AttackSkillInfo->SkillMaxDamage,
									_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint);

								st_GameObjectJob* DamageJob = G_ObjectManager->GameServer->MakeGameObjectDamage(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectType, IsCritical, Damage, FindMeleeSkill->GetSkillInfo()->SkillType);
								DamageTarget->_GameObjectJobQue.Enqueue(DamageJob);
							}

							CMessage* AnimationPlayPacket = G_ObjectManager->GameServer->MakePacketResAnimationPlay(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo.MoveDir,
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
							G_Logger->WriteStdOut(en_Color::RED, L"%s ��ų�� ���� ��� �� �� ����\n", FindMeleeSkill->GetSkillInfo()->SkillName.c_str());
						}						

						// �Ϲ� ���� ��Ÿ���� ���� ����
						CSkill* DefaultAttackSkill = Player->_SkillBox.FindSkill(en_SkillCharacteristic::SKILL_CATEGORY_PUBLIC, en_SkillType::SKILL_DEFAULT_ATTACK);
						if (DefaultAttackSkill != nullptr)
						{
							Player->_OnPlayerDefaultAttack = true;

							Player->_DefaultAttackTick = GetTickCount64() + Player->_GameObjectInfo.ObjectStatInfo.MeleeAttackHitRate;

							DefaultAttackSkill->CoolTimeStart();

							// ��Ÿ�� ǥ�� ( ������ �ٿ� ��ϵǾ� �ִ� ���� ������ ��ų�� ��� ��Ÿ�� ǥ�� ���� �ش� )
							for (auto QuickSlotBarPosition : Player->_QuickSlotManager.FindQuickSlotBar(DefaultAttackSkill->GetSkillInfo()->SkillType))
							{
								// Ŭ�󿡰� ��Ÿ�� ǥ��
								CMessage* ResCoolTimeStartPacket = G_ObjectManager->GameServer->MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
									QuickSlotBarPosition.QuickSlotBarSlotIndex,
									1.0f, DefaultAttackSkill);
								G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCoolTimeStartPacket);
								ResCoolTimeStartPacket->Free();
							}
						}
					}				

					if (Player->_ComboSkill != nullptr)
					{
						st_GameObjectJob* ComboAttackOffJob = G_ObjectManager->GameServer->MakeGameObjectJobComboSkillOff();
						_GameObjectJobQue.Enqueue(ComboAttackOffJob);
					}
				}	
				else
				{
					CRASH("Player Casting Fail");
				}
			}
			break;		
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_COMBO_ATTACK_CREATE:
			{				
				CSkill* ReqMeleeSkill;
				*GameObjectJob->GameObjectJobMessage >> &ReqMeleeSkill;

				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if (Player != nullptr)
				{
					// ���ӱ� ��ų�� ��ųâ���� ã�´�.
					CSkill* FindComboSkill = Player->_SkillBox.FindSkill(ReqMeleeSkill->GetSkillInfo()->SkillCharacteristic, ReqMeleeSkill->GetSkillInfo()->NextComboSkill);
					if (FindComboSkill != nullptr)
					{
						// ���ӱ� ��ų ����
						CSkill* NewComboSkill = G_ObjectManager->SkillCreate();
						// ������ ����
						NewComboSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_COMBO_SKILL, FindComboSkill->GetSkillInfo(), ReqMeleeSkill->GetSkillInfo());
						NewComboSkill->SetOwner(Player);

						// ���ӱ� ��ų ���� �Է�
						Player->_ComboSkill = NewComboSkill;

						NewComboSkill->ComboSkillStart(ReqMeleeSkill->_QuickSlotBarPosition, FindComboSkill->GetSkillInfo()->SkillType);

						CMessage* ResNextComboSkill = G_ObjectManager->GameServer->MakePacketComboSkillOn(ReqMeleeSkill->_QuickSlotBarPosition,
							*FindComboSkill->GetSkillInfo());
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResNextComboSkill);
						ResNextComboSkill->Free();
					}
				}	
				else
				{
					CRASH("Player Casting Fail");
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_COMBO_ATTACK_OFF:
			{
				CPlayer* Player = dynamic_cast<CPlayer*>(this);

				if (Player != nullptr && Player->_ComboSkill != nullptr)
				{
					Player->_ComboSkill->ComboSkillOff();
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SPELL_START:
			{
				int8 SpellCharacteristicType;
				*GameObjectJob->GameObjectJobMessage >> SpellCharacteristicType;

				int16 SpellSkillType;
				*GameObjectJob->GameObjectJobMessage >> SpellSkillType;

				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if (Player != nullptr)
				{
					CSkill* FindSpellSkill = Player->_SkillBox.FindSkill((en_SkillCharacteristic)SpellCharacteristicType, (en_SkillType)SpellSkillType);
					if (FindSpellSkill != nullptr && FindSpellSkill->GetSkillInfo()->CanSkillUse == true)
					{
						vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldOfViewPlayers(this, 1, false);

						if (Player->_ComboSkill != nullptr)
						{
							st_GameObjectJob* ComboAttackOffJob = G_ObjectManager->GameServer->MakeGameObjectJobComboSkillOff();
							_GameObjectJobQue.Enqueue(ComboAttackOffJob);
						}

						if (FindSpellSkill->GetSkillInfo()->SkillType == en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE)
						{
							for (auto DebufSkillIter : _DeBufs)
							{
								// �����̻� ����
								if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE
									|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE
									|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE)
								{
									DebufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
								}
							}

							// ������� ���� ����
							CSkill* NewBufSkill = G_ObjectManager->SkillCreate();
							st_BufSkillInfo* NewShockReleaseSkillInfo = (st_BufSkillInfo*)G_ObjectManager->SkillInfoCreate(FindSpellSkill->GetSkillInfo()->SkillType, FindSpellSkill->GetSkillInfo()->SkillLevel);
							NewBufSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewShockReleaseSkillInfo);
							NewBufSkill->StatusAbnormalDurationTimeStart();

							// ������� ���� ���
							AddBuf(NewBufSkill);

							// Ŭ�󿡰� ���� ��� �˸�
							CMessage* ResBufDebufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_GameObjectInfo.ObjectId, true, NewBufSkill->GetSkillInfo());
							G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDebufSkillPacket);
							ResBufDebufSkillPacket->Free();

							// �����̻� ���� �˸�
							CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
								_GameObjectInfo.ObjectPositionInfo.MoveDir,
								_GameObjectInfo.ObjectType,
								_GameObjectInfo.ObjectPositionInfo.State);
							G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResObjectStateChangePacket);
							ResObjectStateChangePacket->Free();
						}
						else
						{
							if (CheckCantControlStatusAbnormal() > 0)
							{
								CMessage* ResStatusAbnormalSpellCancel = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSOANL_MESSAGE_STATUS_ABNORMAL_SPELL, FindSpellSkill->GetSkillInfo()->SkillName.c_str());
								G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResStatusAbnormalSpellCancel);
								ResStatusAbnormalSpellCancel->Free();
								break;
							}

							switch (FindSpellSkill->GetSkillInfo()->SkillType)
							{
							case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:
							{
								st_BufSkillInfo* ChargePoseSkillInfo = (st_BufSkillInfo*)FindSpellSkill->GetSkillInfo();

								// �����ڼ� ���� ��ų ����
								CSkill* CharPoseBufSkill = G_ObjectManager->SkillCreate();

								// �����ڼ� ���� ��ų ���� ���� �� �ʱ�ȭ
								st_BufSkillInfo* ChargePoseBufSkillInfo = (st_BufSkillInfo*)G_ObjectManager->SkillInfoCreate(FindSpellSkill->GetSkillInfo()->SkillType, FindSpellSkill->GetSkillInfo()->SkillLevel);
								CharPoseBufSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, ChargePoseBufSkillInfo);
								CharPoseBufSkill->StatusAbnormalDurationTimeStart();

								// �����ڼ� ���� ��ų ���
								Player->AddBuf(CharPoseBufSkill);

								// Ŭ�󿡰� ���� ��� �˷���
								CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_GameObjectInfo.ObjectId, true, CharPoseBufSkill->GetSkillInfo());
								G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
								ResBufDeBufSkillPacket->Free();

								// �����ڼ� ����Ʈ ���
								CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(_GameObjectInfo.ObjectId, en_EffectType::EFFECT_CHARGE_POSE, 2.8f);
								G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
								ResEffectPacket->Free();
							}
							break;
							case en_SkillType::SKILL_SPELL_ACTIVE_BUF_TELEPORT:
							{
								// �̵� �Ұ��� �̵� �ӵ� ���� ȿ���� ��� ����
								for (auto DebufSkillIter : _DeBufs)
								{
									if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN
										|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT
										|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT)
									{
										DebufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
									}
								}

								en_MoveDir TelePortDir;

								switch (_GameObjectInfo.ObjectPositionInfo.MoveDir)
								{
								case en_MoveDir::UP:
									TelePortDir = en_MoveDir::DOWN;
									break;
								case en_MoveDir::DOWN:
									TelePortDir = en_MoveDir::UP;
									break;
								case en_MoveDir::LEFT:
									TelePortDir = en_MoveDir::RIGHT;
									break;
								case en_MoveDir::RIGHT:
									TelePortDir = en_MoveDir::LEFT;
									break;
								}

								st_Vector2Int MovePosition;
								MovePosition = GetFrontCellPosition(TelePortDir, 5);

								GetChannel()->GetMap()->ApplyMove(this, MovePosition);

								_GameObjectInfo.ObjectPositionInfo.Position._X = _GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
								_GameObjectInfo.ObjectPositionInfo.Position._Y = _GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;

								// �ð� ��Ʋ�� ��ġ ������
								CMessage* ResSyncPositionPacket = G_ObjectManager->GameServer->MakePacketResSyncPosition(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo);
								G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSyncPositionPacket);
								ResSyncPositionPacket->Free();

								// �ð� ��Ʋ�� ����Ʈ ���
								CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(_GameObjectInfo.ObjectId, en_EffectType::EFFECT_BACK_TELEPORT, 0.5f);
								G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
								ResEffectPacket->Free();
							}
							break;
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_HARPOON:
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:
							case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:
							case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT:
							case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:
							{
								if (_SelectTarget != nullptr)
								{
									// ���� ���� ���� ��� ����
									_SpellSkill = FindSpellSkill;

									// ���� �ð� ����
									_SpellTick = GetTickCount64() + FindSpellSkill->GetSkillInfo()->SkillCastingTime;

									// ���� ���� ���·� ����
									_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPELL;

									// ���� �÷��̾�鿡�� ���� ���� ���� �˷���
									CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
										_GameObjectInfo.ObjectPositionInfo.MoveDir,
										_GameObjectInfo.ObjectType,
										_GameObjectInfo.ObjectPositionInfo.State);
									G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResObjectStateChangePacket);
									ResObjectStateChangePacket->Free();

									// ���� �ð� ���ϱ�
									float SpellCastingTime = _SpellSkill->GetSkillInfo()->SkillCastingTime / 1000.0f;

									// ���� ���� �� ����
									CMessage* ResMagicPacket = G_ObjectManager->GameServer->MakePacketResMagic(_GameObjectInfo.ObjectId,
										true, _SpellSkill->GetSkillInfo()->SkillType, SpellCastingTime);
									G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResMagicPacket);
									ResMagicPacket->Free();
								}
								else
								{
									CMessage* ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT, FindSpellSkill->GetSkillInfo()->SkillName.c_str());
									G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResErrorPacket);
									ResErrorPacket->Free();
								}
							}
							break;
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
							{
								if (_SelectTarget != nullptr)
								{
									// �ñ� �ĵ��� ��� ���ӱ� ��ų�� Ȱ��ȭ �� ��쿡�� ��� �� �� �ֵ��� ���� Ȯ��
									if (Player->_ComboSkill != nullptr && Player->_ComboSkill->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE)
									{
										// �ñ� �ĵ� �����̻� ���� Ȯ��
										bool IsIceWave = _SelectTarget->_StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE;
										if (IsIceWave == false)
										{
											CSkill* NewDebufSkill = G_ObjectManager->SkillCreate();

											st_AttackSkillInfo* NewDebufSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(FindSpellSkill->GetSkillInfo()->SkillType, FindSpellSkill->GetSkillInfo()->SkillLevel);
											NewDebufSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewDebufSkillInfo);
											NewDebufSkill->StatusAbnormalDurationTimeStart();

											_SelectTarget->AddDebuf(NewDebufSkill);
											_SelectTarget->SetStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE);

											CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
												_SelectTarget->_GameObjectInfo.ObjectType,
												_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
												NewDebufSkill->GetSkillInfo()->SkillType,
												true, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE);
											G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
											ResStatusAbnormalPacket->Free();

											CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewDebufSkill->GetSkillInfo());
											G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
											ResBufDeBufSkillPacket->Free();
										}
									}
								}
								else
								{
									CMessage* ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT, FindSpellSkill->GetSkillInfo()->SkillName.c_str());
									G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResErrorPacket);
									ResErrorPacket->Free();
								}
							}
							break;
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:
							{
								if (_SelectTarget != nullptr)
								{
									st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)FindSpellSkill->GetSkillInfo();

									bool IsShmanRoot = _SelectTarget->_StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT;
									if (IsShmanRoot == false)
									{
										CSkill* NewSkill = G_ObjectManager->SkillCreate();

										st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(FindSpellSkill->GetSkillInfo()->SkillType, FindSpellSkill->GetSkillInfo()->SkillLevel);
										NewSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
										NewSkill->StatusAbnormalDurationTimeStart();

										_SelectTarget->AddDebuf(NewSkill);
										_SelectTarget->SetStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT);

										CMessage* SelectTargetMoveStopMessage = G_ObjectManager->GameServer->MakePacketResMoveStop(_SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectPositionInfo);
										G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, SelectTargetMoveStopMessage);
										SelectTargetMoveStopMessage->Free();

										CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
											_SelectTarget->_GameObjectInfo.ObjectType,
											_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
											FindSpellSkill->GetSkillInfo()->SkillType,
											true, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT);
										G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
										ResStatusAbnormalPacket->Free();

										CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
										G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
										ResBufDeBufSkillPacket->Free();

										CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(_SelectTarget->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_DEBUF_ROOT, 0.5f);
										G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
										ResEffectPacket->Free();
									}
								}
								else
								{
									CMessage* ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT, FindSpellSkill->GetSkillInfo()->SkillName.c_str());
									G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResErrorPacket);
									ResErrorPacket->Free();
								}
							}
							break;
							case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:
							{
								if (_SelectTarget != nullptr)
								{
									st_AttackSkillInfo* AttackSkillInfo = (st_AttackSkillInfo*)FindSpellSkill->GetSkillInfo();

									bool IsTaioistRoot = _SelectTarget->_StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT;
									if (IsTaioistRoot == false)
									{
										CSkill* NewSkill = G_ObjectManager->SkillCreate();

										st_AttackSkillInfo* NewAttackSkillInfo = (st_AttackSkillInfo*)G_ObjectManager->SkillInfoCreate(FindSpellSkill->GetSkillInfo()->SkillType, FindSpellSkill->GetSkillInfo()->SkillLevel);
										NewSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
										NewSkill->StatusAbnormalDurationTimeStart();

										CMessage* SelectTargetMoveStopMessage = G_ObjectManager->GameServer->MakePacketResMoveStop(_SelectTarget->_GameObjectInfo.ObjectId, _SelectTarget->_GameObjectInfo.ObjectPositionInfo);
										G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, SelectTargetMoveStopMessage);
										SelectTargetMoveStopMessage->Free();

										_SelectTarget->AddDebuf(NewSkill);
										_SelectTarget->SetStatusAbnormal((int32)en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT);

										CMessage* ResStatusAbnormalPacket = G_ObjectManager->GameServer->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
											_SelectTarget->_GameObjectInfo.ObjectType,
											_SelectTarget->_GameObjectInfo.ObjectPositionInfo.MoveDir,
											FindSpellSkill->GetSkillInfo()->SkillType,
											true, (int32)en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT);
										G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
										ResStatusAbnormalPacket->Free();

										CMessage* ResBufDeBufSkillPacket = G_ObjectManager->GameServer->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
										G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
										ResBufDeBufSkillPacket->Free();

										CMessage* ResEffectPacket = G_ObjectManager->GameServer->MakePacketEffect(_SelectTarget->_GameObjectInfo.ObjectId, en_EffectType::EFFECT_DEBUF_ROOT, 0.5f);
										G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResEffectPacket);
										ResEffectPacket->Free();
									}
								}
								else
								{
									CMessage* ResErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_NON_SELECT_OBJECT, FindSpellSkill->GetSkillInfo()->SkillName.c_str());
									G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResErrorPacket);
									ResErrorPacket->Free();
								}
							}
							break;
							default:
								break;
							}
						}
					}
					else
					{
						CMessage* SkillCoolTimeErrorPacket = G_ObjectManager->GameServer->MakePacketSkillError(en_PersonalMessageType::PERSONAL_MESSAGE_SKILL_COOLTIME, FindSpellSkill->GetSkillInfo()->SkillName.c_str());
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, SkillCoolTimeErrorPacket);
						SkillCoolTimeErrorPacket->Free();
						break;
					}

					// ��� ���� ���� ����� ��� ��Ÿ�� ����
					if (FindSpellSkill->GetSkillInfo()->SkillCastingTime == 0)
					{
						FindSpellSkill->CoolTimeStart();

						// ��Ÿ�� ǥ�� ( ������ �ٿ� ��ϵǾ� �ִ� ���� ������ ��ų�� ��� ��Ÿ�� ǥ�� ���� �ش� )
						for (auto QuickSlotBarPosition : Player->_QuickSlotManager.FindQuickSlotBar(FindSpellSkill->GetSkillInfo()->SkillType))
						{
							// Ŭ�󿡰� ��Ÿ�� ǥ��
							CMessage* ResCoolTimeStartPacket = G_ObjectManager->GameServer->MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
								QuickSlotBarPosition.QuickSlotBarSlotIndex,
								1.0f, FindSpellSkill);
							G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCoolTimeStartPacket);
							ResCoolTimeStartPacket->Free();
						}

						// ��û�� ��ų�� �⺻ ���� ��ų�� �����ϰ� ��ų â���� ������
						vector<CSkill*> GlobalSkills = Player->_SkillBox.GetGlobalSkills(FindSpellSkill->GetSkillInfo()->SkillType, FindSpellSkill->GetSkillKind());

						// ���� ��Ÿ�� ����
						for (CSkill* GlobalSkill : GlobalSkills)
						{
							GlobalSkill->GlobalCoolTimeStart(FindSpellSkill->GetSkillInfo()->SkillMotionTime);

							for (st_Vector2Int QuickSlotPosition : GlobalSkill->_QuickSlotBarPosition)
							{
								CMessage* ResCoolTimeStartPacket = G_ObjectManager->GameServer->MakePacketCoolTime((int8)QuickSlotPosition._Y,
									(int8)QuickSlotPosition._X,
									1.0f, nullptr, FindSpellSkill->GetSkillInfo()->SkillMotionTime);
								G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCoolTimeStartPacket);
								ResCoolTimeStartPacket->Free();
							}
						}
					}
				}
				else
				{
					CRASH("Player Casting Fail");
				}
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

					CCrop* Crop = (CCrop*)GatheringTarget;

					CPlayer* Player = dynamic_cast<CPlayer*>(this);
					if (Player != nullptr)
					{
						if (GatheringTarget != nullptr
							&& Crop->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::READY_DEAD
							&& Crop->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
						{
							st_Vector2 DirNormalVector = (Crop->_GameObjectInfo.ObjectPositionInfo.Position - _GameObjectInfo.ObjectPositionInfo.Position).Normalize();
							en_MoveDir Dir = st_Vector2::GetMoveDir(DirNormalVector);

							if (_GameObjectInfo.ObjectPositionInfo.MoveDir != Dir)
							{
								CMessage* DirErrorPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_DIR_DIFFERENT, Crop->_GameObjectInfo.ObjectName.c_str());
								G_ObjectManager->GameServer->SendPacket(Player->_SessionId, DirErrorPacket);
								DirErrorPacket->Clear();
								break;
							}

							float Distance = st_Vector2::Distance(_GameObjectInfo.ObjectPositionInfo.Position, Crop->_GameObjectInfo.ObjectPositionInfo.Position);
							if (Distance < 1.2f)
							{
								CMessage* ResGatheringPacket = nullptr;

								switch (GatheringTarget->_GameObjectInfo.ObjectType)
								{
								case en_GameObjectType::OBJECT_STONE:
									ResGatheringPacket = G_ObjectManager->GameServer->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"�� ä��");
									break;
								case en_GameObjectType::OBJECT_TREE:
									ResGatheringPacket = G_ObjectManager->GameServer->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"���� ����");
									break;
								case en_GameObjectType::OBJECT_CROP_CORN:
									ResGatheringPacket = G_ObjectManager->GameServer->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"������ ��Ȯ");
									break;
								case en_GameObjectType::OBJECT_CROP_POTATO:
									ResGatheringPacket = G_ObjectManager->GameServer->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"���� ��Ȯ");
									break;
								}

								_GatheringTarget = GatheringTarget;

								_GatheringTick = GetTickCount64() + 1000;

								_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::GATHERING;

								CMessage* ResObjectStateChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
									_GameObjectInfo.ObjectPositionInfo.MoveDir,
									_GameObjectInfo.ObjectType,
									_GameObjectInfo.ObjectPositionInfo.State);
								G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResObjectStateChangePacket);
								ResObjectStateChangePacket->Free();

								G_ObjectManager->GameServer->SendPacketFieldOfView(this, ResGatheringPacket);
								ResGatheringPacket->Free();
							}
							else
							{
								CMessage* DirErrorPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_GATHERING_DISTANCE, Crop->_GameObjectInfo.ObjectName.c_str());
								G_ObjectManager->GameServer->SendPacket(Player->_SessionId, DirErrorPacket);
								DirErrorPacket->Clear();
							}
						}
					}
					else
					{
						CRASH("Player Casting Fail");
					}
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
				int64 AttackerID;
				*GameObjectJob->GameObjectJobMessage >> AttackerID;	

				int16 AttackerType;
				*GameObjectJob->GameObjectJobMessage >> AttackerType;

				bool IsCritical;
				*GameObjectJob->GameObjectJobMessage >> IsCritical;

				int32 Damage;
				*GameObjectJob->GameObjectJobMessage >> Damage;

				int16 SkillType;
				*GameObjectJob->GameObjectJobMessage >> SkillType;

				CGameObject* Attacker = _Channel->FindChannelObject(AttackerID, (en_GameObjectType)AttackerType);
				if (Attacker != nullptr)
				{
					bool IsDead = OnDamaged(Attacker, Damage);

					en_EffectType HitEffectType = en_EffectType::EFFECT_TYPE_NONE;

					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldOfViewPlayers(this, 1, false);

					// ������ �ý��� �޼��� ����	
					CMessage* ResSkillSystemMessagePacket = G_ObjectManager->GameServer->MakePacketResDamageChattingBoxMessage(en_MessageType::MESSAGE_TYPE_DAMAGE_CHATTING,
						Attacker->_GameObjectInfo.ObjectName,
						_GameObjectInfo.ObjectName,
						(en_SkillType)SkillType,
						Damage);
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
					G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, StatChangePacket);
					StatChangePacket->Free();

					if (IsDead == true)
					{
						End();

						if (Attacker->IsPlayer())
						{
							CPlayer* AttackerPlayer = dynamic_cast<CPlayer*>(Attacker);
							if (AttackerPlayer != nullptr)
							{
								if (AttackerPlayer->GetChannel() != nullptr)
								{
									AttackerPlayer->GetChannel()->ExperienceCalculate(AttackerPlayer, _GameObjectInfo.ObjectType, G_Datamanager->FindMonsterExperienceData(_GameObjectInfo.ObjectType));
								}								
								else
								{
									CRASH("Channel nullptr")
								}
							}
							else
							{
								CRASH("Exp Get Not Player")
							}							
						}

						Attacker->_SelectTarget = nullptr;

						((CPlayer*)Attacker)->_OnPlayerDefaultAttack = false;
					}					
				}			
			}
			break;	
		case en_GameObjectJobType::GAMEOJBECT_JOB_TYPE_SKILL_HP_HEAL:
			{
				int64 HealerID;
				*GameObjectJob->GameObjectJobMessage >> HealerID;

				int32 HealPoint;
				*GameObjectJob->GameObjectJobMessage >> HealPoint;

				int16 HealSkillType;
				*GameObjectJob->GameObjectJobMessage >> HealSkillType;

				vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldOfViewPlayers(this, 1, false);

				CGameObject* Healer = _Channel->FindChannelObject(HealerID, en_GameObjectType::OBJECT_PLAYER);				
				if (Healer != nullptr)
				{
					// �ý��� �޼��� ����	
					CMessage* ResSkillSystemMessagePacket = G_ObjectManager->GameServer->MakePacketResDamageChattingBoxMessage(en_MessageType::MESSAGE_TYPE_DAMAGE_CHATTING,
						Healer->_GameObjectInfo.ObjectName,
						_GameObjectInfo.ObjectName,
						(en_SkillType)HealSkillType,
						HealPoint);
					G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSkillSystemMessagePacket);
					ResSkillSystemMessagePacket->Free();

					CMessage* ResDamagePacket = G_ObjectManager->GameServer->MakePacketResDamage(Healer->_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectId,
						(en_SkillType)HealSkillType,
						HealPoint,
						false);
					G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResDamagePacket);
					ResDamagePacket->Free();

					CMessage* StatChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectStatInfo);
					G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, StatChangePacket);
					StatChangePacket->Free();
				}				
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_ON_EQUIPMENT:
			{
				// ������ ��� 
				CItem* EquipmentItem;
				*GameObjectJob->GameObjectJobMessage >> &EquipmentItem;				
				
				// ��� ����
				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if (Player != nullptr)
				{
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
				else
				{
					CRASH("Player Casting Fail");
				}
			}
			break;	
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_OFF_EQUIPMENT:
			{
				int8 EquipmentParts;
				*GameObjectJob->GameObjectJobMessage >> EquipmentParts;

				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if (Player != nullptr)
				{
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
				else
				{
					CRASH("Player Casting Fail");
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_FULL_RECOVERY:
			{
				for (auto DebufSkillIter : _DeBufs)
				{
					if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_FIGHT_ACTIVE_ATTACK_CHOHONE
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT
						|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT)
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

				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if (Player != nullptr)
				{
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
				else
				{
					CRASH("Player Casting Fail");
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_ITEM_INVENTORY_SAVE:
			{
				CGameObject* Item;
				*GameObjectJob->GameObjectJobMessage >> &Item;
								
				CItem* InsertItem = (CItem*)Item;

				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if (Player != nullptr)
				{
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
				else
				{
					CRASH("Player Casting Fail");
				}
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
				
				CCraftingTable* CraftingTable = dynamic_cast<CCraftingTable*>(this);

				CPlayer* CraftingTableItemAddPlayer = dynamic_cast<CPlayer*>(CraftingTableItemAddPlayerGO);

				if (CraftingTable != nullptr && CraftingTableItemAddPlayer != nullptr)
				{
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
				
				CCraftingTable* CraftingTable = dynamic_cast<CCraftingTable*>(this);

				CPlayer* CraftingTableItemSubtractPlayer = dynamic_cast<CPlayer*>(CraftingTableMaterialItemSubtractPlayerGO);

				if (CraftingTable != nullptr && CraftingTableItemSubtractPlayer != nullptr)
				{
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

				CCraftingTable* CraftingTable = dynamic_cast<CCraftingTable*>(this);

				CPlayer* CraftingTableItemSubtractPlayer = dynamic_cast<CPlayer*>(CraftingTableCompleteItemSubtractPlayerGO);

				if (CraftingTable != nullptr && CraftingTableItemSubtractPlayer != nullptr)
				{
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
							CCraftingTable* CraftingTable = dynamic_cast<CCraftingTable*>(this);
							if (CraftingTable != nullptr)
							{
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

				CCraftingTable* CraftingTable = dynamic_cast<CCraftingTable*>(this);
				if (CraftingTable != nullptr)
				{
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
				else
				{
					CRASH("CraftingTable Casting Fail");
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

	PushedOutStatusAbnormalCheck();
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

void CGameObject::SetStatusAbnormal(int32 StatusAbnormalValue)
{
	_StatusAbnormal |= StatusAbnormalValue;
}

void CGameObject::ReleaseStatusAbnormal(int32 StatusAbnormalValue)
{
	_StatusAbnormal &= StatusAbnormalValue;
}

int32 CGameObject::CheckCantControlStatusAbnormal()
{
	int32 StatusAbnormalCount = 0;

	if (_StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_CHOHONE)
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_SHIELD_SMASH)
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE)
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE)
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_ASSASSINATION_BACK_STEP)
	{
		StatusAbnormalCount++;
	}

	return StatusAbnormalCount;
}

int32 CGameObject::CheckCanControlStatusAbnormal()
{
	int32 StatusAbnormalCount = 0;

	if (_StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_SHAEHONE)
	{
		StatusAbnormalCount++;
	}

	if ((_StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT)
		|| (_StatusAbnormal & (int32)en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT))
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

bool CGameObject::IsPlayer()
{
	if (_GameObjectInfo.ObjectType == en_GameObjectType::OBJECT_WARRIOR_PLAYER
		|| _GameObjectInfo.ObjectType == en_GameObjectType::OBJECT_SHAMAN_PLAYER
		|| _GameObjectInfo.ObjectType == en_GameObjectType::OBJECT_TAIOIST_PLAYER
		|| _GameObjectInfo.ObjectType == en_GameObjectType::OBJECT_THIEF_PLAYER
		|| _GameObjectInfo.ObjectType == en_GameObjectType::OBJECT_ARCHER_PLAYER)
	{
		return true;
	}

	return false;
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

	CheckBufDeBufSkill();
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

void CGameObject::CheckBufDeBufSkill()
{
	if (_Bufs.size() > 0)
	{
		// ��ȭȿ�� ��ų ����Ʈ ��ȸ
		for (auto BufSkillIterator : _Bufs)
		{
			// ���ӽð� ���� ��ȭȿ�� ����
			bool DeleteBufSkill = BufSkillIterator.second->Update();
			if (DeleteBufSkill)
			{
				DeleteBuf(BufSkillIterator.first);
				// ��ȭȿ�� ��ų ���� �޸� �ݳ�
				G_ObjectManager->SkillInfoReturn(BufSkillIterator.second->GetSkillInfo()->SkillType,
					BufSkillIterator.second->GetSkillInfo());
				// ��ȭȿ�� ��ų �޸� �ݳ�
				G_ObjectManager->SkillReturn(BufSkillIterator.second);
			}
		}
	}	

	if (_DeBufs.size() > 0)
	{
		// ��ȭȿ�� ��ų ����Ʈ ��ȸ
		for (auto DebufSkillIterator : _DeBufs)
		{
			// ���ӽð� ���� ��ȭȿ�� ����
			bool DeleteDebufSkill = DebufSkillIterator.second->Update();
			if (DeleteDebufSkill)
			{
				DeleteDebuf(DebufSkillIterator.first);
				// ��ȭȿ�� ��ų ���� �޸� �ݳ�
				G_ObjectManager->SkillInfoReturn(DebufSkillIterator.second->GetSkillInfo()->SkillType,
					DebufSkillIterator.second->GetSkillInfo());
				// ��ȭȿ�� ��ų �޸� �ݳ�
				G_ObjectManager->SkillReturn(DebufSkillIterator.second);
			}
		}
	}	
}