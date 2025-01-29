sealed class KOTH_SessionPlayerData : Managed
{
	protected int m_sessionMaxKillStreak = 0;
	protected int m_sessionMaxInsertionStreak = 0;
	protected int m_sessionEndGameBonus = 0;
	protected int m_sessionXpEarned = 0;
	protected int m_sessionMoneyEarned = 0;
	protected int m_sessionTimePlayed = 0;
	
	protected string m_sessionLastFaction;
	protected int m_sessionPointsWhenFactionWasJoined = 0;

	string GetSessionFaction() { return m_sessionLastFaction; }

	void SetSessionPointsWhenFactionWasJoined(int points, string faction) {
		m_sessionPointsWhenFactionWasJoined = points;
		m_sessionLastFaction = faction;
	}
	int GetSessionPointsWhenFactionWasJoined() { return m_sessionPointsWhenFactionWasJoined; }

	void UpdateKillStreak(int kills) 
	{
		if (kills > m_sessionMaxKillStreak)
			m_sessionMaxKillStreak = kills;
	}
	void UpdateInsertionStreak(int insertions) 
	{
		if (insertions > m_sessionMaxInsertionStreak)
			m_sessionMaxInsertionStreak = insertions;
	}
	void AddSessionXpAndMoney(int xp, int money) 
	{
		m_sessionMoneyEarned = m_sessionMoneyEarned + money;
		m_sessionXpEarned = m_sessionXpEarned + xp;
	}

	int GetSessionEndGameBonus() { return m_sessionEndGameBonus; }
	int GetSessionXpEarned() { return m_sessionXpEarned; }
	int GetSessionMoneyEarned() { return m_sessionMoneyEarned; }
	
	// in seconds
	void SetSessionTimePlayed(int time) { m_sessionTimePlayed = time; }
	int GetSessionTimePlayed() { return m_sessionTimePlayed; }
}
