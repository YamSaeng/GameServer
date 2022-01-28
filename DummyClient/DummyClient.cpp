#include "DummyClient.h"
#include <process.h>
#include <random>

CDummyClient::CDummyClient()
{
	WSADATA WSA;
	if (WSAStartup(MAKEWORD(2, 2), &WSA) != 0)
	{
		DWORD Error = WSAGetLastError();
		wprintf(L"WSAStartup Error %d\n", Error);
	}

	_ConnectThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	int64 DummyClientAcountId = 1000000;

	for (int ClientCount = DUMMY_CLIENT_MAX - 1; ClientCount >= 0; ClientCount--)
	{
		_ClientArray[ClientCount] = new st_Client;
		_ClientArray[ClientCount]->IOBlock = (st_IOBlock*)_aligned_malloc(sizeof(st_IOBlock), 16);
		memset(&_ClientArray[ClientCount]->SendPacket, 0, sizeof(CMessage*) * 500);
		_ClientArray[ClientCount]->IOBlock->IOCount = 0;
		_ClientArray[ClientCount]->AccountId = DummyClientAcountId++;
		_ClientArray[ClientCount]->SendPacketCount = 0;
		_ClientArray[ClientCount]->IsConnected = 0;
		_ClientArray[ClientCount]->IsReqLogin = false;
		_ClientArray[ClientCount]->IsReqMove = false;
		_ClientArrayIndexs.Push(ClientCount);
	}

	_ConnectThreadProcEnd = false;

	_DummyClientId = 1;
	_SendPacketTPS = 0;
	_RecvPacketTPS = 0;
	_ClientCount = 0;

	_DummyClientHCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (_DummyClientHCP == NULL)
	{
		DWORD Error = WSAGetLastError();
		wprintf(L"IOCP Create Error %d\n", Error);
	}

	_beginthreadex(NULL, 0, ConnectThreadProc, this, 0, NULL);
	
	SYSTEM_INFO SI;
	GetSystemInfo(&SI);

	for (int i = 0; i < (int)SI.dwNumberOfProcessors; i++)
	{
		_beginthreadex(NULL, 0, WorkerThreadProc, this, 0, NULL);
	}

	_beginthreadex(NULL, 0, SendThreadProc, this, 0, NULL);

	SetEvent(_ConnectThreadWakeEvent);
}

CDummyClient::~CDummyClient()
{
}

unsigned __stdcall CDummyClient::WorkerThreadProc(void* Argument)
{
	CDummyClient* Instance = (CDummyClient*)Argument;

	if (Instance)
	{
		st_Client* NotifyCompleteClient = nullptr;

		for (;;)
		{
			DWORD Transferred = 0;
			OVERLAPPED* DummyClientOverlapped = nullptr;
			int32 CompleteRet;
			DWORD GQCSError;

			do
			{
				CompleteRet = GetQueuedCompletionStatus(Instance->_DummyClientHCP, &Transferred,
					(PULONG_PTR)&NotifyCompleteClient, (LPOVERLAPPED*)&DummyClientOverlapped, INFINITE);

				if (DummyClientOverlapped == nullptr)
				{
					DWORD GQCSError = WSAGetLastError();
					wprintf(L"DummyClientOverlapped null %d\n", GQCSError);
					return -1;
				}

				if (Transferred == 0)
				{				
					NotifyCompleteClient->ServerSocket = INVALID_SOCKET;
					CancelIoEx((HANDLE)NotifyCompleteClient->CloseSocket, NULL);
					break;
				}

				if (DummyClientOverlapped == &NotifyCompleteClient->RecvOverlapped)
				{
					Instance->RecvNotifyComplete(NotifyCompleteClient, Transferred);
				}
				else if (DummyClientOverlapped == &NotifyCompleteClient->SendOverlapped)
				{
					Instance->SendNotifyComplete(NotifyCompleteClient);
				}
			} while (0);

			if (InterlockedDecrement64(&NotifyCompleteClient->IOBlock->IOCount) == 0)
			{
				Instance->ReleaseClient(NotifyCompleteClient);
			}
		}
	}

	return 0;
}

