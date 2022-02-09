#pragma once
#include <winnt.h>
#include <minwindef.h>
#include "Player.h"
#include "GameServerMessage.h"

#define SECTOR_X_MAX 50
#define SECTOR_Y_MAX 50

#define LOGIN_SUCCESS 1
#define LOGIN_FAIL 0

enum class en_JobType : int16
{
	AUTH_NEW_CLIENT_JOIN = 0,
	AUTH_DISCONNECT_CLIENT = 1,
	AUTH_MESSAGE = 2,

	NETWORK_MESSAGE = 3,	

	DATA_BASE_ACCOUNT_CHECK = 100,
	DATA_BASE_CHARACTER_CHECK,	
	DATA_BASE_ITEM_CREATE,
	DATA_BASE_LOOTING_ITEM_INVENTORY_SAVE,
	DATA_BASE_CRAFTING_ITEM_INVENTORY_SAVE,
	DATA_BASE_PLACE_ITEM,
	DATA_BASE_ITEM_USE,
	DATA_BASE_GOLD_SAVE,
	DATA_BASE_CHARACTER_INFO_SEND,
	DATA_BASE_QUICK_SLOT_SAVE,	
	DATA_BASE_QUICK_SWAP,
	DATA_BASE_QUICK_INIT	
};

struct st_Job
{
	en_JobType Type;
	int64 SessionId;
	CGameServerMessage* Message = nullptr;
	st_Session* Session = nullptr;
};

enum class en_TimerJobType : int16
{
	TIMER_JOB_TYPE_NONE,
	TIMER_MELEE_ATTACK_END,
	TIMER_SPELL_END,
	TIMER_SKILL_COOLTIME_END,
	TIMER_OBJECT_SPAWN,
	TIMER_OBJECT_STATE_CHANGE,
	TIMER_OBJECT_DOT,
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