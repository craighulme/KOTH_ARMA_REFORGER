class KOTH_RepairZoneClass : SCR_BaseTriggerEntityClass{}
class KOTH_RepairZone : SCR_BaseTriggerEntity
{
    const int TIMER_TO_REPAIR = 40;
    const int TIMER_TO_REPAIR_ARMED = 100;
    const int TICK_RATE = 5;

    protected IEntity m_currentVehicleToRepair;
    protected int m_timer;

	protected ref map<int, bool> m_notifiedPlayers; // Tracks players notified for rearm
	
    override protected void EOnInit(IEntity owner)
    {
        if (SCR_Global.IsEditMode(owner))
            return;

        if (!Replication.IsServer())
            return;

        BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(owner);
        if (!trigger)
            return;

		m_notifiedPlayers = new map<int, bool>();
		
        trigger.AddClassType(Vehicle);
        trigger.SetUpdateRate(0);
        trigger.EnablePeriodicQueries(false);
        GetGame().GetCallqueue().CallLater(QueryEntitiesInside, KOTH_RepairZone.TICK_RATE * 1000, true);
    }

    override event protected void OnQueryFinished(bool bIsEmpty)
    {
        super.OnQueryFinished(bIsEmpty);

        array<IEntity> outEntities = {};
        GetEntitiesInside(outEntities);
        PlayerManager playerManager = GetGame().GetPlayerManager();

        foreach (IEntity entity : outEntities)
        {
            array<IEntity> occupants = {};
            SCR_BaseCompartmentManagerComponent baseCompartmentManagerComp = SCR_BaseCompartmentManagerComponent.Cast(entity.FindComponent(SCR_BaseCompartmentManagerComponent));
            baseCompartmentManagerComp.GetOccupantsOfType(occupants, ECompartmentType.PILOT);

            foreach (IEntity playerEntity : occupants)
            {
                int playerId = playerManager.GetPlayerIdFromControlledEntity(playerEntity);
				
				// Skip if player has already been notified
			    if (m_notifiedPlayers.Contains(playerId))
			        continue;
				
                if (!KOTH_Helper.ComputeVehicleRearmPrice(playerId))
                    continue;

                PlayerController playerController = playerManager.GetPlayerController(playerId);
                KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
                profileComp.DoRpc_ShowVehicleRearm();
				
				// Mark as notified
    				m_notifiedPlayers.Insert(playerId, true);
            }
			
			// Remove players no longer in the zone for notification repair
		    foreach (int notifiedPlayerId, bool _ : m_notifiedPlayers)
		    {
		        bool playerStillInZone = false;
		        foreach (IEntity playerEntity : occupants)
		        {
		            int playerId = playerManager.GetPlayerIdFromControlledEntity(playerEntity);
		            if (playerId == notifiedPlayerId)
		            {
		                playerStillInZone = true;
		                continue;
		            }
		        }
		
		        if (!playerStillInZone)
		        {
		            // Player is no longer in the zone, remove from notified list
		            m_notifiedPlayers.Remove(notifiedPlayerId);
		            LogWorkbench("RepairZone: Removed player " + notifiedPlayerId + " from notification list due to exit or timeout.");
		        }
		    }

            SCR_VehicleDamageManagerComponent vehDmgComp = SCR_VehicleDamageManagerComponent.Cast(entity.FindComponent(SCR_VehicleDamageManagerComponent));
            if (!vehDmgComp.CanBeHealed())
                continue;

            if (entity == m_currentVehicleToRepair)
            {
                if (m_timer >= TIMER_TO_REPAIR)
                {
                    if (vehDmgComp)
                    {
                        vehDmgComp.FullHeal();
						
                        // Reset tracking damage since it's repaired
                        Vehicle vehicleEntity = Vehicle.Cast(entity);
                        if (vehicleEntity)
                        {
                            KOTH_VehicleDamageTracker damageTracker = vehicleEntity.GetDamageTracker();
                            if (damageTracker)
                            {
                                damageTracker.ResetAllDamage();
                            }
                        }
						
						//refuel vehicle as well
						SCR_FuelNode fuelTank;
						array<BaseFuelNode> fuelTanks = {};
						FuelManagerComponent m_fuelManager = FuelManagerComponent.Cast(vehicleEntity.FindComponent(FuelManagerComponent));
						
						m_fuelManager.GetFuelNodesList(fuelTanks);
						foreach (BaseFuelNode fuelNode : fuelTanks)
						{
							fuelTank = SCR_FuelNode.Cast(fuelNode);
							if (!fuelTank)
								continue;

							fuelTank.SetFuel(fuelTank.GetMaxFuel());
							LogWorkbench("RepairZone: Refueled fuelTank " + fuelTank.GetFuelTankID() + "to " + fuelTank.GetMaxFuel());
						}
						
                        continue; // Continue to process other vehicles
                    }
                }
                else
                {
                    m_timer = m_timer + KOTH_RepairZone.TICK_RATE;
                }
                return;
            }

			
            m_currentVehicleToRepair = entity;

            foreach (IEntity playerEntity : occupants)
            {
                int playerId = playerManager.GetPlayerIdFromControlledEntity(playerEntity);
                PlayerController playerController = playerManager.GetPlayerController(playerId);
                KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
                profileComp.DoRpc_ShowStartRepair();
            }

            return;
        }

        // Stop repair
        m_timer = 0;
        if (m_currentVehicleToRepair)
        {
            SCR_VehicleDamageManagerComponent vehDmgComp = SCR_VehicleDamageManagerComponent.Cast(m_currentVehicleToRepair.FindComponent(SCR_VehicleDamageManagerComponent));
            array<IEntity> occupants = {};
            SCR_BaseCompartmentManagerComponent baseCompartmentManagerComp = SCR_BaseCompartmentManagerComponent.Cast(m_currentVehicleToRepair.FindComponent(SCR_BaseCompartmentManagerComponent));
            baseCompartmentManagerComp.GetOccupantsOfType(occupants, ECompartmentType.PILOT);

            foreach (IEntity playerEntity : occupants)
            {
                int playerId = playerManager.GetPlayerIdFromControlledEntity(playerEntity);
				
				// Clear notification state
			    if (m_notifiedPlayers.Contains(playerId))
			        m_notifiedPlayers.Remove(playerId);
				
                PlayerController playerController = playerManager.GetPlayerController(playerId);
                KOTH_SCR_PlayerProfileComponent profileComp = KOTH_SCR_PlayerProfileComponent.Cast(playerController.FindComponent(KOTH_SCR_PlayerProfileComponent));
                profileComp.DoRpc_StopRepair();
            }
        }

        m_currentVehicleToRepair = null;
    }
}
