class KOTH_ShopWeaponUI : KOTH_ShopUI
{
	override void OnMenuInit()
	{
		super.OnMenuInit();

		m_scoreComp = KOTH_ScoringGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_ScoringGameModeComponent));
		m_listShopItemList.Insert(m_scoreComp.GetWeaponShopItemList());
		m_listShopItemList.Insert(m_scoreComp.GetOpticsShopItemList());	
		m_listShopItemList.Insert(m_scoreComp.GetMuzzleShopItemList());
		m_listShopItemList.Insert(m_scoreComp.GetExplosiveShopItemList());
		m_listShopItemList.Insert(m_scoreComp.GetAccessoryShopItemList());
	}

	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		// here you must change the tabs accordingly to the list above
		m_tabViewComponent.RemoveTab(1);
		m_tabViewComponent.AddTab("", "Optics");
		m_tabViewComponent.AddTab("", "Muzzle");
		m_tabViewComponent.AddTab("", "Explosives");
		m_tabViewComponent.AddTab("", "Accessories");
	}
}