unsigned __stdcall CDummyClient::ConnectThreadProc(void* Argument)
{
	Sleep(10000);	

	CDummyClient* Instance = (CDummyClient*)Argument;	

	if (Instance)
	{
		int32 NewClientIndex;
		
		SOCKADDR_IN ServerAddr;
		ZeroMemory(&ServerAddr, sizeof(ServerAddr));
		ServerAddr.sin_family = AF_INET;
		InetPtonW(AF_INET, L"127.0.0.1", &ServerAddr.sin_addr);
		ServerAddr.sin_port = htons(7777);

		while(!Instance->_ConnectThreadProcEnd)
		{
			WaitForSingleObject(Instance->_ConnectThreadWakeEvent, INFINITE);

			while (Instance->_ClientArrayIndexs.Pop(&NewClientIndex))
			{
				st_Client* NewClient = Instance->_ClientArray[NewClientIndex];

				if (NewClient->ClientReConnectTime < GetTickCount64())
				{
					NewClient->ClientReConnectTime = GetTickCount64() + DUMMY_CLIENT_RE_CONNECT_TIME;					

					NewClient->ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

					int ConnectRet = connect(NewClient->ServerSocket, (SOCKADDR*)&ServerAddr, sizeof(NewClient->ServerAddr));
					if (ConnectRet == SOCKET_ERROR)
					{
						DWORD Error = WSAGetLastError();
						wprintf(L"connect Error %d \n", Error);												
						break;
					}					

					NewClient->IsReqLogin = 0;
					NewClient->IsReqMove = 0;
					NewClient->IsLogin = false;
					NewClient->IsEnterGame = false;

					Instance->_ConnectionTotal++;
					Instance->_ConnectTPS++;

					NewClient->ClientSendMessageTime = GetTickCount64() + DUMMY_CLIENT_SEND_TIME;

					memset(&NewClient->RecvOverlapped, 0, sizeof(OVERLAPPED));
					memset(&NewClient->SendOverlapped, 0, sizeof(OVERLAPPED));

					NewClient->CloseSocket = NewClient->ServerSocket;
					NewClient->ClientId = ADD_CLIENTID_INDEX(Instance->_DummyClientId, NewClientIndex);
					NewClient->ServerAddr = ServerAddr;
					NewClient->IsSend = 0;
					NewClient->IsCancelIO = false;

					HANDLE SocketIORet = CreateIoCompletionPort((HANDLE)NewClient->ServerSocket, Instance->_DummyClientHCP, (ULONG_PTR)NewClient, 0);

					Instance->_ClientArray[NewClientIndex]->IOBlock->IOCount++;
					Instance->_ClientArray[NewClientIndex]->IsDisconnect = false;
					Instance->_ClientArray[NewClientIndex]->IOBlock->IsRelease = 0;

					Instance->_DummyClientId++;

					Instance->RecvPost(NewClient, true);					

					NewClient->IsConnected = 1;					

					InterlockedIncrement64(&Instance->_ClientCount);	
					
					Sleep(0);
				}
				else
				{
					Instance->_ClientArrayIndexs.Push(NewClientIndex);
				}
			}
		}		
	}

	return 0;
}

