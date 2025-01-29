modded class SCR_DestructibleHitzone : SCR_HitZone
{
	override void StartDestruction(bool immediate = false)
	{
	    if (m_CompartmentManager && GetHitZoneGroup() == EVehicleHitZoneGroup.HULL)
	    {
			Log("StartDestruction: Owner of GetHitZoneGroup Hull is : " + GetOwner().ToString());
			Vehicle vehicle = Vehicle.Cast(GetOwner());
			SCR_HelicopterDamageManagerComponent isHeli = SCR_HelicopterDamageManagerComponent.Cast(vehicle.FindComponent(SCR_HelicopterDamageManagerComponent));
			
	        if (!isHeli)
			{
				m_CompartmentManager.EjectRandomOccupants(-1, true);
			}
			else
			{
				LogWorkbench("StartDestruction: This is a heli, not ejecting random occupants");
				
				array<BaseCompartmentSlot> compartments = {};
		        m_CompartmentManager.GetCompartments(compartments);
		
		        // Kill all occupants
		        foreach (BaseCompartmentSlot compartment : compartments)
		        {
		            ChimeraCharacter occupant = ChimeraCharacter.Cast(compartment.GetOccupant());
		            if (occupant)
		            {
		                SCR_DamageManagerComponent damageManager = occupant.GetDamageManager();
		                if (damageManager)
		                {
							//instigator will be checked with vehicle damage tracker later so just use something temp
		                    damageManager.Kill(Instigator.CreateInstigator(occupant));
		                }
		            }
		        }
			}
	    }
	
	    if (m_pDestructionHandler)
	        m_pDestructionHandler.StartDestruction(immediate);
	}
}