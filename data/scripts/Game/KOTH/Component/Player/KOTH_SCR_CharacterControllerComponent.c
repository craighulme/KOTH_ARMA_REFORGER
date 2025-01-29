modded class SCR_CharacterControllerComponent : CharacterControllerComponent
{
	// had to rewrite this to avoid weapon being dropped by players going uncon
    override void OnConsciousnessChanged(bool conscious)
	{
		if (GetLifeState() != ECharacterLifeState.INCAPACITATED)
			return;

		// AIControlComponent aiControl = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
		// if (!aiControl || !aiControl.IsAIActivated())
		// 	return;
		
		IEntity currentWeapon;
		BaseWeaponManagerComponent wpnMan = GetWeaponManagerComponent();
		if (wpnMan && wpnMan.GetCurrentWeapon())
			currentWeapon = wpnMan.GetCurrentWeapon().GetOwner();
				
		if (currentWeapon)
		{
			bool dropGrenade = false;
			
			SCR_CharacterCommandHandlerComponent handler = SCR_CharacterCommandHandlerComponent.Cast(GetAnimationComponent().GetCommandHandler());
			
			EWeaponType wt = wpnMan.GetCurrentWeapon().GetWeaponType();
			if (currentWeapon.FindComponent(GrenadeMoveComponent))
			{
				BaseTriggerComponent triggerComp = BaseTriggerComponent.Cast(currentWeapon.FindComponent(BaseTriggerComponent));
				
				if ((triggerComp && triggerComp.WasTriggered()) || (handler && handler.IsThrowingAction()))
					dropGrenade = true;
			}
			
			if (dropGrenade) {
				handler.DropLiveGrenadeFromHand(false); 
			} else {
				TryEquipRightHandItem(null, EEquipItemType.EEquipTypeUnarmedContextual, true);
			} 
		}
	}
	
	//try to fix switching sights to a weapon with rails or an iron sight if an optic is attached.
	override void SetNextSights(int direction = 1)
	{
		// Check if we are in a turret and switch the turret's optics instead
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetOwner());
		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (compartmentAccess)
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
			{
				TurretControllerComponent turretController = TurretControllerComponent.Cast(compartment.GetController());
				if (turretController)
				{
					TurretComponent turretComponent = turretController.GetTurretComponent();
					if (turretComponent)
					{
						if (direction > 0) {
							turretComponent.SwitchNextSights();
						} else {
							turretComponent.SwitchPrevSights();
						}

						return;
					}
				}
			}
		}
		
		BaseWeaponManagerComponent weaponManager = GetWeaponManagerComponent();
		if (!weaponManager)
			return;

		BaseWeaponComponent weaponComponent = weaponManager.GetCurrentWeapon();
		if (!weaponComponent)
			return;

		int maxAvailableSightIndeces = weaponComponent.FindAvailableSights();
		int currentSightIndex = weaponManager.GetCurrentSightsIndex();
				
		if (direction > 0)
	    {
	        if (HasOptic(weaponComponent))
	        {
	            // Skip to the next sight. index 0 is skipped if its an iron sight
	            int nextIndex = currentSightIndex + 1;
	            if (nextIndex > maxAvailableSightIndeces)
	            {
	                nextIndex = 0;
	            }
	
	            if (IsValidOptic(nextIndex, weaponComponent))
	            {
	                weaponComponent.SetSights(nextIndex);
	            }
	            else
	            {
	                weaponComponent.SetSights(1);
					LogWorkbench("Detected non optic sight when optic attached, skipping rail or iron sight");
	            }
	        }
	        else
	        {
	            weaponComponent.SwitchNextSights();
	        }
	    }
	    else
	    {
	        weaponComponent.SwitchPrevSights();
	    }
	
	    //LogSightsInfo(weaponManager, weaponComponent);
	}
	
	// Check if the weapon has at least one valid optic
	bool HasOptic(BaseWeaponComponent weaponComponent)
	{
	    int availableSights = weaponComponent.FindAvailableSights();
	    for (int i = 0; i < availableSights; i++)
	    {
	        if (IsValidOptic(i, weaponComponent))
	            return true;
	    }
	    return false;
	}
	
	// Check if the sight has the SCR_2DPIPSightsComponent (not a rail or iron sight)
	bool IsValidOptic(int sightIndex, BaseWeaponComponent weaponComponent)
	{
	    BaseSightsComponent sightComp = weaponComponent.GetSightsAt(sightIndex);
	    return SCR_2DPIPSightsComponent.Cast(sightComp) != null;
	}
	
	void LogSightsInfo(BaseWeaponManagerComponent weaponManager, BaseWeaponComponent weaponComponent)
	{
	    LogWorkbench("Available Sights = " + weaponComponent.FindAvailableSights());
	    LogWorkbench("Current Sight Index = " + weaponManager.GetCurrentSightsIndex());
	}

}