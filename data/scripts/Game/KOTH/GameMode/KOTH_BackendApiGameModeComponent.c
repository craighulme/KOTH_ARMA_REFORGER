// #define DEBUG_BACKEND_KOTH

class KOTH_BackendApiGameModeComponentClass : SCR_BaseGameModeComponentClass {}
class KOTH_BackendApiGameModeComponent : SCR_BaseGameModeComponent
{
	private string m_BaseUri = "http://kothrbackend.docker/api/";
	private string m_ApiKey = "official_testapikey";
	private ref KOTH_PlayerAuditSystem m_playerAuditSystem;

	protected RestContext m_RestContext;
	protected ref KOTH_RestCallback m_CallBackContext = new KOTH_RestCallback();
	protected ref KOTH_OnDisconnectRestCallback m_OnDisconnectCallBackContext = new KOTH_OnDisconnectRestCallback();
	protected ref KOTH_BackendProfileRestCallback m_BackendProfileCallBackContext = new KOTH_BackendProfileRestCallback();
	protected ref KOTH_BackendPlayerPresetRestCallback m_PlayerPresetCallback = new KOTH_BackendPlayerPresetRestCallback();
	protected ref KOTH_CheckBanRestCallback m_checkBanCallBackContext = new KOTH_CheckBanRestCallback();
	protected ref KOTH_BonusCodeRestCallback m_BonusCodeCallback = new KOTH_BonusCodeRestCallback();
	protected ref KOTH_PlayerStatsRestCallback m_PlayerStatsCallback = new KOTH_PlayerStatsRestCallback();
	
	protected KOTH_SessionDataGameModeComponent m_sessionDataGameComp;
	ref map<string, ref KOTH_PlayerProfileJson> m_CurrentProfileList = new map<string, ref KOTH_PlayerProfileJson>();
	ref map<string, ref KOTH_PlayerStatsJson> m_playerStatsList = new map<string, ref KOTH_PlayerStatsJson>();

	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;
		if (!Replication.IsServer())
			return;

		SetEventMask(owner, EntityEvent.INIT);
		super.OnPostInit(owner);
		
		m_RestContext = GetGame().GetRestApi().GetContext(m_BaseUri);
		
