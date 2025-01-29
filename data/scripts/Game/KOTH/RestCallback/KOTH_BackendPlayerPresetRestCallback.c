class KOTH_BackendPlayerPresetRestCallback : RestCallback 
{
	override void OnError(int errorCode)
	{
		Log("KOTH_BackendPlayerPresetRestCallback Error with code "+errorCode, LogLevel.ERROR);
	}

	override void OnTimeout()
	{
		Log("KOTH_BackendPlayerPresetRestCallback timeout", LogLevel.ERROR);
	}

	override void OnSuccess(string data, int dataSize)
	{
		Log("KOTH_BackendPlayerPresetRestCallback success size= " + dataSize.ToString() + " data= "+data);
	}
}
