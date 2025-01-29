class KOTH_ShopVehicleUI : KOTH_ShopUI
{
	override void OnMenuInit()
	{
		super.OnMenuInit();
		
		m_scoreComp = KOTH_ScoringGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_ScoringGameModeComponent));
		m_listShopItemList.Insert(m_scoreComp.GetVehicleShopItemList());
	}
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		// here you must change the tabs accordingly to the list above
		m_tabViewComponent.SetTabText(0, "Vehicles");
		m_tabViewComponent.RemoveTab(1);
	}
}