		m_sessionDataGameComp = KOTH_SessionDataGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_SessionDataGameModeComponent));
		m_playerAuditSystem = new KOTH_PlayerAuditSystem();
	}
	
	override void OnGameModeStart()
	{
		super.OnGameModeStart();

		if (!Replication.IsServer())
			return;
		
		// update uri from default value
		if (m_BaseUri == "http://kothrbackend.docker/api/") {
			ArmaReforgerScripted game = GetGame();
			m_BaseUri = game.KOTH_BackendBaseUri;

			if (m_BaseUri == "http://kothrbackend.docker/api/") {
				SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
				if (header && header.KOTH_BackendBaseUri != string.Empty) {
					game.KOTH_BackendBaseUri = header.KOTH_BackendBaseUri;
					m_BaseUri = header.KOTH_BackendBaseUri;
				} else {
					Log("ERROR no mission header found so no api key can be found");
					#ifdef WORKBENCH 
					#else
						game.RequestClose();
					#endif
				}
			}
		}
		
		// update api_key from default value
		if (m_ApiKey == "official_testapikey") {
			ArmaReforgerScripted game = GetGame();
			m_ApiKey = game.KOTH_ApiKey;

			if (m_ApiKey == "official_testapikey") {
				SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
				if (header) {
					game.KOTH_ApiKey = header.KOTH_ApiKey;
					m_ApiKey = header.KOTH_ApiKey;
				} else {
					Log("ERROR no mission header found so no api key can be found");
					#ifdef WORKBENCH 
					#else
						game.RequestClose();
					#endif
				}
			}
		}

		m_RestContext = GetGame().GetRestApi().GetContext(m_BaseUri);

		#ifdef WORKBENCH
			#ifdef DEBUG_BACKEND_KOTH
				GetGame().GetCallqueue().CallLater(SaveAllProfiles, 20000, true);
			#endif
		#else
			GetGame().GetCallqueue().CallLater(SaveAllProfiles, 60000 * 5, true);
		#endif
	}

	override void OnGameEnd()
	{
		if (!Replication.IsServer() || RplSession.Mode() != RplMode.Dedicated)
			return;
		
		Log("OnGameEnd launch KOTH_BackendApi.SavePlayersProfile()");
		
		SaveAllProfiles();
	}

	override void OnPlayerAuditSuccess(int playerId)
	{
		GetGame().GetCallqueue().CallLater(HandleOnConnect, 1000, false, playerId);
	}
	
	void LogAsciiValues(string word)
	{
		Log("Start for "+word);

		int index = 0;
		while(index <= word.Length() - 1)
		{
			Log(word.Get(index) + " = " + word.ToAscii(index));
			index++;
		}
		Log("End for "+word);
	}
	
	override void OnPlayerConnected(int playerId)
	{
		if (!Replication.IsServer())
			return;

		m_playerAuditSystem.AddPlayer(playerId);

		#ifdef WORKBENCH
			GetGame().GetCallqueue().CallLater(HandleOnConnect, 1000, false, playerId);
		#endif
	}
	
	void AddPlayerToAudit(int playerId)
	{
		m_playerAuditSystem.AddPlayer(playerId);
	}
	
	void SaveAllProfiles()
	{
		if (!Replication.IsServer())
			return;
		
		Log(m_CurrentProfileList.Count().ToString()+" profiles to saves");
		KOTH_ListPlayerProfileJson list = new KOTH_ListPlayerProfileJson();
		foreach (KOTH_PlayerProfileJson profile: m_CurrentProfileList)
		{
			list.m_list.Insert(profile);
		}
		SendListProfilesToBackend(list);
		
		KOTH_ListPlayerStatsJson listStats = new KOTH_ListPlayerStatsJson();
		foreach (KOTH_PlayerStatsJson profile: m_playerStatsList)
		{
			listStats.m_list.Insert(profile);
		}
		SendPlayersStatsToBackend(listStats);
	}
	
	private void HandleOnConnect(int playerId)
	{
		#ifdef WORKBENCH
			#ifdef DEBUG_BACKEND_KOTH
			#else
				WB_GetFakeProfile(playerId);
				return;
			#endif
		#endif
		
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);

		if (!GetGame().GetPlayerManager().IsPlayerConnected(playerId))
			return;

		if (playerUID == string.Empty) 
		{
			GetGame().GetCallqueue().CallLater(HandleOnConnect, 2000, false, playerId);
			return;
		}

		if (m_CurrentProfileList.Contains(playerUID))
		{
			PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
			KOTH_SCR_PlayerProfileComponent playerProfile = KOTH_SCR_PlayerProfileComponent.Cast(pc.FindComponent(KOTH_SCR_PlayerProfileComponent));
			playerProfile.SetPlayerUID(playerUID);
			
			Log("KOTH_BackendApiGameModeComponent.HandleOnConnect profile found "+playerUID);
			return;
		}

		GetPlayerProfileFromBackend(playerUID);
		GetGame().GetCallqueue().CallLater(HandleOnConnect, 10000, false, playerId);
	}
	
	private void GetPlayerProfileFromBackend(string playerUID)
	{
		if (!m_sessionDataGameComp.m_sessionPlayersData.Contains(playerUID))
			m_sessionDataGameComp.m_sessionPlayersData.Insert(playerUID, new KOTH_SessionPlayerData());
		
		if (m_CurrentProfileList.Contains(playerUID))
			return;
		
		string uri = "profile?bohemiaUID="+playerUID;
		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.GET(m_BackendProfileCallBackContext, uri);
	}
	
	void GetPlayerStats(string playerUID)
	{
		string uri = "stats/playerstats?bohemiaUID="+playerUID;
		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.GET(m_PlayerStatsCallback, uri);
	}
	
	void GetBonusCodeForPlayer(string playerUID)
	{
		string uri = "bonus?bohemiaUID="+playerUID;
		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.GET(m_BonusCodeCallback, uri);
	}
	
	void UseBonusCode(string bonusCode, int playerId)
	{
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);
		
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("code", bonusCode);
		saveContext.WriteValue("playerUID", playerUID);
		string data = saveContext.ExportToString();

		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.POST(m_BonusCodeCallback, "bonusCode", "content="+data);		
	}
	
	void DoRpcSyncProfileToPlayer(KOTH_PlayerProfileJson profile)
	{
		if (!profile || !Replication.IsServer())
			return;

		array<int> allPlayers = {};
		GetGame().GetPlayerManager().GetPlayers(allPlayers);
		
		foreach (int playerId : allPlayers)
		{
			string playerUID = KOTH_Helper.GetPlayerUID(playerId);
			if (playerUID == profile.m_playerUID)
			{
				PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
				KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
				profileComp.DoRpc_SyncPlayerProfile(profile);
				break;
			}
		}
	}
	
	void RemoveProfileToCurrentList(string profileUID)
	{
		if (!Replication.IsServer())
			return;

		if (m_CurrentProfileList.Contains(profileUID))
			m_CurrentProfileList.Remove(profileUID);
	}

	void AddProfileToCurrentList(KOTH_PlayerProfileJson profile)
	{
		if (!Replication.IsServer())
			return;

		if (!m_CurrentProfileList.Contains(profile.m_playerUID))
			m_CurrentProfileList.Insert(profile.m_playerUID, profile);
	}
	
	void SendProfilePresetToBackend(KOTH_PlayerPresetJson preset, string playerUID)
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", preset);
		string data = saveContext.ExportToString();

		Log("send this to backend "+data);
		
		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.POST(m_CallBackContext, "preset/"+playerUID, "content="+data);
	}
	
	// -- send profiles to backend
	private void SendListProfilesToBackend(KOTH_ListPlayerProfileJson list)
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", list);
		string data = saveContext.ExportToString();

		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.POST(m_CallBackContext, "profiles", "content="+data);
	}
	
	private void SendProfileToBackend(KOTH_PlayerProfileJson profile)
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", profile);
		string data = saveContext.ExportToString();

		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.POST(m_CallBackContext, "profile", "content="+data);
	}

	private void CheckPlayersBan(KOTH_ListPlayerStatsJson list)
	{
		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.GET(m_checkBanCallBackContext, "activeBans");
	}
	
	// -- send stats to backend
	private void SendPlayersStatsToBackend(KOTH_ListPlayerStatsJson list)
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", list);
		string data = saveContext.ExportToString();

		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.POST(m_CallBackContext, "stats/playersstats", "content="+data);
		GetGame().GetCallqueue().CallLater(CheckPlayersBan, 1000, false, list);
	}
	
	private void SendPlayerStatsToBackend(KOTH_PlayerStatsJson stats)
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", stats);
		string data = saveContext.ExportToString();

		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.POST(m_CallBackContext, "stats/playerstats", "content="+data);
	}
	
	void SendStatVoteMap(string key, int votes)
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("scenario", key);
		saveContext.WriteValue("votes", votes);
		string data = saveContext.ExportToString();

		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.POST(m_CallBackContext, "stats/votemap", "content="+data);	
	}
	void SendStatSelectedVoteMap(string key)
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("scenario", key);
		string data = saveContext.ExportToString();

		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.POST(m_CallBackContext, "stats/votemapwinner", "content="+data);	
	}
	void SendStatTeamWinner(string key)
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("team", key);
		string data = saveContext.ExportToString();

		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.POST(m_CallBackContext, "stats/teamwinner", "content="+data);	
	}

	//---- disconnect 
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		if (!Replication.IsServer())
			return;
		
		m_playerAuditSystem.RemovePlayer(playerId);
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);

		KickCauseGroup2 groupInt;
		int reasonInt;
		string group, reason;
		
		//~ Get the kick reason ID
		GetGame().GetFullKickReason(cause, groupInt, reasonInt, group, reason);

		string playerName = GetGame().GetPlayerManager().GetPlayerName(playerId);
		int currentPlayers = GetGame().GetPlayerManager().GetPlayerCount();
		Log("KOTH_BackendApiGameModeComponent.OnPlayerDisconnected "+playerName+" group: "+group+" reason: "+reason+" totalPlayers: "+currentPlayers.ToString()+" ");
		m_sessionDataGameComp.StopTimeMeasurement(playerUID);
		HandleOnDisconnect(playerUID);
	}
	
	private void HandleOnDisconnect(string profileUID)
	{
		if (!m_CurrentProfileList.Contains(profileUID))
			return;
		
		Log("KOTH_BackendApiGameModeComponent.HandleOnDisconnect "+profileUID);
		OnDisconnectSendProfileToBackend(profileUID);
		GetGame().GetCallqueue().CallLater(HandleOnDisconnect, 10000, false, profileUID);
	}
	
	private void SendDisconnectProfileToBackend(KOTH_PlayerProfileJson profile)
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", profile);
		string data = saveContext.ExportToString();

		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.POST(m_OnDisconnectCallBackContext, "profile", "content="+data);
	}
	
	private void OnDisconnectSendProfileToBackend(string profileUID)
	{
		if (!m_CurrentProfileList.Contains(profileUID))
			return;
		
		KOTH_PlayerProfileJson profile = m_CurrentProfileList.Get(profileUID);
		if (!profile)
			return;

		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", profile);
		string data = saveContext.ExportToString();

		m_RestContext.SetHeaders("X-AUTH-TOKEN,"+m_ApiKey);
		m_RestContext.POST(m_OnDisconnectCallBackContext, "profile", "content="+data);
	}
	
	
	// ---- static helpers
	static int FindPlayerIdFromBohemiaUID(string playerBohemiaUID)
	{
		array<int> allPlayers = {};
		PlayerManager playerManager = GetGame().GetPlayerManager();
		playerManager.GetPlayers(allPlayers);
		
		foreach (int playerId : allPlayers)
		{
			string playerUID = KOTH_Helper.GetPlayerUID(playerId); // its local info so can stupid spam
			if (playerUID == playerBohemiaUID)
				return playerId;
		}
		
		return 0;
	}
	
	private void WB_GetFakeProfile(int playerId)
	{
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);
		string playerName = GetGame().GetPlayerManager().GetPlayerName(playerId);
		KOTH_PlayerProfileJson profile = new KOTH_PlayerProfileJson();
		profile.m_playerUID = playerUID;
		profile.m_playerName = playerName;
		profile.AddLevel(101);
		profile.AddMoney(99999);
		profile.AddXp(1900000);

		KOTH_BackendApiGameModeComponent kothBackendApiGameModeComp = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
		kothBackendApiGameModeComp.AddProfileToCurrentList(profile);
		kothBackendApiGameModeComp.DoRpcSyncProfileToPlayer(profile);
		
		if (!m_sessionDataGameComp.m_sessionPlayersData.Contains(playerUID))
			m_sessionDataGameComp.m_sessionPlayersData.Insert(playerUID, new KOTH_SessionPlayerData());

		if (!m_playerStatsList.Contains(playerUID))
			m_playerStatsList.Insert(playerUID, new KOTH_PlayerStatsJson());
		
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		KOTH_SCR_PlayerProfileComponent playerProfile = KOTH_SCR_PlayerProfileComponent.Cast(pc.FindComponent(KOTH_SCR_PlayerProfileComponent));
		playerProfile.SetPlayerUID(playerUID);
	}
}