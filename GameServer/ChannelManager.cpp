#include "pch.h"
#include "ChannelManager.h"

CChannelManager::CChannelManager()
{

}

CChannelManager::~CChannelManager()
{
	for (auto ChannelIterator : _Channels)
	{
		delete ChannelIterator.second;
	}
}

void CChannelManager::Init(CMap* OwnerMap, int8 ChannelCount)
{
	_OwnerMap = OwnerMap;

	for (int8 i = 0; i < ChannelCount; i++)
	{
		CChannel* NewChannel = new CChannel();
		NewChannel->Init(_OwnerMap);

		_Channels.insert(pair<int32, CChannel*>(i + 1, NewChannel));
	}
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
