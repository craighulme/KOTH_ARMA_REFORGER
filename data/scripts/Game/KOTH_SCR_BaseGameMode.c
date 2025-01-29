modded class SCR_BaseGameMode
{
	protected override void OnGameStart()
	{
		super.OnGameStart();
		
		string missionName = GetGame().GetMissionName();
		missionName.ToLower();
		if (missionName.Contains("lat valley") || missionName.Contains("langbiang") || missionName.Contains("hill 55") || missionName.Contains("dankia"))
		{
			ChimeraWorld world = GetGame().GetWorld();
			TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
			array<int> hours = {14, 8, 14, 18, 14};
			weatherManager.SetHoursMinutesSeconds(hours.GetRandomElement(),0,0);
		}
	}
}
