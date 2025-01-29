class KOTH_InputCodeBoxUI : ChimeraMenuBase
{
	protected Widget m_wRoot;
	protected KOTH_SCR_PlayerProfileComponent m_playerProfileComp;
	protected int m_playerId;

	override void OnMenuInit()
	{
		super.OnMenuInit();

		PlayerController controller = GetGame().GetPlayerController();
		m_playerId = controller.GetPlayerId();
		m_playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(controller.FindComponent(KOTH_SCR_PlayerProfileComponent));
	}

	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_wRoot = GetRootWidget();
		
		// add listeners
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.GetInputButtonComponent("Cancel", m_wRoot);
		if (cancel) { cancel.m_OnActivated.Insert(OnClickEscape); }
		
		SCR_InputButtonComponent confirm = SCR_InputButtonComponent.GetInputButtonComponent("Confirm", m_wRoot);
		if (confirm) { confirm.m_OnActivated.Insert(OnClickconfirm); }
	}

	protected void OnClickconfirm()
	{
		EditBoxWidget editBox = EditBoxWidget.Cast(m_wRoot.FindAnyWidget("EditBox"));
		Log(editBox.GetText());
		
		if (editBox.GetText() == string.Empty)
			return;
		
		m_playerProfileComp.AskRpc_UseBonusCode(editBox.GetText(), m_playerId);
		GetGame().GetMenuManager().CloseMenu(this);	
	}
	
	protected void OnClickEscape()
	{
		GetGame().GetMenuManager().CloseMenu(this);
	}
}
