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
	// DB 동기화 
	CDBConnection* GameServerDBConnection = G_DBConnectionPool->Pop(en_DBConnect::GAME);
	DBSynchronizer GameServerDBSync(*GameServerDBConnection);
	GameServerDBSync.Synchronize(L"GameServerDB.xml");

	G_DBConnectionPool->Push(en_DBConnect::GAME, GameServerDBConnection);

	// 서버 초기화
	vector<st_ServerInfo> ServerInfos = ServerConfigParser(L"GameServerConfig.xml");

	for (int32 i = 0; i < ServerInfos.size(); i++)
	{
		if (ServerInfos[i].ServerName == L"GameServer")
		{
			G_GameServer.GameServerStart(ServerInfos[i].IP.c_str(), ServerInfos[i].Port);
		}

		if (ServerInfos[i].ServerName == L"LoginServer")
		{
			G_GameServer.LoginServerConnect(ServerInfos[i].IP.c_str(), ServerInfos[i].Port);
		}
	}		
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
		G_Logger->WriteStdOut(en_Color::WHITE, L"AcceptTotal      : [%lld]\n", G_GameServer._AcceptTotal);  // 총 Accept 수
		G_Logger->WriteStdOut(en_Color::WHITE, L"ConnectSession   : [%zd]\n", G_GameServer._SessionCount); // 세션 수		
		G_Logger->WriteStdOut(en_Color::WHITE, L"AcceptTPS        : [%d]\n", G_GameServer._AcceptTPS);      // Accept TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"RecvPacket TPS   : [%d]\n", G_GameServer._RecvPacketTPS);  // Recv TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"SendPacket TPS   : [%d]\n", G_GameServer._SendPacketTPS);  // Send TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Alloc : [%d]\n", CMessage::_ObjectPoolFreeList.GetAllocCount());  // 남아 있는 Alloc 횟수
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Remain : [%d]\n", CMessage::_ObjectPoolFreeList.GetUseCount());   // 남아 있는 청크 파편 개수
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Return : [%d]\n", CMessage::_ObjectPoolFreeList.ReturnCount());   // 반납되지 않은 메세지 개수
		G_Logger->WriteStdOut(en_Color::WHITE, L"================================================\n\n");
		G_Logger->WriteStdOut(en_Color::YELLOW, L"GameServer\n");
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerJobPool Alloc  : [%d]\n", G_GameServer._GameServerJobMemoryPool->GetAllocCount()); // 게임서버 잡 메모리풀 Alloc
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerJobPool Remain : [%d]\n", G_GameServer._GameServerJobMemoryPool->GetUseCount()); // 게임서버 잡 메모리풀 남아 있는 청크 파편 개수
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerJobPool Return : [%d]\n", G_GameServer._GameServerJobMemoryPool->ReturnCount()); // 게임서버 잡 메모리풀 반납되지 않은 청크 파편 개수
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerTimerJobPool Alloc : [%d]\n", G_GameServer._TimerJobMemoryPool->GetAllocCount()); // 게임서버 타이머 잡 메모리풀 Alloc
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerTimerJobPool Remain : [%d]\n", G_GameServer._TimerJobMemoryPool->GetUseCount()); // 게임서버 타이머 잡 메모리풀 남은 청크 파편 개수
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerTimerJobPool Return : [%d]\n", G_GameServer._TimerJobMemoryPool->ReturnCount()); // 게임서버 타이머 잡 메모리풀 반납되지 않은 청크 파편 개수		
		G_Logger->WriteStdOut(en_Color::WHITE, L"DataBaseThread Que Size :  [%d]\n", G_GameServer._GameServerUserDBThreadMessageQue.GetUseSize()); // 게임서버 데이터베이스 쓰레드 큐 사이즈
		G_Logger->WriteStdOut(en_Color::WHITE, L"DataBaseThread TPS :       [%d]\n", G_GameServer._UserDBThreadTPS); // 게임서버 데이터베이스 쓰레드 TPS		
		G_Logger->WriteStdOut(en_Color::WHITE, L"LogicThread TPS :       [%d]\n", G_GameServer._LogicThreadFPS); // 게임서버 로직 쓰레드 FPS		
		G_Logger->WriteStdOut(en_Color::WHITE, L"TimerJobThread Que Size :  [%d]\n", G_GameServer._TimerHeapJob->GetUseSize()); // 게임서버 데이터베이스 쓰레드 큐 사이즈
		G_Logger->WriteStdOut(en_Color::WHITE, L"TimerJobThread TPS :       [%d]\n", G_GameServer._TimerJobThreadTPS); // 게임서버 데이터베이스 활성화된 횟수
		G_Logger->WriteStdOut(en_Color::WHITE, L"TimerJobThread WakeCount : [%d]\n", G_GameServer._TimerJobThreadWakeCount); // 게임서버 데이터베이스 활성화된 횟수
		G_Logger->WriteStdOut(en_Color::WHITE, L"\n================================================\n\n");

		G_GameServer._AcceptTPS = 0;
		G_GameServer._RecvPacketTPS = 0;
		G_GameServer._SendPacketTPS = 0;
		G_GameServer._UserDBThreadTPS = 0;
		G_GameServer._LogicThreadFPS = 0;
		G_GameServer._TimerJobThreadTPS = 0;

		Sleep(1000);
	}

	return 0;
}