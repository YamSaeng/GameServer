#include "pch.h"
#include "Furnace.h"
#include "Item.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include <atlbase.h>

CFurnace::CFurnace()
{
	_GameObjectInfo.ObjectType = en_GameObjectType::OBJECT_FURNACE;
	_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;

	_GameObjectInfo.ObjectName = L"용광로";

	auto FindFurnaceData = G_Datamanager->_CraftingTableData.find((int16)_GameObjectInfo.ObjectType);
	_FurnaceCraftingTable = *(*FindFurnaceData).second;	
}

map<en_SmallItemCategory, CItem*> CFurnace::GetMaterialItems()
{
	return _MaterialItems;
}

void CFurnace::InputMaterialItem(CItem* MaterialItem, int16 MaterialItemCount)
{
	// 우선 용광로 재료 템들 중에 넣을 아이템이 존재하는지 확인
	auto FindMaterialIter = _MaterialItems.find(MaterialItem->_ItemInfo.ItemSmallCategory);
	// 발견하지 못햇을 경우
	if (FindMaterialIter == _MaterialItems.end())
	{		
		// 아이템 생성
		CItem* NewMaterialItem = G_ObjectManager->ItemCreate(MaterialItem->_ItemInfo.ItemSmallCategory);	
		if (NewMaterialItem != nullptr)
		{
			NewMaterialItem->_ItemInfo.ItemDBId = 0;
			NewMaterialItem->_ItemInfo.InventoryItemNumber = 0;
			NewMaterialItem->_ItemInfo.ItemIsQuickSlotUse = false;
			NewMaterialItem->_ItemInfo.Rotated = false;
			NewMaterialItem->_ItemInfo.Width = 1;
			NewMaterialItem->_ItemInfo.Height = 1;
			NewMaterialItem->_ItemInfo.TileGridPositionX = MaterialItem->_ItemInfo.TileGridPositionX;
			NewMaterialItem->_ItemInfo.TileGridPositionY = MaterialItem->_ItemInfo.TileGridPositionY;
			NewMaterialItem->_ItemInfo.ItemLargeCategory = MaterialItem->_ItemInfo.ItemLargeCategory;
			NewMaterialItem->_ItemInfo.ItemMediumCategory = MaterialItem->_ItemInfo.ItemMediumCategory;
			NewMaterialItem->_ItemInfo.ItemSmallCategory = MaterialItem->_ItemInfo.ItemSmallCategory;
			NewMaterialItem->_ItemInfo.ItemName = MaterialItem->_ItemInfo.ItemName;
			NewMaterialItem->_ItemInfo.ItemExplain = MaterialItem->_ItemInfo.ItemExplain;
			NewMaterialItem->_ItemInfo.ItemMaxCount = MaterialItem->_ItemInfo.ItemMaxCount;
			NewMaterialItem->_ItemInfo.ItemCount = MaterialItemCount;
			NewMaterialItem->_ItemInfo.ItemThumbnailImagePath = MaterialItem->_ItemInfo.ItemThumbnailImagePath;
			NewMaterialItem->_ItemInfo.ItemIsEquipped = MaterialItem->_ItemInfo.ItemIsEquipped;

			_MaterialItems.insert(pair<en_SmallItemCategory, CItem*>(NewMaterialItem->_ItemInfo.ItemSmallCategory, NewMaterialItem));
		}		
	}	
	else
	{
		// 발견 했을 경우 개수만 늘려줌
		(*FindMaterialIter).second->_ItemInfo.ItemCount += MaterialItemCount;
	}
}

