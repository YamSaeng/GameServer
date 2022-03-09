#include "LoginServer.h"
#include "CommonProtocol.h"
#include "DBConnectionPool.h"
#include "DBStoreProcedure.h"
#include "KISA_SHA256.h"
#include <process.h>

CLoginServer::CLoginServer()
{
	_AuthThreadEnd = false;
	_DataBaseThreadEnd = false;

	_AuthThread = nullptr;
	_DataBaseThread = nullptr;

	_AuthThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_DataBaseThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	_LoginServerJobMemoryPool = new CMemoryPoolTLS<st_LoginServerJob>();
}

CLoginServer::~CLoginServer()
{
}

void CLoginServer::LoginServerStart(const WCHAR* OpenIP, int32 Port)
{
	CNetworkLib::Start(OpenIP, Port);

	_AuthThread = (HANDLE)_beginthreadex(NULL, 0, AuthThreadProc, this, 0, NULL);

	_DataBaseThread = (HANDLE)_beginthreadex(NULL, 0, DataBaseThreadProc, this, 0, NULL);
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
			case en_LoginServerJobType::AUTH_MESSAGE:
				Instance->PacketProc(Job->SessionID, Job->Message);
				break;			
			}

			Instance->_LoginServerJobMemoryPool->Free(Job);
		}
	}

	return 0;
}

unsigned __stdcall CLoginServer::DataBaseThreadProc(void* Argument)
{
	CLoginServer* Instance = (CLoginServer*)Argument;

	while (!Instance->_DataBaseThreadEnd)
	{
		WaitForSingleObject(Instance->_DataBaseThreadWakeEvent, INFINITE);

		while (!Instance->_DataBaseThreadMessageQue.IsEmpty())
		{
			st_LoginServerJob* Job = nullptr;

			if (!Instance->_DataBaseThreadMessageQue.Dequeue(&Job))
			{
				break;
			}

			switch (Job->Type)
			{
			case en_LoginServerJobType::DATA_BASE_ACCOUNT_NEW:
				Instance->AccountNew(Job->SessionID, Job->Message);
				break;		
			case en_LoginServerJobType::DATA_BASE_ACCOUNT_LOGIN:
				Instance->AccountLogIn(Job->SessionID, Job->Message);
				break;
			case en_LoginServerJobType::DATA_BASE_ACCOUNT_LOGOUT:
				Instance->AccountLogOut(Job->SessionID, Job->Message);
				break;
			}

			Job->Message->Free();
			Instance->_LoginServerJobMemoryPool->Free(Job);			
		}
	}

	return 0;
}

void CLoginServer::OnClientJoin(int64 SessionID)
{
}

void CLoginServer::OnClientLeave(st_LoginSession* LeaveSession)
{
	st_LoginServerJob* LogOutServerJob = _LoginServerJobMemoryPool->Alloc();
	LogOutServerJob->Type = en_LoginServerJobType::DATA_BASE_ACCOUNT_LOGOUT;
	LogOutServerJob->SessionID = LeaveSession->SessionId;	

	CMessage* JobMessage = CMessage::Alloc();
	JobMessage->Clear();

	*JobMessage << LeaveSession->AccountId;

	LogOutServerJob->Message = JobMessage;

	_DataBaseThreadMessageQue.Enqueue(LogOutServerJob);
	SetEvent(_DataBaseThreadWakeEvent);
}

bool CLoginServer::OnConnectionRequest(const wchar_t ClientIP, int32 Port)
{
	return true;
}