unsigned __stdcall CDummyClient::SendThreadProc(void* Argument)
{
	Sleep(25000);

	CDummyClient* Instance = (CDummyClient*)Argument;

	if (Instance)
	{
		while (1)
		{
			for (int32 i = 0; i < DUMMY_CLIENT_MAX; i++)
			{				
				if (Instance->_ClientArray[i]->IsConnected == 1 && Instance->_ClientArray[i]->IsDisconnect == false)
				{
					if (Instance->_ClientArray[i]->IsLogin == true
						&& Instance->_ClientArray[i]->IsEnterGame == true
						&& Instance->_ClientArray[i]->ClientSendMessageTime < GetTickCount64())
					{
						Instance->_ClientArray[i]->ClientSendMessageTime = GetTickCount64() + DUMMY_CLIENT_SEND_TIME;

						random_device Seed;
						default_random_engine Eng(Seed());

						float DisconnectPoint = DUMMY_CLIENT_DISCONNECT / 1000.0f;
						bernoulli_distribution DisconnectPointCheck(DisconnectPoint);
						bool IsDisconnect = DisconnectPointCheck(Eng);

						CMessage* RandPacket = CMessage::Alloc();
						RandPacket->Clear();

						if (IsDisconnect)
						{
							Instance->_ClientArray[i]->ServerSocket = INVALID_SOCKET;
							Instance->_ClientArray[i]->IsDisconnect = true;

							Instance->Disconnect(Instance->_ClientArray[i]->ClientId);

							InterlockedIncrement64(&Instance->_DisconnectTPS);
						}
						else
						{
							random_device RandomSeed;
							mt19937 Gen(RandomSeed());

							uniform_int_distribution<int> RandomMessageTypeCreate(0, 1);
							uniform_int_distribution<int> RandomDirCreate(0, 3);

							int32 RandMessageType = RandomMessageTypeCreate(Gen);
							int8 RandDir;

							wstring SendChatMsg;
							int8 DummyChatLen;

							switch ((en_DummyClientMessage)RandMessageType)
							{
							case en_DummyClientMessage::CHAT_MSG:
								*RandPacket << (int16)en_PACKET_C2S_MESSAGE;
								*RandPacket << Instance->_ClientArray[i]->AccountId;
								*RandPacket << Instance->_ClientArray[i]->MyCharacterGameObjectInfo.ObjectId;

								swprintf_s(Instance->_ClientArray[i]->ChatMsg, sizeof(Instance->_ClientArray[i]->ChatMsg), L"[%s]가 [%d]를 보냈습니다.", Instance->_ClientArray[i]->LoginId.c_str(), Instance->_ClientArray[i]->MyCharacterGameObjectInfo.ObjectId, RandMessageType * i);
								SendChatMsg = Instance->_ClientArray[i]->ChatMsg;

								//wprintf(L"[%s]가 [%d]를 보냈습니다.\n", G_ClientArray[i].ChatMsg);

								DummyChatLen = (int8)(SendChatMsg.length() * 2);
								*RandPacket << DummyChatLen;
								RandPacket->InsertData(SendChatMsg.c_str(), DummyChatLen);

								Instance->SendPacket(Instance->_ClientArray[i]->ClientId, RandPacket);
								break;
							case en_DummyClientMessage::MOVE:	
								if (Instance->_ClientArray[i]->IsReqMove == 0)
								{
									InterlockedExchange(&Instance->_ClientArray[i]->IsReqMove, 1);									

									*RandPacket << (int16)en_PACKET_C2S_MOVE;
									*RandPacket << Instance->_ClientArray[i]->AccountId;
									*RandPacket << Instance->_ClientArray[i]->MyCharacterGameObjectInfo.ObjectId;
									*RandPacket << (int8)Instance->_ClientArray[i]->MyCharacterGameObjectInfo.ObjectPositionInfo.State;
									*RandPacket << Instance->_ClientArray[i]->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionX;
									*RandPacket << Instance->_ClientArray[i]->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionY;
									*RandPacket << (int8)Instance->_ClientArray[i]->MyCharacterGameObjectInfo.ObjectPositionInfo.MoveDir;

									RandDir = RandomDirCreate(Gen);

									*RandPacket << (int8)RandDir;
																		
									Instance->SendPacket(Instance->_ClientArray[i]->ClientId, RandPacket);
								}									
								break;
							default:
								break;
							}
						}

						RandPacket->Free();
					}
					else
					{
						if (Instance->_ClientArray[i]->IsReqLogin == false)
						{
							Instance->_ClientArray[i]->IsReqLogin = true;

							bool IsDummy = true;

							// 로그인 패킷 전송
							CMessage* ReqGameServerLoginPacket = CMessage::Alloc();
							ReqGameServerLoginPacket->Clear();

							*ReqGameServerLoginPacket << (WORD)en_PACKET_C2S_GAME_REQ_LOGIN;
							*ReqGameServerLoginPacket << Instance->_ClientArray[i]->AccountId;

							int8 DummyClientNameLen = (int8)(Instance->_ClientArray[i]->LoginId.length() * 2);
							*ReqGameServerLoginPacket << DummyClientNameLen;
							ReqGameServerLoginPacket->InsertData(Instance->_ClientArray[i]->LoginId.c_str(), DummyClientNameLen);

							*ReqGameServerLoginPacket << IsDummy;

							Instance->SendPacket(Instance->_ClientArray[i]->ClientId, ReqGameServerLoginPacket);
							ReqGameServerLoginPacket->Free();
						}						
					}
				}
			}	

			Sleep(200);
		}		
	}

	return 0;
}

