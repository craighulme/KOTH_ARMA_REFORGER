class KOTH_ShopUI : ChimeraMenuBase
{
	protected Widget m_wRoot;
	protected TextWidget m_nameContainer;
	protected ItemPreviewManagerEntity m_ItemPreviewManager;
	
	protected Widget m_parentShopCategorie_Weapons;
	protected Widget m_parentShopCategorie_Ammo;
	protected Widget m_parentShopCategorie_Gadget;
	
	protected GridLayoutWidget m_contentContainer;
	
	protected SCR_TabViewComponent m_tabViewComponent;
	
	protected KOTH_SCR_PlayerShopComponent m_playerShopComp;
	protected KOTH_SCR_PlayerProfileComponent m_playerProfileComp;
	protected KOTH_ScoringGameModeComponent m_scoreComp;
	
	protected int m_playerId;

	protected ref array<ResourceName> m_listShopItemList = {};

	override void OnMenuInit()
	{
		super.OnMenuInit();
		
		m_scoreComp = KOTH_ScoringGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_ScoringGameModeComponent));
		
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
		m_contentContainer = GridLayoutWidget.Cast(m_wRoot.FindAnyWidget("ContentContainer"));
		
		// add listeners
		SCR_InputButtonComponent cancel = SCR_InputButtonComponent.GetInputButtonComponent("Cancel", m_wRoot);
		if (cancel) { cancel.m_OnActivated.Insert(OnClickEscape); }
			
		Widget tabViewRootWidget = m_wRoot.FindAnyWidget("TabViewRoot");
		m_tabViewComponent = SCR_TabViewComponent.Cast(tabViewRootWidget.FindHandler(SCR_TabViewComponent));
		m_tabViewComponent.GetOnContentSelect().Insert(OnSelectedTab);

		AddItemsFromConfig(m_listShopItemList[0]);
	}
	
	void OnSelectedTab(SCR_TabViewComponent tabView, SCR_TabViewContent tabContent, int index)
	{
		ClearItemList();
		
		AddItemsFromConfig(m_listShopItemList[m_tabViewComponent.GetShownTab()]);
	}
	
	void RemoveViperHood(array<ref KOTH_ShopItem> items)
	{
		Faction ownerFaction = KOTH_Helper.GrabFaction(GetGame().GetPlayerController().GetControlledEntity());
		
		string currentFactionViperhood = ownerFaction.GetFactionKey() + "_VIPERHOOD";

		array<KOTH_ShopItem> indexToRemove = {};
		foreach (int index, KOTH_ShopItem item : items)
		{
			if (item.m_itemResource.Contains("VIPERHOOD") && !item.m_itemResource.Contains(currentFactionViperhood))
				indexToRemove.Insert(item);
		}

		foreach (KOTH_ShopItem item : indexToRemove) { items.RemoveItemOrdered(item); }
	}

	protected void AddItemsFromConfig(string itemListResource)
	{
		KOTH_ShopItemList itemList = SCR_ConfigHelperT<KOTH_ShopItemList>.GetConfigObject(itemListResource);
		array<ref KOTH_ShopItem> items = itemList.GetItemsOrderedByLevel();
	
		if (itemListResource.Contains("Accessory"))
			RemoveViperHood(items);
		
		// Puts bought items at the top of the list
		int boughtItems = 0;
		array<string> unlockedItems = m_playerProfileComp.GetUnlockedItemList();
        foreach (int index, KOTH_ShopItem item : items)
		{
			if (KOTH_CustomizabilityManager.isUnlocked(unlockedItems, item.m_itemResource)) {
				items.RemoveItemOrdered(item);
				items.InsertAt(item, boughtItems);
				boughtItems++;
			}
		}
		
		bool firstLine = true;
		int row = 0;
		foreach (int index, KOTH_ShopItem item : items)
		{
			Widget newRow = GetGame().GetWorkspace().CreateWidgets("{8D7C219CE84659E2}UI/Layouts/Shop/ShopGun_ItemListV3.layout", m_contentContainer);
			int column = index % 2;
			GridSlot.SetRow(newRow, row);
			GridSlot.SetColumn(newRow, column);
			if (!firstLine && column != 0)
				row++;

			firstLine = false;
			
			// add preview image
			ChimeraWorld world = GetGame().GetWorld();
			ItemPreviewManagerEntity previewManager = world.GetItemPreviewManager();
			ItemPreviewWidget preview_Image = ItemPreviewWidget.Cast(newRow.FindAnyWidget("Preview_Image"));
			if (preview_Image)
				previewManager.SetPreviewItemFromPrefab(preview_Image, item.m_itemResource, null, false);

			// add click events
			SCR_ButtonBaseComponent buyOnce = SCR_ButtonBaseComponent.GetButtonBase("PurchaseOnceButton", newRow);
			if (buyOnce)
				buyOnce.m_OnClicked.Insert(OnClickBuyOnce);
			
			if (item.m_pricePermanent > 1) {
				ShowNotOwned(newRow);
			} else {
				Widget buyPermLayoutWidget = newRow.FindAnyWidget("BuyPerm_Layout0");
				buyPermLayoutWidget.SetVisible(false);
			}
			SCR_ButtonBaseComponent buyPermanent = SCR_ButtonBaseComponent.GetButtonBase("PurchasePermanentButton", newRow);
			if (buyPermanent)
				buyPermanent.m_OnClicked.Insert(OnClickBuyPermanent);
			
			SCR_ButtonBaseComponent equipButton = SCR_ButtonBaseComponent.GetButtonBase("EquipButton", newRow);
			if (equipButton)
				equipButton.m_OnClicked.Insert(OnClickEquip);

			// set text infos
			TextWidget resourceNameWidget = TextWidget.Cast(newRow.FindAnyWidget("ResourceName"));
			if (resourceNameWidget)
				resourceNameWidget.SetText(item.m_itemResource);
			TextWidget nameWidget = TextWidget.Cast(newRow.FindAnyWidget("ItemName"));
			if (nameWidget)
				nameWidget.SetText(item.m_itemName);
			TextWidget priceOnceWidget = TextWidget.Cast(newRow.FindAnyWidget("PriceOnceText"));
			if (priceOnceWidget)
				priceOnceWidget.SetText(item.m_priceOnce.ToString() + "$");
			
			TextWidget pricePermWidget = TextWidget.Cast(newRow.FindAnyWidget("PricePermText"));
			if (pricePermWidget)
				pricePermWidget.SetText(item.m_pricePermanent.ToString() + "$");
			
			OverlayWidget desactivatePermHINT = OverlayWidget.Cast(newRow.FindAnyWidget("DesactivatePermHINT"));
			ButtonWidget purchaseOnceButton = ButtonWidget.Cast(newRow.FindAnyWidget("PurchaseOnceButton"));
			ButtonWidget purchasePermanentButton = ButtonWidget.Cast(newRow.FindAnyWidget("PurchasePermanentButton"));
			
			OverlayWidget purchaseOnceNoLVL = OverlayWidget.Cast(newRow.FindAnyWidget("PurchaseOnceNoLVL"));
			OverlayWidget purchasePermNoLVL = OverlayWidget.Cast(newRow.FindAnyWidget("PurchasePermNoLVL"));
			
			
			if (m_playerProfileComp.GetLevel() < item.m_level)
			{				
				ImageWidget fadeLVLReq = ImageWidget .Cast(newRow.FindAnyWidget("FadeLVLReq"));
				TextWidget textLevelReq = TextWidget.Cast(newRow.FindAnyWidget("TextLevelReq"));
				ButtonWidget fakeButtonLevelReq = ButtonWidget.Cast(newRow.FindAnyWidget("FakeButtonLevelReq"));
				
				purchaseOnceButton.SetVisible(false);
				purchasePermanentButton.SetVisible(false);
				desactivatePermHINT.SetVisible(false);
				
				purchaseOnceNoLVL.SetVisible(true);
				purchasePermNoLVL.SetVisible(true);
				fakeButtonLevelReq.SetVisible(true);
				
				fadeLVLReq.SetVisible(true);
				textLevelReq.SetVisible(true);
				
				textLevelReq.SetText("LEVEL " + item.m_level.ToString() + " REQUIRED");
			}
			else
			{
				// set visible or not buy
				OverlayWidget purchaseOnceHINT = OverlayWidget.Cast(newRow.FindAnyWidget("PurchaseOnceHINT"));
				purchaseOnceButton.SetVisible(true);
				purchaseOnceHINT.SetVisible(false);
		
				m_playerProfileComp.GetUnlockedItemList().Contains("") || m_playerProfileComp.GetUnlockedItemList().Contains("");
								
				if (KOTH_CustomizabilityManager.isUnlocked(unlockedItems, item.m_itemResource))
				{
					purchaseOnceButton.SetVisible(false);
					equipButton.SetVisible(true);
					
					TextWidget priceOnceTextWdg = TextWidget.Cast(newRow.FindAnyWidget("PriceOnceText"));
					priceOnceTextWdg.SetText(" ");
					TextWidget pricePermTextWdg = TextWidget.Cast(newRow.FindAnyWidget("PricePermText"));
					pricePermTextWdg.SetText(string.Empty);

					OverlayWidget purchasePermHINT = OverlayWidget.Cast(newRow.FindAnyWidget("PurchasePermHINT"));
					purchasePermanentButton.SetVisible(false);
					desactivatePermHINT.SetVisible(false);
					purchasePermHINT.SetVisible(true);
				}
			}
		}
	}
	
	void ShowNotOwned(Widget newRow)
	{
		OverlayWidget overlayOwned = OverlayWidget.Cast(newRow.FindAnyWidget("PurchasePermHINT"));
		overlayOwned.SetVisible(false);
		OverlayWidget overlayDeactived = OverlayWidget.Cast(newRow.FindAnyWidget("DesactivatePermHINT"));
		overlayDeactived.SetVisible(false);
		ButtonWidget overlayBuyPerm = ButtonWidget.Cast(newRow.FindAnyWidget("PurchasePermanentButton"));
		overlayBuyPerm.SetVisible(true);
	}
	
	void ShowOwned(Widget newRow)
	{
		OverlayWidget overlayOwned = OverlayWidget.Cast(newRow.FindAnyWidget("PurchasePermHINT"));
		overlayOwned.SetVisible(true);
		
		OverlayWidget overlayDeactived = OverlayWidget.Cast(newRow.FindAnyWidget("DesactivatePermHINT"));
		overlayDeactived.SetVisible(false);
		ButtonWidget overlayBuyPerm = ButtonWidget.Cast(newRow.FindAnyWidget("PurchasePermanentButton"));
		overlayBuyPerm.SetVisible(false);
	}
	
	void ClearItemList()
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

	protected void OnClickBuyOnce(SCR_ButtonBaseComponent button)
	{
		Widget row = button.GetRootWidget().GetParent().GetParent().GetParent();
		TextWidget priceOnceWidget = TextWidget.Cast(row.FindAnyWidget("PriceOnceText"));
		TextWidget resourceNameWidget = TextWidget.Cast(row.FindAnyWidget("ResourceName"));
		
		m_playerShopComp.DoRpcBuy(resourceNameWidget.GetText(), m_playerId);

		// set widget equiped visible
		ButtonWidget purchaseOnceButton = ButtonWidget.Cast(row.FindAnyWidget("PurchaseOnceButton"));
		OverlayWidget purchaseOnceHINT = OverlayWidget.Cast(row.FindAnyWidget("PurchaseOnceHINT"));
		purchaseOnceButton.SetVisible(false);
		purchaseOnceHINT.SetVisible(true);
	}

	protected void OnClickBuyPermanent(SCR_ButtonBaseComponent button)
	{
		Widget row = button.GetRootWidget().GetParent().GetParent().GetParent();
		TextWidget pricePermWidget = TextWidget.Cast(row.FindAnyWidget("PricePermText"));
		TextWidget resourceNameWidget = TextWidget.Cast(row.FindAnyWidget("ResourceName"));

		m_playerShopComp.m_permanentBuyResourceNameConfirm = resourceNameWidget.GetText();
		GetGame().GetMenuManager().CloseMenu(this);
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.KOTH_ShopUIConfirmPerm);
	}
	
	protected void OnClickEquip(SCR_ButtonBaseComponent button)
	{
		Widget row = button.GetRootWidget().GetParent().GetParent().GetParent();
		TextWidget resourceNameWidget = TextWidget.Cast(row.FindAnyWidget("ResourceName"));
		m_playerShopComp.AskRpc_Equip(resourceNameWidget.GetText(), m_playerId);
		
		// set widget equiped visible
		OverlayWidget purchaseOnceHINT = OverlayWidget.Cast(row.FindAnyWidget("PurchaseOnceHINT"));
		button.SetVisible(false);
		purchaseOnceHINT.SetVisible(true);
	}

	//inform player error in buy
	void NotifErrorShop(string firstLine, string secondLine)
	{
		Widget root = GetRootWidget();
		VerticalLayoutWidget koth_scrollListshop = VerticalLayoutWidget.Cast(root.FindWidget("Overlay.VerticalLayout1.ScrollList.NotifContainer"));

		Widget w = GetGame().GetWorkspace().CreateWidgets("{993FD33C936EB12C}UI/Layouts/HUD/KingOfTheHill/KOTH_ShopNotification.layout", koth_scrollListshop);
		TextWidget Seconde = TextWidget.Cast(w.FindAnyWidget("Seconde"));
		Seconde.SetText(firstLine);

		TextWidget Third = TextWidget.Cast(w.FindAnyWidget("Third"));
		Third.SetText(secondLine);

		SCR_FadeUIComponent compFade = SCR_FadeUIComponent.Cast(w.FindHandler(SCR_FadeUIComponent));
		compFade.DelayedFadeOut(10000, true);
	}

	void HUD_NotifBuy(int amount)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifBuy(amount);
			}
		}
	}
	
	void NotifBuy_PermanentBuySuccess(string itemResource)
	{
		int i = 0;
		bool foundRow = false;
		Widget child = m_contentContainer.GetChildren();
		TextWidget resourceNameWidget = TextWidget.Cast(child.FindAnyWidget("ResourceName"));
		
		if (resourceNameWidget.GetText() == itemResource) {
			Widget PurchaseOnceButton = child.FindAnyWidget("PurchaseOnceButton");
			PurchaseOnceButton.SetVisible(false);
			OverlayWidget EquipButton = OverlayWidget.Cast(child.FindAnyWidget("EquipButton"));
			if (EquipButton)
				EquipButton.SetVisible(true);
		}

		while (!foundRow && i < 1000) 
		{
			i++;
			child = child.GetSibling();
			resourceNameWidget = TextWidget.Cast(child.FindAnyWidget("ResourceName"));
			if (resourceNameWidget.GetText() == itemResource) 
			{
				Widget PurchaseOnceButton = child.FindAnyWidget("PurchaseOnceButton");
				PurchaseOnceButton.SetVisible(false);
				Widget EquipButton = child.FindAnyWidget("EquipButton");
				EquipButton.SetVisible(true);
				foundRow = true;
			}
		}
	}

	protected void OnClickEscape()
	{
		GetGame().GetMenuManager().CloseMenu(this);
	}
}
