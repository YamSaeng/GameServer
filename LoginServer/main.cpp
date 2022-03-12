#include"LoginServer.h"
#include"XmlParser.h"
#include"DBConnectionPool.h"
#include"DBSynchronizer.h"

CLoginServer G_LoginServer;

vector<st_ServerInfo> ServerConfigParser(const wchar_t* FileName)
{
	vector<st_ServerInfo> ServerInfos;
	CXmlNode Root;
	CXmlParser Parser;

	if (Parser.ParseFromFile(FileName, Root) == false)
	{
		wprintf(L"%s를 열수 없습니다.", FileName);
		return ServerInfos;
	}

	vector<CXmlNode> Servers = Root.FindChildren(L"Server");

	for (int32 i = 0; i < Servers.size(); i++)
	{
		st_ServerInfo ServerInfo;
		wstring ServerName = Servers[i].GetStringAttr(L"name");
		ServerInfo.ServerName = ServerName;

		CXmlNode Option = Servers[i].FindChild(L"Option");
		ServerInfo.ServerIP = Option.GetStringAttr(L"IP");
		ServerInfo.ServerPort = Option.GetInt32Attr(L"PORT");

		ServerInfos.push_back(ServerInfo);
	}

	return ServerInfos;
}

void ServerInfoInit()
{
	// 서버 초기화
	vector<st_ServerInfo> ServerInfos = ServerConfigParser(L"LoginServerConfig.xml");

	for (int32 i = 0; i < ServerInfos.size(); i++)
	{
		if (ServerInfos[i].ServerName == L"LoginServer")
		{
			G_LoginServer.LoginServerStart(ServerInfos[i].ServerIP.c_str(), ServerInfos[i].ServerPort);
		}
	}	

	CDBConnection* LoginServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::ACCOUNT);
	DBSynchronizer LoginServerDBSync(*LoginServerDBConnection);
	LoginServerDBSync.Synchronize(L"LoginServerDB.xml");

	G_DBConnectionPool->Push(en_DBConnect::ACCOUNT, LoginServerDBConnection);

	wprintf(L"로그인서버 준비 완료\n");
}

int main()
{
	ServerInfoInit();

	SYSTEMTIME NowTime;
	GetLocalTime(&NowTime);

	while (1)
	{
		G_Logger->WriteStdOut(en_Color::WHITE, L"================================================\n\n");
		G_Logger->WriteStdOut(en_Color::RED, L"Login ServerStart Time [%04d-%02d-%02d %02d:%02d:%02d] \n", NowTime.wYear, NowTime.wMonth, NowTime.wDay, NowTime.wHour, NowTime.wMinute, NowTime.wSecond);
		G_Logger->WriteStdOut(en_Color::WHITE, L"================================================\n\n");
		G_Logger->WriteStdOut(en_Color::YELLOW, L"NetworkLib\n");
		G_Logger->WriteStdOut(en_Color::WHITE, L"AcceptTotal      : [%lld]\n", G_LoginServer._AcceptTotal);  // 총 Accept 수
		G_Logger->WriteStdOut(en_Color::WHITE, L"ConnectSession   : [%zd]\n", G_LoginServer._SessionCount); // 세션 수		
		G_Logger->WriteStdOut(en_Color::WHITE, L"AcceptTPS        : [%d]\n", G_LoginServer._AcceptTPS);      // Accept TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"RecvPacket TPS   : [%d]\n", G_LoginServer._RecvPacketTPS);  // Recv TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"SendPacket TPS   : [%d]\n", G_LoginServer._SendPacketTPS);  // Send TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Alloc : [%d]\n", CMessage::_ObjectPoolFreeList.GetAllocCount());  // 남아 있는 Alloc 횟수
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Remain : [%d]\n", CMessage::_ObjectPoolFreeList.GetUseCount());   // 남아 있는 청크 파편 개수
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Return : [%d]\n", CMessage::_ObjectPoolFreeList.ReturnCount());   // 반납되지 않은 메세지 개수
		G_Logger->WriteStdOut(en_Color::WHITE, L"================================================\n\n");
		
		G_LoginServer._AcceptTPS = 0;
		G_LoginServer._RecvPacketTPS = 0;
		G_LoginServer._SendPacketTPS = 0;

		Sleep(1000);
	}

	return 0;
}