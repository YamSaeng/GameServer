#include "pch.h"
#include "Sawmill.h"
#include "ObjectManager.h"
#include "DataManager.h"

CSawmill::CSawmill()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_ARCHITECTURE_CRAFTING_TABLE_SAWMILL;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	_GameObjectInfo.ObjectWidth = 2;
	_GameObjectInfo.ObjectHeight = 1;

	_GameObjectInfo.ObjectName = L"�����";

	auto FindSamillData = G_Datamanager->_CraftingTableData.find((int16)_GameObjectInfo.ObjectType);
	_CraftingTableRecipe = *(*FindSamillData).second;
}

void CSawmill::Update()
{
	CGameObject::Update();

	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::CRAFTING:
		UpdateCrafting();
		break;
	default:
		break;
	}
}

void CSawmill::UpdateCrafting()
{
	for (CItem* CraftingCompleteItem : _CraftingTableRecipe.CraftingTableCompleteItems)
	{
		if (CraftingCompleteItem->_ItemCrafting == CItem::en_ItemCrafting::ITEM_CRAFTING_START)
		{
			CraftingCompleteItem->_ItemInfo.ItemCraftingRemainTime = CraftingCompleteItem->_CraftingTick - GetTickCount64();

			if (CraftingCompleteItem->_ItemInfo.ItemCraftingRemainTime < 0)
			{
				switch (CraftingCompleteItem->_ItemInfo.ItemSmallCategory)
				{
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_WOOD_FLANK:
				{
					// ��ᰡ �ִ��� �ѹ� �� Ȯ��
					bool IsCraftingSuccess = true;
					for (st_CraftingMaterialItemInfo MaterialItem : CraftingCompleteItem->_ItemInfo.Materials)
					{
						if (!FindMaterialItem(MaterialItem.MaterialItemType, MaterialItem.ItemCount))
						{
							IsCraftingSuccess = false;
						}
					}

					if (IsCraftingSuccess == true)
					{
						CPlayer* Player = (CPlayer*)_SelectedObject;

						// ��� ���� ����
						for (st_CraftingMaterialItemInfo MaterialItem : CraftingCompleteItem->_ItemInfo.Materials)
						{
							auto FindMaterialIter = _MaterialItems.find(MaterialItem.MaterialItemType);
							if (FindMaterialIter != _MaterialItems.end())
							{
								(*FindMaterialIter).second->_ItemInfo.ItemCount -= MaterialItem.ItemCount;
							}
						}

						// ���� �� ��� �������� �ִ��� Ȯ��
						bool IsNextCraftingSuccess = true;
						for (st_CraftingMaterialItemInfo MaterialItem : CraftingCompleteItem->_ItemInfo.Materials)
						{
							if (!FindMaterialItem(MaterialItem.MaterialItemType, MaterialItem.ItemCount))
							{
								IsNextCraftingSuccess = false;
							}
						}

						if (IsNextCraftingSuccess == true)
						{
							CraftingCompleteItem->_CraftingTick = CraftingCompleteItem->_ItemInfo.ItemCraftingTime + GetTickCount64();

							CraftingCompleteItem->_ItemInfo.ItemCraftingRemainTime = CraftingCompleteItem->_CraftingTick - GetTickCount64();

							if (_SelectedObject != nullptr)
							{
								CMessage* ResCraftingStartPacket = G_ObjectManager->GameServer->MakePacketResCraftingStart(
									_GameObjectInfo.ObjectId,
									CraftingCompleteItem->_ItemInfo);
								G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCraftingStartPacket);
							}
						}
						else
						{
							_CraftingStartCompleteItem = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;

							_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

							CraftingCompleteItem->_ItemCrafting = CItem::en_ItemCrafting::ITEM_CRAFTING_STOP;
						}

						auto CompleteItemIter = _CompleteItems.find(CraftingCompleteItem->_ItemInfo.ItemSmallCategory);
						if (CompleteItemIter == _CompleteItems.end())
						{
							CItem* CompleteItemCharCoal = G_ObjectManager->ItemCreate(CraftingCompleteItem->_ItemInfo.ItemSmallCategory);
							CompleteItemCharCoal->_ItemInfo.ItemCount = 1;				

							_CompleteItems.insert(pair<en_SmallItemCategory, CItem*>(CompleteItemCharCoal->_ItemInfo.ItemSmallCategory, CompleteItemCharCoal));
						}
						else
						{
							(*CompleteItemIter).second->_ItemInfo.ItemCount += 1;
						}

						if (_SelectedObject != nullptr)
						{
							// ����� ���� ��� ��� ����
							CMessage* ResCraftingTableMaterialItemListPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableMaterialItemList(
								_GameObjectInfo.ObjectId,
								_GameObjectInfo.ObjectType,
								CraftingCompleteItem->_ItemInfo.ItemSmallCategory,
								_MaterialItems);
							G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCraftingTableMaterialItemListPacket);
							ResCraftingTableMaterialItemListPacket->Free();

							// ����� ���� ���� �Ϸ� ������ ��� ����
							CMessage* ResCrafintgTableCompleteItemListPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableCompleteItemList(
								_GameObjectInfo.ObjectId,
								_GameObjectInfo.ObjectType,
								_CompleteItems);
							G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCrafintgTableCompleteItemListPacket);
							ResCrafintgTableCompleteItemListPacket->Free();
						}
					}
					else
					{
						_CraftingStartCompleteItem = en_SmallItemCategory::ITEM_SMALL_CATEGORY_NONE;

						_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

						CraftingCompleteItem->_ItemCrafting = CItem::en_ItemCrafting::ITEM_CRAFTING_STOP;
					}
				}
				break;
				}
			}
		}
	}
}
