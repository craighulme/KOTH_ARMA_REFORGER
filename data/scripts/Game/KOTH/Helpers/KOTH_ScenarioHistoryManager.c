class KOTH_ScenarioHistoryManager
{
	const string scenarioHistoryFilePath = "$profile:koth_scenarioHistory.json";

	static void HandleScenarioHistory()
	{
		// handle history scenario
		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		Log("HandleScenarioHistory start");
		if (header) 
		{
			string name = header.GetHeaderResourceName();
			if (name != string.Empty) 
			{
				Log("HandleScenarioHistory name = "+name);
				ArmaReforgerScripted game = GetGame();
				if (!game.KOTH_ScenarioHistory || game.KOTH_ScenarioHistory.m_list.Count() == 0) 
				{
					KOTH_ScenarioHistoryJson scenarioHistoryJson = new KOTH_ScenarioHistoryJson();
					scenarioHistoryJson.LoadFromFile(scenarioHistoryFilePath);
					game.KOTH_ScenarioHistory = scenarioHistoryJson;
					foreach (string scenar : game.KOTH_ScenarioHistory.m_list)
					{
						Log("Loaded from file scenar history = " + scenar);
					}
				}
				
				if (!game.KOTH_ScenarioHistory.m_list.Contains(name) && game.KOTH_ScenarioHistory.m_list.Count() > 0) 
				{
					string nextScenario = game.KOTH_ScenarioHistory.m_list.Get(game.KOTH_ScenarioHistory.m_list.Count() - 1);
					if (nextScenario != string.Empty && nextScenario != name) 
					{
						Log("HandleScenarioHistory reloaded scenario = "+nextScenario+" instead of "+name);
						SCR_BaseGameMode.Cast(game.GetGameMode()).ChangeScenario(nextScenario);
					}
				}
			}
		}
		Log("HandleScenarioHistory end");
	}
	
	static array<ResourceName> GetMapChoices()
	{
		ArmaReforgerScripted game = GetGame();
		if (!game.KOTH_ScenarioHistory)
		{
			KOTH_ScenarioHistoryJson scenarioHistoryJson = new KOTH_ScenarioHistoryJson();
			scenarioHistoryJson.LoadFromFile(scenarioHistoryFilePath);
			game.KOTH_ScenarioHistory = scenarioHistoryJson;
		}
		if (!game.KOTH_ScenarioHistory || game.KOTH_ScenarioHistory.m_list.Count() == 0)
		{
			KOTH_ScenarioHistoryJson scenarioHistoryJson = new KOTH_ScenarioHistoryJson();
			scenarioHistoryJson.LoadFromFile(scenarioHistoryFilePath);
			game.KOTH_ScenarioHistory = scenarioHistoryJson;
			Log("ChooseNextScenario KOTH_ScenarioHistory is empty, loaded from file");
		}
		
		array<ref KOTH_ScenarioEntry> scenarioList = SCR_ConfigHelperT<KOTH_Scenarios>
				.GetConfigObject("{01AF5BCD303E87E6}Configs/System/ScenarioList.conf")
				.GetItems();
		array<ResourceName> all = {};
		foreach(KOTH_ScenarioEntry scenario : scenarioList)
		{
			all.InsertAll(scenario.GetItems());
		}
		
		if (GetGame().GetPlayerManager().GetPlayerCount() >= 35) 
		{
			all.RemoveItem("{43A2DC866897E5FE}Missions/KOTH-Arleville-V1.conf");
			all.RemoveItem("{DD04E8FC104D01F5}Missions/KOTH-Chotain-V1.conf");
			all.RemoveItem("{F8C8A21273E32FB9}Missions/KOTH-Beauregard-V1.conf");
		}
		if (GetGame().GetPlayerManager().GetPlayerCount() < 35) 
		{
			all.RemoveItem("{08096F926A21678B}Missions/KOTH-Saintphilipe-V1.conf");
			all.RemoveItem("{8ADD561C1F2F256F}Missions/KOTH-Montignac-V1.conf");
			all.RemoveItem("{D83C0CE8539C8A73}Missions/KOTH-SaintPierre.conf");
		}

		foreach (ResourceName scenar : game.KOTH_ScenarioHistory.m_list)
		{
			all.RemoveItem(scenar);
		}
		
		SCR_ArrayHelperT<ResourceName>.Shuffle(all, 3);

		return all;
	}
	
	static string ChooseNextScenario()
	{
		return GetMapChoices().GetRandomElement();
	}
	
	static void AddScenarioToHistory(string scenario)
	{
		ArmaReforgerScripted game = GetGame();	
		foreach (string scenar : game.KOTH_ScenarioHistory.m_list)
		{
			Log("previous scenar history = " + scenar);
		}

		game.KOTH_ScenarioHistory.m_list.Insert(scenario);
		
		if (game.KOTH_ScenarioHistory.m_list.Count() > 4)
			game.KOTH_ScenarioHistory.m_list.RemoveOrdered(0);

		foreach (string scenar : game.KOTH_ScenarioHistory.m_list)
		{
			Log("new scenar history = " + scenar);
		}

		game.KOTH_ScenarioHistory.SaveToFile(scenarioHistoryFilePath);
	}
}

