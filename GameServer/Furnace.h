#pragma once
#include "CraftingTable.h"

class CItem;

class CFurnace : public CCraftingTable
{
public:
	CFurnace();

	// �뱤�ο� ��� ������ �ֱ�
	void InputMaterialItem(CItem* MaterialItem, int16 MaterialItemCount);	
	// �뱤�ο� �������� ���� ��ŭ �ִ��� Ȯ��
	bool FindMaterialItem(en_SmallItemCategory FindSmallItemCategory, int16 ItemCount);

	virtual void Update() override;

	st_CraftingTable GetFurnaceCraftingTable();
protected:
	virtual void UpdateCrafting() override;
private:	
	// �뱤�� ������ ���
	st_CraftingTable _FurnaceCraftingTable;	
};

