#pragma once
#include"GameObject.h"

class CEnvironment : public CGameObject
{
public:
	st_Vector2Int _SpawnPosition;

	CEnvironment();
};

class CStone : public CEnvironment
{
public:
	CStone();
};

class CTree : public CEnvironment
{
public:
	CTree();
};