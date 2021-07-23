#include "pch.h"
#include "XmlParser.h"
#include "FileUtils.h"

/*-------------
	XmlNode
--------------*/

_locale_t kr = _create_locale(LC_NUMERIC, "kor");

bool CXmlNode::GetBoolAttr(const WCHAR* key, bool defaultValue)
{
	XmlAttributeType* attr = _Node->first_attribute(key);
	if (attr)
	{
		return ::_wcsicmp(attr->value(), L"true") == 0;
	}		

	return defaultValue;
}

int8 CXmlNode::GetInt8Attr(const WCHAR* key, int8 defaultValue)
{
	XmlAttributeType* attr = _Node->first_attribute(key);
	if (attr)
	{
		return static_cast<int8>(::_wtoi(attr->value()));
	}		

	return defaultValue;
}

int16 CXmlNode::GetInt16Attr(const WCHAR* key, int16 defaultValue)
{
	XmlAttributeType* attr = _Node->first_attribute(key);
	if (attr)
	{
		return static_cast<int16>(::_wtoi(attr->value()));
	}		

	return defaultValue;
}

int32 CXmlNode::GetInt32Attr(const WCHAR* key, int32 defaultValue)
{
	XmlAttributeType* attr = _Node->first_attribute(key);
	if (attr)
	{
		return ::_wtoi(attr->value());
	}		

	return defaultValue;
}

int64 CXmlNode::GetInt64Attr(const WCHAR* key, int64 defaultValue)
{
	xml_attribute<WCHAR>* attr = _Node->first_attribute(key);
	if (attr)
	{
		return ::_wtoi64(attr->value());
	}		

	return defaultValue;
}

float CXmlNode::GetFloatAttr(const WCHAR* key, float defaultValue)
{
	XmlAttributeType* attr = _Node->first_attribute(key);
	if (attr)
	{
		return static_cast<float>(::_wtof(attr->value()));
	}		

	return defaultValue;
}

double CXmlNode::GetDoubleAttr(const WCHAR* key, double defaultValue)
{
	XmlAttributeType* attr = _Node->first_attribute(key);
	if (attr)
	{
		return ::_wtof_l(attr->value(), kr);
	}	

	return defaultValue;
}

const WCHAR* CXmlNode::GetStringAttr(const WCHAR* key, const WCHAR* defaultValue)
{
	XmlAttributeType* attr = _Node->first_attribute(key);
	if (attr)
	{
		return attr->value();
	}		

	return defaultValue;
}

bool CXmlNode::GetBoolValue(bool defaultValue)
{
	WCHAR* val = _Node->value();
	if (val)
	{
		return ::_wcsicmp(val, L"true") == 0;
	}	

	return defaultValue;
}

int8 CXmlNode::GetInt8Value(int8 defaultValue)
{
	WCHAR* val = _Node->value();
	if (val)
	{
		return static_cast<int8>(::_wtoi(val));
	}		

	return defaultValue;
}

int16 CXmlNode::GetInt16Value(int16 defaultValue)
{
	WCHAR* val = _Node->value();
	if (val)
	{
		return static_cast<int16>(::_wtoi(val));
	}

	return defaultValue;
}

int32 CXmlNode::GetInt32Value(int32 defaultValue)
{
	WCHAR* val = _Node->value();
	if (val)
	{
		return static_cast<int32>(::_wtoi(val));
	}		

	return defaultValue;
}

int64 CXmlNode::GetInt64Value(int64 defaultValue)
{
	WCHAR* val = _Node->value();
	if (val)
	{
		return static_cast<int64>(::_wtoi64(val));
	}		

	return defaultValue;
}

float CXmlNode::GetFloatValue(float defaultValue)
{
	WCHAR* val = _Node->value();
	if (val)
	{
		return static_cast<float>(::_wtof(val));
	}	

	return defaultValue;
}

double CXmlNode::GetDoubleValue(double defaultValue)
{
	WCHAR* val = _Node->value();
	if (val)
	{
		return ::_wtof_l(val, kr);
	}		

	return defaultValue;
}

const WCHAR* CXmlNode::GetStringValue(const WCHAR* defaultValue)
{
	WCHAR* val = _Node->first_node()->value();
	if (val)
	{
		return val;
	}		

	return defaultValue;
}

CXmlNode CXmlNode::FindChild(const WCHAR* key)
{
	return CXmlNode(_Node->first_node(key));
}

vector<CXmlNode> CXmlNode::FindChildren(const WCHAR* key)
{
	vector<CXmlNode> nodes;

	xml_node<WCHAR>* node = _Node->first_node(key);
	while (node)
	{
		nodes.push_back(CXmlNode(node));
		node = node->next_sibling(key);
	}

	return nodes;
}

/*---------------
	XmlParser
----------------*/

bool CXmlParser::ParseFromFile(const WCHAR* Path, OUT CXmlNode& Root)
{
	// 경로에 있는 파일을 읽어서 배열에 저장
	vector<BYTE> Bytes = FileUtils::ReadFile(Path);
	// wstring으로 변환해서 _Data에 저장
	_Data = FileUtils::Convert(string(Bytes.begin(), Bytes.end()));

	// 데이터가 없으면 false
	if (_Data.empty())
	{
		return false;
	}		

	// XmlDocument 생성
	_Document = new XmlDocumentType();
	// 파싱
	_Document->parse<0>(reinterpret_cast<WCHAR*>(&_Data[0]));	
	Root = CXmlNode(_Document->first_node());	
	return true;
}