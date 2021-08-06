#pragma once
#include "Channel.h"

class CChannelManager
{
private:
	map<int32, CChannel*> _Channels;
	int32 _ChannelId;
public:
	CChannelManager();
	~CChannelManager();

	//-------------------------
	// 채널에 맵 할당
	//-------------------------
	CChannel* Add(int32 MapId);

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

