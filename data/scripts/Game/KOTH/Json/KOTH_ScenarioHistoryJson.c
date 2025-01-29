class KOTH_ScenarioHistoryJson : JsonApiStruct
{
	ref array<ref string> m_list = {};

	void KOTH_ScenarioHistoryJson()
	{
		RegV("m_list");
	}
}
