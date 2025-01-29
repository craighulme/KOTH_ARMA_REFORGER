modded class PauseMenuUI: ChimeraMenuBase
{
	int m_RespawnCoolDownTimeSec = 10;
	SCR_ButtonTextComponent m_RespawnBtn;
	SCR_ButtonTextComponent m_FactionBtn;
	SCR_ButtonTextComponent m_FieldManualBtn;
	SCR_ButtonTextComponent m_FeedbackBtn;
	SCR_ButtonTextComponent m_PlayerMenu;
	SCR_ButtonTextComponent m_EditorPhotoOpen;
	

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		// Respawn with cooldown
		m_RespawnBtn = SCR_ButtonTextComponent.GetButtonText("Respawn", m_wRoot);
		if (m_RespawnBtn)
		{
			m_RespawnBtn.SetText("Respawn Cooldown...");
			m_RespawnBtn.SetEnabled(false);
			GetGame().GetCallqueue().CallLater(ActivateRespawnBtn, 16000, false);
			GetGame().GetCallqueue().CallLater(SetRespawnTime, 1000, true);
		}

		m_FactionBtn = SCR_ButtonTextComponent.GetButtonText("LeaveFaction", m_wRoot);
		if (m_FactionBtn)
		{
			BaseGameMode gamemode = GetGame().GetGameMode();
			if (!gamemode)
				return;
			KOTH_ScoringGameModeComponent m_scoreComp = KOTH_ScoringGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_ScoringGameModeComponent));
			if (m_scoreComp.PlayersCanChangeFaction()) {
				m_FactionBtn.GetRootWidget().SetVisible(true);
			} else {
				m_FactionBtn.GetRootWidget().SetVisible(false);
			}
		}

		m_FieldManualBtn = SCR_ButtonTextComponent.GetButtonText("FieldManual", m_wRoot);
		if (m_FieldManualBtn)
		{
			m_FieldManualBtn.GetRootWidget().SetVisible(false);
		}

		m_EditorPhotoOpen = SCR_ButtonTextComponent.GetButtonText("EditorPhotoOpen", m_wRoot);
		if (m_EditorPhotoOpen)
		{
			m_EditorPhotoOpen.GetRootWidget().SetVisible(false);
		}

		m_FeedbackBtn = SCR_ButtonTextComponent.GetButtonText("Feedback", m_wRoot);
		if (m_FeedbackBtn)
		{
			m_FeedbackBtn.GetRootWidget().SetVisible(false);
		}
		
		m_PlayerMenu = SCR_ButtonTextComponent.GetButtonText("PlayerMenu", m_wRoot);
		if (m_PlayerMenu)
		{
			m_PlayerMenu.GetRootWidget().SetVisible(true);
			m_PlayerMenu.m_OnClicked.Insert(OnPlayerMenu);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ActivateRespawnBtn()
	{
		m_RespawnBtn.SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	void SetRespawnTime()
	{
		if (m_RespawnCoolDownTimeSec == 0)
		{
			GetGame().GetCallqueue().Remove(SetRespawnTime);
			m_RespawnBtn.SetText("Respawn");
			m_RespawnBtn.SetEnabled(true);
			return;
		}

		m_RespawnBtn.SetText(string.Format("Respawn in: %1", m_RespawnCoolDownTimeSec));
		m_RespawnCoolDownTimeSec--;
	}
	
	void OnPlayerMenu()
	{
		GetGame().GetMenuManager().CloseAllMenus();
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.KOTH_PlayerMenu);
	}	
}
