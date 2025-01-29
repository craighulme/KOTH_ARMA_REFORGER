class KOTH_ExperienceManager
{
	protected static ref KOTH_ExperienceManager s_Instance;
	protected ref map<string, ref KOTH_PlayerBonusCode> m_CurrentProfileBonusCode = new map<string, ref KOTH_PlayerBonusCode>();

	int GetKillBonus(string playerUID)
	{
		KOTH_PlayerBonusCode currentBonusCode = m_CurrentProfileBonusCode.Get(playerUID);
		
		if (currentBonusCode && currentBonusCode.multiplier > 0)
			return KOTH_Globals.MONEY_PER_KILL * currentBonusCode.multiplier;

		return KOTH_Globals.MONEY_PER_KILL;
	}
	
	int GetKillAssistBonus(string playerUID)
	{
		KOTH_PlayerBonusCode currentBonusCode = m_CurrentProfileBonusCode.Get(playerUID);
		
		if (currentBonusCode && currentBonusCode.multiplier > 0)
			return KOTH_Globals.MONEY_PER_KILL_ASSIST * currentBonusCode.multiplier;

		return KOTH_Globals.MONEY_PER_KILL_ASSIST;
	}
	

    int GetVehicleHitZoneGroupKillBonus(string playerUID, string vehicleType, string hitZoneGroup)
    {
        KOTH_PlayerBonusCode currentBonusCode = m_CurrentProfileBonusCode.Get(playerUID);
        
        int baseMoney = KOTH_VehicleScoring.GetXPForHitZoneGroup(vehicleType, hitZoneGroup);
        
        if (currentBonusCode && currentBonusCode.multiplier > 0)
            return baseMoney * currentBonusCode.multiplier;
        
		return baseMoney;
    }

    int GetVehicleHitZoneGroupAssistBonus(string playerUID, string vehicleType, string hitZoneGroup)
    {
        KOTH_PlayerBonusCode currentBonusCode = m_CurrentProfileBonusCode.Get(playerUID);
        
        int assistMoney = KOTH_VehicleScoring.GetAssistXPForHitZoneGroup(vehicleType, hitZoneGroup);
        
        if (currentBonusCode && currentBonusCode.multiplier > 0)
            return assistMoney * currentBonusCode.multiplier;

        return assistMoney;
    }
	
	int GetReviveBonus(string playerUID)
	{
		KOTH_PlayerBonusCode currentBonusCode = m_CurrentProfileBonusCode.Get(playerUID);
		
		if (currentBonusCode && currentBonusCode.multiplier > 0)
			return KOTH_Globals.MONEY_PER_REVIVEFRIENDLY * currentBonusCode.multiplier;
		
		return KOTH_Globals.MONEY_PER_REVIVEFRIENDLY;

	}
	
	int GetDropBonus(string playerUID)
	{	
		KOTH_PlayerBonusCode currentBonusCode = m_CurrentProfileBonusCode.Get(playerUID);
		
		if (currentBonusCode && currentBonusCode.multiplier > 0)
			return KOTH_Globals.XP_PER_DROPFRIENDLY_IN_ZONE * currentBonusCode.multiplier;

		return KOTH_Globals.XP_PER_DROPFRIENDLY_IN_ZONE;
	}
	
	int GetZoneBonus(string playerUID)
	{
		#ifdef WORKBENCH
		#else
			if (GetGame().GetPlayerManager().GetPlayerCount() < 4)
				return KOTH_Globals.XP_IN_ZONE / 10;
		
			if (GetGame().GetPlayerManager().GetPlayerCount() < 7)
				return KOTH_Globals.XP_IN_ZONE / 5;
		#endif
		
		KOTH_PlayerBonusCode currentBonusCode = m_CurrentProfileBonusCode.Get(playerUID);
		
		if (currentBonusCode && currentBonusCode.multiplier > 0)
			return KOTH_Globals.XP_IN_ZONE * currentBonusCode.multiplier;
		
		return KOTH_Globals.XP_IN_ZONE;
	}
	
	bool AddKothBonus(string playerUID, KOTH_PlayerBonusCode bonusCode)
	{
		if (m_CurrentProfileBonusCode.Contains(playerUID)) 
		{
			Log("KOTH_ExperienceManager.AddKothBonus profile "+playerUID+" already has a bonus code "+bonusCode.name);
			return false;
		}

		m_CurrentProfileBonusCode.Insert(playerUID, bonusCode);
		return true;
	}

	static void ClearAll()
	{
		KOTH_ExperienceManager instance = KOTH_ExperienceManager.GetInstance();
		instance.m_CurrentProfileBonusCode.Clear();
	}

	static KOTH_ExperienceManager GetInstance()
	{
		if (s_Instance)
			return s_Instance;

		s_Instance = new KOTH_ExperienceManager();
		return s_Instance;
	}
}