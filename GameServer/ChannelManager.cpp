#include "pch.h"
#include "ChannelManager.h"

CChannelManager::CChannelManager()
{	
	_ChannelId = 1;
}

CChannelManager::~CChannelManager()
{
}

CChannel* CChannelManager::Add(int32 MapId)
{
	CChannel* Channel = new CChannel();
	Channel->Init(MapId, 10);

	Channel->_ChannelId = _ChannelId;
	_Channels.insert(pair<int32,CChannel*>(Channel->_ChannelId,Channel));
	_ChannelId++;

	return Channel;
}

bool CChannelManager::Remove(int32 ChannelId)
{
	return _Channels.erase(ChannelId);
}

CChannel* CChannelManager::Find(int32 ChannelId)
{
	CChannel* Channel = nullptr;

	auto ChannelFindIterator = _Channels.find(ChannelId);
	if (ChannelFindIterator != _Channels.end())
	{
		Channel = (*ChannelFindIterator).second;	
	}
	else
	{
		CRASH("ChannelManager Find 채널을 찾지 못함");		
	}

	return Channel;
}

void CChannelManager::Update()
{
	for (auto ChannelIterator : _Channels)
	{
		CChannel* Channel = ChannelIterator.second;

		Channel->Update();
	}
}
