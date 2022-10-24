#pragma once

enum en_GAME_SERVER_PACKET_TYPE
{
	////////////////////////////////////////////////////////
	//
	//	Client & GameServer Protocol
	//
	////////////////////////////////////////////////////////

	//------------------------------------------------------
	// Game Server
	//------------------------------------------------------
	en_PACKET_CS_GAME_SERVER = 0,

	//------------------------------------------------------------
	// ��Ʈ��Ʈ	
	//
	//	{
	//		WORD		Type
	//	}
	//
	//
	// Ŭ���̾�Ʈ�� �̸� 30�ʸ��� ������.
	// ������ 40�� �̻󵿾� �޽��� ������ ���� Ŭ���̾�Ʈ�� ������ ������� ��.
	//------------------------------------------------------------	
	en_PACKET_CS_GAME_REQ_HEARTBEAT,


	//-----------------------------------------------------------
	// ���Ӽ��� Ŭ���̾�Ʈ ���� �Ϸ� ����
	// {
	// 	   short Type
	// }
	//-----------------------------------------------------------
	en_PACKET_S2C_GAME_CLIENT_CONNECTED,

	//------------------------------------------------------------
	// ���Ӽ��� �α��� ��û
	//	{
	//		WORD	Type
	//
	//		int 	AccountNo
	//		WCHAR	ClientID[20];		
	//		int     TokenKey
	//	}
	//
	//------------------------------------------------------------
	en_PACKET_C2S_GAME_REQ_LOGIN,

	//------------------------------------------------------------
	// ���Ӽ��� �α��� ����
	//	{
	//		WORD	Type
	//
	//		bool	Status				// 0:����	1:����	
	//	}
	//------------------------------------------------------------
	en_PACKET_S2C_GAME_RES_LOGIN,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��û
	// int32 CharacterNameLen
	// WCHAR CharacterName
	//------------------------------------------------------------
	en_PACKET_C2S_GAME_CREATE_CHARACTER,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��û ����
	// int32 PlayerDBId
	// bool IsSuccess
	// wstring PlayerName
	//------------------------------------------------------------
	en_PACKET_S2C_GAME_CREATE_CHARACTER,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ����
	// int64 AccoountId
	// int8 EnterGameCharacterNameLen
	// wstring EnterGameCharacterName
	//------------------------------------------------------------
	en_PACKET_C2S_GAME_ENTER,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��û ����
	// int64 AccountId
	// int32 PlaterDBId
	// wstring EnterCharaceterName
	// st_GameObjectInfo ObjectInfo
	//------------------------------------------------------------
	en_PACKET_S2C_GAME_ENTER,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��û
	// int64 AccountId
	// int64 PlayerDBId	
	//------------------------------------------------------------
	en_PACKET_C2S_CHARACTER_INFO,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��û ����	
	//------------------------------------------------------------
	en_PACKET_S2C_CHARACTER_INFO,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� �����̱� ��û
	// int64 AccountId
	// int32 PlayerDBId
	// st_PositionInfo PositionInfo
	// int8 Dir
	//------------------------------------------------------------
	en_PACKET_C2S_MOVE,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� �����̱� ��û ����	
	// int32 PlayerDBId
	// bool CanGo
	// st_PositionInfo PositionInfo
	//------------------------------------------------------------
	en_PACKET_S2C_MOVE,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��û
	// int64 AccountId
	// int64 ObjectId
	// st_PositionInfo PositionInfo
	//------------------------------------------------------------
	en_PACKET_C2S_MOVE_STOP,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��û ����
	// int64 AccountId
	// int64 ObjectId
	// st_PositionInfo PositionInfo
	//------------------------------------------------------------
	en_PACKET_S2C_MOVE_STOP,

	//------------------------------------------------------------
	// ���Ӽ��� ���� ������ 
	// int64 ObjectId
	// en_GameObjectType ObjectType
	// st_PositionInfo PositionInfo
	//------------------------------------------------------------
	en_PACKET_S2C_MONSTER_MOVE,

