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
	class CDBGameServerPlayersGet : public CDBBind<1, 15>
	{
	public:
		CDBGameServerPlayersGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetPlayers(?)}") { }
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		void OutPlayerDBID(int64& PlayerDBID) { BindCol(0, PlayerDBID); }
		template<int32 N> void OutPlayerName(WCHAR(&PlayerName)[N]) { BindCol(1, PlayerName); }
		void OutPlayerIndex(int8& PlayerIndex) { BindCol(2, PlayerIndex); }
		void OutLevel(int32& Level) { BindCol(3, Level); }
		void OutCurrentHP(int32& CurrentHP) { BindCol(4, CurrentHP); }
		void OutMaxHP(int32& MaxHP) { BindCol(5, MaxHP); }
		void OutCurrentMP(int32& CurrentMP) { BindCol(6, CurrentMP); }
		void OutMaxMP(int32& MaxMP) { BindCol(7, MaxMP); }
		void OutCurrentDP(int32& CurrentDP) { BindCol(8, CurrentDP); }
		void OutMaxDP(int32& MaxDP) { BindCol(9, MaxDP); }
		void OutMinAttack(int32& MinAttack) { BindCol(10, MinAttack); }
		void OutMaxAttack(int32& MaxAttack) { BindCol(11, MaxAttack); }
		void OutCriticalPoint(int16& CriticalPoint) { BindCol(12, CriticalPoint); }
		void OutSpeed(float& Speed) { BindCol(13, Speed); }
		void OutPlayerObjectType(int16& ObjectType) { BindCol(14, ObjectType); }
	};

	// DB�� �Է��� �ش� ĳ���Ͱ� �ִ��� Ȯ��
	class CDBGameServerCharacterNameGet : public CDBBind<1, 0>
	{
	public:
		CDBGameServerCharacterNameGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetCharacterName(?)}") {}
		void InCharacterName(wstring& CharacterName) { BindParam(0, CharacterName.c_str()); }
	};

	// DB�� ���ο� ĳ�� ����
	class CDBGameServerCreateCharacterPush : public CDBBind<15, 0>
	{
	public:
		CDBGameServerCreateCharacterPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spNewCharacterPush(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
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
		void InMinAttack(int32& MinAttack) { BindParam(11, MinAttack); }
		void InMaxAttack(int32& MaxAttack) { BindParam(12, MaxAttack); }
		void InCriticalPoint(int16& CriticalPoint) { BindParam(13, CriticalPoint); }
		void InSpeed(float Speed) { BindParam(14, Speed); }
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
	class CDBGameServerCreateItem : public CDBBind<9, 0>
	{
	public:
		CDBGameServerCreateItem(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemCreate(?,?,?,?,?,?,?,?,?)}") {}
		void InItemUse(bool& ItemUse) { BindParam(0, ItemUse); }
		void InIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(1, IsQuickSlotUse); }
		void InItemLargeCategory(int8& ItemLargeCategory) { BindParam(2, ItemLargeCategory); }
		void InItemMediumCategory(int8& ItemMediumCategory) { BindParam(3, ItemMediumCategory); }
		void InItemSmallCategory(int16& ItemSmallCategory) { BindParam(4, ItemSmallCategory); }
		void InItemName(wstring& ItemName) { BindParam(5, ItemName.c_str()); }
		void InItemCount(int16& ItemCount) { BindParam(6, ItemCount); }
		void InIsEquipped(bool& IsEquipped) { BindParam(7, IsEquipped); }
		void InThumbnailImagePath(wstring& ThumbnailImagePath) { BindParam(8, ThumbnailImagePath.c_str()); }
	};

	// ItemDBId ���
	class CDBGameServerItemDBIdGet : public CDBBind<0, 1>
	{
	public:
		CDBGameServerItemDBIdGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemDBIdGet()}") {}
		void OutItemDBId(int64& ItemDBId) { BindCol(0, ItemDBId); }
	};

	// �κ��丮 �ʱ�ȭ �Ҷ� �󲮵��� �����ؼ� �ִ� ���ν��� Ŭ����
	class CDBGameServerItemCreateToInventory : public CDBBind<11, 0>
	{
	public:
		CDBGameServerItemCreateToInventory(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemToInventoryInit(?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InQuickSlotUse(bool& QuickSlotUse) { BindParam(0, QuickSlotUse); }
		void InItemLargeCategory(int8& ItemLargeCategory) { BindParam(1, ItemLargeCategory); }
		void InItemMediumCategory(int8& ItemMediumCategory) { BindParam(2, ItemMediumCategory); }
		void InItemSmallCategory(int16& ItemSmallCategory) { BindParam(3, ItemSmallCategory); }
		void InItemName(wstring& ItemName) { BindParam(4, ItemName.c_str()); }
		void InItemCount(int16& ItemCount) { BindParam(5, ItemCount); }
		void InSlotIndex(int8& SlotIndex) { BindParam(6, SlotIndex); }
		void InIsEquipped(bool& IsEquipped) { BindParam(7, IsEquipped); }
		void InThumbnailImagePath(wstring& ThumbnailImagePath) { BindParam(8, ThumbnailImagePath.c_str()); }
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(9, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(10, OwnerPlayerId); }
	};

	// InventoryTable�� ���ο� Item ����
	class CDBGameServerItemToInventoryPush : public CDBBind<11, 0>
	{
	public:
		CDBGameServerItemToInventoryPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemToInventorySave(?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(0, IsQuickSlotUse); }
		void InItemLargeCategory(int8& ItemLargeCategory) { BindParam(1, ItemLargeCategory); }
		void InItemMediumCategory(int8& ItemMediumCategory) { BindParam(2, ItemMediumCategory); }
		void InItemSmallCategory(int16& ItemSmallCategory) { BindParam(3, ItemSmallCategory); }
		void InItemName(wstring& ItemName) { BindParam(4, ItemName.c_str()); }
		void InItemCount(int16& ItemCount) { BindParam(5, ItemCount); }
		void InSlotIndex(int8& SlotIndex) { BindParam(6, SlotIndex); }
		void InIsEquipped(bool& IsEquipped) { BindParam(7, IsEquipped); }
		void InThumbnailImagePath(wstring& ThumbnailImagePath) { BindParam(8, ThumbnailImagePath.c_str()); }
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(9, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(10, OwnerPlayerId); }
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
	class CDBGameServerItemCheck : public CDBBind<3, 8>
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
		void OutItemCount(int16& ItemCount) { BindCol(5, ItemCount); }
		void OutItemIsEquipped(bool& ItemIsEquipped) { BindCol(6, ItemIsEquipped); }
		template<int16 Length> void OutItemThumbnailImagePath(WCHAR(&ItemThumbnailImagePath)[Length]) { BindCol(7, ItemThumbnailImagePath); }
	};

	// InventoryItemSwap Swap 
	class CDBGameServerItemSwap : public CDBBind<20, 0>
	{
	public:
		CDBGameServerItemSwap(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventoryItemSwap(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InAIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(2, IsQuickSlotUse); }
		void InAItemLargeCategory(int8& AItemCategoryType) { BindParam(3, AItemCategoryType); }
		void InAItemMediumCategory(int8& AItemMediumCategory) { BindParam(4, AItemMediumCategory); }
		void InAItemSmallCategory(int16& AItemType) { BindParam(5, AItemType); }
		void InAItemName(wstring& AItemName) { BindParam(6, AItemName.c_str()); }
		void InAItemCount(int16& AItemCount) { BindParam(7, AItemCount); }
		void InAItemIsEquipped(bool& AItemIsEquipped) { BindParam(8, AItemIsEquipped); }
		void InAItemThumbnailImagePath(wstring& AItemThumbnailImagePath) { BindParam(9, AItemThumbnailImagePath.c_str()); }
		void InAItemSlotIndex(int8& AItemSlotIndex) { BindParam(10, AItemSlotIndex); }

		void InBIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(11, IsQuickSlotUse); }
		void InBItemLargeCategory(int8& BItemLargeCategory) { BindParam(12, BItemLargeCategory); }
		void InBItemMediumCategory(int8& BItemMediumCategory) { BindParam(13, BItemMediumCategory); }
		void InBItemSmallCategory(int16& BItemSmallCategory) { BindParam(14, BItemSmallCategory); }
		void InBItemName(wstring& BItemName) { BindParam(15, BItemName.c_str()); }
		void InBItemCount(int16& BItemCount) { BindParam(16, BItemCount); }
		void InBItemIsEquipped(bool& BItemIsEquipped) { BindParam(17, BItemIsEquipped); }
		void InBItemThumbnailImagePath(wstring& BItemThumbnailImagePath) { BindParam(18, BItemThumbnailImagePath.c_str()); }
		void InBItemSlotIndex(int8& BItemSlotIndex) { BindParam(19, BItemSlotIndex); }
	};

	// Item Delete
	class CDBGameServerItemDelete : public CDBBind<2, 0>
	{
	public:
		CDBGameServerItemDelete(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spItemDelete(?,?)}") {}
		void InItemDBId(int64& ItemDBId) { BindParam(0, ItemDBId); }
		void InItemUse(bool& ItemUse) { BindParam(1, ItemUse); }
	};

	// Inventory Item �ʱ�ȭ
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

	// ItemTable�� �ִ� Item ��� �ܾ��
	class CDBGameServerInventoryItemGet : public CDBBind<2, 8>
	{
	public:
		CDBGameServerInventoryItemGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetItemTableInfoToInventory(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutItemLargeCategory(int8& ItemLargeCategory) { BindCol(0, ItemLargeCategory); }
		void OutItemMediumCategory(int8& ItemMediumCategory) { BindCol(1, ItemMediumCategory); }
		void OutItemSmallCategory(int16& ItemSmallCategory) { BindCol(2, ItemSmallCategory); }
		template<int8 Length> void OutItemName(WCHAR(&ItemName)[Length]) { BindCol(3, ItemName); }
		void OutItemCount(int16& itemCount) { BindCol(4, itemCount); }
		void OutSlotIndex(int8& SlotIndex) { BindCol(5, SlotIndex); }
		void OutIsEquipped(bool& IsEquipped) { BindCol(6, IsEquipped); }
		template<int16 Length> void OutItemThumbnailImagePath(WCHAR(&ItemThumbnailImagePath)[Length]) { BindCol(7, ItemThumbnailImagePath); }
	};

	// SkillTable�� �ִ� Skill ��� �ܾ��
	class CDBGameServerSkillGet : public CDBBind<2, 7>
	{
	public:
		CDBGameServerSkillGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetSkill(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutIsQuickSlotUse(bool& OutIsQuickSlotUse) { BindCol(0, OutIsQuickSlotUse); }
		void OutSkillType(int16& SkillType) { BindCol(1, SkillType); }
		void OutSkillLevel(int8& SkillLevel) { BindCol(2, SkillLevel); }
		template<int8 Length> void OutSkillName(WCHAR(&SkillName)[Length]) { BindCol(3, SkillName); }
		void OutSkillCoolTime(int32& SkillCoolTime) { BindCol(4, SkillCoolTime); }
		void OutSkillCastingTime(int32& SkillCastingTime) { BindCol(5, SkillCastingTime); }
		template<int8 Length> void OutSkillThumbnailImagePath(WCHAR(&SkillThumbnailImagePath)[Length]) { BindCol(6, SkillThumbnailImagePath); }
	};

	// QuickSlotBarSlot���� ���� ����
	class CDBGameServerQuickSlotBarSlotCreate : public CDBBind<11, 0>
	{
	public:
		CDBGameServerQuickSlotBarSlotCreate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spQuickSlotBarSlotCreate(?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InQuickSlotKey(wstring& QuickSlotKey) { BindParam(4, QuickSlotKey.c_str()); }
		void InSkillType(int16& SkillType) { BindParam(5, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(6, SkillLevel); }
		void InSkillName(wstring& SkillName) { BindParam(7, SkillName.c_str()); }
		void InSkillCoolTime(int32& SkillCoolTime) { BindParam(8, SkillCoolTime); }
		void InSkillCastingTime(int32& SkillCastingTime) { BindParam(9, SkillCastingTime); }
		void InSkillThumbnailImagePath(wstring& SkillThumbnailImagePath) { BindParam(10, SkillThumbnailImagePath.c_str()); }
	};

	// QuickSlotBarSlot���� ������Ʈ ���ν���
	class CDBGameServerQuickSlotBarSlotUpdate : public CDBBind<11, 0>
	{
	public:
		CDBGameServerQuickSlotBarSlotUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spQuickSlotBarSlotUpdate(?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InQuickSlotKey(wstring& QuickSlotKey) { BindParam(4, QuickSlotKey.c_str()); }
		void InSkillType(int16& SkillType) { BindParam(5, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(6, SkillLevel); }
		void InSkillName(wstring& SkillName) { BindParam(7, SkillName.c_str()); }
		void InSkillCoolTime(int32& SkillCoolTime) { BindParam(8, SkillCoolTime); }
		void InSkillCastingTime(int32& SkillCastingTime) { BindParam(9, SkillCastingTime); }
		void InSkillThumbnailImagePath(wstring& SkillThumbnailImagePath) { BindParam(10, SkillThumbnailImagePath.c_str()); }
	};

	// QuickSlotBarTable�� �ִ� QuickSlotBar ���� ��� �ܾ�´�.
	class CDBGameServerQuickSlotBarGet : public CDBBind<2, 8>
	{
	public:
		CDBGameServerQuickSlotBarGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetQuickSlotBarSlot(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutQuickSlotBarIndex(int8& SlotBarIndex) { BindCol(0, SlotBarIndex); }
		void OutQuickSlotBarItemIndex(int8& SlotBarItemIndex) { BindCol(1, SlotBarItemIndex); }
		template<int8 Length> void OutQuickSlotKey(WCHAR(&QuickSlotKey)[Length]) { BindCol(2, QuickSlotKey); }
		void OutQuickSlotSkillType(int16& SkillType) { BindCol(3, SkillType); }
		void OutQuickSlotSkillLevel(int8& SkillLevel) { BindCol(4, SkillLevel); }
		void OutQuickSlotSkillCoolTime(int32& SkillCoolTime) { BindCol(5, SkillCoolTime); }
		void OutQuickSlotSkillCastingTime(int32& SkillCastingTime) { BindCol(6, SkillCastingTime); }
		template<int8 Length> void OutQuickSlotSkillThumbnailImagePath(WCHAR(&SkillThumbnailImagePath)[Length]) { BindCol(7, SkillThumbnailImagePath); }
	};

	// Swap ��û�� �������� QuickSlot�� �ִ��� Ȯ���ϰ�
	// ��û�� ������ ���� ��ȯ
	class CDBGameServerQuickSlotCheck : public CDBBind<4, 7>
	{
	public:
		CDBGameServerQuickSlotCheck(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spQuickSlotCheck(?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }

		template<int8 Length> void OutQuickSlotKey(WCHAR(&QuickSlotKey)[Length]) { BindCol(0, QuickSlotKey); }
		void OutQuickSlotSkillType(int16& SkillType) { BindCol(1, SkillType); }
		void OutQuickSlotSkillLevel(int8& SkillLevel) { BindCol(2, SkillLevel); }
		template<int8 Length> void OutQuickSlotSkillName(WCHAR(&QuickSlotSkillName)[Length]) { BindCol(3, QuickSlotSkillName); }
		void OutQuickSlotSkillCoolTime(int32& SkillCoolTime) { BindCol(4, SkillCoolTime); }
		void OutQuickSlotSkillCastingTime(int32& SkillCastingTime) { BindCol(5, SkillCastingTime); }
		template<int8 Length> void OutQuickSlotSkillThumbnailImagePath(WCHAR(&SkillThumbnailImagePath)[Length]) { BindCol(6, SkillThumbnailImagePath); }
	};

	// QuickSlot Swap
	class CDBGameServerQuickSlotSwap : public CDBBind<18, 0>
	{
	public:
		CDBGameServerQuickSlotSwap(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spQuickSlotSwap(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InAQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InAQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InAQuickSlotSkillType(int16& QuickSlotSkillType) { BindParam(4, QuickSlotSkillType); }
		void InAQuickSlotSkillLevel(int8& QuickSkillLevel) { BindParam(5, QuickSkillLevel); }
		void InAQuickSlotSKillName(wstring& QuickSlotSKillNam) { BindParam(6, QuickSlotSKillNam.c_str()); }
		void InAQuickSlotSkillCoolTime(int32& QuickSlotSkillCoolTime) { BindParam(7, QuickSlotSkillCoolTime); }
		void InAQuickSlotSkillCastingTime(int32& QuickSlotSkillCastingTime) { BindParam(8, QuickSlotSkillCastingTime); }
		void InAQuickSlotSKillImagePath(wstring& QuickSlotSKillImagePath) { BindParam(9, QuickSlotSKillImagePath.c_str()); }

		void InBQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(10, QuickSlotBarIndex); }
		void InBQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(11, QuickSlotBarSlotIndex); }
		void InBQuickSlotSkillType(int16& QuickSlotSkillType) { BindParam(12, QuickSlotSkillType); }
		void InBQuickSlotSkillLevel(int8& QuickSkillLevel) { BindParam(13, QuickSkillLevel); }
		void InBQuickSlotSKillName(wstring& QuickSlotSKillNam) { BindParam(14, QuickSlotSKillNam.c_str()); }
		void InBQuickSlotSkillCoolTime(int32& QuickSlotSkillCoolTime) { BindParam(15, QuickSlotSkillCoolTime); }
		void InBQuickSlotSkillCastingTime(int32& QuickSlotSkillCastingTime) { BindParam(16, QuickSlotSkillCastingTime); }
		void InBQuickSlotSKillImagePath(wstring& QuickSlotSKillImagePath) { BindParam(17, QuickSlotSKillImagePath.c_str()); }
	};

	// ������ ���� �ʱ�ȭ
	class CDBGameServerQuickSlotInit : public CDBBind<4, 0>
	{
	public:
		CDBGameServerQuickSlotInit(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spQuickSlotInit(?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
	};
}