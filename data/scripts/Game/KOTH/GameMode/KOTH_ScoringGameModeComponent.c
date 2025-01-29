class KOTH_ScoringGameModeComponentClass : SCR_BaseGameModeComponentClass {}
class KOTH_ScoringGameModeComponent : SCR_BaseGameModeComponent
{
	[Attribute(params: "conf")]
	protected ResourceName m_weaponShopItemList;
	ResourceName GetWeaponShopItemList() { return m_weaponShopItemList; }

	[Attribute()]
	protected ResourceName m_muzzleShopItemList;
	ResourceName GetMuzzleShopItemList() { return m_muzzleShopItemList; }
	
	[Attribute()]
	protected ResourceName m_explosiveShopItemList;
	ResourceName GetExplosiveShopItemList() { return m_explosiveShopItemList; }
	
	[Attribute()]
	protected ResourceName m_opticsShopItemList;
	ResourceName GetOpticsShopItemList() { return m_opticsShopItemList; }
	
	[Attribute()]
	protected ResourceName m_accessoryShopItemList;
	ResourceName GetAccessoryShopItemList() { return m_accessoryShopItemList; }
	
	[Attribute()]
	protected ResourceName m_vehicleShopItemList;
	ResourceName GetVehicleShopItemList() { return m_vehicleShopItemList; }


	// empty or KOTH_FACTION
	[RplProp()]
	protected string m_currentFactionCapturing = string.Empty;
	string GetCurrentFactionCapturing() { return m_currentFactionCapturing; }
	void SetCurrentFactionCapturing(string faction) { m_currentFactionCapturing = faction; };
		
	[RplProp()]
	protected int m_blueforPoints = 0;
	int GetBlueforPoint() { return m_blueforPoints; }
	void AddBlueforPoint(int i = 0) 
	{
		m_currentFactionCapturing = KOTH_Faction.BLUFOR;
		if (i == 0) {
			m_blueforPoints++; 
		} else {
			m_blueforPoints = m_blueforPoints + i;
		}
	}
	void RemoveBlueforPoint(int i = 0) 
	{
		if (i == 0) {
			m_blueforPoints--; 
		} else {
			m_blueforPoints = m_blueforPoints - i;
		}
		
		if (m_blueforPoints < 0)
			m_blueforPoints = 0;
	}
	
	[RplProp()]
	protected int m_redforPoints = 0;
	int GetRedforPoint() { return m_redforPoints; }
	void AddRedforPoint(int i = 0) 
	{
		m_currentFactionCapturing = KOTH_Faction.OPFOR;
		if (i == 0) {
			m_redforPoints++; 
		} else {
			m_redforPoints = m_redforPoints + i;
		}
	}
	void RemoveRedforPoint(int i = 0) 
	{ 
		if (i == 0) {
			m_redforPoints--; 
		} else {
			m_redforPoints = m_redforPoints - i;
		}
		
		if (m_redforPoints < 0)
			m_redforPoints = 0;
	}
	
	[RplProp()]
	protected int m_greenforPoints = 0;
	int GetGreenforPoint() { return m_greenforPoints; }
	void AddGreenforPoint(int i = 0) 
	{
		m_currentFactionCapturing = KOTH_Faction.INDFOR;
		if (i == 0) {
			m_greenforPoints++; 
		} else {
			m_greenforPoints = m_greenforPoints + i;
		}
	}
	void RemoveGreenforPoint(int i = 0) 
	{ 
		if (i == 0) {
			m_greenforPoints--; 
		} else {
			m_greenforPoints = m_greenforPoints - i;
		}
		
		if (m_greenforPoints < 0)
			m_greenforPoints = 0;
	}
	
	[RplProp()]
	protected int m_bluePlayers = 0;
	int GetBluePlayers() { return m_bluePlayers; }
	
	[RplProp()]
	protected int m_redPlayers = 0;
	int GetRedPlayers() { return m_redPlayers; }
	
	[RplProp()]
	protected int m_greenPlayers = 0;
	int GetGreenPlayers() { return m_greenPlayers; }

