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
	class CDBGameServerPlayersGet : public CDBBind<1, 10>
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
		void OutAttack(int32& Attack) { BindCol(6, Attack); }
		void OutCriticalPoint(int16& CriticalPoint) { BindCol(7, CriticalPoint); }
		void OutSpeed(float& Speed) { BindCol(8, Speed); }
		void OutPlayerObjectType(int16& ObjectType) { BindCol(9, ObjectType); }
	};

	// DB에 입력한 해당 캐릭터가 있는지 확인
	class CDBGameServerCharacterNameGet : public CDBBind<1, 0>
	{
	public:
		CDBGameServerCharacterNameGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetCharacterName(?)}") {}
		void InCharacterName(wstring& CharacterName) { BindParam(0, CharacterName.c_str()); }
	};

	// DB에 새로운 캐릭 저장
	class CDBGameServerCreateCharacterPush : public CDBBind<10, 0>
	{	
	public:		
		CDBGameServerCreateCharacterPush(CDBConnection& DBConnection) : CDBBind(DBConnection,L"{CALL dbo.spNewCharacterPush(?,?,?,?,?,?,?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		void InPlayerName(wstring& PlayerName) { BindParam(1, PlayerName.c_str()); }
		void InPlayerType(int16& PlayerType) { BindParam(2, PlayerType); }
		void InPlayerIndex(int8& PlayerIndex) { BindParam(3, PlayerIndex); }
		void InLevel(int32& Level)  { BindParam(4, Level); }
		void InCurrentHP(int32& CurrentHP) { BindParam(5, CurrentHP); }
		void InMaxHP(int32& MaxHP) { BindParam(6, MaxHP); }
		void InAttack(int32& Attack) { BindParam(7, Attack); }
		void InCriticalPoint(int16& CriticalPoint) { BindParam(8, CriticalPoint); }
		void InSpeed(float Speed) { BindParam(9, Speed); }
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
	class CDBGameServerCreateItem : public CDBBind<6, 0>
	{
	public:
		CDBGameServerCreateItem(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemCreate(?,?,?,?,?,?)}") {}
		void InItemObjectId(int64& ItemObjectId) { BindParam(0, ItemObjectId); }
		void InItemType(int16& ItemType) { BindParam(1, ItemType); }
		void InItemName(wstring& ItemName) { BindParam(2, ItemName.c_str()); }
		void InItemCount(int16& ItemCount) { BindParam(3, ItemCount); }
		void InIsEquipped(bool& IsEquipped) { BindParam(4, IsEquipped); }
		void InThumbnailImagePath(wstring& ThumbnailImagePath) { BindParam(5, ThumbnailImagePath.c_str()); }
	};

	// ItemDBId 얻기
	class CDBGameServerItemDBIdGet : public CDBBind<1, 1>
	{
	public:
		CDBGameServerItemDBIdGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemDBIdGet(?,?)}") {}
		void InItemObjectId(int64& ObjectId) { BindParam(0, ObjectId); }
		void OutItemDBId(int64& ItemDBId) { BindCol(0, ItemDBId); }
	};

	// InventoryTable에 새로운 Item 저장
	class CDBGameServerItemToInventoryPush : public CDBBind<9, 0>
	{
	public:
		CDBGameServerItemToInventoryPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemToInventorySave(?,?,?,?,?,?,?,?,?)}") {}				
		void InItemDBId(int64& ItemDBId) { BindParam(0, ItemDBId); }
		void InItemType(int16& ItemType) { BindParam(1, ItemType); }
		void InItemName(wstring& ItemName) { BindParam(2, ItemName.c_str()); }
		void InItemCount(int16& ItemCount) { BindParam(3, ItemCount); }
		void InSlotIndex(int8& SlotIndex) { BindParam(4, SlotIndex); }
		void InIsEquipped(bool& IsEquipped) { BindParam(5, IsEquipped); }		
		void InThumbnailImagePath(wstring& ThumbnailImagePath) { BindParam(6, ThumbnailImagePath.c_str()); }
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(7, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(8, OwnerPlayerId); }		
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

	// Swap 요청한 아이템이 DB에 있는지 확인
	class CDBGameServerItemCheck : public CDBBind<3, 2>
	{
	public:
		CDBGameServerItemCheck(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spItemCheck(?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InSlotIndex(int8& SlotIndex) { BindParam(2, SlotIndex); }
		void OutItemType(int16& ItemType) { BindCol(0, ItemType); }				
		void OutItemCount(int16& ItemCount) { BindCol(1, ItemCount); }
	};

	// Item Swap 
	class CDBGameServerItemSwap : public CDBBind<5, 0>
	{
	public:		
		CDBGameServerItemSwap(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL spItemSwap(?,?,?,?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void InItemType(int16& ItemType) { BindParam(2, ItemType); }
		void InItemCount(int16& ItemCount) { BindParam(3, ItemCount); }
		void InSlotIndex(int8& SlotIndex) { BindParam(4, SlotIndex); }
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
		void InSliverCoin(int8& SliverCoin) { BindParam(3, SliverCoin); }
		void InBronzeCoin(int8& BronzeCoin) { BindParam(4, BronzeCoin); }
	};

	// GoldTable에 있는 Gold 긁어옴
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
	
	// ItemTable에 있는 Item 모두 긁어옴
	class CDBGameServerInventoryItemGet : public CDBBind<2, 5>
	{
	public:
		CDBGameServerInventoryItemGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetItemTableInfoToInventory(?,?)}") {}
		void InAccountDBId(int64& AccountDBId) { BindParam(0, AccountDBId); }
		void InPlayerDBId(int64& PlayerDBId) { BindParam(1, PlayerDBId); }
		void OutDataSheetId(int16& DataSheetId) { BindCol(0, DataSheetId); }
		void OutItemType(int16& ItemType) { BindCol(1, ItemType); }
		void OutItemCount(int16& itemCount) { BindCol(2, itemCount); }
		void OutSlotIndex(int8& SlotIndex) { BindCol(3, SlotIndex); }		
		void OutIsEquipped(bool& IsEquipped) { BindCol(4, IsEquipped); }
	};
}