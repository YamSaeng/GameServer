#include "pch.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "NetworkManager.h"
#include "DataManager.h"
#include "Skill.h"
#include "Furnace.h"
#include "Crop.h"
#include "EquipmentBox.h"
#include "RectCollision.h"

CGameObject::CGameObject()
{
	_StatusAbnormal = 0;

	_ObjectManagerArrayIndex = -1;
	_ChannelArrayIndex = -1;
	_NetworkState = en_ObjectNetworkState::OBJECT_NETWORK_STATE_READY;
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
	_NetworkState = en_ObjectNetworkState::OBJECT_NETWORK_STATE_READY;
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
	bool IsShamanIceWave = _StatusAbnormal & (int64)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE;
	if (IsShamanIceWave == true)
	{		
		
	}
}

void CGameObject::Update()
{
	if (_NetworkState == en_ObjectNetworkState::OBJECT_NETWORK_STATE_LEAVE)
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_MOVE:
			{
				float DirectionX;
				*GameObjectJob->GameObjectJobMessage >> DirectionX;
				float DirectionY;
				*GameObjectJob->GameObjectJobMessage >> DirectionY;
				float PositionX;
				*GameObjectJob->GameObjectJobMessage >> PositionX;
				float PositionY;
				*GameObjectJob->GameObjectJobMessage >> PositionY;
				int8 GameObjectState;
				*GameObjectJob->GameObjectJobMessage >> GameObjectState;

				// 서버와 클라 위치 차이 구함
				float CheckPositionX = abs(_GameObjectInfo.ObjectPositionInfo.Position.X - PositionX);
				float CheckPositionY = abs(_GameObjectInfo.ObjectPositionInfo.Position.Y - PositionY);
				
				vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

				if (CheckPositionX < 1.0f && CheckPositionY < 1.0f)
				{					
					_GameObjectInfo.ObjectPositionInfo.MoveDirection.X = DirectionX;
					_GameObjectInfo.ObjectPositionInfo.MoveDirection.Y = DirectionY;

					_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::MOVING;					

					CMessage* ResMovePacket = G_NetworkManager->GetGameServer()->MakePacketResMove(_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectPositionInfo.LookAtDireciton,
						_GameObjectInfo.ObjectPositionInfo.MoveDirection,
						_GameObjectInfo.ObjectPositionInfo.Position);
					G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIds, ResMovePacket);
					ResMovePacket->Free();
				}	
				else if (CheckPositionX >= 1.0f || CheckPositionY >= 1.0f)
				{
					CMessage* ResMoveStopPacket = G_NetworkManager->GetGameServer()->MakePacketResMoveStop(_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectPositionInfo.Position.X,
						_GameObjectInfo.ObjectPositionInfo.Position.Y);
					G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIds, ResMoveStopPacket);
					ResMoveStopPacket->Free();
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_MOVE_STOP:
			{
				float PositionX;
				*GameObjectJob->GameObjectJobMessage >> PositionX;
				float PositionY;
				*GameObjectJob->GameObjectJobMessage >> PositionY;
				int8 GameObjectState;
				*GameObjectJob->GameObjectJobMessage >> GameObjectState;
			
				_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
				_GameObjectInfo.ObjectPositionInfo.MoveDirection = Vector2::Zero;

				float CheckPositionX = abs(_GameObjectInfo.ObjectPositionInfo.Position.X - PositionX);
				float CheckPositionY = abs(_GameObjectInfo.ObjectPositionInfo.Position.Y - PositionY);

				vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIds = _Channel->GetMap()->GetFieldAroundPlayers(this);

				CMessage* ResMoveStopPacket = G_NetworkManager->GetGameServer()->MakePacketResMoveStop(_GameObjectInfo.ObjectId,
					_GameObjectInfo.ObjectPositionInfo.Position.X,
					_GameObjectInfo.ObjectPositionInfo.Position.Y);
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIds, ResMoveStopPacket);
				ResMoveStopPacket->Free();				
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SELECT_SKILL_CHARACTERISTIC:
			{
				int8 SkillCharacteristicType;
				*GameObjectJob->GameObjectJobMessage >> SkillCharacteristicType;

				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if (Player != nullptr)
				{
					if (Player->_SkillBox.CheckCharacteristic((en_SkillCharacteristic)SkillCharacteristicType) == true)
					{
						CMessage* ResReqCancel = G_NetworkManager->GetGameServer()->MakePacketReqCancel(en_GAME_SERVER_PACKET_TYPE::en_PACKET_S2C_SELECT_SKILL_CHARACTERISTIC);
						G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResReqCancel);
						ResReqCancel->Free();
					}
					else
					{
						Player->_SkillBox.CreateChracteristic(SkillCharacteristicType);

						CSkillCharacteristic* SkillCharacteristic = Player->_SkillBox.FindCharacteristic(SkillCharacteristicType);

						CMessage* ResSkillCharacteristicPacket = G_NetworkManager->GetGameServer()->MakePacketResSelectSkillCharacteristic(true,
							SkillCharacteristicType,
							SkillCharacteristic->GetPassiveSkill(), SkillCharacteristic->GetActiveSkill());
						G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResSkillCharacteristicPacket);
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
						vector<en_SkillType> LearnSkillTypes;

						if (IsSkillLearn == true)
						{
							if (Skill->GetSkillInfo()->IsSkillLearn == false)
							{
								if (Player->_GameObjectInfo.ObjectSkillPoint > 0)
								{
									Player->_SkillBox.SkillLearn(IsSkillLearn, (en_SkillType)LearnSkillType);
								
									LearnSkillTypes.push_back((en_SkillType)LearnSkillType);

									if (Skill->GetSkillInfo()->NextComboSkill != en_SkillType::SKILL_TYPE_NONE)
									{
										Player->_SkillBox.SkillLearn(IsSkillLearn, Skill->GetSkillInfo()->NextComboSkill);
										LearnSkillTypes.push_back(Skill->GetSkillInfo()->NextComboSkill);
									}

									Player->_GameObjectInfo.ObjectSkillPoint--;
								}
								else
								{
									IsSkillLearn = false;
								}						
							}

							CMessage* ResSkillLearnPacket = G_NetworkManager->GetGameServer()->MakePacketResSkillLearn(IsSkillLearn, LearnSkillTypes, Player->_GameObjectInfo.ObjectSkillMaxPoint, Player->_GameObjectInfo.ObjectSkillPoint);
							G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResSkillLearnPacket);
							ResSkillLearnPacket->Free();
						}
						else
						{
							if (Skill->GetSkillInfo()->CanSkillUse == true)
							{
								if (Skill->GetSkillInfo()->IsSkillLearn == true)
								{
									LearnSkillTypes.push_back(Skill->GetSkillInfo()->SkillType);

									if (Player->_GameObjectInfo.ObjectSkillPoint < Player->_GameObjectInfo.ObjectSkillMaxPoint)
									{
										// 퀵슬롯에서 취소하고자 하는스킬을 찾고 퀵슬롯 등록 제거
										vector<st_QuickSlotBarSlotInfo*> QuickSlotBars = Player->_QuickSlotManager.FindQuickSlotBarInfo((en_SkillType)LearnSkillType);
										for (auto QuickSlotBar : QuickSlotBars)
										{
											for (auto QuickSlotPositionIter = Skill->_QuickSlotBarPosition.begin();
												QuickSlotPositionIter != Skill->_QuickSlotBarPosition.end();
												++QuickSlotPositionIter)
											{
												Vector2Int QuickSlotPosition = *QuickSlotPositionIter;

												if (QuickSlotPosition.Y == QuickSlotBar->QuickSlotBarIndex && QuickSlotPosition.X == QuickSlotBar->QuickSlotBarSlotIndex)
												{
													QuickSlotBar->QuickSlotBarType = en_QuickSlotBarType::QUICK_SLOT_BAR_TYPE_NONE;
													QuickSlotBar->QuickBarSkill = nullptr;
													QuickSlotBar->QuickBarItem = nullptr;

													Skill->_QuickSlotBarPosition.erase(QuickSlotPositionIter);

													CMessage* ResQuickSlotInitMessage = G_NetworkManager->GetGameServer()->MakePacketResQuickSlotInit(QuickSlotBar->QuickSlotBarIndex,
														QuickSlotBar->QuickSlotBarSlotIndex);
													G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResQuickSlotInitMessage);
													ResQuickSlotInitMessage->Free();

													break;
												}
											}
										}

										Player->_SkillBox.SkillLearn(IsSkillLearn, (en_SkillType)LearnSkillType);

										if (Skill->GetSkillInfo()->NextComboSkill != en_SkillType::SKILL_TYPE_NONE)
										{
											Player->_SkillBox.SkillLearn(IsSkillLearn, Skill->GetSkillInfo()->NextComboSkill);
											LearnSkillTypes.push_back(Skill->GetSkillInfo()->NextComboSkill);
										}

										Player->_GameObjectInfo.ObjectSkillPoint++;
									}
									else
									{
										IsSkillLearn = false;
									}
								}

								CMessage* ResSkillLearnPacket = G_NetworkManager->GetGameServer()->MakePacketResSkillLearn(IsSkillLearn, LearnSkillTypes, Player->_GameObjectInfo.ObjectSkillMaxPoint, Player->_GameObjectInfo.ObjectSkillPoint);
								G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResSkillLearnPacket);
								ResSkillLearnPacket->Free();
							}
							else
							{
								CMessage* ResSkillCancelFailtPacket = G_NetworkManager->GetGameServer()->MakePacketCommonError(en_GlobalMessageType::GLOBAL_MESSAGE_SKILL_CANCEL_FAIL_COOLTIME, Skill->GetSkillInfo()->SkillName.c_str());
								G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResSkillCancelFailtPacket);
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_SKILL_MELEE_ATTACK:
			{
				int8 MeleeChracteristicType;
				*GameObjectJob->GameObjectJobMessage >> MeleeChracteristicType;

				int16 MeleeSkillType;
				*GameObjectJob->GameObjectJobMessage >> MeleeSkillType;

				float WeaponPositionX;
				*GameObjectJob->GameObjectJobMessage >> WeaponPositionX;

				float WeaponPositionY;
				*GameObjectJob->GameObjectJobMessage >> WeaponPositionY;

				float MeleeAttackDirectionX;
				*GameObjectJob->GameObjectJobMessage >> MeleeAttackDirectionX;

				float MeleeAttackDirectionY;
				*GameObjectJob->GameObjectJobMessage >> MeleeAttackDirectionY;

				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if(Player != nullptr)
				{
					Player->_SkillBox.SkillProcess(Player, nullptr, (en_SkillCharacteristic)MeleeChracteristicType, (en_SkillType)MeleeSkillType, WeaponPositionX, WeaponPositionY, MeleeAttackDirectionX, MeleeAttackDirectionY);
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
					// 연속기 스킬이 스킬창에서 찾는다.
					CSkill* FindComboSkill = Player->_SkillBox.FindSkill(ReqMeleeSkill->GetSkillInfo()->SkillCharacteristic, ReqMeleeSkill->GetSkillInfo()->NextComboSkill);
					if (FindComboSkill != nullptr)
					{
						// 연속기 스킬 생성
						CSkill* NewComboSkill = G_ObjectManager->SkillCreate();
						// 정보를 저장
						NewComboSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_COMBO_SKILL, FindComboSkill->GetSkillInfo(), ReqMeleeSkill->GetSkillInfo());
						NewComboSkill->SetOwner(Player);

						// 연속기 스킬 정보 입력
						Player->_ComboSkill = NewComboSkill;

						NewComboSkill->ComboSkillStart(ReqMeleeSkill->_QuickSlotBarPosition, FindComboSkill->GetSkillInfo()->SkillType);

						CMessage* ResNextComboSkill = G_NetworkManager->GetGameServer()->MakePacketComboSkillOn(ReqMeleeSkill->_QuickSlotBarPosition,
							*FindComboSkill->GetSkillInfo());
						G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResNextComboSkill);
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
				CCreature* Creature = dynamic_cast<CCreature*>(this);

				if (Creature != nullptr && Creature->_ComboSkill != nullptr)
				{
					Creature->_ComboSkill->ComboSkillOff();
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
						vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

						if (Player->_ComboSkill != nullptr)
						{
							st_GameObjectJob* ComboAttackOffJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobComboSkillOff();
							_GameObjectJobQue.Enqueue(ComboAttackOffJob);
						}

						if (FindSpellSkill->GetSkillInfo()->SkillType == en_SkillType::SKILL_PUBLIC_ACTIVE_BUF_SHOCK_RELEASE)
						{
							for (auto DebufSkillIter : _DeBufs)
							{
								// 상태이상 해제
								if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE
									|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE
									|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE)
								{
									DebufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
								}
							}

							// 충격해제 버프 생성
							CSkill* NewBufSkill = G_ObjectManager->SkillCreate();
							st_SkillInfo* NewShockReleaseSkillInfo = G_ObjectManager->SkillInfoCreate(FindSpellSkill->GetSkillInfo()->SkillType, FindSpellSkill->GetSkillInfo()->SkillLevel);
							NewBufSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewShockReleaseSkillInfo);
							NewBufSkill->StatusAbnormalDurationTimeStart();

							// 충격해제 버프 등록
							AddBuf(NewBufSkill);

							// 클라에게 버프 등록 알림
							CMessage* ResBufDebufSkillPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBuf(_GameObjectInfo.ObjectId, true, NewBufSkill->GetSkillInfo());
							G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDebufSkillPacket);
							ResBufDebufSkillPacket->Free();

							// 상태이상 해제 알림
							CMessage* ResObjectStateChangePacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,																
								_GameObjectInfo.ObjectPositionInfo.State);
							G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResObjectStateChangePacket);
							ResObjectStateChangePacket->Free();
						}
						else
						{
							if (CheckCantControlStatusAbnormal() > 0)
							{
								CMessage* ResStatusAbnormalSpellCancel = G_NetworkManager->GetGameServer()->MakePacketCommonError(en_GlobalMessageType::GLOBAL_MESSAGE_STATUS_ABNORMAL, FindSpellSkill->GetSkillInfo()->SkillName.c_str());
								G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResStatusAbnormalSpellCancel);
								ResStatusAbnormalSpellCancel->Free();
								break;
							}

							switch (FindSpellSkill->GetSkillInfo()->SkillType)
							{
							case en_SkillType::SKILL_FIGHT_ACTIVE_BUF_CHARGE_POSE:
								{
									st_SkillInfo* ChargePoseSkillInfo = FindSpellSkill->GetSkillInfo();

									// 돌격자세 버프 스킬 생성
									CSkill* CharPoseBufSkill = G_ObjectManager->SkillCreate();

									// 돌격자세 버프 스킬 정보 생성 및 초기화
									st_SkillInfo* ChargePoseBufSkillInfo = G_ObjectManager->SkillInfoCreate(FindSpellSkill->GetSkillInfo()->SkillType, FindSpellSkill->GetSkillInfo()->SkillLevel);
									CharPoseBufSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, ChargePoseBufSkillInfo);
									CharPoseBufSkill->StatusAbnormalDurationTimeStart();

									// 돌격자세 버프 스킬 등록
									Player->AddBuf(CharPoseBufSkill);

									// 클라에게 버프 등록 알려줌
									CMessage* ResBufDeBufSkillPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBuf(_GameObjectInfo.ObjectId, true, CharPoseBufSkill->GetSkillInfo());
									G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
									ResBufDeBufSkillPacket->Free();
								}
								break;
							case en_SkillType::SKILL_SPELL_ACTIVE_BUF_BACK_TELEPORT:
								{
									// 이동 불가와 이동 속도 감소 효과를 모두 삭제
									for (auto DebufSkillIter : _DeBufs)
									{
										if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN
											|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT
											|| DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT)
										{
											DebufSkillIter.second->GetSkillInfo()->SkillRemainTime = 0;
										}
									}								

									Vector2Int MovePosition;								
									// 다섯칸 바라보고 있는 방향 반대 위치값 구해야함

									GetChannel()->GetMap()->ApplyMove(this, MovePosition);

									_GameObjectInfo.ObjectPositionInfo.Position.X = _GameObjectInfo.ObjectPositionInfo.CollisionPosition.X + 0.5f;
									_GameObjectInfo.ObjectPositionInfo.Position.Y = _GameObjectInfo.ObjectPositionInfo.CollisionPosition.Y + 0.5f;

									// 시공 뒤틀림 위치 재조정
									CMessage* ResSyncPositionPacket = G_NetworkManager->GetGameServer()->MakePacketResSyncPosition(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectPositionInfo);
									G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSyncPositionPacket);
									ResSyncPositionPacket->Free();
								}
								break;
							case en_SkillType::SKILL_SPELL_ACTIVE_BUF_ILLUSION:
								{

								}
								break;
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_BOLT:
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_FLAME_BLAZE:
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_CHAIN:														
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_LIGHTNING_STRIKE:
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_HEL_FIRE:							
							case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_DIVINE_STRIKE:
							case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_LIGHT:
							case en_SkillType::SKILL_DISCIPLINE_ACTIVE_HEAL_HEALING_WIND:
								{
									if (_SelectTarget != nullptr)
									{
										// 시전 중인 마법 기술 저장
										_SpellSkill = FindSpellSkill;

										// 시전 시간 설정
										_SpellTick = GetTickCount64() + FindSpellSkill->GetSkillInfo()->SkillCastingTime;

										// 마법 시전 상태로 변경
										_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPELL;

										// 주위 플레이어들에게 마법 시전 상태 알려줌
										CMessage* ResObjectStateChangePacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
											_GameObjectInfo.ObjectPositionInfo.State);
										G_NetworkManager->GetGameServer()->SendPacketFieldOfView(this, ResObjectStateChangePacket);
										ResObjectStateChangePacket->Free();

										// 시전 시간 구하기
										float SpellCastingTime = _SpellSkill->GetSkillInfo()->SkillCastingTime / 1000.0f;

										// 마법 시전 바 시작
										CMessage* ResMagicPacket = G_NetworkManager->GetGameServer()->MakePacketResMagic(_GameObjectInfo.ObjectId,
											true, _SpellSkill->GetSkillInfo()->SkillType, SpellCastingTime);
										G_NetworkManager->GetGameServer()->SendPacketFieldOfView(this, ResMagicPacket);
										ResMagicPacket->Free();
									}
									else
									{
										CMessage* ResErrorPacket = G_NetworkManager->GetGameServer()->MakePacketSkillError(en_GlobalMessageType::GLOBAL_MESSAGE_NON_SELECT_OBJECT, FindSpellSkill->GetSkillInfo()->SkillName.c_str());
										G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResErrorPacket);
										ResErrorPacket->Free();
									}
								}
								break;
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_SLEEP:
								{

								}
								break;
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_WINTER_BINDING:
								{
	
								}
								break;
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE:
								{
									if (_SelectTarget != nullptr)
									{
										// 냉기 파동의 경우 연속기 스킬로 활성화 된 경우에만 사용 할 수 있도록 조건 확인
										if (Player->_ComboSkill != nullptr && Player->_ComboSkill->GetSkillInfo()->SkillType == en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ICE_WAVE)
										{
											// 냉기 파동 상태이상 인지 확인
											bool IsIceWave = _SelectTarget->_StatusAbnormal & (int64)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE;
											if (IsIceWave == false)
											{
												CSkill* NewDebufSkill = G_ObjectManager->SkillCreate();

												st_SkillInfo* NewDebufSkillInfo = G_ObjectManager->SkillInfoCreate(FindSpellSkill->GetSkillInfo()->SkillType, FindSpellSkill->GetSkillInfo()->SkillLevel);
												NewDebufSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewDebufSkillInfo);
												NewDebufSkill->StatusAbnormalDurationTimeStart();

												_SelectTarget->AddDebuf(NewDebufSkill);
												_SelectTarget->SetStatusAbnormal((int64)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE);

												/*CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(Target->_GameObjectInfo.ObjectId,
													Target->_GameObjectInfo.ObjectPositionInfo.Position.X,
													Target->_GameObjectInfo.ObjectPositionInfo.Position.Y,
													NewSkillInfo,
													true, (int64)StatusType);*/

												CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
													_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.X,
													_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.Y,																				
													NewDebufSkill->GetSkillInfo(),
													true, (int64)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE);
												G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
												ResStatusAbnormalPacket->Free();

												CMessage* ResBufDeBufSkillPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewDebufSkill->GetSkillInfo());
												G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
												ResBufDeBufSkillPacket->Free();
											}
										}
									}
									else
									{
										CMessage* ResErrorPacket = G_NetworkManager->GetGameServer()->MakePacketSkillError(en_GlobalMessageType::GLOBAL_MESSAGE_NON_SELECT_OBJECT, FindSpellSkill->GetSkillInfo()->SkillName.c_str());
										G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResErrorPacket);
										ResErrorPacket->Free();
									}
								}
								break;
							case en_SkillType::SKILL_SPELL_ACTIVE_ATTACK_ROOT:
								{
									if (_SelectTarget != nullptr)
									{
										st_SkillInfo* AttackSkillInfo = FindSpellSkill->GetSkillInfo();

										bool IsShmanRoot = _SelectTarget->_StatusAbnormal & (int64)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT;
										if (IsShmanRoot == false)
										{
											CSkill* NewSkill = G_ObjectManager->SkillCreate();

											st_SkillInfo* NewAttackSkillInfo = G_ObjectManager->SkillInfoCreate(FindSpellSkill->GetSkillInfo()->SkillType, FindSpellSkill->GetSkillInfo()->SkillLevel);
											NewSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
											NewSkill->StatusAbnormalDurationTimeStart();

											_SelectTarget->AddDebuf(NewSkill);
											_SelectTarget->SetStatusAbnormal((int64)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT);

											CMessage* SelectTargetMoveStopMessage = G_NetworkManager->GetGameServer()->MakePacketResMoveStop(_SelectTarget->_GameObjectInfo.ObjectId,
												_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.X,
												_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.Y);
											G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, SelectTargetMoveStopMessage);
											SelectTargetMoveStopMessage->Free();

											CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
												_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.X,
												_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.Y,
												FindSpellSkill->GetSkillInfo(),
												true, (int64)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT);											
											G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
											ResStatusAbnormalPacket->Free();

											CMessage* ResBufDeBufSkillPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
											G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
											ResBufDeBufSkillPacket->Free();									
										}
									}
									else
									{
										CMessage* ResErrorPacket = G_NetworkManager->GetGameServer()->MakePacketSkillError(en_GlobalMessageType::GLOBAL_MESSAGE_NON_SELECT_OBJECT, FindSpellSkill->GetSkillInfo()->SkillName.c_str());
										G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResErrorPacket);
										ResErrorPacket->Free();
									}
								}
								break;
							case en_SkillType::SKILL_DISCIPLINE_ACTIVE_ATTACK_ROOT:
								{
									if (_SelectTarget != nullptr)
									{
										st_SkillInfo* AttackSkillInfo = FindSpellSkill->GetSkillInfo();

										bool IsTaioistRoot = _SelectTarget->_StatusAbnormal & (int64)en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT;
										if (IsTaioistRoot == false)
										{
											CSkill* NewSkill = G_ObjectManager->SkillCreate();

											st_SkillInfo* NewAttackSkillInfo = G_ObjectManager->SkillInfoCreate(FindSpellSkill->GetSkillInfo()->SkillType, FindSpellSkill->GetSkillInfo()->SkillLevel);
											NewSkill->SetSkillInfo(en_SkillCategory::SKILL_CATEGORY_STATUS_ABNORMAL_SKILL, NewAttackSkillInfo);
											NewSkill->StatusAbnormalDurationTimeStart();

											CMessage* SelectTargetMoveStopMessage = G_NetworkManager->GetGameServer()->MakePacketResMoveStop(_SelectTarget->_GameObjectInfo.ObjectId,
												_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.X,
												_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.Y);
											G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, SelectTargetMoveStopMessage);
											SelectTargetMoveStopMessage->Free();

											_SelectTarget->AddDebuf(NewSkill);
											_SelectTarget->SetStatusAbnormal((int64)en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT);

											CMessage* ResStatusAbnormalPacket = G_NetworkManager->GetGameServer()->MakePacketStatusAbnormal(_SelectTarget->_GameObjectInfo.ObjectId,
												_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.X,
												_SelectTarget->_GameObjectInfo.ObjectPositionInfo.Position.Y,
												FindSpellSkill->GetSkillInfo(),
												true, (int64)en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT);											
											G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResStatusAbnormalPacket);
											ResStatusAbnormalPacket->Free();

											CMessage* ResBufDeBufSkillPacket = G_NetworkManager->GetGameServer()->MakePacketBufDeBuf(_SelectTarget->_GameObjectInfo.ObjectId, false, NewSkill->GetSkillInfo());
											G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResBufDeBufSkillPacket);
											ResBufDeBufSkillPacket->Free();
										}
									}
									else
									{
										CMessage* ResErrorPacket = G_NetworkManager->GetGameServer()->MakePacketSkillError(en_GlobalMessageType::GLOBAL_MESSAGE_NON_SELECT_OBJECT, FindSpellSkill->GetSkillInfo()->SkillName.c_str());
										G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResErrorPacket);
										ResErrorPacket->Free();
									}
								}
								break;							
							}
						}
					}
					else
					{
						CMessage* SkillCoolTimeErrorPacket = G_NetworkManager->GetGameServer()->MakePacketSkillError(en_GlobalMessageType::GLOBAL_MESSAGE_SKILL_COOLTIME, FindSpellSkill->GetSkillInfo()->SkillName.c_str());
						G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, SkillCoolTimeErrorPacket);
						SkillCoolTimeErrorPacket->Free();
						break;
					}

					// 즉시 시전 마법 기술의 경우 쿨타임 시작
					if (FindSpellSkill->GetSkillInfo()->SkillCastingTime == 0)
					{
						FindSpellSkill->CoolTimeStart();

						// 쿨타임 표시 ( 퀵술롯 바에 등록되어 있는 같은 종류의 스킬을 모두 쿨타임 표시 시켜 준다 )
						for (auto QuickSlotBarPosition : Player->_QuickSlotManager.FindQuickSlotBar(FindSpellSkill->GetSkillInfo()->SkillType))
						{
							// 클라에게 쿨타임 표시
							CMessage* ResCoolTimeStartPacket = G_NetworkManager->GetGameServer()->MakePacketCoolTime(QuickSlotBarPosition.QuickSlotBarIndex,
								QuickSlotBarPosition.QuickSlotBarSlotIndex,
								1.0f, FindSpellSkill);
							G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResCoolTimeStartPacket);
							ResCoolTimeStartPacket->Free();
						}

						// 요청한 스킬과 기본 공격 스킬을 제외하고 스킬 창에서 가져옴
						vector<CSkill*> GlobalSkills = Player->_SkillBox.GetGlobalSkills(FindSpellSkill->GetSkillInfo()->SkillType, FindSpellSkill->GetSkillInfo()->SkillKind);

						// 전역 쿨타임 적용
						for (CSkill* GlobalSkill : GlobalSkills)
						{
							GlobalSkill->GlobalCoolTimeStart(FindSpellSkill->GetSkillInfo()->SkillMotionTime);

							for (Vector2Int QuickSlotPosition : GlobalSkill->_QuickSlotBarPosition)
							{
								CMessage* ResCoolTimeStartPacket = G_NetworkManager->GetGameServer()->MakePacketCoolTime((int8)QuickSlotPosition.Y,
									(int8)QuickSlotPosition.X,
									1.0f, nullptr, FindSpellSkill->GetSkillInfo()->SkillMotionTime);
								G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResCoolTimeStartPacket);
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
				CMessage* ResMagicCancelPacket = G_NetworkManager->GetGameServer()->MakePacketMagicCancel(_GameObjectInfo.ObjectId);
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(this, ResMagicCancelPacket);
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
							&& Crop->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
						{
							Vector2 DirNormalVector = (Crop->_GameObjectInfo.ObjectPositionInfo.Position - _GameObjectInfo.ObjectPositionInfo.Position).Normalize();

							// 작물 채집할 때 같은 방향을 바라보고 있지 않으면 에러 메세지 출력
							/*if (_GameObjectInfo.ObjectPositionInfo.MoveDir != Dir)
							{
								CMessage* DirErrorPacket = G_NetworkManager->GetGameServer()->MakePacketCommonError(en_GlobalMessageType::GLOBAL_MESSAGE_DIR_DIFFERENT, Crop->_GameObjectInfo.ObjectName.c_str());
								G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, DirErrorPacket);
								DirErrorPacket->Clear();
								break;
							}*/

							float Distance = Vector2::Distance(_GameObjectInfo.ObjectPositionInfo.Position, Crop->_GameObjectInfo.ObjectPositionInfo.Position);
							if (Distance < 1.2f)
							{
								CMessage* ResGatheringPacket = nullptr;

								switch (GatheringTarget->_GameObjectInfo.ObjectType)
								{
								case en_GameObjectType::OBJECT_STONE:
									ResGatheringPacket = G_NetworkManager->GetGameServer()->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"돌 채집");
									break;
								case en_GameObjectType::OBJECT_TREE:
									ResGatheringPacket = G_NetworkManager->GetGameServer()->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"나무 벌목");
									break;
								case en_GameObjectType::OBJECT_CROP_CORN:
									ResGatheringPacket = G_NetworkManager->GetGameServer()->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"옥수수 수확");
									break;
								case en_GameObjectType::OBJECT_CROP_POTATO:
									ResGatheringPacket = G_NetworkManager->GetGameServer()->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"감자 수확");
									break;
								}

								_GatheringTarget = GatheringTarget;

								_GatheringTick = GetTickCount64() + 1000;

								_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::GATHERING;

								CMessage* ResObjectStateChangePacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectState(_GameObjectInfo.ObjectId,
									_GameObjectInfo.ObjectPositionInfo.State);
								G_NetworkManager->GetGameServer()->SendPacketFieldOfView(this, ResObjectStateChangePacket);
								ResObjectStateChangePacket->Free();

								G_NetworkManager->GetGameServer()->SendPacketFieldOfView(this, ResGatheringPacket);
								ResGatheringPacket->Free();
							}
							else
							{
								CMessage* DirErrorPacket = G_NetworkManager->GetGameServer()->MakePacketCommonError(en_GlobalMessageType::GLOBAL_MESSAGE_GATHERING_DISTANCE, Crop->_GameObjectInfo.ObjectName.c_str());
								G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, DirErrorPacket);
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
				CMessage* ResGatheringCancelPacket = G_NetworkManager->GetGameServer()->MakePacketGatheringCancel(_GameObjectInfo.ObjectId);
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(this, ResGatheringCancelPacket);
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

						FindAggroTargetIterator->second.AggroPoint += (Damage * (0.8f + G_Datamanager->_MonsterAggroData.MonsterAggroAttacker));
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

				int16 Skilltype;
				*GameObjectJob->GameObjectJobMessage >> Skilltype;

				int32 SkillMinDamage;                         
				*GameObjectJob->GameObjectJobMessage >> SkillMinDamage;

				int32 SkillMaxDamage;
				*GameObjectJob->GameObjectJobMessage >> SkillMaxDamage;
				
				CGameObject* Attacker = _Channel->FindChannelObject(AttackerID, (en_GameObjectType)AttackerType);
				if (Attacker != nullptr)
				{
					bool IsCritical = true;
					int32 Damage = 0;

					switch (Attacker->_GameObjectInfo.ObjectType)
					{
					case en_GameObjectType::OBJECT_PLAYER:
						{
							CPlayer* AttackerPlayer = dynamic_cast<CPlayer*>(Attacker);
							if (AttackerPlayer != nullptr)
							{
								Damage = AttackerPlayer->_SkillBox.CalculateDamage((en_SkillType)Skilltype,
									Attacker->_GameObjectInfo.ObjectStatInfo.Str,
									Attacker->_GameObjectInfo.ObjectStatInfo.Dex,
									Attacker->_GameObjectInfo.ObjectStatInfo.Int,
									Attacker->_GameObjectInfo.ObjectStatInfo.Luck,
									&IsCritical,
									_GameObjectInfo.ObjectStatInfo.Defence,
									Attacker->_GameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage + SkillMinDamage,
									Attacker->_GameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage + SkillMaxDamage,
									Attacker->_GameObjectInfo.ObjectStatInfo.MeleeCriticalPoint);
							}
						}
						break;
					case en_GameObjectType::OBJECT_GOBLIN:
						break;					
					}								

					bool IsDead = OnDamaged(Attacker, Damage);

					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

					CMessage* ResDamagePacket = G_NetworkManager->GetGameServer()->MakePacketResDamage(AttackerID,
						_GameObjectInfo.ObjectId,
						Skilltype,
						en_ResourceName::CLIENT_EFFECT_ATTACK_TARGET_HIT,
						Damage,
						_GameObjectInfo.ObjectStatInfo.HP,
						IsCritical);
					G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResDamagePacket);
					ResDamagePacket->Free();										
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

				vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Channel->GetMap()->GetFieldAroundPlayers(this, false);

				CGameObject* Healer = _Channel->FindChannelObject(HealerID, en_GameObjectType::OBJECT_PLAYER);				
				if (Healer != nullptr)
				{
					// 시스템 메세지 전송	
					CMessage* ResSkillSystemMessagePacket = G_NetworkManager->GetGameServer()->MakePacketResDamageChattingBoxMessage(en_MessageType::MESSAGE_TYPE_DAMAGE_CHATTING,
						Healer->_GameObjectInfo.ObjectName,
						_GameObjectInfo.ObjectName,
						(en_SkillType)HealSkillType,
						HealPoint);
					G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResSkillSystemMessagePacket);
					ResSkillSystemMessagePacket->Free();

					CMessage* StatChangePacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectStatInfo);
					G_NetworkManager->GetGameServer()->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, StatChangePacket);
					StatChangePacket->Free();
				}				
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_ON_EQUIPMENT:
			{
				// 착용할 장비 
				CItem* EquipmentItem;
				*GameObjectJob->GameObjectJobMessage >> &EquipmentItem;				
				
				// 장비 착용
				CPlayer* Player = dynamic_cast<CPlayer*>(this);
				if (Player != nullptr)
				{
					CItem* ReturnEquipmentItem = Player->_Equipment.ItemOnEquipment(EquipmentItem);

					CMessage* EquipmentUpdateMessage = G_NetworkManager->GetGameServer()->MakePacketOnEquipment(_GameObjectInfo.ObjectId, EquipmentItem->_ItemInfo);
					G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, EquipmentUpdateMessage);
					EquipmentUpdateMessage->Free();

					// 가방에서 착용한 장비 없애기
					Player->GetInventoryManager()->InitItem(0, EquipmentItem->_ItemInfo.ItemTileGridPositionX, EquipmentItem->_ItemInfo.ItemTileGridPositionY);

					// 장비 해제한 아이템이 있을 경우 가방에 해당 아이템을 새로 넣음
					if (ReturnEquipmentItem != nullptr)
					{
						Player->GetInventoryManager()->InsertItem(0, ReturnEquipmentItem);

						CMessage* ResItemToInventoryPacket = G_NetworkManager->GetGameServer()->MakePacketResItemToInventory(Player->_GameObjectInfo.ObjectId,
							ReturnEquipmentItem->_ItemInfo, false, ReturnEquipmentItem->_ItemInfo.ItemCount);
						G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResItemToInventoryPacket);
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
						// 클라 장비창에서 장비 해제
						CMessage* OffEquipmentMessage = G_NetworkManager->GetGameServer()->MakePacketOffEquipment(Player->_GameObjectInfo.ObjectId, ReturnEquipmentItem->_ItemInfo.ItemEquipmentPart);
						G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, OffEquipmentMessage);
						OffEquipmentMessage->Free();

						// 클라 인벤토리에 장비 해제한 아이템 넣음
						Player->GetInventoryManager()->InsertItem(0, ReturnEquipmentItem);

						// 클라에게 알려줌
						CMessage* ResItemToInventoryPacket = G_NetworkManager->GetGameServer()->MakePacketResItemToInventory(Player->_GameObjectInfo.ObjectId,
							ReturnEquipmentItem->_ItemInfo, false, ReturnEquipmentItem->_ItemInfo.ItemCount);
						G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResItemToInventoryPacket);
						ResItemToInventoryPacket->Free();
					}
					else
					{
						CRASH("착용한 장비가 없는데 장비해제 요청");
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
					if (DebufSkillIter.second->GetSkillInfo()->SkillType == en_SkillType::SKILL_PROTECTION_ACTIVE_ATTACK_CAPTURE
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

				CMessage* StatChangePacket = G_NetworkManager->GetGameServer()->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectStatInfo);
				G_NetworkManager->GetGameServer()->SendPacketFieldOfView(this, StatChangePacket);
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
					// 가방에 버리고자 하는 아이템이 있는지 확인
					CItem* FindDropItem = Player->GetInventoryManager()->FindInventoryItem(0, (en_SmallItemCategory)DropItemType);
					if (FindDropItem != nullptr)
					{
						// 아이템 개수가 맞는지 확인
						if (FindDropItem->_ItemInfo.ItemCount >= DropItemCount)
						{
							G_ObjectManager->ObjectItemDropToSpawn(Player, Player->GetChannel(), (en_SmallItemCategory)DropItemType,
								DropItemCount);

							FindDropItem->_ItemInfo.ItemCount -= DropItemCount;

							if (FindDropItem->_ItemInfo.ItemCount < 0)
							{
								FindDropItem->_ItemInfo.ItemCount = 0;
							}

							CMessage* DropItemUpdatePacket = G_NetworkManager->GetGameServer()->MakePacketInventoryItemUpdate(Player->_GameObjectInfo.ObjectId, FindDropItem->_ItemInfo);
							G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, DropItemUpdatePacket);
							DropItemUpdatePacket->Free();

							if (FindDropItem->_ItemInfo.ItemCount == 0)
							{
								Player->GetInventoryManager()->InitItem(0, FindDropItem->_ItemInfo.ItemTileGridPositionX, FindDropItem->_ItemInfo.ItemTileGridPositionY);
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
							Player->GetInventoryManager()->InsertMoney(0, InsertItem);

							CMessage* ResMoneyToInventoryPacket = G_NetworkManager->GetGameServer()->MakePacketResMoneyToInventory(Player->_GameObjectInfo.ObjectId,
								Player->GetInventoryManager()->GetGoldCoin(),
								Player->GetInventoryManager()->GetSliverCoin(),
								Player->GetInventoryManager()->GetBronzeCoin(),
								InsertItem->_ItemInfo,
								ItemEach);
							G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResMoneyToInventoryPacket);
							ResMoneyToInventoryPacket->Free();
						}
						break;
					default:
						{
							CItem* FindItem = Player->GetInventoryManager()->FindInventoryItem(0, InsertItem->_ItemInfo.ItemSmallCategory);
							if (FindItem == nullptr)
							{
								CItem* NewItem = G_NetworkManager->GetGameServer()->NewItemCrate(InsertItem->_ItemInfo);
								Player->GetInventoryManager()->InsertItem(0, NewItem);

								FindItem = Player->GetInventoryManager()->GetItem(0, NewItem->_ItemInfo.ItemTileGridPositionX, NewItem->_ItemInfo.ItemTileGridPositionY);
							}
							else
							{
								IsExistItem = true;
								FindItem->_ItemInfo.ItemCount += ItemEach;
							}

							CMessage* ResItemToInventoryPacket = G_NetworkManager->GetGameServer()->MakePacketResItemToInventory(Player->_GameObjectInfo.ObjectId,
								FindItem->_ItemInfo, IsExistItem, ItemEach);
							G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResItemToInventoryPacket);
							ResItemToInventoryPacket->Free();
						}
						break;
					}

					st_GameObjectJob* DeSpawnMonsterChannelJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobObjectDeSpawnObjectChannel(InsertItem);
					_Channel->_ChannelJobQue.Enqueue(DeSpawnMonsterChannelJob);

					st_GameObjectJob* LeaveChannerMonsterJob = G_NetworkManager->GetGameServer()->MakeGameObjectJobLeaveChannel(InsertItem);
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
					// 넣을 아이템이 넣는 대상의 인벤토리에 있는지 확인
					CItem* FindAddItem = CraftingTableItemAddPlayer->GetInventoryManager()->FindInventoryItem(0, (en_SmallItemCategory)AddItemSmallCategory);
					if (FindAddItem != nullptr && FindAddItem->_ItemInfo.ItemCount > 0)
					{
						// 제작법을 가지고 옴
						st_CraftingTableRecipe CraftingTableRecipe = CraftingTable->GetCraftingTableRecipe();

						bool IsMaterial = false;

						// 제작대 제작법 아이템 중
						for (CItem* Item : CraftingTableRecipe.CraftingTableCompleteItems)
						{
							// 제작하고자 하는 제작 아이템을 찾고
							if (Item->_ItemInfo.ItemSmallCategory == CraftingTable->_SelectCraftingItemType)
							{
								// 요청한 재료템이 제작 아이템의 재료로 들어가는지 최종확인
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

							CMessage* ResInventoryItemUpdatePacket = G_NetworkManager->GetGameServer()->MakePacketInventoryItemUpdate(CraftingTableItemAddPlayer->_GameObjectInfo.ObjectId,
								FindAddItem->_ItemInfo);
							G_NetworkManager->GetGameServer()->SendPacket(CraftingTableItemAddPlayer->_SessionId, ResInventoryItemUpdatePacket);
							ResInventoryItemUpdatePacket->Free();

							if (FindAddItem->_ItemInfo.ItemCount == 0)
							{
								CraftingTableItemAddPlayer->GetInventoryManager()->InitItem(0, FindAddItem->_ItemInfo.ItemTileGridPositionX, FindAddItem->_ItemInfo.ItemTileGridPositionY);
							}

							CMessage* ResCraftingTableAddItemPacket = G_NetworkManager->GetGameServer()->MakePacketResCraftingTableInput(_GameObjectInfo.ObjectId, CraftingTable->GetMaterialItems());
							G_NetworkManager->GetGameServer()->SendPacket(CraftingTableItemAddPlayer->_SessionId, ResCraftingTableAddItemPacket);
							ResCraftingTableAddItemPacket->Free();
						}
						else
						{
							CMessage* WrongItemInputMessage = G_NetworkManager->GetGameServer()->MakePacketCommonError(en_GlobalMessageType::GLOBAL_MESSAGE_CRAFTING_TABLE_MATERIAL_WRONG_ITEM_ADD);
							G_NetworkManager->GetGameServer()->SendPacket(CraftingTableItemAddPlayer->_SessionId, WrongItemInputMessage);
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
						CItem* InsertItem = CraftingTableItemSubtractPlayer->GetInventoryManager()->InsertItem(0, (en_SmallItemCategory)SubtractItemSmallCategory, SubtractItemCount, &IsExistItem);

						CMessage* InsertItemToInventoryPacket = G_NetworkManager->GetGameServer()->MakePacketResItemToInventory(CraftingTableItemSubtractPlayer->_GameObjectInfo.ObjectId,
							InsertItem->_ItemInfo,
							IsExistItem,
							SubtractItemCount);
						G_NetworkManager->GetGameServer()->SendPacket(CraftingTableItemSubtractPlayer->_SessionId, InsertItemToInventoryPacket);
						InsertItemToInventoryPacket->Free();

						CMessage* ResCraftingTableMaterialItemListPacket = G_NetworkManager->GetGameServer()->MakePacketResCraftingTableMaterialItemList(
							_GameObjectInfo.ObjectId,
							_GameObjectInfo.ObjectType,
							CraftingTable->_SelectCraftingItemType,
							CraftingTable->GetMaterialItems());
						G_NetworkManager->GetGameServer()->SendPacket(CraftingTableItemSubtractPlayer->_SessionId, ResCraftingTableMaterialItemListPacket);
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
						CItem* InsertItem = CraftingTableItemSubtractPlayer->GetInventoryManager()->InsertItem(0, (en_SmallItemCategory)SubtractItemSmallCategory, SubtractItemCount, &IsExistItem);

						CMessage* InsertItemToInventoryPacket = G_NetworkManager->GetGameServer()->MakePacketResItemToInventory(CraftingTableItemSubtractPlayer->_GameObjectInfo.ObjectId,
							InsertItem->_ItemInfo,
							IsExistItem,
							SubtractItemCount);
						G_NetworkManager->GetGameServer()->SendPacket(CraftingTableItemSubtractPlayer->_SessionId, InsertItemToInventoryPacket);
						InsertItemToInventoryPacket->Free();

						CMessage* ResCraftingTableMaterialItemListPacket = G_NetworkManager->GetGameServer()->MakePacketResCraftingTableCompleteItemList(
							_GameObjectInfo.ObjectId,
							_GameObjectInfo.ObjectType,
							CraftingTable->GetCompleteItems());
						G_NetworkManager->GetGameServer()->SendPacket(CraftingTableItemSubtractPlayer->_SessionId, ResCraftingTableMaterialItemListPacket);
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
								// 용광로가 가지고 있는 제작법을 가지고옴
								st_CraftingTableRecipe CraftingTableRecipe = CraftingTable->GetCraftingTableRecipe();

								// 제작법 중에서 요청한 아이템 타입의 제작법이 있는지 찾음
								for (CItem* CraftingCompleteItem : CraftingTableRecipe.CraftingTableCompleteItems)
								{
									if (CraftingCompleteItem->_ItemInfo.ItemSmallCategory == (en_SmallItemCategory)CraftingCompleteItemType)
									{
										bool IsCrafting = true;

										// 찾으면 가방에 제작법 재료가 있는지 확인함
										for (st_CraftingMaterialItemInfo CraftingMaterialItemInfo : CraftingCompleteItem->_ItemInfo.Materials)
										{
											// 제작템을 한개 만들떄 필요한 재료의 개수를 얻는다.
											int16 OneReqMaterialcount = CraftingMaterialItemInfo.ItemCount;
											// 클라가 요청한 제작템 제작갯수와 위에서 구한 개수를 곱해서 총 필요개수를 구한다.
											int16 ReqCraftingItemTotalCount = CraftingCount * OneReqMaterialcount;

											// 인벤토리에 제작템의 최소 재료 필요 개수만큼 용광로 재료템에 있는지 확인한다.
											if (!CraftingTable->FindMaterialItem(CraftingMaterialItemInfo.MaterialItemType, OneReqMaterialcount))
											{
												IsCrafting = false;
											}
										}

										// 재료가 모두 있을 경우
										if (IsCrafting == true)
										{
											CraftingTable->_CraftingStartCompleteItem = CraftingCompleteItem->_ItemInfo.ItemSmallCategory;

											// 아이템 제작 시작
											CraftingCompleteItem->CraftingStart();

											CPlayer* Player = (CPlayer*)CraftingTable->_SelectedObject;
											CMessage* ResCraftingstartPacket = G_NetworkManager->GetGameServer()->MakePacketResCraftingStart(
												_GameObjectInfo.ObjectId,
												CraftingCompleteItem->_ItemInfo);
											G_NetworkManager->GetGameServer()->SendPacket(Player->_SessionId, ResCraftingstartPacket);

											// 용광로 제작 상태 진입
											CraftingTable->CraftingStart();
										}
										else
										{
											// 재료 부족 에러 메세지 띄우기
											CMessage* CraftingStartErrorPacket = G_NetworkManager->GetGameServer()->MakePacketCommonError(en_GlobalMessageType::GLOBAL_MESSAGE_CRAFTING_TABLE_MATERIAL_COUNT_NOT_ENOUGH);
											G_NetworkManager->GetGameServer()->SendPacket(((CPlayer*)CraftingStartObject)->_SessionId, CraftingStartErrorPacket);
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
					CMessage* CraftingStartErrorPacket = G_NetworkManager->GetGameServer()->MakePacketCommonError(en_GlobalMessageType::GLOBAL_MESSAGE_CRAFTING_TABLE_OVERLAP_CRAFTING_START);
					G_NetworkManager->GetGameServer()->SendPacket(((CPlayer*)CraftingStartObject)->_SessionId, CraftingStartErrorPacket);
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

						CMessage* CraftingTableCraftingStopPacket = G_NetworkManager->GetGameServer()->MakePacketResCraftingStop(
							_GameObjectInfo.ObjectId,
							CraftingStopItem->_ItemInfo);
						G_NetworkManager->GetGameServer()->SendPacket(((CPlayer*)CraftingStoptObject)->_SessionId, CraftingTableCraftingStopPacket);
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

	// 힐을 대상에게 시전시 주위 몬스터들에게 힐 어그로를 계산하기 위해 알려준다.
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

Vector2 CGameObject::PositionCheck(Vector2Int& CheckPosition)
{
	Vector2 ResultPosition;

	if (CheckPosition.Y > 0)
	{
		ResultPosition.Y =
			CheckPosition.Y + 0.5f;
	}
	else if (CheckPosition.Y == 0)
	{
		ResultPosition.Y =
			CheckPosition.Y;
	}
	else if (CheckPosition.Y < 0)
	{
		ResultPosition.Y =
			CheckPosition.Y - 0.5f;
	}

	if (CheckPosition.X > 0)
	{
		ResultPosition.X =
			CheckPosition.X + 0.5f;
	}
	else if (CheckPosition.X == 0)
	{
		ResultPosition.X =
			CheckPosition.X;
	}
	else if (CheckPosition.X < 0)
	{
		ResultPosition.X =
			CheckPosition.X - 0.5f;
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

vector<Vector2Int> CGameObject::GetAroundCellPositions(Vector2Int CellPosition, int8 Distance)
{
	vector<Vector2Int> AroundPosition;	

	Vector2Int LeftTop(Distance * -1, Distance);
	Vector2Int RightDown(Distance, Distance * -1);

	Vector2Int LeftTopPosition = CellPosition + LeftTop;
	Vector2Int RightDownPosition = CellPosition + RightDown;

	for (int32 Y = LeftTopPosition.Y; Y >= RightDownPosition.Y; Y--)
	{
		for (int32 X = LeftTopPosition.X; X <= RightDownPosition.X; X++)
		{
			if (X == CellPosition.X && Y == CellPosition.Y)
			{
				continue;
			}

			AroundPosition.push_back(Vector2Int(X, Y));
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

void CGameObject::SetStatusAbnormal(int64 StatusAbnormalValue)
{
	_StatusAbnormal |= StatusAbnormalValue;
}

void CGameObject::ReleaseStatusAbnormal(int64 StatusAbnormalValue)
{
	_StatusAbnormal &= StatusAbnormalValue;
}

int64 CGameObject::CheckCantControlStatusAbnormal()
{
	int64 StatusAbnormalCount = 0;

	if (_StatusAbnormal & (int64)en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_CAPTURE)
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & (int64)en_GameObjectStatusType::STATUS_ABNORMAL_PROTECTION_SHIELD_SMASH)
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & (int64)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ICE_WAVE)
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & (int64)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_LIGHTNING_STRIKE)
	{
		StatusAbnormalCount++;
	}

	if (_StatusAbnormal & (int64)en_GameObjectStatusType::STATUS_ABNORMAL_ASSASSINATION_BACK_STEP)
	{
		StatusAbnormalCount++;
	}

	return StatusAbnormalCount;
}

int64 CGameObject::CheckCanControlStatusAbnormal()
{
	int64 StatusAbnormalCount = 0;

	if (_StatusAbnormal & (int64)en_GameObjectStatusType::STATUS_ABNORMAL_FIGHT_JUMPING_ATTACK)
	{
		StatusAbnormalCount++;
	}

	if ((_StatusAbnormal & (int64)en_GameObjectStatusType::STATUS_ABNORMAL_SPELL_ROOT)
		|| (_StatusAbnormal & (int64)en_GameObjectStatusType::STATUS_ABNORMAL_DISCIPLINE_ROOT))
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
	if (_RectCollision == nullptr)
	{
		_RectCollision = G_ObjectManager->RectCollisionCreate();
	}

	return _RectCollision;
}

void CGameObject::Init(en_GameObjectType GameObjectType)
{
	_GameObjectInfo.ObjectType = GameObjectType;		
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

vector<CGameObject*> CGameObject::GetFieldOfViewObjects()
{
	return _FieldOfViewObjects;
}

void CGameObject::SetMeleeSkill(CSkill* MeleeSkill)
{
	_MeleeSkill = MeleeSkill;
}

vector<st_FieldOfViewInfo> CGameObject::GetFieldOfViewInfo()
{
	return _FieldOfViewInfos;
}

void CGameObject::UpdateSpawnReady()
{
	
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

void CGameObject::UpdateRooting()
{
}

void CGameObject::UpdateDead()
{
}

void CGameObject::CheckBufDeBufSkill()
{
	if (_Bufs.size() > 0)
	{
		// 강화효과 스킬 리스트 순회
		for (auto BufSkillIterator : _Bufs)
		{
			// 지속시간 끝난 강화효과 삭제
			bool DeleteBufSkill = BufSkillIterator.second->Update();
			if (DeleteBufSkill)
			{
				DeleteBuf(BufSkillIterator.first);
				// 강화효과 스킬 정보 메모리 반납
				G_ObjectManager->SkillInfoReturn(BufSkillIterator.second->GetSkillInfo()->SkillType,
					BufSkillIterator.second->GetSkillInfo());
				// 강화효과 스킬 메모리 반납
				G_ObjectManager->SkillReturn(BufSkillIterator.second);
			}
		}
	}	

	if (_DeBufs.size() > 0)
	{
		// 약화효과 스킬 리스트 순회
		for (auto DebufSkillIterator : _DeBufs)
		{
			// 지속시간 끝난 약화효과 삭제
			bool DeleteDebufSkill = DebufSkillIterator.second->Update();
			if (DeleteDebufSkill)
			{
				DeleteDebuf(DebufSkillIterator.first);
				// 약화효과 스킬 정보 메모리 반납
				G_ObjectManager->SkillInfoReturn(DebufSkillIterator.second->GetSkillInfo()->SkillType,
					DebufSkillIterator.second->GetSkillInfo());
				// 약화효과 스킬 메모리 반납
				G_ObjectManager->SkillReturn(DebufSkillIterator.second);
			}
		}
	}	
}