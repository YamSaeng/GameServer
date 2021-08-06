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
	// ä�ο� �� �Ҵ�
	//-------------------------
	CChannel* Add(int32 MapId);

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

