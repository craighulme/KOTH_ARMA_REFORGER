modded class ArmaReforgerScripted
{
	string KOTH_ApiKey = "official_testapikey";
	string KOTH_BackendBaseUri = "http://kothrbackend.docker/api/";
	ref KOTH_ScenarioHistoryJson KOTH_ScenarioHistory = new KOTH_ScenarioHistoryJson();
	int maxPlayers;

	//handle showing some UIs
	bool ShowTeamNames = true;
	bool ShowScoreUI = true;
	bool ShowWaypointAO = true;
	
	override bool OnGameStart()
	{
		super.OnGameStart();

		#ifdef WORKBENCH
			KOTH_DebugMenu.Init();
		#endif
		
		BackendApi backendApi = GetGame().GetBackendApi();
		DSSession sessionDS = backendApi.GetDSSession();
		
		if (sessionDS)
			maxPlayers = sessionDS.PlayerLimit();

		return true;
	}
	
	override void OnUpdate(BaseWorld world, float timeslice)
	{
		super.OnUpdate(world, timeslice);
		
		#ifdef WORKBENCH
			KOTH_DebugMenu.UpdateMenus();
		#endif
	}
}