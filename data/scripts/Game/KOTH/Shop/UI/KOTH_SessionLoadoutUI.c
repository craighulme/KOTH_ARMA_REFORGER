class KOTH_SessionLoadoutUI : ChimeraMenuBase
{
	protected Widget m_wRoot;
	protected TextWidget m_nameContainer;
	protected VerticalLayoutWidget m_contentContainer;
	
	protected KOTH_SCR_PlayerShopComponent m_playerShopComp;
	protected KOTH_SCR_PlayerProfileComponent m_playerProfileComp;
	protected KOTH_SessionDataGameModeComponent m_sessionDataComp;
	
	protected int m_playerId;

	protected ref array<ResourceName> m_listShopItemList = {};

	override void OnMenuInit()
	{
		super.OnMenuInit();
		
		m_sessionDataComp = KOTH_SessionDataGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_SessionDataGameModeComponent));
		
		PlayerController controller = GetGame().GetPlayerController();
		m_playerId = controller.GetPlayerId();
		m_playerShopComp = KOTH_SCR_PlayerShopComponent.Cast(controller.FindComponent(KOTH_SCR_PlayerShopComponent));
		m_playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(controller.FindComponent(KOTH_SCR_PlayerProfileComponent));
	}
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_wRoot = GetRootWidget();
		m_nameContainer = TextWidget.Cast(m_wRoot.FindAnyWidget("NameContainer"));
		m_contentContainer = VerticalLayoutWidget.Cast(m_wRoot.FindAnyWidget("NotifContainer"));

		// add listeners
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.GetInputButtonComponent("Cancel", m_wRoot);
		if (cancel) { cancel.m_OnActivated.Insert(OnActivateEscape); }
		
		SCR_InputButtonComponent confirm = SCR_InputButtonComponent.GetInputButtonComponent("Confirm", m_wRoot);
		if (confirm) { confirm.m_OnActivated.Insert(OnActivateConfirm); }
		
		ClearItemList();
		
		KOTH_SessionPlayerLoadout m_loadoutData = m_playerShopComp.m_sessionLoadout;
		
		int total = 0;
		if (m_loadoutData.m_primary) {
			total = total + AddTextItem(m_loadoutData.m_primary);
		}
		if (m_loadoutData.m_handgun) {
			total = total + AddTextItem(m_loadoutData.m_handgun);
		}
		if (m_loadoutData.m_launcher) {
			total = total + AddTextItem(m_loadoutData.m_launcher);
		}
		if (m_loadoutData.m_optic) {
			total = total + AddTextItem(m_loadoutData.m_optic);
		}
		if (m_loadoutData.m_muzzle) {
			total = total + AddTextItem(m_loadoutData.m_muzzle);
		}
		if (m_loadoutData.m_viperhood) {
			total = total + AddTextItem(m_loadoutData.m_viperhood);
		}
		if (m_loadoutData.m_rangeFinder) {
			total = total + AddTextItem(m_loadoutData.m_rangeFinder);
		}

		if (m_loadoutData.m_throwables)
		{
			foreach (KOTH_ShopItem item : m_loadoutData.m_throwables)
			{
				total = total + AddTextItem(item);
			}
		}

		TextWidget totalPriceTextWdg = TextWidget.Cast(m_wRoot.FindAnyWidget("TotalPrice"));
		if (totalPriceTextWdg)
			totalPriceTextWdg.SetText("TOTAL = " + total + "$");
		}

	protected void AddTotalLine(int total)
	{
		GetGame().GetWorkspace().CreateWidgets("{58A2DBEC3607A192}UI/Layouts/Shop/SessionLoadout_ItemLine.layout", m_contentContainer);
		Widget newRow = GetGame().GetWorkspace().CreateWidgets("{58A2DBEC3607A192}UI/Layouts/Shop/SessionLoadout_ItemLine.layout", m_contentContainer);
		TextWidget nameTextWdg = TextWidget.Cast(newRow.FindAnyWidget("Name"));
		if (nameTextWdg)
			nameTextWdg.SetText("TOTAL");
		
		TextWidget priceTextWdg = TextWidget.Cast(newRow.FindAnyWidget("Name"));
		if (priceTextWdg)
			priceTextWdg.SetText(total.ToString()+"$");
	}
	
	protected int AddTextItem(KOTH_ShopItem item)
	{
		Widget newRow = GetGame().GetWorkspace().CreateWidgets("{58A2DBEC3607A192}UI/Layouts/Shop/SessionLoadout_ItemLine.layout", m_contentContainer);
		
		TextWidget nameTextWdg = TextWidget.Cast(newRow.FindAnyWidget("Name"));
		if (nameTextWdg)
			nameTextWdg.SetText(item.m_itemName);
		
		array<string> unlockedItems = m_playerProfileComp.GetUnlockedItemList();
		
		TextWidget priceTextWdg = TextWidget.Cast(newRow.FindAnyWidget("Price"));
		if (priceTextWdg)
		{
			if (KOTH_CustomizabilityManager.isUnlocked(unlockedItems, item.m_itemResource))
			{
				priceTextWdg.SetText("FREE");
			} else {
				priceTextWdg.SetText(item.m_priceOnce.ToString());
				return item.m_priceOnce;
			}
		}
		
		return 0;
	}
	
	protected void OnActivateConfirm()
	{
		m_playerShopComp.AskRpc_BuyOrEquipSessionLoadout(m_playerId);
		GetGame().GetMenuManager().CloseMenu(this);
	}

	protected void OnActivateEscape()
	{
		m_playerShopComp.AskRpc_ClearSessionLoadout(m_playerId);
		GetGame().GetMenuManager().CloseMenu(this);
	}

	void ClearItemList()
	{
		int i = 0;
		bool clearFinished = false;
		while (!clearFinished && i < 1000) 
		{
			i++;
			Widget child = m_contentContainer.GetChildren();
			if (child) {
				m_contentContainer.RemoveChild(child);
			} else {
				clearFinished = true;
			}
		}
	}
}