bool CFurnace::FindMaterialItem(en_SmallItemCategory FindSmallItemCategory, int16 ItemCount)
{
	auto FindMaterialIter = _MaterialItems.find(FindSmallItemCategory);
	if (FindMaterialIter == _MaterialItems.end())
	{
		return false;
	}
	
	if ((*FindMaterialIter).second->_ItemInfo.ItemCount >= ItemCount)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CFurnace::Update()
{
	CGameObject::Update();

	switch (_GameObjectInfo.ObjectPositionInfo.State)
	{
	case en_CreatureState::CRAFTING:
		UpdateCrafting();
		break;	
	}
}

st_CraftingTable CFurnace::GetFurnaceCraftingTable()
{
	return _FurnaceCraftingTable;
}

void CFurnace::UpdateCrafting()
{
	if (_CraftingTick < GetTickCount64())
	{
		for (st_CraftingCompleteItem CraftingCompleteItem : _FurnaceCraftingTable.CraftingTableCompleteItems)
		{
			if (CraftingCompleteItem.CompleteItemType == _SelectCraftingTableCompleteItem)
			{
				switch (_SelectCraftingTableCompleteItem)
				{
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL:
					{
						// 숯 조합 재료에 해당하는 아이템들을 용광로 재료에서 없애줘야함
						int16 OneReqMaterialCount = 0;
						
						// 재료가 용광로에 있는지 우선 확인
						bool IsCraftingSuccess = true;
						for (st_CraftingMaterialItemInfo MaterialItem : CraftingCompleteItem.Materials)
						{
							if (!FindMaterialItem(MaterialItem.MaterialItemType, MaterialItem.ItemCount))
							{
								IsCraftingSuccess = false;
							}							
						}

						if (IsCraftingSuccess == true)
						{
							// 재료 갯수 차감
							for (st_CraftingMaterialItemInfo MaterialItem : CraftingCompleteItem.Materials)
							{
								auto FindMaterialIter = _MaterialItems.find(MaterialItem.MaterialItemType);
								if (FindMaterialIter != _MaterialItems.end())
								{
									(*FindMaterialIter).second->_ItemInfo.ItemCount -= MaterialItem.ItemCount;
								}								
							}

							CMessage* ResCraftingTableMaterialItemListPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableMaterialItemList(
								_GameObjectInfo.ObjectId,
								_GameObjectInfo.ObjectType,
								_SelectCraftingTableCompleteItem,
								_MaterialItems);							
							CPlayer* Player = (CPlayer*)_Channel->FindChannelObject(_SelectedObjectID, en_GameObjectType::OBJECT_PLAYER);
							G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCraftingTableMaterialItemListPacket);
							ResCraftingTableMaterialItemListPacket->Free();

							st_ItemData* ItemData = G_Datamanager->FindItemData(_SelectCraftingTableCompleteItem);

							auto CompleteItemIter = _CompleteItems.find(_SelectCraftingTableCompleteItem);
							if (CompleteItemIter == _CompleteItems.end())
							{
								CItem* CompleteItemCharCoal = G_ObjectManager->ItemCreate(_SelectCraftingTableCompleteItem);
								CompleteItemCharCoal->_ItemInfo.ItemDBId = 0;
								CompleteItemCharCoal->_ItemInfo.Rotated = 0;
								CompleteItemCharCoal->_ItemInfo.Width = ItemData->ItemWidth;
								CompleteItemCharCoal->_ItemInfo.Height = ItemData->ItemHeight;
								CompleteItemCharCoal->_ItemInfo.ItemLargeCategory = (en_LargeItemCategory)ItemData->LargeItemCategory;
								CompleteItemCharCoal->_ItemInfo.ItemMediumCategory = (en_MediumItemCategory)ItemData->MediumItemCategory;
								CompleteItemCharCoal->_ItemInfo.ItemSmallCategory = (en_SmallItemCategory)ItemData->SmallItemCategory;
								CompleteItemCharCoal->_ItemInfo.ItemName = (LPWSTR)CA2W(ItemData->ItemName.c_str());
								CompleteItemCharCoal->_ItemInfo.ItemExplain = (LPWSTR)CA2W(ItemData->ItemExplain.c_str());
								CompleteItemCharCoal->_ItemInfo.ItemCount = 1;
								CompleteItemCharCoal->_ItemInfo.ItemThumbnailImagePath = (LPWSTR)CA2W(ItemData->ItemThumbnailImagePath.c_str());
								CompleteItemCharCoal->_ItemInfo.ItemIsEquipped = false;
								CompleteItemCharCoal->_ItemInfo.ItemMaxCount = ItemData->ItemMaxCount;

								_CompleteItems.insert(pair<en_SmallItemCategory, CItem*>(CompleteItemCharCoal->_ItemInfo.ItemSmallCategory, CompleteItemCharCoal));
							}
							else
							{
								(*CompleteItemIter).second->_ItemInfo.ItemCount += 1;
							}							

							CMessage* ResCrafintgTableCompleteItemListPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableCompleteItemList(
								_GameObjectInfo.ObjectId,
								_GameObjectInfo.ObjectType,								
								_CompleteItems);
							G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCrafintgTableCompleteItemListPacket);
							ResCrafintgTableCompleteItemListPacket->Free();
						}	
						else
						{
							_GameObjectInfo.ObjectPositionInfo.State = en_CreatureState::IDLE;
						}
					}
				break;
				}
			}			
		}
		
		_CraftingTick = GetTickCount64() + 5000;		
	}
}
