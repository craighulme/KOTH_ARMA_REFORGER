modded class SCR_AIInfoComponent : SCR_AIInfoBaseComponent
{

	//fix null error on chimeracharacter
	override void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState != ECharacterLifeState.INCAPACITATED)
			RemoveUnitState(EUnitState.UNCONSCIOUS);
		else
			AddUnitState(EUnitState.UNCONSCIOUS);
		
		AIAgent agent = AIAgent.Cast(GetOwner());
		IEntity vehicle;
		// find which vehicle this agent is in
		if (HasUnitState(EUnitState.IN_VEHICLE))
		{
			if(!agent) return;  //add null guard. caused high script usage
			
			ChimeraCharacter ent = ChimeraCharacter.Cast(agent.GetControlledEntity());
			CompartmentAccessComponent compartComp = ent.GetCompartmentAccessComponent();
			vehicle = compartComp.GetVehicleIn(ent);
			
		}
		m_OnAgentLifeStateChanged.Invoke(agent, this, vehicle, newLifeState);
	}
}