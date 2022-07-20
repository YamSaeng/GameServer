#pragma once
#include "GameObjectInfo.h"

class CDay
{
public:
	// 1440�� = 24�� -> �Ϸ� 	
	// 0�� ~ 5�� ����
	const int16 DAWN = 300;
	// 5�� ~ 9�� ��ħ
	const int16 MORNING = 540;
	// 9�� ~ 17�� 20�� ��
	const int16 AFTERNOON = 1040;
	// 17�� 20�� ~ 20�� 40�� ����
	const int16 EVENING = 1240;
	// 20�� 40�� ~ 24�� ��
	const int16 NIGHT = 1440;
	
	const float MIDNIGHT = 1440.0f;

	// ���� -> ��ħ ������
	const float MORNING_SUNLIGHT = 0.9f;
	// ��ħ -> �� ������
	const float AFTERNOON_SUNLIGHT = 0.2f;
	// �� -> ���� ������
	const float EVENING_SUNLIGHT = 0.7f;
	// ���� -> �� ������
	const float NIGHT_SUNLIGHT = 0.4f;

	CDay();
	~CDay();

	void Update();

	st_Day GetDayInfo();
private:	
	st_Day _DayInfo;	
};

