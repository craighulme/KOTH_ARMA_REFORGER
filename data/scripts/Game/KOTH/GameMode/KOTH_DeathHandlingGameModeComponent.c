class KOTH_DeathHandlingGameModeComponentClass : SCR_BaseGameModeComponentClass {}
class KOTH_DeathHandlingGameModeComponent : SCR_BaseGameModeComponent
{
	protected PlayerManager m_playerManager;
	protected KOTH_BackendApiGameModeComponent m_kothBackendApi;
	protected KOTH_PlayerEventsGameModeComponent m_playerEventsComp;
	protected KOTH_SessionDataGameModeComponent m_sessionDataGameComp;
	protected KOTH_VehicleEventsGameModeComponent m_vehEventGameComp;
	protected KOTH_DeathRewardsHandlingComponent m_rewardComp;
	protected RplComponent m_ReplicationComponent;
	
	//used for determining last pilot correctly and cache results
	private ref map<string, bool> m_pilotDeathCache; // Key: Vehicle ID, Value: Was pilot killed by another?
	private ref map<string, int> m_lastKillerIdForPilotCache; // Key: Vehicle ID, Value: Last killer ID for the pilot
	private ref map<int, int> m_generalKillerCache; // Key: Victim ID -> Resolved Killer ID
	
	//cache used for determining resolved killer in similar scenarios
	private ref map<string, ref map<int, int>> m_vehicleKillerCache; // Key: Vehicle ID -> (Original Killer ID -> Resolved Killer ID)

	private ref map<string, bool> m_vehiclePartDestroyedCache; //vehicle id string, bool if a part was destroyed
	
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;

