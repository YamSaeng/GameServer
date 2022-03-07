#pragma once
#include "DBBind.h"

namespace SP
{
	// 계정 새로 생성
	class CLoginServerDBAccountNew : public CDBBind<2, 0>
	{
	public:
		CLoginServerDBAccountNew(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spAccountNew(?,?)}") {}

		void InAccountName(wstring& AccountName) { BindParam(0, AccountName.c_str()); }
		void InPassword(wstring& Password) { BindParam(1, Password.c_str()); }
	};

	// 계정 테이블에 계정 있는지 확인
	class CLoginServerAccountDBGetAccount : public CDBBind<2, 0>
	{
	public:
		CLoginServerAccountDBGetAccount(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetAccount(?,?)}") {}
		void InAccountName(wstring& AccountName) { BindParam(0, AccountName.c_str()); }
		void InPassword(wstring& Password) { BindParam(1, Password.c_str()); }
	};	

	// 계정 로그인
	class CLoginServerDBAccountLogin : public CDBBind<4, 0>
	{
	public:
		CLoginServerDBAccountLogin(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spAccountLogin(?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }		
		void InAccountName(wstring& AccountName) { BindParam(1, AccountName.c_str()); }
		void InPassword(wstring& Password) { BindParam(2, Password.c_str()); }		
		void InLoginSuccessTime(TIMESTAMP_STRUCT& TokenExpiredTime) { BindParam(3, TokenExpiredTime); }
	};

	// 로그인 테이블에 계정 있는지 확인
	class CLoginServerLoginDBGetAccount : public CDBBind<2, 0>
	{
	public:
		CLoginServerLoginDBGetAccount(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetLogin(?,?)}") {}
		void InAccountID(int64 AccountID) { BindParam(0, AccountID); }
		void InAccountName(wstring& AccountName) { BindParam(1, AccountName.c_str()); }
	};

	class CTokenServerDBGetToken : public CDBBind<2, 0>
	{
		CTokenServerDBGetToken(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetAccount(?,?)}") {}
	};
}