void CLoginServer::OnRecv(int64 SessionID, CMessage* Packet)
{
	st_LoginServerJob* LoginServerJob = _LoginServerJobMemoryPool->Alloc();
	LoginServerJob->Type = en_LoginServerJobType::AUTH_MESSAGE;
	LoginServerJob->SessionID = SessionID;	

	CMessage* JobMessage = CMessage::Alloc();
	JobMessage->Clear();

	JobMessage->InsertData(Packet->GetFrontBufferPtr(), Packet->GetUseBufferSize());
	Packet->MoveReadPosition(Packet->GetUseBufferSize());

	LoginServerJob->Message = JobMessage;

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
	case en_LOGIN_SERVER_PACKET_TYPE::en_LOGIN_SERVER_C2S_ACCOUNT_LOGIN:
		PacketProcReqAccountLogin(SessionID, Packet);
		break;
	}

	Packet->Free();
}

void CLoginServer::PacketProcReqAccountNew(int64 SessionID, CMessage* Packet)
{
	st_LoginSession* Session = FindSession(SessionID);

	if (Session)
	{
		st_LoginServerJob* DBAccountNewJob = _LoginServerJobMemoryPool->Alloc();
		DBAccountNewJob->SessionID = Session->SessionId;
		DBAccountNewJob->Type = en_LoginServerJobType::DATA_BASE_ACCOUNT_NEW;
		
		CMessage* DBAccountNewMessage = CMessage::Alloc();
		DBAccountNewMessage->Clear();
		
		DBAccountNewMessage->InsertData(Packet->GetFrontBufferPtr(), Packet->GetUseBufferSize());
		Packet->MoveReadPosition(Packet->GetUseBufferSize());		

		DBAccountNewJob->Message = DBAccountNewMessage;

		_DataBaseThreadMessageQue.Enqueue(DBAccountNewJob);
		SetEvent(_DataBaseThreadWakeEvent);

		ReturnSession(Session);
	}
}

void CLoginServer::PacketProcReqAccountLogin(int64 SessionID, CMessage* Packet)
{
	st_LoginSession* Session = FindSession(SessionID);

	if (Session)
	{
		st_LoginServerJob* DBAccountLoginJob = _LoginServerJobMemoryPool->Alloc();
		DBAccountLoginJob->SessionID = Session->SessionId;
		DBAccountLoginJob->Type = en_LoginServerJobType::DATA_BASE_ACCOUNT_LOGIN;

		CMessage* DBAccountLoginMessage = CMessage::Alloc();
		DBAccountLoginMessage->Clear();

		DBAccountLoginMessage->InsertData(Packet->GetFrontBufferPtr(), Packet->GetUseBufferSize());
		Packet->MoveReadPosition(Packet->GetUseBufferSize());

		DBAccountLoginJob->Message = DBAccountLoginMessage;

		_DataBaseThreadMessageQue.Enqueue(DBAccountLoginJob);
		SetEvent(_DataBaseThreadWakeEvent);		

		ReturnSession(Session);
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
		
		BYTE PasswordHash[50] = { 0 };		

		SHA256_Encrpyt((unsigned char*)Password, PasswordLen, (unsigned char*)PasswordHash);		

		CDBConnection* AccountNewDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);
		SP::CLoginServerDBAccountNew AccountNew(*AccountNewDBConnection);
		AccountNew.InAccountName(AccountNewNameString);
		AccountNew.InPassword(PasswordHash);

		bool AccountNewSuccess = AccountNew.Execute();		

		free(AccountNewName);
		free(Password);		

		AccountNewName = nullptr;
		Password = nullptr;

		G_DBConnectionPool->Push(en_DBConnect::ACCOUNT, AccountNewDBConnection);

		CMessage* ResAccountNewMessage = MakePacketResAccountNew(AccountNewSuccess);
		SendPacket(Session->SessionId, ResAccountNewMessage);
		ResAccountNewMessage->Free();
		
		ReturnSession(Session);
	}
}

