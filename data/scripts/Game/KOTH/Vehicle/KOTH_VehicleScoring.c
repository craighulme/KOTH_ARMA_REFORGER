class KOTH_VehicleScoring
{
    static ref map<string, ref VehicleScoringData> vehicleScoringData;
    static bool isInitialized = false;

    // Defined scoring values. Just need to create VehicleScoringData and define the substring of the Vehicle HitZoneGroup.
    static void Init()
	{
	    if (isInitialized)
	        return;
	
	    vehicleScoringData = new map<string, ref VehicleScoringData>();
	
	    ref VehicleScoringData carScoring = new VehicleScoringData();
	    carScoring.AddHitZoneGroupScoring("ENGINE", 100, 25);
	    carScoring.AddHitZoneGroupScoring("WHEELS", 50, 10);
	    carScoring.AddHitZoneGroupScoring("FUEL_TANKS", 75, 20);
	    vehicleScoringData["CAR"] = carScoring;
	
	    ref VehicleScoringData helicopterScoring = new VehicleScoringData();
	    helicopterScoring.AddHitZoneGroupScoring("ROTOR_ASSEMBLY", 500, 100);
	    helicopterScoring.AddHitZoneGroupScoring("TAIL_ROTOR", 300, 100);
		helicopterScoring.AddHitZoneGroupScoring("ENGINE", 200, 50);
	    helicopterScoring.AddHitZoneGroupScoring("FUEL_TANKS", 100, 50);
	    vehicleScoringData["HELICOPTER"] = helicopterScoring;
	
	    ref VehicleScoringData apcScoring = new VehicleScoringData();
	    apcScoring.AddHitZoneGroupScoring("ENGINE", 1000, 100);
	    apcScoring.AddHitZoneGroupScoring("WHEELS", 100, 25);
	    apcScoring.AddHitZoneGroupScoring("FUEL_TANKS", 100, 75);
	    vehicleScoringData["APC"] = apcScoring;
	
	    ref VehicleScoringData supplyTruckScoring = new VehicleScoringData();
	    supplyTruckScoring.AddHitZoneGroupScoring("ENGINE", 200, 50);
	    supplyTruckScoring.AddHitZoneGroupScoring("WHEELS", 50, 25);
	    supplyTruckScoring.AddHitZoneGroupScoring("FUEL_TANKS", 100, 30);
	    vehicleScoringData["SUPPLY_TRUCK"] = supplyTruckScoring;
	
	    isInitialized = true;
	}

    static int GetXPForHitZoneGroup(string vehicleType, string hitZoneGroup)
    {
        Init(); // Ensure initialization is done once
        VehicleScoringData scoringData = vehicleScoringData.Get(vehicleType);
        if (scoringData)
        {
            VehicleHitZoneGroupScoring hitGroupScoring = scoringData.GetHitZoneGroupScoring(hitZoneGroup);
            if (hitGroupScoring)
            {
                return hitGroupScoring.xp;
            }
        }
        return 0;
    }

    static int GetAssistXPForHitZoneGroup(string vehicleType, string hitZoneGroup)
    {
        Init(); // Ensure initialization is done once
        VehicleScoringData scoringData = vehicleScoringData.Get(vehicleType);
        if (scoringData)
        {
            VehicleHitZoneGroupScoring hitGroupScoring = scoringData.GetHitZoneGroupScoring(hitZoneGroup);
            if (hitGroupScoring)
            {
                return hitGroupScoring.assistXP;
            }
        }
        return 0;
    }

    // Test method to print out the scoring to see if it is instantiated correctly
    static void PrintScoringData()
    {
        Init(); // Ensure initialization is done once
        foreach (string vehicleType, VehicleScoringData scoringData : vehicleScoringData)
        {
            Log("Vehicle Type: " + vehicleType);
            foreach (string hitZoneGroup, VehicleHitZoneGroupScoring hitGroupScoring : scoringData.hitZoneGroupScoring)
            {
                Log("  HitZoneGroup: " + hitZoneGroup);
                Log("    XP: " + hitGroupScoring.xp);
                Log("    Assist XP: " + hitGroupScoring.assistXP);
            }
        }
    }
}

// Structure Class Used For Scoring
class VehicleHitZoneGroupScoring
{
    int xp;
    int assistXP;

    void VehicleHitZoneGroupScoring(int _xp, int _assistXP)
    {
        this.xp = _xp;
        this.assistXP = _assistXP;
    }
}

// Structure Class Used For Scoring
class VehicleScoringData
{
    ref map<string, ref VehicleHitZoneGroupScoring> hitZoneGroupScoring;

    void VehicleScoringData()
    {
        hitZoneGroupScoring = new map<string, ref VehicleHitZoneGroupScoring>();
    }

    void AddHitZoneGroupScoring(string hitZoneGroup, int xp, int assistXP)
    {
        hitZoneGroupScoring[hitZoneGroup] = new VehicleHitZoneGroupScoring(xp, assistXP);
    }

    VehicleHitZoneGroupScoring GetHitZoneGroupScoring(string hitZoneGroup)
    {
        return hitZoneGroupScoring[hitZoneGroup];
    }
}
