class KOTH_ListPlayerBanJson : JsonApiStruct
{
	ref array<string> m_list;

	void KOTH_ListPlayerBanJson()
	{
		RegV("m_list");
	}
}
