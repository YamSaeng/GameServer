#include "pch.h"
#include "DBModel.h"
#include <regex>

using namespace DBModel;
using namespace std;

//-----------------------------
// Column
//-----------------------------
wstring CColumn::CreateText()
{
	// 입력되어 있는 정보를 토대로 컬럼에 관한 SQL문 작성
	return DBModel::Helpers::Format(
		L"[%s] %s %s %s",
		_name.c_str(),
		_typeText.c_str(),
		_nullable ? L"NULL" : L"NOT NULL",
		_identity ? DBModel::Helpers::Format(L"IDENTITY(%d, %d)", _seedValue, _incrementValue).c_str() : L"");
}

//-----------------------------
// Column
//-----------------------------
wstring CIndex::GetUniqueName()
{
	wstring UniqueName;

	UniqueName += _primaryKey ? L"PK " : L" ";
	UniqueName += _uniqueConstraint ? L"UK " : L" ";
	UniqueName += (_type == IndexType::Clustered ? L"C " : L"NC ");

	for (const CColumn* Column : _columns)
	{
		UniqueName += L"*";
		UniqueName += Column->_name;
		UniqueName += L" ";
	}

	return UniqueName;
}

wstring CIndex::CreateName(const wstring& TableName)
{
	wstring RetTableName = L"IX_" + TableName;

	for (const CColumn* Column : _columns)
	{
		RetTableName += L"_";
		RetTableName += Column->_name;
	}

	return RetTableName;
}

// Index의 타입이 클러스터인지 아닌지에 따라 wstring 반환
wstring CIndex::GetTypeText()
{
	return (_type == IndexType::Clustered ? L"CLUSTERED" : L"NONCLUSTERED");
}

wstring CIndex::GetKeyText()
{
	if (_primaryKey)
	{
		return L"PRIMARY KEY";
	}

	if (_uniqueConstraint)
	{
		return L"UNIQUE";
	}

	return L"";
}

wstring CIndex::CreateColumnsText()
{
	wstring ret;

	const int32 size = static_cast<int32>(_columns.size());
	for (int32 i = 0; i < size; i++)
	{
		if (i > 0)
		{
			ret += L", ";
		}

		ret += DBModel::Helpers::Format(L"[%s]", _columns[i]->_name.c_str());
	}

	return ret;
}

bool CIndex::DependsOn(const wstring& columnName)
{
	auto findIt = std::find_if(_columns.begin(), _columns.end(),
		[&](const CColumn* column) { return column->_name == columnName; });

	return findIt != _columns.end();
}

/*-----------
	Table
------------*/

CColumn* CTable::FindColumn(const wstring& columnName)
{
	auto findIt = std::find_if(_columns.begin(), _columns.end(),
		[&](const CColumn* column) { return column->_name == columnName; });

	if (findIt != _columns.end())
	{
		return *findIt;
	}

	return nullptr;
}

/*----------------
	Procedures
-----------------*/

wstring CProcedure::GenerateCreateQuery()
{
	const WCHAR* query = L"CREATE PROCEDURE [dbo].[%s] %s AS BEGIN %s END";

	wstring paramString = GenerateParamWString();
	return DBModel::Helpers::Format(query, _name.c_str(), paramString.c_str(), _body.c_str());
}

wstring CProcedure::GenerateAlterQuery()
{
	const WCHAR* query = L"ALTER PROCEDURE [dbo].[%s] %s AS	BEGIN %s END";

	wstring paramString = GenerateParamWString();
	return DBModel::Helpers::Format(query, _name.c_str(), paramString.c_str(), _body.c_str());
}

wstring CProcedure::GenerateParamWString()
{
	wstring str;

	const int32 size = static_cast<int32>(_parameters.size());
	for (int32 i = 0; i < size; i++)
	{
		if (i < size - 1)
		{
			str += DBModel::Helpers::Format(L"\t%s %s,\n", _parameters[i]._name.c_str(), _parameters[i]._type.c_str());
		}
		else
		{
			str += DBModel::Helpers::Format(L"\t%s %s", _parameters[i]._name.c_str(), _parameters[i]._type.c_str());
		}
	}

	return str;
}


/*-------------
	Helpers
--------------*/

wstring Helpers::Format(const WCHAR* format, ...)
{
	WCHAR buf[20480];

	va_list ap;
	va_start(ap, format);
	::vswprintf_s(buf, 20480, format, ap);
	va_end(ap);

	return wstring(buf);
}

wstring Helpers::DataTypeToWString(DataType type)
{
	switch (type)
	{
	case DataType::TinyInt:		return L"TinyInt";
	case DataType::SmallInt:	return L"SmallInt";
	case DataType::Int:			return L"Int";
	case DataType::Real:		return L"Real";
	case DataType::DateTime:	return L"DateTime";
	case DataType::Float:		return L"Float";
	case DataType::Bit:			return L"Bit";
	case DataType::Numeric:		return L"Numeric";
	case DataType::BigInt:		return L"BigInt";
	case DataType::VarBinary:	return L"VarBinary";
	case DataType::Varchar:		return L"Varchar";
	case DataType::Binary:		return L"Binary";
	case DataType::NVarChar:	return L"NVarChar";
	default:					return L"None";
	}
}

wstring Helpers::RemoveWhiteSpace(const wstring& str)
{
	wstring ret = str;

	ret.erase(
		std::remove_if(ret.begin(), ret.end(), [=](WCHAR ch) { return ::isspace(ch); }),
		ret.end());

	return ret;
}

DataType Helpers::WStringToDataType(const WCHAR* str, OUT int32& maxLen)
{	
	std::wregex reg(L"([a-zA-Z]+)(\\((max|\\d+)\\))?");
	std::wcmatch ret;

	if (std::regex_match(str, OUT ret, reg) == false)
	{
		return DataType::None;
	}

	if (ret[3].matched)
	{
		maxLen = ::_wcsicmp(ret[3].str().c_str(), L"max") == 0 ? -1 : _wtoi(ret[3].str().c_str());
	}
	else
	{
		maxLen = 0;
	}

	if (::_wcsicmp(ret[1].str().c_str(), L"TinyInt") == 0) return DataType::TinyInt;
	if (::_wcsicmp(ret[1].str().c_str(), L"SmallInt") == 0) return DataType::SmallInt;
	if (::_wcsicmp(ret[1].str().c_str(), L"Int") == 0) return DataType::Int;
	if (::_wcsicmp(ret[1].str().c_str(), L"Real") == 0) return DataType::Real;
	if (::_wcsicmp(ret[1].str().c_str(), L"DateTime") == 0) return DataType::DateTime;
	if (::_wcsicmp(ret[1].str().c_str(), L"Float") == 0) return DataType::Float;
	if (::_wcsicmp(ret[1].str().c_str(), L"Bit") == 0) return DataType::Bit;
	if (::_wcsicmp(ret[1].str().c_str(), L"Numeric") == 0) return DataType::Numeric;
	if (::_wcsicmp(ret[1].str().c_str(), L"BigInt") == 0) return DataType::BigInt;
	if (::_wcsicmp(ret[1].str().c_str(), L"VarBinary") == 0) return DataType::VarBinary;
	if (::_wcsicmp(ret[1].str().c_str(), L"Varchar") == 0) return DataType::Varchar;
	if (::_wcsicmp(ret[1].str().c_str(), L"Binary") == 0) return DataType::Binary;
	if (::_wcsicmp(ret[1].str().c_str(), L"NVarChar") == 0) return DataType::NVarChar;

	return DataType::None;
}