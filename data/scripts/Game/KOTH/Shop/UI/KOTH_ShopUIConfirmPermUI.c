class KOTH_ShopUIConfirmPermUI : ChimeraMenuBase
{
	protected KOTH_SCR_PlayerShopComponent m_playerShopComp;
	protected ref KOTH_ShopItem m_itemtoPermaBuy;
	protected int m_playerId;
	
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
		Widget m_wRoot = GetRootWidget();
		
		// add listeners
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.GetInputButtonComponent("Cancel", m_wRoot);
		if (cancel) { cancel.m_OnActivated.Insert(OnClickEscape); }
		
		SCR_InputButtonComponent confirm = SCR_InputButtonComponent.GetInputButtonComponent("Confirm", m_wRoot);
		if (confirm) { confirm.m_OnActivated.Insert(OnClickConfirm); }
		
		m_itemtoPermaBuy = m_playerShopComp.FindShopItemByResourceName(m_playerShopComp.m_permanentBuyResourceNameConfirm);
		TextWidget textNameWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("Name"));
		textNameWdg.SetText(m_itemtoPermaBuy.m_itemName);
		
		TextWidget textPriceWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("Price"));
		textPriceWdg.SetText(""+m_itemtoPermaBuy.m_pricePermanent +"$");
	}
	
	protected void OnClickEscape()
	{
		GetGame().GetMenuManager().CloseMenu(this);
	}
		
	protected void OnClickConfirm(SCR_ButtonBaseComponent button)
	{
		m_playerShopComp.DoRpcBuy(m_itemtoPermaBuy.m_itemResource, m_playerId, true);
		GetGame().GetMenuManager().CloseMenu(this);
	}
}