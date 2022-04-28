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
	// ä�� ����
	//-------------------------
	bool Remove(int32 ChannelId);

	//------------------------------
	// ä�� ã�Ƽ� ��ȯ
	//------------------------------
	CChannel* Find(int32 ChannelId);

	//------------------------------------------
	// �����ϰ� �ִ� ä�� ��ȯ�ϸ鼭 Update ����
	//------------------------------------------
	void Update();
};

