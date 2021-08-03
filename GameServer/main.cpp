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
		wprintf(L"ServerConfig.ini�� ���� �����ϴ�.");
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
	// ���� �ʱ�ȭ
	vector<st_ServerInfo> ServerInfos = ServerConfigParser(L"ServerConfig.xml");

	for (int32 i = 0; i < ServerInfos.size(); i++)
	{
		if (ServerInfos[i].ServerName == L"NetworkLib")
		{
			G_GameServer.Start(ServerInfos[i].IP.c_str(), ServerInfos[i].Port);
		}
	}

	// DB �ʱ�ȭ
	// xml�� ��ϵǾ� �ִ� ���� �������� DB�� ������Ʈ �����ش�.
	// ���� ��ū�� �����ϴ� ������ c#���� �𵨸��� �ϰ� �־ �ش� ������ ���� ���ν����� �̰����� �켱 �����Ѵ�.
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
		G_Logger->WriteStdOut(en_Color::WHITE, L"ConnectSession   : [%zd]\n", G_GameServer._SessionCount); // ���� ��
		G_Logger->WriteStdOut(en_Color::WHITE, L"AcceptTotal      : [%lld]\n", G_GameServer._AcceptTotal);  // �� Accept ��
		G_Logger->WriteStdOut(en_Color::WHITE, L"AcceptTPS        : [%d]\n", G_GameServer._AcceptTPS);      // Accept TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"RecvPacket TPS   : [%d]\n", G_GameServer._RecvPacketTPS);  // Recv TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"SendPacket TPS   : [%d]\n", G_GameServer._SendPacketTPS);  // Send TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Alloc : [%d]\n", CMessage::_ObjectPoolFreeList.GetAllocCount());  // ��ŶǮ Alloc
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Use   : [%d]\n", CMessage::_ObjectPoolFreeList.GetUseCount());    // ������� ��Ŷ Ƚ��
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Return : [%d]\n", CMessage::_ObjectPoolFreeList.ReturnCount()); // ��Ŷ�ݳ� ȸ��	
		G_Logger->WriteStdOut(en_Color::WHITE, L"================================================\n\n");
		G_Logger->WriteStdOut(en_Color::YELLOW, L"ChattingServer\n");
		G_Logger->WriteStdOut(en_Color::WHITE, L"PlayerPool Alloc	: [%d]\n", G_GameServer._ClientMemoryPool->GetAllocCount()); //ä�ü��� Ŭ�� �޸�Ǯ Alloc
		G_Logger->WriteStdOut(en_Color::WHITE, L"PlayerPool Use		: [%d]\n", G_GameServer._ClientMemoryPool->GetUseCount());
		G_Logger->WriteStdOut(en_Color::WHITE, L"PlayerCount		: [%d]\n", G_GameServer._ClientMap.size()); // ä�ü��� Ŭ�� ����
		G_Logger->WriteStdOut(en_Color::WHITE, L"ChatServerJobPool Alloc	: [%d]\n", G_GameServer._JobMemoryPool->GetAllocCount()); //ä�ü��� �� �޸�Ǯ Alloc
		G_Logger->WriteStdOut(en_Color::WHITE, L"ChatServerJobPool Use	: [%d]\n", G_GameServer._JobMemoryPool->GetUseCount());
		G_Logger->WriteStdOut(en_Color::WHITE, L"ChatServerJobQue Size	: [%d]\n", G_GameServer._GameServerCommonMessageQue.GetUseSize()); //ä�ü��� ��ť�� � ����ִ���
		G_Logger->WriteStdOut(en_Color::WHITE, L"UpdateThread TPS	: [%d]\n", G_GameServer._UpdateTPS); //ä�ü��� ������Ʈ ������ TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"UpdateThread Wake Count : [%d]\n", G_GameServer._UpdateWakeCount); //ä�ü��� ������Ʈ �����尡 ��� �Ͼ����
		G_Logger->WriteStdOut(en_Color::WHITE, L"\n================================================\n\n");

		G_GameServer._AcceptTPS = 0;
		G_GameServer._RecvPacketTPS = 0;
		G_GameServer._SendPacketTPS = 0;
		G_GameServer._UpdateTPS = 0;

		Sleep(1000);
	}

	return 0;
}