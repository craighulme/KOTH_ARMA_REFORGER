modded class SCR_NameTagDisplay
{
	private ArmaReforgerScripted game = GetGame();

	override protected void InitNameTags()
	{
		super.InitNameTags();
		GetGame().GetCallqueue().CallLater(CheckNameTagVisibility, 1000, true);		
	}
	
	void CheckNameTagVisibility()
	{
		if (game.ShowTeamNames)
		{
			m_bSleepDisplay = false;
		}
		
		if (!game.ShowTeamNames)
		{
			m_bSleepDisplay = true;
		}
	}
}