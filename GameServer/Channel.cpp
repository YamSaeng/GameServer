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
#include "DataManager.h"
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
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_INVITE:
				{
					CGameObject* ReqPartyPlayer;
					*GameObjectJob->GameObjectJobMessage >> &ReqPartyPlayer;

					int64 InvitePlayerID;
					*GameObjectJob->GameObjectJobMessage >> InvitePlayerID;

					CPlayer* PartyPlayer = dynamic_cast<CPlayer*>(ReqPartyPlayer);
					CPlayer* InvitePlayer = dynamic_cast<CPlayer*>(FindChannelObject(InvitePlayerID, en_GameObjectType::OBJECT_PLAYER));

					// �׷쿡 ����ִ��� ����
					int8 PartySize = (int8)PartyPlayer->_PartyManager.GetPartyPlayerArray().size();
					if (PartySize == 0) // 0 ���� ���
					{						
						// �׷��ʴ� ����� �׷����� �ƴ��� Ȯ��
						if (InvitePlayer->_PartyManager._IsParty == false)
						{
							CMessage* ResPartyInvitePacket = G_ObjectManager->GameServer->MakePacketResPartyInvite(PartyPlayer->_GameObjectInfo.ObjectId, PartyPlayer->_GameObjectInfo.ObjectName);
							G_ObjectManager->GameServer->SendPacket(InvitePlayer->_SessionId, ResPartyInvitePacket);
							ResPartyInvitePacket->Free();							
						}		
						else
						{
							// �׷����̶�� �׷����̶�� �޼��� ����
							CMessage* ResExistPartyPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_EXIST_PARTY_PLAYER,
								InvitePlayer->_GameObjectInfo.ObjectName.c_str());
							G_ObjectManager->GameServer->SendPacket(PartyPlayer->_SessionId, ResExistPartyPacket);
							ResExistPartyPacket->Free();
						}
					}
					else
					{
						// �׷��ʴ� ��û ����� ��Ƽ������, �׷��ʴ� ����� �׷� ������ Ȯ��
						if (PartyPlayer->_PartyManager._IsPartyLeader == true && InvitePlayer->_PartyManager._IsParty == false)
						{
							vector<CPlayer*> PartyPlayers = PartyPlayer->_PartyManager.GetPartyPlayerArray();
							for (CPlayer* PartyPlayer : PartyPlayers)
							{
								PartyPlayer->_PartyManager.PartyInvite(InvitePlayer);
							}							
						}
					}									
				}
				break;			
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_INVITE_ACCEPT:
				{
					int64 PartyReqPlayerID;
					*GameObjectJob->GameObjectJobMessage >> PartyReqPlayerID;

					int64 PartyAcceptPlayerID;
					*GameObjectJob->GameObjectJobMessage >> PartyAcceptPlayerID;					

					// �׷� ��û�� ����� ä�ο� �ִ��� Ȯ��
					CPlayer* PartyReqPlayer = dynamic_cast<CPlayer*>(FindChannelObject(PartyReqPlayerID, en_GameObjectType::OBJECT_PLAYER));
					if (PartyReqPlayer != nullptr)
					{
						// �׷� ��û ������ ����� ä�ο� �ִ��� Ȯ��
						CPlayer* PartyAcceptPlayer = dynamic_cast<CPlayer*>(FindChannelObject(PartyAcceptPlayerID, en_GameObjectType::OBJECT_PLAYER));
						if (PartyAcceptPlayer != nullptr)
						{
							// �׷��� �� á���� Ȯ��
							int8 PartySize = (int8)PartyReqPlayer->_PartyManager.GetPartyPlayerArray().size();
							if (PartySize == CPartyManager::en_PartyManager::PARTY_MAX)
							{
								// �׷쿡 �� �ڸ��� ���� ���
								CMessage* ResExistPartyPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_PARTY_MAX);
								G_ObjectManager->GameServer->SendPacket(PartyAcceptPlayer->_SessionId, ResExistPartyPacket);
								ResExistPartyPacket->Free();
							}
							else
							{
								PartyReqPlayer->_PartyManager.PartyManagerInit(PartyReqPlayer);
								PartyAcceptPlayer->_PartyManager.PartyManagerInit(PartyAcceptPlayer);

								// �׷��ʴ� ��û ����� �׷������� ���ϰ� �׷��ʴ� ����� �׷쿡 �ʴ�
								PartyReqPlayer->_PartyManager.PartyLeaderInvite(PartyAcceptPlayer);
								PartyAcceptPlayer->_PartyManager.PartyInvite(PartyReqPlayer);

								vector<CPlayer*> PartyPlayers = PartyReqPlayer->_PartyManager.GetPartyPlayerArray();
								CGameServerMessage* ResPartyAcceptPacket = G_ObjectManager->GameServer->MakePacketResPartyAccept(PartyPlayers);
								for (CPlayer* PartyPlayer : PartyPlayers)
								{
									G_ObjectManager->GameServer->SendPacket(PartyPlayer->_SessionId, ResPartyAcceptPacket);
								}

								ResPartyAcceptPacket->Free();
							}							
						}
						else
						{

						}
					}
					else
					{

					}
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_INVITE_REJECT:
				{
					int64 PartyRejectPlayerID;
					*GameObjectJob->GameObjectJobMessage >> PartyRejectPlayerID;

					int64 ReqPartyInvitePlayerID;
					*GameObjectJob->GameObjectJobMessage >> ReqPartyInvitePlayerID;

					CPlayer* PartyRejectPlayer = dynamic_cast<CPlayer*>(FindChannelObject(PartyRejectPlayerID, en_GameObjectType::OBJECT_PLAYER));
					if (PartyRejectPlayer != nullptr)
					{
						CPlayer* ReqPartyInvitePlayer = dynamic_cast<CPlayer*>(FindChannelObject(ReqPartyInvitePlayerID, en_GameObjectType::OBJECT_PLAYER));
						if (ReqPartyInvitePlayer != nullptr)
						{
							// �׷����̶�� �׷����̶�� �޼��� ����
							CMessage* ResExistPartyPacket = G_ObjectManager->GameServer->MakePacketCommonError(en_PersonalMessageType::PERSONAL_MESSAGE_PARTY_INVITE_REJECT,
								PartyRejectPlayer->_GameObjectInfo.ObjectName.c_str());
							G_ObjectManager->GameServer->SendPacket(ReqPartyInvitePlayer->_SessionId, ResExistPartyPacket);
							ResExistPartyPacket->Free();
						}
						else
						{

						}
					}					
					else
					{

					}
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_QUIT:
				{
					int64 PartyQuitPlayerID;
					*GameObjectJob->GameObjectJobMessage >> PartyQuitPlayerID;

					// Ż�� ��û ĳ���͸� ã��
					CPlayer* PartyQuitPlayer = dynamic_cast<CPlayer*>(FindChannelObject(PartyQuitPlayerID, en_GameObjectType::OBJECT_PLAYER));
					if (PartyQuitPlayer != nullptr)
					{
						// Ż�� ��û ĳ���Ͱ� ��Ƽ������ Ȯ��
						if (PartyQuitPlayer->_PartyManager._IsParty == true)	
						{
							// Ż�� ��û ĳ���͸� ������ �÷��̾ ã��
							for (CPlayer* PartyPlayer : PartyQuitPlayer->_PartyManager.GetPartyPlayerArray())
							{
								// Ż�� ��û ĳ���͸� ������ ������ �÷��̾� �׷쿡�� Ż�� ��û ĳ���͸� �׷� Ż�� ��Ŵ
								if (PartyPlayer->_GameObjectInfo.ObjectId != PartyQuitPlayerID)
								{
									PartyPlayer->_PartyManager.PartyQuited(PartyQuitPlayerID);

									CMessage* ResPartyPlaryerOneQuitPacket = G_ObjectManager->GameServer->MakePacketResPartyQuit(false, PartyQuitPlayerID);
									G_ObjectManager->GameServer->SendPacket(PartyPlayer->_SessionId, ResPartyPlaryerOneQuitPacket);
									ResPartyPlaryerOneQuitPacket->Free();
								}
							}							

							// Ż�� ��û ĳ���� ��Ƽ���� ����
							PartyQuitPlayer->_PartyManager.PartyQuit();

							// Ż�� ��û �÷��̾�� �׷� Ż�� ��Ŷ ����
							CMessage* ResPartyQuitPacket = G_ObjectManager->GameServer->MakePacketResPartyQuit(true, PartyQuitPlayerID);
							G_ObjectManager->GameServer->SendPacket(PartyQuitPlayer->_SessionId, ResPartyQuitPacket);
							ResPartyQuitPacket->Free();
						}
						else
						{
							CRASH("�׷� ���� �ƴѵ� �׷� Ż�� ��û");
						}
					}
					else
					{
						CRASH("�׷� Ż�� ��û �÷��̾ ã�� �� ����");
					}
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_BANISH:
				{
					CGameObject* PartyBanishReqGameObject;
					*GameObjectJob->GameObjectJobMessage >> &PartyBanishReqGameObject;

					int64 PartyBanishPlayerID;
					*GameObjectJob->GameObjectJobMessage >> PartyBanishPlayerID;

					// �׷� �߹� ��û ĳ���͸� ������
					CPlayer* PartyBanishReqPlayer = dynamic_cast<CPlayer*>(PartyBanishReqGameObject);

					// �׷� �߹� �� ����� ������
					CPlayer* PartyBanishPlayer = dynamic_cast<CPlayer*>(FindChannelObject(PartyBanishPlayerID, en_GameObjectType::OBJECT_PLAYER));
					if (PartyBanishPlayer != nullptr)
					{
						// �׷� �߹� ��û ĳ���Ͱ� �׷������� Ȯ��
						if (PartyBanishReqPlayer->_PartyManager._IsPartyLeader == true)
						{
							// �׷� �߹� ����� ������ ������ �÷��̾�鿡�� �׷� �߹� ����� �߹��Ű��, �޼��� ����
							for (CPlayer* PartyPlayer : PartyBanishPlayer->_PartyManager.GetPartyPlayerArray())
							{
								if (PartyPlayer->_GameObjectInfo.ObjectId != PartyBanishPlayerID)
								{
									PartyPlayer->_PartyManager.PartyQuited(PartyBanishPlayer->_GameObjectInfo.ObjectId);

									CMessage* ResPartyBanishPacekt = G_ObjectManager->GameServer->MakePacketResPartyBanish(PartyBanishPlayerID);
									G_ObjectManager->GameServer->SendPacket(PartyPlayer->_SessionId, ResPartyBanishPacekt);
									ResPartyBanishPacekt->Free();
								}								
							}

							PartyBanishPlayer->_PartyManager.PartyAllQuit();

							CMessage* ResPartyQuitPacket = G_ObjectManager->GameServer->MakePacketResPartyQuit(true, PartyBanishPlayerID);
							G_ObjectManager->GameServer->SendPacket(PartyBanishPlayer->_SessionId, ResPartyQuitPacket);
							ResPartyQuitPacket->Free();
						}
						else
						{
							CRASH("�׷����� �ƴѵ� �׷� �߹� ��û");
						}						
					}
					else
					{
						CRASH("��Ƽ �߹� ĳ���� ã���� ����");
					}
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_PARTY_LEADER_MANDATE:
				{
					int64 ReqPartyLeaderMandatePlayerID;
					*GameObjectJob->GameObjectJobMessage >> ReqPartyLeaderMandatePlayerID;

					int64 PartyLeaderMandatePlayerID;
					*GameObjectJob->GameObjectJobMessage >> PartyLeaderMandatePlayerID;

					// �׷��� ���� ��û ĳ���Ͱ� ä�ο� �ִ��� ã�´�.
					CPlayer* ReqPartyLeader = dynamic_cast<CPlayer*>(FindChannelObject(ReqPartyLeaderMandatePlayerID, en_GameObjectType::OBJECT_PLAYER));
					if (ReqPartyLeader != nullptr)
					{
						// ���ο� �׷��� ĳ���Ͱ� ä�ο� �ִ��� ã�´�.
						CPlayer* NewPartyLeader = dynamic_cast<CPlayer*>(FindChannelObject(PartyLeaderMandatePlayerID, en_GameObjectType::OBJECT_PLAYER));
						if (NewPartyLeader != nullptr)
						{
							// ��û ĳ���Ͱ� �׷������� Ȯ���ϰ�
							// ���ο� �׷��� ĳ���Ͱ� ��û ĳ���� �׷쿡 �ִ��� Ȯ���Ѵ�.
							if (ReqPartyLeader->_PartyManager._IsPartyLeader == true && ReqPartyLeader->_PartyManager.IsPartyMember(NewPartyLeader->_GameObjectInfo.ObjectId) == true)
							{								
								ReqPartyLeader->_PartyManager._IsPartyLeader = false;
								NewPartyLeader->_PartyManager._IsPartyLeader = true;

								CMessage* ResPartyLeaderMandatePacket = G_ObjectManager->GameServer->MakePacketResPartyLeaderMandate(ReqPartyLeader->_GameObjectInfo.ObjectId, NewPartyLeader->_GameObjectInfo.ObjectId);
								G_ObjectManager->GameServer->SendPacket(ReqPartyLeader->_SessionId, ResPartyLeaderMandatePacket);
								G_ObjectManager->GameServer->SendPacket(NewPartyLeader->_SessionId, ResPartyLeaderMandatePacket);
								ResPartyLeaderMandatePacket->Free();
							}
						}
					}					
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_OBJECT_SPAWN:
				{
					
				}
				break;
			case en_GameObjectJobType::GAMEOBJECT_JOB_TYPE_CHANNEL_OBJECT_DESPAWN:
				{
					CGameObject* DeSpawnObject;
					*GameObjectJob->GameObjectJobMessage >> &DeSpawnObject;					

					// �� �����ؼ� ���� �þ߹��� �÷��̾� ����
					vector<st_FieldOfViewInfo> CurrentFieldOfViewObjectIDs = _Map->GetFieldOfViewPlayers(DeSpawnObject, 1, false);

					switch (DeSpawnObject->_GameObjectInfo.ObjectType)
					{
					case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
					case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
					case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
					case en_GameObjectType::OBJECT_THIEF_PLAYER:
					case en_GameObjectType::OBJECT_ARCHER_PLAYER:
					case en_GameObjectType::OBJECT_SLIME:						
						// �浹 ���� �ڽ� ��Ȱ��ȭ
						DeSpawnObject->GetRectCollision()->SetActive(false);
						break;							
					}					

					// ���� �þ߹��� �÷��̾�鿡�� �ش� ������Ʈ�� ��ȯ���� �϶�� �˸�
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

					// �浹 ���� Ȱ��ȭ
					EnterPlayer->GetRectCollision()->SetActive(true);

					EnterChannel(EnterPlayer, &EnterPlayer->_SpawnPosition);					

					// ������ �� �����϶�� �˷���
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

							// �浹 ���� �ڽ� Ȱ��ȭ
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
								G_ObjectManager->ObjectReturn(Item);
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

					// �� �����ؼ� ���� �þ߹��� �÷��̾� ����
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
							// ������ �뱤�� UI���� �ٸ� �뱤�� UI�� �� ��� ���� �뱤�� UI�� �ݴ� �۾��� �ʿ���
							CRASH("���õǾ� ���� ���� ����� ���� Ǯ���� ��");
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

								// ������� �ƴ�
								if (CraftingTable->_SelectedCraftingTable == false)
								{
									// ��û�� �÷��̾ ���� �������̾��� ���۴밡 �ִ��� Ȯ��				
									vector<CGameObject*> ChannelFindObjects = FindChannelObjects(en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE);
									for (CGameObject* ChannelFindObject : ChannelFindObjects)
									{
										CCraftingTable* FindCraftingTable = (CCraftingTable*)ChannelFindObject;

										// �������̾��� �뱤�ΰ� ������
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

									// ���۴밡 �������̶�� �������� �������� ������ Ŭ�󿡰� ����
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

									// �ϼ��� ����ǰ�� ���� ��� ����� �����ش�.
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
									// �������
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
					// �ɰ��� �ϴ� ���� ��ġ�� �ٸ� ������Ʈ�� �ɾ��� �ִ��� Ȯ��
					// ���� ��� ���� ����
					CGameObject* ReqPlayer;
					*GameObjectJob->GameObjectJobMessage >> &ReqPlayer;
					
					CPlayer* Player = (CPlayer*)ReqPlayer;

					// ��û ���� ������
					int16 SeedItemSmallCategory;
					*GameObjectJob->GameObjectJobMessage >> SeedItemSmallCategory;

					// ���濡 ��û�� ���� �������� �ִ��� ���� Ȯ��
					CItem* SeedItem = Player->_InventoryManager.FindInventoryItem(0, (en_SmallItemCategory)SeedItemSmallCategory);
					if (SeedItem != nullptr)
					{
						// �ɰ��� �ϴ� �ڸ��� �ٸ� �۹��� �մ��� Ȯ��
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

							CMessage* ResSeedFarmingPacket = G_ObjectManager->GameServer->MakePacketSeedFarming(SeedItem->_ItemInfo, SeedObject->_GameObjectInfo.ObjectId);
							G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResSeedFarmingPacket);
							ResSeedFarmingPacket->Free();
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
						CRASH("���濡 ��û�� ������ ���µ� �ɱ� ��û");
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

					CCrop* Plant = (CCrop*)FindChannelObject(PlantObjectID, (en_GameObjectType)PlantObjectType);
					if (Plant != nullptr)
					{
						CMessage* ResPlantGrowthCheckPacket = G_ObjectManager->GameServer->MakePacketPlantGrowthStep(Plant->_GameObjectInfo.ObjectId,
							Plant->_GameObjectInfo.ObjectCropStep,
							Plant->_CropGrowthRatio);
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
							// �þ߰� �ȿ� ������Ʈ�� ����
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
							// �þ߰� �ȿ� ������Ʈ�� ����
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
							// �þ߰� �ȿ� ������Ʈ�� ����
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
		
vector<CGameObject*> CChannel::FindRangeAttackChannelObjects(CGameObject* Object, int16 Distance)
{
	vector<CGameObject*> FindObjects;

	for (int32 i = 0; i < en_Channel::PLAYER_MAX; i++)
	{
		if (_ChannelPlayerArray[i] != nullptr			
			&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectId != Object->_GameObjectInfo.ObjectId
			&& _ChannelPlayerArray[i]->_NetworkState == en_ObjectNetworkState::LIVE
			&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::READY_DEAD
			&& _ChannelPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			float TargetDistance = st_Vector2::Distance(_ChannelPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
			if (TargetDistance <= Distance)
			{
				FindObjects.push_back(_ChannelPlayerArray[i]);
			}
		}
	}

	for (int32 i = 0; i < en_Channel::DUMMY_PLAYER_MAX; i++)
	{
		if (_ChannelDummyPlayerArray[i] != nullptr)
		{
			// �Ÿ� �ȿ� ������Ʈ�� ����
			float TargetDistance = st_Vector2::Distance(_ChannelDummyPlayerArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
			if (TargetDistance <= Distance)
			{
				FindObjects.push_back(_ChannelDummyPlayerArray[i]);
			}
		}
	}

	for (int32 i = 0; i < en_Channel::MONSTER_MAX; i++)
	{
		if (_ChannelMonsterArray[i] != nullptr			
			&& _ChannelMonsterArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::READY_DEAD
			&& _ChannelMonsterArray[i]->_GameObjectInfo.ObjectPositionInfo.State != en_CreatureState::DEAD)
		{
			// �Ÿ� �ȿ� ������Ʈ�� ����
			float TargetDistance = st_Vector2::Distance(_ChannelMonsterArray[i]->_GameObjectInfo.ObjectPositionInfo.Position, Object->_GameObjectInfo.ObjectPositionInfo.Position);
			if (TargetDistance <= Distance)
			{
				FindObjects.push_back(_ChannelMonsterArray[i]);
			}
		}
	}

	return FindObjects;
}

bool CChannel::ChannelColliderCheck(CGameObject* Object)
{	
	// �÷��̾��� �浹�ϴ��� �˻�
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
				// �浹
				IsPlayerCollision = false;
				//G_Logger->WriteStdOut(en_Color::RED, L"%s�� %s �浹\n", Object->_GameObjectInfo.ObjectName.c_str(), _ChannelPlayerArray[i]->_GameObjectInfo.ObjectName.c_str());
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
	// ä�� ����
	if (EnterChannelGameObject == nullptr)
	{
		CRASH("GameObject�� nullptr");
		return false;
	}

	random_device RD;
	mt19937 Gen(RD());

	st_Vector2Int SpawnPosition;		
	EnterChannelGameObject->SetChannel(this);

	if (EnterChannelGameObject->_GameObjectInfo.ObjectType == en_GameObjectType::OBJECT_PLAYER_DUMMY)
	{
		// ���̸� ������� ���� ��ǥ �޾Ƽ� ä�ο� ����
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
	
	// ������ ������Ʈ�� Ÿ�Կ� ����
	switch ((en_GameObjectType)EnterChannelGameObject->_GameObjectInfo.ObjectType)
	{
	case en_GameObjectType::OBJECT_WARRIOR_PLAYER:
	case en_GameObjectType::OBJECT_SHAMAN_PLAYER:
	case en_GameObjectType::OBJECT_TAIOIST_PLAYER:
	case en_GameObjectType::OBJECT_THIEF_PLAYER:
	case en_GameObjectType::OBJECT_ARCHER_PLAYER:	
		{
			// �÷��̾�� ����ȯ
			CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;
			EnterChannelPlayer->_SpawnPosition = SpawnPosition;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;

			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;

			EnterChannelPlayer->GetRectCollision()->CollisionUpdate();

			// �÷��̾� ����
			_ChannelPlayerArrayIndexs.Pop(&EnterChannelPlayer->_ChannelArrayIndex);
			_ChannelPlayerArray[EnterChannelPlayer->_ChannelArrayIndex] = EnterChannelPlayer;		
											
			//_Players.insert(pair<int64, CPlayer*>(EnterChannelPlayer->_GameObjectInfo.ObjectId, EnterChannelPlayer));			

			// �ʿ� ����
			IsEnterChannel = _Map->ApplyMove(EnterChannelPlayer, SpawnPosition);

			// ���Ϳ� ����
			CSector* EnterSector = _Map->GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelPlayer);
		}
		break;
	case en_GameObjectType::OBJECT_PLAYER_DUMMY:
		{
			// �÷��̾�� ����ȯ
			CPlayer* EnterChannelPlayer = (CPlayer*)EnterChannelGameObject;
			EnterChannelPlayer->_SpawnPosition = SpawnPosition;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;

			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
			EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelPlayer->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;			

			// �÷��̾� ����
			_ChannelDummyPlayerArrayIndexs.Pop(&EnterChannelPlayer->_ChannelArrayIndex);
			_ChannelDummyPlayerArray[EnterChannelPlayer->_ChannelArrayIndex] = EnterChannelPlayer;
			
			// �ʿ� ����
			IsEnterChannel = _Map->ApplyMove(EnterChannelPlayer, SpawnPosition);

			// ���Ϳ� ����
			CSector* EnterSector = _Map->GetSector(SpawnPosition);
			EnterSector->Insert(EnterChannelPlayer);
		}	
		break;
	case en_GameObjectType::OBJECT_SLIME:
	case en_GameObjectType::OBJECT_BEAR:
		{		
			// ���ͷ� ����ȯ	
			CMonster* EnterChannelMonster = (CMonster*)EnterChannelGameObject;		
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.Position._X = EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._X + 0.5f;
			EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.Position._Y = EnterChannelMonster->_GameObjectInfo.ObjectPositionInfo.CollisionPosition._Y + 0.5f;

			EnterChannelMonster->GetRectCollision()->CollisionUpdate();

			EnterChannelMonster->Start();		
					
			// ���� ����
			_ChannelMonsterArrayIndexs.Pop(&EnterChannelMonster->_ChannelArrayIndex);
			_ChannelMonsterArray[EnterChannelMonster->_ChannelArrayIndex] = EnterChannelMonster;
			//G_Logger->WriteStdOut(en_Color::RED, L"ObjectID %d EnterChannelIndex %d\n", EnterChannelMonster->_GameObjectInfo.ObjectId, EnterChannelMonster->_ChannelArrayIndex);					

			// �ʿ� ����
			IsEnterChannel = _Map->ApplyMove(EnterChannelMonster, SpawnPosition);

			// ���� �� �ش� ���Ϳ��� ����
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
			// ���������� ����ȯ
			CItem* EnterChannelItem = (CItem*)EnterChannelGameObject;
			EnterChannelItem->_GameObjectInfo.ObjectPositionInfo.CollisionPosition = SpawnPosition;					

			// �� ������ ����			
			IsEnterChannel = _Map->ApplyMove(EnterChannelItem, SpawnPosition, false, false);

			// �ߺ����� �ʴ� �������� ��쿡�� ä�ο� �ش� �������� ä�ΰ� ���Ϳ� ����
			if (IsEnterChannel == true)
			{
				// ������ ����
				_ChannelItemArrayIndexs.Pop(&EnterChannelItem->_ChannelArrayIndex);
				_ChannelItemArray[EnterChannelItem->_ChannelArrayIndex] = EnterChannelItem;
							
				// ���� �� �ش� ���Ϳ��� ����
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

			// ȯ�� ������Ʈ ����
			_ChannelEnvironmentArrayIndexs.Pop(&EnterChannelEnvironment->_ChannelArrayIndex);
			_ChannelEnvironmentArray[EnterChannelEnvironment->_ChannelArrayIndex] = EnterChannelEnvironment;			

			IsEnterChannel = _Map->ApplyMove(EnterChannelEnvironment, SpawnPosition);

			// ���� �� �ش� ���Ϳ��� ����
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
	// ä�� ����
	// �����̳ʿ��� ������ �� �ʿ����� ����
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
		G_ObjectManager->ObjectReturn(LeaveChannelGameObject);

		_ChannelEnvironmentArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);
		break;
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_FURNACE:
	case en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL:
		_ChannelCraftingTableArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);
		break;
	case en_GameObjectType::OBJECT_CROP_POTATO:
	case en_GameObjectType::OBJECT_CROP_CORN:
		G_ObjectManager->ObjectReturn(LeaveChannelGameObject);

		_ChannelCropArrayIndexs.Push(LeaveChannelGameObject->_ChannelArrayIndex);
		break;
	}	
}

void CChannel::ExperienceCalculate(CPlayer* TargetPlayer, en_GameObjectType TargetMonsterObjectType, int32 ExperiencePoint)
{
	TargetPlayer->_Experience.CurrentExperience += ExperiencePoint;
	TargetPlayer->_Experience.CurrentExpRatio = ((float)TargetPlayer->_Experience.CurrentExperience) / TargetPlayer->_Experience.RequireExperience;	

	if (TargetPlayer->_Experience.CurrentExpRatio >= 1.0f)
	{
		// ���� ����
		TargetPlayer->_GameObjectInfo.ObjectStatInfo.Level += 1;
		TargetPlayer->_GameObjectInfo.ObjectSkillPoint += 1;

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

	CGameServerMessage* ResMonsterGetExpMessage = G_ObjectManager->GameServer->MakePacketExperience(TargetMonsterObjectType,
		ExperiencePoint,
		TargetPlayer->_Experience.CurrentExperience,
		TargetPlayer->_Experience.RequireExperience,
		TargetPlayer->_Experience.TotalExperience);
	G_ObjectManager->GameServer->SendPacket(TargetPlayer->_SessionId, ResMonsterGetExpMessage);
	ResMonsterGetExpMessage->Free();	
}
