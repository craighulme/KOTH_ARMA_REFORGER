class KOTH_SessionPlayerLoadout : Managed
{
	ref KOTH_ShopItem m_primary;
	ref KOTH_ShopItem m_optic;
	ref KOTH_ShopItem m_muzzle;
	ref KOTH_ShopItem m_launcher;
	ref KOTH_ShopItem m_handgun;
	ref KOTH_ShopItem m_viperhood;
	ref KOTH_ShopItem m_rangeFinder;
	ref array<ref KOTH_ShopItem> m_throwables = {};
	
	bool IsEmpty()
	{
		if (m_primary)
			return false;
		
		if (m_optic)
			return false;
		
		if (m_muzzle)
			return false;
		
		if (m_launcher)
			return false;
		
		if (m_handgun)
			return false;
		
		if (m_viperhood)
			return false;

		if (m_rangeFinder)
			return false;
		
		if (!m_throwables.IsEmpty())
			return false;
		
		return true;
	}
	
	static KOTH_SessionPlayerLoadout FromBackendPresetToSessionLoadout(KOTH_SCR_PlayerShopComponent playerShopComp, KOTH_PlayerPresetJson preset)
	{
		KOTH_SessionPlayerLoadout loadout = new KOTH_SessionPlayerLoadout();
		loadout.m_primary = playerShopComp.FindShopItemByResourceName(preset.m_primaryWeaponResource);
		loadout.m_optic = playerShopComp.FindShopItemByResourceName(preset.m_opticWeaponResource);
		loadout.m_muzzle = playerShopComp.FindShopItemByResourceName(preset.m_muzzleResource);
		loadout.m_launcher = playerShopComp.FindShopItemByResourceName(preset.m_launcherResource);
		loadout.m_handgun = playerShopComp.FindShopItemByResourceName(preset.m_secondaryWeaponResource);
		loadout.m_rangeFinder = playerShopComp.FindShopItemByResourceName(preset.m_rangeFinderResource);
		loadout.m_viperhood = playerShopComp.FindShopItemByResourceName(preset.m_viperhoodResource);
		
		return loadout;
	}

	void Clear()
	{
		m_primary = null;
		m_optic = null;
		m_muzzle = null;
		m_launcher = null;
		m_handgun = null;
		m_viperhood = null;
		m_rangeFinder = null;
		m_throwables = {};
	}
}