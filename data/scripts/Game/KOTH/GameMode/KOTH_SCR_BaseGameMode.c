modded class SCR_BaseGameMode
{
	const int WINNER_POINTS_NEEDED = 100;

	IEntity m_kothTrigger;
	IEntity m_kothPriorityTrigger;
	
	IEntity m_firstSpawn;
	IEntity m_secondSpawn;
	IEntity m_thirdSpawn;
	SCR_SpawnPoint m_firstSpawnPoint;
	SCR_SpawnPoint m_secondSpawnPoint;
	SCR_SpawnPoint m_thirdSpawnPoint;
	KOTH_SpawnProtectionTriggerEntity m_firstProtect;
	KOTH_SpawnProtectionTriggerEntity m_secondProtect;
	KOTH_SpawnProtectionTriggerEntity m_thirdProtect;
	
	protected ref array<Vehicle> m_vehicles = {};
	
	protected TagSystem m_tagSystem;
	
	void AddVehicletoRefund(notnull Vehicle vehicle) 
	{
		m_vehicles.Insert(vehicle);
		GetGame().GetCallqueue().CallLater(RemoveFromVehiclesToRefund, 60000, false, vehicle);
	}
	protected void RemoveFromVehiclesToRefund(Vehicle vehicle) 
	{
		if (vehicle)
			m_vehicles.RemoveItem(vehicle);
	}
	protected void RefundVehicles() 
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		KOTH_BackendApiGameModeComponent kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
		
		foreach (Vehicle vehicle : m_vehicles)
		{
			if (!vehicle)
				continue;

			if (vehicle.GetDamageManager().GetState() == EDamageState.DESTROYED)
				continue;

			int playerId = KOTH_BackendApiGameModeComponent.FindPlayerIdFromBohemiaUID(vehicle.GetOwnerUID());
			if (!playerId)
				continue;
			
			PlayerController playerController = playerManager.GetPlayerController(playerId);
			if (!playerController)
				continue;

			KOTH_SCR_PlayerShopComponent pcShopComp = KOTH_SCR_PlayerShopComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerShopComponent));
			KOTH_ShopItem kothShopItem = pcShopComp.FindShopItemByResourceName(vehicle.GetPrefabData().GetPrefabName());
			if (!kothShopItem)
				continue;

			KOTH_PlayerProfileJson profile = kothBackendApi.m_CurrentProfileList.Get(vehicle.GetOwnerUID());
			if (!profile)
				continue;

			profile.Refund(kothShopItem.m_priceOnce);
			kothBackendApi.DoRpcSyncProfileToPlayer(profile);
			
			KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));			
			profileComp.DoRpc_NotifRefundVehicleEndGame(kothShopItem.m_priceOnce.ToString());
			Log("refund "+kothShopItem.m_priceOnce.ToString()+ " for "+playerManager.GetPlayerName(playerId) +" UID "+vehicle.GetOwnerUID());
		}
		
	}
	
	
	// ---------- game start
	protected override void OnGameStart()
	{
		super.OnGameStart();
		
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		m_tagSystem = TagSystem.Cast(world.FindSystem(TagSystem));

		// handle spawns
		m_firstSpawn = world.FindEntityByName("KOTH_FirstSpawn");
		m_secondSpawn = world.FindEntityByName("KOTH_SecondSpawn");
		m_thirdSpawn = world.FindEntityByName("KOTH_ThirdSpawn");
		
		m_firstSpawnPoint = SCR_SpawnPoint.Cast(KOTH_SpawnHelper.FindSpawnPoint(m_firstSpawn));
		m_secondSpawnPoint = SCR_SpawnPoint.Cast(KOTH_SpawnHelper.FindSpawnPoint(m_secondSpawn));
		m_thirdSpawnPoint = SCR_SpawnPoint.Cast(KOTH_SpawnHelper.FindSpawnPoint(m_thirdSpawn));
		
		m_firstProtect = KOTH_SpawnHelper.FindSpawnProtection(m_firstSpawn);
		m_secondProtect = KOTH_SpawnHelper.FindSpawnProtection(m_secondSpawn);
		m_thirdProtect = KOTH_SpawnHelper.FindSpawnProtection(m_thirdSpawn);

		GetGame().GetCallqueue().CallLater(KOTH_SpawnHelper.AttachProperFlag, 5000, false, m_firstSpawn);
		GetGame().GetCallqueue().CallLater(KOTH_SpawnHelper.AttachProperFlag, 5000, false, m_secondSpawn);
		GetGame().GetCallqueue().CallLater(KOTH_SpawnHelper.AttachProperFlag, 5000, false, m_thirdSpawn);
		
		// reset repair ui
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget repairWidget = workspace.FindAnyWidget("repairZoneRootFrame");
		if (repairWidget) {
			repairWidget.SetVisible(false);
			ProgressBarWidget pBar = ProgressBarWidget.Cast(repairWidget.FindAnyWidget("ProgressBar"));
			if (pBar)
				pBar.SetCurrent(0);
		}
		
		if (!Replication.IsServer())
			return;

		#ifdef WORKBENCH
		#else
			GetGame().GetCallqueue().CallLater(KOTH_ScenarioHistoryManager.HandleScenarioHistory, 1000, false);
		#endif
		
		KOTH_SpawnHelper.SpawnFreeVehicles();
		GetGame().GetCallqueue().CallLater(KOTH_SpawnHelper.SpawnFreeVehicles, 60000 * 5, true);
		
		// random cloudy weather and random night time
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		array<string> weathers = {"Clear", "Cloudy", "Rain"};
		weatherManager.ForceWeatherTo(false, weathers.GetRandomElement());
		array<int> hours = {14, 6, 14, 18, 14};
		weatherManager.SetHoursMinutesSeconds(hours.GetRandomElement(),0,0);
	}

	// called only server side
	override void StartGameMode()
	{
		if (!Replication.IsServer())
			return;

		KOTH_SpawnPrefab firstVehicleSpawn = KOTH_SpawnPrefab.Cast(GetGame().GetWorld().FindEntityByName("KOTH_FirstVehicleSpawn"));
		KOTH_SpawnPrefab secondVehicleSpawn = KOTH_SpawnPrefab.Cast(GetGame().GetWorld().FindEntityByName("KOTH_SecondVehicleSpawn"));
		KOTH_SpawnPrefab thirdVehicleSpawn = KOTH_SpawnPrefab.Cast(GetGame().GetWorld().FindEntityByName("KOTH_ThirdVehicleSpawn"));
		
		IEntity firstFlag = KOTH_SpawnHelper.FindFlag(m_firstSpawnPoint);
		IEntity secondFlag = KOTH_SpawnHelper.FindFlag(m_secondSpawnPoint);
		IEntity thirdFlag = KOTH_SpawnHelper.FindFlag(m_thirdSpawnPoint);

		// randomize spawns
		array<int> randomInts = KOTH_Helper.GetUniqueRandomInts(3);
		array<string> factions = {};
		factions.Insert(KOTH_Faction.BLUFOR);
		factions.Insert(KOTH_Faction.OPFOR);
		factions.Insert(KOTH_Faction.INDFOR);

		m_firstSpawnPoint.SetFactionKey(factions.Get(randomInts[0]));
		firstVehicleSpawn.SetFactionKey(factions.Get(randomInts[0]));
		
		m_secondSpawnPoint.SetFactionKey(factions.Get(randomInts[1]));
		secondVehicleSpawn.SetFactionKey(factions.Get(randomInts[1]));
		
		m_thirdSpawnPoint.SetFactionKey(factions.Get(randomInts[2]));
		thirdVehicleSpawn.SetFactionKey(factions.Get(randomInts[2]));
		
		GetGame().GetCallqueue().CallLater(PlayersProtection, 500, true);
		GetGame().GetCallqueue().CallLater(DeleteProjectilesAtSpawns, 100, true);

		super.StartGameMode();
	}

	private void PlayersProtection()
	{
		array<int> playerIds = {};
		PlayerManager playerManager = GetGame().GetPlayerManager();
		playerManager.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			bool isPlayerInProtectionZone = false;
			IEntity controlledEntity = playerManager.GetPlayerControlledEntity(playerId);
			if (!controlledEntity) 
				continue;

			SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(controlledEntity.FindComponent(SCR_DamageManagerComponent));
			if (!damageManager)
				continue;

			FactionAffiliationComponent targetFactionComp = FactionAffiliationComponent.Cast(controlledEntity.FindComponent(FactionAffiliationComponent));
			Faction faction = targetFactionComp.GetAffiliatedFaction();
			if (!faction)
				continue;

			FactionKey userFactionKey = faction.GetFactionKey();
				
			if (m_firstSpawnPoint.GetFactionKey() == userFactionKey) {
				if (m_firstProtect.QueryEntityInside(controlledEntity))
					isPlayerInProtectionZone = true;
			}
				
			if (m_secondSpawnPoint.GetFactionKey() == userFactionKey) {
				if (m_secondProtect.QueryEntityInside(controlledEntity))
					isPlayerInProtectionZone = true;
			}

			if (m_thirdSpawnPoint.GetFactionKey() == userFactionKey) {
				if (m_thirdProtect.QueryEntityInside(controlledEntity))
					isPlayerInProtectionZone = true;
			}
			
			SCR_CharacterControllerComponent characterController = KOTH_Helper.GetCharacterControllerFromEntity(controlledEntity);
			PlayerController pController = playerManager.GetPlayerController(playerId);
			if (isPlayerInProtectionZone)
			{
				if (damageManager.IsDamageHandlingEnabled()) 
				{
					KOTH_SCR_PlayerShopComponent playerShopComp = KOTH_SCR_PlayerShopComponent.Cast(pController.FindComponent(KOTH_SCR_PlayerShopComponent));
					playerShopComp.RpcDo_LockInventory();
					damageManager.EnableDamageHandling(false);
				}
			}
			else
			{
				if (!damageManager.IsDamageHandlingEnabled())
				{
					KOTH_SCR_PlayerShopComponent playerShopComp = KOTH_SCR_PlayerShopComponent.Cast(pController.FindComponent(KOTH_SCR_PlayerShopComponent));
					playerShopComp.RpcDo_UnlockInventory();
					damageManager.EnableDamageHandling(true);
				}
			}
		}
	}

	private void DeleteProjectilesAtSpawns()
	{
		// thx MarioE for showing the TagSystem -> faster than big triggers
		array<IEntity> playerEntities = {};
		array<IEntity> allPlayersEntities = {};
		m_tagSystem.GetTagsInRange(playerEntities, m_firstProtect.GetOrigin(), m_firstProtect.GetSphereRadius(), ETagCategory.NameTag);
		allPlayersEntities.InsertAll(playerEntities);
		m_tagSystem.GetTagsInRange(playerEntities, m_secondProtect.GetOrigin(), m_secondProtect.GetSphereRadius(), ETagCategory.NameTag);
		allPlayersEntities.InsertAll(playerEntities);
		m_tagSystem.GetTagsInRange(playerEntities, m_thirdProtect.GetOrigin(), m_thirdProtect.GetSphereRadius(), ETagCategory.NameTag);
		allPlayersEntities.InsertAll(playerEntities);
		
		array<int> playerIds = {};
		PlayerManager playerManager = GetGame().GetPlayerManager();
		playerManager.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			IEntity playerControlledEnt = playerManager.GetPlayerControlledEntity(playerId);
			if (!playerControlledEnt)
				continue;
			
			CharacterWeaponManagerComponent wpnManager = CharacterWeaponManagerComponent.Cast(playerControlledEnt.FindComponent(CharacterWeaponManagerComponent));
	        if (!wpnManager)
	            return;
	        
	        array<IEntity> outWeapons = {};
	        wpnManager.GetWeaponsList(outWeapons);

			// check if player is in one of spawn protection zone
			if (allPlayersEntities.Contains(playerControlledEnt))
			{
				// subscribe to OnFired events to delete projectiles/bullets
				foreach (IEntity weapon : outWeapons)
		        {
		            SCR_MuzzleEffectComponent muzzleEffectComp = SCR_MuzzleEffectComponent.Cast(weapon.FindComponent(SCR_MuzzleEffectComponent));
		            if (!muzzleEffectComp)
		                continue;
		
		            muzzleEffectComp.Subscribe(true);
	            }
				
				//TODO: move lock inventory and character invincible here ? 
			}
			else
			{
				foreach (IEntity weapon : outWeapons)
		        {
		            SCR_MuzzleEffectComponent muzzleEffectComp = SCR_MuzzleEffectComponent.Cast(weapon.FindComponent(SCR_MuzzleEffectComponent));
		            if (!muzzleEffectComp)
		                continue;
		
		            muzzleEffectComp.Subscribe(false);
	            }
			}
		}

		array<IEntity> nadeEntities = {};
		array<IEntity> allNadesEntities = {};
		m_tagSystem.GetTagsInRange(nadeEntities, m_firstProtect.GetOrigin(), m_firstProtect.GetSphereRadius(), ETagCategory.Grenade);
		allNadesEntities.InsertAll(nadeEntities);
		m_tagSystem.GetTagsInRange(nadeEntities, m_secondProtect.GetOrigin(), m_secondProtect.GetSphereRadius(), ETagCategory.Grenade);
		allNadesEntities.InsertAll(nadeEntities);
		m_tagSystem.GetTagsInRange(nadeEntities, m_thirdProtect.GetOrigin(), m_thirdProtect.GetSphereRadius(), ETagCategory.Grenade);
		allNadesEntities.InsertAll(nadeEntities);

		foreach (IEntity nadeEnt : allNadesEntities)
		{
			InventoryItemComponent invItemComp = InventoryItemComponent.Cast(nadeEnt.FindComponent(InventoryItemComponent));
            if (!invItemComp)
				continue;
			
			if (invItemComp.GetParentSlot() != null)
				continue;

			SCR_EntityHelper.DeleteEntityAndChildren(nadeEnt);
		}

	}	

	// ---------- game end stuff

	void CheckGameEnd()
	{
		if (!Replication.IsServer())
			return;

		array<Faction> factions = {};
		ArmaReforgerScripted game = GetGame();
		FactionManager factionManager = game.GetFactionManager();
		factionManager.GetFactionsList(factions);

		string factionName = string.Empty;
		KOTH_ScoringGameModeComponent scoreComp = KOTH_ScoringGameModeComponent.Cast(FindComponent(KOTH_ScoringGameModeComponent));
		if (!scoreComp) {
			Log("Missing KOTH_ScoringGameModeComponent on gameMode", LogLevel.FATAL);
			return;
		}

		Log("CheckGameEnd BLU="+scoreComp.GetBlueforPoint()+" RED="+scoreComp.GetRedforPoint()+" IND="+scoreComp.GetGreenforPoint());

		if (scoreComp.GetBlueforPoint() >= WINNER_POINTS_NEEDED) { factionName = KOTH_Faction.BLUFOR; };
		if (scoreComp.GetRedforPoint() >= WINNER_POINTS_NEEDED) { factionName = KOTH_Faction.OPFOR; };
		if (scoreComp.GetGreenforPoint() >= WINNER_POINTS_NEEDED) { factionName = KOTH_Faction.INDFOR; };
		if (factionName == string.Empty) { return; }

		foreach (Faction faction : factions)
		{
			if (faction.GetFactionName() == factionName) 
			{
				RefundVehicles();
				GetGame().GetCallqueue().Remove(DeleteProjectilesAtSpawns);
				GetGame().GetCallqueue().Remove(PlayersProtection);
				KOTH_ZoneManagerComponent kothZoneManager = KOTH_ZoneManagerComponent.Cast(FindComponent(KOTH_ZoneManagerComponent));
				kothZoneManager.ClearCallLater();
				KOTH_BackendApiGameModeComponent kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(FindComponent(KOTH_BackendApiGameModeComponent));
				kothBackendApi.SendStatTeamWinner(factionName);
				scoreComp.OnBeforeGameEnd();
				int factionIndex = GetGame().GetFactionManager().GetFactionIndex(faction);
				SCR_GameModeEndData gameModeEndData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.FACTION_VICTORY_SCORE, winnerFactionId: factionIndex);
				EndGameMode(gameModeEndData);
				
				// random next scenario
				string nextScenario = KOTH_ScenarioHistoryManager.ChooseNextScenario();
				
				// check if server need restart 8388608 = 8gig
				Log("MemoryAllocationKB "+System.MemoryAllocationKB());
				if (System.MemoryAllocationKB() > 8388608) 
				{
					Log("Automated server restart due to high memory usage.");
					Rpc(RpcDo_SendRestartMessage);
					GetGame().GetCallqueue().CallLater(CloseGame, 20000, false);
					return;
				}

				GetGame().GetCallqueue().CallLater(ChangeScenario, 30000, false, nextScenario);
			}
		}
	}
	
	void ChangeScenario(string nextScenario)
	{
		KOTH_ExperienceManager.ClearAll();
		KOTH_ScoringGameModeComponent scoreComp = KOTH_ScoringGameModeComponent.Cast(FindComponent(KOTH_ScoringGameModeComponent));
		string votedMap = scoreComp.GetMostVotedMapConf();
		if (votedMap != string.Empty)
			nextScenario = votedMap;

		KOTH_ScenarioHistoryManager.AddScenarioToHistory(nextScenario);
		GameStateTransitions.RequestScenarioChangeTransition(nextScenario, KOTH_Helper.GetAddonsGUIDs());
	}
	
	private void CloseGame()
	{
		GetGame().RequestClose();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_SendRestartMessage()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_ChatComponent chatComp = SCR_ChatComponent.Cast(playerController.FindComponent(SCR_ChatComponent));
		if (!chatComp)
			return;
		
		chatComp.ShowMessage("SERVER RESTART");
		chatComp.ShowMessage("SERVER RESTART");
		chatComp.ShowMessage("SERVER RESTART");
		chatComp.ShowMessage("SERVER RESTART");
		chatComp.ShowMessage("SERVER RESTART");
	};
}
