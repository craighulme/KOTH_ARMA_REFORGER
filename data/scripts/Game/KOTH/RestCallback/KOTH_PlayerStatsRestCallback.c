class KOTH_PlayerStatsRestCallback : RestCallback 
{
	override void OnError(int errorCode)
	{
		Log("KOTH_PlayerStatsRestCallback Error with code "+errorCode+" = "+typename.EnumToString(ERestResult, errorCode), LogLevel.ERROR);
	}

	override void OnTimeout()
	{
		Log("KOTH_PlayerStatsRestCallback timeout", LogLevel.ERROR);
	}

	override void OnSuccess(string data, int dataSize)
	{
		Log("KOTH_PlayerStatsRestCallback success data= "+data);
		KOTH_PlayerStatsJson stats = new KOTH_PlayerStatsJson();
		stats.ExpandFromRAW(data);

		KOTH_BackendApiGameModeComponent kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
		kothBackendApi.m_playerStatsList.Insert(stats.m_playerUID, stats);
	}
}
