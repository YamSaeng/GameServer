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
				if (_Items[Y][X] != nullptr)
				{
					G_ObjectManager->ObjectReturn(_Items[Y][X]->InventoryItem);
					delete _Items[Y][X];
				}				
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
	for (int X = 0; X < CleanItem->_ItemInfo.ItemWidth; X++)
	{
		for (int Y = 0; Y < CleanItem->_ItemInfo.ItemHeight; Y++)
		{
			_Items[CleanItem->_ItemInfo.ItemTileGridPositionX + X][CleanItem->_ItemInfo.ItemTileGridPositionY + Y]->IsEmptySlot = true;
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

	int SearchingHeight = _InventoryHeight - ItemInfo->_ItemInfo.ItemHeight + 1;
	int SearchingWidth = _InventoryWidth - ItemInfo->_ItemInfo.ItemWidth + 1;

	for (int Y = 0; Y < SearchingHeight; Y++)
	{
		for (int X = 0; X < SearchingWidth; X++)
		{
			if (CheckEmptySpace(X, Y, ItemInfo->_ItemInfo.ItemWidth, ItemInfo->_ItemInfo.ItemHeight) == true)
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
	return CheckEmptySpace(Item->_ItemInfo.ItemTileGridPositionX, Item->_ItemInfo.ItemTileGridPositionY, Item->_ItemInfo.ItemWidth, Item->_ItemInfo.ItemHeight);
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
	if (BoundryCheck(PositionX, PositionY, PlaceItemInfo->_ItemInfo.ItemWidth, PlaceItemInfo->_ItemInfo.ItemHeight) == false)
	{
		return false;
	}

	// 아이템을 넣을 위치에 아이템이 이미 있는지 확인한다.
	if (OverlapCheck(PositionX, PositionY, PlaceItemInfo->_ItemInfo.ItemWidth, PlaceItemInfo->_ItemInfo.ItemHeight, OverlapItem) == false)
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
	PlaceItemInfo->_ItemInfo.ItemTileGridPositionX = PositionX;
	PlaceItemInfo->_ItemInfo.ItemTileGridPositionY = PositionY;

	// 아이템의 넓이만큼 시작 위치에서 슬롯에 채워넣는다.
	for (int X = 0; X < PlaceItemInfo->_ItemInfo.ItemWidth; X++)
	{
		for (int Y = 0; Y < PlaceItemInfo->_ItemInfo.ItemHeight; Y++)
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
		
		int16 InitTileWidth = InitInventoryItem->InventoryItem->_ItemInfo.ItemWidth;
		int16 InitTileHeight = InitInventoryItem->InventoryItem->_ItemInfo.ItemHeight;

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
	TilePosition._X = TilePositionX * en_Inventory::TILE_SIZE_WIDTH + en_Inventory::TILE_SIZE_WIDTH * Item->_ItemInfo.ItemWidth / 2;
	TilePosition._Y = -(TilePositionY * en_Inventory::TILE_SIZE_HEIGHT + en_Inventory::TILE_SIZE_HEIGHT * Item->_ItemInfo.ItemHeight / 2);

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
 	for (int X = 0; X < _InventoryWidth; X++)
	{
		for (int Y = 0; Y < _InventoryHeight; Y++)
		{
			if (_Items[X][Y]->IsEmptySlot == false && _Items[X][Y]->InventoryItem->_ItemInfo.ItemSmallCategory == FindItemSmallItemCategory)
			{
				return _Items[X][Y]->InventoryItem;
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
					if (FindItem->_ItemInfo.ItemTileGridPositionX == _Items[Y][X]->InventoryItem->_ItemInfo.ItemTileGridPositionX
						&& FindItem->_ItemInfo.ItemTileGridPositionY == _Items[Y][X]->InventoryItem->_ItemInfo.ItemTileGridPositionY)
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
	
	// 가방 아이템 반납	
	for (int X = 0; X < _InventoryWidth; X++)
	{
		for (int Y = 0; Y < _InventoryHeight; Y++)
		{
			// 아이템이 들어가 있는 공간 확인		
			if (_Items[X][Y]->IsEmptySlot == false)
			{
				// 아이템 반납 진행
				ReturnItem.push_back(_Items[X][Y]->InventoryItem->_ItemInfo);
				// 사용 다한 아이템 메모리 반납
				G_ObjectManager->ObjectReturn((CGameObject*)_Items[X][Y]);	

				// 아이템 공간 크기 가져옴
				int16 InitWidth = _Items[X][Y]->InventoryItem->_ItemInfo.ItemWidth;
				int16 InitHeight = _Items[X][Y]->InventoryItem->_ItemInfo.ItemHeight;

				// 가방에서 비움 처리
				for (int16 WidthX = 0; WidthX < InitWidth; WidthX++)
				{
					for (int16 HeightY = 0; HeightY < InitHeight; HeightY++)
					{
						_Items[X + WidthX][Y + HeightY]->IsEmptySlot = true;
						_Items[X + WidthX][Y + HeightY]->InventoryItem = nullptr;
					}
				}
			}
			else
			{
				SaveItemInfo.ItemTileGridPositionX = X;
				SaveItemInfo.ItemTileGridPositionY = Y;				

				ReturnItem.push_back(SaveItemInfo);
			}
		}
	}

	return ReturnItem;
}
