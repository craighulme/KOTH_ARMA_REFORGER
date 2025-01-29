class KOTH_CustomizabilityManager
{
	static bool isUnlocked(array<string> unlockedItems, ResourceName itemResource)
	{
				
		if (unlockedItems.Contains(itemResource))
			return true;
		
		if (itemResource.Contains("VIPERHOOD") && arrayContains(unlockedItems, "VIPERHOOD"))
			return true;
		
		return false;
	}
	
	// Checks whether array has a value that contains
	static bool arrayContains(array<string> searchArray, string containsString)
	{
		foreach (string item : searchArray) {
			if (item.Contains(containsString))
				return true;
		}
		
		return false;
	}
}