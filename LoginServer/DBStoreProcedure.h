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
		template<int8 Length> void InPassword(BYTE(&Password)[Length]) { BindParam(1, Password); }		
	};

	// ���� ���̺� ���� �ִ��� Ȯ��
	class CLoginServerAccountDBGetAccount : public CDBBind<1, 2>
	{
	public:
		CLoginServerAccountDBGetAccount(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetAccount(?)}") {}
		void InAccountName(wstring& AccountName) { BindParam(0, AccountName.c_str()); }		

		void OutAccountId(int64& AccountID) { BindCol(0, AccountID); }
		template<int8 Length> void OutPassword(BYTE(&Password)[Length]) { BindCol(1, Password); }
	};	

	// ���� �α���
	class CLoginServerDBAccountLogin : public CDBBind<4, 0>
	{
	public:
		CLoginServerDBAccountLogin(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spAccountLogin(?,?,?,?)}") {}
		void InLoginState(int8& LoginState) { BindParam(0, LoginState); }
		void InAccountID(int64& AccountID) { BindParam(1, AccountID); }		
		void InAccountName(wstring& AccountName) { BindParam(2, AccountName.c_str()); }
		template<int8 Length> void InPassword(BYTE(&Password)[Length]) { BindParam(3, Password); }				
	};

	// �α��� ���̺� ���� �ִ��� Ȯ��
	class CLoginServerLoginDBGetAccount : public CDBBind<2, 1>
	{
	public:
		CLoginServerLoginDBGetAccount(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetLogin(?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		void InAccountName(wstring& AccountName) { BindParam(1, AccountName.c_str()); }

		void OutLoginState(int8& LoginState) { BindCol(0, LoginState); }
	};

	// �α��� ���� ������Ʈ
	class CLoginServerLoginDBLoginStateUpdate : public CDBBind<2, 0>
	{
	public:
		CLoginServerLoginDBLoginStateUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spLoginStateUpdate(?,?)}") {}
		void InLoginState(int8& LoginState) { BindParam(0, LoginState); }
		void InAccountID(int64& AccountID) { BindParam(1, AccountID); }
	};

	// ��ū �ִ��� Ȯ��
	class CTokenServerDBIsToken : public CDBBind<1, 2>
	{
	public:
		CTokenServerDBIsToken(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spIsToken(?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }

		void OutTokenTime(TIMESTAMP_STRUCT& TokenTime) { BindCol(0, TokenTime); }
		template<int8 Length> void OutToken(BYTE(&Token)[Length]) { BindCol(1, Token); }
	};

	// ���ο� ��ū �����ؼ� �ֱ�
	class CLoginServerDBTokenNew : public CDBBind<4, 0>
	{
	public:
		CLoginServerDBTokenNew(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spTokenInfoInput(?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		template<int8 Length> void InToken(BYTE(&Token)[Length]) { BindParam(1, Token); }
		void InTokenCreateTime(TIMESTAMP_STRUCT& LoginSuccessTime) { BindParam(2, LoginSuccessTime); }
		void InTokenExpiredTime(TIMESTAMP_STRUCT& TokenExpiredTime) { BindParam(3, TokenExpiredTime); }
	};

	// ��ū ���� �����ϱ�
	class CLoginServerDBTokenUpdate : public CDBBind<4, 0>
	{
	public:
		CLoginServerDBTokenUpdate(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spTokenUpdate(?,?,?,?)}") {}
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); }
		template<int8 Length> void InToken(BYTE(&Token)[Length]) { BindParam(1, Token); }
		void InTokenCreateTime(TIMESTAMP_STRUCT& TokenCreateTime) { BindParam(2, TokenCreateTime); }
		void InTokenExpiredTime(TIMESTAMP_STRUCT& TokenExpiredTime) { BindParam(3, TokenExpiredTime); }
	};	

	// ���� ��� ��������
	class CLoginServerDBGetServerList : public CDBBind<0, 4>
	{
	public:
		CLoginServerDBGetServerList(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetServerList()}") {}
		template<int8 Length> void OutServerName(WCHAR(&ServerName)[Length]) { BindCol(0, ServerName); }
		template<int8 Length> void OutServerIP(WCHAR(&ServerIP)[Length]) { BindCol(1, ServerIP); }
		void OutServerPort(int32& ServerPort) { BindCol(2, ServerPort); }
		void OutServerBusy(float& ServerBusy) { BindCol(3, ServerBusy); }
	};
}