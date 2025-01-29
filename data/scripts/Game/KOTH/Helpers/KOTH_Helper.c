class KOTH_Helper
{
	// copypasta from EPF
	static string GetPlayerUID(int playerId)
	{
		#ifdef WORKBENCH
			return string.Format("bbbbdddd-0000-0000-0000-%1", playerId.ToString(12));
		#endif
		
		if (!Replication.IsServer())
		{
			Log("GetPlayerUID can only be used on the server and after OnPlayerAuditSuccess.", LogLevel.ERROR);
			return string.Empty;
		}

		string uid = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);
		if (!uid)
		{
			if (RplSession.Mode() != RplMode.Dedicated)
			{
				// Peer tool support
				uid = string.Format("bbbbdddd-0000-0000-0000-%1", playerId.ToString(12));
			}
			else
			{
				Debug.DumpStack();
				Log("Dedicated server is not correctly configured to connect to the BI backend. playerId is "+playerId, LogLevel.ERROR);
			}
		}

		return uid;
	}
	
	static string GetAddonsGUIDs()
	{
		string addonIDs;
		
		array<string> addonsGUIDs = {};
		GameProject.GetLoadedAddons(addonsGUIDs);
		
		foreach (string GUID: addonsGUIDs)
		{
			if (!GameProject.IsVanillaAddon(GUID))
			{
				if (!addonIDs.IsEmpty())
					addonIDs += ",";
				
				addonIDs += GUID;
			}
		}
		
		return addonIDs;
	}
	
	static array<int> GetUniqueRandomInts(int totalNumberNeeded)
	{
		array<int> valueUsed = {};
		for (int i = 0; i < 100000; i++)
		{
			int randomInt = Math.RandomInt(0, 3);
			if (false == valueUsed.Contains(randomInt))
			{
				valueUsed.Insert(randomInt);
				if (valueUsed.Count() == totalNumberNeeded)
				{
					break;
				}
			}
		}

		return valueUsed;
	}
	
	static int ComputeVehicleRearmPrice(int playerId)
	{
		IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!playerEnt)
			return null;
		
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return null;
		
		SCR_CompartmentAccessComponent compartmentAccessComp = SCR_CompartmentAccessComponent.Cast(playerEnt.FindComponent(SCR_CompartmentAccessComponent));
		BaseCompartmentSlot compartment = compartmentAccessComp.GetCompartment();
		if (!compartment)
			return null;
		
		IEntity vehicle = compartment.GetVehicle();
		bool canRearm = false;
		
		KOTH_SCR_PlayerShopComponent playerShopComp = KOTH_SCR_PlayerShopComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerShopComponent));
		KOTH_ShopItem item = playerShopComp.FindShopItemByResourceName(vehicle.GetPrefabData().GetPrefabName());
		SCR_VehicleInventoryStorageManagerComponent inventoryStorageManagerComp = SCR_VehicleInventoryStorageManagerComponent.Cast(vehicle.FindComponent(SCR_VehicleInventoryStorageManagerComponent));
		
		if (!item)
			return null;

		int count = inventoryStorageManagerComp.GetDepositItemCountByResource(playerEnt, item.m_magazineResource);
		
		array<Managed> allVehicleWeapSlots = {};
		vehicle.FindComponents(WeaponSlotComponent, allVehicleWeapSlots);
		
		foreach (Managed weapSlotTemp : allVehicleWeapSlots)
		{
			WeaponSlotComponent weapSlot = WeaponSlotComponent.Cast(weapSlotTemp);
			
			if(!weapSlot)
				continue;
			
			IEntity weaponEntity = weapSlot.GetWeaponEntity();
			if (weaponEntity)
			{
				WeaponComponent weaponComp = WeaponComponent.Cast(weaponEntity.FindComponent(WeaponComponent));
				if (weaponComp)
				{
					BaseMagazineComponent magazineComp = weaponComp.GetCurrentMagazine();
					if (magazineComp && magazineComp.GetAmmoCount() < magazineComp.GetMaxAmmoCount())
					{
						canRearm = true;
						break;
					}
				}
			}
		}
		
		if (count < item.m_magazineNumber)
			canRearm = true;
		
		if (item.m_secondaryMagazineResource != string.Empty)
		{
			int countSecondary = inventoryStorageManagerComp.GetDepositItemCountByResource(playerEnt, item.m_secondaryMagazineResource);
			if (countSecondary < item.m_secondaryMagazineNumber)
				canRearm = true;
		}

		if (!canRearm)
			return null;
		
		return item.m_rearmCost;
	}
	
	static int ComputeRearmPrice(IEntity playerEnt, PlayerController playerController)
	{
		if (!playerEnt)
			return null;

		KOTH_SCR_PlayerShopComponent playerShopComp = KOTH_SCR_PlayerShopComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerShopComponent));
		SCR_CharacterInventoryStorageComponent characterInventoryStorageComp = SCR_CharacterInventoryStorageComponent.Cast(playerEnt.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!characterInventoryStorageComp)
			return null;

		BaseInventoryStorageComponent inventoryStorageComponent = characterInventoryStorageComp.GetWeaponStorage();
		if (!inventoryStorageComponent)
			return null;
		
		SCR_InventoryStorageManagerComponent inventoryStorageManagerComp = SCR_InventoryStorageManagerComponent.Cast(playerEnt.FindComponent(SCR_InventoryStorageManagerComponent));
		

		int totalPrice = 0;
		IEntity ent1 = inventoryStorageComponent.GetSlot(0).GetAttachedEntity();
		if (ent1)
		{
			KOTH_ShopItem item = playerShopComp.FindShopItemByResourceName(ent1.GetPrefabData().GetPrefabName());
			int count = inventoryStorageManagerComp.GetDepositItemCountByResource(playerEnt, item.m_magazineResource);
			if (count < item.m_magazineNumber)
				totalPrice = totalPrice + item.m_rearmCost;
			
			if (item.m_secondaryMagazineResource != string.Empty)
			{
				int countSecondary = inventoryStorageManagerComp.GetDepositItemCountByResource(playerEnt, item.m_secondaryMagazineResource);
				if (countSecondary < item.m_secondaryMagazineNumber)
					totalPrice = totalPrice + item.m_rearmCost;
			}
		}
		IEntity ent2 = inventoryStorageComponent.GetSlot(1).GetAttachedEntity();
		if (ent2)
		{
			KOTH_ShopItem item = playerShopComp.FindShopItemByResourceName(ent2.GetPrefabData().GetPrefabName());
			int count = inventoryStorageManagerComp.GetDepositItemCountByResource(playerEnt, item.m_magazineResource);
			if (count < item.m_magazineNumber)
				totalPrice = totalPrice + item.m_rearmCost;
			
			if (item.m_secondaryMagazineResource != string.Empty)
			{
				int countSecondary = inventoryStorageManagerComp.GetDepositItemCountByResource(playerEnt, item.m_secondaryMagazineResource);
				if (countSecondary < item.m_secondaryMagazineNumber)
					totalPrice = totalPrice + item.m_rearmCost;
			}
		}
		IEntity ent3 = inventoryStorageComponent.GetSlot(2).GetAttachedEntity();
		if (ent3)
		{
			KOTH_ShopItem item = playerShopComp.FindShopItemByResourceName(ent3.GetPrefabData().GetPrefabName());
			int count = inventoryStorageManagerComp.GetDepositItemCountByResource(playerEnt, item.m_magazineResource);
			if (count < item.m_magazineNumber)
				totalPrice = totalPrice + item.m_rearmCost;
			
			if (item.m_secondaryMagazineResource != string.Empty)
			{
				int countSecondary = inventoryStorageManagerComp.GetDepositItemCountByResource(playerEnt, item.m_secondaryMagazineResource);
				if (countSecondary < item.m_secondaryMagazineNumber)
					totalPrice = totalPrice + item.m_rearmCost;
			}
		}

		return totalPrice;
	}
	
	static SCR_PlayerController GetPlayerControllerFromEntity(notnull IEntity userEntity)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		int playerId = playerManager.GetPlayerIdFromControlledEntity(userEntity);

		return SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));
	}
	
	static SCR_CharacterControllerComponent GetCharacterControllerFromEntity(notnull IEntity from)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(from);
		if (!character)
			return SCR_CharacterControllerComponent.Cast(from.FindComponent(SCR_CharacterControllerComponent));
		
		return SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
	}
	
	//returns the faction for player entity
	static Faction GrabFaction(IEntity entity)
	{
		if (entity != null)
		{
			FactionAffiliationComponent targetFactionComp = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
			if (targetFactionComp)
				return targetFactionComp.GetAffiliatedFaction();
		}
		
		return null;
	}
	
	//returns the faction for player ID
	static Faction GrabFactionByPlayerID(int playerID)
	{
		if (playerID != 0)
	    {
	        // Get the player controller using the player ID
	        PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
	        if (playerController)
	        {
	            // Get the controlled entity from the player controller
	            IEntity playerEntity = playerController.GetControlledEntity();
	            if (playerEntity)
	            {
	                // Get the FactionAffiliationComponent from the player entity
	                FactionAffiliationComponent targetFactionComp = FactionAffiliationComponent.Cast(playerEntity.FindComponent(FactionAffiliationComponent));
	                if (targetFactionComp)
	                {
	                    return targetFactionComp.GetAffiliatedFaction();
	                }
	            }
	        }
	    }
	
	    return null;
	}

    static Faction GrabVehicleFaction(Vehicle vehicle)
    {
        if(vehicle)
        {
            return vehicle.GetFaction();
        }
        return null;
    }
	
	static Vehicle GetVehicleFromEntity(IEntity entity)
    {
        return Vehicle.Cast(entity.GetRootParent());
    }

    // Use instead of default ToLower
    static string ToLower(string input)
    {
		int temp = input.ToLower();
        return input;
    }

    // Use instead of default ToUpper
    static string ToUpper(string input)
    {
		int temp = input.ToUpper();
        return input;
    }
}


