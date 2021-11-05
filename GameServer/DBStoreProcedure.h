#pragma once
#include "DBBind.h"

namespace SP
{
	class CDBAccountTokenGet : public CDBBind<1, 3>
	{
	public:
		CDBAccountTokenGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetAccountToken(?)}") { }
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		void OutToken(int32& Token) { BindCol(0, Token); }
		void OutLoginsuccessTime(TIMESTAMP_STRUCT& LoginSuccessTime) { BindCol(1, LoginSuccessTime); }
		void OutTokenExpiredTime(TIMESTAMP_STRUCT& TokenExpiredTime) { BindCol(2, TokenExpiredTime); }
	};

	// AccountID�� �������� Ŭ�� �����ϰ� �ִ� ĳ���͸� ã�´�.
	class CDBGameServerPlayersGet : public CDBBind<1, 26>
	{
	public:
		CDBGameServerPlayersGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetPlayers(?)}") { }
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }

		void OutPlayerDBID(int64& PlayerDBID) { BindCol(0, PlayerDBID); }
		template<int32 N> void OutPlayerName(WCHAR(&PlayerName)[N]) { BindCol(1, PlayerName); }
		void OutPlayerObjectType(int16& ObjectType) { BindCol(2, ObjectType); }
		void OutPlayerIndex(int8& PlayerIndex) { BindCol(3, PlayerIndex); }
		void OutLevel(int32& Level) { BindCol(4, Level); }
		void OutCurrentHP(int32& CurrentHP) { BindCol(5, CurrentHP); }
		void OutMaxHP(int32& MaxHP) { BindCol(6, MaxHP); }
		void OutCurrentMP(int32& CurrentMP) { BindCol(7, CurrentMP); }
		void OutMaxMP(int32& MaxMP) { BindCol(8, MaxMP); }
		void OutCurrentDP(int32& CurrentDP) { BindCol(9, CurrentDP); }
		void OutMaxDP(int32& MaxDP) { BindCol(10, MaxDP); }
		void OutMinMeleeAttackDamage(int32& MinMeleeAttackDamage) { BindCol(11, MinMeleeAttackDamage); }
		void OutMaxMeleeAttackDamage(int32& MaxMeleeAttackDamage) { BindCol(12, MaxMeleeAttackDamage); }
		void OutMeleeAttackHitRate(int16& MeleeAttackHitRate) { BindCol(13, MeleeAttackHitRate); }
		void OutMagicDamage(int16& MagicDamage) { BindCol(14, MagicDamage); }
		void OutMagicHitRate(int16& MagicHitRate) { BindCol(15, MagicHitRate); }
		void OutDefence(int32& Defence) { BindCol(16, Defence); }
		void OutEvasionRate(int16& EvasionRate) { BindCol(17, EvasionRate); }
		void OutMeleeCriticalPoint(int16& MeleeCriticalPoint) { BindCol(18, MeleeCriticalPoint); }
		void OutMagicCriticalPointt(int16& MagicCriticalPoint) { BindCol(19, MagicCriticalPoint); }
		void OutSpeed(float& Speed) { BindCol(20, Speed); }		
		void OutLastPositionY(int32& LastPositionY) { BindCol(21, LastPositionY); }
		void OutLastPositionX(int32& LastPositionX) { BindCol(22, LastPositionX); }
		void OutCurrentExperience(int64& CurrentExperience) { BindCol(23, CurrentExperience); }
		void OutRequireExperience(int64& RequireExperience) { BindCol(24, RequireExperience); }
		void OutTotalExperience(int64& TotalExperience) { BindCol(25, TotalExperience); }
	};

	// DB�� �Է��� �ش� ĳ���Ͱ� �ִ��� Ȯ��
	class CDBGameServerCharacterNameGet : public CDBBind<1, 0>
	{
	public:
		CDBGameServerCharacterNameGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetCharacterName(?)}") {}
		void InCharacterName(wstring& CharacterName) { BindParam(0, CharacterName.c_str()); }
	};

	// DB�� ���ο� ĳ�� ����
	class CDBGameServerCreateCharacterPush : public CDBBind<26, 0>
	{
	public:
		CDBGameServerCreateCharacterPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spNewCharacterPush(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		void InPlayerName(wstring& PlayerName) { BindParam(1, PlayerName.c_str()); }
		void InPlayerType(int16& PlayerType) { BindParam(2, PlayerType); }
		void InPlayerIndex(int8& PlayerIndex) { BindParam(3, PlayerIndex); }
		void InLevel(int32& Level) { BindParam(4, Level); }
		void InCurrentHP(int32& CurrentHP) { BindParam(5, CurrentHP); }
		void InMaxHP(int32& MaxHP) { BindParam(6, MaxHP); }
		void InCurrentMP(int32& CurrentMP) { BindParam(7, CurrentMP); }
		void InMaxMP(int32& MaxMP) { BindParam(8, MaxMP); }
		void InCurrentDP(int32& CurrentDP) { BindParam(9, CurrentDP); }
		void InMaxDP(int32& MaxDP) { BindParam(10, MaxDP); }
		void InMinMeleeAttackDamage(int32& MinMeleeAttackDamage) { BindParam(11, MinMeleeAttackDamage); }
		void InMaxMeleeAttackDamage(int32& MaxMeleeAttackDamage) { BindParam(12, MaxMeleeAttackDamage); }
		void InMeleeAttackHitRate(int16& MeleeAttackHitRate) { BindParam(13, MeleeAttackHitRate); }
		void InMagicDamage(int16& MagicDamage) { BindParam(14, MagicDamage); }
		void InMagicHitRate(int16& MagicHitRate) { BindParam(15, MagicHitRate); }
		void InDefence(int32& Defence) { BindParam(16, Defence); }
		void InEvasionRate(int16& EvasionRate) { BindParam(17, EvasionRate); }
		void InMeleeCriticalPoint(int16& MeleeCriticalPoint) { BindParam(18, MeleeCriticalPoint); }
		void InMagicCriticalPoint(int16& MagicCriticalPoint) { BindParam(19, MagicCriticalPoint); }
		void InSpeed(float& Speed) { BindParam(20, Speed); }
		void InLastPositionY(int32& LastPositionY) { BindParam(21, LastPositionY); }
		void InLastPositionX(int32& LastPositionX) { BindParam(22, LastPositionX); }
		void InCurrentExperence(int64& CurrentExperence) { BindParam(23, CurrentExperence); }
		void InRequireExperience(int64& RequireExperience) { BindParam(24, RequireExperience); }
		void InTotalExperience(int64& CurrentExperence) { BindParam(25, CurrentExperence); }
	};

	// ĳ���� DBid ���
	class CDBGameServerPlayerDBIDGet : public CDBBind<2, 1>
	{
	public:
		CDBGameServerPlayerDBIDGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spPlayerDBIdGet(?,?)}") {}
		void InAccountID(int64& AccountId) { BindParam(0, AccountId); }
		void InPlayerSlotIndex(int8& PlayerSlotIndex) { BindParam(1, PlayerSlotIndex); }
		void OutPlayerDBID(int64& PlayerDBId) { BindCol(0, PlayerDBId); }
	};

	// ItemTable�� ���ο� Item ����
	class CDBGameServerCreateItem : public CDBBind<13, 0>
	{
	public:
		CDBGameServerCreateItem(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemCreate(?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InItemUse(bool& ItemUse) { BindParam(0, ItemUse); }
		void InIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(1, IsQuickSlotUse); }
		void InItemLargeCategory(int8& ItemLargeCategory) { BindParam(2, ItemLargeCategory); }
		void InItemMediumCategory(int8& ItemMediumCategory) { BindParam(3, ItemMediumCategory); }
		void InItemSmallCategory(int16& ItemSmallCategory) { BindParam(4, ItemSmallCategory); }
		void InItemName(wstring& ItemName) { BindParam(5, ItemName.c_str()); }
		void InItemMinDamage(int32& ItemMinDamage) { BindParam(6, ItemMinDamage); }
		void InItemMaxDamage(int32& ItemMaxDamage) { BindParam(7, ItemMaxDamage); }
		void InItemDefence(int32& ItemDefence) { BindParam(8, ItemDefence); }
		void InItemMaxCount(int32& ItemMaxCount) { BindParam(9, ItemMaxCount); }
		void InItemCount(int16& ItemCount) { BindParam(10, ItemCount); }
		void InIsEquipped(bool& IsEquipped) { BindParam(11, IsEquipped); }		
		void InThumbnailImagePath(wstring& ThumbnailImagePath) { BindParam(12, ThumbnailImagePath.c_str()); }
	};

	// ItemDBId ���
	class CDBGameServerItemDBIdGet : public CDBBind<0, 1>
	{
	public:
		CDBGameServerItemDBIdGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemDBIdGet()}") {}
		void OutItemDBId(int64& ItemDBId) { BindCol(0, ItemDBId); }
	};

	// �κ��丮 �ʱ�ȭ �Ҷ� �󲮵��� �����ؼ� �ִ� ���ν��� Ŭ����
	class CDBGameServerItemCreateToInventory : public CDBBind<5, 0>
	{
	public:
		CDBGameServerItemCreateToInventory(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemToInventoryInit(?,?,?,?,?)}") {}
		void InItemName(wstring& ItemName) { BindParam(0, ItemName.c_str()); }
		void InSlotIndex(int8& SlotIndex) { BindParam(1, SlotIndex); }
		void InThumbnailImagePath(wstring& ThumbnailImagePath) { BindParam(2, ThumbnailImagePath.c_str()); }
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(3, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(4, OwnerPlayerId); }
	};

	// InventoryTable�� ���ο� Item ����
	class CDBGameServerItemToInventoryPush : public CDBBind<15, 0>
	{
	public:
		CDBGameServerItemToInventoryPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemToInventorySave(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(0, IsQuickSlotUse); }
		void InItemLargeCategory(int8& ItemLargeCategory) { BindParam(1, ItemLargeCategory); }
		void InItemMediumCategory(int8& ItemMediumCategory) { BindParam(2, ItemMediumCategory); }
		void InItemSmallCategory(int16& ItemSmallCategory) { BindParam(3, ItemSmallCategory); }
		void InItemName(wstring& ItemName) { BindParam(4, ItemName.c_str()); }
		void InItemMinDamage(int32& ItemMinDamage) { BindParam(5, ItemMinDamage); }
		void InItemMaxDamage(int32& ItemMaxDamage) { BindParam(6, ItemMaxDamage); }
		void InItemDefence(int32& ItemDefence) { BindParam(7, ItemDefence); }
		void InItemMaxCount(int32& ItemMaxCount) { BindParam(8, ItemMaxCount); }
		void InItemCount(int16& ItemCount) { BindParam(9, ItemCount); }
		void InSlotIndex(int8& SlotIndex) { BindParam(10, SlotIndex); }
		void InIsEquipped(bool& IsEquipped) { BindParam(11, IsEquipped); }		
		void InThumbnailImagePath(wstring& ThumbnailImagePath) { BindParam(12, ThumbnailImagePath.c_str()); }
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(13, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(14, OwnerPlayerId); }
	};

	// ItemTable�� Item ���� ����
	class CDBGameServerItemRefreshPush : public CDBBind<5, 0>
	{
	public:
		CDBGameServerItemRefreshPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spItemRefreshCount(?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InItemType(int16& ItemType) { BindParam(2, ItemType); }
		void InCount(int16& Count) { BindParam(3, Count); }
		void InSlotIndex(int8& SlotIndex) { BindParam(4, SlotIndex); }
	};

	// Swap ��û�� �������� Inventory�� �ִ��� Ȯ���ϰ�
	// ��û�� �������� ������ ��ȯ
	class CDBGameServerItemCheck : public CDBBind<3, 12>
	{
	public:
		CDBGameServerItemCheck(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventoryItemCheck(?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InSlotIndex(int8& SlotIndex) { BindParam(2, SlotIndex); }

		void OutIsQuickSlotUse(bool& IsQuickSlotUse) { BindCol(0, IsQuickSlotUse); }
		void OutItemLargeCategory(int8& ItemLargeCategory) { BindCol(1, ItemLargeCategory); }
		void OutItemMediumCategory(int8& ItemMediumCategory) { BindCol(2, ItemMediumCategory); }
		void OutItemSmallCategory(int16& ItemSmallCategory) { BindCol(3, ItemSmallCategory); }
		template<int8 Length> void OutItemName(WCHAR(&ItemName)[Length]) { BindCol(4, ItemName); }
		void OutItemMinDamage(int32& ItemMinDamage) { BindCol(5, ItemMinDamage); }
		void OutItemMaxDamage(int32& ItemMaxDamage) { BindCol(6, ItemMaxDamage); }
		void OutItemDefence(int32& ItemDefence) { BindCol(7, ItemDefence); }
		void OutItemMaxCount(int32& ItemMaxCount) { BindCol(8, ItemMaxCount); }
		void OutItemCount(int16& ItemCount) { BindCol(9, ItemCount); }
		void OutItemIsEquipped(bool& ItemIsEquipped) { BindCol(10, ItemIsEquipped); }	
		template<int16 Length> void OutItemThumbnailImagePath(WCHAR(&ItemThumbnailImagePath)[Length]) { BindCol(11, ItemThumbnailImagePath); }
	};

	// �κ��丮 ������ ����
	class CDBGameServerItemSwap : public CDBBind<28, 0>
	{
	public:
		CDBGameServerItemSwap(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventoryItemSwap(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InAIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(2, IsQuickSlotUse); }
		void InAItemLargeCategory(int8& AItemCategoryType) { BindParam(3, AItemCategoryType); }
		void InAItemMediumCategory(int8& AItemMediumCategory) { BindParam(4, AItemMediumCategory); }
		void InAItemSmallCategory(int16& AItemType) { BindParam(5, AItemType); }
		void InAItemName(wstring& AItemName) { BindParam(6, AItemName.c_str()); }
		void InAItemMinDamage(int32& AItemMinDamage) { BindParam(7, AItemMinDamage); }
		void InAItemMaxDamage(int32& AItemMaxDamage) { BindParam(8, AItemMaxDamage); }
		void InAItemDefence(int32& AItemDefence) { BindParam(9, AItemDefence); }
		void InAItemMaxCount(int32& AItemMaxCount) { BindParam(10, AItemMaxCount); }
		void InAItemCount(int16& AItemCount) { BindParam(11, AItemCount); }
		void InAItemIsEquipped(bool& AItemIsEquipped) { BindParam(12, AItemIsEquipped); }		
		void InAItemThumbnailImagePath(wstring& AItemThumbnailImagePath) { BindParam(13, AItemThumbnailImagePath.c_str()); }
		void InAItemSlotIndex(int8& AItemSlotIndex) { BindParam(14, AItemSlotIndex); }

		void InBIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(15, IsQuickSlotUse); }
		void InBItemLargeCategory(int8& BItemLargeCategory) { BindParam(16, BItemLargeCategory); }
		void InBItemMediumCategory(int8& BItemMediumCategory) { BindParam(17, BItemMediumCategory); }
		void InBItemSmallCategory(int16& BItemSmallCategory) { BindParam(18, BItemSmallCategory); }
		void InBItemName(wstring& BItemName) { BindParam(19, BItemName.c_str()); }
		void InBItemMinDamage(int32& AItemCount) { BindParam(20, AItemCount); }
		void InBItemMaxDamage(int32& AItemCount) { BindParam(21, AItemCount); }
		void InBItemDefence(int32& AItemCount) { BindParam(22, AItemCount); }
		void InBItemMaxCount(int32& AItemCount) { BindParam(23, AItemCount); }
		void InBItemCount(int16& BItemCount) { BindParam(24, BItemCount); }
		void InBItemIsEquipped(bool& BItemIsEquipped) { BindParam(25, BItemIsEquipped); }		
		void InBItemThumbnailImagePath(wstring& BItemThumbnailImagePath) { BindParam(26, BItemThumbnailImagePath.c_str()); }
		void InBItemSlotIndex(int8& BItemSlotIndex) { BindParam(27, BItemSlotIndex); }
	};

	// ������ ����
	class CDBGameServerItemDelete : public CDBBind<2, 0>
	{
	public:
		CDBGameServerItemDelete(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spItemDelete(?,?)}") {}
		void InItemDBId(int64& ItemDBId) { BindParam(0, ItemDBId); }
		void InItemUse(bool& ItemUse) { BindParam(1, ItemUse); }
	};

	// �κ��丮 ������ �ʱ�ȭ
	class CDBGameServerInventorySlotInit : public CDBBind<5, 0>
	{
	public:
		CDBGameServerInventorySlotInit(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventorySlotInit(?,?,?,?,?)}") {}
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(0, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(1, OwnerPlayerId); }
		void InSlotIndex(int8& SlotIndex) { BindParam(2, SlotIndex); }
		void InItemName(wstring& ItemName) { BindParam(3, ItemName.c_str()); }
		void InItemThumbnailImagePath(wstring& ItemThumbnailImagePath) { BindParam(4, ItemThumbnailImagePath.c_str()); }
	};

	// �κ��丮 ������ ������Ʈ
	class CDBGameServerInventoryItemUpdate : public CDBBind<5, 0>
	{
	public:
		CDBGameServerInventoryItemUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventoryItemUpdate(?,?,?,?,?)}") {}
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(0, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(1, OwnerPlayerId); }
		void InSlotIndex(int8& SlotIndex) { BindParam(2, SlotIndex); }
		void InItemCount(int16& ItemCount) { BindParam(3, ItemCount); }
		void InIsEquipped(bool& IsEquipped) { BindParam(4, IsEquipped); }
	};

	// GoldTable ����
	class CDBGameServerGoldTableCreatePush : public CDBBind<2, 0>
	{
	public:
		CDBGameServerGoldTableCreatePush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGoldTableCreate(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
	};

	// GoldTable�� Gold ����
	class CDBGameServerGoldPush : public CDBBind<5, 0>
	{
	public:
		CDBGameServerGoldPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGoldSave(?,?,?,?,?)}") {}
		void InAccoountId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InGoldCoin(int64& GoldCoin) { BindParam(2, GoldCoin); }
		void InSliverCoin(int16& SliverCoin) { BindParam(3, SliverCoin); }
		void InBronzeCoin(int16& BronzeCoin) { BindParam(4, BronzeCoin); }
	};

	// GoldTable�� �ִ� Gold �ܾ��
	class CDBGameServerGoldGet : public CDBBind<2, 3>
	{
	public:
		CDBGameServerGoldGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetGoldTableInfoToInventory(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutGoldCoin(int64& GoldCoin) { BindCol(0, GoldCoin); }
		void OutSliverCoin(int16& SliverCoin) { BindCol(1, SliverCoin); }
		void OutBronzeCoin(int16& BronzeCoin) { BindCol(2, BronzeCoin); }
	};

	// InventoryTable�� �ִ� Item ��� �ܾ��
	class CDBGameServerInventoryItemGet : public CDBBind<2, 12>
	{
	public:
		CDBGameServerInventoryItemGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetItemTableInfoToInventory(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutItemLargeCategory(int8& ItemLargeCategory) { BindCol(0, ItemLargeCategory); }
		void OutItemMediumCategory(int8& ItemMediumCategory) { BindCol(1, ItemMediumCategory); }	
		void OutItemSmallCategory(int16& ItemSmallCategory) { BindCol(2, ItemSmallCategory); }
		template<int8 Length> void OutItemName(WCHAR(&ItemName)[Length]) { BindCol(3, ItemName); }
		void OutMinDamage(int32& MinDamage) { BindCol(4, MinDamage); }
		void OutMaxDamage(int32& MaxDamage) { BindCol(5, MaxDamage); }
		void OutDefence(int32& Defence) { BindCol(6, Defence); }
		void OutMaxCount(int32& MaxCount) { BindCol(7, MaxCount); }
		void OutItemCount(int16& itemCount) { BindCol(8, itemCount); }
		void OutSlotIndex(int8& SlotIndex) { BindCol(9, SlotIndex); }
		void OutIsEquipped(bool& IsEquipped) { BindCol(10, IsEquipped); }		
		template<int16 Length> void OutItemThumbnailImagePath(WCHAR(&ItemThumbnailImagePath)[Length]) { BindCol(11, ItemThumbnailImagePath); }
	};

	// SkillTable�� �ִ� Skill ��� �ܾ��
	class CDBGameServerSkillGet : public CDBBind<2, 5>
	{
	public:
		CDBGameServerSkillGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetSkill(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutIsQuickSlotUse(bool& OutIsQuickSlotUse) { BindCol(0, OutIsQuickSlotUse); }
		void OutSkillLargeCategory(int8& SkillLargeCategory) { BindCol(1, SkillLargeCategory); }
		void OutSkillMediumCategory(int8& SkillMediumCategory) { BindCol(2, SkillMediumCategory); }
		void OutSkillType(int16& SkillType) { BindCol(3, SkillType); }
		void OutSkillLevel(int8& SkillLevel) { BindCol(4, SkillLevel); }		
	};

	// ��ų ���̺� ��ų �ֱ�
	class CDBGameServerSkillToSkillBox : public CDBBind<7, 0>
	{
	public:
		CDBGameServerSkillToSkillBox(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spSkillToSkillBox(?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(2, IsQuickSlotUse); }
		void InSkillLargeCategory(int8& SkillLargeCategory) { BindParam(3, SkillLargeCategory); };
		void InSkillMediumCategory(int8& SkillMediumCategory) { BindParam(4, SkillMediumCategory); };
		void InSkillType(int16& SkillType) { BindParam(5, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(6, SkillLevel); }		
	};

	// QuickSlotBarSlot���� ���� ����
	class CDBGameServerQuickSlotBarSlotCreate : public CDBBind<9, 0>
	{
	public:
		CDBGameServerQuickSlotBarSlotCreate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spQuickSlotBarSlotCreate(?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InQuickSlotKey(int16& QuickSlotKey) { BindParam(4, QuickSlotKey); }
		void InSkillLargeCategory(int8& SkillLargeCategory) { BindParam(5, SkillLargeCategory); };
		void InSkillMediumCategory(int8& SkillMediumCategory) { BindParam(6, SkillMediumCategory); };
		void InSkillType(int16& SkillType) { BindParam(7, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(8, SkillLevel); }		
	};

	// QuickSlotBarSlot���� ������Ʈ ���ν���
	class CDBGameServerQuickSlotBarSlotUpdate : public CDBBind<9, 0>
	{
	public:
		CDBGameServerQuickSlotBarSlotUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spQuickSlotBarSlotUpdate(?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InQuickSlotKey(int16& QuickSlotKey) { BindParam(4, QuickSlotKey); }
		void InSkillLargeCategory(int8& SkillLargeCategory) { BindParam(5, SkillLargeCategory); };
		void InSkillMediumCategory(int8& SkillMediumCategory) { BindParam(6, SkillMediumCategory); };
		void InSkillType(int16& SkillType) { BindParam(7, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(8, SkillLevel); }		
	};

	// QuickSlotBarTable�� �ִ� QuickSlotBar ���� ��� �ܾ�´�.
	class CDBGameServerQuickSlotBarGet : public CDBBind<2, 7>
	{
	public:
		CDBGameServerQuickSlotBarGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetQuickSlotBarSlot(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutQuickSlotBarIndex(int8& SlotBarIndex) { BindCol(0, SlotBarIndex); }
		void OutQuickSlotBarItemIndex(int8& SlotBarItemIndex) { BindCol(1, SlotBarItemIndex); }
		void OutQuickSlotKey(int16& QuickSlotKey) { BindCol(2, QuickSlotKey); }
		void OutQuickSlotSkillLargeCategory(int8& SkillLargeCategory) { BindCol(3, SkillLargeCategory); }
		void OutQuickSlotSkillMediumCategory(int8& SkillMediumCategory) { BindCol(4, SkillMediumCategory); }
		void OutQuickSlotSkillType(int16& SkillType) { BindCol(5, SkillType); }
		void OutQuickSlotSkillLevel(int8& SkillLevel) { BindCol(6, SkillLevel); }		
	};

	// Swap ��û�� �������� QuickSlot�� �ִ��� Ȯ���ϰ�
	// ��û�� ������ ���� ��ȯ
	class CDBGameServerQuickSlotCheck : public CDBBind<4, 5>
	{
	public:
		CDBGameServerQuickSlotCheck(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spQuickSlotCheck(?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }

		void OutQuickSlotKey(int16& QuickSlotKey) { BindCol(0, QuickSlotKey); }
		void OutQuickSlotSkillLargeCategory(int8& SkillLargeCategory) { BindCol(1, SkillLargeCategory); }
		void OutQuickSlotSkillMediumCategory(int8& SkillMediumCategory) { BindCol(2, SkillMediumCategory); }
		void OutQuickSlotSkillType(int16& SkillType) { BindCol(3, SkillType); }
		void OutQuickSlotSkillLevel(int8& SkillLevel) { BindCol(4, SkillLevel); }		
	};

	// QuickSlot Swap
	class CDBGameServerQuickSlotSwap : public CDBBind<14, 0>
	{
	public:
		CDBGameServerQuickSlotSwap(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spQuickSlotSwap(?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InAQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InAQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InASkillLargeCategory(int8& SkillLargeCategory) { BindParam(4, SkillLargeCategory); };
		void InASkillMediumCategory(int8& SkillMediumCategory) { BindParam(5, SkillMediumCategory); };
		void InAQuickSlotSkillType(int16& QuickSlotSkillType) { BindParam(6, QuickSlotSkillType); }
		void InAQuickSlotSkillLevel(int8& QuickSkillLevel) { BindParam(7, QuickSkillLevel); }		

		void InBQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(8, QuickSlotBarIndex); }
		void InBQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(9, QuickSlotBarSlotIndex); }
		void InBSkillLargeCategory(int8& SkillLargeCategory) { BindParam(10, SkillLargeCategory); };
		void InBSkillMediumCategory(int8& SkillMediumCategory) { BindParam(11, SkillMediumCategory); };
		void InBQuickSlotSkillType(int16& QuickSlotSkillType) { BindParam(12, QuickSlotSkillType); }
		void InBQuickSlotSkillLevel(int8& QuickSkillLevel) { BindParam(13, QuickSkillLevel); }		
	};

	// ������ ���� �ʱ�ȭ
	class CDBGameServerQuickSlotInit : public CDBBind<6, 0>
	{
	public:
		CDBGameServerQuickSlotInit(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spQuickSlotInit(?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InQuickSlotBarSkillName(wstring& QuickSlotBarSkillName) { BindParam(4, QuickSlotBarSkillName.c_str()); }
		void InQuickSlotBarSkillThumbnailImagePath(wstring& QuickSlotBarSkillThumbnailImagePath) { BindParam(5, QuickSlotBarSkillThumbnailImagePath.c_str()); }
	};

	// ���� ����� �÷��̾� ���� DB�� ���
	class CDBGameServerPlayerLeaveInfoSave : public CDBBind<21, 0>
	{
	public:
		CDBGameServerPlayerLeaveInfoSave(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spPlayerLeaveInfoSave(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}"){}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InLevel(int32& Level) { BindParam(2, Level); }
		void InMaxHP(int32& MaxHP) { BindParam(3, MaxHP); }
		void InMaxMP(int32& MaxMP) { BindParam(4, MaxMP); }
		void InMaxDP(int32& MaxDP) { BindParam(5, MaxDP); }
		void InMinMeleeAttackDamage(int32& MinMeleeAttackDamage) { BindParam(6, MinMeleeAttackDamage); }
		void InMaxMeleeAttackDamage(int32& MaxMeleeAttackDamage) { BindParam(7, MaxMeleeAttackDamage); }
		void InMeleeAttackHitRate(int16& MeleeAttackHitRate) { BindParam(8, MeleeAttackHitRate); }
		void InMagicDamage(int16& MagicDamage) { BindParam(9, MagicDamage); }
		void InMagicHitRate(int16& MagicHitRate) { BindParam(10, MagicHitRate); }
		void InDefence(int32& Defence) { BindParam(11, Defence); }
		void InEvasionRate(int16& EvasionRate) { BindParam(12, EvasionRate); }
		void InMeleeCriticalPoint(int16& MeleeCriticalPoint) { BindParam(13, MeleeCriticalPoint); }
		void InMagicCriticalPoint(int16& MagicCriticalPoint) { BindParam(14, MagicCriticalPoint); }
		void InSpeed(float& Speed) { BindParam(15, Speed); }
		void InLastPositionY(int32& LastPositionY) { BindParam(16, LastPositionY); }
		void InLastPositionX(int32& LastPositionX) { BindParam(17, LastPositionX); }
		void InCurrentExperience(int64& CurrentExperience) { BindParam(18, CurrentExperience); }
		void InRequireExperience(int64& RequireExperience) { BindParam(19, RequireExperience); }
		void InTotalExperience(int64& TotalExperience) { BindParam(20, TotalExperience); }
	};
}