	//------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� 
	// int64 ObjectId
	// st_PositionInfo PositionInfo
	//------------------------------------------------------------
	en_PACKET_S2C_MONSTER_PATROL,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ������ ���� 
	// st_GameObjectInfo ItemObjectInfo	
	//------------------------------------------------------------
	en_PACKET_S2C_ITEM_MOVE_START,

	//------------------------------------------------------------
	// ���Ӽ��� �Ϲ� ������ ���
	// int64 ObjectID
	// int64 TargetObjectID
	// en_SkillType SkillType
	// int32 Damage
	// bool IsCritical
	//------------------------------------------------------------
	en_PACKET_S2C_COMMON_DAMAGE,

	//------------------------------------------------------------
	// ���Ӽ��� ä�� ������ ���	
	// int64 TargetObjectID		
	//------------------------------------------------------------
	en_PACKET_S2C_GATHERING_DAMAGE,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��û
	// int64 AccountId
	// int32 PlayerDBId
	// int8 Dir
	// en_AttackRange RangeAttack;
	// int8 RangeDistance;
	//------------------------------------------------------------	
	en_PACKET_C2S_ATTACK,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��û ����
	// int64 AccountId
	// int32 PlayerDBId
	// int8 Dir
	//------------------------------------------------------------	
	en_PACKET_S2C_ATTACK,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��û
	// int64 AccountId
	// int64 PlayerDBId
	// en_MoveDir Dir
	// en_SkillType SpellSkillType
	//------------------------------------------------------------
	en_PACKET_C2S_SPELL,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��û ����	
	// int64 PlayerDBId
	// bool SpellStart
	// en_SkillType SpellSkillType
	// float SpellTime
	//------------------------------------------------------------
	en_PACKET_S2C_SPELL,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��� ��û
	// int64 AccountId
	// int64 PlayerId	
	//------------------------------------------------------------
	en_PACKET_C2S_MAGIC_CANCEL,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ���� ��� ��û ����
	// int64 AccountId
	// int64 PlayerId	
	//------------------------------------------------------------
	en_PACKET_S2C_MAGIC_CANCEL,

	//------------------------------------------------------------
	// ���Ӽ��� ä�� ��û
	// int64 AccountId
	// int64 PlayerID
	// int64 ObjectID
	// st_GameObjectType ObjectType
	//------------------------------------------------------------
	en_PACKET_C2S_GATHERING,

	//------------------------------------------------------------
	// ���Ӽ��� ä�� ��û ����
	// int64 ObjectID
	//------------------------------------------------------------
	en_PACKET_S2C_GATHERING,

	//------------------------------------------------------------
	// ���Ӽ��� ä�� ��� ��û
	// int64 AccountID
	// int64 ObjectID
	//------------------------------------------------------------
	en_PACKET_C2S_GATHERING_CANCEL,

	//------------------------------------------------------------
	// ���Ӽ��� ä�� ��� ��û ����
	// int64 AccountID
	// int64 ObjectID
	//------------------------------------------------------------
	en_PACKET_S2C_GATHERING_CANCEL,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� �ִϸ��̼� ���
	// int64 PlayerID
	// en_MoveDir Dir
	// string AnimationName
	//------------------------------------------------------------
	en_PACKET_S2C_ANIMATION_PLAY,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ����
	// int64 AccountId
	// int32 PlayerDBId
	// wstring SpawnObjectName
	// st_GameObjectInfo GameObjectInfo
	//------------------------------------------------------------	
	en_PACKET_S2C_SPAWN,

	//------------------------------------------------------------
	// ���Ӽ��� ĳ���� ����
	// int64 AccountId
	// int32 PlayerDBId
	//------------------------------------------------------------
	en_PACKET_S2C_DESPAWN,

	//------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ����
	// int64 AccountId
	// int32 PlayerDBId
	// st_StatInfo ChangeStatInfo
	//------------------------------------------------------------
	en_PACKET_S2C_OBJECT_STAT_CHANGE,

