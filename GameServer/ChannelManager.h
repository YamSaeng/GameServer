#pragma once
#include "Channel.h"

class CMap;

class CChannelManager
{
private:
	map<int8, CChannel*> _Channels;
	CMap* _OwnerMap;
public:
	CChannelManager();
	~CChannelManager();

	void Init(CMap* OwnerMap, int8 ChannelCount);

	//-------------------------
	// 채널 삭제
	//-------------------------
	bool Remove(int32 ChannelId);

	//------------------------------
	// 채널 찾아서 반환
	//------------------------------
	CChannel* Find(int32 ChannelId);

	//------------------------------------------
	// 보유하고 있는 채널 순환하면서 Update 실행
	//------------------------------------------
	void Update();
};

