modded enum SCR_DebugMenuID
{
	KOTH_MENU,
	KOTH_SHOW_PLAYER_PANEL,
	KOTH_SHOW_GAMEMODE_PANEL
}

class KOTH_DebugMenu
{
	static const string DEBUG_MENU_NAME = "KOTH";
	
	static void Init()
	{
		DiagMenu.RegisterMenu(SCR_DebugMenuID.KOTH_MENU, DEBUG_MENU_NAME, "");
		
		DiagMenu.RegisterBool(SCR_DebugMenuID.KOTH_SHOW_PLAYER_PANEL, "", "Player Panel", DEBUG_MENU_NAME);
		DiagMenu.RegisterBool(SCR_DebugMenuID.KOTH_SHOW_GAMEMODE_PANEL, "", "GameMode Panel", DEBUG_MENU_NAME);
	}

	static void DrawGameModePanel()
	{
		DbgUI.Begin("KOTH GameMode Panel");
		PlayerController playerController = GetGame().GetPlayerController();
		if (playerController)
			int playerID = playerController.GetPlayerId();
		
		KOTH_ScoringGameModeComponent m_scoreComp = KOTH_ScoringGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_ScoringGameModeComponent));
			
		DbgUI.Text(KOTH_Faction.BLUFOR);
		if (DbgUI.Button("Add 1 point to "+KOTH_Faction.BLUFOR))
		{
			m_scoreComp.AddBlueforPoint();
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_PICK_UP);
		}
		if (DbgUI.Button("Add 10 point to "+KOTH_Faction.BLUFOR))
		{
			m_scoreComp.AddBlueforPoint(10);
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_PICK_UP);
		}
		if (DbgUI.Button("remove 1 point to blufor"))
		{
			m_scoreComp.RemoveBlueforPoint();
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_PICK_UP);
		}
		if (DbgUI.Button("remove 10 point to blufor"))
		{
			m_scoreComp.RemoveBlueforPoint(10);
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_PICK_UP);
		}
		
		DbgUI.Text("REDFOR");
		if (DbgUI.Button("Add 1 point to redfor"))
		{
			m_scoreComp.AddRedforPoint();
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_PICK_UP);
		}
		if (DbgUI.Button("Add 10 point to redfor"))
		{
			m_scoreComp.AddRedforPoint(10);
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_PICK_UP);
		}
		if (DbgUI.Button("remove 1 point to redfor"))
		{
			m_scoreComp.RemoveRedforPoint();
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_PICK_UP);
		}
		if (DbgUI.Button("remove 10 point to redfor"))
		{
			m_scoreComp.RemoveRedforPoint(10);
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_PICK_UP);
		}
		
		DbgUI.Text("INDFOR");
		if (DbgUI.Button("Add 1 point to indfor"))
		{
			m_scoreComp.AddGreenforPoint();
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_PICK_UP);
		}
		if (DbgUI.Button("Add 10 point to indfor"))
		{
			m_scoreComp.AddGreenforPoint(10);
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_PICK_UP);
		}
		if (DbgUI.Button("remove 1 point to indfor"))
		{
			m_scoreComp.RemoveGreenforPoint();
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_PICK_UP);
		}
		if (DbgUI.Button("remove 10 point to indfor"))
		{
			m_scoreComp.RemoveGreenforPoint(10);
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_PICK_UP);
		}
		
		DbgUI.End();
	}
	
	static void DrawPlayerPanel()
	{
		DbgUI.Begin("KOTH Player Panel");
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		int playerId = playerController.GetPlayerId();
		KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
		KOTH_BackendApiGameModeComponent m_kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);
		KOTH_PlayerProfileJson profile = m_kothBackendApi.m_CurrentProfileList.Get(playerUID);
	
		if (DbgUI.Button("Add me 100$"))
		{
			profile.AddMoney(100);
			profileComp.DoRpc_SyncPlayerProfile(profile);
		}
		if (DbgUI.Button("Remove me 100$"))
		{
			profile.RemoveMoney(100);
			profileComp.DoRpc_SyncPlayerProfile(profile);
		}
		
		if (DbgUI.Button("Add me 1000$"))
		{
			profile.RemoveMoney(1000);
			profileComp.DoRpc_SyncPlayerProfile(profile);
		}
		if (DbgUI.Button("Remove me 1000$"))
		{
			profile.RemoveMoney(1000);
			profileComp.DoRpc_SyncPlayerProfile(profile);
		}
		if (DbgUI.Button("Add me 10000$"))
		{
			profile.AddMoney(1000);
			profileComp.DoRpc_SyncPlayerProfile(profile);
		}
		if (DbgUI.Button("Remove me 10000$"))
		{
			profile.RemoveMoney(10000);
			profileComp.DoRpc_SyncPlayerProfile(profile);
		}
		
		if (DbgUI.Button("Add me 1LVL"))
		{
			profile.AddLevel(1);
			profileComp.DoRpc_SyncPlayerProfile(profile);
		}
		if (DbgUI.Button("Remove me 1LVL"))
		{
			profile.RemoveLevel(1);
			profileComp.DoRpc_SyncPlayerProfile(profile);
		}
		
		if (DbgUI.Button("Add me 10LVL"))
		{
			profile.AddLevel(10);
			profileComp.DoRpc_SyncPlayerProfile(profile);
		}
		
		DbgUI.End();
	}
	
	static void UpdateMenus()
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.KOTH_SHOW_PLAYER_PANEL))
			DrawPlayerPanel();
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.KOTH_SHOW_GAMEMODE_PANEL))
			DrawGameModePanel();
	}
}