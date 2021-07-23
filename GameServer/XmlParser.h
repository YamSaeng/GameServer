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
	
	// Key���� �ش��ϴ� �Ӽ��� ã�Ƽ� ��ȯ
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

	// Ű���� �ش��ϴ� ���� ã�Ƽ� ��ȯ
	CXmlNode			FindChild(const WCHAR* key);
	// Ű���� �ش��ϴ� ������ ã�Ƽ� �迭�� �����ϰ� ��ȯ
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
	// Path�� �ִ� ������ �о���̰� Root�� ��ȯ�Ѵ�. Root (���� ����)
	bool ParseFromFile(const WCHAR* Path, OUT CXmlNode& Root);
};