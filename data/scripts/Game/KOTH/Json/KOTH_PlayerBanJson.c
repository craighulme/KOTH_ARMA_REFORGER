class KOTH_PlayerBanJson : JsonApiStruct
{
	bool active;
	string playerUID;

	void KOTH_PlayerBanJson()
	{
		RegV("active");
		RegV("playerUID");
	}
}
