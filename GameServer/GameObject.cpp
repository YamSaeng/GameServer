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
	_Owner = nullptr;
	_SelectTarget = nullptr;	

	_NatureRecoveryTick = 0;
	_FieldOfViewUpdateTick = 0;
	_IsSendPacketTarget = false;

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

			// 바뀐 좌표 값 시야범위 오브젝트들에게 전송
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_COMBO_ATTACK_CREATE:
			{
				int8 QuickSlotBarIndex;
				*GameObjectJob->GameObjectJobMessage >> QuickSlotBarIndex;
				int8 QuickSlotBarSlotIndex;
				*GameObjectJob->GameObjectJobMessage >> QuickSlotBarSlotIndex;
				CSkill* ReqMeleeSkill;
				*GameObjectJob->GameObjectJobMessage >> &ReqMeleeSkill;

				CPlayer* MyPlayer = ((CPlayer*)this);

				// 연속기 스킬이 스킬창에서 찾는다.
				CSkill* FindComboSkill = MyPlayer->_SkillBox.FindSkill(ReqMeleeSkill->GetSkillInfo()->NextComboSkill);
				if (FindComboSkill != nullptr)
				{
					// 연속기 스킬 생성
					CSkill* NewComboSkill = G_ObjectManager->SkillCreate();				
					// 정보를 저장
					NewComboSkill->SetSkillInfo(en_SkillCategory::COMBO_SKILL, FindComboSkill->GetSkillInfo(), ReqMeleeSkill->GetSkillInfo());
					NewComboSkill->SetOwner(MyPlayer);

					// 연속기 스킬 정보 입력
					MyPlayer->_ComboSkill = NewComboSkill;

					NewComboSkill->ComboSkillStart(QuickSlotBarIndex, QuickSlotBarSlotIndex, FindComboSkill->GetSkillInfo()->SkillType);

					CMessage* ResNextComboSkill = G_ObjectManager->GameServer->MakePacketComboSkillOn(QuickSlotBarIndex,
						QuickSlotBarSlotIndex,
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
						
				_CurrentSpellSkill = ReqSpellSkill;

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
						ResGatheringPacket = G_ObjectManager->GameServer->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"돌 채집");
						break;
					case en_GameObjectType::OBJECT_TREE:
						ResGatheringPacket = G_ObjectManager->GameServer->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"나무 벌목");
						break;
					case en_GameObjectType::OBJECT_CROP_POTATO:
						ResGatheringPacket = G_ObjectManager->GameServer->MakePacketResGathering(_GameObjectInfo.ObjectId, true, L"감자 수확");
						break;
					default:
						CRASH("채집할 수 없는 채집물 채집 요청");
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_AGGRO_LIST_INSERT_OR_UPDATE:
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_AGGRO_LIST_REMOVE:
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_DAMAGE:
			{
				CGameObject* Attacker;
				*GameObjectJob->GameObjectJobMessage >> &Attacker;

				int32 Damage;
				*GameObjectJob->GameObjectJobMessage >> Damage;

				if (OnDamaged(Attacker, Damage))
				{
					End();
					
					ExperienceCalculate((CPlayer*)Attacker, this);
					// 캐릭터 죽으면 버프 디버프 창 관리해야함
					Attacker->_SelectTarget = nullptr;
				}

				CMessage* StatChangePacket = G_ObjectManager->GameServer->MakePacketResChangeObjectStat(_GameObjectInfo.ObjectId, _GameObjectInfo.ObjectStatInfo);
				G_ObjectManager->GameServer->SendPacketFieldOfView(this, StatChangePacket);
				StatChangePacket->Free();
			}
			break;
		case en_GameObjectJobType::GAMEOJBECT_JOB_HEAL:
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_FULL_RECOVERY:
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_ITEM_DROP:
			{
				int16 DropItemType;
				*GameObjectJob->GameObjectJobMessage >> DropItemType;

				int32 DropItemCount;
				*GameObjectJob->GameObjectJobMessage >> DropItemCount;

				CPlayer* Player = (CPlayer*)this;

				// 가방에 버리고자 하는 아이템이 있는지 확인
				CItem* FindDropItem = Player->_InventoryManager.FindInventoryItem(0, (en_SmallItemCategory)DropItemType);
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

						CMessage* DropItemUpdatePacket = G_ObjectManager->GameServer->MakePacketInventoryItemUpdate(Player->_GameObjectInfo.ObjectId, FindDropItem->_ItemInfo);
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, DropItemUpdatePacket);
						DropItemUpdatePacket->Free();

						if (FindDropItem->_ItemInfo.ItemCount == 0)
						{
							Player->_InventoryManager.InitItem(0, FindDropItem->_ItemInfo.TileGridPositionX, FindDropItem->_ItemInfo.TileGridPositionY);
						}						
					}
				}
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_ITEM_INVENTORY_SAVE:
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

							FindItem = Player->_InventoryManager.GetItem(0, NewItem->_ItemInfo.TileGridPositionX, NewItem->_ItemInfo.TileGridPositionY);
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_CRAFTING_TABLE_SELECT:
			{
				CGameObject* SelectCraftingTableObject;
				*GameObjectJob->GameObjectJobMessage >> &SelectCraftingTableObject;
				CGameObject* OwnerObject;
				*GameObjectJob->GameObjectJobMessage >> &OwnerObject;

				CCraftingTable* CraftingTableObject = (CCraftingTable*)SelectCraftingTableObject;
				
				CraftingTableObject->_SelectedCraftingTable = true;		

				CraftingTableObject->_SelectedObject = OwnerObject;				
								
				// 제작대가 제작중이라면 제작중인 아이템의 정보를 클라에게 보냄
				if (CraftingTableObject->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::CRAFTING)
				{
					for (CItem* CraftingTableItem : CraftingTableObject->GetCraftingTableRecipe().CraftingTableCompleteItems)
					{
						CMessage* ResCraftingTableSelectPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableCraftRemainTime(
							CraftingTableObject->_GameObjectInfo.ObjectId,
							CraftingTableItem->_ItemInfo);
						G_ObjectManager->GameServer->SendPacket(((CPlayer*)CraftingTableObject->_SelectedObject)->_SessionId, ResCraftingTableSelectPacket);
						ResCraftingTableSelectPacket->Free();
					}			
				}

				// 완성된 제작품이 있을 경우 목록을 보내준다.
				if (CraftingTableObject->GetCompleteItems().size() > 0)
				{
					CMessage* ResCrafintgTableCompleteItemListPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableCompleteItemList(
						_GameObjectInfo.ObjectId,
						_GameObjectInfo.ObjectType,
						CraftingTableObject->GetCompleteItems());
					G_ObjectManager->GameServer->SendPacket(((CPlayer*)CraftingTableObject->_SelectedObject)->_SessionId, ResCrafintgTableCompleteItemListPacket);
					ResCrafintgTableCompleteItemListPacket->Free();
				}				
			}
			break;
		case en_GameObjectJobType::GAMEOJBECT_JOB_CRAFTING_TABLE_NON_SELECT:
			{
				CGameObject* SelectCraftingTableObject;
				*GameObjectJob->GameObjectJobMessage >> &SelectCraftingTableObject;

				CCraftingTable* CraftingTableObject = (CCraftingTable*)SelectCraftingTableObject;

				CraftingTableObject->_SelectedCraftingTable = false;

				CraftingTableObject->_SelectedObject = nullptr;

				CraftingTableObject->_SelectCraftingItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;
			}
			break;
		case en_GameObjectJobType::GAMEOBJECT_JOB_CRAFTING_TABLE_ITEM_ADD:
			{
				CGameObject* CraftingTableItemAddPlayerGO;
				*GameObjectJob->GameObjectJobMessage >> &CraftingTableItemAddPlayerGO;

				int16 AddItemSmallCategory;
				*GameObjectJob->GameObjectJobMessage >> AddItemSmallCategory;

				int16 AddItemCount;
				*GameObjectJob->GameObjectJobMessage >> AddItemCount;

				CCraftingTable* CraftingTable = (CCraftingTable*)this;

				CPlayer* CraftingTableItemAddPlayer = (CPlayer*)CraftingTableItemAddPlayerGO;

				// 넣을 아이템이 넣는 대상의 인벤토리에 있는지 확인
				CItem* FindAddItem = CraftingTableItemAddPlayer->_InventoryManager.FindInventoryItem(0, (en_SmallItemCategory)AddItemSmallCategory);
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

						CMessage* ResInventoryItemUpdatePacket = G_ObjectManager->GameServer->MakePacketInventoryItemUpdate(CraftingTableItemAddPlayer->_GameObjectInfo.ObjectId,
							FindAddItem->_ItemInfo);
						G_ObjectManager->GameServer->SendPacket(CraftingTableItemAddPlayer->_SessionId, ResInventoryItemUpdatePacket);
						ResInventoryItemUpdatePacket->Free();
						
						if (FindAddItem->_ItemInfo.ItemCount == 0)
						{
							CraftingTableItemAddPlayer->_InventoryManager.InitItem(0, FindAddItem->_ItemInfo.TileGridPositionX, FindAddItem->_ItemInfo.TileGridPositionY);
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_CRAFTING_TABLE_MATERIAL_ITEM_SUBTRACT:
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_CRAFTING_TABLE_COMPLETE_ITEM_SUBTRACT:
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_CRAFTING_TABLE_CRAFTING_START:
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
										CMessage* ResCraftingstartPacket = G_ObjectManager->GameServer->MakePacketResCraftingStart(
											_GameObjectInfo.ObjectId,
											CraftingCompleteItem->_ItemInfo);
										G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCraftingstartPacket);
										
										// 용광로 제작 상태 진입
										CraftingTable->CraftingStart();
									}
									else
									{
										// 재료 부족 에러 메세지 띄우기
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
		case en_GameObjectJobType::GAMEOBJECT_JOB_CRAFTING_TABLE_CRAFTING_STOP:
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

	// 힐을 대상에게 시전시 주위 몬스터들에게 힐 어그로를 계산하기 위해 알려준다.
	vector<CMonster*> AroundMonsters = _Channel->GetMap()->GetAroundMonster(Healer, 1);
	for (CMonster* AroundMonster : AroundMonsters)
	{
		st_GameObjectJob* AggroUpdateJob = G_ObjectManager->GameObjectJobCreate();
		AggroUpdateJob->GameObjectJobType = en_GameObjectJobType::GAMEOBJECT_JOB_AGGRO_LIST_INSERT_OR_UPDATE;

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
			// 레벨 증가
			TargetPlayer->_GameObjectInfo.ObjectStatInfo.Level += 1;

			// 증가한 레벨에 해당하는 능력치 정보를 읽어온 후 적용한다.
			st_ObjectStatusData NewCharacterStatus;
			st_LevelData LevelData;

			switch (TargetPlayer->_GameObjectInfo.ObjectType)
			{
			case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
			{
				auto FindStatus = G_Datamanager->_WarriorStatus.find(TargetPlayer->_GameObjectInfo.ObjectStatInfo.Level);
				if (FindStatus == G_Datamanager->_WarriorStatus.end())
				{
					CRASH("레벨 스테이터스 찾지 못함");
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
					CRASH("레벨 데이터 찾지 못함");
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
					CRASH("레벨 데이터 찾지 못함");
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
					CRASH("레벨 데이터 찾지 못함");
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
					CRASH("레벨 데이터 찾지 못함");
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
				CRASH("레벨 데이터 찾지 못함");
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
