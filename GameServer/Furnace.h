#pragma once
#include "CraftingTable.h"

class CItem;

class CFurnace : public CCraftingTable
{
public:
	CFurnace();

	virtual void Update() override;
	
protected:
	virtual void UpdateCrafting() override;		
};

