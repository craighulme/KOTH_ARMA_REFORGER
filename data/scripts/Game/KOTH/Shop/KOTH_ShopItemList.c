[BaseContainerProps(configRoot:true)]
class KOTH_ShopItemList
{
	[Attribute("", UIWidgets.Auto)]
	protected ref array<ref KOTH_ShopItem> m_items;

	array<ref KOTH_ShopItem> GetItems()
	{
		return m_items;
	}
	
	array<ref KOTH_ShopItem> GetItemsOrderedByLevel()
	{
		return SortByLevel(m_items);
	}
	
	array<ref KOTH_ShopItem> SortByLevel(array<ref KOTH_ShopItem> itemList)
	{
		KOTH_ShopItem previousItem;
		bool isSortFinish = true;
		foreach(int index, KOTH_ShopItem item : itemList)
		{
			if (previousItem && previousItem.m_level > item.m_level)
			{
				isSortFinish = false;
				itemList.RemoveItemOrdered(previousItem);
				itemList.RemoveItemOrdered(item);
				itemList.InsertAt(item, index - 1);
				itemList.InsertAt(previousItem, index);
			}
			
			previousItem = item;
		}
		
		if (!isSortFinish)
			return SortByLevel(itemList);

		return m_items;
	}
}
