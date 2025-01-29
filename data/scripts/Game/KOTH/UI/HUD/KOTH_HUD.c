class KOTH_HUD : SCR_InfoDisplay
{
	TextWidget m_blueforPointsText;
	TextWidget m_greenforPointsText;
	TextWidget m_redforPointsText;

	TextWidget m_blueforPlayersText;
	TextWidget m_greenforPlayersText;
	TextWidget m_redforPlayersText;

	TextWidget m_moneyText;
	TextWidget m_lvlText;
	TextWidget m_xpText;
	SCR_WLibProgressBarComponent m_xpProgressBar;
	
	ImageWidget m_bluforGlow;
	ImageWidget m_redforGlow;
	ImageWidget m_indforGlow;

	private ArmaReforgerScripted m_game = GetGame();
	KOTH_ScoringGameModeComponent m_scoreComp;
	KOTH_SCR_PlayerProfileComponent m_playerProfileComp;
	
	WorldTimestamp m_lastTimePlayedAudio;
	
	Widget m_root;

	Widget m_HorizontalLayoutPpl;
	Widget m_HorizontalLayoutMoney;
	Widget m_HorizontalLayoutXpLVL;
	Widget m_HorizontalLayoutFondPoint;
	Widget m_Demi_Front;
	Widget m_Front;
	
	int m_playerId;

	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		m_root = GetRootWidget();
		if (!m_root)
			return;
		
		OverlayWidget koth_hub = OverlayWidget.Cast(m_root.FindWidget("OverlayRoot.VerticalLayoutRoot.HorizontalLayoutRoot.KOTH_HUD"));
		if (!koth_hub)
			return;
	
		m_playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(GetGame().GetPlayerController().FindComponent(KOTH_SCR_PlayerProfileComponent));

		// points
		m_blueforPointsText = TextWidget.Cast(koth_hub.FindWidget("Front.CountPoint_Footer.BlueforPoints"));
		m_greenforPointsText = TextWidget.Cast(koth_hub.FindWidget("Front.CountPoint_Footer.GreenforPoints"));
		m_redforPointsText = TextWidget.Cast(koth_hub.FindWidget("Front.CountPoint_Footer.RedforPoints"));

		// players
		m_blueforPlayersText = TextWidget.Cast(koth_hub.FindWidget("Front.CountPlayer_Footer.BlueforPlayers"));
		m_greenforPlayersText = TextWidget.Cast(koth_hub.FindWidget("Front.CountPlayer_Footer.GreenforPlayers"));
		m_redforPlayersText = TextWidget.Cast(koth_hub.FindWidget("Front.CountPlayer_Footer.RedforPlayers"));

		// money/xp/lvl
		SizeLayoutWidget m_xpSizeLayout = SizeLayoutWidget.Cast(koth_hub.FindWidget("Back.HorizontalLayoutXpLVL.ProgressBar_EXP"));
		m_xpProgressBar = SCR_WLibProgressBarComponent.Cast(m_xpSizeLayout.FindHandler(SCR_WLibProgressBarComponent));

		m_xpText = TextWidget.Cast(koth_hub.FindWidget("Front.EXPERIENCE_Footer.Exp"));
		m_lvlText = TextWidget.Cast(koth_hub.FindWidget("Demi_Front.Demi_EXPERIENCE_Footer.Level"));
		m_moneyText = TextWidget.Cast(koth_hub.FindWidget("Back.HorizontalLayoutMoney.Money"));
		
		m_bluforGlow = ImageWidget.Cast(koth_hub.FindAnyWidget("BluforGlowCapture"));
		m_redforGlow = ImageWidget.Cast(koth_hub.FindAnyWidget("RedforGlowCapture"));
		m_indforGlow = ImageWidget.Cast(koth_hub.FindAnyWidget("IndforGlowCapture"));

		BaseGameMode gamemode = GetGame().GetGameMode();
		if (!gamemode)
			return;
		m_scoreComp = KOTH_ScoringGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_ScoringGameModeComponent));
		
		
		m_playerId = GetGame().GetPlayerController().GetPlayerId();
		
		m_HorizontalLayoutPpl = m_root.FindAnyWidget("HorizontalLayoutPpl");
		m_HorizontalLayoutMoney = m_root.FindAnyWidget("HorizontalLayoutMoney");
		m_HorizontalLayoutXpLVL = m_root.FindAnyWidget("HorizontalLayoutXpLVL");
		m_HorizontalLayoutFondPoint = m_root.FindAnyWidget("HorizontalLayoutFondPoint");
		m_Demi_Front = m_root.FindAnyWidget("Demi_Front");
		m_Front = m_root.FindAnyWidget("Front");
		
		GetGame().GetCallqueue().CallLater(SetKothVersion, 2000, false);
	}
	
	void SetKothVersion()
	{
		WorkshopApi wsApi = GetGame().GetBackendApi().GetWorkshop();
		wsApi.ScanOfflineItems();
		array<WorkshopItem> rawWorkshopItems = {};
		wsApi.GetOfflineItems(rawWorkshopItems);

		string version = "WORKBENCH";
		foreach(WorkshopItem item : rawWorkshopItems)
		{
			if (item.Id() == "5E055A77849516EF")
			{
				version = item.GetActiveRevision().GetVersion();
				break;
			}
		}
		
		TextWidget versionText = TextWidget.Cast(m_root.FindAnyWidget("KothVersion"));
		if (versionText)
			versionText.SetText("KOTH Reforged: v"+version);
	}
	
	
	void PlaySuccessNotificationSound()
	{
		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp timestamp = world.GetLocalTimestamp();
		if (m_lastTimePlayedAudio)
		{
			if (timestamp.DiffMilliseconds(m_lastTimePlayedAudio) > 2000)
			{
				AudioSystem.PlaySound("{E3E34E1B0BC94432}Sounds/UI/Samples/Menu/UI_Task_Succeded.wav");
				m_lastTimePlayedAudio = timestamp;
			}
		}
		else
		{
			AudioSystem.PlaySound("{E3E34E1B0BC94432}Sounds/UI/Samples/Menu/UI_Task_Succeded.wav");
			m_lastTimePlayedAudio = timestamp;
		}
	}
	
	void NotifFlushToilet(string bonus)
	{
		PlaySuccessNotificationSound();
		Notif("Toilet Flushed", ""+bonus+" xp", ""+bonus+" $", 5000);
	}
	void NotifKillDistance(int bonus)
	{
		Notif(""+bonus+"m kill !", ""+bonus+" xp", ""+bonus+" $", 20000);
	}
	void NotifKillStreak(int nbKills, int bonus)
	{
		Notif(""+nbKills+"x KILL STREAK ! ", ""+bonus+" xp", ""+bonus+" $", 20000);
	}
	void NotifInsertionStreak(int nbInsertions, int bonus)
	{
		Notif(""+nbInsertions+"x INSERTION STREAK ! ", ""+bonus+" xp", ""+bonus+" $", 20000);
	}
	void NotifEnemyKill(string bonus)
	{
		PlaySuccessNotificationSound();
		Notif("Enemy killed", ""+bonus+" xp", ""+bonus+" $", 5000);
	}
	void NotifEnemyKillAssist(string bonus)
	{
		PlaySuccessNotificationSound();
		Notif("Enemy kill assist", ""+bonus+" xp", ""+bonus+" $", 5000);
	}
	
	void NotifEnemyVehicleDestroyed(string bonus, string vehicleType, string vehiclePart)
	{
		PlaySuccessNotificationSound();
		Notif("Vehicle " + vehicleType + " " + vehiclePart + " Destroyed", ""+bonus+" xp", ""+bonus+" $", 5000);
	}
	
	void NotifEnemyVehicleAssist(string bonus, string vehicleType, string vehiclePart)
	{
		PlaySuccessNotificationSound();
		Notif("Vehicle " + vehicleType + " " + vehiclePart + " Destroy Assist", ""+bonus+" xp", ""+bonus+" $", 5000);
	}
	
	void NotifFriendlyVehicleDestroyed(string vehicleType, string vehiclePart)
	{
		VerticalLayoutWidget koth_scrollList = VerticalLayoutWidget.Cast(m_root.FindWidget("OverlayRoot.VerticalLayoutRoot.ScrollList.NotifContainer"));
		Widget w = GetGame().GetWorkspace().CreateWidgets("{74686613FDE00759}UI/Layouts/HUD/KingOfTheHill/KOTH_Notification.layout", koth_scrollList);

		TextWidget TextNotif = TextWidget.Cast(w.FindAnyWidget("TextNotif"));
		TextNotif.SetText("Friendly " + vehicleType + " " + vehiclePart + " Destroyed");
		
		int amount = KOTH_Globals.XP_PER_KILLFRIEND_VEHICLE_PART;

		TextWidget XpNotif = TextWidget.Cast(w.FindAnyWidget("XpNotif"));
		XpNotif.SetText("- "+amount+" xp");
		XpNotif.SetColor(Color.DarkRed);

		TextWidget MoneyNotif = TextWidget.Cast(w.FindAnyWidget("MoneyNotif"));
		MoneyNotif.SetText("- "+amount+" $");
		MoneyNotif.SetColor(Color.Red);

		SCR_FadeUIComponent compFade = SCR_FadeUIComponent.Cast(w.FindHandler(SCR_FadeUIComponent));
		compFade.DelayedFadeOut(2000, true);
	}
	
	void NotifCapture(string bonus)
	{
		Notif("Objective offensive", ""+bonus+" xp", ""+bonus+" $");
	}
	void NotifCapturePriorityArea(string bonus)
	{
		Notif("Priority Area Bonus", ""+bonus+" xp", ""+bonus+" $");
	}
	
	void NotifDropFriendly(string bonus)
	{
		Notif("Tactical Insertion", ""+bonus+" xp", ""+bonus+" $");
	}
	void NotifReviveFriendly(string bonus)
	{
		Notif("Revive friendly", ""+bonus+" xp", ""+bonus+" $");
	}
	
	
	void Notif(string textNotifContent, string xpNotifContent, string moneyNotifContent, int delay = 2000)
	{
		VerticalLayoutWidget koth_scrollList = VerticalLayoutWidget.Cast(m_root.FindWidget("OverlayRoot.VerticalLayoutRoot.ScrollList.NotifContainer"));

		Widget w = GetGame().GetWorkspace().CreateWidgets("{74686613FDE00759}UI/Layouts/HUD/KingOfTheHill/KOTH_Notification.layout", koth_scrollList);

		TextWidget TextNotif = TextWidget.Cast(w.FindAnyWidget("TextNotif"));
		TextNotif.SetText(textNotifContent);

		TextWidget XpNotif = TextWidget.Cast(w.FindAnyWidget("XpNotif"));
		XpNotif.SetText(xpNotifContent);

		TextWidget MoneyNotif = TextWidget.Cast(w.FindAnyWidget("MoneyNotif"));
		MoneyNotif.SetText(moneyNotifContent);

		SCR_FadeUIComponent compFade = SCR_FadeUIComponent.Cast(w.FindHandler(SCR_FadeUIComponent));
		compFade.DelayedFadeOut(delay, true);
	}

	void NotifFriendlyKill(bool isHeloTK = false)
	{
		VerticalLayoutWidget koth_scrollList = VerticalLayoutWidget.Cast(m_root.FindWidget("OverlayRoot.VerticalLayoutRoot.ScrollList.NotifContainer"));
		Widget w = GetGame().GetWorkspace().CreateWidgets("{74686613FDE00759}UI/Layouts/HUD/KingOfTheHill/KOTH_Notification.layout", koth_scrollList);

		TextWidget TextNotif = TextWidget.Cast(w.FindAnyWidget("TextNotif"));
		TextNotif.SetText("Friendly killed");
		
		
		int amount = KOTH_Globals.XP_PER_KILLFRIEND;
		if (isHeloTK)
			amount = amount / 2;

		TextWidget XpNotif = TextWidget.Cast(w.FindAnyWidget("XpNotif"));
		XpNotif.SetText("- "+amount+" xp");
		XpNotif.SetColor(Color.DarkRed);

		TextWidget MoneyNotif = TextWidget.Cast(w.FindAnyWidget("MoneyNotif"));
		MoneyNotif.SetText("- "+amount+" $");
		MoneyNotif.SetColor(Color.Red);

		SCR_FadeUIComponent compFade = SCR_FadeUIComponent.Cast(w.FindHandler(SCR_FadeUIComponent));
		compFade.DelayedFadeOut(2000, true);
	}

	void NotifBuy(int amount)
	{
		VerticalLayoutWidget koth_scrollList = VerticalLayoutWidget.Cast(m_root.FindWidget("OverlayRoot.VerticalLayoutRoot.ScrollList.NotifContainer"));

		Widget w = GetGame().GetWorkspace().CreateWidgets("{74686613FDE00759}UI/Layouts/HUD/KingOfTheHill/KOTH_Notification.layout", koth_scrollList);

		TextWidget TextNotif = TextWidget.Cast(w.FindAnyWidget("TextNotif"));
		TextNotif.SetText("Baguette tax");

		TextWidget XpNotif = TextWidget.Cast(w.FindAnyWidget("XpNotif"));
		XpNotif.SetVisible(false);

		TextWidget MoneyNotif = TextWidget.Cast(w.FindAnyWidget("MoneyNotif"));
		MoneyNotif.SetText("—"+amount+"$");
		MoneyNotif.SetColor(Color.Red);

		SCR_FadeUIComponent compFade = SCR_FadeUIComponent.Cast(w.FindHandler(SCR_FadeUIComponent));
		compFade.DelayedFadeOut(5000, true);
	}
	

	protected override event void UpdateValues(IEntity owner, float timeSlice)
	{
		super.UpdateValues(owner, timeSlice);
		if (!m_playerProfileComp)
			return;
		
		//no need to update ScoreUI if its disabled
		if (!m_game.ShowScoreUI)
		{
			m_HorizontalLayoutPpl.SetVisible(false);
			m_HorizontalLayoutMoney.SetVisible(false);
			m_HorizontalLayoutXpLVL.SetVisible(false);
			m_HorizontalLayoutFondPoint.SetVisible(false);
			m_Demi_Front.SetVisible(false);
			m_Front.SetVisible(false);
			return;
		} else {
			m_HorizontalLayoutPpl.SetVisible(true);
			m_HorizontalLayoutMoney.SetVisible(true);
			m_HorizontalLayoutXpLVL.SetVisible(true);
			m_HorizontalLayoutFondPoint.SetVisible(true);
			m_Demi_Front.SetVisible(true);
			m_Front.SetVisible(true);
		}
		
		int nextLevelXPNeeded = KOTH_Globals.GetXpNextLevel(m_playerProfileComp.GetLevel());
		m_moneyText.SetText(m_playerProfileComp.GetMoney().ToString() + " $");
		m_xpText.SetText(m_playerProfileComp.GetXp().ToString() + " / " + nextLevelXPNeeded.ToString());
		m_lvlText.SetText(m_playerProfileComp.GetLevel().ToString());
		m_xpProgressBar.SetValue(m_playerProfileComp.GetXp() / nextLevelXPNeeded, true);
	
		
		// teamPoints
		m_blueforPointsText.SetText(m_scoreComp.GetBlueforPoint().ToString());
		m_greenforPointsText.SetText(m_scoreComp.GetGreenforPoint().ToString());
		m_redforPointsText.SetText(m_scoreComp.GetRedforPoint().ToString());
		
		// teamPlayers
		m_blueforPlayersText.SetText(m_scoreComp.GetBluePlayers().ToString());
		m_greenforPlayersText.SetText(m_scoreComp.GetGreenPlayers().ToString());
		m_redforPlayersText.SetText(m_scoreComp.GetRedPlayers().ToString());
		
		m_bluforGlow.SetVisible(false);
		m_redforGlow.SetVisible(false);
		m_indforGlow.SetVisible(false);
		switch (m_scoreComp.GetCurrentFactionCapturing())
		{
			case KOTH_Faction.BLUFOR:
				m_bluforGlow.SetVisible(true);
			break;
			case KOTH_Faction.OPFOR:
				m_redforGlow.SetVisible(true);
			break;
			case KOTH_Faction.INDFOR:
				m_indforGlow.SetVisible(true);
			break;
			default:
			break;
		}
	}
}
