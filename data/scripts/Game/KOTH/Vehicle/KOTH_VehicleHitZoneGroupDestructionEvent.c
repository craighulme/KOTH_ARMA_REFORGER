class VehicleHitZoneGroupDestructionEvent
{
    ref map<int, ref DamageInfo> HitZoneGroupDamages;
    string VehicleTypeString;
    Faction VehicleFaction;
    IEntity Pilot;
    string HitZoneGroup;
    KOTH_VehicleDamageTracker Tracker;
	Vehicle vehicle;

    void VehicleHitZoneGroupDestructionEvent(map<int, ref DamageInfo> hitZoneGroupDamages, string vehicleTypeString, Faction vehicleFaction, IEntity pilot, string hitZoneGroup, Vehicle passedVehicle)
    {
        HitZoneGroupDamages = hitZoneGroupDamages;
        VehicleTypeString = vehicleTypeString;
        VehicleFaction = vehicleFaction;
        Pilot = pilot;
        HitZoneGroup = hitZoneGroup;
		vehicle = passedVehicle;
    }

    int GetMaxDamagePlayerID()
    {
        float maxDamage = 0;
        int maxDamagePlayerID = -1;

        foreach (int playerID, DamageInfo damageInfo : HitZoneGroupDamages)
        {
            if (damageInfo != null && damageInfo.PlayerID != -1 && damageInfo.Damage > maxDamage)
            {
                maxDamage = damageInfo.Damage;
                maxDamagePlayerID = damageInfo.PlayerID;

                LogWorkbench("MaxDamagePlayerID updated: PlayerID = " + maxDamagePlayerID + ", Damage = " + maxDamage);
            }
        }

        // If maxDamagePlayerID is -1, find the player most likely caused the collateral damage
        if (maxDamagePlayerID == -1 && HitZoneGroupDamages.Count() > 0)
        {
            maxDamagePlayerID = GetLikelyCollateralDamageCauser();
        }

        return maxDamagePlayerID;
    }

    int GetLikelyCollateralDamageCauser()
    {
        int lastPlayerID = -1;
        float lastDamage = 0;

        foreach (int playerID, DamageInfo damageInfo : HitZoneGroupDamages)
        {
            // Keep track of the last player who dealt damage
            if (damageInfo != null && damageInfo.PlayerID != -1 && damageInfo.Damage > lastDamage)
            {
                lastDamage = damageInfo.Damage;
                lastPlayerID = damageInfo.PlayerID;
            }
        }

        return lastPlayerID;
    }
}

class DamageInfo
{
    int PlayerID;
    float Damage;

    void DamageInfo(int playerID, float damage)
    {
        PlayerID = playerID;
        Damage = damage;
    }
}
