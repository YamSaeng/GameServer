#pragma once
#include "DBBind.h"

namespace SP
{
	// ���� ���� ����
	class CLoginServerDBAccountNew : public CDBBind<2, 0>
	{
	public:
		CLoginServerDBAccountNew(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spAccountNew(?,?)}") {}

		void InAccountName(wstring& AccountName) { BindParam(0, AccountName.c_str()); }
		void InPassword(wstring& Password) { BindParam(1, Password.c_str()); }
	};

	// ���� ���̺� ���� �ִ��� Ȯ��
	class CLoginServerAccountDBGetAccount : public CDBBind<2, 0>
	{
	public:
		CLoginServerAccountDBGetAccount(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetAccount(?,?)}") {}
		void InAccountName(wstring& AccountName) { BindParam(0, AccountName.c_str()); }
		void InPassword(wstring& Password) { BindParam(1, Password.c_str()); }
	};	

	// ���� �α���
	class CLoginServerDBAccountLogin : public CDBBind<4, 0>
	{
	public:
		CLoginServerDBAccountLogin(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spAccountLogin(?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }		
		void InAccountName(wstring& AccountName) { BindParam(1, AccountName.c_str()); }
		void InPassword(wstring& Password) { BindParam(2, Password.c_str()); }		
		void InLoginSuccessTime(TIMESTAMP_STRUCT& TokenExpiredTime) { BindParam(3, TokenExpiredTime); }
	};

	// �α��� ���̺� ���� �ִ��� Ȯ��
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