class KOTH_SCR_PlayerProfileComponentClass : ScriptComponentClass {}
class KOTH_SCR_PlayerProfileComponent : ScriptComponent 
{
	[RplProp()]
	protected string m_bohemiaUID;
	string GetMyPlayerUID() { return m_bohemiaUID; }
	void SetPlayerUID(string uid) 
	{ 
		m_bohemiaUID = uid; 
		Replication.BumpMe(); 
	}
	
	protected ref KOTH_BonusCodeResponseJson m_sessionBonusCodeResponse;
	KOTH_BonusCodeResponseJson GetSessionBonusCode() { return m_sessionBonusCodeResponse; }
	
	protected int m_sessionMaxKillStreak = 0;
	int GetSessionMaxKillStreak() { return m_sessionMaxKillStreak; }
	
	protected int m_sessionEndGameBonus = 0;
	int GetSessionEndGameBonus() { return m_sessionEndGameBonus; }

	// this get reset with DroppedNearZone
	protected bool m_hasFlushedToilet = false;
	bool GetHasFlushToilet() { return m_hasFlushedToilet; }
	void ResetFlushToilet() { m_hasFlushedToilet = false; }
	void FlushToilet(string bonus)
	{
		if (m_hasFlushedToilet)
			return;

		m_hasFlushedToilet = true;
		Rpc(RpcDo_Notif_FlushToilet, bonus);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_Notif_FlushToilet(string bonus)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifFlushToilet(bonus);
			}
		}
	}

	// re add them and show them in endgame screen ?
