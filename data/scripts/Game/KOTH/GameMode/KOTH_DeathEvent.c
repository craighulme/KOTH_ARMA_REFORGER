//Event used to save parameter passing. Filled as needed.
class KOTH_DeathEvent
{
	int playerId;
	int killerId;
	int originalKillerId;
	int lastKillerIdForPilot;
	string playerUID;
	string killerUID;
	IEntity playerEntity;
	IEntity killerEntity;
	Vehicle playerVehicle;
	Vehicle killerVehicle;
	bool isSuicide;
	bool playerIsInVehicle;
	bool killerIsInVehicle;
	bool pilotKilledByAnother;
	bool vehicleWeaponDeath;
	FactionAffiliationComponent playerFactionComp;
	FactionAffiliationComponent killerFactionComp;
	KOTH_SCR_PlayerProfileComponent playerProfileComp;
	KOTH_SCR_PlayerProfileComponent killerProfileComp;
	KOTH_PlayerProfileJson playerProfileJson;
	KOTH_PlayerProfileJson killerProfileJson;

	void Reset()
	{
		playerId = -1;
		killerId = -1;
		originalKillerId = -1;
		lastKillerIdForPilot = -1;
		playerUID = string.Empty;
		killerUID = string.Empty;
		playerEntity = null;
		killerEntity = null;
		playerVehicle = null;
		killerVehicle = null;
		playerIsInVehicle = false;
		killerIsInVehicle = false;
		isSuicide = false;
		pilotKilledByAnother = false;
		vehicleWeaponDeath = false;
		playerFactionComp = null;
		killerFactionComp = null;
		playerProfileComp = null;
		killerProfileComp = null;
		playerProfileJson = null;
		killerProfileJson = null;
	}
	
	bool isFriendlyFire()
	{
		if (isSuicide)
		{
			LogWorkbench("OnControllableDestroyed: Suicide detected for PlayerID: " + playerId + ", Player Name: " + GetGame().GetPlayerManager().GetPlayerName(playerId));
			return false;
		}

		if (killerFactionComp.GetAffiliatedFaction() == playerFactionComp.GetAffiliatedFaction())
		{
			return true;
		}

		return false;
	}
}