void CLoginServer::AccountLogIn(int64 SessionID, CMessage* Packet)
{
	st_LoginSession* Session = FindSession(SessionID);

	if (Session)
	{
		en_LoginInfo LoginInfo;

		// Ŭ�� ���� ���̵� ��������
		int8 AccountLoginNameLen;
		*Packet >> AccountLoginNameLen;

		WCHAR* AccountLoginName = (WCHAR*)malloc(sizeof(WCHAR) * AccountLoginNameLen);
		memset(AccountLoginName, 0, sizeof(WCHAR) * AccountLoginNameLen);
		Packet->GetData(AccountLoginName, AccountLoginNameLen);

		wstring AccountLoginNameString = AccountLoginName;

		// Ŭ�� ���� ��й�ȣ ��������
		int8 PasswordLen;
		*Packet >> PasswordLen;

		WCHAR* Password = (WCHAR*)malloc(sizeof(WCHAR) * PasswordLen);
		memset(Password, 0, sizeof(WCHAR) * PasswordLen);
		Packet->GetData(Password, PasswordLen);				

		// ��й�ȣ Hash�� ����� ���� �迭 �غ�
		char PasswordHash[TOKEN_MAX_LENGTH] = { 0 };
		// Hash�� ���ϱ�
		SHA256_Encrpyt((unsigned char*)Password, PasswordLen, (unsigned char*)PasswordHash);

		int8 PasswordHashLen = (int8)strlen(PasswordHash);

		// ���� �ִ��� Ȯ��
		CDBConnection* AccountCheckDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);
		SP::CLoginServerAccountDBGetAccount AccountCheck(*AccountCheckDBConnection);
		AccountCheck.InAccountName(AccountLoginNameString);		

		int64 AccountID;
		BYTE OutPassword[TOKEN_MAX_LENGTH] = { 0 };
		
		// AccountID �� ��������
		AccountCheck.OutAccountId(AccountID);
		// ��й�ȣ �ؽ��� ��������
		AccountCheck.OutPassword(OutPassword);

		AccountCheck.Execute();

		bool AccountCheckSuccess = AccountCheck.Fetch();

		G_DBConnectionPool->Push(en_DBConnect::ACCOUNT, AccountCheckDBConnection);

		// ���� ���
		vector<st_ServerInfo> ServerLists;
		// ��ū ��
		BYTE TokenHash[TOKEN_MAX_LENGTH] = { 0 };		

		// ���� ���� ��� �α��� ����															
		if (AccountCheckSuccess == true)
		{
			int8 LoginState;

			// ����� �´��� Ȯ��
			if (memcmp(PasswordHash, OutPassword, PasswordHashLen) == 0)
			{
				// AccountID�� ����
				Session->AccountId = AccountID;

				// �α��� ���̺� �α��� ���� �ִ��� Ȯ��
				CDBConnection* LoginCheckDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);
				SP::CLoginServerLoginDBGetAccount LoginCheck(*LoginCheckDBConnection);
				LoginCheck.InAccountID(Session->AccountId);
				LoginCheck.InAccountName(AccountLoginNameString);
				
				LoginCheck.OutLoginState(LoginState);

				LoginCheck.Execute();				

				bool LoginCheckSuccess = LoginCheck.Fetch();

				G_DBConnectionPool->Push(en_DBConnect::ACCOUNT, LoginCheckDBConnection);
				
				TIMESTAMP_STRUCT LoginSuccessTime;
				memset(&LoginSuccessTime, 0, sizeof(TIMESTAMP_STRUCT));
				
				time_t CurrentTime;
				time(&CurrentTime);

				// UTC �������� �ð��� �����״� 32400 ��ŭ ( KST + 9 ) �����ش�.
				CurrentTime += KST_TIME;

				tm UTCTime;
				gmtime_s(&UTCTime, &CurrentTime);

				// �α��� ���� �ð� ���ϱ�
				LoginSuccessTime.year = UTCTime.tm_year + UTC_YAER;
				LoginSuccessTime.month = UTCTime.tm_mon + UTC_MONTH;
				LoginSuccessTime.day = UTCTime.tm_mday;
				LoginSuccessTime.hour = UTCTime.tm_hour;
				LoginSuccessTime.minute = UTCTime.tm_min;
				LoginSuccessTime.second = UTCTime.tm_sec;
				LoginSuccessTime.fraction = 0;

				// �α��� ���̺� ���� ( = ���� ��� �ؾ��� )
				if (!LoginCheckSuccess)
				{
					// ��ū ���� ����
					CurrentTime += (TOKEN_EXPIRED_TIME);

					tm TokenTime;
					gmtime_s(&TokenTime, &CurrentTime);
						
					// ��ū ��ȿ �ð� ����
					TIMESTAMP_STRUCT TokenExpiredTime;
					TokenExpiredTime.year = TokenTime.tm_year + UTC_YAER;
					TokenExpiredTime.month = TokenTime.tm_mon + UTC_MONTH;
					TokenExpiredTime.day = TokenTime.tm_mday;
					TokenExpiredTime.hour = TokenTime.tm_hour;
					TokenExpiredTime.minute = TokenTime.tm_min;
					TokenExpiredTime.second = TokenTime.tm_sec;
					TokenExpiredTime.fraction = 0;

					// Hash�� ���ϱ�
					SHA256_Encrpyt((unsigned char*)OutPassword, PasswordHashLen, (unsigned char*)TokenHash);

					CDBConnection* TokenNewDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);
					SP::CLoginServerDBTokenNew TokenNew(*TokenNewDBConnection);
					TokenNew.InAccountID(Session->AccountId);
					TokenNew.InToken(TokenHash);
					TokenNew.InTokenCreateTime(LoginSuccessTime);
					TokenNew.InTokenExpiredTime(TokenExpiredTime);

					TokenNew.Execute();

					G_DBConnectionPool->Push(en_DBConnect::ACCOUNT, TokenNewDBConnection);

					LoginState = 1;
					// �α��� ���̺� ���
					CDBConnection* AccountLoginDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);
					SP::CLoginServerDBAccountLogin AccountLogin(*AccountLoginDBConnection);
					AccountLogin.InLoginState(LoginState);
					AccountLogin.InAccountID(Session->AccountId);
					AccountLogin.InAccountName(AccountLoginNameString);
					AccountLogin.InPassword(OutPassword);					

					AccountLogin.Execute();

					G_DBConnectionPool->Push(en_DBConnect::ACCOUNT, AccountLoginDBConnection);										

					// �α��� ����
					LoginInfo = en_LoginInfo::LOGIN_ACCOUNT_LOGIN_SUCCESS;

					ServerLists = GetServerList();
				}
				else
				{
					// �α��� ���̺� ���� 

					// �α��� ���� Ȯ��							
					switch ((en_LoginState)LoginState)
					{
					// �α��� ��
					case en_LoginState::LOGIN_IN:
						// �ߺ� �α���
						LoginInfo = en_LoginInfo::LOGIN_ACCOUNT_OVERLAP;
						break;
					// �α׾ƿ� ��
					case en_LoginState::LOGIN_OUT:
						{
							// ��ū�� �ִ��� Ȯ��
							CDBConnection* TokenDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);
							SP::CTokenServerDBIsToken TokenGet(*TokenDBConnection);

							TokenGet.InAccountID(Session->AccountId);

							TIMESTAMP_STRUCT DBTokenCreateTime;
							BYTE OutToken[TOKEN_MAX_LENGTH] = { 0 };

							TokenGet.OutTokenTime(DBTokenCreateTime);
							TokenGet.OutToken(OutToken);

							TokenGet.Execute();

							bool TokenGetSuccess = TokenGet.Fetch();

							G_DBConnectionPool->Push(en_DBConnect::ACCOUNT, TokenDBConnection);

							// ��ū�� ã�� ( = ��ū ���� )
							if (TokenGetSuccess == true)
							{
								tm TokenTime;
																
								TokenTime.tm_year = DBTokenCreateTime.year - UTC_YAER;
								TokenTime.tm_mon = DBTokenCreateTime.month - UTC_MONTH;
								TokenTime.tm_mday = DBTokenCreateTime.day;
								TokenTime.tm_hour = DBTokenCreateTime.hour;
								TokenTime.tm_min = DBTokenCreateTime.minute;
								TokenTime.tm_sec = DBTokenCreateTime.second;

								time_t TokenTimeT = mktime(&TokenTime);

								time_t Deltatime = CurrentTime - ( TokenTimeT + KST_TIME);

								// ��ū �߸����� 600�� ������ ��� ���� ����
								if (Deltatime >= TOKEN_EXPIRED_TIME)
								{
									CurrentTime += (TOKEN_EXPIRED_TIME);

									tm TokenTime;
									gmtime_s(&TokenTime, &CurrentTime);

									TIMESTAMP_STRUCT TokenExpiredTime;
									TokenExpiredTime.year = TokenTime.tm_year + UTC_YAER;
									TokenExpiredTime.month = TokenTime.tm_mon + UTC_MONTH;
									TokenExpiredTime.day = TokenTime.tm_mday;
									TokenExpiredTime.hour = TokenTime.tm_hour;
									TokenExpiredTime.minute = TokenTime.tm_min;
									TokenExpiredTime.second = TokenTime.tm_sec;
									TokenExpiredTime.fraction = 0;

									SHA256_Encrpyt((unsigned char*)OutPassword, PasswordHashLen, (unsigned char*)TokenHash);

									CDBConnection* TokenUpdateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);
									SP::CLoginServerDBTokenUpdate TokenUpdate(*TokenUpdateDBConnection);

									TokenUpdate.InAccountID(Session->AccountId);
									TokenUpdate.InToken(TokenHash);
									TokenUpdate.InTokenCreateTime(LoginSuccessTime);
									TokenUpdate.InTokenExpiredTime(TokenExpiredTime);

									TokenUpdate.Execute();
								}
								else
								{
									// ����� ���� �ʾƵ� �� ��� 
									memcpy(TokenHash, OutToken, TOKEN_MAX_LENGTH);
								}
							}	

							LoginState = 1;

							CDBConnection* LoginStateUpdateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);
							SP::CLoginServerLoginDBLoginStateUpdate LoginStateUpdate(*LoginStateUpdateDBConnection);
							LoginStateUpdate.InAccountID(Session->AccountId);
							LoginStateUpdate.InLoginState(LoginState);

							LoginStateUpdate.Execute();

							G_DBConnectionPool->Push(en_DBConnect::ACCOUNT, LoginStateUpdateDBConnection);

							LoginInfo = en_LoginInfo::LOGIN_ACCOUNT_LOGIN_SUCCESS;

							ServerLists = GetServerList();
						}
						break;
					}					
				}
			}
			else
			{
				// ��й�ȣ�� �ٸ�
				LoginInfo = en_LoginInfo::LOGIN_ACCOUNT_DIFFERENT_PASSWORD;
			}			
		}	
		else
		{
			// ������ ����
			LoginInfo = en_LoginInfo::LOGIN_ACCOUNT_NOT_EXIST;
		}

		CMessage* ResAccountLoginMessage = MakePacketResAccountLogin(LoginInfo, Session->AccountId, AccountLoginNameString, TOKEN_MAX_LENGTH, TokenHash, ServerLists);
		SendPacket(Session->SessionId, ResAccountLoginMessage);
		ResAccountLoginMessage->Free();		

		free(AccountLoginName);
		free(Password);		

		ReturnSession(Session);
	}
}

