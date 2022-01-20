#pragma comment(lib,"winmm")
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <xstring>
#include <vector>
#include "RingBuffer.h"
#include "Message.h"
#include "CommonProtocol.h"
#include "LockFreeQue.h"
#include "DummyClient.h"
#include <process.h>

CDummyClient G_DummyClient;

using namespace std;

int main()
{	
	SYSTEMTIME NowTime;
	GetLocalTime(&NowTime);

	while (1)
	{
		wprintf(L"DummyClientStart Time [%04d-%02d-%02d %02d:%02d:%02d] \n", NowTime.wYear, NowTime.wMonth, NowTime.wDay, NowTime.wHour, NowTime.wMinute, NowTime.wSecond);
		wprintf(L"================================================\n");
		wprintf(L"Connect Client : [%ld]\n", G_DummyClient._ClientCount);
		wprintf(L"Connect Total  : [%ld]\n", G_DummyClient._ConnectionTotal);
		wprintf(L"ConnectTPS     : [%ld]\n", G_DummyClient._ConnectTPS);
		wprintf(L"SendPacketTPS  : [%ld]\n", G_DummyClient._SendPacketTPS);
		wprintf(L"RecvPacketTPS  : [%ld]\n", G_DummyClient._RecvPacketTPS);
		wprintf(L"DisconnectTPS  : [%ld]\n", G_DummyClient._DisconnectTPS);
		wprintf(L"================================================\n\n");
		
		G_DummyClient._ConnectTPS = 0;
		G_DummyClient._SendPacketTPS = 0;
		G_DummyClient._RecvPacketTPS = 0;
		G_DummyClient._DisconnectTPS = 0;

		Sleep(1000);		
	}
}