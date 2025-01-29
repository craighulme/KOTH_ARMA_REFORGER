modded class Vehicle : BaseVehicle
{
    SCR_BaseGameMode m_gameMode;

    SCR_VehicleDamageManagerComponent m_vehdamageManager;
    SCR_WheeledDamageManagerComponent m_wheeleddamageManager;
    SCR_HelicopterDamageManagerComponent m_helodamageManager;
    SCR_RotorDamageManagerComponent m_rotorMaindamageManager;
    SCR_RotorDamageManagerComponent m_rotorTaildamageManager;
	KOTH_SCR_VehicleLockComponent m_vehLockComponent;

    ref array<SCR_DamageManagerComponent> m_wheelsDamageManagers = {};

    // For vehicle damage tracking and xp
    ref private KOTH_VehicleDamageTracker vehicleDamageTracker;

    // Track pilot id even if they die before an occupant
    protected int m_PilotID = -1;
	
	// Track buyer for lock action and possible refund at end game
	[RplProp()]
    protected string m_OwnerUID = string.Empty;
    string GetOwnerUID() { return m_OwnerUID; }
    void SetOwnerUID(string uid) { 
		m_OwnerUID = uid; 
		Replication.BumpMe(); 
	}

    void ~Vehicle()
    {
        GetGame().GetCallqueue().Remove(HandleVehicleProtection);
    }

    override void EOnInit(IEntity owner)
    {
        super.EOnInit(owner);

        if (SCR_Global.IsEditMode(owner))
            return;

        if (!Replication.IsServer())
            return;

        m_gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

        m_vehdamageManager = SCR_VehicleDamageManagerComponent.Cast(FindComponent(SCR_VehicleDamageManagerComponent));
        m_helodamageManager = SCR_HelicopterDamageManagerComponent.Cast(FindComponent(SCR_HelicopterDamageManagerComponent));

        // Find wheels dmg managers
        SlotManagerComponent slotmanager = SlotManagerComponent.Cast(FindComponent(SlotManagerComponent));
        array<EntitySlotInfo> outSlotInfos = {};
        slotmanager.GetSlotInfos(outSlotInfos);
        foreach(EntitySlotInfo slot : outSlotInfos)
        {
            if (SCR_WheelSlotInfo.Cast(slot))
            {
                SCR_DamageManagerComponent dmgManager = SCR_DamageManagerComponent.Cast(slot.GetAttachedEntity().FindComponent(SCR_DamageManagerComponent));
                if (dmgManager)
                    m_wheelsDamageManagers.Insert(dmgManager);
            }
        }

        // Find rotors dmg manager
        SlotManagerComponent slotManager = SlotManagerComponent.Cast(FindComponent(SlotManagerComponent));
        if (slotManager)
        {
            EntitySlotInfo slotMain = slotManager.GetSlotByName("RotorMain");
            if (slotMain)
            {
                IEntity rotor = slotMain.GetAttachedEntity();
                m_rotorMaindamageManager = SCR_RotorDamageManagerComponent.Cast(rotor.FindComponent(SCR_RotorDamageManagerComponent));
            }
            EntitySlotInfo slotTail = slotManager.GetSlotByName("RotorTail");
            if (slotTail)
            {
                IEntity rotor = slotTail.GetAttachedEntity();
                m_rotorTaildamageManager = SCR_RotorDamageManagerComponent.Cast(rotor.FindComponent(SCR_RotorDamageManagerComponent));
            }
        }

        GetGame().GetCallqueue().CallLater(HandleVehicleProtection, 500, true);

        BaseVehicleControllerComponent vehControllerComp = BaseVehicleControllerComponent.Cast(FindComponent(BaseVehicleControllerComponent));
        vehControllerComp.LockPilotControls(true);

        // Initialize vehicleDamageTracker
        vehicleDamageTracker = new KOTH_VehicleDamageTracker();

        // Set event handlers to track vehicle damage
        RegisterKOTHVehicleDamageHandlers();

        // Only initialized once inside class so doesn't matter its per vehicle
        KOTH_VehicleScoring.Init();

		m_gameMode.AddVehicletoRefund(this);
		m_vehLockComponent = KOTH_SCR_VehicleLockComponent.Cast(FindComponent(KOTH_SCR_VehicleLockComponent));
		
		#ifdef WORKBENCH
		GetGame().GetCallqueue().CallLater(DebugRemain, 10000, true);
		#endif
    }
	
	protected void DebugRemain()
	{
		GarbageSystem gbSystem = ChimeraWorld.CastFrom(GetGame().GetWorld()).GetGarbageSystem();
		float remainingTime = gbSystem.GetRemainingLifetime(this);
		
		if (m_OwnerUID != string.Empty)
		{
			Log(m_OwnerUID+" vehicle garbage remainingTime "+remainingTime);
		}
		else 
		{
			Log("free vehicle garbage remainingTime "+remainingTime);
		}
	}
	
    protected void HandleVehicleProtection()
    {
        Faction faction = GetFaction();
        if (!faction)
        {
            if (m_gameMode.m_firstProtect.QueryEntityInside(this))
            {
                EnableDamages(false);
                return;
            }

            if (m_gameMode.m_secondProtect.QueryEntityInside(this))
            {
                EnableDamages(false);
                return;
            }

            if (m_gameMode.m_thirdProtect.QueryEntityInside(this))
            {
                EnableDamages(false);
                return;
            }

            EnableDamages(true);
            return;
        }

        if (m_gameMode.m_firstSpawnPoint.GetFactionKey() == faction.GetFactionKey()) 
        {
            if (m_gameMode.m_firstProtect.QueryEntityInside(this))
            {
                EnableDamages(false);
                return;
            }
        }
        if (m_gameMode.m_secondSpawnPoint.GetFactionKey() == faction.GetFactionKey()) 
        {
            if (m_gameMode.m_secondProtect.QueryEntityInside(this))
            {
                EnableDamages(false);
                return;
            }
        }
        if (m_gameMode.m_thirdSpawnPoint.GetFactionKey() == faction.GetFactionKey()) 
        {
            if (m_gameMode.m_thirdProtect.QueryEntityInside(this))
            {
                EnableDamages(false);
                return;
            }
        }

        EnableDamages(true);
    }

    protected void EnableDamages(bool shouldEnable)
    {
        if (m_vehdamageManager)
        {
            if (!shouldEnable && m_vehdamageManager.IsDamageHandlingEnabled() || shouldEnable && !m_vehdamageManager.IsDamageHandlingEnabled())
                m_vehdamageManager.EnableDamageHandling(shouldEnable);
        }

        if (m_wheeleddamageManager)
        {
            if (!shouldEnable && m_wheeleddamageManager.IsDamageHandlingEnabled() || shouldEnable && !m_wheeleddamageManager.IsDamageHandlingEnabled())
                m_wheeleddamageManager.EnableDamageHandling(shouldEnable);
        }

        if (m_rotorMaindamageManager)
        {
            if (!shouldEnable && m_rotorMaindamageManager.IsDamageHandlingEnabled() || shouldEnable && !m_rotorMaindamageManager.IsDamageHandlingEnabled())
                m_rotorMaindamageManager.EnableDamageHandling(shouldEnable);
        }
        if (m_rotorTaildamageManager)
        {
            if (!shouldEnable && m_rotorTaildamageManager.IsDamageHandlingEnabled() || shouldEnable && !m_rotorTaildamageManager.IsDamageHandlingEnabled())
                m_rotorTaildamageManager.EnableDamageHandling(shouldEnable);
        }

        if (m_helodamageManager)
        {
            if (!shouldEnable && m_helodamageManager.IsDamageHandlingEnabled() || shouldEnable && !m_helodamageManager.IsDamageHandlingEnabled())
                m_helodamageManager.EnableDamageHandling(shouldEnable);
        }

        foreach (SCR_DamageManagerComponent dmgManager : m_wheelsDamageManagers)
        {
            if (dmgManager)
            {
                if (!shouldEnable && dmgManager.IsDamageHandlingEnabled() || shouldEnable && !dmgManager.IsDamageHandlingEnabled())
                    dmgManager.EnableDamageHandling(shouldEnable);
            }
        }
    }

    protected void RegisterKOTHVehicleDamageHandlers()
    {
        // Helo considered a vehicle so only use this one for ground vehicles and helos to avoid duplication of damage events.
        if (m_vehdamageManager)
        {
            m_vehdamageManager.GetOnDamage().Insert(OnVehicleDamage);
        }

        if (m_rotorMaindamageManager)
        {
            m_rotorMaindamageManager.GetOnDamage().Insert(OnRotorMainDamage);
        }

        if (m_rotorTaildamageManager)
        {
            m_rotorTaildamageManager.GetOnDamage().Insert(OnRotorTailDamage);
        }

        // If there is a wheel manager, add a damage handler to each wheel
        if (m_wheelsDamageManagers.Count() > 0)
        {
            foreach (SCR_DamageManagerComponent dmgManager : m_wheelsDamageManagers)
            {
                if (dmgManager)
                {
                    dmgManager.GetOnDamage().Insert(OnWheelDamage);
                }
            }
        }
    }

    protected void OnVehicleDamage(BaseDamageContext damageContext)
    {
        KOTH_VehicleDamageEventHandler.KOTHOnDamage(damageContext, this, vehicleDamageTracker);
    }

    protected void OnWheelDamage(BaseDamageContext damageContext)
    {
        KOTH_VehicleDamageEventHandler.KOTHOnDamage(damageContext, this, vehicleDamageTracker);
    }

    protected void OnRotorMainDamage(BaseDamageContext damageContext)
    {
        KOTH_VehicleDamageEventHandler.KOTHOnDamage(damageContext, this, vehicleDamageTracker);
    }

    protected void OnRotorTailDamage(BaseDamageContext damageContext)
    {
        KOTH_VehicleDamageEventHandler.KOTHOnDamage(damageContext, this, vehicleDamageTracker);
    }

    void SetPilotID(int pilotId)
    {
        m_PilotID = pilotId;
    }

    int GetPilotID()
    {
        return m_PilotID;
    }
	
	//not sure if this is being returned correctly if the rotor damage cs vehicle damage
	KOTH_VehicleDamageTracker GetDamageTracker()
    {
        return vehicleDamageTracker;
    }
}
