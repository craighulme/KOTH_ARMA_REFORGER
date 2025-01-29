class KOTH_VehicleEventsGameModeComponentClass : SCR_BaseGameModeComponentClass {}
class KOTH_VehicleEventsGameModeComponent : SCR_BaseGameModeComponent
{
    protected KOTH_BackendApiGameModeComponent m_kothBackendApi;
    protected KOTH_SessionDataGameModeComponent m_sessionDataGameComp;
    protected KOTH_ExperienceManager m_expManager;
    protected PlayerManager m_playerManager;
	protected KOTH_DeathRewardsHandlingComponent rewardComp;
	protected SCR_BaseScoringSystemComponent scoringComp;
	protected KOTH_DeathHandlingGameModeComponent deathHandling;
	
	const ref array<string> hitZoneGroups = {"ROTOR_ASSEMBLY", "ENGINE", "FUEL_TANKS", "TAIL_ROTOR", "WHEELS", "HULL"};

    override void OnPostInit(IEntity owner)
    {
        if (SCR_Global.IsEditMode(owner))
            return;

		BaseGameMode gameMode = GetGame().GetGameMode();
        m_expManager = KOTH_ExperienceManager.GetInstance();
        m_playerManager = GetGame().GetPlayerManager();
        m_sessionDataGameComp = KOTH_SessionDataGameModeComponent.Cast(gameMode.FindComponent(KOTH_SessionDataGameModeComponent));
        m_kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(gameMode.FindComponent(KOTH_BackendApiGameModeComponent));
		rewardComp = KOTH_DeathRewardsHandlingComponent.Cast(gameMode.FindComponent(KOTH_DeathRewardsHandlingComponent));
		scoringComp = SCR_BaseScoringSystemComponent.Cast(gameMode.FindComponent(SCR_BaseScoringSystemComponent));
		deathHandling = KOTH_DeathHandlingGameModeComponent.Cast(gameMode.FindComponent(KOTH_DeathHandlingGameModeComponent));
    }

