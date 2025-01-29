class KOTH_VehicleDamageTracker
{
    private ref map<string, ref map<int, ref DamageInfo>> damageMap;
    private ref map<string, bool> processedHitZoneGroups;
    private ref map<string, bool> validVehicleHitZoneGroups;

    void KOTH_VehicleDamageTracker()
    {
        damageMap = new map<string, ref map<int, ref DamageInfo>>();
        processedHitZoneGroups = new map<string, bool>();
        validVehicleHitZoneGroups = new map<string, bool>();

        // List valid EVehicleHitZoneGroups to score
        validVehicleHitZoneGroups["WHEELS"] = true;
        validVehicleHitZoneGroups["ENGINE"] = true;
        validVehicleHitZoneGroups["ROTOR_ASSEMBLY"] = true;
        validVehicleHitZoneGroups["TAIL_ROTOR"] = true;
		validVehicleHitZoneGroups["FUEL_TANKS"] = true;
		validVehicleHitZoneGroups["HULL"] = true;
    }

    void RecordDamage(int playerID, EVehicleHitZoneGroup hitZoneGroup, float damage)
    {
        string hitZoneGroupStr = typename.EnumToString(EVehicleHitZoneGroup, hitZoneGroup);
        LogWorkbench("Attempting to record damage: PlayerID = " + playerID + ", HitZoneGroup = " + hitZoneGroupStr + ", Damage = " + damage);

        if (!validVehicleHitZoneGroups.Contains(hitZoneGroupStr) || (processedHitZoneGroups.Contains(hitZoneGroupStr) && processedHitZoneGroups[hitZoneGroupStr]))
        {
            LogWorkbench("Invalid or already processed hit zone group: " + hitZoneGroupStr);
            return;
        }

        if (damage == 0)
        {
            LogWorkbench("No damage to record.");
            return;
        }

        if (!damageMap.Contains(hitZoneGroupStr))
        {
            damageMap[hitZoneGroupStr] = new map<int, ref DamageInfo>();
        }

        map<int, ref DamageInfo> playerDamageMap = damageMap[hitZoneGroupStr];
        DamageInfo damageInfo;

        if (!playerDamageMap.Contains(playerID))
        {
            damageInfo = new DamageInfo(playerID, 0.0);
            playerDamageMap[playerID] = damageInfo;
        }
        else
        {
            damageInfo = playerDamageMap[playerID];
            if (damageInfo == null)
            {
                Log("Error: Retrieved DamageInfo is null for PlayerID = " + playerID + ", HitZoneGroup = " + hitZoneGroupStr);
                return;
            }
        }

        damageInfo.Damage = damageInfo.Damage + damage;

        LogWorkbench("Recorded damage: PlayerID = " + damageInfo.PlayerID + ", HitZoneGroup = " + hitZoneGroupStr + ", Total Damage = " + damageInfo.Damage);
    }

    ref map<int, ref DamageInfo> GetAllPlayerDamageOnHitZoneGroup(string hitZoneGroupStr)
    {
        ref map<int, ref DamageInfo> playerDamageMap;

        if (damageMap.Contains(hitZoneGroupStr))
        {
            playerDamageMap = damageMap[hitZoneGroupStr];

            #ifdef WORKBENCH
            Log("GetAllPlayerDamageOnHitZoneGroup-------------");
            foreach (int playerID, ref DamageInfo damageInfo : playerDamageMap)
            {
                if (damageInfo != null)
                {
                    Log("Returning damage: PlayerID = " + playerID + ", Damage = " + damageInfo.Damage);
                }
                else
                {
                    Log("Error: DamageInfo is null for PlayerID = " + playerID);
                }
            }
            #endif

            return playerDamageMap;
        }

        return new map<int, ref DamageInfo>();
    }

    void SetGroupHitZoneProcessed(string hitZoneGroupStr)
    {
        if (damageMap.Contains(hitZoneGroupStr))
        {
            processedHitZoneGroups[hitZoneGroupStr] = true;
        }
    }
	
	void ResetAllDamage()
    {
        damageMap.Clear();
        processedHitZoneGroups.Clear();
		
        foreach (auto key, auto value : validVehicleHitZoneGroups)
	    {
	        processedHitZoneGroups[key] = false;
	    }
    }
	

    bool IsHitZoneGroupProcessed(string hitZoneGroupStr)
    {
        return processedHitZoneGroups.Contains(hitZoneGroupStr) && processedHitZoneGroups[hitZoneGroupStr];
    }
}
