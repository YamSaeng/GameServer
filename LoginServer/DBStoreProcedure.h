#pragma once
#include "DBBind.h"

namespace SP
{
	class CLoginServerDBAccountNew : public CDBBind<2, 0>
	{
	public:
		CLoginServerDBAccountNew(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.}") {}

		void InAccountName(wstring& AccountName) { BindParam(0, AccountName.c_str()); }
		void InPassword(wstring& Password) { BindParam(1, Password.c_str()); }
	};

	// DB�� �Է��� �ش� ĳ���Ͱ� �ִ��� Ȯ��
	class CDBGameServerCharacterNameGet : public CDBBind<2, 0>
	{
	public:
		CDBGameServerCharacterNameGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetCharacterName(?)}") {}
		void InCharacterName(wstring& CharacterName) { BindParam(0, CharacterName.c_str()); }
	};	
}