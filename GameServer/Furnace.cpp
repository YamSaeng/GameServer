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
	_CraftingTableRecipe = *(*FindFurnaceData).second;	
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

void CFurnace::UpdateCrafting()
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
				case en_SmallItemCategory::ITEM_SMALL_CATEGORY_MATERIAL_CHAR_COAL:
				{
					// 재료가 있는지 한번 더 확인
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

						// 재료 갯수 차감
						for (st_CraftingMaterialItemInfo MaterialItem : CraftingCompleteItem->_ItemInfo.Materials)
						{
							auto FindMaterialIter = _MaterialItems.find(MaterialItem.MaterialItemType);
							if (FindMaterialIter != _MaterialItems.end())
							{
								(*FindMaterialIter).second->_ItemInfo.ItemCount -= MaterialItem.ItemCount;
							}
						}

						// 다음 번 재료 조합템이 있는지 확인
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
							CompleteItemCharCoal->_ItemInfo = CraftingCompleteItem->_ItemInfo;
							CompleteItemCharCoal->_ItemInfo.ItemCount = 1;

							_CompleteItems.insert(pair<en_SmallItemCategory, CItem*>(CompleteItemCharCoal->_ItemInfo.ItemSmallCategory, CompleteItemCharCoal));
						}
						else
						{
							(*CompleteItemIter).second->_ItemInfo.ItemCount += 1;
						}

						if (_SelectedObject != nullptr)
						{
							// 용광로 보유 재료 목록 갱신
							CMessage* ResCraftingTableMaterialItemListPacket = G_ObjectManager->GameServer->MakePacketResCraftingTableMaterialItemList(
								_GameObjectInfo.ObjectId,
								_GameObjectInfo.ObjectType,
								CraftingCompleteItem->_ItemInfo.ItemSmallCategory,
								_MaterialItems);
							G_ObjectManager->GameServer->SendPacket(Player->_SessionId, ResCraftingTableMaterialItemListPacket);
							ResCraftingTableMaterialItemListPacket->Free();

							// 용광로 보유 제작 완료 아이템 목록 갱신
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
