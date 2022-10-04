#include "pch.h"
#include "Channel.h"
#include "Player.h"
#include "Message.h"
#include "Monster.h"
#include "Item.h"
#include "Heap.h"
#include "Environment.h"
#include "CraftingTable.h"
#include "Crop.h"
#include "Map.h"
#include "Potato.h"
#include "ObjectManager.h"
#include "RectCollision.h"

CChannel::CChannel()
{	
	InitializeSRWLock(&_ChannelLock);

	for (int32 PlayerCount = PLAYER_MAX - 1; PlayerCount >= 0; --PlayerCount)
	{
		_ChannelPlayerArray[PlayerCount] = nullptr;
		_ChannelPlayerArrayIndexs.Push(PlayerCount);
	}
	
	for (int32 DummyPlayerCount = DUMMY_PLAYER_MAX - 1; DummyPlayerCount >= 0; --DummyPlayerCount)
	{
		_ChannelDummyPlayerArray[DummyPlayerCount] = nullptr;
		_ChannelDummyPlayerArrayIndexs.Push(DummyPlayerCount);
	}

	for (int32 MonsterCount = MONSTER_MAX - 1; MonsterCount >= 0; --MonsterCount)
	{
		_ChannelMonsterArray[MonsterCount] = nullptr;
		_ChannelMonsterArrayIndexs.Push(MonsterCount);
	}

	for (int32 Environment = ENVIRONMENT_MAX - 1; Environment >= 0; --Environment)
	{
		_ChannelEnvironmentArray[Environment] = nullptr;
		_ChannelEnvironmentArrayIndexs.Push(Environment);
	}

	for (int32 Crafting = CRAFTING_TABLE_MAX - 1; Crafting >= 0; --Crafting)
	{
		_ChannelCraftingTableArray[Crafting] = nullptr;
		_ChannelCraftingTableArrayIndexs.Push(Crafting);
	}

	for (int32 CropCount = CROP_MAX - 1; CropCount >= 0; --CropCount)
	{
		_ChannelCropArray[CropCount] = nullptr;
		_ChannelCropArrayIndexs.Push(CropCount);
	}

	for (int32 ItemCount = ITEM_MAX - 1; ItemCount >= 0; --ItemCount)
	{
		_ChannelItemArray[ItemCount] = nullptr;
		_ChannelItemArrayIndexs.Push(ItemCount);
	}	
}

CChannel::~CChannel()
{
	for (int i = 0; i < _SectorCountY; i++)
	{
		delete _Sectors[i];
		_Sectors[i] = nullptr;
	}

	delete _Map;
	delete _Sectors;
}

void CChannel::Init()
{
	
}

