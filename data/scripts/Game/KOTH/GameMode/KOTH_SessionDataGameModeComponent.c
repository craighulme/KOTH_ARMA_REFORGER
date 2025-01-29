class KOTH_SessionDataGameModeComponentClass : SCR_BaseGameModeComponentClass {}
class KOTH_SessionDataGameModeComponent : SCR_BaseGameModeComponent
{
	int m_totalGameTime = 0;
	ref map<string, ref KOTH_SessionPlayerData> m_sessionPlayersData = new map<string, ref KOTH_SessionPlayerData>();
	ref map<string, ref KOTH_SessionPlayerLoadout> m_sessionPlayersLoadout = new map<string, ref KOTH_SessionPlayerLoadout>();

	const string KOTH_TotalGameModeTimeKey = "KOTH_TotalGameModeTimeKey";
	protected ref SCR_TimeMeasurementHelper timeMeasurementHelper = new SCR_TimeMeasurementHelper();
	
	void SetSessionPointsWhenFactionWasJoined(int points, string faction, string playerUID)
	{
		KOTH_SessionPlayerData playerData = m_sessionPlayersData.Get(playerUID);
		if (playerData) {
			playerData.SetSessionPointsWhenFactionWasJoined(points, faction);
		} else {
			Log("KOTH_SessionDataGameModeComponent.SetSessionPointsWhenFactionWasJoined no found for playerUID "+playerUID);
		}
	}
	
	void AddSessionXpAndMoney(int money, int xp, string playerUID)
	{
		KOTH_SessionPlayerData playerData = m_sessionPlayersData.Get(playerUID);
		if (playerData) {
			playerData.AddSessionXpAndMoney(money, xp);
		} else {
			Log("KOTH_SessionDataGameModeComponent.AddSessionXpAndMoney no found for playerUID "+playerUID);
		}
	}

	void UpdateKillStreak(string playerUID, int kills)
	{
		KOTH_SessionPlayerData playerData = m_sessionPlayersData.Get(playerUID);
		if (playerData) {
			playerData.UpdateKillStreak(kills);
		} else {
			Log("KOTH_SessionDataGameModeComponent.UpdateKillStreak no found for playerUID "+playerUID);
		}
	}

	void UpdateInsertionStreak(string playerUID, int insertions)
	{
		KOTH_SessionPlayerData playerData = m_sessionPlayersData.Get(playerUID);
		if (playerData) {
			playerData.UpdateInsertionStreak(insertions);
		} else {
			Log("KOTH_SessionDataGameModeComponent.UpdateInsertionStreak no found for playerUID "+playerUID);
		}
	}
	
	KOTH_ShopItem FindKOTH_ItemShopFromResourceName(ResourceName resourceNameToSearch, ResourceName list)
	{
		KOTH_ShopItem shopItem = null;
		KOTH_ShopItemList itemList = SCR_ConfigHelperT<KOTH_ShopItemList>.GetConfigObject(list);
		foreach (int index, KOTH_ShopItem item : itemList.GetItems())
		{
			if (item.m_itemResource == resourceNameToSearch) {
				shopItem = item;
				break;
			}
		}

		return shopItem;
	}
	
