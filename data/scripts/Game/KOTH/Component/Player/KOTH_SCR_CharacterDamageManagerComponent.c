modded class SCR_CharacterDamageManagerComponent
{
	EDamageType lastDamageType;
	int lastKillerId = -1;
	KOTH_SCR_PlayerProfileComponent playerProfileComp;
	
	event override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		GetOnDamage().Insert(OnCustomDamage);
	}
	
	void OnCustomDamage(notnull BaseDamageContext damageContext)
	{
		//added for last damage time tracking. Have to filter out bleeding as cause of death
		if(	damageContext.damageType == EDamageType.BLEEDING &&
			damageContext.damageType == EDamageType.REGENERATION &&
			damageContext.damageType == EDamageType.HEALING &&
			damageContext.damageType == EDamageType.TRUE)
			return;
		

        if (!playerProfileComp)
        {
            int ownerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(GetOwner());
            PlayerController ownerPlayerController = GetGame().GetPlayerManager().GetPlayerController(ownerId);

            if (ownerPlayerController)
            {
                playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(ownerPlayerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
            }
        }

        if (playerProfileComp)
        {
            playerProfileComp.lastDamageReceivedTime = GetGame().GetWorld().GetWorldTime();
			//LogWorkbench("Setting Last Damaged Time for " + GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(GetOwner()) + " as : " + playerProfileComp.lastDamageReceivedTime);
        }
		
        // Update last damage type
        lastDamageType = damageContext.damageType;

        // Check if the hit zone health drops below zero and store last killer
        if ((damageContext.struckHitZone.GetHealth() - damageContext.damageValue <= 0) && damageContext.instigator.GetInstigatorPlayerID() > 0)
        {
            lastKillerId = damageContext.instigator.GetInstigatorPlayerID();
            //LogWorkbench("Stored last killer as: " + lastKillerId);
        }
	}
	
	bool WasPilotKilledByAnother(int pilotID)
	{
	    // Check if the last killer ID is not the pilot ID
	    return !(lastKillerId <= 0) && lastKillerId != pilotID;
	}

}