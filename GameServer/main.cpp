#include "pch.h"
#include "GameServer.h"
#include "XmlParser.h"
#include "DBConnectionPool.h"
#include "DBSynchronizer.h"

struct st_ServerInfo
{
	wstring ServerName;
	wstring IP;
	int Port;
};

CGameServer G_GameServer;

vector<st_ServerInfo> ServerConfigParser(const wchar_t* FileName)
{
	vector<st_ServerInfo> ServerInfos;
	CXmlNode Root;
	CXmlParser Parser;

	if (Parser.ParseFromFile(FileName, Root) == false)
	{
		wprintf(L"ServerConfig.ini를 열수 없습니다.");
		return ServerInfos;
	}

	vector<CXmlNode> Servers = Root.FindChildren(L"Server");
	for (int32 i = 0; i < Servers.size(); i++)
	{
		st_ServerInfo ServerInfo;
		wstring ServerName = Servers[i].GetStringAttr(L"name");
		ServerInfo.ServerName = ServerName;

		CXmlNode Option = Servers[i].FindChild(L"Option");
		ServerInfo.IP = Option.GetStringAttr(L"IP");
		ServerInfo.Port = Option.GetInt32Attr(L"PORT");

		ServerInfos.push_back(ServerInfo);
	}

	return ServerInfos;
}

void ServerInfoInit()
{
	// 서버 초기화
	vector<st_ServerInfo> ServerInfos = ServerConfigParser(L"ServerConfig.xml");

	for (int32 i = 0; i < ServerInfos.size(); i++)
	{
		if (ServerInfos[i].ServerName == L"NetworkLib")
		{
			G_GameServer.Start(ServerInfos[i].IP.c_str(), ServerInfos[i].Port);
		}
	}

	// DB 초기화
	// xml에 기록되어 있는 것을 기준으로 DB를 업데이트 시켜준다.
	// 현재 토큰을 관리하는 서버는 c#에서 모델링을 하고 있어서 해당 서버의 저장 프로시저만 이곳에서 우선 관리한다.
	CDBConnection* TokenDBConnection = G_DBConnectionPool->Pop(en_DBConnect::TOKEN);
	DBSynchronizer TokenDBSync(*TokenDBConnection);	
	TokenDBSync.Synchronize(L"TokenDB.xml");

	CDBConnection* GameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
	DBSynchronizer GameServerDBSync(*GameServerDBConnection);
	GameServerDBSync.Synchronize(L"GameServerDB.xml");

	G_DBConnectionPool->Push(en_DBConnect::TOKEN, TokenDBConnection);
	G_DBConnectionPool->Push(en_DBConnect::GAME, GameServerDBConnection);
}

int main()
{		
	ServerInfoInit();		

	SYSTEMTIME NowTime;
	GetLocalTime(&NowTime);

	while (true)
	{
		G_Logger->WriteStdOut(en_Color::WHITE, L"================================================\n\n");
		G_Logger->WriteStdOut(en_Color::RED, L"ServerStart Time [%04d-%02d-%02d %02d:%02d:%02d] \n", NowTime.wYear, NowTime.wMonth, NowTime.wDay, NowTime.wHour, NowTime.wMinute, NowTime.wSecond);
		G_Logger->WriteStdOut(en_Color::WHITE, L"================================================\n\n");
		G_Logger->WriteStdOut(en_Color::YELLOW, L"NetworkLib\n");
		G_Logger->WriteStdOut(en_Color::WHITE, L"ConnectSession   : [%zd]\n", G_GameServer._SessionCount); // 세션 수
		G_Logger->WriteStdOut(en_Color::WHITE, L"AcceptTotal      : [%lld]\n", G_GameServer._AcceptTotal);  // 총 Accept 수
		G_Logger->WriteStdOut(en_Color::WHITE, L"AcceptTPS        : [%d]\n", G_GameServer._AcceptTPS);      // Accept TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"RecvPacket TPS   : [%d]\n", G_GameServer._RecvPacketTPS);  // Recv TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"SendPacket TPS   : [%d]\n", G_GameServer._SendPacketTPS);  // Send TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Alloc : [%d]\n", CMessage::_ObjectPoolFreeList.GetAllocCount());  // 패킷풀 Alloc
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Remain : [%d]\n", CMessage::_ObjectPoolFreeList.GetUseCount());    
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Return : [%d]\n", CMessage::_ObjectPoolFreeList.ReturnCount()); // 패킷반납 회수	
		G_Logger->WriteStdOut(en_Color::WHITE, L"================================================\n\n");
		G_Logger->WriteStdOut(en_Color::YELLOW, L"GameServer\n");
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerJobPool Alloc  : [%d]\n", G_GameServer._JobMemoryPool->GetAllocCount()); //게임서버 잡 메모리풀 Alloc
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerJobPool Remain : [%d]\n", G_GameServer._JobMemoryPool->GetUseCount());
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerJobPool Return : [%d]\n", G_GameServer._JobMemoryPool->ReturnCount());
		G_Logger->WriteStdOut(en_Color::WHITE, L"NetworkThread Que Size	:  [%d]\n", G_GameServer._GameServerNetworkThreadMessageQue.GetUseSize()); //게임서버 잡큐에 몇개 들어있는지
		G_Logger->WriteStdOut(en_Color::WHITE, L"NetworkThread TPS	:  [%d]\n", G_GameServer._NetworkThreadTPS); //게임서버 Network 쓰레드 TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"AuthThread Que Size :      [%d]\n", G_GameServer._GameServerAuthThreadMessageQue.GetUseSize());
		G_Logger->WriteStdOut(en_Color::WHITE, L"AuthThread TPS :	   [%d]\n", G_GameServer._AuthThreadTPS);
		G_Logger->WriteStdOut(en_Color::WHITE, L"AuthThread WakeCount :     [%d]\n", G_GameServer._AuthThreadWakeCount);
		G_Logger->WriteStdOut(en_Color::WHITE, L"\n================================================\n\n");

		G_GameServer._AcceptTPS = 0;
		G_GameServer._RecvPacketTPS = 0;
		G_GameServer._SendPacketTPS = 0;
		G_GameServer._NetworkThreadTPS = 0;
		G_GameServer._AuthThreadTPS = 0;

		Sleep(1000);
	}

	return 0;
}