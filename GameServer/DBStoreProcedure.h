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
	class CDBGameServerPlayersGet : public CDBBind<1, 32>
	{
	public:
		CDBGameServerPlayersGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetPlayers(?)}") { }
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }

		void OutPlayerDBID(int64& PlayerDBID) { BindCol(0, PlayerDBID); }
		template<int32 N> void OutPlayerName(WCHAR(&PlayerName)[N]) { BindCol(1, PlayerName); }		
		void OutPlayerIndex(int8& PlayerIndex) { BindCol(2, PlayerIndex); }
		void OutLevel(int32& Level) { BindCol(3, Level); }
		void OutStr(int32& Str) { BindCol(4, Str); }
		void OutDex(int32& Dex) { BindCol(5, Dex); }
		void OutInt(int32& Int) { BindCol(6, Int); }
		void OutLuck(int32& Luck) { BindCol(7, Luck); }
		void OutCurrentHP(int32& CurrentHP) { BindCol(8, CurrentHP); }
		void OutMaxHP(int32& MaxHP) { BindCol(9, MaxHP); }
		void OutCurrentMP(int32& CurrentMP) { BindCol(10, CurrentMP); }
		void OutMaxMP(int32& MaxMP) { BindCol(11, MaxMP); }
		void OutAutoRecoveryHPPercent(int16& AutoRecoveryHPPercent) { BindCol(12, AutoRecoveryHPPercent); }
		void OutAutoRecoveryMPPercent(int16& AutoRecoveryMPPercent) { BindCol(13, AutoRecoveryMPPercent); }
		void OutMinAttackPoint(int32& MinAttackPoint) { BindCol(14, MinAttackPoint); }
		void OutMaxAttackPoint(int32& MaxAttackPoint) { BindCol(15, MaxAttackPoint); }
		void OutSpiritPoint(int16& SpiritPoint) { BindCol(16, SpiritPoint); }
		void OutAttackHitRate(int16& MeleeAttackHitRate) { BindCol(17, MeleeAttackHitRate); }		
		void OutSpellCastingRate(float& CastingRate) { BindCol(18, CastingRate); }
		void OutDefence(int32& Defence) { BindCol(19, Defence); }
		void OutEvasionRate(int16& EvasionRate) { BindCol(20, EvasionRate); }
		void OutAttackCriticalPoint(int16& AttackCriticalPoint) { BindCol(21, AttackCriticalPoint); }
		void OutSpellCriticalPoint(int16& SpellCriticalPoint) { BindCol(22, SpellCriticalPoint); }
		void OutAttackCriticalResistance(int16& AttackCriticalResistance) { BindCol(23, AttackCriticalResistance); }
		void OutStatusAbnormalResistance(int16& StatusAbnormalResistance) { BindCol(24, StatusAbnormalResistance); }
		void OutSpeed(float& Speed) { BindCol(25, Speed); }		
		void OutLastPositionY(int32& LastPositionY) { BindCol(26, LastPositionY); }
		void OutLastPositionX(int32& LastPositionX) { BindCol(27, LastPositionX); }
		void OutCurrentExperience(int64& CurrentExperience) { BindCol(28, CurrentExperience); }
		void OutRequireExperience(int64& RequireExperience) { BindCol(29, RequireExperience); }
		void OutTotalExperience(int64& TotalExperience) { BindCol(30, TotalExperience); }		
		void OutSkillMaxPoint(int8& SkillMaxPoint) { BindCol(31, SkillMaxPoint); }
	};

	// DB에 입력한 해당 캐릭터가 있는지 확인
	class CDBGameServerCharacterNameGet : public CDBBind<1, 0>
	{
	public:
		CDBGameServerCharacterNameGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetCharacterName(?)}") {}
		void InCharacterName(wstring& CharacterName) { BindParam(0, CharacterName.c_str()); }
	};

	// DB에 새로운 캐릭 저장
	class CDBGameServerCreateCharacterPush : public CDBBind<32, 0>
	{
	public:
		CDBGameServerCreateCharacterPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spNewCharacterPush(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		void InPlayerName(wstring& PlayerName) { BindParam(1, PlayerName.c_str()); }		
		void InPlayerIndex(int8& PlayerIndex) { BindParam(2, PlayerIndex); }
		void InLevel(int8& Level) { BindParam(3, Level); }
		void InStr(int32& Str) { BindParam(4, Str); }
		void InDex(int32& Dex) { BindParam(5, Dex); }
		void InInt(int32& Int) { BindParam(6, Int); }
		void InLuck(int32& Luck) { BindParam(7, Luck); }
		void InCurrentHP(int32& CurrentHP) { BindParam(8, CurrentHP); }
		void InMaxHP(int32& MaxHP) { BindParam(9, MaxHP); }
		void InCurrentMP(int32& CurrentMP) { BindParam(10, CurrentMP); }
		void InMaxMP(int32& MaxMP) { BindParam(11, MaxMP); }
		void InAutoRecoveryHPPercent(int16& AutoRecoveryHPPercent) { BindParam(12, AutoRecoveryHPPercent); }
		void InAutoRecoveryMPPercent(int16& AutoRecoveryMPPercent) { BindParam(13, AutoRecoveryMPPercent); }
		void InMinAttackPoint(int32& MinAttackPoint) { BindParam(14, MinAttackPoint); }
		void InMaxAttackPoint(int32& MaxAttackPoint) { BindParam(15, MaxAttackPoint); }
		void InSpiritPoint(int16& SpiritPoint) { BindParam(16, SpiritPoint); }
		void InAttackHitRate(int16& AttackHitRate) { BindParam(17, AttackHitRate); }		
		void InSpellCastingRate(float& SpellCastingRate) { BindParam(18, SpellCastingRate); }
		void InDefence(int32& Defence) { BindParam(19, Defence); }
		void InEvasionRate(int16& EvasionRate) { BindParam(20, EvasionRate); }
		void InAttackCriticalPoint(int16& AttackCriticalPoint) { BindParam(21, AttackCriticalPoint); }
		void InSpellCriticalPoint(int16& SpellCriticalPoint) { BindParam(22, SpellCriticalPoint); }
		void InAttackCriticalResistance(int16& AttackCriticalResistance) { BindParam(23, AttackCriticalResistance); }
		void InSpellCriticalResistance(int16& SpellCriticalResistance) { BindParam(24, SpellCriticalResistance); }
		void InSpeed(float& Speed) { BindParam(25, Speed); }
		void InLastPositionY(int32& LastPositionY) { BindParam(26, LastPositionY); }
		void InLastPositionX(int32& LastPositionX) { BindParam(27, LastPositionX); }
		void InCurrentExperence(int64& CurrentExperence) { BindParam(28, CurrentExperence); }
		void InRequireExperience(int64& RequireExperience) { BindParam(29, RequireExperience); }
		void InTotalExperience(int64& CurrentExperence) { BindParam(30, CurrentExperence); }		
		void InSkillMaxPoint(int8& SkillMaxPoint) { BindParam(31, SkillMaxPoint); }
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
	class CDBGameServerInventoryPlace : public CDBBind<10, 0>
	{
	public:
		CDBGameServerInventoryPlace(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spInventoryItemPlace(?,?,?,?,?,?,?,?,?,?)}") {}
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(0, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(1, OwnerPlayerId); }		
		void InIsEquipped(bool& IsEquipped) { BindParam(2, IsEquipped); }		
		void InItemTileGridPositionX(int16& TileGridPositionX) { BindParam(3, TileGridPositionX); }
		void InItemTileGridPositionY(int16& TileGridPositionY) { BindParam(4, TileGridPositionY); }		
		void InItemSmallCategory(int16& ItemType) { BindParam(5, ItemType); }		
		void InItemCount(int16& ItemCount) { BindParam(6, ItemCount); }				
		void InItemIsSearching(bool& IsSearching) { BindParam(7, IsSearching); }
		void InItemDurability(int32& ItemDurability) { BindParam(8, ItemDurability); }
		void InItemEnchantPoint(int8& ItemEnchantPoint) { BindParam(9, ItemEnchantPoint); }
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
	class CDBGameServerGoldPush : public CDBBind<3, 0>
	{
	public:
		CDBGameServerGoldPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGoldSave(?,?,?)}") {}
		void InAccoountId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InCoin(int64& Coin) { BindParam(2, Coin); }
	};

	// GoldTable에 있는 Gold 긁어옴
	class CDBGameServerGoldGet : public CDBBind<2, 1>
	{
	public:
		CDBGameServerGoldGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetGoldTableInfoToInventory(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutCoin(int64& Coin) { BindCol(0, Coin); }		
	};

	// InventoryTable에 있는 Item 모두 긁어옴
	class CDBGameServerInventoryItemGet : public CDBBind<2, 8>
	{
	public:
		CDBGameServerInventoryItemGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetItemTableInfoToInventory(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
			
		void OutItemIsEquipped(bool& ItemIsEquipped) { BindCol(0, ItemIsEquipped); }
		void OutItemTileGridPositionX(int16& TileGridPositionX) { BindCol(1, TileGridPositionX); }
		void OutItemTileGridPositionY(int16& TileGridPositionY) { BindCol(2, TileGridPositionY); }		
		void OutItemSmallCategory(int16& ItemSmallCategory) { BindCol(3, ItemSmallCategory); }				
		void OutItemCount(int16& itemCount) { BindCol(4, itemCount); }		
		void OutItemIsSearching(bool& IsSearching) { BindCol(5, IsSearching); }
		void OutItemDurability(int32& ItemDurability) { BindCol(6, ItemDurability); }
		void OutItemEnchantPoint(int8& ItemEnchantPoint) { BindCol(7, ItemEnchantPoint); }
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
	class CDBGameServerSkillCharacteristicGet : public CDBBind<2, 1>
	{
	public:
		CDBGameServerSkillCharacteristicGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetSkillCharacteristicGet(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }		
		
		void OutSKillCharacteristicType(int8& SkillCharacteristicType) { BindCol(0, SkillCharacteristicType); }
	};

	// 캐릭터 스킬 특성 정보 업데이트 하기
	class CDBGameServerSkillCharacteristicUpdate : public CDBBind<3, 0>
	{
	public:
		CDBGameServerSkillCharacteristicUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetSkillCharacteristicUpdate(?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }		
		void InSkillCharacteristicType(int8& SkillCharacteristicType) { BindParam(2, SkillCharacteristicType); }
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

	// QuickSlotBarSlot정보 업데이트 프로시저
	class CDBGameServerQuickSlotBarSlotUpdate : public CDBBind<9, 0>
	{
	public:
		CDBGameServerQuickSlotBarSlotUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spQuickSlotBarSlotUpdate(?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }		
		void InCharacteristicType(int8& ChracteristicType) { BindParam(4, ChracteristicType); }
		void InSkillType(int16& SkillType) { BindParam(5, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(6, SkillLevel); }				
		void InItemSmallCategory(int16& ItemSmallCategory) { BindParam(7, ItemSmallCategory); }
		void InItemCount(int16& ItemCount) { BindParam(8, ItemCount); }
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
		void OutQuickSlotCharacteristicType(int8& CharacteristicType) { BindCol(2, CharacteristicType); }
		void OutQuickSlotSkillType(int16& SkillType) { BindCol(3, SkillType); }
		void OutQuickSlotSkillLevel(int8& SkillLevel) { BindCol(4, SkillLevel); }		
		void OutQuickSlotItemSmallCategory(int16& ItemSmallCategory) { BindCol(5, ItemSmallCategory); }
		void OutQuickSlotItemCount(int16& ItemCount) { BindCol(6, ItemCount); }
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

	// 단축키 정보 가져옴
	class CDBGameSerGetQuickSlotKey : public CDBBind<2, 2>
	{
	public:
		CDBGameSerGetQuickSlotKey(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spGetQuickSlotKey(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void OutQuickSlotKey(int16& QuickSlotKey) { BindCol(0, QuickSlotKey); }
		void OutQuickSlotKeyCode(int16& QuickSlotKeyCode) { BindCol(1, QuickSlotKeyCode); }
	};

	// 접속 종료시 플레이어 정보 DB에 기록
	class CDBGameServerLeavePlayerStatInfoSave : public CDBBind<29, 0>
	{
	public:
		CDBGameServerLeavePlayerStatInfoSave(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spPlayerLeaveInfoSave(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}"){}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InLevel(int8& Level) { BindParam(2, Level); }
		void InStr(int32& Str) { BindParam(3, Str); }
		void InDex(int32& Dex) { BindParam(4, Dex); }
		void InInt(int32& Int) { BindParam(5, Int); }
		void InLuck(int32& Luck) { BindParam(6, Luck); }
		void InMaxHP(int32& MaxHP) { BindParam(7, MaxHP); }
		void InMaxMP(int32& MaxMP) { BindParam(8, MaxMP); }
		void InAutoRecoveryHPPercent(int16& AutoRecoveryHPPercent) { BindParam(9, AutoRecoveryHPPercent); }
		void InAutoRecoveryMPPercent(int16& AutoRecoveryMPPercent) { BindParam(10, AutoRecoveryMPPercent); }
		void InMinAttackPoint(int32& MinMeleeAttackDamage) { BindParam(11, MinMeleeAttackDamage); }
		void InMaxAttackPoint(int32& MaxMeleeAttackDamage) { BindParam(12, MaxMeleeAttackDamage); }
		void InSpiritPoint(int16& MagicDamage) { BindParam(13, MagicDamage); }
		void InAttackHitRate(int16& MeleeAttackHitRate) { BindParam(14, MeleeAttackHitRate); }		
		void InSpellCastingRate(float& MagicHitRate) { BindParam(15, MagicHitRate); }
		void InDefence(int32& Defence) { BindParam(16, Defence); }
		void InEvasionRate(int16& EvasionRate) { BindParam(17, EvasionRate); }
		void InAttackCriticalPoint(int16& MeleeCriticalPoint) { BindParam(18, MeleeCriticalPoint); }
		void InSpellCriticalPoint(int16& MagicCriticalPoint) { BindParam(19, MagicCriticalPoint); }
		void InAttackCriticalResistance(int16& AttackCriticalResistance) { BindParam(20, AttackCriticalResistance); }
		void InStatusAbnormalResistance(int16& StatusAbnormalResistance) { BindParam(21, StatusAbnormalResistance); }
		void InSpeed(float& Speed) { BindParam(22, Speed); }
		void InLastPositionY(int32& LastPositionY) { BindParam(23, LastPositionY); }
		void InLastPositionX(int32& LastPositionX) { BindParam(24, LastPositionX); }
		void InCurrentExperience(int64& CurrentExperience) { BindParam(25, CurrentExperience); }
		void InRequireExperience(int64& RequireExperience) { BindParam(26, RequireExperience); }
		void InTotalExperience(int64& TotalExperience) { BindParam(27, TotalExperience); }		
		void InSkillMaxPoint(int8& SkillMaxPoint) { BindParam(28, SkillMaxPoint); }
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
	class CDBGameServerOnEquipment : public CDBBind<6, 0>
	{
	public:
		CDBGameServerOnEquipment(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spOnEquipment(?,?,?,?,?,?)}") {}
		void InAccountDBID(int64& AccountDBID) { BindParam(0, AccountDBID); }
		void InPlayerDBID(int64& AccountDBID) { BindParam(1, AccountDBID); }
		void InEquipmentParts(int8& EquipmentParts) { BindParam(2, EquipmentParts); }
		void InEquipmentSmallCategory(int16& EquipmentSmallCategory) { BindParam(3, EquipmentSmallCategory); }
		void InEquipmentDurability(int32& EquipmentDurability) { BindParam(4, EquipmentDurability); }
		void InEquipmentEnchantPoint(int8& EquipmentEnchantPoint) { BindParam(5, EquipmentEnchantPoint); }
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
	class CDBGameServerGetEquipment : public CDBBind<2, 4>
	{
	public:
		CDBGameServerGetEquipment(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spGetEquipemt(?,?)}") {}

		void InAccountDBID(int64& AccountDBID) { BindParam(0, AccountDBID); }
		void InPlayerDBID(int64& PlayerDBID) { BindParam(1, PlayerDBID); }

		void OutEquipmentParts(int8& EquipmentParts) { BindCol(0, EquipmentParts); }		
		void OutEquipmentSmallCategory(int16& EquipmentSmallCategory) { BindCol(1, EquipmentSmallCategory); }
		void OutEquipmentDurability(int32& EquipmentDurability) { BindCol(2, EquipmentDurability); }
		void OutEquipmentEnchantPoint(int8& EquipmentEnchantPoint) { BindCol(3, EquipmentEnchantPoint); }
	};

	// 유저 가입시 타일 정보 범위 업데이트
	class CDBGameServerTileRangeUpdate : public CDBBind<6, 0>
	{
	public:
		CDBGameServerTileRangeUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spTileRangeUpdate(?,?,?,?,?,?)}") {}
		
		void InIsTileOccupation(bool& TileOccupation) { BindParam(0, TileOccupation); }
		void InTileOwnerObjectID(int64& TileOwnerObjectID) { BindParam(1, TileOwnerObjectID); }
		void InStartTilePositionY(int16& TilePositionY) { BindParam(2, TilePositionY); }
		void InStartTilePositionX(int16& TilePositionX) { BindParam(3, TilePositionX); }
		void InTileRangeY(int16& TileRangeY) { BindParam(4, TileRangeY); }
		void InTileRangeX(int16& TileRangeX) { BindParam(5, TileRangeX); }
	};

	// 타일 정보 업데이트
	class CDBGameServerTileUpdate : public CDBBind<4, 0>
	{
	public:
		CDBGameServerTileUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spTileUpdate(?,?,?,?)}") {}

		void InIsTileOccupation(bool& IsTileOccupation) { BindParam(0, IsTileOccupation); }
		void InTileOwnerObjectID(int64& TileOwnerObjectID) { BindParam(1, TileOwnerObjectID); }
		void InTilePositionY(int16& TilePositionY) { BindParam(2, TilePositionY); }
		void InTilePositionX(int16& TilePositionX) { BindParam(3, TilePositionX); }
	};

	// 타일 정보 가져오기
	class CDBGameServerGetTileInfos : public CDBBind<0, 4>
	{
	public:
		CDBGameServerGetTileInfos(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spGetTileInfos()}") {}
	
		void OutIsTileOccupation(bool& IsTileOccupation) { BindCol(0, IsTileOccupation); }
		void OutIsTileOwnerObjectID(int64& TileOwnerObjectID) { BindCol(1, TileOwnerObjectID); }
		void OutTilePositionY(int16& TilePositionY) { BindCol(2, TilePositionY); }
		void OutTilePositionX(int16& TilePositionX) { BindCol(3, TilePositionX); }
	};

	// 타일 정보 범위 가져오기
	class CDBGameServerGetRangeTileInfos : public CDBBind<4, 4>
	{
	public:
		CDBGameServerGetRangeTileInfos(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spGetRangeTileInfos(?,?,?,?)}") {}
		
		void InStartTilePositionY(int16& StartTilePositionY) { BindParam(0, StartTilePositionY); }
		void InStartTilePositionX(int16& StarttilePositionX) { BindParam(1, StarttilePositionX); }
		void InTileRangeY(int16& StarttilePositionX) { BindParam(2, StarttilePositionX); }
		void InTileRangeX(int16& StarttilePositionX) { BindParam(3, StarttilePositionX); }

		void OutIsTileOccupation(bool& IsTileOccupation) { BindCol(0, IsTileOccupation); }
		void OutIsTileOwnerObjectID(int64& TileOwnerObjectID) { BindCol(1, TileOwnerObjectID); }
		void OutTilePositionY(int16& TilePositionY) { BindCol(2, TilePositionY); }
		void OutTilePositionX(int16& TilePositionX) { BindCol(3, TilePositionX); }
	};
}