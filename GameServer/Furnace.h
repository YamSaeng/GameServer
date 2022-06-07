#pragma once
#include "CraftingTable.h"

class CItem;

class CFurnace : public CCraftingTable
{
public:
	CFurnace();

	// �뱤�� �������� ��� ������ ��ȯ
	map<en_SmallItemCategory, CItem*> GetMaterialItems();
	// �뱤�ο� ��� ������ �ֱ�
	void InputMaterialItem(CItem* MaterialItem, int16 MaterialItemCount);	

	void CraftingStart();
private:	
	// �뱤�ΰ� �������� ��� ������ 
	map<en_SmallItemCategory, CItem*> _MaterialItems;

	// �뱤�� ������ ���
	st_CraftingTable _FurnaceCraftingTable;	
};

