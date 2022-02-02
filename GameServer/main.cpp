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
			G_GameServer.GameServerStart(ServerInfos[i].IP.c_str(), ServerInfos[i].Port);
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
		G_Logger->WriteStdOut(en_Color::WHITE, L"AcceptTotal      : [%lld]\n", G_GameServer._AcceptTotal);  // �� Accept ��
		G_Logger->WriteStdOut(en_Color::WHITE, L"ConnectSession   : [%zd]\n", G_GameServer._SessionCount); // ���� ��		
		G_Logger->WriteStdOut(en_Color::WHITE, L"AcceptTPS        : [%d]\n", G_GameServer._AcceptTPS);      // Accept TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"RecvPacket TPS   : [%d]\n", G_GameServer._RecvPacketTPS);  // Recv TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"SendPacket TPS   : [%d]\n", G_GameServer._SendPacketTPS);  // Send TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Alloc : [%d]\n", CMessage::_ObjectPoolFreeList.GetAllocCount());  // ���� �ִ� Alloc Ƚ��
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Remain : [%d]\n", CMessage::_ObjectPoolFreeList.GetUseCount());   // ���� �ִ� ûũ ���� ����
		G_Logger->WriteStdOut(en_Color::WHITE, L"PacketPool Return : [%d]\n", CMessage::_ObjectPoolFreeList.ReturnCount());   // �ݳ����� ���� �޼��� ����
		G_Logger->WriteStdOut(en_Color::WHITE, L"================================================\n\n");
		G_Logger->WriteStdOut(en_Color::YELLOW, L"GameServer\n");
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerJobPool Alloc  : [%d]\n", G_GameServer._JobMemoryPool->GetAllocCount()); // ���Ӽ��� �� �޸�Ǯ Alloc
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerJobPool Remain : [%d]\n", G_GameServer._JobMemoryPool->GetUseCount()); // ���Ӽ��� �� �޸�Ǯ ���� �ִ� ûũ ���� ����
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerJobPool Return : [%d]\n", G_GameServer._JobMemoryPool->ReturnCount()); // ���Ӽ��� �� �޸�Ǯ �ݳ����� ���� ûũ ���� ����
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerTimerJobPool Alloc : [%d]\n", G_GameServer._TimerJobMemoryPool->GetAllocCount()); // ���Ӽ��� Ÿ�̸� �� �޸�Ǯ Alloc
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerTimerJobPool Remain : [%d]\n", G_GameServer._TimerJobMemoryPool->GetUseCount()); // ���Ӽ��� Ÿ�̸� �� �޸�Ǯ ���� ûũ ���� ����
		G_Logger->WriteStdOut(en_Color::WHITE, L"GameServerTimerJobPool Return : [%d]\n", G_GameServer._TimerJobMemoryPool->ReturnCount()); // ���Ӽ��� Ÿ�̸� �� �޸�Ǯ �ݳ����� ���� ûũ ���� ����
		G_Logger->WriteStdOut(en_Color::WHITE, L"AuthThread Que Size :      [%d]\n", G_GameServer._GameServerAuthThreadMessageQue.GetUseSize()); //���Ӽ��� ���� ������ ť ������
		G_Logger->WriteStdOut(en_Color::WHITE, L"AuthThread TPS :	   [%d]\n", G_GameServer._AuthThreadTPS); // ���Ӽ��� ���� ������ TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"AuthThread WakeCount :     [%d]\n", G_GameServer._AuthThreadWakeCount); // ���Ӽ��� ���� ������ Ȱ��ȭ�� Ƚ��
		G_Logger->WriteStdOut(en_Color::WHITE, L"DataBaseThread Que Size :  [%d]\n", G_GameServer._GameServerDataBaseThreadMessageQue.GetUseSize()); // ���Ӽ��� �����ͺ��̽� ������ ť ������
		G_Logger->WriteStdOut(en_Color::WHITE, L"DataBaseThread TPS :       [%d]\n", G_GameServer._DataBaseThreadTPS); // ���Ӽ��� �����ͺ��̽� ������ TPS
		G_Logger->WriteStdOut(en_Color::WHITE, L"DataBaseThread WakeCount : [%d]\n", G_GameServer._DataBaseThreadWakeCount); // ���Ӽ��� �����ͺ��̽� Ȱ��ȭ�� Ƚ��		
		G_Logger->WriteStdOut(en_Color::WHITE, L"TimerJobThread Que Size :  [%d]\n", G_GameServer._TimerHeapJob->GetUseSize()); // ���Ӽ��� �����ͺ��̽� ������ ť ������
		G_Logger->WriteStdOut(en_Color::WHITE, L"TimerJobThread TPS :       [%d]\n", G_GameServer._TimerJobThreadTPS); // ���Ӽ��� �����ͺ��̽� Ȱ��ȭ�� Ƚ��
		G_Logger->WriteStdOut(en_Color::WHITE, L"TimerJobThread WakeCount : [%d]\n", G_GameServer._TimerJobThreadWakeCount); // ���Ӽ��� �����ͺ��̽� Ȱ��ȭ�� Ƚ��
		G_Logger->WriteStdOut(en_Color::WHITE, L"\n================================================\n\n");

		G_GameServer._AcceptTPS = 0;
		G_GameServer._RecvPacketTPS = 0;
		G_GameServer._SendPacketTPS = 0;
		G_GameServer._AuthThreadTPS = 0;
		G_GameServer._DataBaseThreadTPS = 0;
		G_GameServer._TimerJobThreadTPS = 0;

		Sleep(1000);
	}

	return 0;
}