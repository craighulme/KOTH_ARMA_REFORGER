class KOTH_DeathRewardsHandlingComponentClass : SCR_BaseGameModeComponentClass {}
class KOTH_DeathRewardsHandlingComponent : SCR_BaseGameModeComponent
{
	protected KOTH_BackendApiGameModeComponent kothBackendApi;
	protected KOTH_SessionDataGameModeComponent sessionDataGameComp;
	protected KOTH_ExperienceManager expManager;
	protected SCR_BaseScoringSystemComponent scoringComp;
	protected RplComponent m_ReplicationComponent;
	protected PlayerManager m_playerManager;
	protected KOTH_AssistSystemComponent m_assistSystem;
	
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;

		expManager = KOTH_ExperienceManager.GetInstance();
		sessionDataGameComp = KOTH_SessionDataGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_SessionDataGameModeComponent));
		kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
		m_ReplicationComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_playerManager = GetGame().GetPlayerManager();
		scoringComp = SCR_BaseScoringSystemComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_BaseScoringSystemComponent));
		m_assistSystem = KOTH_AssistSystemComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_AssistSystemComponent));
	}

	// Method to reward killer XP and money on a non-vehicle target
	void RewardKiller(KOTH_DeathEvent deathEvent)
	{
		// Check if this instance is authoritative
        if (!m_ReplicationComponent.IsMaster())
        {
            return;
        }
		
		Log("RewardKiller - Rewarding Player for killing a player (not in a vehicle).  PlayerName = " +  m_playerManager.GetPlayerName(deathEvent.playerId) + ", KillerName = " +  m_playerManager.GetPlayerName(deathEvent.killerId));
		
		//handle the kill first
		int bonusKiller = expManager.GetKillBonus(deathEvent.killerUID);
		ApplyRewardXpKillandMoney(deathEvent, bonusKiller);
		deathEvent.killerProfileComp.DoRpc_Notif_EnemyKill(bonusKiller.ToString());
		deathEvent.killerProfileComp.AddToKillStreak();
		
		// reward any assistants who helped the killer
		array<string> assistants = m_assistSystem.GetAssistants(deathEvent.killerUID);
		foreach (string assistantUID : assistants)
		{
			int bonusAssister = expManager.GetKillBonus(assistantUID);
			m_assistSystem.RewardAssistant(assistantUID, bonusAssister, bonusAssister, 
				func ref void(KOTH_SCR_PlayerProfileComponent profileComp) {
					profileComp.DoRpc_Notif_EnemyKill(bonusKiller.ToString());
				}
			);
		}

		//vehicle weapon specific add kill 
		if(deathEvent.vehicleWeaponDeath)
		{
			scoringComp.AddKill(deathEvent.killerId, 1);
			LogWorkbench("Adding kill for Non-Vehicle Player killed by Vehicle Weapon.");
		}
		
		//Handle other bonuses
		KOTH_PlayerStatsJson stats = kothBackendApi.m_playerStatsList.Get(deathEvent.killerUID);
		if (stats)
			stats.SetNewMaxKillStreak(deathEvent.killerProfileComp.GetKillStreak());

		int killStreakBonus = FindKillStreakBonus(deathEvent);
		int killDistanceBonus = FindKillDistanceBonus(deathEvent);

		if (killDistanceBonus != 0) {
			ApplyKillDistanceReward(deathEvent, killDistanceBonus);

			foreach (string assistantUID : assistants)
			{
				m_assistSystem.RewardAssistant(assistantUID, killDistanceBonus, killDistanceBonus, 
					func ref void(KOTH_SCR_PlayerProfileComponent profileComp) {
						profileComp.DoRpc_Notif_KillDistance(killDistanceBonus, killDistanceBonus);
					}
				);
			}
		}
		if (killStreakBonus != 0) {
			ApplyKillStreakReward(deathEvent, killStreakBonus);

			foreach (string assistantUID : assistants)
			{
				m_assistSystem.RewardAssistant(assistantUID, killStreakBonus, killStreakBonus, 
					func ref void(KOTH_SCR_PlayerProfileComponent profileComp) {
						profileComp.DoRpc_Notif_KillStreak(deathEvent.killerProfileComp.GetKillStreak(), bonus);
					}
				);
			}
		}



		deathEvent.killerProfileComp.DoRpc_SyncPlayerProfile(deathEvent.killerProfileJson);
	}

	void ApplyRewardXpKillandMoney(KOTH_DeathEvent deathEvent, int bonus)
	{
		deathEvent.killerProfileJson.AddXp(bonus);
		deathEvent.killerProfileJson.AddMoney(bonus);
		deathEvent.killerProfileJson.AddKill();
		sessionDataGameComp.AddSessionXpAndMoney(bonus, bonus, deathEvent.killerUID);
	}

	void ApplyKillDistanceReward(KOTH_DeathEvent deathEvent, int bonus)
	{
		deathEvent.killerProfileJson.AddMoney(bonus);
		deathEvent.killerProfileJson.AddXp(bonus);
		deathEvent.killerProfileComp.DoRpc_Notif_KillDistance(bonus, bonus);

		float sessionBonus = bonus / 2;
		sessionDataGameComp.AddSessionXpAndMoney(sessionBonus.ToString(lenDec: 0).ToInt(), sessionBonus.ToString(lenDec: 0).ToInt(), deathEvent.killerUID);

		KOTH_PlayerStatsJson stats = kothBackendApi.m_playerStatsList.Get(deathEvent.killerUID);
		stats.SetNewMaxKillDistance(bonus);
	}

	void ApplyKillStreakReward(KOTH_DeathEvent deathEvent, int bonus)
	{
		deathEvent.killerProfileJson.AddMoney(bonus);
		deathEvent.killerProfileJson.AddXp(bonus);
		deathEvent.killerProfileComp.DoRpc_Notif_KillStreak(deathEvent.killerProfileComp.GetKillStreak(), bonus);
		sessionDataGameComp.UpdateKillStreak(deathEvent.killerUID, deathEvent.killerProfileComp.GetKillStreak());
	}

	// Returns the kill streak bonus for non-vehicle targets
	static int FindKillStreakBonus(KOTH_DeathEvent deathEvent)
	{
		int killStreakBonus = 0;
		switch (deathEvent.killerProfileComp.GetKillStreak())
		{
			case 3:
				killStreakBonus = 150;
				break;
			case 5:
				killStreakBonus = 250;
				Log("killStreak 5 for player " + deathEvent.killerUID);
				break;
			case 10:
				killStreakBonus = 750;
				Log("killStreak 10 for player " + deathEvent.killerUID);
				break;
			case 20:
				killStreakBonus = 1500;
				Log("killStreak 20 for player " + deathEvent.killerUID);
				break;
			case 30:
				killStreakBonus = 2000;
				Log("killStreak 30 for player " + deathEvent.killerUID);
				break;
		}
		return killStreakBonus;
	}


	// Returns the kill distance bonus for non-vehicle targets
	static int FindKillDistanceBonus(KOTH_DeathEvent deathEvent)
	{
		float currentTime = GetGame().GetWorld().GetWorldTime(); // Time in milliseconds

		//check if the last damage received from killer was larger than 10 seconds
		if (currentTime - deathEvent.playerProfileComp.lastDamageReceivedTime >= 10000)
		{
			LogWorkbench("The time the killer last damaged the player exceeded 10 seconds, no kill distance bonus for killer." + deathEvent.killerUID);
			return 0;
		}
		// Check if the killer respawned within the last 30 seconds
		if (currentTime - deathEvent.killerProfileComp.lastRespawnTime <= 30000)
		{
			LogWorkbench("Killer respawned within the last 30 seconds, no kill distance bonus for killer." + deathEvent.killerUID);
			return 0;
		}

		// Continue with normal kill distance bonus logic
		float killDistance = vector.Distance(deathEvent.playerEntity.GetOrigin(), deathEvent.killerEntity.GetOrigin());
		if (killDistance > 400)
		{
			if (killDistance >= 2000)
				Log("killDistance sus ("+killDistance+") for player " + deathEvent.killerUID);

			LogWorkbench("killDistance is ("+killDistance+") for player " + deathEvent.killerUID);
			return killDistance.ToString(lenDec: 0).ToInt();
		}

		return 0;
	}
}