void CLoginServer::AccountLogOut(int64 SessionID, CMessage* Packet)
{
	int64 AccountID;
	*Packet >> AccountID;

	int8 LoginState = 0;

	CDBConnection* LoginStateUpdateDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);
	SP::CLoginServerLoginDBLoginStateUpdate LoginStateUpdate(*LoginStateUpdateDBConnection);
	LoginStateUpdate.InAccountID(AccountID);
	LoginStateUpdate.InLoginState(LoginState);

	LoginStateUpdate.Execute();

	G_DBConnectionPool->Push(en_DBConnect::ACCOUNT, LoginStateUpdateDBConnection);
}

void CLoginServer::DeleteClient(int64 SessionID)
{

}

vector<st_ServerInfo> CLoginServer::GetServerList()
{
	CDBConnection* GetServerListDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);
	SP::CLoginServerDBGetServerList GetServerList(*GetServerListDBConnection);

	WCHAR ServerName[20] = { 0 };
	WCHAR ServerIP[40] = { 0 };
	int32 ServerPort = 0;
	float ServerBusy = 0;

	GetServerList.OutServerName(ServerName);
	GetServerList.OutServerIP(ServerIP);
	GetServerList.OutServerPort(ServerPort);
	GetServerList.OutServerBusy(ServerBusy);

	GetServerList.Execute();

	vector<st_ServerInfo> ServerLists;

	while (GetServerList.Fetch())
	{
		st_ServerInfo ServerList;
		ServerList.ServerName = ServerName;
		ServerList.ServerIP = ServerIP;
		ServerList.ServerPort = ServerPort;
		ServerList.ServerBusy = ServerBusy;

		ServerLists.push_back(ServerList);
	}

	G_DBConnectionPool->Push(en_DBConnect::ACCOUNT, GetServerListDBConnection);

	return ServerLists;
}

