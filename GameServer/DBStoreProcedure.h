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
	class CDBGameServerPlayersGet : public CDBBind<1, 8>
	{	
	public:
		CDBGameServerPlayersGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetPlayers(?)}") { }
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		void OutPlayerDBID(int64& PlayerDBID) { BindCol(0, PlayerDBID); }		
		template<int32 N> void OutPlayerName(WCHAR(&PlayerName)[N]) { BindCol(1, PlayerName); }
		void OutLevel(int32& Level) { BindCol(2, Level); }
		void OutCurrentHP(int32& CurrentHP) { BindCol(3, CurrentHP); }
		void OutMaxHP(int32& MaxHP) { BindCol(4, MaxHP); }
		void OutAttack(int32& Attack) { BindCol(5, Attack); }
		void OutCriticalPoint(int16& CriticalPoint) { BindCol(6, CriticalPoint); }
		void OutSpeed(float& Speed) { BindCol(7, Speed); }
	};

	// DB에 입력한 해당 캐릭터가 있는지 확인
	class CDBGameServerCharacterNameGet : public CDBBind<1, 0>
	{
	public:
		CDBGameServerCharacterNameGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetCharacterName(?)}") {}
		void InCharacterName(wstring& CharacterName) { BindParam(0, CharacterName.c_str()); }
	};

	// DB에 새로운 캐릭 저장
	class CDBGameServerCreateCharacterPush : public CDBBind<8, 0>
	{	
	public:		
		CDBGameServerCreateCharacterPush(CDBConnection& DBConnection) : CDBBind(DBConnection,L"{CALL dbo.spNewCharacterPush(?,?,?,?,?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		void InCharacterName(wstring& CharacterName) { BindParam(1, CharacterName.c_str()); }
		void InLevel(int32& Level)  { BindParam(2, Level); }
		void InCurrentHP(int32& CurrentHP) { BindParam(3, CurrentHP); }
		void InMaxHP(int32& MaxHP) { BindParam(4, MaxHP); }
		void InAttack(int32& Attack) { BindParam(5, Attack); }
		void InCriticalPoint(int16& CriticalPoint) { BindParam(6, CriticalPoint); }
		void InSpeed(float Speed) { BindParam(7, Speed); }
	};

	// 캐릭터 DBid 얻기
	class CDBGameServerPlayerDBIDGet : public CDBBind<1, 1>
	{
	public:
		CDBGameServerPlayerDBIDGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spPlayerDBIdGet(?)}") {}	
		void InAccountID(int64& AccountId) { BindParam(0, AccountId); }
		void OutPlayerDBID(int64& PlayerDBId) { BindCol(0, PlayerDBId); }
	};

	// ItemTable에 새로운 Item 저장
	class CDBGameServerItemToInventoryPush : public CDBBind<7, 0>
	{
	public:
		CDBGameServerItemToInventoryPush(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spItemToInventorySave(?,?,?,?,?,?,?)}") {}		
		void InDataSheetId(int16& DataSheetId) { BindParam(0, DataSheetId); }
		void InItemType(int16& ItemType) { BindParam(1, ItemType); }
		void InCount(int16& Count) { BindParam(2, Count); }
		void InSlotIndex(int8& SlotIndex) { BindParam(3, SlotIndex); }
		void InOwnerAccountId(int64& OwnerAccountId) { BindParam(4, OwnerAccountId); }
		void InOwnerPlayerId(int64& OwnerPlayerId) { BindParam(5, OwnerPlayerId); }
		void InIsEquipped(bool& IsEquipped) { BindParam(6, IsEquipped); }		
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