class KOTH_SCR_PlayerShopComponentClass : ScriptComponentClass {}
class KOTH_SCR_PlayerShopComponent : ScriptComponent 
{
	protected KOTH_ScoringGameModeComponent m_scoreComp;
	protected KOTH_SessionDataGameModeComponent m_sessionData;
	protected KOTH_BackendApiGameModeComponent m_kothBackendApi;
	protected ref array<string> m_shopItemListResources = {};
	
	string m_permanentBuyResourceNameConfirm;

	ref KOTH_SessionPlayerLoadout m_sessionLoadout;

	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;
		
		m_kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
		m_sessionData = KOTH_SessionDataGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_SessionDataGameModeComponent));
		m_scoreComp = KOTH_ScoringGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_ScoringGameModeComponent));
		PlayerController controller = GetGame().GetPlayerController();

		m_shopItemListResources.Insert(m_scoreComp.GetWeaponShopItemList());
		m_shopItemListResources.Insert(m_scoreComp.GetOpticsShopItemList());
		m_shopItemListResources.Insert(m_scoreComp.GetExplosiveShopItemList());
		m_shopItemListResources.Insert(m_scoreComp.GetAccessoryShopItemList());
		m_shopItemListResources.Insert(m_scoreComp.GetVehicleShopItemList());
		if (m_scoreComp.GetMuzzleShopItemList())
			m_shopItemListResources.Insert(m_scoreComp.GetMuzzleShopItemList());
	}
	
	bool SpawnVehicle(string resourceName, int playerId, bool isArmed = false)
	{
		KOTH_SpawnPrefab firstSpawn = KOTH_SpawnPrefab.Cast(GetGame().GetWorld().FindEntityByName("KOTH_FirstVehicleSpawn"));
		KOTH_SpawnPrefab secondSpawn = KOTH_SpawnPrefab.Cast(GetGame().GetWorld().FindEntityByName("KOTH_SecondVehicleSpawn"));
		KOTH_SpawnPrefab thirdSpawn = KOTH_SpawnPrefab.Cast(GetGame().GetWorld().FindEntityByName("KOTH_ThirdVehicleSpawn"));
		if (!firstSpawn || !secondSpawn || !thirdSpawn)
			return false;

		Faction playerFaction = SCR_FactionManager.Cast(GetGame().GetFactionManager()).GetPlayerFaction(playerId);
		if (!playerFaction)
			return false;
		
		if (firstSpawn.GetFactionKey() == playerFaction.GetFactionKey())
			return firstSpawn.Spawn(resourceName, isArmed, playerId);
		
		if (secondSpawn.GetFactionKey() == playerFaction.GetFactionKey())
			return secondSpawn.Spawn(resourceName, isArmed, playerId);
		
		if (thirdSpawn.GetFactionKey() == playerFaction.GetFactionKey())
			return thirdSpawn.Spawn(resourceName, isArmed, playerId);
		
		return false;
	}
	
	bool CanBuyVehicleArmed(Faction playerFaction, KOTH_ShopItem item, int playerId)
	{
		switch (playerFaction.GetFactionKey())
		{
			case KOTH_Faction.BLUFOR:
				if (m_scoreComp.m_bluforArmedVehiclesCount >= 2)
				{
					Refund(item.m_priceOnce, playerId);
					DoRpc_Notif_Failed("cannot spawn vehicle", "too much armed vehicle in your team "+"\n"+" (max 2)");
					return false;
				}
			break;
			case KOTH_Faction.OPFOR:
				if (m_scoreComp.m_opforArmedVehiclesCount >= 2)
				{
					Refund(item.m_priceOnce, playerId);
					DoRpc_Notif_Failed("cannot spawn vehicle", "too much armed vehicle in your team "+"\n"+"(max 2)");
					return false;
				}
			break;
			case KOTH_Faction.INDFOR:
				if (m_scoreComp.m_indforArmedVehiclesCount >= 2)
				{
					Refund(item.m_priceOnce, playerId);
					DoRpc_Notif_Failed("cannot spawn vehicle", "too much armed vehicle in your team "+"\n"+" (max 2)");
					return false;
				}
			break;
			default:
				DoRpc_Notif_Failed("cannot spawn vehicle", "error: your faction was not found");
				return false;
			break;
		}
		
		return true;
	}
	
	void AddCountVehicleArmed(Faction playerFaction)
	{
		switch (playerFaction.GetFactionKey())
		{
			case KOTH_Faction.BLUFOR:
				m_scoreComp.m_bluforArmedVehiclesCount++;
			break;
			case KOTH_Faction.OPFOR:
				m_scoreComp.m_opforArmedVehiclesCount++;
			break;
			case KOTH_Faction.INDFOR:
				m_scoreComp.m_indforArmedVehiclesCount++;
			break;
			default:
				DoRpc_Notif_Failed("cannot spawn vehicle", "error: your faction was not found");
				return;
			break;
		}
	}

	// --------------------- RPC START --------------------- \\
	// ---- SERVER RPC
	void AskRpc_ClearSessionLoadout(int playerId)
	{
		Rpc(RpcAsk_ClearSessionLoadout, playerId);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_ClearSessionLoadout(int playerId)
	{
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);
		KOTH_SessionPlayerLoadout loadout = m_sessionData.m_sessionPlayersLoadout.Get(playerUID);
		loadout.Clear();
	}

	void AskRpc_BuyOrEquipSessionLoadout(int playerId)
	{
		Rpc(RpcAsk_BuyOrEquipSessionLoadout, playerId);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_BuyOrEquipSessionLoadout(int playerId)
	{
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);

		KOTH_SessionPlayerLoadout loadout = m_sessionData.m_sessionPlayersLoadout.Get(playerUID);		
		if (!loadout) {
			Log("no loadout found for "+playerUID);
			return;
		}
		
		KOTH_SessionPlayerLoadout cloneLoadout = KOTH_SessionPlayerLoadout.Cast(loadout.Clone());
		loadout.Clear();

		if (cloneLoadout.m_primary)
			BuyOrEquip(cloneLoadout.m_primary, playerId);
		
		if (cloneLoadout.m_optic)
			BuyOrEquip(cloneLoadout.m_optic, playerId);
		
		// have to delay or its not spawned
		if (cloneLoadout.m_launcher)
			GetGame().GetCallqueue().CallLater(BuyOrEquip, 4000, false, cloneLoadout.m_launcher, playerId);
			
		if (cloneLoadout.m_handgun)
			BuyOrEquip(cloneLoadout.m_handgun, playerId);

		if (cloneLoadout.m_muzzle)
			BuyOrEquip(cloneLoadout.m_muzzle, playerId);

		if (cloneLoadout.m_viperhood)
			BuyOrEquip(cloneLoadout.m_viperhood, playerId);

		if (cloneLoadout.m_rangeFinder)
			BuyOrEquip(cloneLoadout.m_rangeFinder, playerId);

		if (cloneLoadout.m_throwables)
		{
			foreach (KOTH_ShopItem item : cloneLoadout.m_throwables)
			{
				BuyOrEquip(item, playerId);
			}
		}
	}

	void BuyOrEquip(KOTH_ShopItem shopItem, int playerId)
	{
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);
		KOTH_PlayerProfileJson profile = m_kothBackendApi.m_CurrentProfileList.Get(playerUID);

		if (KOTH_CustomizabilityManager.isUnlocked(profile.m_unlockedItems, shopItem.m_itemResource))
		{
			RpcAsk_Equip(shopItem.m_itemResource, playerId);
		} else {
			RpcAsk_Buy(shopItem.m_itemResource, playerId, false);
		}
	}

	void AskRpc_Equip(string resourceName, int playerId)
	{
		Rpc(RpcAsk_Equip, resourceName, playerId);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_Equip(string resourceName, int playerId)
	{
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);
		if (!m_kothBackendApi.m_CurrentProfileList.Get(playerUID))
		{
			DoRpc_Notif_Failed("You can't equip that weapon", "you must perma buy it first");
			return;
		}
			
		KOTH_ShopItem item = FindShopItemByResourceName(resourceName);
		if (!item) {
			Log("player "+playerUID+" no item found for resourceName "+resourceName);
			return;
		}
		
		bool isSuccess = RemoveOldItemsAndAddNewOnes(item, playerId);
		if (!isSuccess) {
			DoRpc_Notif_Failed("You can't buy the weapon", "your inventory are full");
			return;
		}
		
		KOTH_ShopItem shopItem = FindShopItemByResourceName(resourceName);
		m_sessionData.SaveLoadoutChoiceInSession(shopItem, playerId, string.Empty);
	}
	
	void DoRpcBuy(string resourceName, int playerId, bool permanentBuy = false)
	{
		Rpc(RpcAsk_Buy, resourceName, playerId, permanentBuy);			
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_Buy(string resourceName, int playerId, bool permanentBuy)
	{
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);
		
		Log("----------- RpcAsk_Buy "+resourceName+" for "+playerUID+" is permanentBuy "+permanentBuy);
		KOTH_ShopItem item = FindShopItemByResourceName(resourceName);

		if (!item) {
			Log("no item found for resourceName "+resourceName);
			return;
		}
		int price = item.m_priceOnce;
		if (permanentBuy)
			price = item.m_pricePermanent;
		
		bool buySuccess = TryBuy(price, playerId);
		if (!buySuccess) {
			DoRpc_Notif_Failed("You can't buy the weapon", "not enough money");
			return;
		}

		if (permanentBuy)
		{
			KOTH_PlayerProfileJson profile = m_kothBackendApi.m_CurrentProfileList.Get(playerUID);
			profile.m_unlockedItems.Insert(item.m_itemResource);
			m_kothBackendApi.DoRpcSyncProfileToPlayer(profile);

			DoRpc_Notif_Succeed(item.m_pricePermanent);
		} 
		else 
		{
			// check category for vehicles
			// KOTH_TODO: refactor this
			switch (item.m_category) {
				case KOTH_ShopItemCategory.Vehicle:
					if (SpawnVehicle(resourceName, playerId))
					{
						DoRpc_Notif_Succeed(item.m_priceOnce);
					} else {
						Refund(item.m_priceOnce, playerId);
						DoRpc_Notif_Failed("cannot spawn vehicle", "no place found");
					}
				break;
				case KOTH_ShopItemCategory.VehicleArmed:
					Faction playerFaction = SCR_FactionManager.Cast(GetGame().GetFactionManager()).GetPlayerFaction(playerId);
					if (!playerFaction)
						return;
					
					if (!CanBuyVehicleArmed(playerFaction, item, playerId))
						return;
				
					if (SpawnVehicle(resourceName, playerId, true))
					{
						AddCountVehicleArmed(playerFaction);
						DoRpc_Notif_Succeed(item.m_priceOnce);
					} else {
						Refund(item.m_priceOnce, playerId);
						DoRpc_Notif_Failed("cannot spawn vehicle", "no place found");
					}
				break;
				default:
					bool isSuccess = RemoveOldItemsAndAddNewOnes(item, playerId);
					if (!isSuccess) {
						Refund(item.m_priceOnce, playerId);
						DoRpc_Notif_Failed("You can't buy the weapon", "your inventory are full");
						return;
					}

					DoRpc_Notif_Succeed(item.m_priceOnce);
				break;
			}
		}

		m_sessionData.SaveLoadoutChoiceInSession(item, playerId, string.Empty);
	}
	
	// should only be server side
	void Refund(int price, int playerId)
	{
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);
		KOTH_PlayerProfileJson profile = m_kothBackendApi.m_CurrentProfileList.Get(playerUID);
		profile.Refund(price);
		m_kothBackendApi.DoRpcSyncProfileToPlayer(profile);
	}

	// should only be server side
	bool TryBuy(int price, int playerId)
	{
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);
		KOTH_PlayerProfileJson profile = m_kothBackendApi.m_CurrentProfileList.Get(playerUID);
		
		if (price > profile.GetMoney()) {
			return false;
		}
		
		profile.Buy(price);
		m_kothBackendApi.DoRpcSyncProfileToPlayer(profile);
		
		return true;
	}
	
	// ---- OWNER RPC
	void DoRpc_ShowSessionLoadout(KOTH_SessionPlayerLoadout loadout)
	{
		ResourceName primaryResource;
		ResourceName opticResource;
		ResourceName muzzleResource;
		ResourceName handgunResource;
		ResourceName launcherResource;
		ResourceName grenadeResource;
		ResourceName smokeResource;
		ResourceName viperhoodResource;
		ResourceName rangeFinderResource;

		if (loadout.m_primary)
			primaryResource = loadout.m_primary.m_itemResource;
		if (loadout.m_optic)
			opticResource = loadout.m_optic.m_itemResource;
		if (loadout.m_muzzle)
			muzzleResource = loadout.m_muzzle.m_itemResource;
		if (loadout.m_handgun)
			handgunResource = loadout.m_handgun.m_itemResource;
		if (loadout.m_launcher)
			launcherResource = loadout.m_launcher.m_itemResource;
		if (loadout.m_viperhood)
			viperhoodResource = loadout.m_viperhood.m_itemResource;
		if (loadout.m_rangeFinder)
			rangeFinderResource = loadout.m_rangeFinder.m_itemResource;

		int nbGrenade = 0;
		int nbSmoke = 0;
		
		if (loadout.m_throwables)
		{
			foreach(KOTH_ShopItem throwable : loadout.m_throwables)
			{
				if (throwable.m_category == KOTH_ShopItemCategory.Grenade)
				{
					nbGrenade++;
					grenadeResource = throwable.m_itemResource;
				}

				if (throwable.m_category == KOTH_ShopItemCategory.Smoke)
				{
					nbSmoke++;
					smokeResource = throwable.m_itemResource;
				}
			}
		}

		Rpc(RpcDo_SetThrowablesInSession, grenadeResource, nbGrenade, smokeResource, nbSmoke);
		Rpc(RpcDo_ShowSessionLoadout, primaryResource, opticResource, handgunResource, launcherResource, muzzleResource, viperhoodResource, rangeFinderResource);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_SetThrowablesInSession(ResourceName grenade, int nbGrenade, ResourceName smoke, int nbSmoke)
	{
		m_sessionLoadout = new KOTH_SessionPlayerLoadout();
		
		KOTH_ShopItem grenadeItem = FindShopItemByResourceName(grenade);
		KOTH_ShopItem smokeItem = FindShopItemByResourceName(smoke);
		int i = 0;
		while (i < nbGrenade)
		{
			m_sessionLoadout.m_throwables.Insert(grenadeItem);
			i++;
		}
		
		i = 0;
		while (i < nbSmoke)
		{
			m_sessionLoadout.m_throwables.Insert(smokeItem);
			i++;
		}
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_ShowSessionLoadout(ResourceName primary, ResourceName optic, ResourceName handgun, ResourceName launcher, ResourceName muzzle, ResourceName viperhood, ResourceName rangeFinder)
	{
		m_sessionLoadout.m_primary = FindShopItemByResourceName(primary);
		m_sessionLoadout.m_optic = FindShopItemByResourceName(optic);
		m_sessionLoadout.m_handgun = FindShopItemByResourceName(handgun);
		m_sessionLoadout.m_launcher = FindShopItemByResourceName(launcher);
		m_sessionLoadout.m_muzzle = FindShopItemByResourceName(muzzle);
		m_sessionLoadout.m_viperhood = FindShopItemByResourceName(viperhood);
		m_sessionLoadout.m_rangeFinder = FindShopItemByResourceName(rangeFinder);
		
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.KOTH_SessionLoadout);
	}

	void DoRpc_Notif_Succeed(int price)
	{
		Rpc(RpcDo_NotifBuy_Succeed, price);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_NotifBuy_Succeed(int price)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (hudManager) {
			KOTH_HUD kothHud = KOTH_HUD.Cast(hudManager.FindInfoDisplay(KOTH_HUD));
			if (kothHud) {
				kothHud.NotifBuy(price);
			}
		}
	}
	
	void DoRpc_Notif_Failed(string firstLine, string secondLine)
	{
		Rpc(RpcDo_NotifBuy_Failed, firstLine, secondLine);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_NotifBuy_Failed(string firstLine, string secondLine)
	{
		// re open because of confirm menu
		if (firstLine.Contains("weapon"))
			GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.KOTH_ShopWeapon);

		ChimeraMenuBase menu = ChimeraMenuBase.CurrentChimeraMenu();
		if (!menu)
			return;
		
		KOTH_ShopUI shopLayout = KOTH_ShopUI.Cast(menu);
		if (!shopLayout)
			return;
		
		shopLayout.NotifErrorShop(firstLine, secondLine);
	}

	// --------------------- RPC END --------------------- \\
	
	bool RemoveOldItemsAndAddNewOnes(KOTH_ShopItem item, int playerId)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		InventoryStorageManagerComponent inventoryStorageManagerComp = InventoryStorageManagerComponent.Cast(player.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryStorageManagerComp)
			return false;
		
		SCR_CharacterInventoryStorageComponent characterInventoryStorageComp = SCR_CharacterInventoryStorageComponent.Cast(player.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!characterInventoryStorageComp)
			return false;

		BaseInventoryStorageComponent inventoryStorageComponent = characterInventoryStorageComp.GetWeaponStorage();
		if (!inventoryStorageComponent)
			return false;
		
		KOTH_ShopItem itemReplaced = null;

		if (item.m_category == KOTH_ShopItemCategory.Primary || item.m_category == KOTH_ShopItemCategory.Launcher) 
		{
			// remove primary and secondary weapon slot
			IEntity ent1 = inventoryStorageComponent.GetSlot(0).GetAttachedEntity();
			IEntity ent2 = inventoryStorageComponent.GetSlot(1).GetAttachedEntity();

			if (ent1)
			{
				KOTH_ShopItemCategory category = FindEntityKOTH_ShopItemCategory(ent1);
				
				if (category == item.m_category)
				{
					inventoryStorageComponent.GetSlot(0).DetachEntity();
					itemReplaced = FindEntityKOTH_ShopItem(ent1);
					inventoryStorageManagerComp.TryRemoveItemFromStorage(ent1, inventoryStorageComponent);
					RplComponent.DeleteRplEntity(ent1, false);
				}
			}

			if (ent2)
			{
				KOTH_ShopItemCategory category = FindEntityKOTH_ShopItemCategory(ent2);
				
				if (category == item.m_category)
				{
					inventoryStorageComponent.GetSlot(1).DetachEntity();
					itemReplaced = FindEntityKOTH_ShopItem(ent2);
					inventoryStorageManagerComp.TryRemoveItemFromStorage(ent2, inventoryStorageComponent);
					RplComponent.DeleteRplEntity(ent2, false);
				}
			}
		}

		if (item.m_category == KOTH_ShopItemCategory.Handgun) 
		{
			// remove handgun weapon slot
			IEntity ent = inventoryStorageComponent.GetSlot(2).GetAttachedEntity();
			if (ent)
			{
				KOTH_ShopItemCategory category = FindEntityKOTH_ShopItemCategory(ent);
				
				if (category == item.m_category)
				{
					inventoryStorageComponent.GetSlot(2).DetachEntity();
					itemReplaced = FindEntityKOTH_ShopItem(ent);
					inventoryStorageManagerComp.TryRemoveItemFromStorage(ent, inventoryStorageComponent);
					RplComponent.DeleteRplEntity(ent, false);
				}
			}
		}

		// remove previous weapon mags
		if (itemReplaced)
		{
			array<IEntity> outItems = {};
			inventoryStorageManagerComp.GetItems(outItems);
			foreach (IEntity itemInventory : outItems)
			{
				string prefabName = itemInventory.GetPrefabData().GetPrefabName();
				
				if (itemReplaced.m_magazineResource == prefabName || itemReplaced.m_secondaryMagazineResource == prefabName) 
				{
					inventoryStorageManagerComp.TryRemoveItemFromStorage(itemInventory, inventoryStorageComponent);
					RplComponent.DeleteRplEntity(itemInventory, false);
				}
			}			
		}

		// add new weapon mags
		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (item.m_magazineResource)
		{
			if (false == AddMags(inventory, item.m_magazineResource, item.m_magazineNumber))
				return false;
		}
			
		if (item.m_secondaryMagazineResource != string.Empty) 
		{
			if (false == AddMags(inventory, item.m_secondaryMagazineResource, item.m_secondaryMagazineNumber))
				return false;
		}
		
		// add new weapon
        bool successAddInInventory = false;
		if (item.m_category == KOTH_ShopItemCategory.Primary || 
			item.m_category == KOTH_ShopItemCategory.Handgun ||
			item.m_category == KOTH_ShopItemCategory.Launcher
		) {
			IEntity itemBought;			
			successAddInInventory = inventory.TrySpawnPrefabToStorage(item.m_itemResource);
			if (successAddInInventory)
			{
				itemBought = FindIEntityFromResourceName(item.m_itemResource, inventory);
				if (itemBought && item.m_category == KOTH_ShopItemCategory.Primary)
					EquipWeaponInHands_S(itemBought, player);
			}
		} 
		else 
		{
			if (item.m_category == KOTH_ShopItemCategory.Optics || item.m_category == KOTH_ShopItemCategory.Muzzle || item.m_category == KOTH_ShopItemCategory.Bayonet)
			{
				IEntity ent1 = inventoryStorageComponent.GetSlot(0).GetAttachedEntity();
				if (ent1)
				{
					WeaponAttachmentsStorageComponent weaponStorageComponent = WeaponAttachmentsStorageComponent.Cast(ent1.FindComponent(WeaponAttachmentsStorageComponent));
					if (inventory.TrySpawnPrefabToStorage(item.m_itemResource, weaponStorageComponent))
						successAddInInventory = true;
				}
				IEntity ent2 = inventoryStorageComponent.GetSlot(1).GetAttachedEntity();
				if (ent2)
				{
					WeaponAttachmentsStorageComponent weaponStorageComponent = WeaponAttachmentsStorageComponent.Cast(ent2.FindComponent(WeaponAttachmentsStorageComponent));
					if (inventory.TrySpawnPrefabToStorage(item.m_itemResource, weaponStorageComponent))
						successAddInInventory = true;
				}
				// handgun
				IEntity ent3 = inventoryStorageComponent.GetSlot(2).GetAttachedEntity();
				if (ent3)
				{
					WeaponAttachmentsStorageComponent weaponStorageComponent = WeaponAttachmentsStorageComponent.Cast(ent3.FindComponent(WeaponAttachmentsStorageComponent));
					if (inventory.TrySpawnPrefabToStorage(item.m_itemResource, weaponStorageComponent))
						successAddInInventory = true;
				}
			}
			
			if (item.m_category == KOTH_ShopItemCategory.Binocular)
			{				
				InventorySearchPredicate search = new InventorySearchPredicate();
				search.QueryComponentTypes.Insert(SCR_BinocularsComponent);
				array<IEntity> itemsSearched = {};
				inventory.FindItems(itemsSearched, search, EStoragePurpose.PURPOSE_ANY);			

				foreach(IEntity itemFound : itemsSearched)
				{						
					inventory.TryDeleteItem(itemFound);
				}
			}
			
			if (item.m_category == KOTH_ShopItemCategory.ViperHood)
			{
				InventorySearchPredicate search = new InventorySearchPredicate();
				search.QueryComponentTypes.Insert(BaseLoadoutClothComponent);
				array<IEntity> itemsSearched = {};
				inventory.FindItems(itemsSearched, search, EStoragePurpose.PURPOSE_LOADOUT_PROXY);

				foreach(IEntity itemFound : itemsSearched)
				{	
					if (itemFound.GetPrefabData().GetPrefabName().Contains("Helmet"))
					{
						inventory.TryDeleteItem(itemFound);
					}
				}
			}
			
			//handle multiple scope buys ?
			if (!successAddInInventory)
				successAddInInventory = inventory.TrySpawnPrefabToStorage(item.m_itemResource);

			if (successAddInInventory && item.m_category == KOTH_ShopItemCategory.Grenade || 
				successAddInInventory && item.m_category == KOTH_ShopItemCategory.Handgun)
			{
				IEntity itemBought = FindIEntityFromResourceName(item.m_itemResource, inventory);
				inventory.EquipWeapon(itemBought);
			}
		}

		return successAddInInventory;
	}
	
	void RpcDo_LockInventory()
	{
		Rpc(DoRpc_LockInventory);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void DoRpc_LockInventory()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(character.GetCharacterController().GetInventoryStorageManager());
		if (storageManager)
			storageManager.SetInventoryLocked(true);
	}
	
	void RpcDo_UnlockInventory()
	{
		Rpc(DoRpc_UnlockInventory);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void DoRpc_UnlockInventory()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(character.GetCharacterController().GetInventoryStorageManager());
		if (storageManager)
			storageManager.SetInventoryLocked(false);
	}

	int lastTimeEquip = System.GetUnixTime();
	void EquipWeaponInHands_S(IEntity itemBought, IEntity player)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(player);
		if (!character)
	    	return;

		if (System.GetUnixTime() - lastTimeEquip > 5)
		{
			lastTimeEquip = System.GetUnixTime();
			GetGame().GetCallqueue().CallLater(DoRpcLaterEquip, 1000, false, itemBought.GetPrefabData().GetPrefabName());
		} else {
			GetGame().GetCallqueue().CallLater(EquipWeaponInHands_S, 1000, false, itemBought, player);
		}
	}
	void DoRpcLaterEquip(string prefabName)
	{
		Rpc(RpcDoEquipWeaponInHands_O, prefabName);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDoEquipWeaponInHands_O(string prefabName)
	{
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		ChimeraCharacter character = ChimeraCharacter.Cast(player);
		CharacterControllerComponent controller = character.GetCharacterController();
		
		if (!controller)
			return;
		
		if (controller.IsChangingItem())
			return;
		
		SCR_InventoryStorageManagerComponent inventoryStorageManagerComponent = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		IEntity itemBought = FindIEntityFromResourceName(prefabName, inventoryStorageManagerComponent);
		inventoryStorageManagerComponent.EquipWeapon(itemBought);
		
		if (!itemBought) {
			Log("failed to find itemBought, cannot select weapon", LogLevel.WARNING);
			return;
		}
		
		SCR_CharacterInventoryStorageComponent characterInventoryStorageComp = SCR_CharacterInventoryStorageComponent.Cast(player.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!characterInventoryStorageComp)
			return;
		
		BaseInventoryStorageComponent inventoryStorageComponent = characterInventoryStorageComp.GetWeaponStorage();
		if (!inventoryStorageComponent)
			return;
		
		IEntity ent1 = inventoryStorageComponent.GetSlot(0).GetAttachedEntity();
		IEntity ent2 = inventoryStorageComponent.GetSlot(1).GetAttachedEntity();
		if (ent1)
		{
			KOTH_ShopItem item = FindEntityKOTH_ShopItem(ent1);
			if (item.m_category != KOTH_ShopItemCategory.Primary)
			{
				if (ent2) {
					inventoryStorageManagerComponent.TrySwapItemStorages(ent1, ent2);
				} else {
					inventoryStorageManagerComponent.TryMoveItemToStorage(ent1, inventoryStorageComponent, 0);
				}
			}
			
			if (item.m_category == KOTH_ShopItemCategory.Primary)
			{
				BaseWeaponComponent baseWpComp = BaseWeaponComponent.Cast(itemBought.FindComponent(BaseWeaponComponent));
				if (controller.IsGadgetInHands())
					controller.RemoveGadgetFromHand();
		
				controller.SelectWeapon(baseWpComp);
			}
		}
		
		if (ent2)
		{
			KOTH_ShopItem item = FindEntityKOTH_ShopItem(ent2);
			if (item.m_category != KOTH_ShopItemCategory.Launcher)
			{
				if (ent1) {
					inventoryStorageManagerComponent.TrySwapItemStorages(ent1, ent2);
				} else {
					inventoryStorageManagerComponent.TryMoveItemToStorage(ent2, inventoryStorageComponent, 0);
				}
			}
			
			if (item.m_category == KOTH_ShopItemCategory.Primary)
			{
				BaseWeaponComponent baseWpComp = BaseWeaponComponent.Cast(itemBought.FindComponent(BaseWeaponComponent));
				if (controller.IsGadgetInHands())
					controller.RemoveGadgetFromHand();
		
				controller.SelectWeapon(baseWpComp);
			}
		}
		
		//GetGame().GetCallqueue().CallLater(RpcDoEquipWeaponInHands_O, 1000, false, prefabName);
	}

	void DoAskRpc_RearmLoadout(int playerId)
	{
		Rpc(RpcAskRearmLoadout, playerId);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAskRearmLoadout(int playerId)
	{
		IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		SCR_CharacterInventoryStorageComponent characterInventoryStorageComp = SCR_CharacterInventoryStorageComponent.Cast(playerEnt.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!characterInventoryStorageComp)
			return;
		BaseInventoryStorageComponent inventoryStorageComponent = characterInventoryStorageComp.GetWeaponStorage();
		if (!inventoryStorageComponent)
			return;

		int price = KOTH_Helper.ComputeRearmPrice(playerEnt, playerController);
		if (!price)
		{
			Log("ComputeRearmPrice is null");
			return;
		}
		
		bool buySuccess = TryBuy(price, playerId);
		if (!buySuccess) 
		{
			DoRpc_Notif_Failed("You can't rearm", "not enough money");
			return;
		}

		IEntity ent1 = inventoryStorageComponent.GetSlot(0).GetAttachedEntity();
		if (ent1)
			RearmEntityAndSecondary(ent1, playerEnt);
		
		IEntity ent2 = inventoryStorageComponent.GetSlot(1).GetAttachedEntity();
		if (ent2)
			RearmEntityAndSecondary(ent2, playerEnt);
		
		IEntity ent3 = inventoryStorageComponent.GetSlot(2).GetAttachedEntity();
		if (ent3)
			RearmEntityAndSecondary(ent3, playerEnt);
	}
	
	void RearmEntityAndSecondary(IEntity entity, IEntity playerEnt)
	{
		KOTH_ShopItem item = FindShopItemByResourceName(entity.GetPrefabData().GetPrefabName());
		SCR_InventoryStorageManagerComponent inventoryStorageManagerComp = SCR_InventoryStorageManagerComponent.Cast(playerEnt.FindComponent(SCR_InventoryStorageManagerComponent));
		int count = inventoryStorageManagerComp.GetDepositItemCountByResource(GetOwner(), item.m_magazineResource);
		for (int i = count; i < item.m_magazineNumber; i++)
		{
			inventoryStorageManagerComp.TrySpawnPrefabToStorage(item.m_magazineResource);
			Log("added "+item.m_magazineResource);
		}
		
		int countBis = inventoryStorageManagerComp.GetDepositItemCountByResource(GetOwner(),item.m_secondaryMagazineResource);
		for (int i = countBis; i < item.m_secondaryMagazineNumber; i++)
		{
			inventoryStorageManagerComp.TrySpawnPrefabToStorage(item.m_secondaryMagazineResource);
			Log("added "+item.m_magazineResource);
		}
	}
	
	
	void DoAskRpc_RearmVehicle(int playerId)
	{
		Rpc(RpcAskRearmVehicle, playerId);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAskRearmVehicle(int playerId)
	{
		IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		SCR_CompartmentAccessComponent compartmentAccessComp = SCR_CompartmentAccessComponent.Cast(playerEnt.FindComponent(SCR_CompartmentAccessComponent));
		BaseCompartmentSlot compartment = compartmentAccessComp.GetCompartment();
		if (!compartment)
			return;
	
		IEntity vehicle = compartment.GetVehicle();
		int price = KOTH_Helper.ComputeVehicleRearmPrice(playerId);
		if (!price)
		{
			Log("ComputeVehicleRearmPrice is null");
			return;
		}
	
		bool buySuccess = TryBuy(price, playerId);
		if (!buySuccess)
		{
			DoRpc_Notif_Failed("You can't rearm", "not enough money");
			return;
		}
	
		KOTH_ShopItem item = FindEntityKOTH_ShopItem(vehicle);
		if (!item)
			return;
	
		SCR_VehicleInventoryStorageManagerComponent inventoryStorageManagerComp = SCR_VehicleInventoryStorageManagerComponent.Cast(vehicle.FindComponent(SCR_VehicleInventoryStorageManagerComponent));
		if (!inventoryStorageManagerComp)
			return;
	
		array<Managed> allVehicleWeapSlots = {};
		vehicle.FindComponents(WeaponSlotComponent, allVehicleWeapSlots);
	
		if (item.m_magazineResource)
		{
			RearmWeaponSlots(allVehicleWeapSlots, inventoryStorageManagerComp, item.m_magazineResource, item.m_magazineNumber);
		}
	
		if (item.m_secondaryMagazineResource)
		{
			RearmWeaponSlots(allVehicleWeapSlots, inventoryStorageManagerComp, item.m_secondaryMagazineResource, item.m_secondaryMagazineNumber);
		}
	}
	
	void RearmWeaponSlots(array<Managed> weaponSlots, SCR_VehicleInventoryStorageManagerComponent inventoryStorageManagerComp, ResourceName magazineResource, int requiredMagazineCount)
	{
		bool hasAValidWeaponSlot = false;
	
		foreach (Managed weapSlotTemp : weaponSlots)
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
					hasAValidWeaponSlot = true;
					BaseMagazineComponent magazineComp = weaponComp.GetCurrentMagazine();
					if (magazineComp)
					{
						LogWorkbench("Current vehicle weapon has " + magazineComp.ToString() + " rounds before rearm");
						magazineComp.SetAmmoCount(magazineComp.GetMaxAmmoCount());
					}
				}
			}
		}
	
		if (!hasAValidWeaponSlot)
		{
			int currentCount = inventoryStorageManagerComp.GetDepositItemCountByResource(GetOwner(), magazineResource);
			for (int i = currentCount; i < requiredMagazineCount; i++)
			{
				inventoryStorageManagerComp.TrySpawnPrefabToStorage(magazineResource);
				Log("Added " + magazineResource);
			}
		}
	}

	IEntity FindIEntityFromResourceName(string resourceName, SCR_InventoryStorageManagerComponent inventory)
	{
		IEntity entity;
		array<IEntity> outItems = {};
		inventory.GetItems(outItems);
		foreach(IEntity item : outItems)
		{
			if (item.GetPrefabData().GetPrefabName() == resourceName)
				entity = item;
		}
		return entity;
	}
	
	KOTH_ShopItem FindEntityKOTH_ShopItem(IEntity entity)
	{
		string prefabData = entity.GetPrefabData().GetPrefabName();
		return FindShopItemByResourceName(prefabData);
	}
	
	KOTH_ShopItemCategory FindEntityKOTH_ShopItemCategory(IEntity entity)
	{
		string prefabData = entity.GetPrefabData().GetPrefabName();
		KOTH_ShopItemCategory currentItemCategory;
	
		foreach(string shopItemListResourceName : m_shopItemListResources)
		{
			array<ref KOTH_ShopItem> shopList = SCR_ConfigHelperT<KOTH_ShopItemList>
				.GetConfigObject(shopItemListResourceName)
				.GetItems();
			
			foreach(KOTH_ShopItem shopItem : shopList)
			{
				if (shopItem.m_itemResource == prefabData) {
					currentItemCategory = shopItem.m_category;
					break;
				}
			}
		}
		
		return currentItemCategory;
	}
	
	KOTH_ShopItem FindShopItemByResourceName(string resourceName)
	{
		KOTH_ShopItem item;
		
		foreach(string shopItemListResourceName : m_shopItemListResources)
		{
			array<ref KOTH_ShopItem> shopList = SCR_ConfigHelperT<KOTH_ShopItemList>
				.GetConfigObject(shopItemListResourceName)
				.GetItems();
			
			foreach(KOTH_ShopItem shopItem : shopList)
			{
				if (shopItem.m_itemResource == resourceName)
				{
					item = shopItem;
					break;
				}
			}
		}
		
		return item;
	}

	bool AddMags(SCR_InventoryStorageManagerComponent inventory, string itemResource, int itemNumber)
	{
		bool minimumMagInsertOk = true;
		for (int i = 1; i <= itemNumber; i++)
		{
        	bool tryInsert = inventory.TrySpawnPrefabToStorage(itemResource);
			if (i == 2 && tryInsert == false) 
			{
				minimumMagInsertOk = false;
				break;
			}
		}
		
		return minimumMagInsertOk;
	}
}