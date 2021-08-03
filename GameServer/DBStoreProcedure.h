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
	class CDBGameServerPlayersGet : public CDBBind<1, 7>
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
		void OutSpeed(float& Speed) { BindCol(6, Speed); }
	};

	// DB�� �Է��� �ش� ĳ���Ͱ� �ִ��� Ȯ��
	class CDBGameServerCharacterNameGet : public CDBBind<1, 0>
	{
	public:
		CDBGameServerCharacterNameGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetCharacterName(?)}") {}
		void InCharacterName(wstring& CharacterName) { BindParam(0, CharacterName.c_str()); }
	};

	// DB�� ���ο� ĳ�� ����
	class CDBGameServerCreateCharacterPush : public CDBBind<7, 0>
	{	
	public:		
		CDBGameServerCreateCharacterPush(CDBConnection& DBConnection) : CDBBind(DBConnection,L"{CALL dbo.spNewCharacterPush(?,?,?,?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		void InCharacterName(wstring& CharacterName) { BindParam(1, CharacterName.c_str()); }
		void InLevel(int32& Level)  { BindParam(2, Level); }
		void InCurrentHP(int32& CurrentHP) { BindParam(3, CurrentHP); }
		void InMaxHP(int32& MaxHP) { BindParam(4, MaxHP); }
		void InAttack(int32& Attack) { BindParam(5, Attack); }
		void InSpeed(float Speed) { BindParam(6, Speed); }		
	};

	// ĳ���� DBid ���
	class CDBGameServerPlayerDBIDGet : public CDBBind<1, 1>
	{
	public:
		CDBGameServerPlayerDBIDGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spPlayerDBIDGet(?)}") {}	
		void InAccountID(int32& AccountId) { BindCol(0, AccountId); }
		void OutPlayerDBID(int32& PlayerDBId) { BindCol(0, PlayerDBId); }
	};
}