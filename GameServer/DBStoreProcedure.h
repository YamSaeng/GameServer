#pragma once
#include "DBBind.h"

namespace SP
{
	class CDBAccountTokenGet : public CDBBind<1, 3>
	{
	private:
		int32 _AccountID;
	public:
		CDBAccountTokenGet(CDBConnection& DBConnection) : CDBBind(DBConnection, L"{CALL dbo.spGetAccountToken(?)}") { }
		void InAccountID(int64& AccountID) { BindParam(0, AccountID); };		
		void OutToken(int32& Token) { BindCol(0, Token); }		
		void OutLoginsuccessTime(TIMESTAMP_STRUCT& LoginSuccessTime) { BindCol(1, LoginSuccessTime); }
		void OutTokenExpiredTime(TIMESTAMP_STRUCT& TokenExpiredTime) { BindCol(2, TokenExpiredTime); }
	};
}