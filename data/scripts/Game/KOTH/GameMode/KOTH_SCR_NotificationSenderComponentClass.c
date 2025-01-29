modded class SCR_NotificationSenderComponent : SCR_BaseGameModeComponent
{
    KOTH_VehicleEventsGameModeComponent m_vehEventGameComp;
    PlayerManager m_playerManager;
    KOTH_DeathHandlingGameModeComponent m_deathHandlingComp;

    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);

        if (SCR_Global.IsEditMode(owner))
            return;

        m_vehEventGameComp = KOTH_VehicleEventsGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_VehicleEventsGameModeComponent));
        m_playerManager = GetGame().GetPlayerManager();
        m_deathHandlingComp = KOTH_DeathHandlingGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_DeathHandlingGameModeComponent));
    }
    
    override void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
	    LogWorkbench("============================= NotificationSenderComponent ==============================");
	
	    if (!Replication.IsServer())
	        return;
	
	    int victimPlayerId = instigatorContextData.GetVictimPlayerID();
	    IEntity victimEntity = instigatorContextData.GetVictimEntity();
	    
	    if (!victimEntity || !ChimeraCharacter.Cast(victimEntity))
	    {
	        LogWorkbench("NotificationSenderComponent: Victim entity is not a character. Skipping.");
	        return;
	    }
		
		int killerPlayerId = m_deathHandlingComp.GetCachedGeneralKillerId(victimPlayerId);
		IEntity killerEntity = m_playerManager.GetPlayerControlledEntity(killerPlayerId);
		
	    // Default to original killer if no resolved killer found
	    if (killerPlayerId == -1)
	        killerPlayerId = instigatorContextData.GetKillerPlayerID();
	
	    SCR_ECharacterControlType victimControlType = instigatorContextData.GetVictimCharacterControlType();
	    SCR_ECharacterControlType killerControlType = SCR_ECharacterControlType.PLAYER; // Default to player
	
	    if (killerEntity)
	        killerControlType = SCR_CharacterHelper.GetCharacterControlType(killerEntity);
	
	    // Suicide handling
	    if (killerPlayerId == victimPlayerId)
	    {
	        LogWorkbench("NotificationSenderComponent: Victim committed suicide.");
	        SendDeathNotification(victimControlType, ENotification.PLAYER_DIED, victimPlayerId);
	        return;
	    }
	
	    // Player killed by another player
	    if (killerControlType == SCR_ECharacterControlType.PLAYER || killerControlType == SCR_ECharacterControlType.UNLIMITED_EDITOR)
	    {
	        LogWorkbench("NotificationSenderComponent: Victim killed by player.");
	        SendKillNotification(killerPlayerId, victimPlayerId, victimControlType, ENotification.PLAYER_KILLED_PLAYER);
	        return;
	    }
	
	    // AI killed player
	    if (killerControlType == SCR_ECharacterControlType.AI)
	    {
	        LogWorkbench("NotificationSenderComponent: Victim killed by AI.");
	        SendKillNotification(killerPlayerId, victimPlayerId, victimControlType, ENotification.AI_KILLED_PLAYER);
	        return;
	    }
	
	    // Unknown killer
	    LogWorkbench("NotificationSenderComponent: Unknown killer. Defaulting to player death.");
	    SendDeathNotification(victimControlType, ENotification.PLAYER_DIED, victimPlayerId);
		
		m_deathHandlingComp.RemovePlayerFromGeneralKillerCache(victimPlayerId);
	}

	// send death notifications
	void SendDeathNotification(SCR_ECharacterControlType victimControlType, ENotification notificationType, int victimPlayerId)
	{
	    if (victimControlType == SCR_ECharacterControlType.POSSESSED_AI)
	        SCR_NotificationsComponent.SendToEveryone(ENotification.POSSESSED_AI_DIED, victimPlayerId);
	    else
	        SCR_NotificationsComponent.SendToEveryone(notificationType, victimPlayerId);
	}
	
	// send kill notifications
	void SendKillNotification(int killerPlayerId, int victimPlayerId, SCR_ECharacterControlType victimControlType, ENotification notificationType)
	{
	    if (victimControlType == SCR_ECharacterControlType.POSSESSED_AI)
	        SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_KILLED_POSSESSED_AI, killerPlayerId, victimPlayerId);
	    else
	        SCR_NotificationsComponent.SendToEveryone(notificationType, killerPlayerId, victimPlayerId);
	}

	
	//disable the gm left message
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout) 
	{
		//~ Should never be called for clients but just in case
		if (!GetGameMode().IsMaster())
			return;
		
		SCR_EditorManagerEntity editorManager;
		
		//~ Get editorManager if has any
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
			editorManager = core.GetEditorManager(playerId);

		bool hasUnlimitedEditor = editorManager && !editorManager.IsLimited();
		
		//~ Is GM, Always show GM left notification even if kicked/banned to notify players that the GM has left
//		if (hasUnlimitedEditor)
//			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_GM_LEFT, playerId);		
//		
		bool isKickedOrBanned = false;
		
		//~ Check if disconnect cause has message attached to it. If true: show kick/ban reason. If false: only show gm/player left
		if (m_PlayerKickReasonsConfig)
		{
			string groupId, reasonId;
			KickCauseGroup2 groupInt;
			int reasonInt;
			
			//~ Get disconnect message preset
			GetGame().GetFullKickReason(cause, groupInt, reasonInt, groupId, reasonId);
			SCR_ConfigurableDialogUiPreset preset = m_PlayerKickReasonsConfig.FindPreset(groupId + "_" + reasonId);
			
			//~ If has kick/Ban message it will send out a notification
			isKickedOrBanned = preset != null && !preset.m_sMessage.IsEmpty();
		}
		//~ No config
		else 
		{
			Print("'SCR_NotificationSenderComponent' has no 'm_PlayerKickReasonsConfig'! Make sure it is added else it will never know if a player was kicked!", LogLevel.ERROR);
		}
		
		//~ Is kicked/banned. Will also send ban notification if for some reason there is a timeout attached even if there is no specific kick message
		if (isKickedOrBanned || timeout != 0)
		{
			SCR_DataCollectorComponent dataCollector = GetGame().GetDataCollector();
			if (dataCollector)
			{
				SCR_PlayerData playerData = dataCollector.GetPlayerData(playerId);
				
				if (playerData)
				{
					float banTimeOut = playerData.GetTimeOut();
				
					//~ If playerData has ban timeout which is greater then timeout use that instead. This is because Heavy ban kicks the player and bans it via backend. So the timeout is set somewhere else
					if (banTimeOut > 0 && banTimeOut > timeout)
						timeout = banTimeOut;
				}
			}
			
			//~ Player kicked 
			if (timeout == 0)
				SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_KICKED, playerId, cause, timeout);	
			//~ Player perminent ban
			else if (timeout < 0)
				SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_BANNED_NO_DURATION, playerId, cause, timeout);
			//~ Player temp ban
			else
				SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_BANNED, playerId, cause, timeout);
		}		
		//~ Is Not kicked/banned, is player and should send on leave notification. 
		else if (m_bShowPlayerLeftNotification && !hasUnlimitedEditor)
		{
			SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_LEFT, playerId);
		}
	}
}