    // Handle distributing XP and money for vehicle part destruction
    bool HandleVehicleGroupHitZoneDestroyed(VehicleHitZoneGroupDestructionEvent damageEvent)
    {
        if (!Replication.IsServer())
            return true;

        LogWorkbench("======================= Handle Vehicle Group Hit Zone Destroyed ================================");
        LogWorkbench("Vehicle Type for XP: " + damageEvent.VehicleTypeString + ", Vehicle Hit Zone Group: " + damageEvent.HitZoneGroup);

        if (damageEvent.Tracker.IsHitZoneGroupProcessed(damageEvent.HitZoneGroup))
        {
            //LogWorkbench("Hit zone already processed: " + damageEvent.HitZoneGroup);
            return true;
        }
		
        int maxDamagePlayerID = damageEvent.GetMaxDamagePlayerID();
        if (maxDamagePlayerID == -1)
        {
            LogWorkbench("No valid player ID found for max damage.");
            return true;
        }
		
		// Get the vehicle ID for cache updates
		string vehicleId = damageEvent.vehicle.GetID().ToString();
		deathHandling.SetVehiclePartDestroyed(vehicleId, true);

        PlayerController pilotController = KOTH_Helper.GetPlayerControllerFromEntity(damageEvent.Pilot);
        if (!pilotController)
        {
            LogWorkbench("Pilot controller not found for pilot: " + damageEvent.Pilot);
            return true;
        }

        string pilotUID = KOTH_Helper.GetPlayerUID(pilotController.GetPlayerId());

        foreach (auto damageInfo : damageEvent.HitZoneGroupDamages)
        {
            if (damageInfo.PlayerID == -1) continue;

            PlayerController killerController = m_playerManager.GetPlayerController(damageInfo.PlayerID);
            if (!killerController)
            {
                LogWorkbench("Killer controller not found for PlayerID: " + damageInfo.PlayerID);
                continue;
            }

            string killerUID = KOTH_Helper.GetPlayerUID(damageInfo.PlayerID);
            auto killerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(killerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
            auto killerProfile = m_kothBackendApi.m_CurrentProfileList.Get(killerUID);

            int bonus;
            bool isKill = false;
			
            if (damageInfo.PlayerID == maxDamagePlayerID)
			{
				isKill = true;
				bonus = m_expManager.GetVehicleHitZoneGroupKillBonus(killerUID, damageEvent.VehicleTypeString, damageEvent.HitZoneGroup);
				if (IsVehicleFriendlyFire(damageEvent.VehicleFaction, damageInfo.PlayerID, pilotUID, killerUID, killerProfile, killerProfileComp, damageEvent.VehicleTypeString, damageEvent.HitZoneGroup))
	            {
	                LogWorkbench("Friendly fire detected for vehicle hit zone group destruction: PlayerID: " + damageInfo.PlayerID + ", PilotUID: " + pilotUID + ", KillerUID: " + killerUID);
	                return true;
	            }
			}
			else
			{
				bonus = m_expManager.GetVehicleHitZoneGroupAssistBonus(killerUID, damageEvent.VehicleTypeString, damageEvent.HitZoneGroup);
			}

            AssignVehicleRewards(killerProfile, killerProfileComp, bonus, isKill, damageEvent.VehicleTypeString, damageEvent.HitZoneGroup);
            Log("Assigned rewards: PlayerName: " +  m_playerManager.GetPlayerName(damageInfo.PlayerID) + ", Bonus: " + bonus + ", HitZoneGroup: " + damageEvent.HitZoneGroup + ", VehicleType: " + damageEvent.VehicleTypeString);
        }

        return true;
    }

    bool IsVehicleFriendlyFire(Faction vehicleFaction, int playerID, string pilotUID, string killerUID, KOTH_PlayerProfileJson killerProfile, KOTH_SCR_PlayerProfileComponent killerProfileComp, 
        string vehicleType, string hitZoneGroup)
    {
        if (vehicleFaction == KOTH_Helper.GrabFactionByPlayerID(playerID)) 
        {
            if (killerUID == pilotUID)
                return true;

            killerProfile.RemoveVehicleFriendlyKillXpAndMoney();
            killerProfileComp.DoRpc_SyncPlayerProfile(killerProfile);
            killerProfileComp.DoRpc_Notif_FriendlyVehicleDestroy(KOTH_Helper.ToUpper(vehicleType), KOTH_Helper.ToUpper(hitZoneGroup));

            return true;
        }
        return false;
    }

    void AssignVehicleRewards(KOTH_PlayerProfileJson profile, KOTH_SCR_PlayerProfileComponent profileComp, int bonus, bool isKill, string vehicleType, string hitZoneGroup)
    {
        profile.AddXp(bonus);
        profile.AddMoney(bonus);
        profileComp.DoRpc_SyncPlayerProfile(profile);

        if (isKill)
            profileComp.DoRpc_Notif_EnemyVehicleDestroy(bonus.ToString(), KOTH_Helper.ToUpper(vehicleType), KOTH_Helper.ToUpper(hitZoneGroup));
        else
            profileComp.DoRpc_Notif_EnemyVehicleAssist(bonus.ToString(), KOTH_Helper.ToUpper(vehicleType), KOTH_Helper.ToUpper(hitZoneGroup));
    }

    int GetMaxDamageCauserIDForDestroyedHitZoneGroups(Vehicle vehicle)
    {
        KOTH_VehicleDamageTracker damageTracker = vehicle.GetDamageTracker();
        if (!damageTracker) 
        {
            LogWorkbench("GetMaxDamageCauserIDForDestroyedHitZoneGroups: damageTracker is null for vehicle, returning -1 for id");
            return -1;
        }

        int maxDamageCauserID = -1;
        float maxDamage = 0;

        foreach (string hitZoneGroup : hitZoneGroups)
        {
            if (!damageTracker.IsHitZoneGroupProcessed(hitZoneGroup))
            {
                continue;
            }

            foreach (int playerID, DamageInfo damageInfo : damageTracker.GetAllPlayerDamageOnHitZoneGroup(hitZoneGroup))
            {
                if (damageInfo && damageInfo.Damage > maxDamage)
                {
                    maxDamage = damageInfo.Damage;
                    maxDamageCauserID = playerID;

                    LogWorkbench("New max damage causer found: PlayerID = " + playerID + ", Damage = " + damageInfo.Damage);
                }
            }
        }

        LogWorkbench("maxDamageCauserID: " + maxDamageCauserID);
        return maxDamageCauserID;
    }

    bool IsHitZoneGroupDestroyed(IEntity vehicle, string hitZoneGroup, string killerUID)
    {
        Vehicle victimVehicle = Vehicle.Cast(vehicle);
        if (!victimVehicle) return false;

        KOTH_VehicleDamageTracker damageTracker = victimVehicle.GetDamageTracker();
        return IsHitZoneGroupProcessed(damageTracker, hitZoneGroup) && CheckDamageCausedByKiller(damageTracker, hitZoneGroup, killerUID);
    }

    bool IsHitZoneGroupProcessed(KOTH_VehicleDamageTracker damageTracker, string hitZoneGroup)
    {
        if (!damageTracker)
        {
            LogWorkbench("IsHitZoneGroupProcessed: damageTracker is null for hitZoneGroup: " + hitZoneGroup);
            return false;
        }

        return damageTracker.IsHitZoneGroupProcessed(hitZoneGroup);
    }

    bool CheckDamageCausedByKiller(KOTH_VehicleDamageTracker damageTracker, string hitZoneGroup, string killerUID)
    {
        ref map<int, ref DamageInfo> damageInfoMap = damageTracker.GetAllPlayerDamageOnHitZoneGroup(hitZoneGroup);
        foreach (int playerID, DamageInfo damageInfo : damageInfoMap)
        {
            if (damageInfo && KOTH_Helper.GetPlayerUID(damageInfo.PlayerID) == killerUID)
            {
                LogWorkbench("CheckDamageCausedByKiller: " + hitZoneGroup + " damage caused by killer " + killerUID);
                return true;
            }
        }
        return false;
    }

    bool IsInVehicle(IEntity victimEntity)
    {
		if(victimEntity)
        		return Vehicle.Cast(victimEntity.GetRootParent()) != null;
		
		return false;
    }

    void HandlePilotCrash(KOTH_DeathEvent deathEvent)
	{
	    int pilotID = deathEvent.playerVehicle.GetPilotID();
	    
		// Separate logic to determine if it's a person killed in a turret
	    if (deathEvent.playerIsInVehicle && IsEntityInTurret(deathEvent.playerEntity, deathEvent.playerVehicle))
	    {
	        LogWorkbench("HandlePilotCrash: Player killed in a turret. Handling turret-specific logic.");
	        HandleTurretKill(deathEvent, deathEvent.playerEntity);
	        return;
	    }

		
	    // Ensure valid IDs for both the pilot and the killer
	    if (pilotID == -1 || deathEvent.killerId == -1 || pilotID == 0 || !deathEvent.playerIsInVehicle)
	        return;
		
		IEntity pilotEntity = m_playerManager.GetPlayerControlledEntity(pilotID);
		if(!pilotEntity)
		{
			LogWorkbench("pilot entity for handlecrash was invalid. maybe destroyed already");
			return;
		}
	    
		bool vehPartDestroyed = deathHandling.WasVehiclePartDestroyed(deathEvent.playerVehicle.GetID().ToString());

	    // Check if the pilot was killed by someone else before the crash. Non-Pilot Occupant
	    if (deathEvent.pilotKilledByAnother && !vehPartDestroyed)
	    {
	        Log("HandlePilotCrash: Awarding points to the player who caused occupant death because of pilot.");
	        AwardKillPoints(deathEvent.killerId);
			
			//pilot was actually physically killed so no need to add a kill for him
			if(pilotID != deathEvent.playerId)
			{
				//Add standard profile kill for scoreboard since it would originally be a friendly death by pilot
				scoringComp.AddKill(deathEvent.killerId, 1);
				scoringComp.AddDeath(deathEvent.playerId, 1);
			}
			
	        return;
	    }
		
		// if the pilot wasn't killed before crash but a killer somehow caused the crash by destroying a part. Handle pilot + occupants
		if (!deathEvent.isSuicide && 
			vehPartDestroyed)
		{
			Log("HandlePilotCrash: Awarding points to the player who destroyed a vehicle and caused the death of the occupants.");
	        AwardKillPoints(deathEvent.killerId);

			//Add standard profile kill for scoreboard since it would originally be a friendly death by pilot
			scoringComp.AddKill(deathEvent.killerId, 1);
			
			//need to add death for occupants
			if(pilotID != deathEvent.playerId && !deathEvent.vehicleWeaponDeath)
			{
				scoringComp.AddDeath(deathEvent.playerId, 1);
			}
			
			return;	
		}
		
		// player was directly killed in the chopper by killer, no parts destroyed, and killer is not pilot
		if(deathEvent.killerId != pilotID &&
			!vehPartDestroyed &&
			!deathEvent.isSuicide)
		{
			Log("HandlePilotCrash: Awarding points to the player who killed someone in a vehicle without destroying a part.");
	        AwardKillPoints(deathEvent.killerId);
			
			return;	
		}
		
	    // If the pilot was not killed by another player, and the crash caused the death of an occupant
	    if (deathEvent.playerId != pilotID && 
			!vehPartDestroyed
		)
	    {
	        Log("HandlePilotCrash: Crash due to pilot error, passenger affected. Penalizing pilot.");
			PenalizePilotForCrash(pilotEntity, deathEvent.playerVehicle);
			return;
	    }

		//fallback do nothing - probably the pilot suicide
		LogWorkbench("HandlePilotCrash: Fallback, most likely pilot suicide. No action taken");
	}
	
	void HandleTurretKill(KOTH_DeathEvent deathEvent, IEntity turretOccupant)
	{
	    LogWorkbench("HandleTurretKill: Processing turret occupant death.");
	
	    // If the player in the turret was killed by an external killer
	    if (!deathEvent.isFriendlyFire() && !deathEvent.isSuicide)
	    {
	        Log("HandleTurretKill: Awarding kill points to external killer.");
	        AwardKillPoints(deathEvent.killerId);
	
	        scoringComp.AddKill(deathEvent.killerId, 1);
	        return;
	    }

	    // Fallback
	    Log("HandleTurretKill: Fallback, no points awarded for turret occupant death.");
	}
	
	bool IsEntityInTurret(IEntity entity, Vehicle vehicle)
	{
	    if (!entity || !vehicle)
	        return false;
	
	    SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
	    if (!compartmentManager)
	    {
	        LogWorkbench("IsEntityInTurret: No valid compartment manager found.");
	        return false;
	    }
	
	    array<IEntity> turretOccupants = {};
	    compartmentManager.GetOccupantsOfType(turretOccupants, ECompartmentType.TURRET);
	
	    foreach (IEntity occupant : turretOccupants)
	    {
	        if (occupant == entity)
	            return true;
	    }
	
	    return false;
	}
	
	//only used for assist related xp
	void HandleKillsFromVehicle(KOTH_DeathEvent deathEvent)
	{
	    if (!deathEvent.killerIsInVehicle || !deathEvent.killerVehicle)
	        return;
	
	    int pilotID = deathEvent.killerVehicle.GetPilotID();
	    
	    // Ensure valid IDs for both the pilot and the killer
	    if (pilotID == -1 || deathEvent.killerId == -1 || pilotID == 0)
	        return;
	
		IEntity pilotEntity = m_playerManager.GetPlayerControlledEntity(pilotID);
		if(!pilotEntity)
		{
			LogWorkbench("pilot entity for HandleKillsFromVehicle was invalid. maybe dead already");
			return;
		}
		
		AwardKillAssistPoints(pilotID);
	}
	
    void AwardKillPoints(int killerID)
    {
        PlayerController killerController = m_playerManager.GetPlayerController(killerID);
        if (!killerController)
            return;

        string killerUID = KOTH_Helper.GetPlayerUID(killerID);
        auto killerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(killerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
        auto killerProfile = m_kothBackendApi.m_CurrentProfileList.Get(killerUID);

        int bonus = m_expManager.GetKillBonus(killerUID);

        ApplyVehicleRewardXpKillandMoney(killerProfile, killerUID, bonus);
        killerProfileComp.DoRpc_Notif_EnemyKill(bonus.ToString());
        killerProfileComp.AddToKillStreak();
        killerProfileComp.DoRpc_SyncPlayerProfile(killerProfile);
		
        Log("AwardKillPoints: Awarded points for death in vehicle. PlayerName: " +  m_playerManager.GetPlayerName(killerID) + ", Bonus: " + bonus);
    }
	
	void AwardKillAssistPoints(int assisterID)
    {
        PlayerController assisterController = m_playerManager.GetPlayerController(assisterID);
        if (!assisterController)
            return;

        string assisterUID = KOTH_Helper.GetPlayerUID(assisterID);
        auto assisterProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(assisterController.FindComponent(KOTH_SCR_PlayerProfileComponent));
        auto assisterProfile = m_kothBackendApi.m_CurrentProfileList.Get(assisterUID);

        int bonus = m_expManager.GetKillAssistBonus(assisterUID);

        ApplyVehicleRewardXpKillandMoney(assisterProfile, assisterUID, bonus);	
        assisterProfileComp.DoRpc_Notif_EnemyKillAssist(bonus.ToString());	
		// TODO: Might be nice to implement a AddToAssistStreak at some point
        // assisterProfileComp.AddToKillStreak();
		
        assisterProfileComp.DoRpc_SyncPlayerProfile(assisterProfile);

        Log("AwardKillAssistPoints: Awarded points for kill assist from vehicle. PlayerName: " +  m_playerManager.GetPlayerName(assisterID) + ", Bonus: " + bonus);
    }

    void ApplyVehicleRewardXpKillandMoney(KOTH_PlayerProfileJson profile, string uid, int bonus)
    {
        profile.AddXp(bonus);
        profile.AddMoney(bonus);
        profile.AddKill();
        m_sessionDataGameComp.AddSessionXpAndMoney(bonus, bonus, uid);
    }

    void ApplyVehicleKillStreakReward(KOTH_PlayerProfileJson profile, KOTH_SCR_PlayerProfileComponent profileComp, string uid, int bonus)
    {
        profile.AddMoney(bonus);
        profile.AddXp(bonus);
        profileComp.DoRpc_Notif_KillStreak(profileComp.GetKillStreak(), bonus);
        m_sessionDataGameComp.UpdateKillStreak(uid, profileComp.GetKillStreak());
    }

    void PenalizePilotForCrash(IEntity playerEntity, Vehicle victimVehicle)
    {
        int pilotId = victimVehicle.GetPilotID();
        if (pilotId == -1)
            return;

        PlayerController pilotController = m_playerManager.GetPlayerController(pilotId);
        if (!pilotController)
            return;

        auto pilotProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(pilotController.FindComponent(KOTH_SCR_PlayerProfileComponent));
        if (!pilotProfileComp)
            return;

        string pilotUID = KOTH_Helper.GetPlayerUID(pilotId);
        auto pilotProfile = m_kothBackendApi.m_CurrentProfileList.Get(pilotUID);
        if (!pilotProfile)
            return;

		Log("PenalizePilotForCrash: Removing points for friendly occupant death to PlayerName: " +  m_playerManager.GetPlayerName(pilotId));
        pilotProfileComp.DoRpc_Notif_FriendlyKill(true);
        pilotProfile.RemoveFriendlyKillXpAndMoney(true);
        pilotProfileComp.DoRpc_SyncPlayerProfile(pilotProfile);
    }
}