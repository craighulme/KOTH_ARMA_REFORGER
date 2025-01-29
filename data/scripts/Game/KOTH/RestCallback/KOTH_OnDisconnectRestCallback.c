class KOTH_OnDisconnectRestCallback : RestCallback 
{
	override void OnError(int errorCode)
	{
		Log("KOTH_OnDisconnectRestCallback Error with code "+errorCode, LogLevel.ERROR);
	};

	override void OnTimeout()
	{
		Log("KOTH_OnDisconnectRestCallback timeout", LogLevel.ERROR);
	};

	override void OnSuccess(string data, int dataSize)
	{
		Log("KOTH_OnDisconnectRestCallback success data= "+data);
		KOTH_PlayerProfileJson profile = new KOTH_PlayerProfileJson();
		profile.ExpandFromRAW(data);
		KOTH_BackendApiGameModeComponent kothBackendApiGameModeComp = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
		kothBackendApiGameModeComp.RemoveProfileToCurrentList(profile.m_playerUID);
	};
};
