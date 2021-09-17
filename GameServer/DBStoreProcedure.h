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
	class CDBGameServerPlayersGet : public CDBBind<1, 11>
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
		void OutMinAttack(int32& MinAttack) { BindCol(6, MinAttack); }
		void OutMaxAttack(int32& MaxAttack) { BindCol(7, MaxAttack); }
		void OutCriticalPoint(int16& CriticalPoint) { BindCol(8, CriticalPoint); }
		void OutSpeed(float& Speed) { BindCol(9, Speed); }
		void OutPlayerObjectType(int16& ObjectType) { BindCol(10, ObjectType); }
	};

	// DB�� �Է��� �ش� ĳ���Ͱ� �ִ��� Ȯ��
	class CDBGameServerCharacterNameGet : public CDBBind<1, 0>
	{
	public:
		CDBGameServerCharacterNameGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetCharacterName(?)}") {}
		void InCharacterName(wstring& CharacterName) { BindParam(0, CharacterName.c_str()); }
	};

	// DB�� ���ο� ĳ�� ����
	class CDBGameServerCreateCharacterPush : public CDBBind<11, 0>
	{	
	public:		
		CDBGameServerCreateCharacterPush(CDBConnection& DBConnection) : CDBBind(DBConnection,L"{CALL dbo.spNewCharacterPush(?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		void InPlayerName(wstring& PlayerName) { BindParam(1, PlayerName.c_str()); }
		void InPlayerType(int16& PlayerType) { BindParam(2, PlayerType); }
		void InPlayerIndex(int8& PlayerIndex) { BindParam(3, PlayerIndex); }
		void InLevel(int32& Level)  { BindParam(4, Level); }
		void InCurrentHP(int32& CurrentHP) { BindParam(5, CurrentHP); }
		void InMaxHP(int32& MaxHP) { BindParam(6, MaxHP); }
		void InMinAttack(int32& MinAttack) { BindParam(7, MinAttack); }
		void InMaxAttack(int32& MaxAttack) { BindParam(8, MaxAttack); }		
		void InCriticalPoint(int16& CriticalPoint) { BindParam(9, CriticalPoint); }
		void InSpeed(float Speed) { BindParam(10, Speed); }
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
	class CDBGameServerCreateItem : public CDBBind<8, 0>
	{
	public:
		CDBGameServerCreateItem(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemCreate(?,?,?,?,?,?,?,?)}") {}
		void InItemUse(bool& ItemUse) { BindParam(0, ItemUse); }
		void InIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(1, IsQuickSlotUse); }
		void InItemType(int16& ItemType) { BindParam(2, ItemType); }
		void InItemConsumableType(int16& ItemConsumableType) { BindParam(3, ItemConsumableType); }
		void InItemName(wstring& ItemName) { BindParam(4, ItemName.c_str()); }
		void InItemCount(int16& ItemCount) { BindParam(5, ItemCount); }
		void InIsEquipped(bool& IsEquipped) { BindParam(6, IsEquipped); }
		void InThumbnailImagePath(wstring& ThumbnailImagePath) { BindParam(7, ThumbnailImagePath.c_str()); }
	};

	// ItemDBId ���
	class CDBGameServerItemDBIdGet : public CDBBind<0, 1>
	{
	public:
		CDBGameServerItemDBIdGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemDBIdGet()}") {}
		void OutItemDBId(int64& ItemDBId) { BindCol(0, ItemDBId); }
	};

	// �κ��丮 �ʱ�ȭ �Ҷ� �󲮵��� �����ؼ� �ִ� ���ν��� Ŭ����
	class CDBGameServerItemCreateToInventory : public CDBBind<10, 0>
	{
	public:
		CDBGameServerItemCreateToInventory(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemToInventoryInit(?,?,?,?,?,?,?,?,?,?)}") {}
		void InQuickSlotUse(bool& QuickSlotUse) { BindParam(0, QuickSlotUse); }
		void InItemType(int16& ItemType) { BindParam(1, ItemType); }
		void InItemConsumableType(int16& ItemConsumableType) { BindParam(2, ItemConsumableType); }
		void InItemName(wstring& ItemName) { BindParam(3, ItemName.c_str()); }
		void InItemCount(int16& ItemCount) { BindParam(4, ItemCount); }
		void InSlotIndex(int8& SlotIndex) { BindParam(5, SlotIndex); }
		void InIsEquipped(bool& IsEquipped) { BindParam(6, IsEquipped); }
		void InThumbnailImagePath(wstring& ThumbnailImagePath) { BindParam(7, ThumbnailImagePath.c_str()); }
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(8, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(9, OwnerPlayerId); }
	};

	// InventoryTable�� ���ο� Item ����
	class CDBGameServerItemToInventoryPush : public CDBBind<10, 0>
	{
	public:
		CDBGameServerItemToInventoryPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemToInventorySave(?,?,?,?,?,?,?,?,?,?)}") {}				
		void InIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(0, IsQuickSlotUse); }
		void InItemType(int16& ItemType) { BindParam(1, ItemType); }
		void InItemConsumableType(int16& ItemConsumableType) { BindParam(2, ItemConsumableType); }
		void InItemName(wstring& ItemName) { BindParam(3, ItemName.c_str()); }
		void InItemCount(int16& ItemCount) { BindParam(4, ItemCount); }
		void InSlotIndex(int8& SlotIndex) { BindParam(5, SlotIndex); }
		void InIsEquipped(bool& IsEquipped) { BindParam(6, IsEquipped); }		
		void InThumbnailImagePath(wstring& ThumbnailImagePath) { BindParam(7, ThumbnailImagePath.c_str()); }
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(8, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(9, OwnerPlayerId); }		
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
	class CDBGameServerItemCheck : public CDBBind<3, 7>
	{
	public:
		CDBGameServerItemCheck(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventoryItemCheck(?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InSlotIndex(int8& SlotIndex) { BindParam(2, SlotIndex); }
		
		void OutIsQuickSlotUse(bool& IsQuickSlotUse) { BindCol(0, IsQuickSlotUse); }
		void OutItemType(int16& ItemType) { BindCol(1, ItemType); }		
		void OutItemConsumableType(int16& ItemConsumableType) { BindCol(2, ItemConsumableType); }
		template<int8 Length> void OutItemName(WCHAR(&ItemName)[Length]) { BindCol(3, ItemName); }
		void OutItemCount(int16& ItemCount) { BindCol(4, ItemCount); }
		void OutItemIsEquipped(bool& ItemIsEquipped) { BindCol(5, ItemIsEquipped); }
		template<int16 Length> void OutItemThumbnailImagePath(WCHAR(&ItemThumbnailImagePath)[Length]) { BindCol(6, ItemThumbnailImagePath); }		
	};

	// InventoryItemSwap Swap 
	class CDBGameServerItemSwap : public CDBBind<18, 0>
	{
	public:		
		CDBGameServerItemSwap(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventoryItemSwap(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InAIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(2, IsQuickSlotUse); }
		void InAItemType(int16& AItemType) { BindParam(3, AItemType); }
		void InAItemConsumableType(int16& AItemConsumableType) { BindParam(4, AItemConsumableType); }
		void InAItemName(wstring& AItemName) { BindParam(5, AItemName.c_str()); }
		void InAItemCount(int16& AItemCount) { BindParam(6, AItemCount); }
		void InAItemIsEquipped(bool& AItemIsEquipped) { BindParam(7, AItemIsEquipped); }
		void InAItemThumbnailImagePath(wstring& AItemThumbnailImagePath) { BindParam(8, AItemThumbnailImagePath.c_str()); }
		void InAItemSlotIndex(int8& AItemSlotIndex) { BindParam(9, AItemSlotIndex); }
		
		void InBIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(10, IsQuickSlotUse); }
		void InBItemType(int16& BItemType) { BindParam(11, BItemType); }
		void InBItemConsumableType(int16& BItemConsumableType) { BindParam(12, BItemConsumableType); }
		void InBItemName(wstring& BItemName) { BindParam(13, BItemName.c_str()); }
		void InBItemCount(int16& BItemCount) { BindParam(14, BItemCount); }
		void InBItemIsEquipped(bool& BItemIsEquipped) { BindParam(15, BItemIsEquipped); }
		void InBItemThumbnailImagePath(wstring& BItemThumbnailImagePath) { BindParam(16, BItemThumbnailImagePath.c_str()); }
		void InBItemSlotIndex(int8& BItemSlotIndex) { BindParam(17, BItemSlotIndex); }
	};

	// Item Delete
	class CDBGameServerItemDelete : public CDBBind<2, 0>
	{
	public:
		CDBGameServerItemDelete(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spItemDelete(?,?)}") {}	
		void InItemDBId(int64& ItemDBId) { BindParam(0, ItemDBId); }
		void InItemUse(bool& ItemUse) { BindParam(1, ItemUse); }
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
		void InSliverCoin(int8& SliverCoin) { BindParam(3, SliverCoin); }
		void InBronzeCoin(int8& BronzeCoin) { BindParam(4, BronzeCoin); }
	};

	// GoldTable�� �ִ� Gold �ܾ��
	class CDBGameServerGoldGet : public CDBBind<2, 3>
	{
	public:
		CDBGameServerGoldGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetGoldTableInfoToInventory(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutGoldCoin(int64& GoldCoin) { BindCol(0, GoldCoin); }
		void OutSliverCoin(int8& SliverCoin) { BindCol(1, SliverCoin); }
		void OutBronzeCoin(int8& BronzeCoin) { BindCol(2, BronzeCoin); }
	};
	
	// ItemTable�� �ִ� Item ��� �ܾ��
	class CDBGameServerInventoryItemGet : public CDBBind<2, 7>
	{
	public:
		CDBGameServerInventoryItemGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetItemTableInfoToInventory(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }		

		void OutItemType(int16& ItemType) { BindCol(0, ItemType); }
		void OutItemConsumableType(int16 &ConsumableType) { BindCol(1, ConsumableType); }
		template<int8 Length> void OutItemName(WCHAR(&ItemName)[Length]) { BindCol(2, ItemName); }
		void OutItemCount(int16& itemCount) { BindCol(3, itemCount); }
		void OutSlotIndex(int8& SlotIndex) { BindCol(4, SlotIndex); }		
		void OutIsEquipped(bool& IsEquipped) { BindCol(5, IsEquipped); }				
		template<int16 Length> void OutItemThumbnailImagePath(WCHAR(&ItemThumbnailImagePath)[Length]) { BindCol(6, ItemThumbnailImagePath); }
	};

	// SkillTable�� �ִ� Skill ��� �ܾ��
	class CDBGameServerSkillGet : public CDBBind<2, 7>
	{
	public:
		CDBGameServerSkillGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetSkill(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutSkillType(int16& SkillType) { BindCol(0, SkillType); }
		void OutSkillLevel(int8& SkillLevel) { BindCol(1, SkillLevel); }
		template<int8 Length> void OutSkillName(WCHAR(&SkillName)[Length]) { BindCol(2, SkillName); }
		void OutSkillCoolTime(int32& SkillCoolTime) { BindCol(3, SkillCoolTime); }
		void OutQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindCol(4, QuickSlotBarIndex); }
		void OutQuickSlotBarItemIndex(int8& OutQuickSlotBarItemIndex) { BindCol(5, OutQuickSlotBarItemIndex); }
		template<int8 Length> void OutSkillThumbnailImagePath(WCHAR(&SkillThumbnailImagePath)[Length]) { BindCol(6, SkillThumbnailImagePath); }
	};

	// QuickSlotBarTable�� �ִ� QuickSlotBar ���� ��� �ܾ�´�.
	class CDBGameServerQuickSlotBarGet : public CDBBind<2, 7>
	{
	public:
		CDBGameServerQuickSlotBarGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetQuickSlotBar(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutQuickSlotBarIndex(int8& SlotBarIndex) { BindCol(1, SlotBarIndex); }
		void OutQuickSlotBarItemIndex(int8& SlotBarItemIndex) { BindCol(2, SlotBarItemIndex); }
		void OutSkillType(int16& SkillType) { BindCol(3, SkillType); }
		void OutSkillLevel(int8& SkillLevel) { BindCol(4, SkillLevel); }
		template<int8 Length> void OutSkillName(WCHAR(&SkillName)[Length]) { BindCol(5, SkillName); }
		void OutSkillCoolTime(int32& SkillCoolTime) { BindCol(6, SkillCoolTime); }
		template<int8 Length> void OutSkillThumbnailImagePath(WCHAR(&SkillThumbnailImagePath)[Length]) { BindCol(6, SkillThumbnailImagePath); }
	};
}