void CDummyClient::ReleaseClient(st_Client* ReleaseClient)
{
	st_IOBlock CompareBlock;
	CompareBlock.IOCount = 0;
	CompareBlock.IsRelease = 0;

	if (!InterlockedCompareExchange128((LONG64*)ReleaseClient->IOBlock, (LONG64)true, (LONG64)0, (LONG64*)&CompareBlock))
	{
		return;
	}

	ReleaseClient->IsConnected = 0;

	ReleaseClient->RecvRingBuf.ClearBuffer();

	while (!ReleaseClient->SendRingBuf.IsEmpty())
	{
		CMessage* DeletePacket = nullptr;
		ReleaseClient->SendRingBuf.Dequeue(&DeletePacket);
		DeletePacket->Free();
	}

	for (int i = 0; i < ReleaseClient->SendPacketCount; i++)
	{
		ReleaseClient->SendPacket[i]->Free();
		ReleaseClient->SendPacket[i] = nullptr;
	}

	ReleaseClient->SendPacketCount = 0;

	if (ReleaseClient->SendRingBuf.GetUseSize() > 0)
	{
		int SendPacketCompleteCount = ReleaseClient->SendRingBuf.GetUseSize();
		for (int i = 0; i < SendPacketCompleteCount; i++)
		{
			CMessage* DeletePacket = nullptr;
			ReleaseClient->SendRingBuf.Dequeue(&DeletePacket);
			DeletePacket->Free();
		}
	}
		
	InterlockedDecrement64(&_ClientCount);	

	ReleaseClient->ServerSocket = INVALID_SOCKET;
	closesocket(ReleaseClient->CloseSocket);	

	ReleaseClient->ClientReConnectTime = GetTickCount64() + DUMMY_CLIENT_RE_CONNECT_TIME;
	ReleaseClient->ClientSendMessageTime = GetTickCount64() + 500;

	_ClientArrayIndexs.Push(GET_CLIENTINDEX(ReleaseClient->ClientId));

	//SetEvent(_ConnectThreadWakeEvent);
}

void CDummyClient::SendPost(st_Client* SendClient)
{
	int32 SendRingBufUseSize;
	int32 SendBufCount = 0;
	WSABUF SendBuf[500];

	do
	{
		if (InterlockedExchange(&SendClient->IsSend, 1) == 0)
		{
			SendRingBufUseSize = SendClient->SendRingBuf.GetUseSize();
			if (SendRingBufUseSize == 0)
			{
				InterlockedExchange(&SendClient->IsSend, 0);

				if (!SendClient->SendRingBuf.IsEmpty())
				{
					continue;
				}
				else
				{
					return;
				}
			}
		}
		else
		{
			return;
		}
	} while (0);

	SendClient->SendPacketCount = SendBufCount = SendRingBufUseSize;

	for (int16 i = 0; i < SendBufCount; i++)
	{
		CMessage* Packet = nullptr;
		if (!SendClient->SendRingBuf.Dequeue(&Packet))
		{
			break;
		}

		InterlockedIncrement(&_SendPacketTPS);

		SendBuf[i].buf = Packet->GetHeaderBufferPtr();
		SendBuf[i].len = Packet->GetUseBufferSize();

		SendClient->SendPacket[i] = Packet;
	}	

	memset(&SendClient->SendOverlapped, 0, sizeof(OVERLAPPED));

	InterlockedIncrement64(&SendClient->IOBlock->IOCount);

	int32 WSASendRetval = WSASend(SendClient->ServerSocket, SendBuf, SendBufCount, NULL, 0, (LPWSAOVERLAPPED)&SendClient->SendOverlapped, NULL);
	if (WSASendRetval == SOCKET_ERROR)
	{
		DWORD Error = WSAGetLastError();
		if (Error != ERROR_IO_PENDING)
		{

		}

		if (InterlockedDecrement64(&SendClient->IOBlock->IOCount) == 0)
		{
			ReleaseClient(SendClient);
		}
	}
}