	//------------------------------------------------------------
	// ���Ӽ��� ���� ���� ���콺 ���� ��û	
	// int64 AccountId
	// int32 PlayerDBId
	// int64 ObjectID
	// int16 ObjectType
	//------------------------------------------------------------
	en_PACKET_C2S_LEFT_MOUSE_OBJECT_INFO,

	//------------------------------------------------------------
	// ���Ӽ��� ���� ���� ���콺 ���� ��û ����
	// int64 AccountId
	// int64 PlayerDBId
	// st_GameObjectInfo ObjectInfo
	//------------------------------------------------------------
	en_PACKET_S2C_LEFT_MOUSE_OBJECT_INFO,

	//------------------------------------------------------------
	// ���Ӽ��� ���� UI ���콺 ���� ��û 
	// int64 AccountID
	// int64 PlayerID
	// en_GameObjectType ObjectType
	//------------------------------------------------------------
	en_PACKET_C2S_LEFT_MOUSE_UI_OBJECT_INFO,

	//------------------------------------------------------------
	// ���Ӽ��� ���� UI ���콺 ���� ��û ����
	//------------------------------------------------------------
	en_PACKET_S2C_LEFT_MOUSE_UI_OBJECT_INFO,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���콺 ���� ��û	
	// int64 AccountId
	// int64 PlayerDBId
	// int64 ObjectID
	// int16 ObjectType
	//------------------------------------------------------------
	en_PACKET_C2S_RIGHT_MOUSE_OBJECT_INFO,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���콺 ���� ��û ����
	// int64 ReqPlayerID
	// int64 FindObjectID
	// inf16 FindObjectType
	//------------------------------------------------------------
	en_PACKET_S2C_RIGHT_MOUSE_OBJECT_INFO,

	//------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� ���� �ð�
	// int64 CraftingTableObjectID	
	// st_ItemInfo CraftingItemInfo
	//------------------------------------------------------------
	en_PACKET_S2C_CRAFTING_TABLE_CRAFT_REMAIN_TIME,

	//------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� Ǯ�� ��û
	// int64 AccountID
	// int64 PlayerID
	// int64 CraftingTableObjectID
	// int16 CraftingTableObjectType
	//------------------------------------------------------------
	en_PACKET_C2S_CRAFTING_TABLE_NON_SELECT,

	//------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� Ǯ�� ��û ����
	// int64 CraftingTableObjectID
	// int16 CraftingTableObjectType
	//------------------------------------------------------------
	en_PACKET_S2C_CRAFTING_TABLE_NON_SELECT,

	//------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ���� ��û ����
	// int64 ObjectId
	// en_MoveDir Direction
	// en_GameObjectType ObjectType
	// en_CreatureState ObjectState	
	//------------------------------------------------------------
	en_PACKET_S2C_OBJECT_STATE_CHANGE,

	//------------------------------------------------------------
	// ���Ӽ��� ���� ������Ʈ ���� ���� ��û ����
	// int64 ObjectId
	// en_MoveDir Direction
	// en_GameObjectType ObjectType
	// en_CreatureState ObjectState
	// en_MonsterState MonsterState
	//------------------------------------------------------------
	en_PACKET_S2C_MONSTER_OBJECT_STATE_CHANGE,

	//------------------------------------------------------------
	// ���Ӽ��� �����̻� ���� 
	// int64 ObjectId
	// bool SetStatusAbnormal
	// int8 StatusAbnormal
	//------------------------------------------------------------
	en_PACKET_S2C_STATUS_ABNORMAL,

	//------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ���� ����
	// int64 ObjectId	
	//------------------------------------------------------------
	en_PACKET_S2C_DIE,

	//------------------------------------------------------------
	// ���Ӽ��� ä�� �޼��� ��û
	// int32 ObjectId
	// string Message
	//------------------------------------------------------------
	en_PACKET_C2S_MESSAGE,

