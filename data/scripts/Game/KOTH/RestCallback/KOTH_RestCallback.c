class KOTH_RestCallback : RestCallback 
{
	override void OnError(int errorCode)
	{
		Log("KOTH_RestCallback Error with code "+errorCode+" = "+typename.EnumToString(ERestResult, errorCode), LogLevel.ERROR);
	};

	override void OnTimeout()
	{
		Log("KOTH_RestCallback timeout", LogLevel.ERROR);
	};

	override void OnSuccess(string data, int dataSize)
	{
		Log("KOTH_RestCallback success data= "+data);
	};
};
