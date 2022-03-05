#pragma once
#include "rapidxml.hpp"

using namespace rapidxml;

/*-------------
	XmlNode
--------------*/

using XmlNodeType = xml_node<WCHAR>;
using XmlDocumentType = xml_document<WCHAR>;
using XmlAttributeType = xml_attribute<WCHAR>;

class CXmlNode
{
private:
	XmlNodeType* _Node = nullptr;
public:
	CXmlNode(XmlNodeType* node = nullptr) : _Node(node) { }

	bool				IsValid() { return _Node != nullptr; }
	
	// Key값에 해당하는 속성을 찾아서 반환
	bool				GetBoolAttr(const WCHAR* key, bool defaultValue = false);
	int8				GetInt8Attr(const WCHAR* key, int8 defaultValue = 0);
	int16				GetInt16Attr(const WCHAR* key, int16 defaultValue = 0);
	int32				GetInt32Attr(const WCHAR* key, int32 defaultValue = 0);
	int64				GetInt64Attr(const WCHAR* key, int64 defaultValue = 0);
	float				GetFloatAttr(const WCHAR* key, float defaultValue = 0.0f);
	double				GetDoubleAttr(const WCHAR* key, double defaultValue = 0.0);
	const WCHAR*		GetStringAttr(const WCHAR* key, const WCHAR* defaultValue = L"");

	bool				GetBoolValue(bool defaultValue = false);
	int8				GetInt8Value(int8 defaultValue = 0);
	int16				GetInt16Value(int16 defaultValue = 0);
	int32				GetInt32Value(int32 defaultValue = 0);
	int64				GetInt64Value(int64 defaultValue = 0);
	float				GetFloatValue(float defaultValue = 0.0f);
	double				GetDoubleValue(double defaultValue = 0.0);
	const WCHAR*		GetStringValue(const WCHAR* defaultValue = L"");

	// 키값에 해당하는 값을 찾아서 반환
	CXmlNode			FindChild(const WCHAR* key);
	// 키값에 해당하는 값들을 찾아서 배열에 저장하고 반환
	vector<CXmlNode>	FindChildren(const WCHAR* key);
};

/*---------------
	XmlParser
----------------*/

class CXmlParser
{
private:
	XmlDocumentType*				_Document = nullptr;
	wstring							_Data;
public:
	// Path에 있는 파일을 읽어들이고 Root를 반환한다. Root (시작 지점)
	bool ParseFromFile(const WCHAR* Path, OUT CXmlNode& Root);
};