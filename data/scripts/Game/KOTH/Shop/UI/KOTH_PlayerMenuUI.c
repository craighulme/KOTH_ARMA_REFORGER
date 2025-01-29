class KOTH_PlayerMenuUI : ChimeraMenuBase
{
	protected KOTH_SCR_PlayerProfileComponent m_playerProfileComp;
	protected string m_playerName;
	protected Widget m_wRoot;
	private ArmaReforgerScripted game = GetGame();
	
	protected ref KOTH_FileManager KOTHFileManager;
	protected HudSettings hudSettings;

	override void OnMenuInit()
	{
		super.OnMenuInit();
	
		PlayerController controller = GetGame().GetPlayerController();
		m_playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(controller.FindComponent(KOTH_SCR_PlayerProfileComponent));
		m_playerName = GetGame().GetPlayerManager().GetPlayerName(controller.GetPlayerId());
	}
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_wRoot = GetRootWidget();
		KOTHFileManager = new KOTH_FileManager();
    	hudSettings = HudSettings.Cast(KOTHFileManager.GetHudSettings());
		
		//set the values on menu open
		game.ShowTeamNames = hudSettings.showTeamNames; 
		game.ShowScoreUI = hudSettings.showScoreUI;
		game.ShowWaypointAO = hudSettings.showWayPointUI;
		
		SCR_ButtonTextComponent scrButtonTitlePlayerUID = SCR_ButtonTextComponent.GetButtonText("ButtonTitlePlayerUID", m_wRoot);
		if (scrButtonTitlePlayerUID) { scrButtonTitlePlayerUID.m_OnClicked.Insert(OnClickPlayerUID); Log("ButtonTitlePlayerUID");}
		SCR_ButtonTextComponent scrButtonValuePlayerUID = SCR_ButtonTextComponent.GetButtonText("ButtonValuePlayerUID", m_wRoot);
		if (scrButtonTitlePlayerUID) { scrButtonTitlePlayerUID.m_OnClicked.Insert(OnClickPlayerUID); Log("ButtonValuePlayerUID"); }
		
		TextWidget txtWdgValuePlayerUID = TextWidget.Cast(m_wRoot.FindAnyWidget("ValuePlayerUID"));
		if (txtWdgValuePlayerUID)
			txtWdgValuePlayerUID.SetText(m_playerProfileComp.GetMyPlayerUID());		
		
		TextWidget playerNameWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("PlayerName"));
		if (playerNameWdg)
			playerNameWdg.SetText(m_playerName);
		
		TextWidget valueMoneyWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("ValueMoney"));
		if (valueMoneyWdg)
			valueMoneyWdg.SetText(m_playerProfileComp.GetMoney().ToString()+" $ ");
		
		TextWidget valueXpWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("ValueXp"));
		if (valueXpWdg)
			valueXpWdg.SetText(m_playerProfileComp.GetXp().ToString());
		
		TextWidget valueLevelWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("ValueLevel"));
		if (valueLevelWdg)
			valueLevelWdg.SetText(m_playerProfileComp.GetLevel().ToString());
		
		TextWidget valueKillsWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("ValueKills"));
		if (valueKillsWdg)
			valueKillsWdg.SetText(m_playerProfileComp.m_kills.ToString());
		
		TextWidget valueDeathsWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("ValueDeaths"));
		if (valueDeathsWdg)
			valueDeathsWdg.SetText(m_playerProfileComp.m_deaths.ToString());
		
		TextWidget valueFriendlyKillsWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("ValueFriendlyKills"));
		if (valueFriendlyKillsWdg)
			valueFriendlyKillsWdg.SetText(m_playerProfileComp.m_friendlyKills.ToString());
		
		float ratio = 0;
		if (m_playerProfileComp.m_deaths > 0)
		{
			ratio = m_playerProfileComp.m_kills / m_playerProfileComp.m_deaths;
		} else {
			ratio = m_playerProfileComp.m_kills;
		}

        // Initialize checkboxes with HudSettings values
        CheckBoxWidget valueShowTeamNames = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("ShowNamesCheckBox"));
        valueShowTeamNames.SetChecked(game.ShowTeamNames);
        SCR_InputButtonComponent btnShowTeamNames = SCR_InputButtonComponent.GetInputButtonComponent("ShowNamesCheckBox", m_wRoot);
        if (btnShowTeamNames) { btnShowTeamNames.m_OnActivated.Insert(OnClickShowTeamCheckbox); }
        
        CheckBoxWidget valueShowScoreUI = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("ShowScoreUICheckBox"));
        valueShowScoreUI.SetChecked(game.ShowScoreUI);
        SCR_InputButtonComponent btnShowScoreUI = SCR_InputButtonComponent.GetInputButtonComponent("ShowScoreUICheckBox", m_wRoot);
        if (btnShowScoreUI) { btnShowScoreUI.m_OnActivated.Insert(OnClickShowScoreUICheckbox); }
        
        CheckBoxWidget valueShowWaypointAO = CheckBoxWidget.Cast(m_wRoot.FindAnyWidget("ShowWaypointAOCheckBox"));
        valueShowWaypointAO.SetChecked(game.ShowWaypointAO);
        SCR_InputButtonComponent btnShowWaypointAO = SCR_InputButtonComponent.GetInputButtonComponent("ShowWaypointAOCheckBox", m_wRoot);
        if (btnShowWaypointAO) { btnShowWaypointAO.m_OnActivated.Insert(OnClickShowWaypointAOCheckbox); }
	        
		TextWidget killDeathRatioWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("ValueKillDeathRatio"));
		if (killDeathRatioWdg)
			killDeathRatioWdg.SetText(ratio.ToString(lenDec: 2));
		
		SCR_InputButtonComponent inputCode = SCR_InputButtonComponent.GetInputButtonComponent("InputCode", m_wRoot);
		if (inputCode) { inputCode.m_OnActivated.Insert(OnClickInputCode); }
		
		// add listeners
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.GetInputButtonComponent("Cancel", m_wRoot);
		if (cancel) { cancel.m_OnActivated.Insert(OnClickEscape); }
		
		if (m_playerProfileComp.GetSessionBonusCode())
			OnSuccessCodeUsage(m_playerProfileComp.GetSessionBonusCode());
	}
	
	protected void OnClickShowTeamCheckbox()
	{
	    if (hudSettings)
	    {
	        hudSettings.showTeamNames = !hudSettings.showTeamNames;
			game.ShowTeamNames = hudSettings.showTeamNames; 
	    }
	}
	
	protected void OnClickShowScoreUICheckbox()
	{
	    if (hudSettings)
	    {
	        hudSettings.showScoreUI = !hudSettings.showScoreUI;
			game.ShowScoreUI = hudSettings.showScoreUI;
	    }
	}
	
	protected void OnClickShowWaypointAOCheckbox()
	{
	    if (hudSettings)
	    {
	        hudSettings.showWayPointUI = !hudSettings.showWayPointUI;
			game.ShowWaypointAO = hudSettings.showWayPointUI;
	    }
	}
	
	protected void OnClickInputCode()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.KOTH_InputCodeBox);
	}
	
	void OnErrorCodeUsage(string error)
	{
		TextWidget errorWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("Error"));
		errorWdg.SetVisible(true);
		errorWdg.SetText(error);
	}
	
	void OnSuccessCodeUsage(KOTH_BonusCodeResponseJson bonusCode)
	{
		TextWidget errorWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("Error"));
		errorWdg.SetVisible(false);

		
		TextWidget titleCodeNameWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("TitleCodeName"));
		titleCodeNameWdg.SetVisible(true);
		TextWidget codeNameWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("CodeName"));
		codeNameWdg.SetVisible(true);
		codeNameWdg.SetText(bonusCode.code);
		
		TextWidget titleMultiplierWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("TitleMultiplier"));
		titleMultiplierWdg.SetVisible(true);
		TextWidget moneyMultiplierWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("MoneyMultiplier"));
		moneyMultiplierWdg.SetVisible(true);
		moneyMultiplierWdg.SetText("X"+bonusCode.multiplier+" $");
		TextWidget xpMultiplierWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("XpMultiplier"));
		xpMultiplierWdg.SetVisible(true);
		xpMultiplierWdg.SetText("X"+bonusCode.multiplier+" xp");
		
		TextWidget titleDateEndWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("TitleDateEnd"));
		titleDateEndWdg.SetVisible(true);
		TextWidget dateEndWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("DateEnd"));
		dateEndWdg.SetVisible(true);
		dateEndWdg.SetText(bonusCode.dateEnd);
	}
	
	protected void OnClickEscape()
	{
		KOTHFileManager.SaveSettings();
		GetGame().GetMenuManager().CloseMenu(this);
	}
	
	protected void OnClickPlayerUID()
	{
		System.ExportToClipboard(m_playerProfileComp.GetMyPlayerUID());
	}
}