	//------------------------------------------------------------
	// ���Ӽ��� ä�� �޼��� ��û
	// int32 ObjectId
	// string Message
	//------------------------------------------------------------
	en_PACKET_S2C_MESSAGE,

	//------------------------------------------------------------
	// ���Ӽ��� �ݱ� ��û
	// int64 AccountId
	// int64 PlayerId
	// st_PositionInfo LootingPositionInfo	
	//------------------------------------------------------------
	en_PACKET_C2S_LOOTING,

	//------------------------------------------------------------
	// ���Ӽ��� �ݱ� ��û ����
	// int64 TargetObjectId
	// st_ItemInfo ItemInfo
	//------------------------------------------------------------
	en_PACKET_S2C_LOOTING,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ������ ��û
	// int64 AccountID
	// int64 PlayerID
	// en_SmallItemType DropItemType
	// int32 DropItemCount
	//------------------------------------------------------------
	en_PACKET_C2S_ITEM_DROP,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ������ ��û ����
	// st_GameObjectInfo DropGameObjectInfo
	//------------------------------------------------------------
	en_PACKET_S2C_ITEM_DROP,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���۴뿡 �ֱ� ��û
	// int64 AccountID
	// int64 PlayerDBID
	// int64 CraftingTableObjectID
	// int16 CraftingTableGameObjectType
	// int16 InputItemSmallCategory 
	//------------------------------------------------------------
	en_PACKET_C2S_CRAFTING_TABLE_ITEM_ADD,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���۴뿡 �ֱ� ��û ����
	// int64 CraftingTableObjectID
	// int16 MaterialItemsSize
	// map MaterialItems
	//------------------------------------------------------------
	en_PACKET_S2C_CRAFTING_TABLE_ITEM_ADD,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���۴뿡�� ��� ������ ���� ��û
	// int64 AccountID
	// int64 PlayerID
	// int64 OwnerCraftingTableObjectID
	// en_GameObjectType OwnerCraftingTableObjectType
	// en_SmallCategory MaterialItemType
	//------------------------------------------------------------
	en_PACKET_C2S_CRAFTING_TABLE_MATERIAL_ITEM_SUBTRACT,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���۴뿡�� �ϼ� ������ ���� ��û
	// int64 AccountID
	// int64 PlayerID
	// int64 OwnerCraftingTableObjectID
	// en_GameObjectType OwnerCraftingTableObjectType
	// en_SmallCategory CompleteItemType
	//------------------------------------------------------------
	en_PACKET_C2S_CRAFTING_TABLE_COMPLETE_ITEM_SUBTRACT,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���۴뿡�� ���� ��û ����
	// int64 PlayerID
	// int64 CraftingTableObjectID
	// st_ItemInfo MaterialItemInfo
	//------------------------------------------------------------
	en_PACKET_S2C_CRAFTING_TABLE_ITEM_SUBTRACT,

	//------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� ������ ���� ����
	// int64 CraftingTableObjectID
	// en_SmallItemCategory SelectCompleteItemType
	// map MaterialItems
	//------------------------------------------------------------
	en_PACKET_S2C_CRAFTING_TABLE_COMPLETE_ITEM_SELECT,

	//------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� ����
	// int64 AccountID
	// int64 PlayerID
	// int64 CraftingTableObjectID
	// en_GameObject CraftingTableObjectType
	// en_SmallItemCateogry CraftingCompleteItem
	// int16 CraftingCount
	//------------------------------------------------------------
	en_PACKET_C2S_CRAFTING_TABLE_CRAFTING_START,

	//------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� ���� ����
	//------------------------------------------------------------
	en_PACKET_S2C_CRAFTING_TABLE_CRAFTING_START,

	//------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� ���� ���� ��û
	// int64 AccountID
	// int64 PlayerID
	// int64 CraftingTableObjectID
	//------------------------------------------------------------
	en_PACKET_C2S_CRAFTING_TABLE_CRAFTING_STOP,

