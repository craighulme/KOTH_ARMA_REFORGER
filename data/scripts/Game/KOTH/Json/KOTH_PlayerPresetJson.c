class KOTH_PlayerPresetJson : JsonApiStruct
{
	string m_primaryWeaponResource;
	string m_secondaryWeaponResource;
	string m_opticWeaponResource;

	string m_muzzleResource;
	string m_launcherResource;
	ref array<string> m_throwableResource = {};

	string m_rangeFinderResource;
	string m_viperhoodResource;

	void KOTH_PlayerPresetJson()
	{
		RegV("m_primaryWeaponResource");
		RegV("m_secondaryWeaponResource");
		RegV("m_opticWeaponResource");
		
		RegV("m_muzzleResource");
		RegV("m_launcherResource");
		RegV("m_throwableResource");
		
		RegV("m_rangeFinderResource");
		RegV("m_viperhoodResource");
	}
	
	void PopulateFromSessionLoadout(KOTH_SessionPlayerLoadout sessionLoadout)
	{
		if (sessionLoadout.m_primary)
			m_primaryWeaponResource = sessionLoadout.m_primary.m_itemResource;
		
		if (sessionLoadout.m_handgun)
			m_secondaryWeaponResource = sessionLoadout.m_handgun.m_itemResource;
		
		if (sessionLoadout.m_optic)
			m_opticWeaponResource = sessionLoadout.m_optic.m_itemResource;
		
		if (sessionLoadout.m_muzzle)
			m_muzzleResource = sessionLoadout.m_muzzle.m_itemResource;
		
		if (sessionLoadout.m_launcher)
			m_launcherResource = sessionLoadout.m_launcher.m_itemResource;
		
		if (sessionLoadout.m_rangeFinder)
			m_rangeFinderResource = sessionLoadout.m_rangeFinder.m_itemResource;
		
		if (sessionLoadout.m_viperhood)
			m_viperhoodResource = sessionLoadout.m_viperhood.m_itemResource;
		
		foreach (KOTH_ShopItem item : sessionLoadout.m_throwables)
		{
			m_throwableResource.Insert(item.m_itemResource);
		}
	}
}
