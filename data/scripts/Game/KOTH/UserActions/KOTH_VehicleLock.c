class KOTH_VehicleLock : ScriptedUserAction
{
	const float DEFAULT_LIFE_TIME = 90;
	const float LOCKED_LIFE_TIME = 1200;

	KOTH_SCR_VehicleLockComponent m_vehLockComponent;

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		Vehicle vehicle = Vehicle.Cast(SCR_EntityHelper.GetMainParent(GetOwner(), true));
		if (!vehicle)
			return;
		
		if (!Replication.IsServer())
			return;

		if (!m_vehLockComponent)
			m_vehLockComponent = KOTH_SCR_VehicleLockComponent.Cast(vehicle.FindComponent(KOTH_SCR_VehicleLockComponent));
		
		m_vehLockComponent.ToggleLock();

		if (m_vehLockComponent.IsLocked()) 
		{
			LockVehicle(vehicle);
		}
		else
		{
			UnlockVehicle(vehicle);
		}
	}

	void OnCompartmentLeft()
	{
		GetGame().GetCallqueue().CallLater(OnCompartmentLeftDelayed, 1000);
	}
	
	void OnCompartmentLeftDelayed()
	{
		if (!Replication.IsServer())
			return;

		Vehicle vehicle = Vehicle.Cast(SCR_EntityHelper.GetMainParent(GetOwner(), true));
		if (!m_vehLockComponent)
			m_vehLockComponent = KOTH_SCR_VehicleLockComponent.Cast(vehicle.FindComponent(KOTH_SCR_VehicleLockComponent));

		GarbageSystem gbSystem = ChimeraWorld.CastFrom(GetGame().GetWorld()).GetGarbageSystem();
		if (gbSystem.IsInserted(vehicle) && m_vehLockComponent.IsLocked())
			gbSystem.Bump(vehicle, LOCKED_LIFE_TIME);
	}
	
	protected void UnlockVehicle(Vehicle vehicle)
	{
		if (!Replication.IsServer())
			return;
		
		if (vehicle.IsOccupied())
			return;
		
		GarbageSystem gbSystem = ChimeraWorld.CastFrom(GetGame().GetWorld()).GetGarbageSystem();
		gbSystem.Withdraw(vehicle);
		gbSystem.Insert(vehicle);	
	}

	protected void LockVehicle(Vehicle vehicle)
	{
		if (!Replication.IsServer())
			return;		

		if (vehicle.IsOccupied())
			return;

		GarbageSystem gbSystem = ChimeraWorld.CastFrom(GetGame().GetWorld()).GetGarbageSystem();
		gbSystem.Withdraw(vehicle);
		gbSystem.Insert(vehicle);
		gbSystem.Bump(vehicle, LOCKED_LIFE_TIME);
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		Vehicle vehicle = Vehicle.Cast(SCR_EntityHelper.GetMainParent(GetOwner(), true));

		if (!vehicle)
			return false;

		if (!m_vehLockComponent)
		{
			m_vehLockComponent = KOTH_SCR_VehicleLockComponent.Cast(vehicle.FindComponent(KOTH_SCR_VehicleLockComponent));
			return false;
		}
		
		UIInfo ui = GetUIInfo();
		if (ui && m_vehLockComponent.IsLocked())
			ui.SetName("Unlock");
		
		if (ui && m_vehLockComponent.IsLocked() == false)
			ui.SetName("Lock");

		KOTH_SCR_PlayerProfileComponent playerProfile = KOTH_SCR_PlayerProfileComponent.Cast(GetGame().GetPlayerController().FindComponent(KOTH_SCR_PlayerProfileComponent));
		if (vehicle.GetOwnerUID() == playerProfile.GetMyPlayerUID())
			return true;
		
		return false;
	}
}
