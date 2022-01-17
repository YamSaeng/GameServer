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
		wprintf(L"SendPacketTPS : %ld\n", G_DummyClient._SendPacketTPS);
		wprintf(L"================================================\n\n");
		
		G_DummyClient._SendPacketTPS = 0;

		Sleep(1000);		
	}
}