modded class SCR_GetInUserAction
{
	KOTH_SCR_VehicleLockComponent m_vehLockComponent;
	
	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;
		
		Vehicle vehicle = Vehicle.Cast(SCR_EntityHelper.GetMainParent(GetOwner(), true));
		if (!vehicle)
			return false;

		if (!m_vehLockComponent)
			m_vehLockComponent = KOTH_SCR_VehicleLockComponent.Cast(vehicle.FindComponent(KOTH_SCR_VehicleLockComponent));
		
		PlayerController pc = GetGame().GetPlayerController();
		KOTH_SCR_PlayerProfileComponent playerProfile = KOTH_SCR_PlayerProfileComponent.Cast(pc.FindComponent(KOTH_SCR_PlayerProfileComponent));
		
		if (m_vehLockComponent && 
			m_vehLockComponent.IsLocked() && 
			vehicle.GetOwnerUID() != playerProfile.GetMyPlayerUID()
		) {
			SetCannotPerformReason("Vehicle is locked");
			return false;
		}
		
		return true;
	}
}

