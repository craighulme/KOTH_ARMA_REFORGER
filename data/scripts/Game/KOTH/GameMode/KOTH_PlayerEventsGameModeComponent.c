class KOTH_PlayerEventsGameModeComponentClass : SCR_BaseGameModeComponentClass {}
class KOTH_PlayerEventsGameModeComponent : SCR_BaseGameModeComponent
{
	protected KOTH_BackendApiGameModeComponent m_kothBackendApi;
	protected KOTH_ScoringGameModeComponent m_scoreComp;
	protected KOTH_SessionDataGameModeComponent m_sessionDataGameComp;
	protected KOTH_ExperienceManager m_expManager;
	protected PlayerManager m_playerManager;
	protected KOTH_VehicleEventsGameModeComponent m_vehEventGameComp;
	protected KOTH_AssistSystemComponent m_assistSystem;
	
	//grabbed this from SCR_GetOutAction
	const float MAX_GETOUT_SPEED_METER_PER_SEC_SQ = 17.36138889;
	const float MAX_GETOUT_ALTITUDE_AGL_METERS = 8;

	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		m_expManager = KOTH_ExperienceManager.GetInstance();
		m_playerManager = GetGame().GetPlayerManager();
		m_scoreComp = KOTH_ScoringGameModeComponent.Cast(gameMode.FindComponent(KOTH_ScoringGameModeComponent));
		m_sessionDataGameComp = KOTH_SessionDataGameModeComponent.Cast(gameMode.FindComponent(KOTH_SessionDataGameModeComponent));
		m_kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(gameMode.FindComponent(KOTH_BackendApiGameModeComponent));
		m_vehEventGameComp = KOTH_VehicleEventsGameModeComponent.Cast(gameMode.FindComponent(KOTH_VehicleEventsGameModeComponent));
		m_assistSystem = KOTH_AssistSystemComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_AssistSystemComponent));
	}

	override void OnPlayerConnected(int playerId)
	{
		if (!Replication.IsServer())
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!playerController)
			return;

		SCR_PlayerFactionAffiliationComponent playerFactionAff = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (playerFactionAff)
			playerFactionAff.GetOnPlayerFactionResponseInvoker_S().Insert(OnPlayerFactionResponse_S);
	}

	protected void OnPlayerFactionResponse_S(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		if (!response || !Replication.IsServer() || factionIndex == -1)
			return;

		PlayerController playerController = component.GetPlayerController();
		if (!playerController)
			return;

		int playerId = playerController.GetPlayerId();
		string profileUID = KOTH_Helper.GetPlayerUID(playerId);
		
		//if they join a faction by this time, they should have a profile uid
		if(!profileUID)
		{
			m_kothBackendApi.AddPlayerToAudit(playerId);
		}
		
		KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
		FactionManager factionManager = GetGame().GetFactionManager();
		Faction faction = factionManager.GetFactionByIndex(factionIndex);
		if (!faction)
			return;

		KOTH_SessionPlayerData playerData = m_sessionDataGameComp.m_sessionPlayersData.Get(profileUID);
		if (!playerData)
		{
			if (!GetGame().GetPlayerManager().IsPlayerConnected(playerId))
				return;

			GetGame().GetCallqueue().CallLater(OnPlayerFactionResponse_S, 500, false, component, factionIndex, response);
			return;
		}

		switch (faction.GetFactionKey())
		{
			case KOTH_Faction.BLUFOR:
				if (playerData.GetSessionFaction() != KOTH_Faction.BLUFOR)
					playerData.SetSessionPointsWhenFactionWasJoined(m_scoreComp.GetBlueforPoint(), KOTH_Faction.BLUFOR);
			break;
			case KOTH_Faction.OPFOR:
				if (playerData.GetSessionFaction() != KOTH_Faction.OPFOR)
					playerData.SetSessionPointsWhenFactionWasJoined(m_scoreComp.GetRedforPoint(), KOTH_Faction.OPFOR);
			break;
			case KOTH_Faction.INDFOR:
				if (playerData.GetSessionFaction() != KOTH_Faction.INDFOR)
					playerData.SetSessionPointsWhenFactionWasJoined(m_scoreComp.GetGreenforPoint(), KOTH_Faction.INDFOR);
			break;
		}
	}

	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		super.OnPlayerSpawnFinalize_S(requestComponent, handlerComponent, data, entity);

		if (!Replication.IsServer())
			return;

		SCR_ChimeraCharacter chimeraPlayer = SCR_ChimeraCharacter.Cast(entity);
		SCR_CompartmentAccessComponent baseCompartmentAccessComp = SCR_CompartmentAccessComponent.Cast(chimeraPlayer.FindComponent(SCR_CompartmentAccessComponent));

		baseCompartmentAccessComp.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
		baseCompartmentAccessComp.GetOnCompartmentEntered().Insert(OnCompartmentEntered);
		baseCompartmentAccessComp.GetOnPlayerCompartmentEnter().Insert(OnPlayerEnterCompartment);
		baseCompartmentAccessComp.GetOnPlayerCompartmentExit().Insert(OnPlayerExitCompartment);

		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent));
		if (damageManager)
		{
			// Reset the alive state for the damage manager component
			damageManager.EnableDamageHandling(false);
		}

		PlayerController playerController = requestComponent.GetPlayerController();
		KOTH_SCR_PlayerShopComponent playerShopComp = KOTH_SCR_PlayerShopComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerShopComponent));
		string playerUID = KOTH_Helper.GetPlayerUID(requestComponent.GetPlayerId());

		m_assistSystem.ClearAllAssists(playerUID);

		KOTH_SCR_PlayerProfileComponent playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
		playerProfileComp.lastRespawnTime = GetGame().GetWorld().GetWorldTime();

		m_sessionDataGameComp.StartTimeMeasurement(playerUID);

		KOTH_SessionPlayerLoadout loadoutData = m_sessionDataGameComp.m_sessionPlayersLoadout.Get(playerUID);
		if (loadoutData && false == loadoutData.IsEmpty())
			playerShopComp.DoRpc_ShowSessionLoadout(loadoutData);

		playerShopComp.RpcDo_LockInventory();

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(entity.FindComponent(EventHandlerManagerComponent));
		if (!eventHandlerManager)
			return;

		eventHandlerManager.RegisterScriptHandler("OnProjectileShot", this, OnProjectileShot);
		eventHandlerManager.RegisterScriptHandler("OnGrenadeThrown", this, OnGrenadeThrown);
	}

	void OnProjectileShot(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		KOTH_PlayerStatsJson stats = m_kothBackendApi.m_playerStatsList.Get(KOTH_Helper.GetPlayerUID(playerID));

		if (!stats)
		{
			Log("did not found stats for "+m_playerManager.GetPlayerName(playerID));
			return;
		}

		stats.m_bulletsShot++;
	}

	void OnGrenadeThrown(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		KOTH_PlayerStatsJson stats = m_kothBackendApi.m_playerStatsList.Get(KOTH_Helper.GetPlayerUID(playerID));

		if (!stats)
		{
			Log("did not found stats for "+m_playerManager.GetPlayerName(playerID));
			return;
		}

		stats.m_grenadesThrown++;
	}

	//Pilot logic only
	void OnCompartmentEntered(IEntity targetEntity, SCR_BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		LogWorkbench("Calling OnCompartmentEntered");

		//check if vehicle or compart is valid
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);

		Vehicle vehicle = KOTH_Helper.GetVehicleFromEntity(targetEntity);

		if (!compartment || !vehicle)
			return;

		// Check if the compartment is a pilot compartment slot
		PilotCompartmentSlot pilotCompartment = PilotCompartmentSlot.Cast(compartment);
		if (!pilotCompartment)
			return;

		// Get the ID of the pilot occupant entering the compartment
		int pilotOccupantID = m_playerManager.GetPlayerIdFromControlledEntity(pilotCompartment.GetOccupant());

		SetPilotID(vehicle, slotID, mgrID, pilotOccupantID, manager);
	}

	void SetPilotID(Vehicle vehicle, int slotID, int mgrID, int pilotOccupantID, SCR_BaseCompartmentManagerComponent manager)
	{
		// Get the primary pilot compartment slot
		PilotCompartmentSlot mainPilotSlot = vehicle.GetPilotCompartmentSlot();

		// Get all current pilots
		array<IEntity> currentPilots = GetOccupants(manager, ECompartmentType.PILOT);

		if (currentPilots.Count() == 0)
			return;

		// Check if the occupant is entering the main pilot seat or if the co-pilot seat is being used
		if (mainPilotSlot.GetCompartmentSlotID() == slotID && mainPilotSlot.GetCompartmentMgrID() == mgrID)
		{
			vehicle.SetPilotID(pilotOccupantID);

			LogWorkbench("Setting Pilot ID after entering main pilot seat: " + pilotOccupantID);
		}
		else if (mainPilotSlot.GetOccupant() == null)
		{
			int coPilotID = m_playerManager.GetPlayerIdFromControlledEntity(currentPilots.Get(0));
			vehicle.SetPilotID(coPilotID);

			LogWorkbench("Setting Pilot ID after main pilot seat is empty and co-pilot is used: " + coPilotID);
		}
	}

	//does not work when leaving compartment based on state for pilots for some reason
	array<IEntity> GetOccupants(SCR_BaseCompartmentManagerComponent manager, ECompartmentType occupantType)
	{
		array<IEntity> currentOccupants = {};
		manager.GetOccupantsOfType(currentOccupants, occupantType);

		LogWorkbench("Current occupants of type " + occupantType + ": " + currentOccupants.Count());
		return currentOccupants;
	}


	//all occupant logic including pilot
	void OnPlayerEnterCompartment(ChimeraCharacter playerCharacter, IEntity compartmentEntity)
	{
		LogWorkbench("Calling OnPlayerEnterCompartment");

		Vehicle vehicle = KOTH_Helper.GetVehicleFromEntity(compartmentEntity);

		if (!vehicle)
			return;

		// Get the ID of the occupant entering the compartment
		int occupantID = m_playerManager.GetPlayerIdFromControlledEntity(playerCharacter);

		//set the last vehicle entered in profileComp for use in GetKillerId
		PlayerController playerController = m_playerManager.GetPlayerController(occupantID);
		KOTH_SCR_PlayerProfileComponent playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
		playerProfileComp.lastVehicle = vehicle;

		LogWorkbench("Setting LastVehicle for occupant " + m_playerManager.GetPlayerName(occupantID) + " as: " + playerProfileComp.lastVehicle.ToString());

	}

	//pilot logic only
	void OnCompartmentLeft(IEntity targetEntity, SCR_BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (move) // moving only between compartments
			return;

		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		if (!compartment)
			return;

		//pass the vehicle into handlePilotIDAfterCompartmentLeft method since pilot entity may be dead later after 10 sec timer
		Vehicle vehicle = KOTH_Helper.GetVehicleFromEntity(targetEntity);

		// Update pilot ID after compartment left
		GetGame().GetCallqueue().CallLater(HandlePilotIDAfterCompartmentLeft, 10000, false, targetEntity, slotID, mgrID, vehicle);
		
		// handle vehicle lock action
		ActionsManagerComponent actionMngrComp = ActionsManagerComponent.Cast(vehicle.FindComponent(ActionsManagerComponent));
		array<BaseUserAction> outActions = {};
		actionMngrComp.GetActionsList(outActions);

		foreach (BaseUserAction action : outActions)
		{
			KOTH_VehicleLock kothLockAction = KOTH_VehicleLock.Cast(action);
			if (kothLockAction)
			{
				kothLockAction.OnCompartmentLeft();
				break;
			}
		}
	}

	void HandlePilotIDAfterCompartmentLeft(IEntity targetEntity, int slotID, int mgrID, Vehicle vehicle)
	{
		if (!vehicle)
			return;

		// Get the primary pilot compartment slot
		PilotCompartmentSlot mainPilotSlot = vehicle.GetPilotCompartmentSlot();
		if (!mainPilotSlot)
			return;

		// Check if the leaving pilot is the main pilot
		if (mainPilotSlot.GetCompartmentSlotID() == slotID && mainPilotSlot.GetCompartmentMgrID() == mgrID)
		{
			// No pilots left in main compartment
			LogWorkbench("No Pilots Left - Setting Pilot ID to -1");
			vehicle.SetPilotID(-1);
		}
	}

	//all occupant logic including pilot
	void OnPlayerExitCompartment(ChimeraCharacter playerCharacter, IEntity compartmentEntity)
	{
		// Get the ID of the occupant leaving the compartment
		int occupantID = m_playerManager.GetPlayerIdFromControlledEntity(playerCharacter);
		Vehicle vehicle = KOTH_Helper.GetVehicleFromEntity(compartmentEntity);

		if (!vehicle)
		{
			Log("OnPlayerExitCompartment: No Vehicle Detected for player " + m_playerManager.GetPlayerName(occupantID) + " - Player exit/reward ignored");
			return;
		}
		
		// Check if they are still in a compartment
	    if (CompartmentAccessComponent.Cast(playerCharacter.FindComponent(CompartmentAccessComponent)).IsInCompartment())
	    {
	        LogWorkbench("OnPlayerExitCompartment: Player " + m_playerManager.GetPlayerName(occupantID) + " is still in a compartment, event ignored.");
	        return;
	    }

		int pilotId = vehicle.GetPilotID();

		//don't want to award pilot drop off reward
		if (pilotId == occupantID)
		{
			ClearLastVehicleInProfile(occupantID);
			return;
		}
		// Check altitude above ground and near KOTH zone
		if (pilotId != -1 && IsNearKothZone(compartmentEntity))
		{
			//add check to make sure occupant is alive when they exit to get rewarded
			IEntity occupant = m_playerManager.GetPlayerControlledEntity(occupantID);
			CharacterControllerComponent occCharControllerComp = CharacterControllerComponent.Cast(occupant.FindComponent(CharacterControllerComponent));

			if (occCharControllerComp.IsDead() || occCharControllerComp.IsUnconscious())
			{
				Log("OnPlayerExitCompartment: " + m_playerManager.GetPlayerName(occupantID) + " is dead or unconscious. no points awarded");
				return;
			}
			
			float altitudeAboveGround = GetAltitudeAboveGround(vehicle, 100);

			// Only award bonus if altitude is MAX_GETOUT_ALTITUDE_AGL_METERS meters or less
			if (altitudeAboveGround >= 0 && altitudeAboveGround <= MAX_GETOUT_ALTITUDE_AGL_METERS)
			{
				Log("OnPlayerExitCompartment: " + m_playerManager.GetPlayerName(occupantID) + " IsNearKothZone and Altitude is below threshold at: " + altitudeAboveGround);
				
				//clear immediately for players who successfully landed
				ClearLastVehicleInProfile(occupantID);
				HandleOccupantDropNearZone(occupantID, pilotId);
			}
			else
			{
				Log("OnPlayerExitCompartment: Drop-off bonus not awarded to " + m_playerManager.GetPlayerName(pilotId) + " due to altitude above ground being too high: " + altitudeAboveGround);
			}
		}
		
		//Fallback to clear the last vehicle regardless
		GetGame().GetCallqueue().CallLater(ClearLastVehicleInProfile, 8000, false, occupantID);
	}
	
	void ClearLastVehicleInProfile(int occupantID)
	{
		//grab koth scr profile and clear last vehicle so no false vehicle heights after they exit heli safely
		LogWorkbench("Clearing the last vehicle in profile for: " + m_playerManager.GetPlayerName(occupantID));
		PlayerController playerController = m_playerManager.GetPlayerController(occupantID);
		if(playerController)
		{
			KOTH_SCR_PlayerProfileComponent playerProfile = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
			playerProfile.lastVehicle ;
		}
	}

	bool IsNearKothZone(IEntity compartmentEntity)
	{
		vector vehiclePosition = compartmentEntity.GetOrigin();
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

		if (!gameMode || !gameMode.m_kothTrigger)
			return false;

		vector kothTriggerPosition = gameMode.m_kothTrigger.GetOrigin();
		float distanceFromZone = vector.Distance(vehiclePosition, kothTriggerPosition);

		return (distanceFromZone <= 350);
	}

	void HandleOccupantDropNearZone(int occupantID, int pilotId)
	{
		string occupantUID = KOTH_Helper.GetPlayerUID(occupantID);
		PlayerController occupantPlayerController = m_playerManager.GetPlayerController(occupantID);
		if (!occupantPlayerController)
			return;

		KOTH_SCR_PlayerProfileComponent occupantProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(occupantPlayerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
		if (occupantProfileComp.HasBeenDroppedNearZone())
			return;

		occupantProfileComp.DroppedNearZone();

		Log("Occupant marked as dropped near zone. Points to be awarded to " + m_playerManager.GetPlayerName(pilotId));

		HandlePilotDropBonus(pilotId, occupantUID);
	}

	void HandlePilotDropBonus(int pilotId, string occupantUID)
	{
		string pilotUID = KOTH_Helper.GetPlayerUID(pilotId);
		LogWorkbench("Pilot ID: " + pilotId + ", Pilot UID: " + pilotUID);

		if (pilotUID == occupantUID || pilotUID.IsEmpty())
			return;

		KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(m_playerManager.GetPlayerController(pilotId).FindComponent(KOTH_SCR_PlayerProfileComponent));
		KOTH_PlayerProfileJson profile = m_kothBackendApi.m_CurrentProfileList.Get(pilotUID);
		KOTH_PlayerStatsJson stats = m_kothBackendApi.m_playerStatsList.Get(pilotUID);

		stats.m_insertionBonus++;
		profileComp.AddToInsertionStreak();

		int insertionStreakBonus = FindInsertionStreakBonus(profileComp, pilotUID);
		if (insertionStreakBonus != 0)
			ApplyInsertionStreakReward(profile, profileComp, pilotUID, insertionStreakBonus);

		int bonus = m_expManager.GetDropBonus(pilotUID);
		profile.AddXp(bonus);
		profile.AddMoney(bonus);

		m_assistSystem.AddAssistRelationship(occupantUID, pilotUID);
		m_sessionDataGameComp.AddSessionXpAndMoney(bonus, bonus, pilotUID);
		m_kothBackendApi.DoRpcSyncProfileToPlayer(profile);

		// Show notification for pilot
		profileComp.DoRpc_NotifDropFriendly(bonus.ToString());
	}

	// Helper function to calculate altitude above ground level - adapted from the raycast center position
	float GetAltitudeAboveGround(IEntity entity, float maxDistanceRayCheck = 1000.0)
	{
		vector entityPosition = entity.GetOrigin();
		vector startPos = entityPosition + Vector(0, 0.3, 0);
		vector downVector = "0 -1 0";

		TraceParam trace = new TraceParam();
		trace.Start = startPos;
		trace.End = startPos + downVector * maxDistanceRayCheck;
		trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD | TraceFlags.OCEAN;
		trace.LayerMask = EPhysicsLayerDefs.CharCollide | EPhysicsLayerDefs.Static | EPhysicsLayerDefs.Water | EPhysicsLayerDefs.Dynamic | EPhysicsLayerDefs.Terrain | EPhysicsLayerDefs.Vehicle;

		// Perform the raycast
		float rayDistance = GetGame().GetWorld().TraceMove(trace, null);

		// If the ray hit nothing or was too far, return -1
		if (rayDistance >= 1)
			return -1;

		// Calculate the hit position
		vector hitPos = startPos + downVector * (rayDistance * maxDistanceRayCheck);

		// Calculate the distance between the entity's position and the hit position
		float altitudeAboveObject = vector.Distance(startPos, hitPos);

		return altitudeAboveObject;
	}

	void ApplyInsertionStreakReward(KOTH_PlayerProfileJson profile, KOTH_SCR_PlayerProfileComponent profileComp, string uid, int bonus)
	{
		profile.AddMoney(bonus);
		profile.AddXp(bonus);
		profileComp.DoRpc_Notif_InsertionStreak(profileComp.GetInsertionStreak(), bonus);
		m_sessionDataGameComp.UpdateInsertionStreak(uid, profileComp.GetInsertionStreak());
	}

	// Returns the insertion streak bonus
	int FindInsertionStreakBonus(KOTH_SCR_PlayerProfileComponent pilotProfileComp, string pilotUID)
	{
		int insertionStreakBonus = 0;
		switch (pilotProfileComp.GetInsertionStreak())
		{
			// 2 full UH's = 16 passengers
			case 16:
				insertionStreakBonus = 150;
				Log("insertionStreak 16 for player " + pilotUID);
				break;
			case 32:
				insertionStreakBonus = 400;
				Log("insertionStreak 32 for player " + pilotUID);
				break;
			case 50:
				insertionStreakBonus = 750;
				Log("insertionStreak 50 for player " + pilotUID);
				break;
			case 100:
				insertionStreakBonus = 2000;
				Log("insertionStreak 100 for player " + pilotUID);
				break;
		}
    
		return insertionStreakBonus;
	}

	void CleanUpPlayerEventHandlers(IEntity playerEntity)
	{
		SCR_ChimeraCharacter chimeraPlayer = SCR_ChimeraCharacter.Cast(playerEntity);
		SCR_CompartmentAccessComponent baseCompartmentAccessComp = SCR_CompartmentAccessComponent.Cast(chimeraPlayer.FindComponent(SCR_CompartmentAccessComponent));

		//remove any event handlers just in case as they will be added back later if they respawn
		baseCompartmentAccessComp.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
		baseCompartmentAccessComp.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
		baseCompartmentAccessComp.GetOnPlayerCompartmentEnter().Remove(OnPlayerEnterCompartment);
		baseCompartmentAccessComp.GetOnPlayerCompartmentExit().Remove(OnPlayerExitCompartment);
	}

	void HandleFlushToilet(int playerId)
	{
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);
		LogWorkbench("HandleFlushToilet Player ID: " + playerId);

		KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(m_playerManager.GetPlayerController(playerId).FindComponent(KOTH_SCR_PlayerProfileComponent));
		KOTH_PlayerProfileJson profile = m_kothBackendApi.m_CurrentProfileList.Get(playerUID);

		if (profileComp.GetHasFlushToilet())
			return;

		profile.AddXp(KOTH_Globals.BONUS_FLUSH_TOILET);
		profile.AddMoney(KOTH_Globals.BONUS_FLUSH_TOILET);

		m_sessionDataGameComp.AddSessionXpAndMoney(KOTH_Globals.BONUS_FLUSH_TOILET, KOTH_Globals.BONUS_FLUSH_TOILET, playerUID);
		m_kothBackendApi.DoRpcSyncProfileToPlayer(profile);

		// Show notification for player
		profileComp.FlushToilet(KOTH_Globals.BONUS_FLUSH_TOILET.ToString());
	}

}
