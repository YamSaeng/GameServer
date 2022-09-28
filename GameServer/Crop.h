#pragma once
#include "GameObject.h"

class CCrop : public CGameObject
{
public:
	CCrop();
	~CCrop();

	void Init(en_SmallItemCategory CropItemCategory);

	virtual void Update() override;
	
	virtual bool OnDamaged(CGameObject* Attacker, int32 Damage) override;

	virtual void UpdateIdle() override;
	virtual void UpdateReadyDead() override;
	virtual void UpdateDead() override;
private:
	enum en_CropState
	{
		CROP_IDLE,
		CROP_GROWING,
		CROP_GROW_END
	};

	st_ItemInfo _CropItemInfo;

	float _CropRatio;	

	int64 _CropIdleTick;
	int16 _CropTime;

	en_CropState _CropState;

	vector<wstring> _CropStepString;
};

