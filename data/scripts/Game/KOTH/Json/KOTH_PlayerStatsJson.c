class KOTH_PlayerStatsJson : JsonApiStruct
{
	string m_playerUID;
	
	int m_bulletsShot;
	int m_grenadesThrown;
	int m_maxKillStreak;
	int m_maxKillDistance;
	int m_insertionBonus;
	
	int m_killStreakX3;
	int m_killStreakX5;
	int m_killStreakX10;
	int m_killStreakX20;
	int m_killStreakX30;

	void KOTH_PlayerStatsJson()
	{
		RegV("m_playerUID");
		
		RegV("m_bulletsShot");
		RegV("m_grenadesThrown");
		RegV("m_maxKillStreak");
		RegV("m_maxKillDistance");
		RegV("m_insertionBonus");
		
		RegV("m_killStreakX3");
		RegV("m_killStreakX5");
		RegV("m_killStreakX10");
		RegV("m_killStreakX20");
		RegV("m_killStreakX30");
		
	}

	void SetNewMaxKillDistance(int newkillDistance)
	{
		if (m_maxKillDistance < newkillDistance)
			m_maxKillDistance = newkillDistance;
	}

	void SetNewMaxKillStreak(int newkillStreak)
	{
		switch (newkillStreak)
        {
            case 3:
                m_killStreakX3++;
                break;
            case 5:
                m_killStreakX5++;
                break;
            case 10:
                m_killStreakX10++;
                break;
            case 20:
                m_killStreakX20++;
                break;
            case 30:
                m_killStreakX30++;
                break;
        }
		
		if (m_maxKillStreak < newkillStreak)
			m_maxKillStreak = newkillStreak;
	}
}
