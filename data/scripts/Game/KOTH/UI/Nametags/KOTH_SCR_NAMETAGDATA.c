modded class SCR_NameTagData
{
	private ArmaReforgerScripted m_game = GetGame();
	
	override void UpdateTagPos()
	{
		super.UpdateTagPos();
		
		if (m_game.ShowTeamNames)
		{
			m_fVisibleOpacity = 1;
		} 
		else 
		{
			m_fVisibleOpacity = 0;
		}

		vector matPos[4];
		Animation anim = m_Entity.GetAnimation();
		//m_iHeadBone
		//m_iSpineBone
		anim.GetBoneMatrix(m_iHeadBone, matPos);
		m_vTagWorldPos = m_Entity.CoordToParent(matPos[3]);
	}

	override protected void InitData(SCR_NameTagConfig config)
	{
		super.InitData(config);

		if (!m_Entity)
			return;

		FactionAffiliationComponent targetFactionComp = FactionAffiliationComponent.Cast(m_Entity.FindComponent(FactionAffiliationComponent));
		if (!targetFactionComp)
			return;

		Faction faction = targetFactionComp.GetAffiliatedFaction();
		if (!faction)
			return;
		
		ImageWidget iconFactionKOTH = ImageWidget.Cast(m_NameTagWidget.FindAnyWidget("IconFactionKOTH"));
		switch (faction.GetFactionName())
		{
			case KOTH_Faction.OPFOR:
				iconFactionKOTH.SetColor(Color.Red);
				iconFactionKOTH.SetVisible(true);
			break;
			case KOTH_Faction.BLUFOR:
				iconFactionKOTH.SetColor(Color.DodgerBlue);
				iconFactionKOTH.SetVisible(true);
			break;
			case KOTH_Faction.INDFOR:
				iconFactionKOTH.SetColor(Color.Green);
				iconFactionKOTH.SetVisible(true);
			break;
			default:
				iconFactionKOTH.SetVisible(false);
			break;
		}
	}
}
