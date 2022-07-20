#pragma once
#include "GameObjectInfo.h"

class CDay
{
public:
	// 1440초 = 24분 -> 하루 	
	// 0분 ~ 5분 새벽
	const int16 DAWN = 300;
	// 5분 ~ 9분 아침
	const int16 MORNING = 540;
	// 9분 ~ 17분 20초 낮
	const int16 AFTERNOON = 1040;
	// 17분 20초 ~ 20분 40초 저녁
	const int16 EVENING = 1240;
	// 20분 40초 ~ 24분 밤
	const int16 NIGHT = 1440;
	
	const float MIDNIGHT = 1440.0f;

	// 새벽 -> 아침 일조량
	const float MORNING_SUNLIGHT = 0.9f;
	// 아침 -> 낮 일조량
	const float AFTERNOON_SUNLIGHT = 0.2f;
	// 낮 -> 저녁 일조량
	const float EVENING_SUNLIGHT = 0.7f;
	// 저녁 -> 밤 일조량
	const float NIGHT_SUNLIGHT = 0.4f;

	CDay();
	~CDay();

	void Update();

	st_Day GetDayInfo();
private:	
	st_Day _DayInfo;	
};

