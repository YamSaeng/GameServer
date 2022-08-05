#pragma once
#include "DBBind.h"

namespace SP
{
	class CDBAccountTokenGet : public CDBBind<1, 2>
	{
	public:
		CDBAccountTokenGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spIsToken(?)}") { }
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		
		void OutTokenTime(TIMESTAMP_STRUCT& TokenTime) { BindCol(0, TokenTime); }
		template<int8 Length> void OutToken(BYTE(&Token)[Length]) { BindCol(1, Token); }				
	};

	// AccountID를 기준으로 클라가 소유하고 있는 캐릭터를 찾는다.
	class CDBGameServerPlayersGet : public CDBBind<1, 28>
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
		void OutAutoRecoveryHPPercent(int16& AutoRecoveryHPPercent) { BindCol(11, AutoRecoveryHPPercent); }
		void OutAutoRecoveryMPPercent(int16& AutoRecoveryMPPercent) { BindCol(12, AutoRecoveryMPPercent); }
		void OutMinMeleeAttackDamage(int32& MinMeleeAttackDamage) { BindCol(13, MinMeleeAttackDamage); }
		void OutMaxMeleeAttackDamage(int32& MaxMeleeAttackDamage) { BindCol(14, MaxMeleeAttackDamage); }
		void OutMeleeAttackHitRate(int16& MeleeAttackHitRate) { BindCol(15, MeleeAttackHitRate); }
		void OutMagicDamage(int16& MagicDamage) { BindCol(16, MagicDamage); }
		void OutMagicHitRate(float& MagicHitRate) { BindCol(17, MagicHitRate); }
		void OutDefence(int32& Defence) { BindCol(18, Defence); }
		void OutEvasionRate(int16& EvasionRate) { BindCol(19, EvasionRate); }
		void OutMeleeCriticalPoint(int16& MeleeCriticalPoint) { BindCol(20, MeleeCriticalPoint); }
		void OutMagicCriticalPointt(int16& MagicCriticalPoint) { BindCol(21, MagicCriticalPoint); }
		void OutSpeed(float& Speed) { BindCol(22, Speed); }		
		void OutLastPositionY(int32& LastPositionY) { BindCol(23, LastPositionY); }
		void OutLastPositionX(int32& LastPositionX) { BindCol(24, LastPositionX); }
		void OutCurrentExperience(int64& CurrentExperience) { BindCol(25, CurrentExperience); }
		void OutRequireExperience(int64& RequireExperience) { BindCol(26, RequireExperience); }
		void OutTotalExperience(int64& TotalExperience) { BindCol(27, TotalExperience); }
	};

	// DB에 입력한 해당 캐릭터가 있는지 확인
	class CDBGameServerCharacterNameGet : public CDBBind<1, 0>
	{
	public:
		CDBGameServerCharacterNameGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetCharacterName(?)}") {}
		void InCharacterName(wstring& CharacterName) { BindParam(0, CharacterName.c_str()); }
	};

	// DB에 새로운 캐릭 저장
	class CDBGameServerCreateCharacterPush : public CDBBind<28, 0>
	{
	public:
		CDBGameServerCreateCharacterPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spNewCharacterPush(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
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
		void InAutoRecoveryHPPercent(int16& AutoRecoveryHPPercent) { BindParam(11, AutoRecoveryHPPercent); }
		void InAutoRecoveryMPPercent(int16& AutoRecoveryMPPercent) { BindParam(12, AutoRecoveryMPPercent); }
		void InMinMeleeAttackDamage(int32& MinMeleeAttackDamage) { BindParam(13, MinMeleeAttackDamage); }
		void InMaxMeleeAttackDamage(int32& MaxMeleeAttackDamage) { BindParam(14, MaxMeleeAttackDamage); }
		void InMeleeAttackHitRate(int16& MeleeAttackHitRate) { BindParam(15, MeleeAttackHitRate); }
		void InMagicDamage(int16& MagicDamage) { BindParam(16, MagicDamage); }
		void InMagicHitRate(float& MagicHitRate) { BindParam(17, MagicHitRate); }
		void InDefence(int32& Defence) { BindParam(18, Defence); }
		void InEvasionRate(int16& EvasionRate) { BindParam(19, EvasionRate); }
		void InMeleeCriticalPoint(int16& MeleeCriticalPoint) { BindParam(20, MeleeCriticalPoint); }
		void InMagicCriticalPoint(int16& MagicCriticalPoint) { BindParam(21, MagicCriticalPoint); }
		void InSpeed(float& Speed) { BindParam(22, Speed); }
		void InLastPositionY(int32& LastPositionY) { BindParam(23, LastPositionY); }
		void InLastPositionX(int32& LastPositionX) { BindParam(24, LastPositionX); }
		void InCurrentExperence(int64& CurrentExperence) { BindParam(25, CurrentExperence); }
		void InRequireExperience(int64& RequireExperience) { BindParam(26, RequireExperience); }
		void InTotalExperience(int64& CurrentExperence) { BindParam(27, CurrentExperence); }
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
		void InItemTileGridPositionX(int16& ItemTileGridPositionX) { BindParam(1, ItemTileGridPositionX); }
		void InItemTileGridPositionY(int16& ItemTileGridPositionY) { BindParam(2, ItemTileGridPositionY); }					
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(3, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(4, OwnerPlayerId); }
	};

	// InventoryTable에 새로운 Item 저장
	class CDBGameServerItemToInventoryPush : public CDBBind<18, 0>
	{
	public:
		CDBGameServerItemToInventoryPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemToInventorySave(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(0, IsQuickSlotUse); }
		void InItemRotated(bool& ItemRotated) { BindParam(1, ItemRotated); }
		void InItemWidth(int16& ItemWidth) { BindParam(2, ItemWidth); }
		void InItemHeight(int16& ItemHeight) { BindParam(3, ItemHeight); }
		void InItemTileGridPositionX(int16& InItemTileGridPositionX) { BindParam(4, InItemTileGridPositionX); }
		void InItemTileGridPositionY(int16& InItemTileGridPositionY) { BindParam(5, InItemTileGridPositionY); }
		void InItemLargeCategory(int8& ItemLargeCategory) { BindParam(6, ItemLargeCategory); }
		void InItemMediumCategory(int8& ItemMediumCategory) { BindParam(7, ItemMediumCategory); }
		void InItemSmallCategory(int16& ItemSmallCategory) { BindParam(8, ItemSmallCategory); }
		void InItemName(wstring& ItemName) { BindParam(9, ItemName.c_str()); }
		void InItemMinDamage(int32& ItemMinDamage) { BindParam(10, ItemMinDamage); }
		void InItemMaxDamage(int32& ItemMaxDamage) { BindParam(11, ItemMaxDamage); }
		void InItemDefence(int32& ItemDefence) { BindParam(12, ItemDefence); }
		void InItemMaxCount(int32& ItemMaxCount) { BindParam(13, ItemMaxCount); }
		void InItemCount(int16& ItemCount) { BindParam(14, ItemCount); }		
		void InIsEquipped(bool& IsEquipped) { BindParam(15, IsEquipped); }				
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(16, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(17, OwnerPlayerId); }
	};

	// ItemTable에 Item 개수 갱신
	class CDBGameServerItemRefreshPush : public CDBBind<6, 0>
	{
	public:
		CDBGameServerItemRefreshPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spItemRefreshCount(?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InItemType(int16& ItemType) { BindParam(2, ItemType); }
		void InCount(int16& Count) { BindParam(3, Count); }
		void InItemTileGridPositionX(int16& InItemTileGridPositionX) { BindParam(4, InItemTileGridPositionX); }
		void InItemTileGridPositionY(int16& InItemTileGridPositionY) { BindParam(5, InItemTileGridPositionY); }
	};

	// Swap 요청한 아이템이 Inventory에 있는지 확인하고
	// 요청한 아이템의 정보를 반환
	class CDBGameServerItemCheck : public CDBBind<4, 14>
	{
	public:
		CDBGameServerItemCheck(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventoryItemCheck(?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InItemTileGridPositionX(int16& TileGridPositionX) { BindParam(2, TileGridPositionX); }
		void InItemTileGridPositionY(int16& TileGridPositionY) { BindParam(3, TileGridPositionY); }

		void OutIsQuickSlotUse(bool& IsQuickSlotUse) { BindCol(0, IsQuickSlotUse); }
		void OutIsRotated(bool& IsRotated) { BindCol(1, IsRotated); }
		void OutItemWidth(int16& ItemWidth) { BindCol(2, ItemWidth); }
		void OutItemHeight(int16& ItemHeight) { BindCol(3, ItemHeight); }		
		void OutItemLargeCategory(int8& ItemLargeCategory) { BindCol(4, ItemLargeCategory); }
		void OutItemMediumCategory(int8& ItemMediumCategory) { BindCol(5, ItemMediumCategory); }
		void OutItemSmallCategory(int16& ItemSmallCategory) { BindCol(6, ItemSmallCategory); }
		template<int8 Length> void OutItemName(WCHAR(&ItemName)[Length]) { BindCol(7, ItemName); }
		void OutItemMinDamage(int32& ItemMinDamage) { BindCol(8, ItemMinDamage); }
		void OutItemMaxDamage(int32& ItemMaxDamage) { BindCol(9, ItemMaxDamage); }
		void OutItemDefence(int32& ItemDefence) { BindCol(10, ItemDefence); }
		void OutItemMaxCount(int32& ItemMaxCount) { BindCol(11, ItemMaxCount); }
		void OutItemCount(int16& ItemCount) { BindCol(12, ItemCount); }
		void OutItemIsEquipped(bool& ItemIsEquipped) { BindCol(13, ItemIsEquipped); }			
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
		void InItemTileGridPositionX(int16& TileGridPositionX) { BindParam(2, TileGridPositionX); }
		void InItemTileGridPositionY(int16& TileGridPositionY) { BindParam(3, TileGridPositionY); }
		void InItemName(wstring& ItemName) { BindParam(4, ItemName.c_str()); }		
	};

	// 인벤토리 아이템 업데이트 프로시저
	class CDBGameServerInventoryItemUpdate : public CDBBind<7, 0>
	{
	public:
		CDBGameServerInventoryItemUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventoryItemUpdate(?,?,?,?,?,?,?)}") {}
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(0, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(1, OwnerPlayerId); }
		void InItemRotated(bool& ItemRotated) { BindParam(2, ItemRotated); }
		void InItemTileGridPositionX(int16& TileGridPositionX) { BindParam(3, TileGridPositionX); }
		void InItemTileGridPositionY(int16& TileGridPositionY) { BindParam(4, TileGridPositionY); }
		void InItemCount(int16& ItemCount) { BindParam(5, ItemCount); }
		void InIsEquipped(bool& IsEquipped) { BindParam(6, IsEquipped); }
	};

	// 인벤토리 아이템 넣기 프로시저
	class CDBGameServerInventoryPlace : public CDBBind<18, 0>
	{
	public:
		CDBGameServerInventoryPlace(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventoryItemPlace(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(0, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(1, OwnerPlayerId); }
		void InIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(2, IsQuickSlotUse); }
		void InItemRotated(bool& ItemRotated) { BindParam(3, ItemRotated); }
		void InItemWidth(int16& ItemWidth) { BindParam(4, ItemWidth); }
		void InItemHeight(int16& ItemHeight) { BindParam(5, ItemHeight); }		
		void InItemTileGridPositionX(int16& TileGridPositionX) { BindParam(6, TileGridPositionX); }
		void InItemTileGridPositionY(int16& TileGridPositionY) { BindParam(7, TileGridPositionY); }
		void InItemLargeCategory(int8& ItemCategoryType) { BindParam(8, ItemCategoryType); }
		void InItemMediumCategory(int8& ItemMediumCategory) { BindParam(9, ItemMediumCategory); }
		void InItemSmallCategory(int16& ItemType) { BindParam(10, ItemType); }
		void InItemName(wstring& ItemName) { BindParam(11, ItemName.c_str()); }
		void InItemCount(int16& ItemCount) { BindParam(12, ItemCount); }
		void InIsEquipped(bool& IsEquipped) { BindParam(13, IsEquipped); }
		void InItemMinDamage(int32& ItemMinDamage) { BindParam(14, ItemMinDamage); }
		void InItemMaxDamage(int32& ItemMaxDamage) { BindParam(15, ItemMaxDamage); }
		void InItemDefence(int32& ItemDefence) { BindParam(16, ItemDefence); }
		void InItemMaxCount(int32& ItemMaxCount) { BindParam(17, ItemMaxCount); }				
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
	class CDBGameServerInventoryItemGet : public CDBBind<2, 15>
	{
	public:
		CDBGameServerInventoryItemGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetItemTableInfoToInventory(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutIsRotated(bool& IsRotated) { BindCol(0, IsRotated); }
		void OutItemWidth(int16& ItemWidth) { BindCol(1, ItemWidth); }
		void OutItemHeight(int16& ItemHeight) { BindCol(2, ItemHeight); }
		void OutItemLargeCategory(int8& ItemLargeCategory) { BindCol(3, ItemLargeCategory); }
		void OutItemMediumCategory(int8& ItemMediumCategory) { BindCol(4, ItemMediumCategory); }	
		void OutItemSmallCategory(int16& ItemSmallCategory) { BindCol(5, ItemSmallCategory); }
		template<int8 Length> void OutItemName(WCHAR(&ItemName)[Length]) { BindCol(6, ItemName); }
		void OutMinDamage(int32& MinDamage) { BindCol(7, MinDamage); }
		void OutMaxDamage(int32& MaxDamage) { BindCol(8, MaxDamage); }
		void OutDefence(int32& Defence) { BindCol(9, Defence); }
		void OutMaxCount(int32& MaxCount) { BindCol(10, MaxCount); }
		void OutItemCount(int16& itemCount) { BindCol(11, itemCount); }
		void OutItemTileGridPositionX(int16& TileGridPositionX) { BindCol(12, TileGridPositionX); }
		void OutItemTileGridPositionY(int16& TileGridPositionY) { BindCol(13, TileGridPositionY); }		
		void OutIsEquipped(bool& IsEquipped) { BindCol(14, IsEquipped); }				
	};

	// 스킬 테이블에 있는 스킬 모두 긁어옴
	class CDBGameServerSkillGet : public CDBBind<2, 6>
	{
	public:
		CDBGameServerSkillGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetSkill(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutIsSkillLearn(bool& OutIsSkillLearn) { BindCol(0, OutIsSkillLearn); }
		void OutIsQuickSlotUse(bool& OutIsQuickSlotUse) { BindCol(1, OutIsQuickSlotUse); }
		void OutSkillLargeCategory(int8& SkillLargeCategory) { BindCol(2, SkillLargeCategory); }
		void OutSkillMediumCategory(int8& SkillMediumCategory) { BindCol(3, SkillMediumCategory); }
		void OutSkillType(int16& SkillType) { BindCol(4, SkillType); }
		void OutSkillLevel(int8& SkillLevel) { BindCol(5, SkillLevel); }		
	};

	// 스킬 테이블에 스킬 넣기
	class CDBGameServerSkillToSkillBox : public CDBBind<8, 0>
	{
	public:
		CDBGameServerSkillToSkillBox(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spSkillToSkillBox(?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InIsSkillLearn(bool& IsSkillLearn) { BindParam(2, IsSkillLearn); }
		void InIsQuickSlotUse(bool& IsQuickSlotUse) { BindParam(3, IsQuickSlotUse); }
		void InSkillLargeCategory(int8& SkillLargeCategory) { BindParam(4, SkillLargeCategory); };
		void InSkillMediumCategory(int8& SkillMediumCategory) { BindParam(5, SkillMediumCategory); };
		void InSkillType(int16& SkillType) { BindParam(6, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(7, SkillLevel); }		
	};

	// QuickSlotBarSlot정보 새로 생성
	class CDBGameServerQuickSlotBarSlotCreate : public CDBBind<13, 0>
	{
	public:
		CDBGameServerQuickSlotBarSlotCreate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spQuickSlotBarSlotCreate(?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InQuickSlotKey(int16& QuickSlotKey) { BindParam(4, QuickSlotKey); }
		void InSkillLargeCategory(int8& SkillLargeCategory) { BindParam(5, SkillLargeCategory); };
		void InSkillMediumCategory(int8& SkillMediumCategory) { BindParam(6, SkillMediumCategory); };
		void InSkillType(int16& SkillType) { BindParam(7, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(8, SkillLevel); }		
		void InItemLargeCategory(int8& ItemLargeCategory) { BindParam(9, ItemLargeCategory); }
		void InItemMediumCategory(int8& ItemMediumCategory) { BindParam(10, ItemMediumCategory); }
		void InItemSmallCategory(int16& ItemSmallCategory) { BindParam(11, ItemSmallCategory); }
		void InItemCount(int16& ItemCount) { BindParam(12, ItemCount); }
	};

	// QuickSlotBarSlot정보 업데이트 프로시저
	class CDBGameServerQuickSlotBarSlotUpdate : public CDBBind<13, 0>
	{
	public:
		CDBGameServerQuickSlotBarSlotUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spQuickSlotBarSlotUpdate(?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InQuickSlotKey(int16& QuickSlotKey) { BindParam(4, QuickSlotKey); }
		void InSkillLargeCategory(int8& SkillLargeCategory) { BindParam(5, SkillLargeCategory); };
		void InSkillMediumCategory(int8& SkillMediumCategory) { BindParam(6, SkillMediumCategory); };
		void InSkillType(int16& SkillType) { BindParam(7, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(8, SkillLevel); }		
		void InItemLargeCategory(int8& ItemLargeCategory) { BindParam(9, ItemLargeCategory); }
		void InItemMediumCategory(int8& ItemMediumCategory) { BindParam(10, ItemMediumCategory); }
		void InItemSmallCategory(int16& ItemSmallCategory) { BindParam(11, ItemSmallCategory); }
		void InItemCount(int16& ItemCount) { BindParam(12, ItemCount); }
	};

	// QuickSlotBarTable에 있는 QuickSlotBar 정보 모두 긁어온다.
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

	// Swap 요청한 아이템이 QuickSlot에 있는지 확인하고
	// 요청한 퀵슬롯 정보 반환
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

	// 퀵슬롯 정보 초기화
	class CDBGameServerQuickSlotInit : public CDBBind<4, 0>
	{
	public:
		CDBGameServerQuickSlotInit(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spQuickSlotInit(?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }		
	};

	// 접속 종료시 플레이어 정보 DB에 기록
	class CDBGameServerLeavePlayerStatInfoSave : public CDBBind<23, 0>
	{
	public:
		CDBGameServerLeavePlayerStatInfoSave(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spPlayerLeaveInfoSave(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}"){}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InLevel(int32& Level) { BindParam(2, Level); }
		void InMaxHP(int32& MaxHP) { BindParam(3, MaxHP); }
		void InMaxMP(int32& MaxMP) { BindParam(4, MaxMP); }
		void InMaxDP(int32& MaxDP) { BindParam(5, MaxDP); }
		void InAutoRecoveryHPPercent(int16& AutoRecoveryHPPercent) { BindParam(6, AutoRecoveryHPPercent); }
		void InAutoRecoveryMPPercent(int16& AutoRecoveryMPPercent) { BindParam(7, AutoRecoveryMPPercent); }
		void InMinMeleeAttackDamage(int32& MinMeleeAttackDamage) { BindParam(8, MinMeleeAttackDamage); }
		void InMaxMeleeAttackDamage(int32& MaxMeleeAttackDamage) { BindParam(9, MaxMeleeAttackDamage); }
		void InMeleeAttackHitRate(int16& MeleeAttackHitRate) { BindParam(10, MeleeAttackHitRate); }
		void InMagicDamage(int16& MagicDamage) { BindParam(11, MagicDamage); }
		void InMagicHitRate(float& MagicHitRate) { BindParam(12, MagicHitRate); }
		void InDefence(int32& Defence) { BindParam(13, Defence); }
		void InEvasionRate(int16& EvasionRate) { BindParam(14, EvasionRate); }
		void InMeleeCriticalPoint(int16& MeleeCriticalPoint) { BindParam(15, MeleeCriticalPoint); }
		void InMagicCriticalPoint(int16& MagicCriticalPoint) { BindParam(16, MagicCriticalPoint); }
		void InSpeed(float& Speed) { BindParam(17, Speed); }
		void InLastPositionY(int32& LastPositionY) { BindParam(18, LastPositionY); }
		void InLastPositionX(int32& LastPositionX) { BindParam(19, LastPositionX); }
		void InCurrentExperience(int64& CurrentExperience) { BindParam(20, CurrentExperience); }
		void InRequireExperience(int64& RequireExperience) { BindParam(21, RequireExperience); }
		void InTotalExperience(int64& TotalExperience) { BindParam(22, TotalExperience); }
	};

	// 타일 맵 정보 할당 및 해제
	class CDBGameServerTileMapInfoSave : public CDBBind<6, 0>
	{
	public:
		CDBGameServerTileMapInfoSave(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spTileMapInfoSave(?,?,?,?,?,?)}") {}
		void InMapID(int16& MapID) { BindParam(0, MapID); }
		void InMapTileAllocFree(bool& MapTileAllocFree) { BindParam(1, MapTileAllocFree); }
		void InMapTileAccountID(int64& MapTileAccountID) { BindParam(2, MapTileAccountID); }
		void InMapTilePlayerID(int64& MapTilePlayerID) { BindParam(3, MapTilePlayerID); }
		void InMapTilePositionX(int32& MapTilePositionX) { BindParam(4, MapTilePositionX); }
		void InMapTilePositionY(int32& MapTilePositionY) { BindParam(5, MapTilePositionY); }
	};

	// 타일 맵 할당 및 해제 정보 가져오기
	class CDBGameServerGetTileMapInfoAllocFree : public CDBBind<1, 5>
	{
	public:
		CDBGameServerGetTileMapInfoAllocFree(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spGetTileMapInfoAllocFree(?)}") {}
		void InMapID(int16& MapID) { BindParam(0, MapID); }
		void OutMapTileAllocFree(bool& MapTileAllocFree) { BindCol(0, MapTileAllocFree); }
		void OutMapTileAccountID(int64& MapTileAccountID) { BindCol(1, MapTileAccountID); }
		void OutMapTilePlayerID(int64& MapTilePlayerID) { BindCol(2, MapTilePlayerID); }
		void OutMapTilePositionX(int32& MapTilePositionX) { BindCol(3, MapTilePositionX); }
		void OutMapTilePositionY(int32& MapTilePositionY) { BindCol(4, MapTilePositionY); }
	};
}