void CDummyClient::RecvPost(st_Client* RecvClient, bool IsConnectRecvPost)
{
	int32 RecvBufCount = 0;
	WSABUF RecvBuf[2];

	int32 DirectEnqueSize = RecvClient->RecvRingBuf.GetDirectEnqueueSize();
	int32 RecvRingBufFreeSize = RecvClient->RecvRingBuf.GetFreeSize();

	if (RecvRingBufFreeSize > DirectEnqueSize)
	{
		RecvBufCount = 2;
		RecvBuf[0].buf = RecvClient->RecvRingBuf.GetRearBufferPtr();
		RecvBuf[0].len = DirectEnqueSize;

		RecvBuf[1].buf = RecvClient->RecvRingBuf.GetBufferPtr();
		RecvBuf[1].len = RecvRingBufFreeSize - DirectEnqueSize;
	}
	else
	{
		RecvBufCount = 1;
		RecvBuf[0].buf = RecvClient->RecvRingBuf.GetRearBufferPtr();
		RecvBuf[0].len = DirectEnqueSize;
	}	

	memset(&RecvClient->RecvOverlapped, 0, sizeof(OVERLAPPED));	

	if (IsConnectRecvPost == false)
	{
		InterlockedIncrement64(&RecvClient->IOBlock->IOCount);
	}	

	DWORD Flags = 0;

	int WSARecvRetval = WSARecv(RecvClient->ServerSocket, RecvBuf, RecvBufCount, NULL, &Flags, (LPWSAOVERLAPPED)&RecvClient->RecvOverlapped, NULL);	
	if (WSARecvRetval == SOCKET_ERROR)
	{
		DWORD Error = WSAGetLastError();		
		if (Error != ERROR_IO_PENDING)
		{
			if (Error == WSAENOBUFS)
			{

			}

			if (InterlockedDecrement64(&RecvClient->IOBlock->IOCount) == 0)
			{
				ReleaseClient(RecvClient);
			}
		}		
	}		
}

void CDummyClient::RecvNotifyComplete(st_Client* RecvCompleteClient, const DWORD& Transferred)
{
	RecvCompleteClient->RecvRingBuf.MoveRear(Transferred);

	CMessage::st_ENCODE_HEADER EncodeHeader;
	CMessage* Packet = CMessage::Alloc();

	for (;;)
	{
		Packet->Clear();

		if (RecvCompleteClient->RecvRingBuf.GetUseSize() < sizeof(CMessage::st_ENCODE_HEADER))
		{
			break;
		}

		RecvCompleteClient->RecvRingBuf.Peek((char*)&EncodeHeader, sizeof(CMessage::st_ENCODE_HEADER));
		if (EncodeHeader.PacketLen + sizeof(CMessage::st_ENCODE_HEADER) > RecvCompleteClient->RecvRingBuf.GetUseSize())
		{
			if (EncodeHeader.PacketCode != 119)
			{
				Disconnect(RecvCompleteClient->ClientId);
				break;
			}
		}

		InterlockedIncrement(&_RecvPacketTPS);

		RecvCompleteClient->RecvRingBuf.MoveFront(sizeof(CMessage::st_ENCODE_HEADER));
		RecvCompleteClient->RecvRingBuf.Dequeue(Packet->GetRearBufferPtr(), EncodeHeader.PacketLen);
		Packet->SetHeader((char*)&EncodeHeader, sizeof(CMessage::st_ENCODE_HEADER));
		Packet->MoveWritePosition(EncodeHeader.PacketLen);

		if (!Packet->Decode())
		{
			Disconnect(RecvCompleteClient->ClientId);
			break;
		}

		OnRecv(RecvCompleteClient->ClientId, Packet);
	}

	Packet->Free();

	RecvPost(RecvCompleteClient);
}

void CDummyClient::SendNotifyComplete(st_Client* SendCompleteClient)
{
	for (int32 i = 0; i < SendCompleteClient->SendPacketCount; i++)
	{
		SendCompleteClient->SendPacket[i]->Free();
	}

	SendCompleteClient->SendPacketCount = 0;

	InterlockedExchange(&SendCompleteClient->IsSend, 0);

	if (SendCompleteClient->SendRingBuf.GetUseSize() > 0)
	{
		SendPost(SendCompleteClient);
	}
}

