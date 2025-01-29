class KOTH_BonusCodeResponseJson : JsonApiStruct
{
	string name;
	string code;
	string playerUID;
	string multiplier;
	string dateEnd;

	bool error;
	string errorReason;

	void KOTH_BonusCodeResponseJson()
	{
		RegV("name");
		RegV("code");
		RegV("playerUID");
		RegV("multiplier");
		RegV("dateEnd");

		RegV("error");
		RegV("errorReason");
	}
}
