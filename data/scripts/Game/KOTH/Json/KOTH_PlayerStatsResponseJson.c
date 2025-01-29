class KOTH_PlayerStatsResponseJson : JsonApiStruct
{
	string playerUID;
	string stats;

	void KOTH_PlayerStatsResponseJson()
	{
		RegV("playerUID");
		RegV("stats");
	}
}