void CDummyClient::OnRecv(int64 ClientID, CMessage* Packet)
{
	st_Client* RecvClient = FindClient(ClientID);

	if (RecvClient)
	{
		WORD MessageType;
		*Packet >> MessageType;

		switch (MessageType)
		{
		case en_PACKET_S2C_GAME_CLIENT_CONNECTED:
		{
			WCHAR DummyCharacterName[256] = { 0 };
			swprintf_s(DummyCharacterName, sizeof(DummyCharacterName), L"Dummy_Player %d", RecvClient->AccountId);

			RecvClient->LoginId = DummyCharacterName;
			RecvClient->DummyClientLoginTime = GetTickCount64() + DUMMY_CLIENT_LOGIN_TIME;			
		}
		break;
		case en_PACKET_S2C_GAME_RES_LOGIN:
		{
			bool LoginSuccess = false;
			*Packet >> LoginSuccess;

			RecvClient->IsLogin = LoginSuccess;

			if (LoginSuccess == true)
			{
				int8 PlayerCount;
				*Packet >> PlayerCount;

				if (PlayerCount > 0)
				{
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectId;

					int8 CharacterNameLen;
					*Packet >> CharacterNameLen;
					Packet->GetData(RecvClient->MyCharacterGameObjectInfo.ObjectName, CharacterNameLen);

					int8 CharacterState;
					*Packet >> CharacterState;

					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionX;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionY;

					int8 CharacterMoveDir;
					*Packet >> CharacterMoveDir;

					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.Level;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.HP;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MaxHP;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MP;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MaxMP;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.DP;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MaxDP;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MeleeAttackHitRate;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MagicDamage;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MagicHitRate;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.Defence;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.EvasionRate;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MeleeCriticalPoint;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MagicCriticalPoint;
					*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.Speed;


					int16 CharacterObjectType;
					*Packet >> CharacterObjectType;
					RecvClient->MyCharacterGameObjectInfo.ObjectType = (en_GameObjectType)CharacterObjectType;

					*Packet >> RecvClient->MyCharacterGameObjectInfo.OwnerObjectId;

					int16 CharacterOwnerObjectType;
					*Packet >> CharacterOwnerObjectType;
					RecvClient->MyCharacterGameObjectInfo.OwnerObjectType = (en_GameObjectType)CharacterOwnerObjectType;

					*Packet >> RecvClient->MyCharacterGameObjectInfo.PlayerSlotIndex;

					CMessage* ReqEnterGamePacket = CMessage::Alloc();

					ReqEnterGamePacket->Clear();

					*ReqEnterGamePacket << (int16)en_PACKET_C2S_GAME_ENTER;
					*ReqEnterGamePacket << RecvClient->AccountId;

					int8 DummyClientNameLen = (int8)(RecvClient->LoginId.length() * 2);
					*ReqEnterGamePacket << DummyClientNameLen;
					ReqEnterGamePacket->InsertData(RecvClient->LoginId.c_str(), DummyClientNameLen);

					SendPacket(ClientID, ReqEnterGamePacket);
					ReqEnterGamePacket->Free();
				}
				else
				{
					// 캐릭터 없으면 게임 캐릭터 생성 요청 패킷 전송
					CMessage* ReqCreateCharacterPacket = CMessage::Alloc();
					ReqCreateCharacterPacket->Clear();

					*ReqCreateCharacterPacket << (int16)en_PACKET_C2S_GAME_CREATE_CHARACTER;
					*ReqCreateCharacterPacket << (int16)en_GameObjectType::OBJECT_PLAYER_DUMMY;

					int8 DummyClientNameLen = (int8)(RecvClient->LoginId.length() * 2);
					*ReqCreateCharacterPacket << DummyClientNameLen;
					ReqCreateCharacterPacket->InsertData(RecvClient->LoginId.c_str(), DummyClientNameLen);

					*ReqCreateCharacterPacket << (int8)0;

					SendPacket(ClientID, ReqCreateCharacterPacket);
					ReqCreateCharacterPacket->Free();
				}
			}
		}
		break;
		case en_PACKET_S2C_GAME_CREATE_CHARACTER:
		{
			bool CharacterCreateSuccess = false;

			*Packet >> CharacterCreateSuccess;

			if (CharacterCreateSuccess == true)
			{
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectId;

				int8 CharacterNameLen;
				*Packet >> CharacterNameLen;
				Packet->GetData(RecvClient->MyCharacterGameObjectInfo.ObjectName, CharacterNameLen);

				int8 CharacterState;
				*Packet >> CharacterState;

				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionX;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionY;

				int8 CharacterMoveDir;
				*Packet >> CharacterMoveDir;

				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.Level;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.HP;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MaxHP;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MP;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MaxMP;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.DP;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MaxDP;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MeleeAttackHitRate;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MagicDamage;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MagicHitRate;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.Defence;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.EvasionRate;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MeleeCriticalPoint;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MagicCriticalPoint;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.Speed;

				int16 CharacterObjectType;
				*Packet >> CharacterObjectType;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.OwnerObjectId;

				int16 CharacterOwnerObjectType;
				*Packet >> CharacterOwnerObjectType;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.PlayerSlotIndex;

				CMessage* ReqEnterGamePacket = CMessage::Alloc();
				ReqEnterGamePacket->Clear();

				*ReqEnterGamePacket << (int16)en_PACKET_C2S_GAME_ENTER;
				*ReqEnterGamePacket << RecvClient->AccountId;

				int8 DummyClientNameLen = (int8)(RecvClient->LoginId.length() * 2);
				*ReqEnterGamePacket << DummyClientNameLen;
				ReqEnterGamePacket->InsertData(RecvClient->LoginId.c_str(), DummyClientNameLen);

				SendPacket(ClientID, ReqEnterGamePacket);
				ReqEnterGamePacket->Free();
			}
		}
		break;
		case en_PACKET_S2C_GAME_ENTER:
		{
			bool GameEnterSuccess;
			*Packet >> GameEnterSuccess;		
			
			if (GameEnterSuccess == true)
			{
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectId;

				int8 CharacterNameLen;
				*Packet >> CharacterNameLen;
				Packet->GetData(RecvClient->MyCharacterGameObjectInfo.ObjectName, CharacterNameLen);

				int8 CharacterState;
				*Packet >> CharacterState;

				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionX;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionY;

				int8 CharacterMoveDir;
				*Packet >> CharacterMoveDir;

				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.Level;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.HP;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MaxHP;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MP;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MaxMP;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.DP;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MaxDP;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.AutoRecoveryHPPercent;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.AutoRecoveryMPPercent;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MinMeleeAttackDamage;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MaxMeleeAttackDamage;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MeleeAttackHitRate;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MagicDamage;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MagicHitRate;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.Defence;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.EvasionRate;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MeleeCriticalPoint;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.MagicCriticalPoint;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.ObjectStatInfo.Speed;

				int16 CharacterObjectType;
				*Packet >> CharacterObjectType;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.OwnerObjectId;

				int16 CharacterOwnerObjectType;
				*Packet >> CharacterOwnerObjectType;
				*Packet >> RecvClient->MyCharacterGameObjectInfo.PlayerSlotIndex;

				RecvClient->IsEnterGame = true;
			}
			else 
			{
				RecvClient->IsEnterGame = false;

				CMessage* ReqEnterGamePacket = CMessage::Alloc();
				ReqEnterGamePacket->Clear();

				*ReqEnterGamePacket << (int16)en_PACKET_C2S_GAME_ENTER;
				*ReqEnterGamePacket << RecvClient->AccountId;

				int8 DummyClientNameLen = (int8)(RecvClient->LoginId.length() * 2);
				*ReqEnterGamePacket << DummyClientNameLen;
				ReqEnterGamePacket->InsertData(RecvClient->LoginId.c_str(), DummyClientNameLen);

				SendPacket(ClientID, ReqEnterGamePacket);
				ReqEnterGamePacket->Free();
			}
		}
		break;
		case en_PACKET_S2C_MESSAGE:
			break;
		case en_PACKET_S2C_MOVE:
		{			
			int64 AccountId;
			int64 PlayerId;
			int16 ObjectType;
			int8 CreatureState;
			int32 PositionX;
			int32 PositionY;
			int8 MoveDir;

			*Packet >> AccountId;
			*Packet >> PlayerId;
			*Packet >> ObjectType;
			*Packet >> CreatureState;
			*Packet >> PositionX;
			*Packet >> PositionY;
			*Packet >> MoveDir;		

			st_Client* Client = FindById(PlayerId);

			if (Client)
			{
				Client->MyCharacterGameObjectInfo.ObjectPositionInfo.State = (en_CreatureState)CreatureState;
				Client->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionX = PositionX;
				Client->MyCharacterGameObjectInfo.ObjectPositionInfo.PositionY = PositionY;
				Client->MyCharacterGameObjectInfo.ObjectPositionInfo.MoveDir = (en_MoveDir)MoveDir;

				// 내가 요청한 움직임
				if (RecvClient->AccountId == Client->AccountId)
				{
					InterlockedExchange(&RecvClient->IsReqMove, 0);
				}							
			}							
		}
		break;
		case en_PACKET_S2C_PING:
		{
			CMessage* ReqPongPacket = CMessage::Alloc();
			ReqPongPacket->Clear();

			*ReqPongPacket << (int16)en_PACKET_TYPE::en_PACKET_C2S_PONG;

			SendPacket(ClientID, ReqPongPacket);
			ReqPongPacket->Free();
		}
		break;
		default:
			break;
		}

		ReturnClient(RecvClient);
	}	
}