	//------------------------------------------------------------
	// ���Ӽ��� ���۴� ���� ���� ���� ��û ����	
	// int64 CraftingTableObjectID
	// st_ItemInfo CraftingItemInfo
	//------------------------------------------------------------
	en_PACKET_S2C_CRAFTING_TABLE_CRAFTING_STOP,

	//------------------------------------------------------------
	// ���Ӽ��� ���۴� ��� ��� 
	// int64 CraftingTableObjectID
	// map MaterialItems
	//------------------------------------------------------------
	en_PACKET_S2C_CRAFTING_TABLE_MATERIAL_ITEM_LIST,

	//------------------------------------------------------------
	// ���Ӽ��� ���۴� ������ ���
	// int64 CraftingtableObjectID
	// map CompleteItems
	//------------------------------------------------------------
	en_PACKET_S2C_CRAFTING_TABLE_COMPLETE_ITEM_LIST,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��û
	// int64 AccountId
	// int64 ObjectId
	// int16 SelectItemTileGridPositionX
	// int16 SelectItemTileGridPositionY
	//------------------------------------------------------------
	en_PACKET_C2S_ITEM_SELECT,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��û ����
	// int64 AccountId
	// int64 ObjectId
	// CItem* SelectItem	
	//------------------------------------------------------------
	en_PACKET_S2C_ITEM_SELECT,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ȸ�� ��û
	// int64 AccountId
	// int64 ObjectId	
	// en_SmallItemCategory RotateItemSmallCategory
	//------------------------------------------------------------
	en_PACKET_C2S_ITEM_ROTATE,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ȸ�� ��û ����
	// int16 AccountID
	// int64 ObjectID	
	//------------------------------------------------------------
	en_PACKET_S2C_ITEM_ROTATE,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��û
	// int64 AccountId
	// int64 ObjectId	
	// int16 PlaceTileGridPositionX
	// int16 PlaceTileGridPositionY
	//------------------------------------------------------------
	en_PACKET_C2S_ITEM_PLACE,
	
	//------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��û ����
	// int64 AccountId
	// int64 ObjectId
	// st_ItemInfo OverlapItemInfo
	//------------------------------------------------------------
	en_PACKET_S2C_ITEM_PLACE,		

	//------------------------------------------------------------
	// ���Ӽ��� ������Ʈ ��ġ ���� ����
	// int64 TargetObjectId	
	// st_Position SyncPosition	
	//------------------------------------------------------------
	en_PACKET_S2C_SYNC_OBJECT_POSITION,

	//------------------------------------------------------------
	// ���Ӽ��� ��ų Ư�� ���� ��û
	// int64 AccountID
	// int64 PlayerID
	// int8 SkillCharacteristicType
	//------------------------------------------------------------
	en_PACKET_C2S_SELECT_SKILL_CHARACTERISTIC,

	//------------------------------------------------------------
	// ���Ӽ��� ��ų Ư�� ���� ��û ����
	//------------------------------------------------------------
	en_PACKET_S2C_SELECT_SKILL_CHARACTERISTIC,

	//------------------------------------------------------------
	// ���Ӽ��� ��ų ���� ��û
	// int64 TargetObjectId
	// st_SkillInfo SkillInfo
	//------------------------------------------------------------
	en_PACKET_C2S_SKILL_TO_SKILLBOX,

	//------------------------------------------------------------
	// ���Ӽ��� ��ų ���� ��û ����
	// int64 TargetObjectId
	// st_SkillInfo SkillInfo
	//------------------------------------------------------------
	en_PACKET_S2C_SKILL_TO_SKILLBOX,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���� 
	// int8 QuickSlotBarSize
	// int8 QuickSlotBarSlotSize
	// st_QuickSlotBarSlotInfo[] QuickSlotBarSlotInfos
	//------------------------------------------------------------
	en_PACKET_S2C_QUICKSLOT_CREATE,

