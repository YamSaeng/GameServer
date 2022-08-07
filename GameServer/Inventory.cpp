#include "pch.h"
#include "Inventory.h"
#include "ObjectManager.h"

CInventory::CInventory()
{

}

CInventory::~CInventory()
{
	if (_Items != nullptr)
	{
		for (int Y = 0; Y < _InventoryHeight; Y++)
		{
			for (int X = 0; X < _InventoryWidth; X++)
			{
				G_ObjectManager->ObjectReturn(en_GameObjectType::OBJECT_ITEM, _Items[Y][X]->InventoryItem);				
				delete _Items[Y][X];
			}

			delete _Items[Y];
		}

		delete _Items;
	}	
}

void CInventory::Init(int8 InventoryWidth, int8 InventoryHeight)
{
	_InventoryWidth = InventoryWidth;
	_InventoryHeight = InventoryHeight;
	
	_Items = new st_InventoryItem **[_InventoryHeight];

	for (int Y = 0; Y < _InventoryHeight; Y++)
	{		
		_Items[Y] = new st_InventoryItem *[_InventoryWidth];

		for (int X = 0; X < _InventoryWidth; X++)
		{			
			_Items[Y][X] = new st_InventoryItem();
			_Items[Y][X]->IsEmptySlot = true;
			_Items[Y][X]->InventoryItem = nullptr;
		}		
	}		

	_InventoryItemNumber = 0;
}

