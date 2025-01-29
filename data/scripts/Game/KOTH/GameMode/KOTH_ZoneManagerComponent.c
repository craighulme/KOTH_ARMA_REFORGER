class KOTH_ZoneManagerComponentClass : SCR_BaseGameModeComponentClass {}

[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Handles KOTH Zones")]
class KOTH_ZoneManagerComponent : SCR_BaseGameModeComponent
{
	  //bump these values on client if gm or game updates these
    [RplProp()]
    protected vector m_zoneCenter;

    [RplProp()]
    protected float m_zoneRadius;

    [RplProp(onRplName: "OnPriorityZoneCenterChanged")]
	  protected vector m_priorityZoneCenter;
	
	  [RplProp(onRplName: "OnPriorityZoneRadiusChanged")]
	  protected float m_priorityZoneRadius;

    protected SCR_BaseTriggerEntity m_zoneTrigger;
    protected SCR_BaseTriggerEntity m_priorityZoneTrigger;
	
	  // how many times after it calls getqueuecalllater (15 seconds) before it moves prio. Randomly generated, 20 is just placeholder
    protected int m_priorityZoneMoveInterval = 20;
	
	  protected int m_priorityZoneMoveMinInterval = 20;
    protected int m_priorityZoneMoveMaxInterval = 40;

	  protected float m_priorityZoneMoveMinRadius = 50.0; 
    protected float m_priorityZoneMoveMaxRadius = 100.0; 

    protected int m_priorityZoneMoveCounter = 0;

    protected KOTH_ExperienceManager m_expManager;
    protected KOTH_ScoringGameModeComponent m_scoreComp;
    protected KOTH_SessionDataGameModeComponent m_sessionDataGameComp;
    protected KOTH_SCR_MapDescriptorComponent m_mapDescriptor;
    protected KOTH_BackendApiGameModeComponent m_kothBackendApi;
    protected PlayerManager m_playerManager;
	  protected SCR_BaseGameMode gameMode;

	  // Called when the game mode starts
    override void OnGameModeStart()
    {
        super.OnGameModeStart();

        gameMode = GetGameMode();
        if (!gameMode)
        {
            LogWorkbench("KOTH_ZoneManagerComponent: Game mode not found!");
            return;
        }

		    m_expManager = KOTH_ExperienceManager.GetInstance();
        m_scoreComp = KOTH_ScoringGameModeComponent.Cast(gameMode.FindComponent(KOTH_ScoringGameModeComponent));
        m_sessionDataGameComp = KOTH_SessionDataGameModeComponent.Cast(gameMode.FindComponent(KOTH_SessionDataGameModeComponent));
        m_kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(gameMode.FindComponent(KOTH_BackendApiGameModeComponent));
        m_playerManager = GetGame().GetPlayerManager();
	
        InitializeZoneTriggers();

        if (Replication.IsServer())
        {
			      RandomizePriorityZoneMoveInterval();
            GetGame().GetCallqueue().CallLater(Update, 15000, true);
        }
    }
	
    void InitializeZoneTriggers()
    {
        m_zoneTrigger = KOTH_PresenceTriggerEntity.Cast(gameMode.m_kothTrigger);
        m_priorityZoneTrigger = KOTH_PriorityAreaPresenceTriggerEntity.Cast(gameMode.m_kothPriorityTrigger);

        if (!m_zoneTrigger || !m_priorityZoneTrigger)
        {
            LogWorkbench("KOTH_ZoneManagerComponent: Missing zone triggers!");
            return;
        }
		
        // Set initial values for the zones
        m_zoneCenter = m_zoneTrigger.GetOrigin();
        m_zoneRadius = m_zoneTrigger.GetSphereRadius();
        m_priorityZoneCenter = m_priorityZoneTrigger.GetOrigin();
        m_priorityZoneRadius = m_priorityZoneTrigger.GetSphereRadius();

        LogWorkbench("KOTH_ZoneManagerComponent: Zone triggers initialized successfully.");

        Replication.BumpMe();
    }

    void Update()
    {
        if (!Replication.IsServer())
            return;

        m_priorityZoneMoveCounter++;

        UpdateZonePoints();

        GetGameMode().CheckGameEnd();

        if (m_priorityZoneMoveCounter >= m_priorityZoneMoveInterval)
        {
            MovePriorityZone();
            m_priorityZoneMoveCounter = 0;
			      RandomizePriorityZoneMoveInterval();
        }
    }

    void UpdateZonePoints()
	  {
        int blueforCount = 0;
        int greenforCount = 0;
        int redforCount = 0;

        array<int> players = {};
        m_playerManager.GetAllPlayers(players);

        foreach (int playerId : players)
        {
            IEntity player = m_playerManager.GetPlayerControlledEntity(playerId);

            if (!IsPlayerEligible(player))
                continue;

            vector playerPos = player.GetOrigin();
            float distanceToZone = vector.Distance(playerPos, m_zoneCenter);
            float distanceToPriorityZone = vector.Distance(playerPos, m_priorityZoneCenter);

            // Check if the player is within the main zone
            if (distanceToZone <= m_zoneRadius)
            {
                bool isInPriorityZone = distanceToPriorityZone <= m_priorityZoneRadius;

                // Award points to the player
                GivePlayerZonePoints(playerId, isInPriorityZone);

                // Faction counting logic
                Faction faction = KOTH_Helper.GrabFaction(player);
                if (faction)
                {
                    string factionName = faction.GetFactionName();
                    if (factionName == KOTH_Faction.BLUFOR) blueforCount++;
                    else if (factionName == KOTH_Faction.OPFOR) redforCount++;
                    else if (factionName == KOTH_Faction.INDFOR) greenforCount++;

                    // Count priority zone players as an additional faction member
                    if (isInPriorityZone)
                    {
                        if (factionName == KOTH_Faction.BLUFOR) blueforCount++;
                        else if (factionName == KOTH_Faction.OPFOR) redforCount++;
                        else if (factionName == KOTH_Faction.INDFOR) greenforCount++;
                    }
                }
            }
	    }
	
	    DetermineZoneControl(blueforCount, greenforCount, redforCount);
	    m_scoreComp.BumpMe(); // Update clients for changed score
	  }

    void MovePriorityZone()
    {
        float angle = Math.RandomFloatInclusive(0, Math.PI2);
        float distance = Math.RandomFloatInclusive(m_priorityZoneMoveMinRadius, m_priorityZoneMoveMaxRadius);

        m_priorityZoneCenter[0] = m_zoneCenter[0] + Math.Cos(angle) * distance;
        m_priorityZoneCenter[2] = m_zoneCenter[2] + Math.Sin(angle) * distance;
        m_priorityZoneTrigger.SetOrigin(m_priorityZoneCenter);

        Replication.BumpMe();
    }
	
	  void RandomizePriorityZoneMoveInterval()
    {
        m_priorityZoneMoveInterval = Math.RandomIntInclusive(m_priorityZoneMoveMinInterval, m_priorityZoneMoveMaxInterval);
        LogWorkbench("RandomizePriorityZoneMoveInterval: New priority zone move interval: " + m_priorityZoneMoveInterval);
    }

    protected bool IsPlayerEligible(IEntity player)
    {
        ChimeraCharacter chimera = ChimeraCharacter.Cast(player);
        if (!chimera) return false;

        CharacterControllerComponent controllerComp = chimera.GetCharacterController();
        return !(controllerComp.IsDead() || controllerComp.IsUnconscious());
    }

    protected void GivePlayerZonePoints(int playerId, bool isInPriorityZone)
	  {
        string playerUID = KOTH_Helper.GetPlayerUID(playerId);
        if (!playerUID || playerUID == string.Empty) return;

        PlayerController playerController = m_playerManager.GetPlayerController(playerId);
        if (!playerController) return;

        KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
        if (!profileComp) return;

        KOTH_PlayerProfileJson profile = m_kothBackendApi.m_CurrentProfileList.Get(playerUID);
        int bonus = m_expManager.GetZoneBonus(playerUID);
        if (isInPriorityZone)
        {
            bonus *= 2; // Double the bonus for priority zone
            profileComp.DoRpc_NotifCapturePriorityArea(bonus.ToString()); // Notify priority zone capture
        }
        else
        {
            profileComp.DoRpc_NotifCapture(bonus.ToString()); // Notify regular zone capture
        }

        profile.AddXp(bonus);
        profile.AddMoney(bonus);
        m_sessionDataGameComp.AddSessionXpAndMoney(bonus, bonus, playerUID);
        profileComp.DoRpc_SyncPlayerProfile(profile);
	  }

    protected void DetermineZoneControl(int blueforCount, int greenforCount, int redforCount)
    {
        bool isZoneContested = true;
		
        if(!m_mapDescriptor)
          m_mapDescriptor = KOTH_SCR_MapDescriptorComponent.Cast(m_zoneTrigger.FindComponent(KOTH_SCR_MapDescriptorComponent));
		
        if (blueforCount > greenforCount && blueforCount > redforCount)
        {
            isZoneContested = false;
            m_scoreComp.AddBlueforPoint();
            m_mapDescriptor.SetState(KOTH_Faction.BLUFOR);
        }
        else if (greenforCount > blueforCount && greenforCount > redforCount)
        {
            isZoneContested = false;
            m_scoreComp.AddGreenforPoint();
            m_mapDescriptor.SetState(KOTH_Faction.INDFOR);
        }
        else if (redforCount > blueforCount && redforCount > greenforCount)
        {
            isZoneContested = false;
            m_scoreComp.AddRedforPoint();
            m_mapDescriptor.SetState(KOTH_Faction.OPFOR);
        }

        if (isZoneContested)
        {
            m_mapDescriptor.SetState("contested");
        }
    }

	protected void OnPriorityZoneCenterChanged()
	{
	    if (!Replication.IsClient())
	        return;
		
	    UpdateClientPrioIcon();
	}
	
	protected void OnPriorityZoneRadiusChanged()
	{
	    if (!Replication.IsClient())
	        return;
	
	    UpdateClientPrioIcon();
	}
	
	protected void UpdateClientPrioIcon()
	{
	    PlayerController playerController = m_playerManager.GetPlayerController(SCR_PlayerController.GetLocalPlayerId());
	    if (!playerController)
		{
			return;
		}

	    KOTH_SCR_PlayerMapMarkerHandlerComponent mapHandler = KOTH_SCR_PlayerMapMarkerHandlerComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerMapMarkerHandlerComponent));
	    if (!mapHandler)
	        return;
	
	    mapHandler.UpdatePrioIcon();
	}

	//------------------------------- Getters and Setters used externally--------------------------------
	// Radius bounds
    float GetPriorityZoneMoveMinRadius() { return m_priorityZoneMoveMinRadius; }
    void SetPriorityZoneMoveMinRadius(float minRadius) { m_priorityZoneMoveMinRadius = minRadius; }

    float GetPriorityZoneMoveMaxRadius() { return m_priorityZoneMoveMaxRadius; }
    void SetPriorityZoneMoveMaxRadius(float maxRadius) { m_priorityZoneMoveMaxRadius = maxRadius; }
	
	  vector GetZoneCenter() { return m_zoneCenter; }
    void SetZoneCenter(vector center) { m_zoneCenter = center; }

    float GetZoneRadius() { return m_zoneRadius; }
    void SetZoneRadius(float radius) { m_zoneRadius = radius; }

    vector GetPriorityZoneCenter() { return m_priorityZoneCenter; }
    void SetPriorityZoneCenter(vector center) { m_priorityZoneCenter = center; }

    float GetPriorityZoneRadius() { return m_priorityZoneRadius; }
    void SetPriorityZoneRadius(float radius) { m_priorityZoneRadius = radius; }
	
	  bool IsPointInZone(vector point) { return vector.Distance(point, m_zoneCenter) <= m_zoneRadius; }
	
	  bool IsPointInPriorityZone(vector point) { return vector.Distance(point, m_priorityZoneCenter) <= m_priorityZoneRadius; }

    // Interval bounds
    int GetPriorityZoneMoveMinInterval() { return m_priorityZoneMoveMinInterval; }
    void SetPriorityZoneMoveMinInterval(int minInterval) { m_priorityZoneMoveMinInterval = minInterval; }

    int GetPriorityZoneMoveMaxInterval() { return m_priorityZoneMoveMaxInterval; }
    void SetPriorityZoneMoveMaxInterval(int maxInterval) { m_priorityZoneMoveMaxInterval = maxInterval; }

	  void ClearCallLater() { GetGame().GetCallqueue().Remove(Update); }

};