void CChannel::Update()
{
	while (!_ChannelJobQue.IsEmpty())
	{
		st_GameObjectJob* GameObjectJob = nullptr;

		if (!_ChannelJobQue.Dequeue(&GameObjectJob))
		{
			break;
		}

		if (GameObjectJob != nullptr)
		{
			switch ((en_GameObjectJobType)GameObjectJob->GameObjectJobType)
			{
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_OBJECT_SPAWN:
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_OBJECT_DESPAWN:
				{
					CGameObject* DeSpawnObject;
					*GameObjectJob->GameObjectJobMessage >> &DeSpawnObject;					

					// 나 포함해서 주위 시야범위 플레이어 조사
					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Map->GetFieldOfViewPlayers(DeSpawnObject, 1, false);

					switch (DeSpawnObject->_GameObjectInfo.ObjectType)
					{
					case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
					case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
					case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
					case en_GameObjectType::OBJECT_THIEF_PLAYER:
					case en_GameObjectType::OBJECT_ARCHER_PLAYER:
					case en_GameObjectType::OBJECT_SLIME:						
						// 충돌 감지 박스 비활성화
						DeSpawnObject->GetRectCollision()->SetActive(false);
						break;					
					}					

					// 주위 시야범위 플레이어들에게 해당 오브젝트를 소환해제 하라고 알림
					CMessage* ResObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(DeSpawnObject->_GameObjectInfo.ObjectId);
					G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResObjectDeSpawnPacket);
					ResObjectDeSpawnPacket->Free();

					_Map->ApplyLeave(DeSpawnObject);
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PLAYER_ENTER:
				{				
					CPlayer* EnterPlayer;
					*GameObjectJob->GameObjectJobMessage >> &EnterPlayer;

					EnterPlayer->_SpawnIdleTick = GetTickCount64() + 5000;
					EnterPlayer->_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::SPAWN_IDLE;									

					// 충돌 감지 활성화
					EnterPlayer->GetRectCollision()->SetActive(true);

					EnterChannel(EnterPlayer, &EnterPlayer->_SpawnPosition);					

					// 나한테 나 생성하라고 알려줌
					CMessage* ResEnterGamePacket = G_ObjectManager->GameServer->MakePacketResEnterGame(true, &EnterPlayer->_GameObjectInfo, &EnterPlayer->_SpawnPosition);
					G_ObjectManager->GameServer->SendPacket(EnterPlayer->_SessionId, ResEnterGamePacket);
					ResEnterGamePacket->Free();

					if (EnterPlayer->_GameObjectInfo.ObjectType != en_GameObjectType::OBJECT_PLAYER_DUMMY)
					{
						st_GameServerJob* DBCharacterInfoSendJob = G_ObjectManager->GameServer->_GameServerJobMemoryPool->Alloc();
						DBCharacterInfoSendJob->Type = en_GameServerJobType::DATA_BASE_CHARACTER_INFO_SEND;

						CGameServerMessage* ReqDBCharacterInfoMessage = CGameServerMessage::GameServerMessageAlloc();
						ReqDBCharacterInfoMessage->Clear();

						*ReqDBCharacterInfoMessage << EnterPlayer->_SessionId;

						DBCharacterInfoSendJob->Message = ReqDBCharacterInfoMessage;

						G_ObjectManager->GameServer->_GameServerUserDBThreadMessageQue.Enqueue(DBCharacterInfoSendJob);
						SetEvent(G_ObjectManager->GameServer->_UserDataBaseWakeEvent);
					}	
					else
					{
						EnterPlayer->_NetworkState = en_ObjectNetworkState::LIVE;
					}
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_OBJECT_ENTER:
				{
					CGameObject* EnterObject;
					*GameObjectJob->GameObjectJobMessage >> &EnterObject;										
										
					switch (EnterObject->_GameObjectInfo.ObjectType)
					{
					case en_GameObjectType::OBJECT_SLIME:
					case en_GameObjectType::OBJECT_BEAR:
						{
							CMonster* EnterChannelMonster = (CMonster*)EnterObject;

							// 충돌 감지 박스 활성화
							EnterChannelMonster->GetRectCollision()->SetActive(true);

							EnterChannelMonster->_FieldOfViewPlayers = _Map->GetFieldOfViewPlayer(EnterChannelMonster, EnterChannelMonster->_FieldOfViewDistance);						
							
							EnterChannel(EnterChannelMonster, &EnterChannelMonster->_SpawnPosition);
						}
						break;
					case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
					case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_ARMOR:
					case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
					case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
					case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
					case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
					case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
					case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
					case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
					case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
					case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
					case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
					case en_GameObjectType::OBJECT_ITEM_MATERIAL_CHAR_COAL:
					case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_NUGGET:
					case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_INGOT:
					case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_NUGGET:
					case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_INGOT:
					case en_GameObjectType::OBJECT_ITEM_CROP_SEED_POTATO:
					case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
						{
							CItem* Item = (CItem*)EnterObject;

							bool IsItemEnterChannel = EnterChannel(EnterObject, &EnterObject->_SpawnPosition);
							if (IsItemEnterChannel == true)
							{			
								Item->SetDestoryTime(30000);
								Item->ItemSetTarget(Item->_GameObjectInfo.OwnerObjectType, Item->_GameObjectInfo.OwnerObjectId);
							}
							else
							{
								G_ObjectManager->ObjectReturn(Item->_GameObjectInfo.ObjectType, Item);
							}
						}
						break;	
					case en_GameObjectType::OBJECT_STONE:
					case en_GameObjectType::OBJECT_TREE:
						{
							CEnvironment* Entervironment = (CEnvironment*)EnterObject;

							Entervironment->GetRectCollision()->SetActive(true);

							EnterChannel(Entervironment, &Entervironment->_SpawnPosition);
						}
						break;
					case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
					case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
						{
							CCraftingTable* CraftingTable = (CCraftingTable*)EnterObject;

							CraftingTable->GetRectCollision()->SetActive(true);

							EnterChannel(CraftingTable, &CraftingTable->_SpawnPosition);
						}
						break;
					case en_GameObjectType::OBJECT_CROP_POTATO:
					case en_GameObjectType::OBJECT_CROP_CORN:
						{
							CPotato* Potato = (CPotato*)EnterObject;

							EnterChannel(Potato, &Potato->_SpawnPosition);
						}
						break;
					}										

					//G_Logger->WriteStdOut(en_Color::RED, L"ObjectID %d EnterChannel\n", EnterObject->_GameObjectInfo.ObjectId);

					CMessage* SpawnObjectPacket = G_ObjectManager->GameServer->MakePacketResObjectSpawn(EnterObject);
					G_ObjectManager->GameServer->SendPacketFieldOfView(EnterObject, SpawnObjectPacket);
					SpawnObjectPacket->Free();					
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_LEAVE:
				{
					CGameObject* LeaveGameObject;
					*GameObjectJob->GameObjectJobMessage >> &LeaveGameObject;										

					LeaveChannel(LeaveGameObject);
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PLAYER_LEAVE:
				{
					CGameObject* LeaveGameObject;
					*GameObjectJob->GameObjectJobMessage >> &LeaveGameObject;			

					LeaveChannel(LeaveGameObject);

					// 나 포함해서 주위 시야범위 플레이어 조사
					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Map->GetFieldOfViewPlayers(LeaveGameObject, 1, false);

					CMessage* ResObjectDeSpawnPacket = G_ObjectManager->GameServer->MakePacketResObjectDeSpawn(LeaveGameObject->_GameObjectInfo.ObjectId);
					G_ObjectManager->GameServer->SendPacketFieldOfView(CurrentFieldOfViewObjectIDs, ResObjectDeSpawnPacket);
					ResObjectDeSpawnPacket->Free();

					LeaveGameObject->_NetworkState = en_ObjectNetworkState::LEAVE;

					LeaveGameObject->End();

					for (int8 i = 0; i < SESSION_CHARACTER_MAX; i++)
					{
						int32 PlayerIndex;
						*GameObjectJob->GameObjectJobMessage >> PlayerIndex;

						G_ObjectManager->PlayerIndexReturn(PlayerIndex);
					}					
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_FIND_OBJECT:
				{
					CGameObject* ReqPlayer;
					*GameObjectJob->GameObjectJobMessage >> &ReqPlayer;

					CPlayer* Player = (CPlayer*)ReqPlayer;

					int64 FindObjectID;
					*GameObjectJob->GameObjectJobMessage >> FindObjectID;

					int16 FindObjectType;
					*GameObjectJob->GameObjectJobMessage >> FindObjectType;

					CGameObject* FindObject = FindChannelObject(FindObjectID, (en_GameObjectType)FindObjectType);
					if (FindObject != nullptr)
					{
						int64 PreviousChiceObject = 0;

						if (Player->_SelectTarget != nullptr)
						{
							PreviousChiceObject = Player->_SelectTarget->_GameObjectInfo.ObjectId;
						}

						Player->_SelectTarget = FindObject;

						CMessage* ResMousePositionObjectInfo = G_ObjectManager->GameServer->MakePacketResLeftMousePositionObjectInfo(Player->_SessionId,
							PreviousChiceObject, FindObject->_GameObjectInfo.ObjectId,
							FindObject->_Bufs, FindObject->_DeBufs);
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResMousePositionObjectInfo);
						ResMousePositionObjectInfo->Free();
					}
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_CRAFTING_TABLE_SELECT_ITEM:
				{
					CGameObject* ReqPlayer;
					*GameObjectJob->GameObjectJobMessage >> &ReqPlayer;

					int64 FindObjectID;
					*GameObjectJob->GameObjectJobMessage >> FindObjectID;

					int16 FindObjectType;
					*GameObjectJob->GameObjectJobMessage >> FindObjectType;

					int16 LeftMouseItemCategory;
					*GameObjectJob->GameObjectJobMessage >> LeftMouseItemCategory;

					CPlayer* Player = (CPlayer*)ReqPlayer;
					
					CGameObject* CraftingTableGO = FindChannelObject(FindObjectID, (en_GameObjectType)FindObjectType);
					if (CraftingTableGO != nullptr)
					{
						CCraftingTable* CraftingTable = (CCraftingTable*)CraftingTableGO;

						CraftingTable->_SelectCraftingItemType = (en_SmallItemCategory)LeftMouseItemCategory;

						CMessage* ResCraftingTableCompleteItemSelectPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableCompleteItemSelect(CraftingTableGO->_GameObjectInfo.ObjectId,
							CraftingTable->_SelectCraftingItemType,
							CraftingTable->GetMaterialItems());
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCraftingTableCompleteItemSelectPacket);
						ResCraftingTableCompleteItemSelectPacket->Free();
					}
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_CRAFTING_TABLE_NON_SELECT:
				{
					CGameObject* ReqPlayer;
					*GameObjectJob->GameObjectJobMessage >> &ReqPlayer;

					CPlayer* Player = (CPlayer*)ReqPlayer;

					int64 CraftingTableObjectID;
					*GameObjectJob->GameObjectJobMessage >> CraftingTableObjectID;

					int16 CraftingTableObjectType;
					*GameObjectJob->GameObjectJobMessage >> CraftingTableObjectType;

					CGameObject* FindObject = FindChannelObject(CraftingTableObjectID, (en_GameObjectType)CraftingTableObjectType);
					if (FindObject != nullptr)
					{
						CCraftingTable* CraftingTable = (CCraftingTable*)FindObject;

						if (CraftingTable->_SelectedCraftingTable == true)
						{
							CraftingTable->_SelectedCraftingTable = false;

							CraftingTable->_SelectedObject = nullptr;

							CraftingTable->_SelectCraftingItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;							

							CMessage* ResCraftingTableNonSelectMessage = G_ObjectManager->GameServer->MakePacketResCraftingTableNonSelect(CraftingTable->_GameObjectInfo.ObjectId, CraftingTable->_GameObjectInfo.ObjectType);
							G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCraftingTableNonSelectMessage);
							ResCraftingTableNonSelectMessage->Free();
						}
						else
						{
							// 선택한 용광로 UI에서 다른 용광로 UI를 켤 경우 이전 용광로 UI는 닫는 작업이 필요함
							CRASH("선택되어 있지 않은 대상을 선택 풀려고 함");
						}
					}
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_RIGHT_MOUSE_OBJECT_INFO:
				{
					CGameObject* ReqPlayer;
					*GameObjectJob->GameObjectJobMessage >> &ReqPlayer;

					CPlayer* Player = (CPlayer*)ReqPlayer;

					int64 FindObjectID;
					*GameObjectJob->GameObjectJobMessage >> FindObjectID;

					int16 FindObjectType;
					*GameObjectJob->GameObjectJobMessage >> FindObjectType;

					CGameObject* FindObject = FindChannelObject(FindObjectID, (en_GameObjectType)FindObjectType);
					if (FindObject != nullptr)
					{
						switch (FindObject->_GameObjectInfo.ObjectType)
						{
						case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
						case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
							{
								CCraftingTable* CraftingTable = (CCraftingTable*)FindObject;

								// 사용중이 아님
								if (CraftingTable->_SelectedCraftingTable == false)
								{
									// 요청한 플레이어가 전에 선택중이었던 제작대가 있는지 확인				
									vector<CGameObject*> ChannelFindObjects = FindChannelObjects(en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE);
									for (CGameObject* ChannelFindObject : ChannelFindObjects)
									{
										CCraftingTable* FindCraftingTable = (CCraftingTable*)ChannelFindObject;

										// 선택중이었던 용광로가 있으면
										if (FindCraftingTable->_SelectedObject != nullptr)
										{
											FindCraftingTable->_SelectedCraftingTable = false;

											FindCraftingTable->_SelectedObject = nullptr;

											FindCraftingTable->_SelectCraftingItemType = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;

											CMessage* ResCraftingTableNonSelectMessage = G_ObjectManager->GameServer->MakePacketResCraftingTableNonSelect(FindCraftingTable->_GameObjectInfo.ObjectId, FindCraftingTable->_GameObjectInfo.ObjectType);
											G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCraftingTableNonSelectMessage);
											ResCraftingTableNonSelectMessage->Free();
										}
									}

									CraftingTable->_SelectedCraftingTable = true;

									CraftingTable->_SelectedObject = Player;

									// 제작대가 제작중이라면 제작중인 아이템의 정보를 클라에게 보냄
									if (CraftingTable->_GameObjectInfo.ObjectPositionInfo.State == en_CreatureState::CRAFTING)
									{
										for (CItem* CraftingTableItem : CraftingTable->GetCraftingTableRecipe().CraftingTableCompleteItems)
										{
											CMessage* ResCraftingTableSelectPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableCraftRemainTime(
												CraftingTable->_GameObjectInfo.ObjectId,
												CraftingTableItem->_ItemInfo);
											G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCraftingTableSelectPacket);
											ResCraftingTableSelectPacket->Free();
										}
									}

									// 완성된 제작품이 있을 경우 목록을 보내준다.
									if (CraftingTable->GetCompleteItems().size() > 0)
									{
										CMessage* ResCrafintgTableCompleteItemListPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableCompleteItemList(
											CraftingTable->_GameObjectInfo.ObjectId,
											CraftingTable->_GameObjectInfo.ObjectType,
											CraftingTable->GetCompleteItems());
										G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCrafintgTableCompleteItemListPacket);
										ResCrafintgTableCompleteItemListPacket->Free();
									}

									CMessage* ResRightMousePositionObjectInfoPacket = G_ObjectManager->GameServer->MakePacketResRightMousePositionObjectInfo(Player->_GameObjectInfo.ObjectId,
										FindObject->_GameObjectInfo.ObjectId, FindObject->_GameObjectInfo.ObjectType);
									G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResRightMousePositionObjectInfoPacket);
									ResRightMousePositionObjectInfoPacket->Free();
								}
								else
								{
									// 사용중임
									CMessage* CommonErrorPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MEESAGE_CRAFTING_TABLE_OVERLAP_SELECT, FindObject->_GameObjectInfo.ObjectName.c_str());
									G_ObjectManager->GameServer->SendPacket(Player->_SessionId, CommonErrorPacket);
									CommonErrorPacket->Free();
								}
							}
						default:
							break;
						}
					}
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_SEED_FARMING:
				{
					// 심고자 하는 씨앗 위치에 다른 오브젝트가 심어져 있는지 확인
					// 없을 경우 씨앗 생성
					CGameObject* ReqPlayer;
					*GameObjectJob->GameObjectJobMessage >> &ReqPlayer;
					
					CPlayer* Player = (CPlayer*)ReqPlayer;

					// 요청 씨앗 아이템
					int16 SeedItemSmallCategory;
					*GameObjectJob->GameObjectJobMessage >> SeedItemSmallCategory;

					// 가방에 요청한 씨앗 아이템이 있는지 먼저 확인
					CItem* SeedItem = Player->_InventoryManager.FindInventoryItem(0, (en_SmallItemCategory)SeedItemSmallCategory);
					if (SeedItem != nullptr)
					{
						// 심고자 하는 자리에 다른 작물이 잇는지 확인
						CMap* SeedFarmingMap = _Map;
						CGameObject* Plant = SeedFarmingMap->FindPlant(ReqPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
						if (Plant == nullptr)
						{
							CCrop* SeedObject = nullptr;

							switch ((en_SmallItemCategory)SeedItemSmallCategory)
							{
							case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_SEED_POTATO:							
								SeedObject = (CPotato*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_CROP_POTATO);
								break;							
							case en_SmallItemCategory::ITEM_SMALL_CATEGORY_CROP_SEED_CORN:
								SeedObject = (CPotato*)G_ObjectManager->ObjectCreate(en_GameObjectType::OBJECT_CROP_CORN);
								break;
							}
						
							SeedObject->Init((en_SmallItemCategory)SeedItemSmallCategory);

							EnterChannel(SeedObject, &ReqPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition);
						}
						else
						{
							CMessage* SeedFarmingExistError = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSOANL_MESSAGE_SEED_FARMING_EXIST, Plant->_GameObjectInfo.ObjectName.c_str());
							G_ObjectManager->GameServer->SendPacket(Player->_SessionId, SeedFarmingExistError);
							SeedFarmingExistError->Free();
						}

						SeedItem->_ItemInfo.ItemCount -= 1;

						CMessage* ResSeedItemUpdatePacket = G_ObjectManager->GameServer->MakePacketInventoryItemUpdate(Player->_GameObjectInfo.ObjectId, SeedItem->_ItemInfo);
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSeedItemUpdatePacket);
						ResSeedItemUpdatePacket->Free();
					}		
					else
					{
						CRASH("가방에 요청한 씨앗이 없는데 심기 요청");
					}
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PLANT_GROWTH_CHECK:
				{
					CGameObject* ReqPlayer;
					*GameObjectJob->GameObjectJobMessage >> &ReqPlayer;

					int64 PlantObjectID;
					*GameObjectJob->GameObjectJobMessage >> PlantObjectID;

					int16 PlantObjectType;
					*GameObjectJob->GameObjectJobMessage >> PlantObjectType;

					CPlayer* Player = (CPlayer*)ReqPlayer;

					CGameObject* Plant = FindChannelObject(PlantObjectID, (en_GameObjectType)PlantObjectType);
					if (Plant != nullptr)
					{
						CMessage* ResPlantGrowthCheckPacket = G_ObjectManager->GameServer->MakePacketPlantGrowthStep(Plant->_GameObjectInfo.ObjectId, Plant->_GameObjectInfo.ObjectCropStep);
						G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResPlantGrowthCheckPacket);
						ResPlantGrowthCheckPacket->Free();
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
	}		

	for (int16 i = 0; i < PLAYER_MAX; i++)
	{
		if (_ChannelPlayerArray[i]
			&& _ChannelPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE
			&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			_ChannelPlayerArray[i]->Update();
		}
	}

	for (int16 i = 0; i < DUMMY_PLAYER_MAX; i++)
	{
		if (_ChannelDummyPlayerArray[i]
			&& _ChannelDummyPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE
			&& _ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			_ChannelDummyPlayerArray[i]->Update();
		}
	}

	for (int16 i = 0; i < MONSTER_MAX; i++)
	{
		if (_ChannelMonsterArray[i] != nullptr)
		{
			_ChannelMonsterArray[i]->Update();
		}
	}

	for (int16 i = 0; i < CRAFTING_TABLE_MAX; i++)
	{
		if (_ChannelCraftingTableArray[i] != nullptr)
		{
			_ChannelCraftingTableArray[i]->Update();
		}
	}

	for (int16 i = 0; i < CROP_MAX; i++)
	{
		if (_ChannelCropArray[i] != nullptr)
		{
			_ChannelCropArray[i]->Update();
		}
	}

	for (int16 i = 0; i < ITEM_MAX; i++)
	{
		if (_ChannelItemArray[i] != nullptr)
		{
			_ChannelItemArray[i]->Update();
		}
	}

	for (int16 i = 0; i < ENVIRONMENT_MAX; i++)
	{
		if (_ChannelEnvironmentArray[i] != nullptr)
		{
			_ChannelEnvironmentArray[i]->Update();
		}
	}
}

CMap* CChannel::GetMap()
{
	return _Map;
}

void CChannel::SetMap(CMap* Map)
{
	_Map = Map;
}

CGameObject* CChannel::FindChannelObject(int64 ObjectID, en_GameObjectType GameObjectType)
{
	CGameObject* FindObject = nullptr;	
		
	switch (GameObjectType)
	{	
	case en_GameObjectType::OBJECT_PLAYER:
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
		{
			for (int32 i = 0; i < en_Channel::PLAYER_MAX; i++)
			{
				if (_ChannelPlayerArray[i] != nullptr 
					&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectId == ObjectID
					&& _ChannelPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE)
				{
					FindObject = _ChannelPlayerArray[i];
				}				
			}
		}
		break;
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		{
			for (int32 i = 0; i < en_Channel::DUMMY_PLAYER_MAX; i++)
			{
				if (_ChannelDummyPlayerArray[i] != nullptr && _ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectId == ObjectID)
				{
					FindObject = _ChannelDummyPlayerArray[i];
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_MONSTER:
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		{
			for (int32 i = 0; i < en_Channel::MONSTER_MAX; i++)
			{
				if (_ChannelMonsterArray[i] != nullptr && _ChannelMonsterArray[i]->_GameObjectInfo.ObjectId == ObjectID)
				{
					FindObject = _ChannelMonsterArray[i];
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_ENVIRONMENT:
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		{
			for (int32 i = 0; i < en_Channel::ENVIRONMENT_MAX; i++)
			{
				if (_ChannelEnvironmentArray[i] != nullptr && _ChannelEnvironmentArray[i]->_GameObjectInfo.ObjectId == ObjectID)
				{
					FindObject = _ChannelEnvironmentArray[i];
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_ITEM:
	case en_GameObjectType::OBJECT_ITEM_WEAPON:
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
	case en_GameObjectType::OBJECT_ITEM_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_HEALTH_RESTORATION_POTION_SMALL:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_MANA_RESTORATION_POTION_SMALL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIVER_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_GOLD_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_CHAR_COAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_INGOT:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_INGOT:
	case en_GameObjectType::OBJECT_ITEM_CROP_SEED_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
		{
			for (int32 i = 0; i < en_Channel::ENVIRONMENT_MAX; i++)
			{
				if (_ChannelItemArray[i] != nullptr && _ChannelItemArray[i]->_GameObjectInfo.ObjectId == ObjectID)
				{
					FindObject = _ChannelItemArray[i];
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		{
			for (int32 i = 0; i < en_Channel::CRAFTING_TABLE_MAX; i++)
			{
				if (_ChannelCraftingTableArray[i] != nullptr && _ChannelCraftingTableArray[i]->_GameObjectInfo.ObjectId == ObjectID)
				{	
					FindObject = _ChannelCraftingTableArray[i];
				}
			}
		}
		break;
	case en_GameObjectType::OBJECT_CROP:
	case en_GameObjectType::OBJECT_CROP_POTATO:		
	case en_GameObjectType::OBJECT_CROP_CORN:
		{
			for (int32 i = 0; i < en_Channel::CROP_MAX; i++)
			{
				if (_ChannelCropArray[i] != nullptr && _ChannelCropArray[i]->_GameObjectInfo.ObjectId == ObjectID)
				{
					FindObject = _ChannelCropArray[i];
				}
			}
		}
		break;
	}

	return FindObject;
}

vector<CGameObject*> CChannel::FindChannelObjects(en_GameObjectType GameObjectType)
{
	vector<CGameObject*> FindObjects;

	switch (GameObjectType)
	{
	case en_GameObjectType::OBJECT_PLAYER:
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:
		for (int32 i = 0; i < en_Channel::PLAYER_MAX; i++)
		{
			if (_ChannelPlayerArray[i] != nullptr
				&& _ChannelPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE)
			{
				FindObjects.push_back(_ChannelPlayerArray[i]);
			}
		}
		break;
	case en_GameObjectType::OBJECT_MONSTER:
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		for (int32 i = 0; i < en_Channel::MONSTER_MAX; i++)
		{
			if (_ChannelMonsterArray[i] != nullptr)
			{
				FindObjects.push_back(_ChannelMonsterArray[i]);
			}
		}
		break;
	case en_GameObjectType::OBJECT_ENVIRONMENT:
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		for (int32 i = 0; i < en_Channel::ENVIRONMENT_MAX; i++)
		{
			if (_ChannelEnvironmentArray[i] != nullptr)
			{
				FindObjects.push_back(_ChannelEnvironmentArray[i]);
			}
		}
		break;	
	case en_GameObjectType::OBJECT_ITEM:
	case en_GameObjectType::OBJECT_ITEM_WEAPON:
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
	case en_GameObjectType::OBJECT_ITEM_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_HEALTH_RESTORATION_POTION_SMALL:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_MANA_RESTORATION_POTION_SMALL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIVER_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_GOLD_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_CHAR_COAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_INGOT:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_INGOT:
	case en_GameObjectType::OBJECT_ITEM_CROP_SEED_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
		for (int32 i = 0; i < en_Channel::ENVIRONMENT_MAX; i++)
		{
			if (_ChannelItemArray[i] != nullptr)
			{
				FindObjects.push_back(_ChannelItemArray[i]);
			}
		}
		break;
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		for (int32 i = 0; i < en_Channel::DUMMY_PLAYER_MAX; i++)
		{
			if (_ChannelDummyPlayerArray[i] != nullptr)
			{
				FindObjects.push_back(_ChannelDummyPlayerArray[i]);				
			}
		}
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		for (int32 i = 0; i < en_Channel::CRAFTING_TABLE_MAX; i++)
		{
			if (_ChannelCraftingTableArray[i] != nullptr)
			{
				FindObjects.push_back(_ChannelCraftingTableArray[i]);
			}
		}
		break;
	case en_GameObjectType::OBJECT_CROP:
	case en_GameObjectType::OBJECT_CROP_POTATO:
	case en_GameObjectType::OBJECT_CROP_CORN:
		for (int32 i = 0; i < en_Channel::CROP_MAX; i++)
		{
			if (_ChannelCropArray[i] != nullptr)
			{
				FindObjects.push_back(_ChannelCropArray[i]);
			}
		}
		break;
	}

	return FindObjects;
}

vector<CGameObject*> CChannel::FindChannelObjects(vector<st_FieldOfViewInfo>& FindObjectIDs)
{
	vector<CGameObject*> FindObjects;

	for (st_FieldOfViewInfo FieldOfViewInfo : FindObjectIDs)
	{
		switch (FieldOfViewInfo.ObjectType)
		{			
		case en_GameObjectType::OBJECT_PLAYER:
		case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
		case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
		case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
		case en_GameObjectType::OBJECT_THIEF_PLAYER:
		case en_GameObjectType::OBJECT_ARCHER_PLAYER:
			{
				for (int32 i = 0; i < en_Channel::PLAYER_MAX; i++)
				{
					if (_ChannelPlayerArray[i] != nullptr
						&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID
						&& _ChannelPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE)
					{
						FindObjects.push_back(_ChannelPlayerArray[i]);
					}
				}
			}
			break;
		case en_GameObjectType::OBJECT_PLAYER_DUMMY:
			{
				for (int32 i = 0; i < en_Channel::DUMMY_PLAYER_MAX; i++)
				{
					if (_ChannelDummyPlayerArray[i] != nullptr && _ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
					{
						FindObjects.push_back(_ChannelDummyPlayerArray[i]);
					}
				}
			}
			break;
		case en_GameObjectType::OBJECT_MONSTER:
		case en_GameObjectType::OBJECT_SLIME:
		case en_GameObjectType::OBJECT_BEAR:
			{
				for (int32 i = 0; i < en_Channel::MONSTER_MAX; i++)
				{
					if (_ChannelMonsterArray[i] != nullptr && _ChannelMonsterArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
					{
						FindObjects.push_back(_ChannelMonsterArray[i]);
					}
				}
			}
			break;
		case en_GameObjectType::OBJECT_ENVIRONMENT:
		case en_GameObjectType::OBJECT_STONE:
		case en_GameObjectType::OBJECT_TREE:
			{
				for (int32 i = 0; i < en_Channel::ENVIRONMENT_MAX; i++)
				{
					if (_ChannelEnvironmentArray[i] != nullptr && _ChannelEnvironmentArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
					{
						FindObjects.push_back(_ChannelEnvironmentArray[i]);
					}
				}
			}
			break;
		case en_GameObjectType::OBJECT_ITEM:
		case en_GameObjectType::OBJECT_ITEM_WEAPON:
		case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
		case en_GameObjectType::OBJECT_ITEM_ARMOR:
		case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_ARMOR:
		case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
		case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
		case en_GameObjectType::OBJECT_ITEM_CONSUMABLE:
		case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
		case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_HEALTH_RESTORATION_POTION_SMALL:
		case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_MANA_RESTORATION_POTION_SMALL:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIVER_COIN:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_GOLD_COIN:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_CHAR_COAL:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_NUGGET:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_INGOT:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_NUGGET:
		case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_INGOT:
		case en_GameObjectType::OBJECT_ITEM_CROP_SEED_POTATO:
		case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
			{
				for (int32 i = 0; i < en_Channel::ENVIRONMENT_MAX; i++)
				{
					if (_ChannelItemArray[i] != nullptr && _ChannelItemArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
					{
						FindObjects.push_back(_ChannelItemArray[i]);
					}
				}
			}
			break;
		case en_GameObjectType::OBJECT_ARCHITECTURE:
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE:
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
		case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
			{
				for (int32 i = 0; i < en_Channel::CRAFTING_TABLE_MAX; i++)
				{
					if (_ChannelCraftingTableArray[i] != nullptr && _ChannelCraftingTableArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
					{
						FindObjects.push_back(_ChannelCraftingTableArray[i]);
					}
				}
			}
			break;
		case en_GameObjectType::OBJECT_CROP:
		case en_GameObjectType::OBJECT_CROP_POTATO:
		case en_GameObjectType::OBJECT_CROP_CORN:
			{
				for (int32 i = 0; i < en_Channel::CROP_MAX; i++)
				{
					if (_ChannelCropArray[i] != nullptr && _ChannelCropArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
					{
						FindObjects.push_back(_ChannelCropArray[i]);
					}
				}
			}			
			break;
		}
	}	

	return FindObjects;
}

vector<CGameObject*> CChannel::FindAttackChannelObjects(vector<st_FieldOfViewInfo>& FindObjectIDs, CGameObject* Object, int16 Distance)
{
	vector<CGameObject*> FindObjects;

	for (st_FieldOfViewInfo FieldOfViewInfo : FindObjectIDs)
	{
		switch (FieldOfViewInfo.ObjectType)
		{
		case en_GameObjectType::OBJECT_PLAYER:
		case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
		case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
		case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
		case en_GameObjectType::OBJECT_THIEF_PLAYER:
		case en_GameObjectType::OBJECT_ARCHER_PLAYER:
			{
				for (int32 i = 0; i < en_Channel::PLAYER_MAX; i++)
				{
					if (_ChannelPlayerArray[i] != nullptr
						&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID
						&& _ChannelPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE
						&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::READY_DEAD
						&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
					{	
						if (st_Vector2::CheckFieldOfView(_ChannelPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position, Object->GetPositionInfo().MoveDir, 80))
						{
							// 시야각 안에 오브젝트가 존재
							float TargetDistance = st_Vector2::Distance(_ChannelPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
							if (TargetDistance <= Distance)
							{
								FindObjects.push_back(_ChannelPlayerArray[i]);
							}
						}				
					}
				}
			}
			break;
		case en_GameObjectType::OBJECT_PLAYER_DUMMY:
			{
				for (int32 i = 0; i < en_Channel::DUMMY_PLAYER_MAX; i++)
				{
					if (_ChannelDummyPlayerArray[i] != nullptr && _ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID)
					{
						if (st_Vector2::CheckFieldOfView(_ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position, Object->GetPositionInfo().MoveDir, 80))
						{
							// 시야각 안에 오브젝트가 존재
							float TargetDistance = st_Vector2::Distance(_ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
							if (TargetDistance <= Distance)
							{
								FindObjects.push_back(_ChannelDummyPlayerArray[i]);
							}
						}					
					}
				}
			}
			break;
		case en_GameObjectType::OBJECT_MONSTER:
		case en_GameObjectType::OBJECT_SLIME:
		case en_GameObjectType::OBJECT_BEAR:
			{
				for (int32 i = 0; i < en_Channel::MONSTER_MAX; i++)
				{
					if (_ChannelMonsterArray[i] != nullptr 
						&& _ChannelMonsterArray[i]->_GameObjectInfo.ObjectId == FieldOfViewInfo.ObjectID
						&& _ChannelMonsterArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::READY_DEAD
						&& _ChannelMonsterArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
					{
						if (st_Vector2::CheckFieldOfView(_ChannelMonsterArray[i]->_GameObjectInfo.ObjectPositionInfo.Position,
							Object->_GameObjectInfo.ObjectPositionInfo.Position, 
							Object->GetPositionInfo().MoveDir, 80))
						{
							// 시야각 안에 오브젝트가 존재
							float TargetDistance = st_Vector2::Distance(_ChannelMonsterArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
							if (TargetDistance <= Distance)
							{
								FindObjects.push_back(_ChannelMonsterArray[i]);
							}
						}
					}
				}
			}
			break;			
		}
	}

	return FindObjects;
}

bool CChannel::ChannelColliderCheck(CGameObject* Object)
{	
	// 플레이어들과 충돌하는지 검사
	bool IsPlayerCollision = true;
	for (int32 i = 0; i < en_Channel::PLAYER_MAX; i++)
	{
		if (_ChannelPlayerArray[i] != nullptr
			&& _ChannelPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE)
		{
			if (Object->_GameObjectInfo.ObjectId != _ChannelPlayerArray[i]->_GameObjectInfo.ObjectId 
				&& _ChannelPlayerArray[i]->GetRectCollision()->GetActive() == true
				&& CRectCollision::IsCollision(Object->GetRectCollision(), _ChannelPlayerArray[i]->GetRectCollision()) == true)
			{
				// 충돌
				IsPlayerCollision = false;
				//G_Logger->WriteStdOut(en_Color::RED, L"%s와 %s 충돌\n", Object->_GameObjectInfo.ObjectName.c_str(), _ChannelPlayerArray[i]->_GameObjectInfo.ObjectName.c_str());
				break;
			}
		}
	}

	bool IsDummyPlayerCollision = true;
	for (int32 i = 0; i < en_Channel::DUMMY_PLAYER_MAX; i++)
	{
		if (_ChannelDummyPlayerArray[i] != nullptr
			&& _ChannelDummyPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE)
		{
			if (Object->_GameObjectInfo.ObjectId != _ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectId 
				&& _ChannelDummyPlayerArray[i]->GetRectCollision()->GetActive() == true
				&& CRectCollision::IsCollision(Object->GetRectCollision(), _ChannelDummyPlayerArray[i]->GetRectCollision()) == true)
			{
				IsDummyPlayerCollision = false;
				break;
			}
		}
	}

	bool IsMonsterCollision = true;
	for (int32 i = 0; i < en_Channel::MONSTER_MAX; i++)
	{
		if (_ChannelMonsterArray[i] != nullptr)
		{
			if (Object->_GameObjectInfo.ObjectId != _ChannelMonsterArray[i]->_GameObjectInfo.ObjectId 
				&& _ChannelMonsterArray[i]->GetRectCollision()->GetActive() == true
				&& CRectCollision::IsCollision(Object->GetRectCollision(), _ChannelMonsterArray[i]->GetRectCollision()) == true)
			{
				IsMonsterCollision = false;
				break;
			}
		}
	}

	bool IsCraftingTableCollision = true;
	for (int32 i = 0; i < en_Channel::CRAFTING_TABLE_MAX; i++)
	{
		if (_ChannelCraftingTableArray[i] != nullptr)
		{
			if (Object->_GameObjectInfo.ObjectId != _ChannelCraftingTableArray[i]->_GameObjectInfo.ObjectId 
				&& _ChannelCraftingTableArray[i]->GetRectCollision()->GetActive() == true
				&& CRectCollision::IsCollision(Object->GetRectCollision(), _ChannelCraftingTableArray[i]->GetRectCollision()) == true)
			{
				IsCraftingTableCollision = false;
				break;
			}
		}
	}

	bool IsEnvironmentCollision = true;
	for (int32 i = 0; i < en_Channel::ENVIRONMENT_MAX; i++)
	{
		if (_ChannelEnvironmentArray[i] != nullptr)
		{
			if (Object->_GameObjectInfo.ObjectId != _ChannelEnvironmentArray[i]->_GameObjectInfo.ObjectId 
				&& _ChannelEnvironmentArray[i]->GetRectCollision()->GetActive() == true
				&& CRectCollision::IsCollision(Object->GetRectCollision(), _ChannelEnvironmentArray[i]->GetRectCollision()) == true)
			{
				IsEnvironmentCollision = false;
				break;
			}
		}
	}

	bool IsCollision = (IsPlayerCollision) && (IsDummyPlayerCollision) && (IsMonsterCollision) && (IsCraftingTableCollision) && (IsEnvironmentCollision);
	
	return IsCollision;
}

bool CChannel::EnterChannel(CGameObject* EnterChannelGameObject, st_Vector2Int* ObjectSpawnPosition)
{
	bool IsEnterChannel = false;
	// 채널 입장
	if (EnterChannelGameObject == nullptr)
	{
		CRASH("GameObject가 nullptr");
		return false;
	}

	random_device RD;
	mt19937 Gen(RD());

	st_Vector2Int SpawnPosition;		
	EnterChannelGameObject->SetChannel(this);

	if (EnterChannelGameObject->_GameObjectInfo.ObjectType == en_GameObjectType::OBJECT_PLAYER_DUMMY)
	{
		// 더미를 대상으로 랜덤 좌표 받아서 채널에 입장
		while (true)
		{
			uniform_int_distribution<int> RandomXPosition(0, 89);
			uniform_int_distribution<int> RandomYPosition(0, 73);

			SpawnPosition._X = RandomXPosition(Gen);
			SpawnPosition._Y = RandomYPosition(Gen);

			EnterChannelGameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X = SpawnPosition._X;
			EnterChannelGameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y = SpawnPosition._Y;

			EnterChannelGameObject->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelGameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
			EnterChannelGameObject->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelGameObject->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;

			EnterChannelGameObject->GetRectCollision()->CollisionUpdate();

			if (_Map->CollisionCango(EnterChannelGameObject, SpawnPosition) == true)
			{
				break;
			}
		}
	}
	else
	{
		SpawnPosition = *ObjectSpawnPosition;
	}
	
	// 입장한 오브젝트의 타입에 따라
	switch ((en_GameObjectType)EnterChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:	
		{
			// 플레이어로 형변환
			CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;
			EnterChannelPlayer->_SpawnPosition = SpawnPosition;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;

			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;

			EnterChannelPlayer->GetRectCollision()->CollisionUpdate();

			// 플레이어 저장
			_ChannelPlayerArrayIndexs.Pop(&EnterChannelPlayer->_ChannelArrayIndex);
			_ChannelPlayerArray[EnterChannelPlayer->_ChannelArrayIndex] = EnterChannelPlayer;		
											
			//_Players.insert(pair<int64, CPlayer*>(EnterChannelPlayer->_GameObjectInfo.ObjectId, EnterChannelPlayer));			

			// 맵에 적용
			IsEnterChannel = _Map->ApplyMove(EnterChannelPlayer, SpawnPosition);

			// 섹터에 저장
			CSector* EnterSector = _Map->GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelPlayer);
		}
		break;
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		{
			// 플레이어로 형변환
			CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;
			EnterChannelPlayer->_SpawnPosition = SpawnPosition;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;

			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;			

			// 플레이어 저장
			_ChannelDummyPlayerArrayIndexs.Pop(&EnterChannelPlayer->_ChannelArrayIndex);
			_ChannelDummyPlayerArray[EnterChannelPlayer->_ChannelArrayIndex] = EnterChannelPlayer;
			
			// 맵에 적용
			IsEnterChannel = _Map->ApplyMove(EnterChannelPlayer, SpawnPosition);

			// 섹터에 저장
			CSector* EnterSector = _Map->GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelPlayer);
		}	
		break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		{		
			// 몬스터로 형변환	
			CMonster* EnterChannelMonster = (CMonster*)EnterChannelGameObject;		
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;

			EnterChannelMonster->GetRectCollision()->CollisionUpdate();

			EnterChannelMonster->Start();		
					
			// 몬스터 저장
			_ChannelMonsterArrayIndexs.Pop(&EnterChannelMonster->_ChannelArrayIndex);
			_ChannelMonsterArray[EnterChannelMonster->_ChannelArrayIndex] = EnterChannelMonster;
			//G_Logger->WriteStdOut(en_Color::RED, L"ObjectID %d EnterChannelIndex %d\n", EnterChannelMonster->_GameObjectInfo.ObjectId, EnterChannelMonster->_ChannelArrayIndex);					

			// 맵에 적용
			IsEnterChannel = _Map->ApplyMove(EnterChannelMonster, SpawnPosition);

			// 섹터 얻어서 해당 섹터에도 저장
			CSector* EnterSector = _Map->GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelMonster);
		}
		break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_CHAR_COAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_INGOT:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_INGOT:
	case en_GameObjectType::OBJECT_ITEM_CROP_SEED_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
		{
			// 아이템으로 형변환
			CItem* EnterChannelItem = (CItem*)EnterChannelGameObject;
			EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;					

			// 맵 정보에 보관			
			IsEnterChannel = _Map->ApplyMove(EnterChannelItem, SpawnPosition, false, false);

			// 중복되지 않는 아이템의 경우에만 채널에 해당 아이템을 채널과 섹터에 저장
			if (IsEnterChannel == true)
			{
				// 아이템 저장
				_ChannelItemArrayIndexs.Pop(&EnterChannelItem->_ChannelArrayIndex);
				_ChannelItemArray[EnterChannelItem->_ChannelArrayIndex] = EnterChannelItem;
							
				// 섹터 얻어서 해당 섹터에도 저장
				CSector* EnterSector = _Map->GetSector(SpawnPosition);	
				EnterSector->Insert(EnterChannelItem);
			}
		}
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		{
			CEnvironment* EnterChannelEnvironment = (CEnvironment*)EnterChannelGameObject;
			EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;
			EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
			EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelEnvironment->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;		

			EnterChannelEnvironment->GetRectCollision()->CollisionUpdate();

			EnterChannelEnvironment->Start();

			// 환경 오브젝트 저장
			_ChannelEnvironmentArrayIndexs.Pop(&EnterChannelEnvironment->_ChannelArrayIndex);
			_ChannelEnvironmentArray[EnterChannelEnvironment->_ChannelArrayIndex] = EnterChannelEnvironment;			

			IsEnterChannel = _Map->ApplyMove(EnterChannelEnvironment, SpawnPosition);

			// 섹터 얻어서 해당 섹터에도 저장
			CSector* EnterSector = _Map->GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelEnvironment);
		}
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		{
			CCraftingTable* EnterChannelCraftingTable = (CCraftingTable*)EnterChannelGameObject;
			EnterChannelCraftingTable->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;
			EnterChannelCraftingTable->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelCraftingTable->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
			EnterChannelCraftingTable->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelCraftingTable->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;

			EnterChannelCraftingTable->GetRectCollision()->CollisionUpdate();

			EnterChannelCraftingTable->Start();

			_ChannelCraftingTableArrayIndexs.Pop(&EnterChannelCraftingTable->_ChannelArrayIndex);
			_ChannelCraftingTableArray[EnterChannelCraftingTable->_ChannelArrayIndex] = EnterChannelCraftingTable;			

			IsEnterChannel = _Map->ApplyMove(EnterChannelCraftingTable, SpawnPosition);
			
			CSector* Entersector = _Map->GetSector(SpawnPosition);
			Entersector->Insert(EnterChannelCraftingTable);
		}
		break;
	case en_GameObjectType::OBJECT_CROP_POTATO:
	case en_GameObjectType::OBJECT_CROP_CORN:
		{
			CCrop* Crop = (CCrop*)EnterChannelGameObject;
			Crop->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;
			Crop->_GameObjectInfo.ObjectPositionInfo.Position._X = Crop->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
			Crop->_GameObjectInfo.ObjectPositionInfo.Position._Y = Crop->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;			

			_ChannelCropArrayIndexs.Pop(&Crop->_ChannelArrayIndex);
			_ChannelCropArray[Crop->_ChannelArrayIndex] = Crop;			

			IsEnterChannel = _Map->ApplyMove(Crop, SpawnPosition, true, false);

			CSector* EnterSector = _Map->GetSector(SpawnPosition);
			EnterSector->Insert(Crop);
		}
		break;
	}

	return IsEnterChannel;
}

void CChannel::LeaveChannel(CGameObject* LeaveChannelGameObject)
{
	// 채널 퇴장
	// 컨테이너에서 제거한 후 맵에서도 제거
	switch ((en_GameObjectType)LeaveChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:	
		_ChannelPlayerArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);
		break;
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		_ChannelDummyPlayerArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);
		break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:		
		_ChannelMonsterArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);
		break;
	case en_GameObjectType::OBJECT_ITEM_WEAPON_WOOD_SWORD:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_ARMOR:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_HELMET:
	case en_GameObjectType::OBJECT_ITEM_ARMOR_LEATHER_BOOT:
	case en_GameObjectType::OBJECT_ITEM_CONSUMABLE_SKILL_BOOK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_SLIME_GEL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_BRONZE_COIN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_LEATHER:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_LOG:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_STONE:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_WOOD_FLANK:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_YARN:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_CHAR_COAL:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_COPPER_INGOT:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_NUGGET:
	case en_GameObjectType::OBJECT_ITEM_MATERIAL_IRON_INGOT:
	case en_GameObjectType::OBJECT_ITEM_CROP_SEED_POTATO:
	case en_GameObjectType::OBJECT_ITEM_CROP_FRUIT_POTATO:
		G_ObjectManager->ItemReturn((CItem*)LeaveChannelGameObject);

		_ChannelItemArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);
		break;
	case en_GameObjectType::OBJECT_STONE:
	case en_GameObjectType::OBJECT_TREE:
		_ChannelEnvironmentArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		_ChannelCraftingTableArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);
		break;
	case en_GameObjectType::OBJECT_CROP_POTATO:
	case en_GameObjectType::OBJECT_CROP_CORN:
		_ChannelCropArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);
		break;
	}	
}