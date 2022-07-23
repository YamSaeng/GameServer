#include "pch.h"
#include "Day.h"

CDay::CDay()
{
	_DayInfo.DayTimeCycle = 0;
	_DayInfo.DayTimeCheck = 540.0f;
	_DayInfo.DayRatio = 1.0f;

	_DayInfo.DayType = en_DayType::DAY_AFTERNOON;
}

CDay::~CDay()
{

}

void CDay::Update()
{
	_DayInfo.DayTimeCycle += 0.02f;

	if (_DayInfo.DayTimeCycle >= 1.0f)
	{
		_DayInfo.DayTimeCheck += 1.0f;
		_DayInfo.DayTimeCycle = 0;		

		if (_DayInfo.DayTimeCheck > 0 && _DayInfo.DayTimeCheck <= DAWN)
		{
			_DayInfo.DayType = en_DayType::DAY_DAWN;
			_DayInfo.DayRatio = 0.1f;
		}
		else if (_DayInfo.DayTimeCheck > DAWN && _DayInfo.DayTimeCheck <= MORNING)
		{
			_DayInfo.DayType = en_DayType::DAY_MORNING;
			_DayInfo.DayRatio += ( MORNING_SUNLIGHT / (float)(MORNING - DAWN)); // 0.00375
		}
		else if (_DayInfo.DayTimeCheck > MORNING && _DayInfo.DayTimeCheck <= AFTERNOON)
		{
			_DayInfo.DayType = en_DayType::DAY_AFTERNOON;
			_DayInfo.DayRatio += (AFTERNOON_SUNLIGHT / (float)(AFTERNOON - MORNING)); // 0.0004
		}
		else if (_DayInfo.DayTimeCheck > AFTERNOON && _DayInfo.DayTimeCheck <= EVENING)
		{
			_DayInfo.DayType = en_DayType::DAY_EVENING;
			_DayInfo.DayRatio -= (EVENING_SUNLIGHT / (float)(EVENING - AFTERNOON)); //0.0035f;
		}
		else if (_DayInfo.DayTimeCheck > EVENING && _DayInfo.DayTimeCheck <= NIGHT)
		{
			_DayInfo.DayType = en_DayType::DAY_NIGHT;
			_DayInfo.DayRatio -= (NIGHT_SUNLIGHT / (float)(NIGHT - EVENING)); //0.002f;
		}
				
		if (_DayInfo.DayTimeCheck > MIDNIGHT)
		{
			_DayInfo.DayTimeCheck = 0;
		}
		
		/*switch (_DayInfo.DayType)
		{
		case en_DayType::DAY_DAWN:
			G_Logger->WriteStdOut(en_Color::RED, L"»õº®");
			break;
		case en_DayType::DAY_MORNING:
			G_Logger->WriteStdOut(en_Color::RED, L"¾ÆÄ§");
			break;
		case en_DayType::DAY_AFTERNOON:
			G_Logger->WriteStdOut(en_Color::RED, L"Á¡½É");
			break;
		case en_DayType::DAY_EVENING:
			G_Logger->WriteStdOut(en_Color::RED, L"Àú³á");
			break;
		case en_DayType::DAY_NIGHT:
			G_Logger->WriteStdOut(en_Color::RED, L"¹ã");
			break;		
		}

		G_Logger->WriteStdOut(en_Color::RED, L"Day Time : %.2f Day Ratio : %.2f\n", _DayInfo.DayTimeCheck, _DayInfo.DayRatio);*/
	}
}

st_Day CDay::GetDayInfo()
{
	return _DayInfo;
}
