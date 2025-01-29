modded class SCR_OpenVehicleDoorUserAction
{
	override bool CanBePerformedScript(IEntity user)
	{
		// This was turned to false because of sync issues
		return true;
	}
}