//	protected int m_sessionXpEarned = 0;
//	protected int m_sessionMoneyEarned = 0;
//  protected int m_sessionPointsWhenFactionWasJoined = 0;
	
	protected int m_currentKillStreak = 0;
	void ResetKillStreak() { m_currentKillStreak = 0; }
	int GetKillStreak() { return m_currentKillStreak; }
	void AddToKillStreak() { m_currentKillStreak++; }

	protected int m_currentInsertionStreak = 0;
	void ResetInsertionStreak() { m_currentInsertionStreak = 0; }
	int GetInsertionStreak() { return m_currentInsertionStreak; }
	void AddToInsertionStreak() { m_currentInsertionStreak++; }
	
	protected bool m_hasBeenDroppedNearZone = false;
	bool HasBeenDroppedNearZone() { return m_hasBeenDroppedNearZone; }
	void ResetHasBeenDroppedNearZone() { m_hasBeenDroppedNearZone = false; }
	void DroppedNearZone()
	{
		m_hasBeenDroppedNearZone = true;
		GetGame().GetCallqueue().CallLater(ResetHasBeenDroppedNearZone, 60000 * 5);
	}
	
	//track the last time a player respawned or got damaged to handle kill distance bonus bug
	float lastRespawnTime;
	float lastDamageReceivedTime;
	
	//track the last Vehicle a Player Rode In
	Vehicle lastVehicle;
	
	protected ref array<string> m_unlockedItems = {};
	array<string> GetUnlockedItemList() { return m_unlockedItems; }
	
	int m_kills = 0;
	int m_deaths = 0;
	int m_assists = 0;
	int m_friendlyKills = 0;
	
	protected int m_xp = 0;
	protected int m_money = 0;
	protected int m_level = 0;
	int GetXp() { return m_xp; }
	int GetMoney() { return m_money; }
	int GetLevel() { return m_level; }

	// ----
	void DoRpc_SyncPlayerProfile(KOTH_PlayerProfileJson profile)
	{
		Rpc(RpcDo_SyncPlayerProfile, profile.m_unlockedItems, profile.GetMoney(), profile.GetLevel(), profile.GetXp(), profile.m_kills, profile.m_deaths, profile.m_friendlyKills);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_SyncPlayerProfile(array<string> unlockedItems, int money, int level, int xp, int kills, int deaths, int friendlyKills)
	{
		m_unlockedItems = unlockedItems;
		m_money = money;
		m_level = level;
		m_xp = xp;
		m_kills = kills;
		m_deaths = deaths;
		m_friendlyKills = friendlyKills;
	}

	// -------- ProfileData --------
	void DoRpc_FindLocalProfileDatas()
	{
		Rpc(RpcDo_FindLocalProfileDatas);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_FindLocalProfileDatas()
	{
		Rpc(RpcDo_SendLocalProfileDatasToServer, SCR_Global.GetProfileName(), System.GetMachineName(), System.GetAdapterName());
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_SendLocalProfileDatasToServer(string profileName, string machineName, string adapterName)
	{
		KOTH_BackendApiGameModeComponent kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
		KOTH_PlayerProfileJson profile = kothBackendApi.m_CurrentProfileList.Get(GetMyPlayerUID());
		if (!profile)
		{
			PlayerController pc = PlayerController.Cast(GetOwner());
			if (!pc)
				return;

			int playerId = pc.GetPlayerId();
			if (playerId <= 0)
				return;

			if (!GetGame().GetPlayerManager().IsPlayerConnected(playerId))
				return;

			Log("profile not found for playerId "+playerId+" profileName "+profileName+" machineName "+machineName+" adapterName"+adapterName, LogLevel.ERROR);
			GetGame().GetCallqueue().CallLater(RpcDo_SendLocalProfileDatasToServer, 10000, false, profileName, machineName, adapterName);
			return;
		}
		
		profile.m_profileName = profileName;
		profile.m_machineName = machineName;
		profile.m_adapterName = adapterName;
	}
	
	// -------- Notifications --------
	void DoRpc_NotifCapture(string bonus)
	{
		Rpc(RpcDo_NotifCapture, bonus);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_NotifCapture(string bonus)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifCapture(bonus);
			}
		}
	}
	
	// ----
	void DoRpc_NotifCapturePriorityArea(string bonus)
	{
		Rpc(RpcDo_NotifCapturePriorityArea, bonus);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_NotifCapturePriorityArea(string bonus)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifCapturePriorityArea(bonus);
			}
		}
	}
	
	// ----
	void DoRpc_Notif_KillStreak(int nbKills, int bonus)
	{
		Rpc(RpcDo_Notif_KillStreak, nbKills, bonus);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_Notif_KillStreak(int nbKills, int bonus)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifKillStreak(nbKills, bonus);
			}
		}
	}
	
	// ----
	void DoRpc_Notif_InsertionStreak(int nbInsertions, int bonus)
	{
		Rpc(RpcDo_Notif_InsertionStreak, nbInsertions, bonus);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_Notif_InsertionStreak(int nbInsertions, int bonus)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifInsertionStreak(nbInsertions, bonus);
			}
		}
	}
	
	// ----
	void DoRpc_Notif_KillDistance(int killDistanceBonus, int killDistance)
	{
		Rpc(RpcDo_Notif_KillDistance, killDistanceBonus, killDistance);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_Notif_KillDistance(int killDistanceBonus, int killDistance)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifKillDistance(killDistance);
			}
		}
	}
	
	// ----
	void DoRpc_Notif_EnemyKill(string bonus)
	{
		Rpc(RpcDo_Notif_EnemyKill, bonus);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_Notif_EnemyKill(string bonus)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifEnemyKill(bonus);
			}
		}
	}
	
	// ----
	void DoRpc_Notif_EnemyKillAssist(string bonus)
	{
		Rpc(RpcDo_Notif_EnemyKillAssist, bonus);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_Notif_EnemyKillAssist(string bonus)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifEnemyKillAssist(bonus);
			}
		}
	}
	
	// ----
	void DoRpc_Notif_EnemyVehicleDestroy(string bonus, string vehicleType, string vehiclePart)
	{
		Rpc(RpcDo_Notif_EnemyVehicleDestroy, bonus, vehicleType, vehiclePart);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_Notif_EnemyVehicleDestroy(string bonus, string vehicleType, string vehiclePart)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifEnemyVehicleDestroyed(bonus, vehicleType, vehiclePart);
			}
		}
	}
	
	// ----
	void DoRpc_Notif_EnemyVehicleAssist(string bonus, string vehicleType, string vehiclePart)
	{
		Rpc(RpcDo_Notif_EnemyVehicleAssist, bonus, vehicleType, vehiclePart);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_Notif_EnemyVehicleAssist(string bonus, string vehicleType, string vehiclePart)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifEnemyVehicleAssist(bonus, vehicleType, vehiclePart);
			}
		}
	}
	
	// ----
	void DoRpc_Notif_FriendlyVehicleDestroy(string vehicleType, string vehiclePart)
	{
		Rpc(RpcDo_Notif_FriendlyVehicleDestroy, vehicleType, vehiclePart);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_Notif_FriendlyVehicleDestroy(string vehicleType, string vehiclePart)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifFriendlyVehicleDestroyed(vehicleType, vehiclePart);
			}
		}
	}
	
	// ----
	void DoRpc_Notif_FriendlyKill(bool isHeloTK = false)
	{
		Rpc(RpcDo_Notif_FriendlyKill, isHeloTK);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_Notif_FriendlyKill(bool isHeloTK)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifFriendlyKill(isHeloTK);
			}
		}
	}
	
	// ----
	void DoRpc_NotifDropFriendly(string bonus)
	{
		Rpc(RpcDo_NotifDropFriendly, bonus);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_NotifDropFriendly(string bonus)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifDropFriendly(bonus);
			}
		}
	}
	
	// ----
	void DoRpc_NotifReviveFriendly(string bonus)
	{
		Rpc(RpcDo_NotifReviveFriendly, bonus);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_NotifReviveFriendly(string bonus)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifReviveFriendly(bonus);
			}
		}
	}
	
	// ----
	void DoRpc_NotifRefundVehicleEndGame(string bonus)
	{
		Rpc(RpcDo_NotifRefundVehicleEndGame, bonus);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_NotifRefundVehicleEndGame(string bonus)
	{
		GetGame().GetCallqueue().CallLater(ShowRefund, 500, false, bonus);
	}

	int triesToShowRefund = 0;
	void ShowRefund(string bonus)
	{
		if (triesToShowRefund >= 100)
			return;

		MenuBase menu = ChimeraMenuBase.CurrentChimeraMenu();
		GameOverScreenInput gameOver = GameOverScreenInput.Cast(menu);
		if (!gameOver) 
		{
			triesToShowRefund++;
			Log("triesToShowRefund "+triesToShowRefund);
			GetGame().GetCallqueue().CallLater(ShowRefund, 500, false, bonus);
			return;
		}
		
		gameOver.ShowRefund(bonus);
	}

	// ----
	void DoRpc_SetEndGameBonus(int bonus)
	{
		Rpc(RpcDo_SetEndGameBonus, bonus);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_SetEndGameBonus(int bonus)
	{
		m_sessionEndGameBonus = bonus;
	}
	
	// ---- RepairZone
	void DoRpc_ShowStartRepair()
	{
		Rpc(RpcDo_ShowStartRepair);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_ShowStartRepair()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget repairWidget = workspace.FindAnyWidget("repairZoneRootFrame");
		if (repairWidget) {
			repairWidget.SetVisible(true);
			ProgressBarWidget pBar = ProgressBarWidget.Cast(repairWidget.FindAnyWidget("ProgressBar"));
			if (pBar)
				pBar.SetCurrent(0);
		} else {
			workspace.CreateWidgets("{EFA5D2251372DFB3}UI/Layouts/RepairZone.layout");
		}
		GetGame().GetCallqueue().CallLater(AddProgress, KOTH_RepairZone.TICK_RATE * 1000, true);
	}
	
	void AddProgress()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget repairWidget = workspace.FindAnyWidget("repairZoneRootFrame");
		if (repairWidget) {
			ProgressBarWidget pBar = ProgressBarWidget.Cast(repairWidget.FindAnyWidget("ProgressBar"));
			if (pBar) {
				if (pBar.GetCurrent() >= 100) {
					repairWidget.SetVisible(false);
					GetGame().GetCallqueue().Remove(AddProgress);
				} else {
					float progressPerTick = 100 / KOTH_RepairZone.TIMER_TO_REPAIR;
					pBar.SetCurrent(pBar.GetCurrent() + (progressPerTick * KOTH_RepairZone.TICK_RATE));
				}
			}
		}
	}
	
	void DoRpc_StopRepair()
	{
		Rpc(RpcDo_StopRepair);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_StopRepair()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget repairWidget = workspace.FindAnyWidget("repairZoneRootFrame");
		if (repairWidget) {
			repairWidget.SetVisible(false);
			ProgressBarWidget pBar = ProgressBarWidget.Cast(repairWidget.FindAnyWidget("ProgressBar"));
			if (pBar)
				pBar.SetCurrent(0);
		}
	}

	void DoRpc_ShowVehicleRearm()
	{
		Rpc(RpcDo_ShowVehicleRearm);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_ShowVehicleRearm()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.KOTH_ShopVehicleRearm);
	}
	
	// ---- VoteMap
	// -- sendChoices
	ref array<ResourceName> m_mapList;
	void DoRpc_SendMapChoices(array<ResourceName> mapList)
	{
		Rpc(RpcDo_SendMapChoices, mapList);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_SendMapChoices(array<ResourceName> mapList)
	{
		m_mapList = mapList;
		GetGame().GetCallqueue().CallLater(KOTH_VoteMapUI.ShowVoteMap, 5000, false);
	}

	// -- sendUserVote
	void AskRpc_SendVote(string mapChoice)
	{
		Rpc(RpcAsk_SendVote, mapChoice);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_SendVote(string mapChoice)
	{
		int playerId = PlayerController.Cast(GetOwner()).GetPlayerId();
		KOTH_ScoringGameModeComponent m_scoringGameComp = KOTH_ScoringGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_ScoringGameModeComponent));
		m_scoringGameComp.PlayerVote(playerId, mapChoice);
	}
	
	// -- request
	void AskRpc_UseBonusCode(string bonusCode, int playerId)
	{
		Rpc(RpcAsk_UseBonusCode, bonusCode, playerId);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_UseBonusCode(string bonusCode, int playerId)
	{
		KOTH_BackendApiGameModeComponent m_kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
		m_kothBackendApi.UseBonusCode(bonusCode, playerId);
	}
	
	// -- response
	void DoRpc_ShowErrorBonusCode(string errorReason)
	{
		Rpc(RpcDo_ShowErrorBonusCode, errorReason);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_ShowErrorBonusCode(string errorReason)
	{
		KOTH_PlayerMenuUI playerMenuUI = KOTH_PlayerMenuUI.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.KOTH_PlayerMenu));
		if (playerMenuUI)
			playerMenuUI.OnErrorCodeUsage(errorReason);
	}
	void DoRpc_ShowSuccessBonusCode(KOTH_BonusCodeResponseJson bonusCodeJson)
	{
		Rpc(RpcDo_ShowSuccessBonusCode, bonusCodeJson.code, bonusCodeJson.multiplier.ToFloat(), bonusCodeJson.name, bonusCodeJson.dateEnd);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_ShowSuccessBonusCode(string code, float multiplier, string name, string dateEnd)
	{
		if (!m_sessionBonusCodeResponse)
			m_sessionBonusCodeResponse = new KOTH_BonusCodeResponseJson();

		m_sessionBonusCodeResponse.code = code;
		m_sessionBonusCodeResponse.multiplier = multiplier.ToString();
		m_sessionBonusCodeResponse.name = name;
		m_sessionBonusCodeResponse.dateEnd = dateEnd;
	
		KOTH_PlayerMenuUI playerMenuUI = KOTH_PlayerMenuUI.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.KOTH_PlayerMenu));
		if (playerMenuUI)
			playerMenuUI.OnSuccessCodeUsage(m_sessionBonusCodeResponse);
	}
}