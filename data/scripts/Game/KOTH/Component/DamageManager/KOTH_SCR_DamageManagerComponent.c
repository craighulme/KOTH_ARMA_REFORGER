modded class SCR_DamageManagerComponent : DamageManagerComponent
{
    override bool HijackDamageHandling(notnull BaseDamageContext damageContext)
    {
        CorrectVehicleInstigator(damageContext);

        // Call the original
        return super.HijackDamageHandling(damageContext);
    }

    // Corrects the instigator when the source is a vehicle weapon
    private void CorrectVehicleInstigator(notnull BaseDamageContext damageContext)
    {
        // Only process if the instigator has no player ID and is a vehicle
        if (!damageContext.instigator.GetInstigatorPlayerID())
        {
            IEntity instigatorEntity = damageContext.instigator.GetInstigatorEntity();
            Vehicle vehicleInstigator = Vehicle.Cast(instigatorEntity);

            if (vehicleInstigator)
            {
                int killerPilotID = vehicleInstigator.GetPilotID();

                // Ensure the pilot ID is valid and not the same as the victim's
                if (killerPilotID > 0 && killerPilotID != GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(GetOwner()))
                {
                    IEntity pilotEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(killerPilotID);
                    if (pilotEntity)
                    {
                        damageContext.instigator = Instigator.CreateInstigator(pilotEntity);
                    }
                }
            }
        }
    }
}