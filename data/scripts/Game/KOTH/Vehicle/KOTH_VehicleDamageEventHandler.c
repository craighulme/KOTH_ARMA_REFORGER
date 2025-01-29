class KOTH_VehicleDamageEventHandler
{
    static KOTH_VehicleDamageTracker damageTracker;
    static PlayerManager playerManager;
    static KOTH_VehicleEventsGameModeComponent vehEventComp;
	
    void KOTH_VehicleDamageEventHandler(KOTH_VehicleDamageTracker tracker)
    {
        damageTracker = tracker;
        playerManager = GetGame().GetPlayerManager();
        vehEventComp = KOTH_VehicleEventsGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_VehicleEventsGameModeComponent));
    }

    static void KOTHOnDamage(BaseDamageContext damageContext, Vehicle passedVehicleClass, KOTH_VehicleDamageTracker passedDamageTracker)
	{
	    if (damageContext.damageType == EDamageType.COLLISION || !IsValidDamageContext(damageContext) || !passedVehicleClass.GetPilot())
	        return;
	
	    damageTracker = passedDamageTracker;
	
	    // Cast hitzone into SCR_DestructibleHitzone to find EVehicleHitZoneGroup
	    SCR_DestructibleHitzone destructHitZoneComp = SCR_DestructibleHitzone.Cast(damageContext.struckHitZone);
	    SCR_RotorHitZone rotorHitZoneComp = SCR_RotorHitZone.Cast(destructHitZoneComp);
	    EVehicleHitZoneGroup hitZoneGroup = destructHitZoneComp.GetHitZoneGroup();
	
	    if (!hitZoneGroup)
	    {
	        LogWorkbench("Not a valid EVehicleHitZoneGroup can be cast from Hit Zone: " + damageContext.struckHitZone.GetName());
	        return;
	    }
	
	    string hitZoneGroupStr = typename.EnumToString(EVehicleHitZoneGroup, hitZoneGroup);
		LogWorkbench("===========================KOTH On Damage================================");
	    LogWorkbench("Processing hit zone group: " + hitZoneGroupStr);
		LogWorkbench("Hitzone for damaged part is: " + damageContext.struckHitZone.GetName());
		LogWorkbench("DamageState for damaged part is: " + typename.EnumToString(EDamageState, damageContext.struckHitZone.GetDamageState()));
	
	    if (damageContext.struckHitZone.GetDamageState() == EDamageState.DESTROYED && damageTracker.IsHitZoneGroupProcessed(hitZoneGroupStr))
	        return;
	
	    int playerID = damageContext.instigator.GetInstigatorPlayerID();
	    if (playerID != -1 && playerID != 0)
		{
		    // Record damage if the player ID is valid
		    damageTracker.RecordDamage(playerID, hitZoneGroup, damageContext.damageValue);
		}
		else if (damageContext.instigator && damageContext.instigator.GetInstigatorEntity())
		{
		    // Handle cases where the killerEntity exists but playerID is invalid like vehicle weapons
		   	Vehicle killerVehicle = Vehicle.Cast(damageContext.instigator.GetInstigatorEntity());
			
			//check valid vehicle and not same vehicle
			if(!killerVehicle || killerVehicle == passedVehicleClass)
				return;

	        int resolvedPlayerID = killerVehicle.GetPilotID();
	        if (resolvedPlayerID != -1 && resolvedPlayerID != 0)
	        {
	            damageTracker.RecordDamage(resolvedPlayerID, hitZoneGroup, damageContext.damageValue);
	        }
		}
			
	    // Handle rotor-specific logic and its not already destroyed
	    if (rotorHitZoneComp && rotorHitZoneComp.GetDamageState() != EDamageState.DESTROYED)
	    {
	        LogWorkbench("Rotor current state: " + typename.EnumToString(SCR_ERotorDamageState, rotorHitZoneComp.GetDamageState()));
	
	        GetGame().GetCallqueue().CallLater(ProcessRotorDestruction, 30, false, passedVehicleClass, damageContext, rotorHitZoneComp);
	    }	
	    else
	    {
	        // Schedule a deferred state check for general parts
	        GetGame().GetCallqueue().CallLater(ProcessDeferredDestruction, 30, false, passedVehicleClass, damageContext, destructHitZoneComp, hitZoneGroupStr, hitZoneGroup);
	    }
	}
	
	static void ProcessRotorDestruction(Vehicle passedVehicleClass, BaseDamageContext damageContext, SCR_RotorHitZone rotorHitZoneComp)
	{
	    SCR_ERotorDamageState updatedState = rotorHitZoneComp.GetDamageState();
	
	    if (updatedState == SCR_ERotorDamageState.DESTROYED)
	    {
	        // Handle RotorMain destruction logic
	        if (KOTH_Helper.ToUpper(rotorHitZoneComp.GetName()) == "ROTORMAIN")
	        {
	            // Ensure RotorAssembly is marked as destroyed and scored
	            if (!damageTracker.IsHitZoneGroupProcessed("ROTOR_ASSEMBLY"))
	            {
	                LogWorkbench("Marking RotorAssembly as destroyed and scoring.");
	                OnDestroyedHitZoneGroup(damageContext, passedVehicleClass, EVehicleHitZoneGroup.ROTOR_ASSEMBLY);
	            }
	        }
	        
	        // Handle TailRotor destruction logic
	        if (KOTH_Helper.ToUpper(rotorHitZoneComp.GetName()) == "ROTORTAIL")
	        {
	            // Ensure TailRotor is marked as destroyed and scored
	            if (!damageTracker.IsHitZoneGroupProcessed("TAIL_ROTOR"))
	            {
	                LogWorkbench("Marking TailRotor as destroyed and scoring.");
	                OnDestroyedHitZoneGroup(damageContext, passedVehicleClass, EVehicleHitZoneGroup.TAIL_ROTOR);
	            }
	        }
	    }
	}
	
	static void ProcessDeferredDestruction(Vehicle passedVehicleClass, BaseDamageContext damageContext, SCR_DestructibleHitzone destructHitZoneComp, string hitZoneGroupStr, EVehicleHitZoneGroup hitZoneGroup)
	{
	    EDamageState updatedState = destructHitZoneComp.GetDamageState();
	
	    if (updatedState == EDamageState.DESTROYED)
	    {
	        if (!damageTracker.IsHitZoneGroupProcessed(hitZoneGroupStr))
	        {
	            LogWorkbench("Marking " + hitZoneGroupStr + " as destroyed and scoring.");
	            OnDestroyedHitZoneGroup(damageContext, passedVehicleClass, hitZoneGroup);
	        }
	    }
	}

    static void OnDestroyedHitZoneGroup(BaseDamageContext damageContext, Vehicle vehicleClass, EVehicleHitZoneGroup hitZoneGroup)
    {
        string hitZoneGroupStr = typename.EnumToString(EVehicleHitZoneGroup, hitZoneGroup);
		LogWorkbench("Hitzone for destroyed part is: " + damageContext.struckHitZone.GetName());
		LogWorkbench("HitZoneGroup for destroyed part is: " + hitZoneGroupStr);
		
        map<int, ref DamageInfo> vehHitGroupDamages = damageTracker.GetAllPlayerDamageOnHitZoneGroup(hitZoneGroupStr);
        Faction vehicleFaction = KOTH_Helper.GrabVehicleFaction(vehicleClass);
		
		vehEventComp = KOTH_VehicleEventsGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_VehicleEventsGameModeComponent));
        if (vehEventComp)
        {
            string vehicleTypeString = DetermineVehicleType(vehicleClass);
            VehicleHitZoneGroupDestructionEvent damageEvent = new VehicleHitZoneGroupDestructionEvent(vehHitGroupDamages, vehicleTypeString, vehicleFaction, vehicleClass.GetPilot(), hitZoneGroupStr, vehicleClass);
            damageEvent.Tracker = damageTracker;  
            vehEventComp.HandleVehicleGroupHitZoneDestroyed(damageEvent);
        }

        damageTracker.SetGroupHitZoneProcessed(hitZoneGroupStr);
    }

    static string DetermineVehicleType(Vehicle vehicleClass)
    {
        SCR_HelicopterDamageManagerComponent isHeli = SCR_HelicopterDamageManagerComponent.Cast(vehicleClass.FindComponent(SCR_HelicopterDamageManagerComponent));
        if (isHeli)
        {
            return "HELICOPTER";
        }
        return typename.EnumToString(EVehicleType, vehicleClass.m_eVehicleType);
    }

    static bool IsValidDamageContext(BaseDamageContext damageContext)
    {
        return damageContext.hitEntity != null && damageContext.instigator != null;
    }
}
