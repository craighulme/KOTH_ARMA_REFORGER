class KOTH_ShopVehicleRearmUI : ChimeraMenuBase
{
	protected Widget m_wRoot;
	protected VerticalLayoutWidget m_contentContainer;
	
	protected int m_playerId;
	protected KOTH_SCR_PlayerShopComponent m_playerShopComp;
	
	override void OnMenuInit()
	{
		super.OnMenuInit();
		
		PlayerController controller = GetGame().GetPlayerController();
		m_playerId = controller.GetPlayerId();
		m_playerShopComp = KOTH_SCR_PlayerShopComponent.Cast(controller.FindComponent(KOTH_SCR_PlayerShopComponent));
	}
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_wRoot = GetRootWidget();

		m_contentContainer = VerticalLayoutWidget.Cast(m_wRoot.FindAnyWidget("NotifContainer"));
		ClearItemList();
		
		// add listeners
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.GetInputButtonComponent("Cancel", m_wRoot);
		if (cancel) { cancel.m_OnActivated.Insert(OnActivateEscape); }
		
		SCR_InputButtonComponent confirm = SCR_InputButtonComponent.GetInputButtonComponent("Confirm", m_wRoot);
		if (confirm) { confirm.m_OnActivated.Insert(OnActivateConfirm); }

		int totalPrice = KOTH_Helper.ComputeVehicleRearmPrice(GetGame().GetPlayerController().GetPlayerId());
		AddTotalLine(totalPrice);
	}

	protected void AddTotalLine(int total)
	{
		Widget child = m_contentContainer.GetChildren();
		if (child)
			m_contentContainer.RemoveChild(child);

		Widget newRow = GetGame().GetWorkspace().CreateWidgets("{58A2DBEC3607A192}UI/Layouts/Shop/SessionLoadout_ItemLine.layout", m_contentContainer);
		TextWidget nameTextWdg = TextWidget.Cast(newRow.FindAnyWidget("Name"));
		if (nameTextWdg)
			nameTextWdg.SetText("REARM FOR ");
		
		TextWidget priceTextWdg = TextWidget.Cast(newRow.FindAnyWidget("Price"));
		if (priceTextWdg)
			priceTextWdg.SetText(total.ToString()+"$");
	}

	protected void OnActivateConfirm()
	{
		m_playerShopComp.DoAskRpc_RearmVehicle(m_playerId);
		GetGame().GetMenuManager().CloseMenu(this);
	}

	protected void OnActivateEscape()
	{
		GetGame().GetMenuManager().CloseMenu(this);
	}
		
	private void ClearItemList()
	{
		int i = 0;
		bool clearFinished = false;
		while (!clearFinished && i < 1000) 
		{
			i++;
			Widget child = m_contentContainer.GetChildren();
			if (child) 
			{
				m_contentContainer.RemoveChild(child);
			} else {
				clearFinished = true;
			}
		}
	}
}