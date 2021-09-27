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

	// DB에 입력한 해당 캐릭터가 있는지 확인
	class CDBGameServerCharacterNameGet : public CDBBind<1, 0>
	{
	public:
		CDBGameServerCharacterNameGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetCharacterName(?)}") {}
		void InCharacterName(wstring& CharacterName) { BindParam(0, CharacterName.c_str()); }
	};

	// DB에 새로운 캐릭 저장
	class CDBGameServerCreateCharacterPush : public CDBBind<15, 0>
	{	
	public:		
		CDBGameServerCreateCharacterPush(CDBConnection& DBConnection) : CDBBind(DBConnection,L"{CALL dbo.spNewCharacterPush(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		void InPlayerName(wstring& PlayerName) { BindParam(1, PlayerName.c_str()); }
		void InPlayerType(int16& PlayerType) { BindParam(2, PlayerType); }
		void InPlayerIndex(int8& PlayerIndex) { BindParam(3, PlayerIndex); }
		void InLevel(int32& Level)  { BindParam(4, Level); }
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

	// ItemDBId 얻기
	class CDBGameServerItemDBIdGet : public CDBBind<0, 1>
	{
	public:
		CDBGameServerItemDBIdGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemDBIdGet()}") {}
		void OutItemDBId(int64& ItemDBId) { BindCol(0, ItemDBId); }
	};

	// 인벤토리 초기화 할때 빈껍데기 생성해서 넣는 프로시저 클래스
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

	// InventoryTable에 새로운 Item 저장
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
	
	// ItemTable에 있는 Item 모두 긁어옴
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

	// SkillTable에 있는 Skill 모두 긁어옴
	class CDBGameServerSkillGet : public CDBBind<2, 6>
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
		template<int8 Length> void OutSkillThumbnailImagePath(WCHAR(&SkillThumbnailImagePath)[Length]) { BindCol(5, SkillThumbnailImagePath); }
	};

	// QuickSlotBarSlot정보 새로 생성
	class CDBGameServerQuickSlotBarSlotCreate : public CDBBind<10, 0>
	{
	public:
		CDBGameServerQuickSlotBarSlotCreate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spQuickSlotBarSlotCreate(?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		
		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InQuickSlotKey(wstring& QuickSlotKey) { BindParam(4, QuickSlotKey.c_str()); }
		void InSkillType(int16& SkillType) { BindParam(5, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(6, SkillLevel); }
		void InSkillName(wstring& SkillName) { BindParam(7, SkillName.c_str()); }		
		void InSkillCoolTime(int32& SkillCoolTime) { BindParam(8, SkillCoolTime); }
		void InSkillThumbnailImagePath(wstring& SkillThumbnailImagePath) { BindParam(9, SkillThumbnailImagePath.c_str()); }		
	};

	// QuickSlotBarSlot정보 업데이트 프로시저
	class CDBGameServerQuickSlotBarSlotUpdate : public CDBBind<10, 0>
	{
	public:
		CDBGameServerQuickSlotBarSlotUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spQuickSlotBarSlotUpdate(?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InQuickSlotKey(wstring& QuickSlotKey) { BindParam(4, QuickSlotKey.c_str()); }
		void InSkillType(int16& SkillType) { BindParam(5, SkillType); }
		void InSkillLevel(int8& SkillLevel) { BindParam(6, SkillLevel); }
		void InSkillName(wstring& SkillName) { BindParam(7, SkillName.c_str()); }
		void InSkillCoolTime(int32& SkillCoolTime) { BindParam(8, SkillCoolTime); }
		void InSkillThumbnailImagePath(wstring& SkillThumbnailImagePath) { BindParam(9, SkillThumbnailImagePath.c_str()); }
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
		template<int8 Length> void OutQuickSlotKey(WCHAR(&QuickSlotKey)[Length]) { BindCol(2, QuickSlotKey); }
		void OutQuickSlotSkillType(int16& SkillType) { BindCol(3, SkillType); }
		void OutQuickSlotSkillLevel(int8& SkillLevel) { BindCol(4, SkillLevel); }
		void OutQuickSlotSkillCoolTime(int32& SkillCoolTime) { BindCol(5, SkillCoolTime); }
		template<int8 Length> void OutQuickSlotSkillThumbnailImagePath(WCHAR(&SkillThumbnailImagePath)[Length]) { BindCol(6, SkillThumbnailImagePath); }
	};

	// Swap 요청한 아이템이 QuickSlot에 있는지 확인하고
	// 요청한 퀵슬롯 정보 반환
	class CDBGameServerQuickSlotCheck : public CDBBind<4, 6>
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
		template<int8 Length> void OutQuickSlotSkillThumbnailImagePath(WCHAR(&SkillThumbnailImagePath)[Length]) { BindCol(5, SkillThumbnailImagePath); }
	};

	// QuickSlot Swap
	class CDBGameServerQuickSlotSwap : public CDBBind<16, 0>
	{
	public:
		CDBGameServerQuickSlotSwap(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spQuickSlotSwap(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }

		void InAQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(2, QuickSlotBarIndex); }
		void InAQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(3, QuickSlotBarSlotIndex); }
		void InAQuickSlotSkillType(int16& QuickSlotSkillType) { BindParam(4, QuickSlotSkillType); }
		void InAQuickSlotSkillLevel(int8& QuickSkillLevel) { BindParam(5, QuickSkillLevel); }
		void InAQuickSlotSKillName(wstring& QuickSlotSKillNam) { BindParam(6, QuickSlotSKillNam.c_str()); }
		void InAQuickSlotSkillCoolTime(int32& QuickSlotSkillCoolTime) { BindParam(7, QuickSlotSkillCoolTime); }
		void InAQuickSlotSKillImagePath(wstring& QuickSlotSKillImagePath) { BindParam(8, QuickSlotSKillImagePath.c_str()); }

		void InBQuickSlotBarIndex(int8& QuickSlotBarIndex) { BindParam(9, QuickSlotBarIndex); }
		void InBQuickSlotBarSlotIndex(int8& QuickSlotBarSlotIndex) { BindParam(10, QuickSlotBarSlotIndex); }
		void InBQuickSlotSkillType(int16& QuickSlotSkillType) { BindParam(11, QuickSlotSkillType); }
		void InBQuickSlotSkillLevel(int8& QuickSkillLevel) { BindParam(12, QuickSkillLevel); }
		void InBQuickSlotSKillName(wstring& QuickSlotSKillNam) { BindParam(13, QuickSlotSKillNam.c_str()); }
		void InBQuickSlotSkillCoolTime(int32& QuickSlotSkillCoolTime) { BindParam(14, QuickSlotSkillCoolTime); }
		void InBQuickSlotSKillImagePath(wstring& QuickSlotSKillImagePath) { BindParam(15, QuickSlotSKillImagePath.c_str()); }
	};
}