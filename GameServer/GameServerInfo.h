#pragma once
#include <winnt.h>
#include <minwindef.h>
#include "Player.h"
#include "GameServerMessage.h"

#define LOGIN_SUCCESS 1
#define LOGIN_FAIL 0

enum class en_LoginState : int8
{
	LOGIN_OUT,
	LOGIN_IN,
	LOGIN_DB_WORKING
};

enum class en_GameServerJobType : int16
{
	AUTH_NEW_CLIENT_JOIN = 0,
	AUTH_DISCONNECT_CLIENT = 1,
	AUTH_MESSAGE = 2,

	NETWORK_MESSAGE = 3,	

	DATA_BASE_ACCOUNT_CHECK = 100,
	DATA_BASE_CHARACTER_CHECK,			
	DATA_BASE_CHARACTER_INFO_SEND,	
	DATA_BASE_CHATACTER_INFO_SAVE,

	OBJECT_SPAWN,
	OBJECT_DESAPWN
};

struct st_GameServerJob
{
	en_GameServerJobType Type;	
	CGameServerMessage* Message = nullptr;
	st_Session* Session = nullptr;
};

enum class en_TimerJobType : int16
{
	TIMER_JOB_TYPE_NONE,		
	TIMER_PING
};

struct st_TimerJob
{
	en_TimerJobType TimerJobType;
	int64 SessionId;
	CGameServerMessage* TimerJobMessage = nullptr;
	st_Session* Session = nullptr;
	int64 TimerJobExecTick;		 // 타이머 잡 실행 시간
	bool TimerJobCancel = false; // 타이머 잡 취소 변수	
};