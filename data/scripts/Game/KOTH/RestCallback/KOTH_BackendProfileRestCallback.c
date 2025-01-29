class KOTH_BackendProfileRestCallback : RestCallback 
{
	override void OnError(int errorCode)
	{
		Log("KOTH_BackendProfileRestCallback Error with code "+errorCode, LogLevel.ERROR);
	}

	override void OnTimeout()
	{
		Log("KOTH_BackendProfileRestCallback timeout", LogLevel.ERROR);
	}

	override void OnSuccess(string data, int dataSize)
	{
		Log("KOTH_BackendProfileRestCallback success size= " + dataSize.ToString() + " data= "+data);
		
		KOTH_PlayerBanJson ban = new KOTH_PlayerBanJson();
		if (data.Contains("active")) 
		{
			ban.ExpandFromRAW(data);
			if (ban.active) 
			{
				int playerId = KOTH_BackendApiGameModeComponent.FindPlayerIdFromBohemiaUID(ban.playerUID);
				if (playerId != 0) 
				{
					GetGame().GetPlayerManager().KickPlayer(playerId, PlayerManagerKickReason.BAN);
					return;
				}
			}
		}

		KOTH_PlayerProfileJson profile = new KOTH_PlayerProfileJson();
		profile.ExpandFromRAW(data);
		
		array<int> allPlayers = {};
		PlayerManager playerManager = GetGame().GetPlayerManager();
		playerManager.GetPlayers(allPlayers);
		
		BackendApi backendApi = GetGame().GetBackendApi();
		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		PlayerController pc;
		
		foreach (int playerId : allPlayers)
		{
			// its local info so can stupid spam 
			// we dont use helper since we might ask for identity of players who are not audited yet
			string playerUID = backendApi.GetPlayerIdentityId(playerId);
			#ifdef WORKBENCH
				playerUID = KOTH_Helper.GetPlayerUID(playerId);
			#endif
			if (playerUID && playerUID == profile.m_playerUID)
			{
				profile.m_playerName = playerManager.GetPlayerName(playerId);
				
				PlatformKind playerPlatform = playerManager.GetPlatformKind(playerId);
				profile.m_platformName = SCR_Global.GetPlatformName(playerPlatform);
				
				pc = playerManager.GetPlayerController(playerId);
				KOTH_SCR_PlayerProfileComponent playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(pc.FindComponent(KOTH_SCR_PlayerProfileComponent));
				callQueue.CallLater(playerProfileComp.DoRpc_FindLocalProfileDatas, 1000 + playerId, false);
				break;
			}
		}

		BaseGameMode gameMode = GetGame().GetGameMode();
		KOTH_BackendApiGameModeComponent kothBackendApiGameModeComp = KOTH_BackendApiGameModeComponent.Cast(gameMode.FindComponent(KOTH_BackendApiGameModeComponent));
		kothBackendApiGameModeComp.AddProfileToCurrentList(profile);
		kothBackendApiGameModeComp.DoRpcSyncProfileToPlayer(profile);
		
		kothBackendApiGameModeComp.GetBonusCodeForPlayer(profile.m_playerUID);
		kothBackendApiGameModeComp.GetPlayerStats(profile.m_playerUID);
		
		KOTH_SessionDataGameModeComponent kothSessionDataGameModeComp = KOTH_SessionDataGameModeComponent.Cast(gameMode.FindComponent(KOTH_SessionDataGameModeComponent));
		if (profile.m_playerPresets.IsEmpty())
			return;
		
		if (pc) {
			KOTH_SCR_PlayerShopComponent playerShopComp = KOTH_SCR_PlayerShopComponent.Cast(pc.FindComponent(KOTH_SCR_PlayerShopComponent));
			kothSessionDataGameModeComp.m_sessionPlayersLoadout.Insert(profile.m_playerUID, KOTH_SessionPlayerLoadout.FromBackendPresetToSessionLoadout(playerShopComp, profile.m_playerPresets.Get(0)));		
		}
	}
	
	
}