CMessage* CLoginServer::MakePacketResAccountNew(bool AccountNewSuccess)
{
	CMessage* ResAccountNewPacket = CMessage::Alloc();
	if (ResAccountNewPacket == nullptr)
	{
		return nullptr;
	}

	ResAccountNewPacket->Clear();

	*ResAccountNewPacket << (int16)en_LOGIN_SERVER_PACKET_TYPE::en_LOGIN_SERVER_S2C_ACCOUNT_NEW;
	*ResAccountNewPacket << AccountNewSuccess;

	return ResAccountNewPacket;
}

CMessage* CLoginServer::MakePacketResAccountLogin(en_LoginInfo LoginInfo, int64 AccountID, wstring AccountName, int8 TokenLen, BYTE* Token, vector<st_ServerInfo> ServerLists)
{
	CMessage* ResAccountLoginPacket = CMessage::Alloc();
	if (ResAccountLoginPacket == nullptr)
	{
		return nullptr;
	}

	ResAccountLoginPacket->Clear();

	*ResAccountLoginPacket << (int16)en_LOGIN_SERVER_PACKET_TYPE::en_LOGIN_SERVER_S2C_ACCOUNT_LOGIN;
	*ResAccountLoginPacket << (int8)LoginInfo;
	*ResAccountLoginPacket << AccountID;
	
	int8 AccountNameLen = (int8)AccountName.length() * 2;
	*ResAccountLoginPacket << AccountNameLen;
	ResAccountLoginPacket->InsertData(AccountName.c_str(), AccountNameLen);

	*ResAccountLoginPacket << TokenLen;

	for (int8 i = 0; i < TokenLen; i++)
	{
		*ResAccountLoginPacket << Token[i];
	}

	int8 ServerListSize = (int8)ServerLists.size();

	*ResAccountLoginPacket << ServerListSize;

	for (int8 i = 0; i < ServerListSize; i++)
	{
		int8 ServerNameLen = (int8)ServerLists[i].ServerName.length() * 2;
		*ResAccountLoginPacket << ServerNameLen;
		ResAccountLoginPacket->InsertData(ServerLists[i].ServerName.c_str(), ServerNameLen);

		int8 ServerIPLen = (int8)ServerLists[i].ServerIP.length() * 2;
		*ResAccountLoginPacket << ServerIPLen;
		ResAccountLoginPacket->InsertData(ServerLists[i].ServerIP.c_str(), ServerIPLen);

		*ResAccountLoginPacket << ServerLists[i].ServerPort;
		*ResAccountLoginPacket << ServerLists[i].ServerBusy;
	}	

	return ResAccountLoginPacket;
}