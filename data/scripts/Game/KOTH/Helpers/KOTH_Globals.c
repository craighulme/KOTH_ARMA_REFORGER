class KOTH_Globals
{
	//Default First Person XP and Money
	const int XP_IN_ZONE = 80;
	const int MONEY_IN_ZONE = 80;
	
	const int XP_PER_KILL = 100;
	const int MONEY_PER_KILL = 100;
	
	const int XP_PER_KILL_ASSIST = XP_PER_KILL / 2;
	const int MONEY_PER_KILL_ASSIST = MONEY_PER_KILL / 2;
	
	const int BONUS_FLUSH_TOILET = 200;
	
	//Piloting Transport XP and Money
	const int XP_PER_DROPFRIENDLY_IN_ZONE = 100;
	const int MONEY_PER_DROPFRIENDLY_IN_ZONE = 100;
	
	const int XP_PER_REVIVEFRIENDLY = 300;
	const int MONEY_PER_REVIVEFRIENDLY = 300;
	
	const int XP_PER_KILLFRIEND = 300;
	const int MONEY_PER_KILLFRIEND = 300;
	
	const int XP_PER_KILLFRIEND_VEHICLE_PART = 150;
	const int MONEY_PER_KILLFRIEND_VEHICLE_PART = 150;
	
	static int GetXpNextLevel(int playerLevel) 
	{
		if (playerLevel >= 100)
			return (playerLevel + playerLevel - 1) * 10000;
	
		return (playerLevel + playerLevel - 1) * 1000;
	}
	
}



