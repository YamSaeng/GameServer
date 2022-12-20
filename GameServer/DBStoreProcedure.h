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
	class CDBGameServerPlayersGet : public CDBBind<1, 29>
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
		void OutSkillMaxPoint(int8& SkillMaxPoint) { BindCol(28, SkillMaxPoint); }
	};

	// DB에 입력한 해당 캐릭터가 있는지 확인
	class CDBGameServerCharacterNameGet : public CDBBind<1, 0>
	{
	public:
		CDBGameServerCharacterNameGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetCharacterName(?)}") {}
		void InCharacterName(wstring& CharacterName) { BindParam(0, CharacterName.c_str()); }
	};

	// DB에 새로운 캐릭 저장
	class CDBGameServerCreateCharacterPush : public CDBBind<29, 0>
	{
	public:
		CDBGameServerCreateCharacterPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spNewCharacterPush(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
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
		void InSkillMaxPoint(int8& SkillMaxPoint) { BindParam(28, SkillMaxPoint); }
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
	class CDBGameServerItemCreateToInventory : public CDBBind<4, 0>
	{
	public:
		CDBGameServerItemCreateToInventory(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemToInventoryInit(?,?,?,?)}") {}		
		void InItemTileGridPositionX(int16& ItemTileGridPositionX) { BindParam(0, ItemTileGridPositionX); }
		void InItemTileGridPositionY(int16& ItemTileGridPositionY) { BindParam(1, ItemTileGridPositionY); }					
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(2, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(3, OwnerPlayerId); }
	};	

	// 가방 초기화
	class CDBGameServerInventoryAllSlotInit : public CDBBind<4, 0>
	{
	public:
		CDBGameServerInventoryAllSlotInit(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventoryInit(?,?,?,?)}") {}
		void InOwnerAccountID(int64& OwnerAccountID) { BindParam(0, OwnerAccountID); }
		void InOwnerPlayerID(int64& OwnerAccountID) { BindParam(1, OwnerAccountID); }
		void InInventoryWidth(int32& InventoryWidth) { BindParam(2, InventoryWidth); }
		void InInventoryHeight(int32& InventoryHeight) { BindParam(3, InventoryHeight); }
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

	// 인벤토리 아이템 넣기 프로시저
	class CDBGameServerInventoryPlace : public CDBBind<11, 0>
	{
	public:
		CDBGameServerInventoryPlace(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventoryItemPlace(?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(0, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(1, OwnerPlayerId); }		
		void InIsEquipped(bool& IsEquipped) { BindParam(2, IsEquipped); }		
		void InItemTileGridPositionX(int16& TileGridPositionX) { BindParam(3, TileGridPositionX); }
		void InItemTileGridPositionY(int16& TileGridPositionY) { BindParam(4, TileGridPositionY); }
		void InItemLargeCategory(int8& ItemCategoryType) { BindParam(5, ItemCategoryType); }
		void InItemMediumCategory(int8& ItemMediumCategory) { BindParam(6, ItemMediumCategory); }
		void InItemSmallCategory(int16& ItemType) { BindParam(7, ItemType); }		
		void InItemCount(int16& ItemCount) { BindParam(8, ItemCount); }				
		void InItemDurability(int32& ItemDurability) { BindParam(9, ItemDurability); }
		void InItemEnchantPoint(int8& ItemEnchantPoint) { BindParam(10, ItemEnchantPoint); }
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
	class CDBGameServerInventoryItemGet : public CDBBind<2, 9>
	{
	public:
		CDBGameServerInventoryItemGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetItemTableInfoToInventory(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
			
		void OutItemIsEquipped(bool& ItemIsEquipped) { BindCol(0, ItemIsEquipped); }
		void OutItemTileGridPositionX(int16& TileGridPositionX) { BindCol(1, TileGridPositionX); }
		void OutItemTileGridPositionY(int16& TileGridPositionY) { BindCol(2, TileGridPositionY); }
		void OutItemLargeCategory(int8& ItemLargeCategory) { BindCol(3, ItemLargeCategory); }
		void OutItemMediumCategory(int8& ItemMediumCategory) { BindCol(4, ItemMediumCategory); }	
		void OutItemSmallCategory(int16& ItemSmallCategory) { BindCol(5, ItemSmallCategory); }				
		void OutItemCount(int16& itemCount) { BindCol(6, itemCount); }		
		void OutItemDurability(int32& ItemDurability) { BindCol(7, ItemDurability); }
		void OutItemEnchantPoint(int8& ItemEnchantPoint) { BindCol(8, ItemEnchantPoint); }
	};

	// 캐릭터 새로 생성시 기본적인 정보 셋팅
	class CDBGameCharacterDefaultInfoCreate : public CDBBind<2, 0>
	{
	public:
		CDBGameCharacterDefaultInfoCreate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spNewGameCharacterDefaultInfoCreate(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
	};	

	// 캐릭터 스킬 특성 정보 가져오기
	class CDBGameServerSkillCharacteristicGet : public CDBBind<2, 2>
	{
	public:
		CDBGameServerSkillCharacteristicGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetSkillCharacteristicGet(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		
		void OutSkillCharacteristicIndex(int8& SkillCharacteristicIndex) { BindCol(0, SkillCharacteristicIndex); }
		void OutSKillCharacteristicType(int8& SkillCharacteristicType) { BindCol(1, SkillCharacteristicType); }
	};

	// 캐릭터 스킬 특성 정보 업데이트 하기
	class CDBGameServerSkillCharacteristicUpdate : public CDBBind<4, 0>
	{
	public:
		CDBGameServerSkillCharacteristicUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetSkillCharacteristicUpdate(?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InSkillCharacteristicIndex(int8& SkillCharacteristicIndex) { BindParam(2, SkillCharacteristicIndex); }
		void InSkillCharacteristicType(int8& SkillCharacteristicType) { BindParam(3, SkillCharacteristicType); }
	};

	// 스킬 특성 타입에 따라 스킬 테이블에 있는 스킬을 가져옴
	class CDBGameServerSkillGet : public CDBBind<3, 3>
	{
	public:
		CDBGameServerSkillGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetSkill(?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }		
		void InCharacteristicType(int8& CharacteristicType) { BindParam(2, CharacteristicType); }
		
		void OutSkillLearn(bool& IsSkillLearn) { BindCol(0, IsSkillLearn); }
		void OutSkillType(int16& SkillType) { BindCol(1, SkillType); }
		void OutSkillLevel(int8& SkillLevel) { BindCol(2, SkillLevel); }		
	};

	// 스킬 테이블에 스킬 넣기
	class CDBGameServerSkillToSkillBox : public CDBBind<6, 0>
	{
	public:
		CDBGameServerSkillToSkillBox(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spSkillToSkillBox(?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }		
		void InSkillLearn(bool& SkillLearn) { BindParam(2, SkillLearn); }
		void InCharacteristicType(int8& CharacteristicType) { BindParam(3, CharacteristicType); }
		void InSkillType(int16& SkillType) { BindParam(4, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(5, SkillLevel); }		
	};

	// QuickSlotBarSlot정보 새로 생성
	class CDBGameServerQuickSlotBarSlotCreate : public CDBBind<5, 0>
	{
	public:
		CDBGameServerQuickSlotBarSlotCreate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.CDBGameServerQuickSlotBarSlotCreate(?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InQuickSlotKey(int16& QuickSlotKey) { BindParam(4, QuickSlotKey); }
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
		void InQuickSlotKey(int16& QuickSlotKey) { BindParam(4, QuickSlotKey); }		
		void InCharacteristicType(int8& ChracteristicType) { BindParam(5, ChracteristicType); }
		void InSkillType(int16& SkillType) { BindParam(6, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(7, SkillLevel); }		
		void InItemLargeCategory(int8& ItemLargeCategory) { BindParam(8, ItemLargeCategory); }
		void InItemMediumCategory(int8& ItemMediumCategory) { BindParam(9, ItemMediumCategory); }
		void InItemSmallCategory(int16& ItemSmallCategory) { BindParam(10, ItemSmallCategory); }
		void InItemCount(int16& ItemCount) { BindParam(11, ItemCount); }
	};

	// QuickSlotBarTable에 있는 QuickSlotBar 정보 모두 긁어온다.
	class CDBGameServerQuickSlotBarGet : public CDBBind<2, 10>
	{
	public:
		CDBGameServerQuickSlotBarGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetQuickSlotBarSlot(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutQuickSlotBarIndex(int8& SlotBarIndex) { BindCol(0, SlotBarIndex); }
		void OutQuickSlotBarItemIndex(int8& SlotBarItemIndex) { BindCol(1, SlotBarItemIndex); }
		void OutQuickSlotKey(int16& QuickSlotKey) { BindCol(2, QuickSlotKey); }		
		void OutQuickSlotCharacteristicType(int8& CharacteristicType) { BindCol(3, CharacteristicType); }
		void OutQuickSlotSkillType(int16& SkillType) { BindCol(4, SkillType); }
		void OutQuickSlotSkillLevel(int8& SkillLevel) { BindCol(5, SkillLevel); }		
		void OutQuickSlotItemLargeCategory(int8& ItemLargeCategory) { BindCol(6, ItemLargeCategory); }
		void OutQuickSlotItemMediumCategory(int8& ItemMediumCategory) { BindCol(7, ItemMediumCategory); }
		void OutQuickSlotItemSmallCategory(int16& ItemSmallCategory) { BindCol(8, ItemSmallCategory); }
		void OutQuickSlotItemCount(int16& ItemCount) { BindCol(9, ItemCount); }
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
	class CDBGameServerLeavePlayerStatInfoSave : public CDBBind<24, 0>
	{
	public:
		CDBGameServerLeavePlayerStatInfoSave(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spPlayerLeaveInfoSave(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}"){}
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
		void InSkillMaxPoint(int8& SkillMaxPoint) { BindParam(23, SkillMaxPoint); }
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

	// 장비 테이블 빈 껍데기 채우기
	class CDBGameServerInitEquipment : public CDBBind<3, 0>
	{
	public:
		CDBGameServerInitEquipment(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spEquipmentItemInit(?,?,?)}") {}
		void InAccountDBID(int64& AccountDBID) { BindParam(0, AccountDBID); }
		void InPlayerDBID(int64& PlayerDBID) { BindParam(1, PlayerDBID); }
		void InEquipmentParts(int8& EquipmentParts) { BindParam(2, EquipmentParts); }
	};

	// 장비 착용 DB에 저장하기
	class CDBGameServerOnEquipment : public CDBBind<8, 0>
	{
	public:
		CDBGameServerOnEquipment(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spOnEquipment(?,?,?,?,?,?,?,?)}") {}
		void InAccountDBID(int64& AccountDBID) { BindParam(0, AccountDBID); }
		void InPlayerDBID(int64& AccountDBID) { BindParam(1, AccountDBID); }
		void InEquipmentParts(int8& EquipmentParts) { BindParam(2, EquipmentParts); }
		void InEquipmentLargeCategory(int8& EquipmentLargeCategory) { BindParam(3, EquipmentLargeCategory); }
		void InEquipmentMediumCategory(int8& EquipmentMediumCategory) { BindParam(4, EquipmentMediumCategory); }
		void InEquipmentSmallCategory(int16& EquipmentSmallCategory) { BindParam(5, EquipmentSmallCategory); }
		void InEquipmentDurability(int32& EquipmentDurability) { BindParam(6, EquipmentDurability); }
		void InEquipmentEnchantPoint(int8& EquipmentEnchantPoint) { BindParam(7, EquipmentEnchantPoint); }
	};

	// 장비 착용 해제 DB에 저장하기
	class CDBGameServerOffEquipment : public CDBBind<3, 0>
	{
	public:
		CDBGameServerOffEquipment(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spOffEquipment(?,?,?)}") {}

		void InAccountDBID(int64& AccountDBID) { BindParam(0, AccountDBID); }
		void InPlayerDBID(int64& PlayerDBID) { BindParam(1, PlayerDBID); }
		void InEquipmentParts(int8& EquipmentParts) { BindParam(2, EquipmentParts); }
	};

	// 장비 테이블에서 정보 가져오기
	class CDBGameServerGetEquipment : public CDBBind<2, 6>
	{
	public:
		CDBGameServerGetEquipment(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spGetEquipemt(?,?)}") {}

		void InAccountDBID(int64& AccountDBID) { BindParam(0, AccountDBID); }
		void InPlayerDBID(int64& PlayerDBID) { BindParam(1, PlayerDBID); }

		void OutEquipmentParts(int8& EquipmentParts) { BindCol(0, EquipmentParts); }
		void OutEquipmentLargeCategory(int8& EquipmentLargeCategory) { BindCol(1, EquipmentLargeCategory); }
		void OutEquipmentMediumCateogry(int8& EquipmentMediumCategory){ BindCol(2, EquipmentMediumCategory); }
		void OutEquipmentSmallCategory(int16& EquipmentSmallCategory) { BindCol(3, EquipmentSmallCategory); }
		void OutEquipmentDurability(int32& EquipmentDurability) { BindCol(4, EquipmentDurability); }
		void OutEquipmentEnchantPoint(int8& EquipmentEnchantPoint) { BindCol(5, EquipmentEnchantPoint); }
	};
}