st_Client* CDummyClient::FindClient(int64 ClientID)
{
	int ClientIndex = GET_CLIENTINDEX(ClientID);
	
	if (_ClientArray[ClientIndex]->IOBlock->IsRelease == 1)
	{
		return nullptr;
	}

	/*
		안전하게 증가를 시켰는데 IOCount의 값이 1이라면 릴리즈 작업중이거나 릴리즈를 작업할 예정인 세션(0에서 증가 한 것이니까)이므로
		확인 작업을 해준다. 위에서 Increment로 증가시켜줬으니까 1을 다시 감소시켜서 그값이 0이라면 ReleaseSession을 호출해준다.
	*/
	if (InterlockedIncrement64(&_ClientArray[ClientIndex]->IOBlock->IOCount) == 1)
	{
		if (InterlockedDecrement64(&_ClientArray[ClientIndex]->IOBlock->IOCount) == 0)
		{
			//LOG(L"ERROR", en_LOG_LEVEL::LEVEL_ERROR, L"IOCount is 0!!!!!!");
			ReleaseClient(_ClientArray[ClientIndex]);
		}

		return nullptr;
	}

	/*
		찾을 세션의 세션 아이디가 입력받은 SessionID와 다르다면
	*/
	if (ClientID != _ClientArray[ClientIndex]->ClientId)
	{
		if (InterlockedDecrement64(&_ClientArray[ClientIndex]->IOBlock->IOCount) == 0)
		{
			ReleaseClient(_ClientArray[ClientIndex]);
		}

		return nullptr;
	}

	return _ClientArray[ClientIndex];
}