	//------------------------------------------------------------
	// ���Ӽ��� ��ų ���� ��û ����
	// int64 AccountId
	// int64 PlayerId 
	// st_SkillInfo SkillInfo
	//------------------------------------------------------------
	en_PACKET_C2S_QUICKSLOT_SAVE,

	//-----------------------------------------------------------
	// ���Ӽ��� ��ų ���� ��û ����
	// int64 AccountId
	// int64 PlayerId 
	// st_SkillInfo SkillInfo
	//------------------------------------------------------------
	en_PACKET_S2C_QUICKSLOT_SAVE,

	//-----------------------------------------------------------
	// ���Ӽ��� ��Ÿ�� ��ŸƮ
	// int64 PlayerId
	// int8 QuickSlotBarIndex;
	// int8 QuickSlotBarSlotIndex;
	// float SkillCoolTime;
	// float SkillCoolTimeSpeed;
	//-----------------------------------------------------------
	en_PACKET_S2C_COOLTIME_START,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��û
	// int64 AccountId
	// int64 ObjectId
	// int8 SwapQuickSlotBarIndexA
	// int8 SwapQuickSlotBarSlotIndexA
	// int8 SwapQuickSlotBarIndexB
	// int8 SwapQuickSlotBarSlotIndexB
	//------------------------------------------------------------
	en_PACKET_C2S_QUICKSLOT_SWAP,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���� ��û ����
	// int64 ObjectId
	// st_QuickSlotInfo SwapQuickSlotAItem
	// st_QuickSlotInfo SwapQuickSlotBItem
	//------------------------------------------------------------
	en_PACKET_S2C_QUICKSLOT_SWAP,

	//------------------------------------------------------------
	// ���Ӽ��� ������ �ʱ�ȭ ��û
	// int64 ObjectId
	// int8 QuickSlotBarIndexA
	// int8 QuickSlotBarSlotIndexA	
	//------------------------------------------------------------
	en_PACKET_C2S_QUICKSLOT_EMPTY,

	//------------------------------------------------------------
	// ���Ӽ��� ������ �ʱ�ȭ ��û ����
	// int64 ObjectId
	// int8 QuickSlotBarIndexA
	// int8 QuickSlotBarSlotIndexA	
	//------------------------------------------------------------
	en_PACKET_S2C_QUICKSLOT_EMPTY,

	//------------------------------------------------------------
	// ���Ӽ��� ����Ʈ ��� 
	// int64 ObjectId
	// st_SkillInfo EffectSkillInfo
	//------------------------------------------------------------
	en_PACKET_S2C_EFFECT,

	//------------------------------------------------------------
	// ���Ӽ��� ������ ���
	// int64 ObjectId
	// st_CraftingItemCategory CraftingItemCategory;
	//------------------------------------------------------------	
	en_PACKET_S2C_CRAFTING_LIST,

	//------------------------------------------------------------
	// ���Ӽ��� ���� ��û
	// int64 AccountId
	// int64 PlayerId
	// en_ItemType CompleteItemType
	// int8 MaterialCount
	// st_ItemInfo[] Materials
	//------------------------------------------------------------
	en_PACKET_C2S_CRAFTING_CONFIRM,

	//------------------------------------------------------------
	// ���Ӽ��� �κ��丮 ������ ������Ʈ
	// int64 AccountId
	// int64 PlayerId
	// st_ItemInfo CompleteItem		
	//------------------------------------------------------------
	en_PACKET_S2C_INVENTORY_ITEM_UPDATE,

	//------------------------------------------------------------
	// ���Ӽ��� �κ��丮 ������ ��� ��û
	// int64 AccountId
	// int64 PlayerId
	// st_ItemInfo UseItemInfo
	//------------------------------------------------------------
	en_PACKET_C2S_INVENTORY_ITEM_USE,

	//------------------------------------------------------------
	// ���Ӽ��� �κ��丮 ������ ��� ��û ����
	// int64 PlayerId
	// st_ItemInfo UseItemInfo
	//------------------------------------------------------------
	en_PACKET_S2C_INVENTORY_ITEM_USE,

