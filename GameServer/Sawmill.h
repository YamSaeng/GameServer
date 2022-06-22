#pragma once
#include "CraftingTable.h"

class CSawmill : public CCraftingTable
{
public:
	CSawmill();

	virtual void Update() override;

protected:
	virtual void UpdateCrafting() override;
};