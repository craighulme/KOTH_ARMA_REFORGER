class KOTH_EjectAllOccupantsUserAction : SCR_ScriptedUserAction 
{
	protected const float MAX_GETOUT_SPEED_METER_PER_SEC_SQ = 18;
	protected const float MAX_GETOUT_ALTITUDE_AGL_METERS = 5;
	
	protected Vehicle m_vehicle;

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// Get the m_vehicle the player is currently in
		m_vehicle = Vehicle.Cast(SCR_EntityHelper.GetMainParent(GetOwner(), true));
		if (!m_vehicle)
			return;

		// Eject all occupants from the m_vehicle
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(m_vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
		{
			IEntity parent = m_vehicle.GetParent();
			if (parent)
				compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(parent.FindComponent(SCR_BaseCompartmentManagerComponent));

			if (!compartmentManager)
				return;
		}

		array<IEntity> occupants = new array<IEntity>();
		compartmentManager.GetOccupants(occupants);
		EjectOccupants(occupants);
	}

	override bool CanBeShownScript(IEntity user)
	{
		m_vehicle = Vehicle.Cast(SCR_EntityHelper.GetMainParent(GetOwner(), true));

		if (!m_vehicle)
			return false;

		// Allow the action to be shown only if the player is the pilot of the m_vehicle
		IEntity pilot = m_vehicle.GetPilot();
		if (pilot != user)
			return false;
	
		//Only allow the action to be seen if you're under a certain speed/height
		if (!m_vehicle)
			return false;
		
		Physics phys = m_vehicle.GetPhysics();
		if (!phys)
			return false;

		if ((phys.GetVelocity().LengthSq()) > MAX_GETOUT_SPEED_METER_PER_SEC_SQ)
			return false;

		// Disallow GetOut when flying is above X meters
		HelicopterControllerComponent helicopterController = HelicopterControllerComponent.Cast(m_vehicle.GetVehicleController());
		if (!helicopterController)
			return false;
		
		VehicleHelicopterSimulation simulation = VehicleHelicopterSimulation.Cast(helicopterController.GetBaseSimulation());
		if (simulation && simulation.GetAltitudeAGL() > MAX_GETOUT_ALTITUDE_AGL_METERS)
			return false;

		return true;
	}

	protected void EjectOccupants(array<IEntity> occupants) 
	{
		foreach (IEntity occupant : occupants) 
		{
			ChimeraCharacter occupantCharacter = ChimeraCharacter.Cast(occupant);
			if (!occupantCharacter)
				return;
			
			IEntity pilot = m_vehicle.GetPilot();
			if (!pilot || pilot == occupantCharacter)
				continue;
			
			CompartmentAccessComponent seat = occupantCharacter.GetCompartmentAccessComponent();
			if (!seat)
				return;

			seat.GetOutVehicle(EGetOutType.TELEPORT, 0, ECloseDoorAfterActions.INVALID, true);
		}
	}
}
