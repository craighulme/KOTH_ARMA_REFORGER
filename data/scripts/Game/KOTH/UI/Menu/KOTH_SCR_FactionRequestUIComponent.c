modded class SCR_FactionRequestUIComponent
{
	override void ShowAvailableFactions()
	{
		super.ShowAvailableFactions();

		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_FactionButton factionBtn = SCR_FactionButton.Cast(btn);
			if (factionBtn)
			{
				factionBtn.UpdateButtons();
			}
		}
	}
	override protected void UpdateFactionButtons(Faction faction, int newCount)
	{
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_FactionButton factionBtn = SCR_FactionButton.Cast(btn);
			if (factionBtn)
			{
				factionBtn.UpdatePlayerCount();
				factionBtn.UpdateButtons();
			}
		}
	}
}

modded class SCR_FactionButton
{

//	override bool OnFocus(Widget w, int x, int y)
//	{
//		UpdateButtons();
//		return true;
//	}

//	override void SetSelected(bool selected)
//	{
//		UpdateButtons();
//	}

	bool UpdateButtons()
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return false;

		int playableFactionCount = FactionHelper.GetPlayableFactionCount();
		int highestCount = FactionHelper.GetHighestPlayerCount();
		Faction currentFaction = m_Faction;

		// Allow joining if no players exist yet or up to 2
		if (highestCount < 2)
		{
			SetEnabled(true);
			SetShouldUnlock(true);
			return true;
		}

		if (FactionHelper.IsFactionPlayable(currentFaction))
		{
			int currentFactionCount = FactionHelper.GetFactionPlayerCount(currentFaction);

			//enable current faction button if all factions are balanced or current faction count less than max
			if (FactionHelper.AreAllFactionsBalanced(highestCount))
			{
				SetEnabled(true);
				SetShouldUnlock(true);
				return true;
			}
			
			//enable current faction button if less than the faction with highest player count
			if (currentFactionCount < highestCount)
			{
				SetEnabled(true);
				SetShouldUnlock(true);
				return true;
			}

			//disable button otherwise
			SetEnabled(false);
			SetShouldUnlock(false);
			return false;
		}

		//fallback to enable button
		SetEnabled(true);
		SetShouldUnlock(true);
		return true;
	}
}

// KOTH_TODO: owwww gad plz refactor this entire file you monster
modded class SCR_PlayerFactionAffiliationComponent
{
	override protected bool CanRequestFaction_S(Faction faction)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::CanRequestFaction_S(Faction: %2)", Type().ToString(), faction), LogLevel.NORMAL);
		#endif

		// Allow leaving the faction (no faction selected)
	    if (!faction)
	        return true;
		
		if (!FactionHelper.IsFactionPlayable(faction))
			return false;

		//taken from original method to stop bad logging
		if (GetAffiliatedFaction() == faction)
			return false;

		int playableFactionCount = FactionHelper.GetPlayableFactionCount();
		int highestCount = FactionHelper.GetHighestPlayerCount();

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		int factionPlayerCount = FactionHelper.GetFactionPlayerCount(faction);

		// Allow joining if players less than 2 at start
		if (highestCount < 2)
			return true;
		
		// Allow joining if all faction counts are the same
		if (FactionHelper.AreAllFactionsBalanced(highestCount))
			return true;
		
		// Allow joining if players less than faction with a higher count
		if (factionPlayerCount < highestCount)
			return true;

		return false;
	}
}


//used to share faction info between modded classes for UI
class FactionHelper
{
	static ref const array<string> factionKeys = {KOTH_Faction.BLUFOR, KOTH_Faction.OPFOR, KOTH_Faction.INDFOR};

	static int GetHighestPlayerCount()
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return 0;

		int highestCount = 0;

		foreach (string key : factionKeys)
		{
			Faction faction = factionManager.GetFactionByKey(key);
			if (IsFactionPlayable(faction))
			{
				int playerCount = factionManager.GetFactionPlayerCount(faction);
				if (highestCount < playerCount)
					highestCount = playerCount;
			}
		}

		return highestCount;
	}

	//grab the number of players currently within a faction
	static int GetFactionPlayerCount(Faction faction)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager || !faction)
			return 0;

		return factionManager.GetFactionPlayerCount(faction);
	}

	//this is the number of playable factions
	static int GetPlayableFactionCount()
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return 0;

		int count = 0;

		foreach (string key : factionKeys)
		{
			Faction faction = factionManager.GetFactionByKey(key);
			if (IsFactionPlayable(faction))
				count++;
		}

		return count;
	}

	//checks to make sure all factions have the same number of players
	static bool AreAllFactionsBalanced(int highestCount)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return false;

		foreach (string key : factionKeys)
		{
			Faction faction = factionManager.GetFactionByKey(key);
			if (IsFactionPlayable(faction) && FactionHelper.GetFactionPlayerCount(faction) != highestCount)
				return false;
		}

		return true;
	}

	// Dynamically check if faction is playable and may have been disabled by gamemaster
	static bool IsFactionPlayable(Faction faction)
	{
		if (!faction)
			return false;

		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		return scrFaction && scrFaction.IsPlayable(); 
	}
}
