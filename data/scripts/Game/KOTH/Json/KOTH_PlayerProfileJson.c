class KOTH_PlayerProfileJson : JsonApiStruct
{
	string m_playerUID;
	string m_playerName;
	
	
	string m_profileName;
	string m_platformName;
	string m_machineName;
	string m_adapterName;
	
	// start values
	protected int m_money = -1;
	protected int m_level = -1;
	protected int m_xp = -1;

	int m_kills = 0;
	int m_deaths = 0;
	int m_assists = 0;
	int m_friendlyKills = 0;
	
	ref array<string> m_unlockedItems = {};

	ref array<ref KOTH_PlayerPresetJson> m_playerPresets = {};

	void KOTH_PlayerProfileJson()
	{
		RegV("m_playerUID");
		RegV("m_playerName");
		
		RegV("m_profileName");
		RegV("m_platformName");
		RegV("m_machineName");
		RegV("m_adapterName");
		
		RegV("m_money");
		RegV("m_level");
		RegV("m_xp");

		RegV("m_kills");
		RegV("m_deaths");
		RegV("m_friendlyKills");
		RegV("m_assists");

		RegV("m_unlockedItems");
		RegV("m_playerPresets");
	}

	void AddDeath() { m_deaths++; }
	void AddKill() { m_kills++; }
	void AddAssist() { m_assists++; }
	
	void AddEndGameBonusXpAndMoney(int bonus)
	{
		AddXp(bonus);
		AddMoney(bonus);
	}

	void Buy(int price)
	{
		m_money -= price;
	}
	
	void Refund(int price)
	{
		m_money += price;
	}

	void RemoveFriendlyKillXpAndMoney(bool isHeloTK = false)
	{
		int amount = KOTH_Globals.MONEY_PER_KILLFRIEND;
		if (isHeloTK)
			amount = amount / 2;
		
		m_money = m_money - amount;
		m_xp = m_xp - amount;
		
		if (m_xp < 0)
			m_xp = 0;
		
		m_friendlyKills++;
	}
	
	void RemoveVehicleFriendlyKillXpAndMoney()
	{
		int amount = KOTH_Globals.MONEY_PER_KILLFRIEND_VEHICLE_PART;
		
		m_money = m_money - amount;
		m_xp = m_xp - amount;
	}

	int GetMoney()
	{
		return m_money;
	}
	
	int GetXp()
	{
		return m_xp;
	}

	int GetLevel()
	{
		return m_level;
	}

	int AddMoney(int amount)
	{
		m_money += amount;
		
		return m_money;
	}
	
	int RemoveMoney(int amount)
	{
		m_money -= amount;
		
		return m_money;
	}

	void AddXp(int amount)
	{
		int nextLevelXp = KOTH_Globals.GetXpNextLevel(m_level);

		if (m_xp + amount >= nextLevelXp) 
		{
			m_level++;
			
			int leftOver = (m_xp + amount) - nextLevelXp;
			m_xp = leftOver;
			
			if (m_xp >= KOTH_Globals.GetXpNextLevel(m_level)) 
			{
				m_level++;
				m_xp = 0;
			}

			return;
		}
		

		m_xp = m_xp + amount;
	}
	
	int AddLevel(int amount)
	{
		m_level += amount;
		
		return m_level;
	}
	
	int RemoveLevel(int amount)
	{
		m_level -= amount;
		
		return m_level;
	}
}
