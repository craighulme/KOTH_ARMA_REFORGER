class KOTH_SpawnHelper
{
	static void SpawnFreeVehicles()
	{
		IEntity vehicleSpawnFirstTruck = GetGame().GetWorld().FindEntityByName("KOTH_FirstFreeTruck");
		if (vehicleSpawnFirstTruck)
		{
			SCR_AmbientVehicleSpawnPointComponent compSecFirstTruck = SCR_AmbientVehicleSpawnPointComponent.Cast(vehicleSpawnFirstTruck.FindComponent(SCR_AmbientVehicleSpawnPointComponent));
			compSecFirstTruck.SpawnTruck();
		}
		IEntity vehicleSpawnFirstHelo = GetGame().GetWorld().FindEntityByName("KOTH_FirstFreeHelo");
		if (vehicleSpawnFirstHelo)
		{
			SCR_AmbientVehicleSpawnPointComponent compSecFirstHelo = SCR_AmbientVehicleSpawnPointComponent.Cast(vehicleSpawnFirstHelo.FindComponent(SCR_AmbientVehicleSpawnPointComponent));
			compSecFirstHelo.SpawnHelo();
		}

		IEntity vehicleSpawnSecondTruck = GetGame().GetWorld().FindEntityByName("KOTH_SecondFreeTruck");
		if (vehicleSpawnSecondTruck)
		{
			SCR_AmbientVehicleSpawnPointComponent compSecSecondTruck = SCR_AmbientVehicleSpawnPointComponent.Cast(vehicleSpawnSecondTruck.FindComponent(SCR_AmbientVehicleSpawnPointComponent));
			compSecSecondTruck.SpawnTruck();
		}
		IEntity vehicleSpawnSecondHelo = GetGame().GetWorld().FindEntityByName("KOTH_SecondFreeHelo");
		if (vehicleSpawnSecondHelo)
		{
			SCR_AmbientVehicleSpawnPointComponent compSecSecondHelo = SCR_AmbientVehicleSpawnPointComponent.Cast(vehicleSpawnSecondHelo.FindComponent(SCR_AmbientVehicleSpawnPointComponent));
			compSecSecondHelo.SpawnHelo();
		}

		IEntity vehicleSpawnThirdTruck = GetGame().GetWorld().FindEntityByName("KOTH_ThirdFreeTruck");
		if (vehicleSpawnThirdTruck)
		{
			SCR_AmbientVehicleSpawnPointComponent compSecThreeTruck = SCR_AmbientVehicleSpawnPointComponent.Cast(vehicleSpawnThirdTruck.FindComponent(SCR_AmbientVehicleSpawnPointComponent));
			compSecThreeTruck.SpawnTruck();
		}
		IEntity vehicleSpawnThirdHelo = GetGame().GetWorld().FindEntityByName("KOTH_ThirdFreeHelo");
		if (vehicleSpawnThirdHelo)
		{
			SCR_AmbientVehicleSpawnPointComponent compSecThreeHelo = SCR_AmbientVehicleSpawnPointComponent.Cast(vehicleSpawnThirdHelo.FindComponent(SCR_AmbientVehicleSpawnPointComponent));
			compSecThreeHelo.SpawnHelo();
		}
	}

	static IEntity FindSpawnPoint(IEntity parent)
	{
		if (!parent)
			return null;

		IEntity child = parent.GetChildren();
		SCR_SpawnPoint spawn;
		for (int i = 0; i < 100; i++)
		{
			spawn = SCR_SpawnPoint.Cast(child);
			if (spawn)
				break;

			child = child.GetSibling();
		}

		if (!spawn)
			return null;

		return child;
	}

	static KOTH_SpawnProtectionTriggerEntity FindSpawnProtection(IEntity parent)
	{
		IEntity child = parent.GetChildren();
		KOTH_SpawnProtectionTriggerEntity spawn;
		for (int i = 0; i < 100; i++)
		{
			spawn = KOTH_SpawnProtectionTriggerEntity.Cast(child);
			if (spawn)
				break;

			if (child)
				child = child.GetSibling();
		}

		if (!spawn)
			return null;

		return KOTH_SpawnProtectionTriggerEntity.Cast(child);
	}

	static IEntity FindFlag(IEntity parent)
	{
		if (!parent)
			return null;

		IEntity child = parent.GetChildren();
		KOTH_FlagComponent flag;
		for (int i = 0; i < 100; i++)
		{
			if (child) {
				flag = KOTH_FlagComponent.Cast(child.FindComponent(KOTH_FlagComponent));
				if (flag)
					return child;

				child = child.GetSibling();
			}
		}

		return null;
	}

	static void AttachProperFlag(IEntity spawn)
	{
		if (!spawn)
			return;

		SCR_SpawnPoint sp = SCR_SpawnPoint.Cast(KOTH_SpawnHelper.FindSpawnPoint(spawn));
		IEntity flagPoleEntity = KOTH_SpawnHelper.FindFlag(spawn);
		Resource res;

		if (sp.GetFactionKey() == KOTH_Faction.BLUFOR) {
			res = Resource.Load("{31F54FB5494520B8}Prefabs/Props/Fabric/Flags/Flag_1_Blufor.et");
		}
		if (sp.GetFactionKey() == KOTH_Faction.OPFOR) {
			res = Resource.Load("{167B394478139624}Prefabs/Props/Fabric/Flags/Flag_1_Opfor.et");
		}
		if (sp.GetFactionKey() == KOTH_Faction.INDFOR) {
			res = Resource.Load("{AC8F5AFDF613616F}Prefabs/Props/Fabric/Flags/Flag_1_Indfor.et");
		}

		IEntity flagEntity = GetGame().SpawnEntityPrefab(res);

		SlotManagerComponent compSec = SlotManagerComponent.Cast(flagPoleEntity.FindComponent(SlotManagerComponent));
		EntitySlotInfo slotInfo = compSec.GetSlotByName("Flag");
		slotInfo.AttachEntity(flagEntity);
	}
}
