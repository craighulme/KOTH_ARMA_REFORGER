class KOTH_SCR_ReviveAction : ScriptedUserAction
{
	PlayerManager m_playerMng = GetGame().GetPlayerManager();
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		SetActionDuration(6);
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		bool canBeRevived = CanBeRevived(pUserEntity);
		if (!canBeRevived)
			return;

		ChimeraCharacter chimera = ChimeraCharacter.Cast(GetOwner());
		if (!chimera)
			return;

		array<HitZone> hitZones = {};
		SCR_CharacterDamageManagerComponent scrCharDmgMngComp = SCR_CharacterDamageManagerComponent.Cast(chimera.FindComponent(SCR_CharacterDamageManagerComponent));
		scrCharDmgMngComp.GetAllHitZonesInHierarchy(hitZones);
		foreach (HitZone hitZone : hitZones)
		{
			// restore 3/4 of health to every zone
			float threeQuarterHealth = (hitZone.GetMaxHealth() / 4) * 3;
			if (hitZone.GetHealth() < threeQuarterHealth)
				hitZone.SetHealth(threeQuarterHealth);
		}

		CharacterControllerComponent controllerComp = chimera.GetCharacterController();
		if (controllerComp)
			controllerComp.SetUnconscious(false);

		scrCharDmgMngComp.SoundHeal();
		scrCharDmgMngComp.SetPermitUnconsciousness(false, false);

		int playerId = m_playerMng.GetPlayerIdFromControlledEntity(pUserEntity);
		PlayerController playerController = m_playerMng.GetPlayerController(playerId);
		KOTH_SCR_PlayerProfileComponent playerProfileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));

		// add bonus to healer 
		string playerUID = KOTH_Helper.GetPlayerUID(playerId);
		int bonus = KOTH_ExperienceManager.GetInstance().GetReviveBonus(playerUID);
		KOTH_BackendApiGameModeComponent kothBackendApi = KOTH_BackendApiGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_BackendApiGameModeComponent));
		KOTH_PlayerProfileJson profile = kothBackendApi.m_CurrentProfileList.Get(playerUID);
		profile.AddXp(bonus);
		profile.AddMoney(bonus);
		KOTH_SessionDataGameModeComponent sessionDataGameComp = KOTH_SessionDataGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_SessionDataGameModeComponent));
		sessionDataGameComp.AddSessionXpAndMoney(bonus, bonus, playerUID);
		playerProfileComp.DoRpc_SyncPlayerProfile(profile);
		playerProfileComp.DoRpc_NotifReviveFriendly(bonus.ToString());

		// get assist system for the revive action
		KOTH_AssistSystemComponent assistSystem = KOTH_AssistSystemComponent.Cast(GetGame().GetGameMode().FindComponent(KOTH_AssistSystemComponent));
		int revivedId = m_playerMng.GetPlayerIdFromControlledEntity(pOwnerEntity);
		if (assistSystem)
		{
			if (revivedId != playerId) // don't link assist if the reviver is the same as the revived player
			{
				assistSystem.AddAssistRelationship(revivedId, playerId);
			}
		}

		// remove a bandage from healer (from the end of the array)
		array<IEntity> bandages = GetBandages(pUserEntity);
		if (!bandages.IsEmpty())
		{
			IEntity lastBandage = bandages[bandages.Count() - 1];
			if (lastBandage)
				RplComponent.DeleteRplEntity(lastBandage, false);
		}

		Log("Player "+m_playerMng.GetPlayerName(playerId)+" revived player "+m_playerMng.GetPlayerName(revivedId));
	}


	override bool CanBePerformedScript(IEntity user) 
	{
		array<IEntity> bandages = GetBandages(user);
		if (bandages.Count() <= 0)
		{
			SetCannotPerformReason("No bandage");
			return false;
		}

		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		return CanBeRevived(user);
	}

	private bool CanBeRevived(IEntity user)
	{
		IEntity ownerEntity = SCR_EntityHelper.GetMainParent(GetOwner(), true);
		Faction userFaction = KOTH_Helper.GrabFaction(user);
		Faction ownerFaction = KOTH_Helper.GrabFaction(ownerEntity);
		
		if (!userFaction || !ownerFaction)
			return false;

		if (!userFaction || !ownerFaction)
    			return false;
		
		if (userFaction.GetFactionKey() != ownerFaction.GetFactionKey())
			return false;
		
		ChimeraCharacter chimera = ChimeraCharacter.Cast(ownerEntity);
		if (!chimera)
			return false;

		CharacterControllerComponent controllerComp = chimera.GetCharacterController();
		if (!controllerComp.IsUnconscious())
			return false;
		
		if (controllerComp.IsDead())
			return false;
		
		return true;
	}
	
	private array<IEntity> GetBandages(IEntity user)
	{
		array<IEntity> bandages = {};
		array<IEntity> foundItems = {};
		
		InventoryStorageManagerComponent inventoryStorageManagerComp = InventoryStorageManagerComponent.Cast(user.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryStorageManagerComp)
			return bandages;
		
		inventoryStorageManagerComp.FindItemsWithComponents(foundItems, {SCR_ConsumableItemComponent});
		foreach (IEntity item : foundItems)
		{
			InventoryItemComponent invItemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			if (invItemComp.GetAttributes().GetCommonType() == ECommonItemType.BANDAGE)
				bandages.Insert(item);
		}
		
		return bandages;
	}
}
