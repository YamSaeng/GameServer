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
		template<int8 Length> void InPassword(BYTE(&Password)[Length]) { BindParam(1, Password); }		
	};

	// 계정 테이블에 계정 있는지 확인
	class CLoginServerAccountDBGetAccount : public CDBBind<1, 2>
	{
	public:
		CLoginServerAccountDBGetAccount(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetAccount(?)}") {}
		void InAccountName(wstring& AccountName) { BindParam(0, AccountName.c_str()); }		

		void OutAccountId(int64& AccountID) { BindCol(0, AccountID); }
		template<int8 Length> void OutPassword(BYTE(&Password)[Length]) { BindCol(1, Password); }
	};	

	// 계정 로그인
	class CLoginServerDBAccountLogin : public CDBBind<4, 0>
	{
	public:
		CLoginServerDBAccountLogin(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spAccountLogin(?,?,?,?)}") {}
		void InLoginState(int8& LoginState) { BindParam(0, LoginState); }
		void InAccountID(int64& AccountID) { BindParam(1, AccountID); }		
		void InAccountName(wstring& AccountName) { BindParam(2, AccountName.c_str()); }
		template<int8 Length> void InPassword(BYTE(&Password)[Length]) { BindParam(3, Password); }				
	};

	// 로그인 테이블에 계정 있는지 확인
	class CLoginServerLoginDBGetAccount : public CDBBind<2, 1>
	{
	public:
		CLoginServerLoginDBGetAccount(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetLogin(?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		void InAccountName(wstring& AccountName) { BindParam(1, AccountName.c_str()); }

		void OutLoginState(int8& LoginState) { BindCol(0, LoginState); }
	};

	// 로그인 상태 업데이트
	class CLoginServerLoginDBLoginStateUpdate : public CDBBind<2, 0>
	{
	public:
		CLoginServerLoginDBLoginStateUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spLoginStateUpdate(?,?)}") {}
		void InLoginState(int8& LoginState) { BindParam(0, LoginState); }
		void InAccountID(int64& AccountID) { BindParam(1, AccountID); }
	};

	// 토큰 있는지 확인
	class CTokenServerDBIsToken : public CDBBind<1, 2>
	{
	public:
		CTokenServerDBIsToken(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spIsToken(?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }

		void OutTokenTime(TIMESTAMP_STRUCT& TokenTime) { BindCol(0, TokenTime); }
		template<int8 Length> void OutToken(BYTE(&Token)[Length]) { BindCol(1, Token); }
	};

	// 새로운 토큰 생성해서 넣기
	class CLoginServerDBTokenNew : public CDBBind<4, 0>
	{
	public:
		CLoginServerDBTokenNew(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spTokenInfoInput(?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		template<int8 Length> void InToken(BYTE(&Token)[Length]) { BindParam(1, Token); }
		void InTokenCreateTime(TIMESTAMP_STRUCT& LoginSuccessTime) { BindParam(2, LoginSuccessTime); }
		void InTokenExpiredTime(TIMESTAMP_STRUCT& TokenExpiredTime) { BindParam(3, TokenExpiredTime); }
	};

	// 토큰 새로 갱신하기
	class CLoginServerDBTokenUpdate : public CDBBind<4, 0>
	{
	public:
		CLoginServerDBTokenUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spTokenUpdate(?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		template<int8 Length> void InToken(BYTE(&Token)[Length]) { BindParam(1, Token); }
		void InTokenCreateTime(TIMESTAMP_STRUCT& TokenCreateTime) { BindParam(2, TokenCreateTime); }
		void InTokenExpiredTime(TIMESTAMP_STRUCT& TokenExpiredTime) { BindParam(3, TokenExpiredTime); }
	};	
}