	//------------------------------------------------------------
	// ���Ӽ��� ������� ����
	// int64 PlayerId	
	// st_ItemInfo Equipment
	//------------------------------------------------------------
	en_PACKET_S2C_ON_EQUIPMENT,

	//------------------------------------------------------------
	// ���Ӽ��� ��� ���� ��û
	// int64 AccountID	
	// int64 PlayerID
	// st_ItemInfo OffEquipmentItemInfo
	//------------------------------------------------------------
	en_PACKET_C2S_OFF_EQUIPMENT,

	//------------------------------------------------------------
	// ���Ӽ��� ��� ���� ��û ����
	// int64 AccountID
	// int64 PlayerID
	// en_EquipmentPart EquipmentPart
	//------------------------------------------------------------
	en_PACKET_S2C_OFF_EQUIPMENT,

	//------------------------------------------------------------
	// ���Ӽ��� ����ġ ����
	// int64 AccountId
	// int64 PlayerId
	// int64 GainExp
	// int64 CurrentExp
	// int64 RequireExp
	// int64 TotalExp
	//------------------------------------------------------------
	en_PACKET_S2C_EXPERIENCE,

	//------------------------------------------------------------
	// ���Ӽ��� ���� ��Ŷ
	// int64 PlayerId
	// st_SkillInfo SkillInfo
	//------------------------------------------------------------
	en_PACKET_S2C_BUF_DEBUF,

	//------------------------------------------------------------
	// ���Ӽ��� ��ȭȿ�� ��ȭȿ�� ���� ��Ŷ
	// int64 TargetObjectId
	// en_SkillType OffSkillType
	//------------------------------------------------------------
	en_PACKET_S2C_BUF_DEBUF_OFF,

	//------------------------------------------------------------
	// ���Ӽ��� ���ӱ� ��ų �ѱ� ��Ŷ
	// int8 QuickSlotBarIndex
	// int8 QuickSlotBarSlotIndex
	// st_SkillInfo ComboSkillInfo
	//------------------------------------------------------------
	en_PACKET_S2C_COMBO_SKILL_ON,

	//------------------------------------------------------------
	// ���Ӽ��� ���ӱ� ��ų ���� ��Ŷ
	// int8 QuickSlotBarIndex
	// int8 QuickSlotBarSlotIndex
	// en_SkillType ComboSkilltype
	//------------------------------------------------------------
	en_PACKET_S2C_COMBO_SKILL_OFF,

	//-----------------------------------------------------------
	// ���Ӽ��� ���� �޼��� ����
	// int8 MessageCount
	// wstring Messages	
	//-----------------------------------------------------------
	en_PACKET_S2C_PERSONAL_MESSAGE,

	//-----------------------------------------------------------
	// ���Ӽ��� UI �޴� Ÿ�� ���� ��û
	// int64 AccountID
	// int64 PlayerID
	//-----------------------------------------------------------
	en_PACKET_C2S_UI_MENU_TILE_BUY,

	//-----------------------------------------------------------
	// ���Ӽ��� UI �޴� Ÿ�� ���� ��û ����
	//-----------------------------------------------------------
	en_PACKET_S2C_UI_MENU_TILE_BUY,

	//-----------------------------------------------------------
	// ���Ӽ��� Ÿ�� ���� ��û
	// int64 AccountID
	// int64 PlayerID
	// int32 TilePositionX
	// int32 TilePositionY
	//-----------------------------------------------------------
	en_PACKET_C2S_TILE_BUY,

	//-----------------------------------------------------------
	// ���Ӽ��� Ÿ�� ���� ��û ����   
	// int64 TileObjectID
	// int32 TilePositionX
	// int32 TilePositionY
	//-----------------------------------------------------------
	en_PACKET_S2C_TILE_BUY,