	ref map<string, int> m_lastBackendSaveLoadoutCall = new map<string, int>();
	void SaveLoadoutChoiceInSession(KOTH_ShopItem item, int playerId, string playerUID)
	{
		if (playerUID == string.Empty)
			playerUID = KOTH_Helper.GetPlayerUID(playerId);
		
		KOTH_SessionPlayerLoadout playerLoadout = m_sessionPlayersLoadout.Get(playerUID);
		if (!playerLoadout)
			playerLoadout = new KOTH_SessionPlayerLoadout();
		
		switch (item.m_category)
		{
			case KOTH_ShopItemCategory.Primary:
				playerLoadout.m_primary = item;
			break;
			case KOTH_ShopItemCategory.Optics:
				playerLoadout.m_optic = item;
			break;
			case KOTH_ShopItemCategory.Muzzle:
				playerLoadout.m_muzzle = item;
			break;
			case KOTH_ShopItemCategory.Launcher:
				playerLoadout.m_launcher = item;
			break;
			case KOTH_ShopItemCategory.Handgun:
				playerLoadout.m_handgun = item;
			break;
			case KOTH_ShopItemCategory.ViperHood:
				playerLoadout.m_viperhood = item;
			break;
			case KOTH_ShopItemCategory.Binocular:
				playerLoadout.m_rangeFinder = item;
			break;
			case KOTH_ShopItemCategory.Grenade:
				int nadeCounter = 0;
				foreach(KOTH_ShopItem throwable : playerLoadout.m_throwables)
				{
					if (throwable.m_category == KOTH_ShopItemCategory.Grenade)
						nadeCounter++;
				}

				if (nadeCounter < 5)
					playerLoadout.m_throwables.Insert(item);
			break;
			case KOTH_ShopItemCategory.Smoke:
				int smokeCounter = 0;
				foreach(KOTH_ShopItem throwable : playerLoadout.m_throwables)
				{
					if (throwable.m_category == KOTH_ShopItemCategory.Smoke)
						smokeCounter++;
				}

				if (smokeCounter < 5)
					playerLoadout.m_throwables.Insert(item);
			break;
			default:
			break;
		}
		
		m_sessionPlayersLoadout.Set(playerUID, playerLoadout);
		m_lastBackendSaveLoadoutCall.Set(playerUID, System.GetUnixTime());
		GetGame().GetCallqueue().CallLater(SaveLoadoutInBackend, 10000, false, playerUID);
	}
	
	void SaveLoadoutInBackend(string playerUID)
	{
		int time = m_lastBackendSaveLoadoutCall.Get(playerUID);
		if (System.GetUnixTime() - time < 10)
			return;
		
		KOTH_SessionPlayerLoadout sessionLoadout = m_sessionPlayersLoadout.Get(playerUID);
		if (sessionLoadout)
		{
			KOTH_BackendApiGameModeComponent kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
			KOTH_PlayerPresetJson preset = new KOTH_PlayerPresetJson();
			preset.PopulateFromSessionLoadout(sessionLoadout);
			kothBackendApi.SendProfilePresetToBackend(preset, playerUID);
		}
	}
	
	void StartTimeMeasurement(string playerUID)
	{
		timeMeasurementHelper.BeginMeasure(playerUID);
	}
	
	void StopTimeMeasurement(string playerUID)
	{
		timeMeasurementHelper.EndMeasure(playerUID);
		KOTH_SessionPlayerData sessionPlayerData = m_sessionPlayersData.Get(playerUID);

		if (sessionPlayerData)
		{
			float timespent = timeMeasurementHelper.GetMeasure(playerUID);
			int actualTotalTimespent = sessionPlayerData.GetSessionTimePlayed() + ((int) (timespent/1000));
			sessionPlayerData.SetSessionTimePlayed(actualTotalTimespent);
		} else {
			Log("no session data found for playerUID "+playerUID, LogLevel.ERROR);
		}
	}
	
	void StopPlayersTimeTracking()
	{
		timeMeasurementHelper.EndMeasure(KOTH_TotalGameModeTimeKey);
		float timeqwespent = timeMeasurementHelper.GetMeasure(KOTH_TotalGameModeTimeKey);
		m_totalGameTime = timeqwespent / 1000;
		
		float timespent = 0.0;
		foreach (string keyPlayerUID, KOTH_SessionPlayerData playerSession: m_sessionPlayersData)
		{
			timespent = timeMeasurementHelper.GetMeasure(keyPlayerUID);
			playerSession.SetSessionTimePlayed(playerSession.GetSessionTimePlayed() + ((int)(timespent/1000)));
		}
	}
}