		m_playerManager = GetGame().GetPlayerManager();
		m_playerEventsComp = KOTH_PlayerEventsGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_PlayerEventsGameModeComponent));
		m_sessionDataGameComp = KOTH_SessionDataGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_SessionDataGameModeComponent));
		m_kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
		m_rewardComp = KOTH_DeathRewardsHandlingComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_DeathRewardsHandlingComponent));
		m_vehEventGameComp = KOTH_VehicleEventsGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_VehicleEventsGameModeComponent));
		m_ReplicationComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		m_pilotDeathCache = new map<string, bool>();
		m_lastKillerIdForPilotCache = new map<string, int>();
		m_vehicleKillerCache = new map<string, ref map<int, int>>(); 
		m_vehiclePartDestroyedCache = new map<string, bool>();		
		m_generalKillerCache = new map<int, int>();
	}

	override void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		// Check if this instance is authoritative
        if (!m_ReplicationComponent.IsMaster())
        {
            return;
        }

		//we only want to process valid players
		if (!ChimeraCharacter.Cast(instigatorContextData.GetVictimEntity()))
			return;

		// Create a new instance of KOTH_DeathEvent for this death
    		KOTH_DeathEvent deathEvent = new KOTH_DeathEvent();

		deathEvent.playerId = instigatorContextData.GetVictimPlayerID();

		LogWorkbench("==============================OnControllableDestroyed============================");
		string killerName = m_playerManager.GetPlayerName(instigatorContextData.GetKillerPlayerID());
		Log("OnControllableDestroyed - Player Name :  " + m_playerManager.GetPlayerName(instigatorContextData.GetVictimPlayerID()) + ", Killer Name: " + killerName);

		SetupStartingPlayerVars(deathEvent, instigatorContextData);
		CleanupPlayer(deathEvent);

		deathEvent.killerId = DetermineRealKiller(deathEvent);

		//exit early because no identifiable killer
		if ((deathEvent.killerId == -1 && !deathEvent.killerEntity) || 
			(!deathEvent.killerId && !deathEvent.killerEntity))
		{
			Log("OnControllableDestroyed - deathEvent.killerId is invalid. Not Handling Kill");
			return;
		}
		
		SetupVarsAfterKillerResolution(deathEvent);
		
		if(deathEvent.killerId == deathEvent.playerId)
			deathEvent.isSuicide = true;

		// Cache killer ID for all deaths for use in notification sender
		m_generalKillerCache.Set(deathEvent.playerId, deathEvent.killerId);

		//since it should apply whether player dies in a vehicle or not, separate method
		HandleGunnerAssistXP(deathEvent);

		HandleVehicleDeath(deathEvent);
		HandleNonVehicleDeaths(deathEvent);

		
		LogWorkbench("OnControllableDestroyed: Enemy killed PlayerID: " + deathEvent.playerId + " by KillerID: " + deathEvent.killerId);
		deathEvent.Reset();
	}

	void SetupStartingPlayerVars(KOTH_DeathEvent deathEvent, notnull SCR_InstigatorContextData instigatorContextData)
	{
		// Setup starting vars and only print if not set
		deathEvent.originalKillerId = instigatorContextData.GetInstigator().GetInstigatorPlayerID();
		deathEvent.killerEntity = instigatorContextData.GetInstigator().GetInstigatorEntity();
		
		//check case where pilot weapon is used on vehicle (player id = 0 but killerEntity is vehicle)
		if (deathEvent.originalKillerId == 0 && deathEvent.killerEntity && deathEvent.killerEntity.IsInherited(Vehicle))
		{
		    Log("SetupStartingPlayerVars - originalKillerId is null or invalid but killerEntity is vehicle. Setting bool for use later");
		    Vehicle tempVehicle = Vehicle.Cast(deathEvent.killerEntity);
			WeaponSlotComponent weapSlot = WeaponSlotComponent.Cast(tempVehicle.FindComponent(WeaponSlotComponent));
			if(weapSlot)
			{
				deathEvent.killerVehicle = tempVehicle;
				deathEvent.vehicleWeaponDeath = true;
				deathEvent.killerId = deathEvent.killerVehicle.GetPilotID();
				deathEvent.killerEntity = m_playerManager.GetPlayerControlledEntity(deathEvent.killerId);
			}
		}
		
		else if (deathEvent.originalKillerId == 0)
		{
			//need this to fix when you crash as pilot. doesn't think pilot is killer sometimes
			Log("SetupStartingPlayerVars - originalKillerId is null or invalid but killerEntity is vehicle. Setting to PlayerId");
			deathEvent.originalKillerId = deathEvent.playerId;
		}
	
		deathEvent.playerEntity = instigatorContextData.GetVictimEntity();
		if (!deathEvent.killerEntity) 
		{
			//need this to fix when you crash as pilot. doesn't think pilot is killer sometimes
			Log("SetupStartingPlayerVars - killerEntity is null. Setting to playerEntity being handled");
			deathEvent.killerEntity = deathEvent.playerEntity;
		}
	
		deathEvent.playerFactionComp = FactionAffiliationComponent.Cast(deathEvent.playerEntity.FindComponent(FactionAffiliationComponent));
		deathEvent.playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(m_playerManager.GetPlayerController(deathEvent.playerId).FindComponent(KOTH_SCR_PlayerProfileComponent));
		deathEvent.playerUID = KOTH_Helper.GetPlayerUID(deathEvent.playerId);
		deathEvent.playerProfileJson = m_kothBackendApi.m_CurrentProfileList.Get(deathEvent.playerUID);
		deathEvent.playerVehicle = KOTH_Helper.GetVehicleFromEntity(deathEvent.playerEntity);
		deathEvent.playerIsInVehicle = deathEvent.playerVehicle != null;
		deathEvent.isSuicide = false;
	
		if (deathEvent.playerVehicle)
		{
			deathEvent.pilotKilledByAnother = DeterminePilotKilledByAnother(deathEvent);
			if (!deathEvent.pilotKilledByAnother) Log("SetupStartingPlayerVars - pilotKilledByAnother is null.");
		}
	}

	void SetupVarsAfterKillerResolution(KOTH_DeathEvent deathEvent)
	{
		//setup the updated killerEntity and only print if null vars
		
		//handle vehicle as the killer entity / weapon 
		if(deathEvent.vehicleWeaponDeath)
		{
			deathEvent.killerId = deathEvent.killerVehicle.GetPilotID();
			deathEvent.killerEntity = m_playerManager.GetPlayerController(deathEvent.killerId).GetControlledEntity();
		}
		//normal handling
		else
		{
			deathEvent.killerEntity = m_playerManager.GetPlayerController(deathEvent.killerId).GetControlledEntity();
			deathEvent.killerVehicle = KOTH_Helper.GetVehicleFromEntity(deathEvent.killerEntity);
		}
		
		if (!deathEvent.killerVehicle) Log("SetupVarsAfterKillerResolution - killerVehicle is null.");
	
		deathEvent.killerIsInVehicle = deathEvent.killerVehicle != null;
		deathEvent.killerFactionComp = FactionAffiliationComponent.Cast(deathEvent.killerEntity.FindComponent(FactionAffiliationComponent));
		deathEvent.killerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(m_playerManager.GetPlayerController(deathEvent.killerId).FindComponent(KOTH_SCR_PlayerProfileComponent));
		deathEvent.killerUID = KOTH_Helper.GetPlayerUID(deathEvent.killerId);
		deathEvent.killerProfileJson = m_kothBackendApi.m_CurrentProfileList.Get(deathEvent.killerUID);
		
	}

	void CleanupPlayer(KOTH_DeathEvent deathEvent)
	{
		deathEvent.playerProfileComp.ResetHasBeenDroppedNearZone();
		deathEvent.playerProfileComp.ResetKillStreak();
		deathEvent.playerProfileComp.ResetInsertionStreak();
		deathEvent.playerProfileComp.ResetFlushToilet();
		m_playerEventsComp.CleanUpPlayerEventHandlers(deathEvent.playerEntity);
		m_sessionDataGameComp.StopTimeMeasurement(deathEvent.playerUID);

		///death should be recorded regardless if vehicle or regular death
		deathEvent.playerProfileJson.AddDeath();
	}

	int DetermineRealKiller(KOTH_DeathEvent deathEvent)
	{
		string vehicleId;
		
		//Check for any vehicle relate damage to attribute death to
		if (deathEvent.playerIsInVehicle)
		{
			int killerId;
			vehicleId = deathEvent.playerVehicle.GetID().ToString();
			
			// Attempt to retrieve cached result
	        int cachedKillerId = GetCachedKillerId(vehicleId, deathEvent.originalKillerId);
	        if (cachedKillerId != -1)
	        {
	            Log("DetermineRealKiller: Cached killer ID reused for vehicle ID = " + vehicleId + " and original Killer Name = " + m_playerManager.GetPlayerName(deathEvent.originalKillerId));
	            return cachedKillerId;
	        }
			
			//check if pilot was shot out
			if (deathEvent.pilotKilledByAnother && deathEvent.lastKillerIdForPilot > 0)
			{
				Log("DetermineRealKiller: Pilot was killed by another. Resolved Killer Name = " + m_playerManager.GetPlayerName(deathEvent.lastKillerIdForPilot));
				
		        CacheKillerId(vehicleId, deathEvent.originalKillerId, deathEvent.lastKillerIdForPilot);
				return deathEvent.lastKillerIdForPilot;
			}

			killerId = m_vehEventGameComp.GetMaxDamageCauserIDForDestroyedHitZoneGroups(deathEvent.playerVehicle);

			//Return the killerId if its not -1, indicating we have vehicle damage that caused death
			if (killerId != -1)
			{
				Log("DetermineRealKiller: Killer from max damage determined. Resolved Killer Name = " + m_playerManager.GetPlayerName(killerId));
		        CacheKillerId(vehicleId, deathEvent.originalKillerId, killerId);
				
				return killerId;
			}
			
			//Handle deaths that were in vehicle but not directly shot.  and since no vehicle destruction of parts, should be pilots Error
			SCR_CharacterDamageManagerComponent charDmgMgr = SCR_CharacterDamageManagerComponent.Cast(deathEvent.playerEntity.FindComponent(SCR_CharacterDamageManagerComponent));
			
			if(charDmgMgr && charDmgMgr.lastKillerId != deathEvent.originalKillerId)
			{
				return deathEvent.playerVehicle.GetPilotID();
			}
			
			
		}
		
		EDamageType lastDamageType = SCR_CharacterDamageManagerComponent.Cast(deathEvent.playerEntity.FindComponent(SCR_CharacterDamageManagerComponent)).lastDamageType;
		
		// Handle false pilot-related deaths (high-altitude ejections or velocity too fast on exit by player)
		if (deathEvent.playerProfileComp.lastVehicle && //they were just in a vehicle
			!deathEvent.playerVehicle && //they exited vehicle
			lastDamageType == EDamageType.COLLISION) //collision damage
		{
			float altitude = m_playerEventsComp.GetAltitudeAboveGround(deathEvent.playerProfileComp.lastVehicle);

			if (altitude > m_playerEventsComp.MAX_GETOUT_ALTITUDE_AGL_METERS)
			{
				deathEvent.isSuicide = true;
				Log("DetermineRealKiller: Player "+ m_playerManager.GetPlayerName(deathEvent.playerId)+ " ejected too high. Counted as suicide.");
				m_playerEventsComp.ClearLastVehicleInProfile(deathEvent.playerId);
				return deathEvent.playerId;
			}

			vector velocity = deathEvent.playerEntity.GetPhysics().GetVelocity();

			if (velocity.LengthSq() > m_playerEventsComp.MAX_GETOUT_SPEED_METER_PER_SEC_SQ)
			{
				deathEvent.isSuicide = true;
				Log("DetermineRealKiller: Player "+ m_playerManager.GetPlayerName(deathEvent.playerId)+ " ejected too fast in velocity. Counted as suicide.");
				m_playerEventsComp.ClearLastVehicleInProfile(deathEvent.playerId);
				return deathEvent.playerId;
			}
		}
		

		//fallback to original killer id
		Log("DetermineRealKiller: Falling back to original killer ID = " + m_playerManager.GetPlayerName(deathEvent.originalKillerId));
		
		return deathEvent.originalKillerId;
	}

	void HandleGunnerAssistXP(KOTH_DeathEvent deathEvent)
	{
		//handles assist xp for pilot of gunner
		if (deathEvent.killerIsInVehicle && 
			deathEvent.killerVehicle.GetPilotID() != deathEvent.killerId &&
			!deathEvent.isFriendlyFire() && 
			!deathEvent.isSuicide)
		{
			m_vehEventGameComp.HandleKillsFromVehicle(deathEvent);
			return;
		}
	}

	void HandleVehicleDeath(KOTH_DeathEvent deathEvent)
	{
		//handle death determination within vehicle for player death
		if (deathEvent.playerIsInVehicle)
		{
			m_vehEventGameComp.HandlePilotCrash(deathEvent);
			return;
		}
	}

	void HandleNonVehicleDeaths(KOTH_DeathEvent deathEvent)
	{
		//should handle its own friendly fire and rewards separately

		if (deathEvent.playerIsInVehicle)
			return;

		if (deathEvent.isFriendlyFire())
		{
			LogWorkbench("OnControllableDestroyed: Friendly fire (Non-Vehicle Related) detected for PlayerID: " + deathEvent.playerId + " by KillerID: " + deathEvent.killerId);
			PunishFriendlyFire(deathEvent);
			return;
		}

		if (!deathEvent.isSuicide)
		{
			m_rewardComp.RewardKiller(deathEvent);
		}
	}

	void PunishFriendlyFire(KOTH_DeathEvent deathEvent)
	{
		// Handle regular friendly fire
		Log("Punishing for Friendly Fire - killer name : " +  m_playerManager.GetPlayerName(deathEvent.killerId) + " for player id: " +  m_playerManager.GetPlayerName(deathEvent.playerId));
		deathEvent.killerProfileJson.RemoveFriendlyKillXpAndMoney(false);
		deathEvent.killerProfileComp.DoRpc_SyncPlayerProfile(deathEvent.killerProfileJson);
		deathEvent.killerProfileComp.DoRpc_Notif_FriendlyKill(false);
	}

	//Should be true when pilot is killed but only when we are handling non-pilots death
	bool DeterminePilotKilledByAnother(KOTH_DeathEvent deathEvent)
	{
		if (deathEvent.playerVehicle == null)
		return false;

		string vehicleId = deathEvent.playerVehicle.GetID().ToString();
		int victimPilotID = deathEvent.playerVehicle.GetPilotID();
		
		//do not want to check if its the pilot himself
		if(deathEvent.playerId == victimPilotID || victimPilotID == -1)
			return false;

		// Check if pilot death is already cached
		if (m_pilotDeathCache.Contains(vehicleId))
		{
			LogWorkbench("DeterminePilotKilledByAnother Retrieving Cache: vehicleId: " + vehicleId);
			deathEvent.lastKillerIdForPilot = m_lastKillerIdForPilotCache.Get(vehicleId);

			return m_pilotDeathCache.Get(vehicleId);
		}

		IEntity pilotEntity = m_playerManager.GetPlayerControlledEntity(victimPilotID);
		if(!pilotEntity)
		{
			LogWorkbench("DeterminePilotKilledByAnother returning false because Pilot Entity is null");
			return false;
		}
			
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(
			pilotEntity.FindComponent(SCR_CharacterDamageManagerComponent)
		);

		if (damageManager != null)
		{
			deathEvent.lastKillerIdForPilot = damageManager.lastKillerId;
			bool wasKilled = damageManager.WasPilotKilledByAnother(victimPilotID);

			// Cache the result for subsequent checks
			LogWorkbench("DeterminePilotKilledByAnother Caching: vehicleId: " + vehicleId + " Bool wasKilled: " + wasKilled);
			m_pilotDeathCache.Set(vehicleId, wasKilled);
			m_lastKillerIdForPilotCache.Set(vehicleId, damageManager.lastKillerId);

			return wasKilled;
		}

		return false;
	}
	
	//usage for getting vehicle related killers
	int GetCachedKillerId(string vehicleId, int originalKillerId)
    {
        if (m_vehicleKillerCache.Contains(vehicleId))
        {
            map<int, int> killerMap = m_vehicleKillerCache.Get(vehicleId);
            if (killerMap.Contains(originalKillerId))
            {
                return killerMap.Get(originalKillerId);
            }
        }
        return -1; // Indicate no cached value
    }

	//usage for setting vehicle related killers
    void CacheKillerId(string vehicleId, int originalKillerId, int resolvedKillerId)
    {
        map<int, int> killerMap;
        if (!m_vehicleKillerCache.Contains(vehicleId))
        {
            killerMap = new map<int, int>();
            m_vehicleKillerCache.Set(vehicleId, killerMap);
        }
        else
        {
            killerMap = m_vehicleKillerCache.Get(vehicleId);
        }

        killerMap.Set(originalKillerId, resolvedKillerId);
        LogWorkbench("CacheKillerId: Cached killer ID = " + resolvedKillerId +
                      " for vehicle ID = " + vehicleId + " and original killer ID = " + originalKillerId);
    }
	
	//used for accessing cache needed in vehicle events
	bool WasVehiclePartDestroyed(string vehicleId)
	{
	    if (m_vehiclePartDestroyedCache.Contains(vehicleId))
	    {
	        return m_vehiclePartDestroyedCache.Get(vehicleId);
	    }
	
	    LogWorkbench("WasVehiclePartDestroyed: No entry found for vehicle ID = " + vehicleId + ". Assuming no parts were destroyed.");
	    return false; 
	}
	
	// Sets the vehicle part destroyed cache
	void SetVehiclePartDestroyed(string vehicleId, bool wasDestroyed)
	{
		if(!m_vehiclePartDestroyedCache.Contains(vehicleId))
		{
			m_vehiclePartDestroyedCache.Set(vehicleId, wasDestroyed);
	    		LogWorkbench("SetVehiclePartDestroyed: Cache updated for vehicle ID = " + vehicleId + ", WasDestroyed = " + wasDestroyed);
		}
	}
	
	// usage for notification sender for non-vehicle related deaths since instigator values may be wrong
	int GetCachedGeneralKillerId(int victimPlayerId)
	{
	    if (m_generalKillerCache.Contains(victimPlayerId))
	        return m_generalKillerCache.Get(victimPlayerId);
	
	    return -1; 
	}
	
	//cleanup method for general killer
	void RemovePlayerFromGeneralKillerCache(int playerId)
	{
	    if (m_generalKillerCache.Contains(playerId))
	    {
	        m_generalKillerCache.Remove(playerId);
	        LogWorkbench("RemovePlayerFromGeneralKillerCache: Removed player ID " + playerId + " from the general killer cache.");
	    }
	}

	//when vehicle is actually destroyed, clean up cache instead of using a timer
	override void OnControllableDeleted(IEntity entity)
	{
		LogWorkbench("====================================OnControllableDeleted=============================");
		//only want to cover vehicles in this block for cleanup
		Vehicle vehicle = Vehicle.Cast(entity);
		if (vehicle)
		{
			string vehicleId = vehicle.GetID().ToString();
			if (m_pilotDeathCache.Contains(vehicleId))
				m_pilotDeathCache.Remove(vehicleId);
			
			if (m_lastKillerIdForPilotCache.Contains(vehicleId))
				m_lastKillerIdForPilotCache.Remove(vehicleId);
			
			if (m_vehiclePartDestroyedCache.Contains(vehicleId))
				m_vehiclePartDestroyedCache.Remove(vehicleId);
			
			if (m_vehicleKillerCache.Contains(vehicleId))
            {
                m_vehicleKillerCache.Remove(vehicleId);
                LogWorkbench("OnControllableDeleted: Cache cleared for vehicle ID = " + vehicleId);
            }
			
			
			KOTH_VehicleDamageTracker damageTracker = vehicle.GetDamageTracker();
			
			if(damageTracker)
				damageTracker.ResetAllDamage();
		}
	}

}