void CDummyClient::ReturnClient(st_Client* Client)
{
	if (InterlockedDecrement64(&Client->IOBlock->IOCount) == 0)
	{
		ReleaseClient(Client);
	}
}

void CDummyClient::SendPacket(int64 ClientID, CMessage* Packet)
{
	st_Client* SendClient = FindClient(ClientID);
	if (SendClient == nullptr)
	{
		return;
	}

	Packet->Encode();
	Packet->AddRetCount();

	SendClient->SendRingBuf.Enqueue(Packet);
	
	SendPost(SendClient);

	ReturnClient(SendClient);
}

void CDummyClient::Disconnect(int64 ClientID)
{
	st_Client* DisconnectClient = FindClient(ClientID);
	if (DisconnectClient != nullptr)
	{
		if (DisconnectClient->IOBlock->IOCount == 0)
		{
			ReturnClient(DisconnectClient);
		}
		else
		{	
			DisconnectClient->IsCancelIO = true;
			CancelIoEx((HANDLE)DisconnectClient->CloseSocket, NULL);						
		}

		ReturnClient(DisconnectClient);
	}	
}

st_Client* CDummyClient::FindById(int64& ObjectId)
{
	for (int i = 0; i < DUMMY_CLIENT_MAX; i++)
	{
		if (_ClientArray[i]->MyCharacterGameObjectInfo.ObjectId == ObjectId)
		{			
			return _ClientArray[i];
		}
	}

	return nullptr;
}