CItem* CInventory::SelectItem(int8 TilePositionX, int8 TilePositionY)
{
	if (PositionCheck(TilePositionX, TilePositionY) == true)
	{
		if (_Items[TilePositionX][TilePositionY]->IsEmptySlot == false)
		{
			CItem* ReturnItem = _Items[TilePositionX][TilePositionY]->InventoryItem;
			if (ReturnItem == nullptr)
			{
				return nullptr;
			}

			CleanGridReference(ReturnItem);

			return ReturnItem;
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}	
}

void CInventory::CleanGridReference(CItem* CleanItem)
{
	// 아이템의 넓이 만큼 인벤토리 위치 타일을 정리한다.
	for (int X = 0; X < CleanItem->_ItemInfo.Width; X++)
	{
		for (int Y = 0; Y < CleanItem->_ItemInfo.Height; Y++)
		{
			_Items[CleanItem->_ItemInfo.TileGridPositionX + X][CleanItem->_ItemInfo.TileGridPositionY + Y]->IsEmptySlot = true;
		}
	}
}

CItem* CInventory::GetItem(int8 X, int8 Y)
{
	return _Items[X][Y]->InventoryItem;
}

st_Vector2Int CInventory::FindEmptySpace(CItem* ItemInfo)
{	
	st_Vector2Int FindTilePosition;
	FindTilePosition._X = -1;
	FindTilePosition._Y = -1;

	int SearchingHeight = _InventoryHeight - ItemInfo->_ItemInfo.Height + 1;
	int SearchingWidth = _InventoryWidth - ItemInfo->_ItemInfo.Width + 1;

	for (int Y = 0; Y < SearchingHeight; Y++)
	{
		for (int X = 0; X < SearchingWidth; X++)
		{
			if (CheckEmptySpace(X, Y, ItemInfo->_ItemInfo.Width, ItemInfo->_ItemInfo.Height) == true)
			{
				FindTilePosition._X = X;
				FindTilePosition._Y = Y;
				
				return FindTilePosition;
			}
		}
	}

	return FindTilePosition;
}

bool CInventory::FindItemSpaceEmpty(CItem* Item)
{
	return CheckEmptySpace(Item->_ItemInfo.TileGridPositionX, Item->_ItemInfo.TileGridPositionY, Item->_ItemInfo.Width, Item->_ItemInfo.Height);
}

bool CInventory::CheckEmptySpace(int16 PositionX, int16 PositionY, int32 Width, int32 Height)
{
	// 매개변수로 받은 시작 위치부터 넓이만큼 인벤토리가 비어 있는지 확인한다.
	for(int X = 0; X < Width; X++)
	{
		for (int Y = 0; Y < Height; Y++)
		{			
			if (_Items[PositionX + X][PositionY + Y]->IsEmptySlot == false)
			{
				return false;
			}
		}
	}

	return true;
}

bool CInventory::PlaceItem(CItem* PlaceItemInfo, int16 PositionX, int16 PositionY, CItem** OverlapItem)
{
	// 아이템 범위 체크
	if (BoundryCheck(PositionX, PositionY, PlaceItemInfo->_ItemInfo.Width, PlaceItemInfo->_ItemInfo.Height) == false)
	{
		return false;
	}

	// 아이템을 넣을 위치에 아이템이 이미 있는지 확인한다.
	if (OverlapCheck(PositionX, PositionY, PlaceItemInfo->_ItemInfo.Width, PlaceItemInfo->_ItemInfo.Height, OverlapItem) == false)
	{
		// 넣을 위치에 2가지 이상 아이템이 존재할 경우 아이템을 넣을 수 없다고 판단한다.
		*OverlapItem = nullptr;
		return false;
	}

	// 중복한 아이템을 찾앗을 경우 ( == 놓을 위치에 아이템이 하나만 있을 경우 )
	if (*OverlapItem != nullptr)
	{
		// 중복 아이템의 공간을 비워준다.
		CleanGridReference(*OverlapItem);
	}

	// 아이템을 넣는다.
	PlaceItem(PlaceItemInfo, PositionX, PositionY);

	return true;
}

void CInventory::PlaceItem(CItem* PlaceItemInfo, int16 PositionX, int16 PositionY)
{
	// 넣을 아이템의 위치를 설정한다.
	PlaceItemInfo->_ItemInfo.TileGridPositionX = PositionX;
	PlaceItemInfo->_ItemInfo.TileGridPositionY = PositionY;

	// 아이템의 넓이만큼 시작 위치에서 슬롯에 채워넣는다.
	for (int X = 0; X < PlaceItemInfo->_ItemInfo.Width; X++)
	{
		for (int Y = 0; Y < PlaceItemInfo->_ItemInfo.Height; Y++)
		{			
			_Items[PositionX + X][PositionY + Y]->IsEmptySlot = false;							
			_Items[PositionX + X][PositionY + Y]->InventoryItem = PlaceItemInfo;			
			_Items[PositionX + X][PositionY + Y]->InventoryItem->_ItemInfo.InventoryItemNumber = _InventoryItemNumber;
		}
	}	

	_InventoryItemNumber++;
}

void CInventory::InitItem(int16 TilePositionX, int16 TilePositionY)
{
	if (_Items[TilePositionX][TilePositionY] != nullptr)
	{
		st_InventoryItem* InitInventoryItem = _Items[TilePositionX][TilePositionY];
		
		int16 InitTileWidth = InitInventoryItem->InventoryItem->_ItemInfo.Width;
		int16 InitTileHeight = InitInventoryItem->InventoryItem->_ItemInfo.Height;

		CItem* Item = InitInventoryItem->InventoryItem;
		G_ObjectManager->ItemReturn(Item);

		for (int16 X = 0; X < InitTileWidth; X++)
		{
			for (int16 Y = 0; Y < InitTileHeight; Y++)
			{
				_Items[TilePositionX + X][TilePositionY + Y]->IsEmptySlot = true;	
				_Items[TilePositionX + X][TilePositionY + Y]->InventoryItem = nullptr;
			}
		}		
	}	
}

st_Vector2Int CInventory::CalculatePositionOnGrid(CItem* Item, int8 TilePositionX, int8 TilePositionY)
{
	st_Vector2Int TilePosition;
	TilePosition._X = TilePositionX * en_Inventory::TILE_SIZE_WIDTH + en_Inventory::TILE_SIZE_WIDTH * Item->_ItemInfo.Width / 2;
	TilePosition._Y = -(TilePositionY * en_Inventory::TILE_SIZE_HEIGHT + en_Inventory::TILE_SIZE_HEIGHT * Item->_ItemInfo.Height / 2);

	return TilePosition;
}

bool CInventory::BoundryCheck(int16 PositionX, int16 PositionY, int32 Width, int32 Height)
{
	if (PositionCheck(PositionX, PositionY) == false)
	{
		return false;
	}

	PositionX += Width - 1;
	PositionY += Height - 1;

	if (PositionCheck(PositionX, PositionY) == false)
	{
		return false;
	}

	return true;
}

bool CInventory::PositionCheck(int16 TilePositionX, int16 TilePositionY)
{
	// 위치 값이 음수일 경우 false
	if (TilePositionX < 0 || TilePositionY < 0)
	{
		return false;
	}

	if (TilePositionX >= _InventoryWidth || TilePositionY >= _InventoryHeight)
	{
		return false;
	}

	return true;
}

bool CInventory::OverlapCheck(int16 TilePositionX, int16 TilePositionY, int16 Width, int16 Height, CItem** OverlapItem)
{
	// 매개 변수로 받은 시작 위치로부터 넓이만큼 검사한다.
	for (int X = 0; X < Width; X++)
	{
		for (int Y = 0; Y < Height; Y++)
		{
			// 만약 검사 위치에 아이템이 있을 경우
			if (_Items[TilePositionX + X][TilePositionY + Y]->IsEmptySlot == false)
			{
				// OverlapItem이 null일 경우 즉, 첫번째로 중복된 아이템을 발견햇을 경우
				if (*OverlapItem == nullptr)
				{
					// OverlapItem에 검사 위치에서 발견한 아이템을 넣어준다.
					*OverlapItem = _Items[TilePositionX + X][TilePositionY + Y]->InventoryItem;
				}
				else
				{
					// 중복 아이템을 발견했고
					// 다른 위치를 검사하는데, 놓을 위치에 발견한 중복 아이템과 다른 아이템이 있을 경우
					// 놓을 위치에 2개 이상의 아이템이 존재한다는 것을 의미하므로 false를 반환한다.
					if (_Items[TilePositionX + X][TilePositionY + Y]->IsEmptySlot == false
						&& *OverlapItem != _Items[TilePositionX + X][TilePositionY + Y]->InventoryItem)
					{
						return false;
					}			
				}
			}
		}
	}

	return true;
}

CItem* CInventory::FindInventoryItem(en_SmallItemCategory FindItemSmallItemCategory)
{
	for (int Y = 0; Y < _InventoryHeight; Y++)
	{
		for (int X = 0; X < _InventoryWidth; X++)
		{
			if (_Items[Y][X]->IsEmptySlot == false && _Items[Y][X]->InventoryItem->_ItemInfo.ItemSmallCategory == FindItemSmallItemCategory)
			{
				return _Items[Y][X]->InventoryItem;
			}			
		}
	}	
	 
	return nullptr;
}

vector<CItem*> CInventory::FindAllInventoryItem(en_SmallItemCategory FindItemSmallItemCategory)
{
	bool FindItemCheck = true;

	vector<CItem*> FindItems;

	for (int Y = 0; Y < _InventoryHeight; Y++)
	{
		for (int X = 0; X < _InventoryWidth; X++)
		{
			if (_Items[Y][X]->IsEmptySlot == false && _Items[Y][X]->InventoryItem->_ItemInfo.ItemSmallCategory == FindItemSmallItemCategory)
			{
				for (CItem* FindItem : FindItems)
				{
					if (FindItem->_ItemInfo.TileGridPositionX == _Items[Y][X]->InventoryItem->_ItemInfo.TileGridPositionX
						&& FindItem->_ItemInfo.TileGridPositionY == _Items[Y][X]->InventoryItem->_ItemInfo.TileGridPositionY)
					{
						FindItemCheck = false;
						break;
					}
				}

				if (FindItemCheck == true)
				{
					FindItems.push_back(_Items[Y][X]->InventoryItem);
				}				
			}
		}
	}

	return FindItems;
}

vector<st_ItemInfo> CInventory::DBInventorySaveReturnItems()
{
	st_ItemInfo SaveItemInfo;
	
	vector<st_ItemInfo> ReturnItem;
	
	for (int X = 0; X < _InventoryWidth; X++)
	{
		for (int Y = 0; Y < _InventoryHeight; Y++)
		{
			if (_Items[X][Y]->IsEmptySlot == false)
			{
				ReturnItem.push_back(_Items[X][Y]->InventoryItem->_ItemInfo);
				// 사용 다한 아이템 메모리 반납
				G_ObjectManager->ObjectReturn(_Items[X][Y]->InventoryItem->_GameObjectInfo.ObjectType, (CGameObject*)_Items[X][Y]);
			}
			else
			{
				SaveItemInfo.TileGridPositionX = X;
				SaveItemInfo.TileGridPositionY = Y;				

				ReturnItem.push_back(SaveItemInfo);
			}
		}
	}

	return ReturnItem;
}
