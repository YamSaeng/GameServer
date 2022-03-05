#include "LoginServer.h"
#include "CommonProtocol.h"
#include "DBConnectionPool.h"
#include "DBStoreProcedure.h"
#include <process.h>

void CLoginServer::LoginServerStart(const WCHAR* OpenIP, int32 Port)
{
	CNetworkLib::Start(OpenIP, Port);

	_AuthThread = (HANDLE)_beginthreadex(NULL, 0, AuthThreadProc, this, 0, NULL);		
}

unsigned __stdcall CLoginServer::AuthThreadProc(void* Argument)
{
	CLoginServer* Instance = (CLoginServer*)Argument;

	while (!Instance->_AuthThreadEnd)
	{
		WaitForSingleObject(Instance->_AuthThreadWakeEvent, INFINITE);

		while (!Instance->_AuthThreadMessageQue.IsEmpty())
		{
			st_LoginServerJob* Job = nullptr;

			if (!Instance->_AuthThreadMessageQue.Dequeue(&Job))
			{
				break;
			}

			switch (Job->Type)
			{
			case en_LoginServerJobType::AUTH_ACCOUNT_NEW:
				Instance->AccountNew(Job->SessionID, Job->Message);
				break;
			case en_LoginServerJobType::AUTH_ACCOUNT_LOGIN:
				Instance->AccountLogIn(Job->SessionID);
				break;
			case en_LoginServerJobType::AUTH_ACCOUNT_LOGOUT:
				Instance->AccountLogOut(Job->SessionID);
				break;
			case en_LoginServerJobType::AUTH_ACCOUNT_DISCONNECT:
				break;
			}
		}
	}

	return 0;
}

void CLoginServer::OnRecv(int64 SessionID, CMessage* Packet)
{
	st_LoginServerJob* LoginServerJob = _LoginServerJobMemoryPool->Alloc();
	LoginServerJob->Type = en_LoginServerJobType::AUTH_MESSAGE;
	LoginServerJob->SessionID = SessionID;
	LoginServerJob->Message = Packet;

	_AuthThreadMessageQue.Enqueue(LoginServerJob);
	SetEvent(_AuthThreadWakeEvent);
}

void CLoginServer::PacketProc(int64 SessionID, CMessage* Packet)
{
	int16 MessageType;
	*Packet >> MessageType;

	switch (MessageType)
	{
	case en_LOGIN_SERVER_PACKET_TYPE::en_LOGIN_SERVER_C2S_ACCOUNT_NEW:
		PacketProcReqAccountNew(SessionID, Packet);
		break;
	}
}

void CLoginServer::PacketProcReqAccountNew(int64 SessionID, CMessage* Packet)
{
	st_LoginSession* Session = FindSession(SessionID);

	if (Session)
	{
		st_LoginServerJob* DBAccountCheckJob = _LoginServerJobMemoryPool->Alloc();
		DBAccountCheckJob->SessionID = Session->SessionId;

		CMessage* DBAccountCheckMessage = CMessage::Alloc();
		DBAccountCheckMessage->Clear();

		*DBAccountCheckMessage << (int16)en_LoginServerJobType::DATA_BASE_ACCOUNT_NEW;
		DBAccountCheckMessage->InsertData(Packet->GetFrontBufferPtr(), Packet->GetUseBufferSize());
		Packet->MoveReadPosition(Packet->GetUseBufferSize());		

		DBAccountCheckJob->Message = DBAccountCheckMessage;

		_AuthThreadMessageQue.Enqueue(DBAccountCheckJob);
	}
}

void CLoginServer::AccountNew(int64 SessionID, CMessage* Packet)
{
	st_LoginSession* Session = FindSession(SessionID);

	if (Session)
	{
		int8 AccountNewNameLen;
		*Packet >> AccountNewNameLen;

		WCHAR* AccountNewName = (WCHAR*)malloc(sizeof(WCHAR) * AccountNewNameLen);
		memset(AccountNewName, 0, sizeof(WCHAR) * AccountNewNameLen);
		Packet->GetData(AccountNewName, AccountNewNameLen);

		wstring AccountNewNameString = AccountNewName;

		int8 PasswordLen;
		*Packet >> PasswordLen;

		WCHAR* Password = (WCHAR*)malloc(sizeof(WCHAR) * PasswordLen);
		memset(Password, 0, sizeof(WCHAR) * PasswordLen);
		Packet->GetData(Password, PasswordLen);

		wstring PasswordString = Password;

		CDBConnection* AccountNewDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);
		
		SP::CLoginServerDBAccountNew AccountNew(*AccountNewDBConnection);
		AccountNew.InAccountName(AccountNewNameString);
		AccountNew.InPassword(PasswordString);

		AccountNew.Execute();

		free(AccountNewName);
		free(Password);

		AccountNewName = nullptr;
		Password = nullptr;
	}
}

void CLoginServer::AccountLogIn(int64 SessionID)
{

}

void CLoginServer::AccountLogOut(int64 SessionID)
{

}

void CLoginServer::DeleteClient(int64 SessionID)
{

}