	int m_bluforArmedVehiclesCount = 0;
	int m_opforArmedVehiclesCount = 0;
	int m_indforArmedVehiclesCount = 0;


	override void OnGameModeStart()
	{
		super.OnGameModeStart();

		if (!Replication.IsServer())
			return;

		KOTH_SessionDataGameModeComponent sessionDataGameComp = KOTH_SessionDataGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_SessionDataGameModeComponent));
		sessionDataGameComp.StartTimeMeasurement(KOTH_SessionDataGameModeComponent.KOTH_TotalGameModeTimeKey);
		
		GetGame().GetCallqueue().CallLater(UpdatePlayerCount, 15000, true);
	}

	// compute team and playtime bonus for each players
	void OnBeforeGameEnd()
	{
		float blueFactor = m_blueforPoints / SCR_BaseGameMode.WINNER_POINTS_NEEDED;
		float redFactor = m_redforPoints / SCR_BaseGameMode.WINNER_POINTS_NEEDED;
		float greenFactor = m_greenforPoints / SCR_BaseGameMode.WINNER_POINTS_NEEDED;
		if (blueFactor < 0.1) { blueFactor = 0.1; }
		if (redFactor < 0.1) { redFactor = 0.1; }
		if (greenFactor < 0.1) { greenFactor = 0.1; }
		
		KOTH_SessionDataGameModeComponent sessionDataGameComp = KOTH_SessionDataGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_SessionDataGameModeComponent));
		sessionDataGameComp.StopPlayersTimeTracking();

		array<int> playerIds = {} ;
		SCR_BaseGameMode gameMode = GetGameMode();
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		playerManager.GetPlayers(playerIds);
		array<ResourceName> mapList = KOTH_ScenarioHistoryManager.GetMapChoices();
		foreach (ResourceName smap : mapList) { m_mapVotes.Insert(smap, 0); }

		foreach (int playerId : playerIds)
		{
			IEntity controlledEntity = playerManager.GetPlayerControlledEntity(playerId);
			string playerUID = KOTH_Helper.GetPlayerUID(playerId);
			
			// make everyone invincible
			if (controlledEntity) 
			{
				SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(controlledEntity.FindComponent(SCR_DamageManagerComponent));
				if (damageManager)
					damageManager.EnableDamageHandling(false);
			}
			
			KOTH_SessionPlayerData sessionPlayerData = sessionDataGameComp.m_sessionPlayersData.Get(playerUID);
			if (!sessionPlayerData) 
			{
				Log("OnBeforeGameEnd no sessionPlayerData for name "+playerManager.GetPlayerName(playerId)+" playerUID "+playerUID);
				continue;
			}

			float teambonus;
			float teampointsfactor;
			float percentTimePlayed = sessionPlayerData.GetSessionTimePlayed() / sessionDataGameComp.m_totalGameTime;
			if (percentTimePlayed > 0.70) { percentTimePlayed = 1; }

			if (sessionPlayerData.GetSessionFaction() == KOTH_Faction.BLUFOR) 
			{
				teambonus = blueFactor * sessionPlayerData.GetSessionXpEarned() * 0.3;
				teampointsfactor = (m_blueforPoints - sessionPlayerData.GetSessionPointsWhenFactionWasJoined()) / SCR_BaseGameMode.WINNER_POINTS_NEEDED;
			}
			if (sessionPlayerData.GetSessionFaction() == KOTH_Faction.OPFOR) 
			{
				teambonus = redFactor * sessionPlayerData.GetSessionXpEarned() * 0.3;
				teampointsfactor = (m_redforPoints - sessionPlayerData.GetSessionPointsWhenFactionWasJoined()) / SCR_BaseGameMode.WINNER_POINTS_NEEDED;
			}
			if (sessionPlayerData.GetSessionFaction() == KOTH_Faction.INDFOR) 
			{
				teambonus = greenFactor * sessionPlayerData.GetSessionXpEarned() * 0.3;
				teampointsfactor = (m_greenforPoints - sessionPlayerData.GetSessionPointsWhenFactionWasJoined()) / SCR_BaseGameMode.WINNER_POINTS_NEEDED;
			}
			
			if (teampointsfactor < 0.1) 
				teampointsfactor = 0.1;
			
			teambonus = teambonus * teampointsfactor;

			float playtimebonus = 5000 * percentTimePlayed;
			float selfbonus = sessionPlayerData.GetSessionXpEarned() * 0.4;
			int bonus = (selfbonus + playtimebonus + teambonus).ToString(lenDec: 0).ToInt();
			Log("endgamebonus GetSessionXpEarned "+sessionPlayerData.GetSessionXpEarned()+" playtimebonus "+playtimebonus+" selfbonus "+selfbonus+" teambonus "+teambonus);

			KOTH_BackendApiGameModeComponent kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
			KOTH_PlayerProfileJson profile = kothBackendApi.m_CurrentProfileList.Get(playerUID);
			if (profile) {
				profile.AddEndGameBonusXpAndMoney(bonus);
			} else {
				Log("OnBeforeGameEnd no profile found for name: "+playerManager.GetPlayerName(playerId)+" playerUID: "+playerUID, LogLevel.ERROR);
			}
			
			KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerManager.GetPlayerController(playerId).FindComponent(KOTH_SCR_PlayerProfileComponent));
			profileComp.DoRpc_SetEndGameBonus(bonus);
			profileComp.DoRpc_SendMapChoices(mapList);
			kothBackendApi.DoRpcSyncProfileToPlayer(profile);
		}
	}

	protected ref map<string, int> m_mapVotes = new map<string, int>();
	string GetMostVotedMapConf()
	{
		KOTH_BackendApiGameModeComponent kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
		int maxVote = 0;
		string currentWinner;
		foreach(string key, int votes : m_mapVotes)
		{
			if (key != string.Empty && votes != 0)
				kothBackendApi.SendStatVoteMap(key, votes);

			if (maxVote < votes)
			{
				currentWinner = key;
				maxVote = votes;
			}
		}

		if (currentWinner != string.Empty && maxVote > 2)
			kothBackendApi.SendStatSelectedVoteMap(currentWinner);

		return currentWinner;
	}
	
	protected ref array<int> m_playerVotes = {};
	void PlayerVote(int playerId, string mapChoice)
	{
		// Check if playerId is not 0 and hasn't been used before
		if (playerId != 0 && !m_playerVotes.Contains(playerId)) {
			m_mapVotes.Set(mapChoice, m_mapVotes.Get(mapChoice) + 1);
			m_playerVotes.Insert(playerId);
		}
	}
	
	void UpdatePlayerCount()
	{
		// playerCounts
		array<int> playerIds = {};
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		playerManager.GetPlayers(playerIds);

		int countBluefor;
		int countGreenfor;
		int countRedfor;

		foreach (int playerId : playerIds)
		{
			IEntity entity = playerManager.GetPlayerControlledEntity(playerId);
			if (!entity)
				continue;

			FactionAffiliationComponent targetFactionComp = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
			if (!targetFactionComp)
				continue;

			Faction faction = targetFactionComp.GetAffiliatedFaction();
			if (!faction)
				continue;

			if (faction.GetFactionName() == KOTH_Faction.BLUFOR)
				countBluefor++;
			
			if (faction.GetFactionName() == KOTH_Faction.OPFOR)
				countRedfor++;
			
			if (faction.GetFactionName() == KOTH_Faction.INDFOR)
				countGreenfor++;
		}
		
		m_bluePlayers = countBluefor;
		m_redPlayers = countRedfor;
		m_greenPlayers = countGreenfor;
		
		Replication.BumpMe();
	}
	
	void BumpMe()
	{
		Replication.BumpMe();
	}
	
	bool PlayersCanChangeFaction()
	{
		if (m_redforPoints > 85 || m_greenforPoints > 85 || m_blueforPoints > 85)
			return false;
		
		return true;
	}
}
 