	//-----------------------------------------------------------
	// ���Ӽ��� ���� �ɱ� ��û
	// int64 AccountID
	// int64 PlayerID
	// int16 SeedItemSmallCategory
	//-----------------------------------------------------------
	en_PACKET_C2S_SEED_FARMING,

	//-----------------------------------------------------------
	// ���Ӽ��� ���� �ɱ� ��û ����
	//-----------------------------------------------------------
	en_PACKET_S2C_SEED_FARMING,

	//-----------------------------------------------------------
	// ���Ӽ��� �۹� ���� Ȯ�� ��û
	// int64 AccountID
	// int64 PlayerID
	// int16 PlantObjectType
	//-----------------------------------------------------------
	en_PACKET_C2S_PLANT_GROWTH_CHECK,
	
	//-----------------------------------------------------------
	// ���Ӽ��� �۹� ���� Ȯ�� ��û
	// int64 PlantObjectID
	// int8 PlantGrowthStep	
	//-----------------------------------------------------------
	en_PACKET_S2C_PLANT_GROWTH_CHECK,

	//-----------------------------------------------------------
	// ���Ӽ��� �ð� ��û 
	// int64 AccountID
	// int64 PlayerID
	//-----------------------------------------------------------
	en_PACKET_C2S_SERVER_TIME,

	//-----------------------------------------------------------
	// ���Ӽ��� �ð� ��û ����
	// int64 ServerDayTime
	//-----------------------------------------------------------
	en_PACKET_S2C_SERVER_TIME,
	
	//-----------------------------------------------------------
	// ���Ӽ��� Ŭ�� �� ����	
	//-----------------------------------------------------------
	en_PACKET_C2S_PONG,

	//-----------------------------------------------------------
	// ���Ӽ��� ���� �� ����
	// int64 AccountId
	// int64 PlayerId
	//-----------------------------------------------------------
	en_PACKET_S2C_PING
};

enum en_LOGIN_SERVER_PACKET_TYPE
{
	////////////////////////////////////////////////////////
	//
	//	Client & LoginServer Protocol
	//
	////////////////////////////////////////////////////////

	//------------------------------------------------------
	// Login Server
	//------------------------------------------------------
	en_PACKET_CS_LOGIN_SERVER = 1000,

	//----------------------------------
	// �α��� ���� ȸ������ ��û
	// wstring AccountName
	// wstring Password
	//----------------------------------
	en_LOGIN_SERVER_C2S_ACCOUNT_NEW,

	//----------------------------------
	// �α��� ���� ȸ������ ��û ����
	// bool AccountNewSuccess
	//----------------------------------
	en_LOGIN_SERVER_S2C_ACCOUNT_NEW,

	//----------------------------------
	// �α��� ���� �α��� ��û
	// wstring AccountName
	// wstring Password
	//----------------------------------
	en_LOGIN_SERVER_C2S_ACCOUNT_LOGIN,

	//----------------------------------
	// �α��� ���� �α��� ��û ����
	// en_LoginInfo LoginInfo
	// int64 AccountID
	// wstring AccountName
	// int8 TokenLen
	// byte Token[]
	// int8 ServerListSize
	// vector<st_ServerInfo> ServerLists
	//----------------------------------
	en_LOGIN_SERVER_S2C_ACCOUNT_LOGIN,

	//---------------------------------
	// �α��� ���� �α׾ƿ� ��û 
	// int64 AccountID	
	//---------------------------------
	en_LOGIN_SERVER_C2S_ACCOUNT_LOGOUT,

	//---------------------------------
	// �α��� ���� �α׾ƿ� ��û ����
	//---------------------------------
	en_LOGIN_SERVER_S2C_ACCOUNT_LOGOUT,

	//--------------------------------
	// �α��� ���� �α��� ���� �ٲٱ� ��û
	// int64 AccountID
	// en_LoginState LoginState
	//--------------------------------
	en_LOGIN_SERVER_C2S_LOGIN_STATE_CHANGE

};