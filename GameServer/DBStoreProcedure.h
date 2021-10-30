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

	// AccountID를 기준으로 클라가 소유하고 있는 캐릭터를 찾는다.
	class CDBGameServerPlayersGet : public CDBBind<1, 18>
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
		void OutMinAttack(int32& MinAttack) { BindCol(11, MinAttack); }
		void OutMaxAttack(int32& MaxAttack) { BindCol(12, MaxAttack); }
		void OutDefence(int32& Defence) { BindCol(13, Defence); }
		void OutCriticalPoint(int16& CriticalPoint) { BindCol(13, CriticalPoint); }
		void OutSpeed(float& Speed) { BindCol(14, Speed); }		
		void OutCurrentExperience(int64& CurrentExperience) { BindCol(15, CurrentExperience); }
		void OutRequireExperience(int64& RequireExperience) { BindCol(16, RequireExperience); }
		void OutTotalExperience(int64& TotalExperience) { BindCol(17, TotalExperience); }
	};

	// DB에 입력한 해당 캐릭터가 있는지 확인
	class CDBGameServerCharacterNameGet : public CDBBind<1, 0>
	{
	public:
		CDBGameServerCharacterNameGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetCharacterName(?)}") {}
		void InCharacterName(wstring& CharacterName) { BindParam(0, CharacterName.c_str()); }
	};

	// DB에 새로운 캐릭 저장
	class CDBGameServerCreateCharacterPush : public CDBBind<19, 0>
	{
	public:
		CDBGameServerCreateCharacterPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spNewCharacterPush(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
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
		void InDefence(int32& Defence) { BindParam(13, Defence); }
		void InCriticalPoint(int16& CriticalPoint) { BindParam(14, CriticalPoint); }
		void InSpeed(float& Speed) { BindParam(15, Speed); }
		void InCurrentExperence(int64& CurrentExperence) { BindParam(16, CurrentExperence); }
		void InRequireExperience(int64& RequireExperience) { BindParam(17, RequireExperience); }
		void InTotalExperience(int64& CurrentExperence) { BindParam(18, CurrentExperence); }
	};

	// 캐릭터 DBid 얻기
	class CDBGameServerPlayerDBIDGet : public CDBBind<2, 1>
	{
	public:
		CDBGameServerPlayerDBIDGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spPlayerDBIdGet(?,?)}") {}
		void InAccountID(int64& AccountId) { BindParam(0, AccountId); }
		void InPlayerSlotIndex(int8& PlayerSlotIndex) { BindParam(1, PlayerSlotIndex); }
		void OutPlayerDBID(int64& PlayerDBId) { BindCol(0, PlayerDBId); }
	};

	// ItemTable에 새로운 Item 생성
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

	// ItemDBId 얻기
	class CDBGameServerItemDBIdGet : public CDBBind<0, 1>
	{
	public:
		CDBGameServerItemDBIdGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemDBIdGet()}") {}
		void OutItemDBId(int64& ItemDBId) { BindCol(0, ItemDBId); }
	};

	// 인벤토리 초기화 할때 빈껍데기 생성해서 넣는 프로시저 클래스
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

	// InventoryTable에 새로운 Item 저장
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

	// ItemTable에 Item 개수 갱신
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

	// Swap 요청한 아이템이 Inventory에 있는지 확인하고
	// 요청한 아이템의 정보를 반환
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

	// 인벤토리 아이템 스왑
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

	// 아이템 삭제
	class CDBGameServerItemDelete : public CDBBind<2, 0>
	{
	public:
		CDBGameServerItemDelete(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spItemDelete(?,?)}") {}
		void InItemDBId(int64& ItemDBId) { BindParam(0, ItemDBId); }
		void InItemUse(bool& ItemUse) { BindParam(1, ItemUse); }
	};

	// 인벤토리 아이템 초기화
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

	// 인벤토리 아이템 업데이트
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

	// GoldTable 생성
	class CDBGameServerGoldTableCreatePush : public CDBBind<2, 0>
	{
	public:
		CDBGameServerGoldTableCreatePush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGoldTableCreate(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
	};

	// GoldTable에 Gold 저장
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

	// GoldTable에 있는 Gold 긁어옴
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

	// InventoryTable에 있는 Item 모두 긁어옴
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

	// SkillTable에 있는 Skill 모두 긁어옴
	class CDBGameServerSkillGet : public CDBBind<2, 8>
	{
	public:
		CDBGameServerSkillGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetSkill(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutIsQuickSlotUse(bool& OutIsQuickSlotUse) { BindCol(0, OutIsQuickSlotUse); }
		void OutSkillLargeCategory(int8& SkillLargeCategory) { BindCol(1, SkillLargeCategory); }
		void OutSkillType(int16& SkillType) { BindCol(2, SkillType); }
		void OutSkillLevel(int8& SkillLevel) { BindCol(3, SkillLevel); }
		template<int8 Length> void OutSkillName(WCHAR(&SkillName)[Length]) { BindCol(4, SkillName); }
		void OutSkillCoolTime(int32& SkillCoolTime) { BindCol(5, SkillCoolTime); }
		void OutSkillCastingTime(int32& SkillCastingTime) { BindCol(6, SkillCastingTime); }
		template<int8 Length> void OutSkillThumbnailImagePath(WCHAR(&SkillThumbnailImagePath)[Length]) { BindCol(7, SkillThumbnailImagePath); }
	};

	// 스킬 테이블에 스킬 넣기
	class CDBGameServerSkillToSkillBox : public CDBBind<10, 0>
	{
	public:
		CDBGameServerSkillToSkillBox(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spSkillToSkillBox(?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(2, IsQuickSlotUse); }
		void InSkillLargeCategory(int8& SkillLargeCategory) { BindParam(3, SkillLargeCategory); };
		void InSkillType(int16& SkillType) { BindParam(4, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(5, SkillLevel); }
		void InSkillName(wstring& SkillName) { BindParam(6, SkillName.c_str()); }
		void InSkillCoolTime(int32& SkillCoolTime) { BindParam(7, SkillCoolTime); }
		void InSkillCastingTime(int32& SkillCastingTime) { BindParam(8, SkillCastingTime); }
		void InSkillThumbnailImagePath(wstring& SkillThumbnailImagePath) { BindParam(9, SkillThumbnailImagePath.c_str()); }
	};

	// QuickSlotBarSlot정보 새로 생성
	class CDBGameServerQuickSlotBarSlotCreate : public CDBBind<12, 0>
	{
	public:
		CDBGameServerQuickSlotBarSlotCreate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spQuickSlotBarSlotCreate(?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InQuickSlotKey(wstring& QuickSlotKey) { BindParam(4, QuickSlotKey.c_str()); }
		void InSkillLargeCategory(int8& SkillLargeCategory) { BindParam(5, SkillLargeCategory); };
		void InSkillType(int16& SkillType) { BindParam(6, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(7, SkillLevel); }
		void InSkillName(wstring& SkillName) { BindParam(8, SkillName.c_str()); }
		void InSkillCoolTime(int32& SkillCoolTime) { BindParam(9, SkillCoolTime); }
		void InSkillCastingTime(int32& SkillCastingTime) { BindParam(10, SkillCastingTime); }
		void InSkillThumbnailImagePath(wstring& SkillThumbnailImagePath) { BindParam(11, SkillThumbnailImagePath.c_str()); }
	};

	// QuickSlotBarSlot정보 업데이트 프로시저
	class CDBGameServerQuickSlotBarSlotUpdate : public CDBBind<12, 0>
	{
	public:
		CDBGameServerQuickSlotBarSlotUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spQuickSlotBarSlotUpdate(?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InQuickSlotKey(wstring& QuickSlotKey) { BindParam(4, QuickSlotKey.c_str()); }
		void InSkillLargeCategory(int8& SkillLargeCategory) { BindParam(5, SkillLargeCategory); };
		void InSkillType(int16& SkillType) { BindParam(6, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(7, SkillLevel); }
		void InSkillName(wstring& SkillName) { BindParam(8, SkillName.c_str()); }
		void InSkillCoolTime(int32& SkillCoolTime) { BindParam(9, SkillCoolTime); }
		void InSkillCastingTime(int32& SkillCastingTime) { BindParam(10, SkillCastingTime); }
		void InSkillThumbnailImagePath(wstring& SkillThumbnailImagePath) { BindParam(11, SkillThumbnailImagePath.c_str()); }
	};

	// QuickSlotBarTable에 있는 QuickSlotBar 정보 모두 긁어온다.
	class CDBGameServerQuickSlotBarGet : public CDBBind<2, 9>
	{
	public:
		CDBGameServerQuickSlotBarGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetQuickSlotBarSlot(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutQuickSlotBarIndex(int8& SlotBarIndex) { BindCol(0, SlotBarIndex); }
		void OutQuickSlotBarItemIndex(int8& SlotBarItemIndex) { BindCol(1, SlotBarItemIndex); }
		template<int8 Length> void OutQuickSlotKey(WCHAR(&QuickSlotKey)[Length]) { BindCol(2, QuickSlotKey); }
		void OutQuickSlotSkillLargeCategory(int8& SkillLargeCategory) { BindCol(3, SkillLargeCategory); }
		void OutQuickSlotSkillType(int16& SkillType) { BindCol(4, SkillType); }
		void OutQuickSlotSkillLevel(int8& SkillLevel) { BindCol(5, SkillLevel); }
		void OutQuickSlotSkillCoolTime(int32& SkillCoolTime) { BindCol(6, SkillCoolTime); }
		void OutQuickSlotSkillCastingTime(int32& SkillCastingTime) { BindCol(7, SkillCastingTime); }
		template<int8 Length> void OutQuickSlotSkillThumbnailImagePath(WCHAR(&SkillThumbnailImagePath)[Length]) { BindCol(8, SkillThumbnailImagePath); }
	};

	// Swap 요청한 아이템이 QuickSlot에 있는지 확인하고
	// 요청한 퀵슬롯 정보 반환
	class CDBGameServerQuickSlotCheck : public CDBBind<4, 8>
	{
	public:
		CDBGameServerQuickSlotCheck(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spQuickSlotCheck(?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }

		template<int8 Length> void OutQuickSlotKey(WCHAR(&QuickSlotKey)[Length]) { BindCol(0, QuickSlotKey); }
		void OutQuickSlotSkillLargeCategory(int8& SkillLargeCategory) { BindCol(1, SkillLargeCategory); }
		void OutQuickSlotSkillType(int16& SkillType) { BindCol(2, SkillType); }
		void OutQuickSlotSkillLevel(int8& SkillLevel) { BindCol(3, SkillLevel); }
		template<int8 Length> void OutQuickSlotSkillName(WCHAR(&QuickSlotSkillName)[Length]) { BindCol(4, QuickSlotSkillName); }
		void OutQuickSlotSkillCoolTime(int32& SkillCoolTime) { BindCol(5, SkillCoolTime); }
		void OutQuickSlotSkillCastingTime(int32& SkillCastingTime) { BindCol(6, SkillCastingTime); }
		template<int8 Length> void OutQuickSlotSkillThumbnailImagePath(WCHAR(&SkillThumbnailImagePath)[Length]) { BindCol(7, SkillThumbnailImagePath); }
	};

	// QuickSlot Swap
	class CDBGameServerQuickSlotSwap : public CDBBind<20, 0>
	{
	public:
		CDBGameServerQuickSlotSwap(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spQuickSlotSwap(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InAQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InAQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InASkillLargeCategory(int8& SkillLargeCategory) { BindParam(4, SkillLargeCategory); };
		void InAQuickSlotSkillType(int16& QuickSlotSkillType) { BindParam(5, QuickSlotSkillType); }
		void InAQuickSlotSkillLevel(int8& QuickSkillLevel) { BindParam(6, QuickSkillLevel); }
		void InAQuickSlotSKillName(wstring& QuickSlotSKillNam) { BindParam(7, QuickSlotSKillNam.c_str()); }
		void InAQuickSlotSkillCoolTime(int32& QuickSlotSkillCoolTime) { BindParam(8, QuickSlotSkillCoolTime); }
		void InAQuickSlotSkillCastingTime(int32& QuickSlotSkillCastingTime) { BindParam(9, QuickSlotSkillCastingTime); }
		void InAQuickSlotSKillImagePath(wstring& QuickSlotSKillImagePath) { BindParam(10, QuickSlotSKillImagePath.c_str()); }

		void InBQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(11, QuickSlotBarIndex); }
		void InBQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(12, QuickSlotBarSlotIndex); }
		void InBSkillLargeCategory(int8& SkillLargeCategory) { BindParam(13, SkillLargeCategory); };
		void InBQuickSlotSkillType(int16& QuickSlotSkillType) { BindParam(14, QuickSlotSkillType); }
		void InBQuickSlotSkillLevel(int8& QuickSkillLevel) { BindParam(15, QuickSkillLevel); }
		void InBQuickSlotSKillName(wstring& QuickSlotSKillNam) { BindParam(16, QuickSlotSKillNam.c_str()); }
		void InBQuickSlotSkillCoolTime(int32& QuickSlotSkillCoolTime) { BindParam(17, QuickSlotSkillCoolTime); }
		void InBQuickSlotSkillCastingTime(int32& QuickSlotSkillCastingTime) { BindParam(18, QuickSlotSkillCastingTime); }
		void InBQuickSlotSKillImagePath(wstring& QuickSlotSKillImagePath) { BindParam(19, QuickSlotSKillImagePath.c_str()); }
	};

	// 퀵슬롯 정보 초기화
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
}